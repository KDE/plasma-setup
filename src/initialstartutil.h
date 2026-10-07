// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QWindow>
#include <qqmlintegration.h>

#include <KJob>
#include <KOSRelease>
#include <sessionmanagement.h>

#include "accountcontroller.h"

class InitialStartUtil : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString distroName READ distroName CONSTANT);
    Q_PROPERTY(int state READ state NOTIFY stateChanged)
    Q_PROPERTY(QStringList postSetupActionNames READ postSetupActionNames NOTIFY postSetupActionNamesChanged)

public:
    InitialStartUtil(QObject *parent = nullptr);

    enum SetupState {
        Asking,
        Finishing,
        Finished,
    };
    Q_ENUM(SetupState)

    QString distroName() const;
    int state() const;
    QStringList postSetupActionNames() const;

    /*!
        \fn void logOut()

        Logs out of the plasma-setup user.
     */
    Q_INVOKABLE void logOut();

    /*!
        \fn void finish()

        Runs setup actions, such as creating the new user. Does not log out.
     */
    Q_INVOKABLE void finish();

    /*!
        \fn void registerPostSetupAction(KJob *action, QString description)

        Registers the given \a action to be started after user creation.
        A human-readable description of the action should be given in \a description.

        This allows modules to run post-setup hooks for the new user.
     */
    Q_INVOKABLE void registerPostSetupAction(KJob *action, QString description);

    /*!
        \fn void unregisterPostSetupAction(KJob *action)

        Unregisters the given \a action from the table of post-setup actions.
     */
    Q_INVOKABLE void unregisterPostSetupAction(KJob *action);

    /**
     * Removes the autologin configuration for Plasma Setup.
     *
     * This allows the next login to be a normal login, unless the Plasma Setup systemd service runs again.
     */
    void disablePlasmaSetupAutologin();

    /**
     * Checks if the service is running as the plasma-setup user.
     *
     * Useful for conditionals where e.g. user/system settings shouldn't be modified when
     * running on a developer's machine.
     *
     * We use this instead of QT_DEBUG for compatibility with distros that build with
     * debug symbols enabled by default, like KDE Neon.
     *
     * @return true if running as plasma-setup user, false otherwise.
     */
    static bool runningAsPlasmaSetupUser();

Q_SIGNALS:
    void stateChanged();
    void postSetupActionNamesChanged();

private:
    /**
     * Performs the finishing steps specific to the creation of the new user.
     *
     * Does not handle the system-wide completion steps such as network configuration, keyboard layout, etc.
     * This separation is needed for when Plasma Setup is run in a context where no user creation is needed.
     */
    void doUserCreationSteps();

    /**
     * Enables temporary autologin for the specified new user.
     *
     * This function configures the system so that after the initial setup, the new user will be automatically logged in once,
     * facilitating the transition from the plasma-setup user to the newly created user account.
     */
    void setNewUserTempAutologin();

    /**
     * Creates the completion flag file (usually /etc/plasma-setup-done) to indicate that initial setup is complete.
     */
    void createCompletionFlag();

    /**
     * Creates an autostart hook for the new user to cleanup the initial setup configuration.
     */
    void createNewUserAutostartHook();

    /*!
     * Slot that handles a finished post-setup action.
     *
     * Once all post-setup actions are finished, we proceed to logout.
     */
    void finishPostSetupAction(KJob *);

    /*
     * The account controller instance that manages the new user account creation.
     */
    AccountController *m_accountController;

    /**
     * Provides access to the operating system information.
     */
    KOSRelease m_osrelease;

    /**
     * The window used as the parent for KAuth actions.
     *
     * Without passing this KAuth spams the console with warnings about missing parent windows,
     * which is not relevant to us since we never show an authorization dialog.
     */
    QWindow *m_window = nullptr;

    /**
     * Provides session management capabilities, notably for logging out of plasma-setup user session.
     */
    SessionManagement m_session;

    int m_state = SetupState::Asking;
    QHash<KJob *, QString> m_postSetupActions;
};
