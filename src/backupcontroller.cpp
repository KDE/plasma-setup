// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "backupcontroller.h"

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

QUrl BackupController::sourceUrl() const
{
    return m_sourceUrl;
}

void BackupController::setSourceUrl(const QUrl &sourceUrl)
{
    if (m_sourceUrl != sourceUrl) {
        m_sourceUrl = sourceUrl;
        Q_EMIT sourceUrlChanged();
    }
}
