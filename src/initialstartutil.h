// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QWindow>
#include <qqmlintegration.h>

#include <KOSRelease>
#include <sessionmanagement.h>

#include "accountcontroller.h"

class InitialStartUtil : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString distroName READ distroName CONSTANT);

public:
    InitialStartUtil(QObject *parent = nullptr);

    QString distroName() const;

    /**
     * Completes the initial setup process.
     */
    Q_INVOKABLE void finish();

    /**
     * Removes the autologin configuration for Plasma Setup.
     *
     * This allows the next login to be a normal login, unless the Plasma Setup systemd service runs again.
     */
    void disablePlasmaSetupAutologin();

private:
    /**
     * Logs out of the plasma-setup user.
     *
     * This will cause the new user to be logged in automatically, since the autologin is set for the new user.
     */
    void logOut();

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
};
