// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "initialstartutil.h"
#include "displayutil.h"
#include "plasmasetup_debug.h"

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KLocalizedString>

#include <QApplication>

InitialStartUtil::InitialStartUtil(QObject *parent)
    : QObject{parent}
    , m_accountController(AccountController::instance())
{
    QList<QWindow *> topLevelWindows = QGuiApplication::topLevelWindows();
    m_window = topLevelWindows.isEmpty() ? nullptr : topLevelWindows.first();
    disablePlasmaSetupAutologin();
}

QString InitialStartUtil::distroName() const
{
    return m_osrelease.name();
}

QString InitialStartUtil::finishedMessage() const
{
    if (m_accountController->hasExistingUsers()) {
        return i18nc("%1 is the distro name", "Your device is now ready.<br /><br />Enjoy <b>%1</b>!", m_osrelease.name());
    }

    return i18nc(
        "%1 is the distro name",
        "Your device is now ready.<br /><br />After clicking <b>Finish</b> you will be able to sign in to your new account.<br /><br />Enjoy <b>%1</b>!",
        m_osrelease.name() //
    );
}

void InitialStartUtil::finish()
{
    doUserCreationSteps();
    createCompletionFlag();
    logOut();
}

void InitialStartUtil::doUserCreationSteps()
{
    if (m_accountController->hasExistingUsers()) {
        qCInfo(PlasmaSetup) << "Skipping user creation steps since existing users were detected.";
        return;
    }

    const bool userCreated = m_accountController->createUser();
    if (!userCreated) {
        qCWarning(PlasmaSetup) << "Failed to create user:" << m_accountController->username();
        // TODO: Handle the error appropriately, e.g., show a message to the user
        return;
    }

    // Temporarily disabling the automatic session transition because using SDDM's
    // Autologin causes some issues, like being unable to create a wallet and potentially
    // connecting to new wifi networks until after a reboot. This isn't an issue when the user
    // logs in normally with their password. Re-enable these when we can ensure the automatic
    // transition doesn't cause such issues.
    // setNewUserTempAutologin();
    // createNewUserAutostartHook();

    DisplayUtil displayUtil;
    displayUtil.setGlobalThemeForNewUser(m_window, m_accountController->username());
    displayUtil.setScalingForNewUser(m_window, m_accountController->username());
}

void InitialStartUtil::disablePlasmaSetupAutologin()
{
    qCInfo(PlasmaSetup) << "Removing autologin configuration for plasma-setup user.";

    KAuth::Action action(QStringLiteral("org.kde.plasmasetup.removeautologin"));
    action.setParentWindow(m_window);
    action.setHelperId(QStringLiteral("org.kde.plasmasetup"));
    KAuth::ExecuteJob *job = action.execute();

    if (!job->exec()) {
        qCWarning(PlasmaSetup) << "Failed to remove autologin configuration:" << job->errorString();
    } else {
        qCInfo(PlasmaSetup) << "Autologin configuration removed successfully.";
    }
}

void InitialStartUtil::logOut()
{
    m_session.requestLogout(SessionManagement::ConfirmationMode::Skip);
}

void InitialStartUtil::setNewUserTempAutologin()
{
    const QString username = m_accountController->username();
    qCInfo(PlasmaSetup) << "Setting temporary autologin for new user:" << username;

    KAuth::Action action(QStringLiteral("org.kde.plasmasetup.setnewusertempautologin"));
    action.setParentWindow(m_window);
    action.setHelperId(QStringLiteral("org.kde.plasmasetup"));
    action.setArguments({{QStringLiteral("username"), username}});
    KAuth::ExecuteJob *job = action.execute();

    if (!job->exec()) {
        qCWarning(PlasmaSetup) << "Failed to set temporary autologin for new user:" << job->errorString();
    } else {
        qCInfo(PlasmaSetup) << "Temporary autologin set for new user successfully.";
    }
}

void InitialStartUtil::createCompletionFlag()
{
    qCInfo(PlasmaSetup) << "Creating plasma-setup completion flag file.";

    KAuth::Action action(QStringLiteral("org.kde.plasmasetup.createflagfile"));
    action.setParentWindow(m_window);
    action.setHelperId(QStringLiteral("org.kde.plasmasetup"));
    KAuth::ExecuteJob *job = action.execute();

    if (!job->exec()) {
        qCWarning(PlasmaSetup) << "Failed to create completion flag file:" << job->errorString();
    } else {
        qCInfo(PlasmaSetup) << "Completion flag file created successfully.";
    }
}

void InitialStartUtil::createNewUserAutostartHook()
{
    const QString username = m_accountController->username();
    qCInfo(PlasmaSetup) << "Creating autostart hook for new user:" << username;

    KAuth::Action action(QStringLiteral("org.kde.plasmasetup.createnewuserautostarthook"));
    action.setParentWindow(m_window);
    action.setHelperId(QStringLiteral("org.kde.plasmasetup"));
    action.setArguments({{QStringLiteral("username"), username}});
    KAuth::ExecuteJob *job = action.execute();

    if (!job->exec()) {
        qCWarning(PlasmaSetup) << "Failed to create autostart hook for new user:" << job->errorString();
    } else {
        qCInfo(PlasmaSetup) << "Autostart hook created successfully for new user.";
    }
}

#include "initialstartutil.moc"

#include "moc_initialstartutil.cpp"
