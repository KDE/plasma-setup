// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "displayutil.h"
#include "plasmasetup_debug.h"

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QWindow>

DisplayUtil::DisplayUtil(QObject *parent)
    : QObject(parent)
{
}

void DisplayUtil::setGlobalThemeForNewUser(QWindow *window, QString userName)
{
    qCInfo(PlasmaSetup) << "Setting global theme for new user.";

    KAuth::Action action(QStringLiteral("org.kde.plasmasetup.setnewuserglobaltheme"));
    action.setParentWindow(window);
    action.setHelperId(QStringLiteral("org.kde.plasmasetup"));
    action.addArgument(QStringLiteral("username"), userName);

    KAuth::ExecuteJob *job = action.execute();

    if (!job->exec()) {
        qCWarning(PlasmaSetup) << "Failed to set global theme for new user:" << job->errorString();
    } else {
        qCInfo(PlasmaSetup) << "Global theme for new user set successfully.";
    }
}

void DisplayUtil::setScalingForNewUser(QWindow *window, QString userName)
{
    qCInfo(PlasmaSetup) << "Setting scaling for new user:" << userName;

    KAuth::Action action(QStringLiteral("org.kde.plasmasetup.setnewuserdisplayscaling"));
    action.setParentWindow(window);
    action.setHelperId(QStringLiteral("org.kde.plasmasetup"));
    action.addArgument(QStringLiteral("username"), userName);

    KAuth::ExecuteJob *job = action.execute();

    if (!job->exec()) {
        qCWarning(PlasmaSetup) << "Failed to set scaling for new user:" << job->errorString();
    } else {
        qCInfo(PlasmaSetup) << "Set scaling for new user.";
    }
}

#include "moc_displayutil.cpp"
