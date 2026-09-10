// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <KAuth/Action>

#include <QObject>
#include <QQmlEngine>
#include <qqmlintegration.h>

class BackupController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool restoreWanted READ restoreWanted WRITE setRestoreWanted NOTIFY restoreWantedChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString sourceDir READ sourceDir WRITE setSourceDir NOTIFY sourceDirChanged)
    Q_PROPERTY(QString sourceName READ sourceName WRITE setSourceName NOTIFY sourceNameChanged)
    Q_PROPERTY(QString sourceRevision READ sourceRevision WRITE setSourceRevision NOTIFY sourceRevisionChanged)

public:
    ~BackupController() override = default;

    /**
     * Returns the singleton instance of BackupController.
     *
     * This is intended to be used automatically by the QML engine.
     */
    static BackupController *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

    /**
     * Returns the singleton instance of BackupController.
     *
     * If it doesn't exist, it will create one.
     */
    static BackupController *instance();

    bool restoreWanted() const;
    void setRestoreWanted(bool restoreWanted);

    QString username() const;
    void setUsername(const QString &username);

    QString sourceDir() const;
    void setSourceDir(const QString &sourceDir);

    QString sourceName() const;
    void setSourceName(const QString &sourceName);

    QString sourceRevision() const;
    void setSourceRevision(const QString &sourceRevision);

    KAuth::Action restoreAction();

Q_SIGNALS:
    void restoreWantedChanged();
    void usernameChanged();
    void sourceDirChanged();
    void sourceNameChanged();
    void sourceRevisionChanged();

private:
    /**
     * Private constructor to enforce singleton pattern.
     *
     * Use `instance()` to get the singleton instance.
     */
    explicit BackupController(QObject *parent = nullptr);

    /**
     * Static instance of BackupController.
     */
    static BackupController *s_instance;

    bool m_restoreWanted = false;
    QString m_username;
    QString m_sourceDir;
    QString m_sourceName;
    QString m_sourceRevision;
    QUrl m_sourceUrl;
};
