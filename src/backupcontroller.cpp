// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "backupcontroller.h"

#include <QGuiApplication>

#include "config-plasma-setup.h"

BackupController::BackupController(QObject *parent)
    : QObject(parent)
{
}

BackupController *BackupController::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)

    QJSEngine::setObjectOwnership(instance(), QQmlEngine::CppOwnership);
    return instance();
}

BackupController *BackupController::s_instance = nullptr;

BackupController *BackupController::instance()
{
    if (!s_instance) {
        s_instance = new BackupController();
    }
    return s_instance;
}

QString BackupController::username() const
{
    return m_username;
}

void BackupController::setUsername(const QString &username)
{
    if (m_username != username) {
        m_username = username;
        Q_EMIT usernameChanged();
    }
}

bool BackupController::restoreWanted() const
{
    return m_restoreWanted;
}

void BackupController::setRestoreWanted(bool restoreWanted)
{
    if (m_restoreWanted != restoreWanted) {
        m_restoreWanted = restoreWanted;
        Q_EMIT restoreWantedChanged();
    }
}

QString BackupController::sourceDir() const
{
    return m_sourceDir;
}

void BackupController::setSourceDir(const QString &sourceDir)
{
    if (m_sourceDir != sourceDir) {
        m_sourceDir = sourceDir;
        Q_EMIT sourceDirChanged();
    }
}

QString BackupController::sourceName() const
{
    return m_sourceName;
}

void BackupController::setSourceName(const QString &sourceName)
{
    if (m_sourceName != sourceName) {
        m_sourceName = sourceName;
        Q_EMIT sourceNameChanged();
    }
}

QString BackupController::sourceRevision() const
{
    return m_sourceRevision;
}

void BackupController::setSourceRevision(const QString &sourceRevision)
{
    if (m_sourceRevision != sourceRevision) {
        m_sourceRevision = sourceRevision;
        Q_EMIT sourceRevisionChanged();
    }
}

KAuth::Action BackupController::restoreAction()
{
    QList<QWindow *> topLevelWindows = QGuiApplication::topLevelWindows();
    QWindow *window = topLevelWindows.isEmpty() ? nullptr : topLevelWindows.first();

    KAuth::Action action(QStringLiteral("org.kde.plasmasetup.restorebackup"));
    action.setParentWindow(window);
    action.setHelperId(QStringLiteral("org.kde.plasmasetup"));
    action.setArguments({
        {QStringLiteral("username"), m_username},
        {QStringLiteral("backupDir"), m_sourceDir},
        {QStringLiteral("backupName"), m_sourceName},
        {QStringLiteral("backupRevision"), m_sourceRevision},
    });

    return action;
}
