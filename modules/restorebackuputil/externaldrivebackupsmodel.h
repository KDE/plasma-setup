// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QFutureWatcher>
#include <QObject>
#include <QQmlEngine>

struct HomeBackup {
    QString fsPath;
    QString bupName;
    // why both a datestamp and date?
    // so that we don't need to rely on the exact date format used by the bup:// worker.
    // we store the datestamp we get from it as-is, and get an actual QDateTime separately using the UDS entry modification time
    QString datestamp;
    QDateTime date;
    QString username;

    bool operator==(const HomeBackup &) const = default;
};

class ExternalDriveBackupsModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        DriveDescriptionRole = Qt::UserRole + 1,
        DriveIsMountedRole,
        DriveMountPathRole,
        DriveUDIRole,
        DriveIsScanningRole,

        BackupUsernameRole,
        BackupDateRole,
        BackupFSPathRole,
        BackupRelativeFSPathRole,
    };
    Q_ENUM(Roles)

    explicit ExternalDriveBackupsModel(QObject *parent = nullptr);
    ~ExternalDriveBackupsModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    bool hasChildren(const QModelIndex &parent) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void addDrive(const QString &udi);
    void removeDrive(const QString &udi);
    QList<QString> drives;
    QHash<QString, QList<HomeBackup>> driveBackups;
    QHash<QString, QPointer<QFutureWatcher<QString>>> driveSearchWatchers;
    void exploreFs(const QString &path);

    /* a bup repo looks like:
     * repo_root/
     *   backup_name/ (like "kup")
     *     tag/ (in the bup:// worker, a date is given instead of a tag name)
     *       home/
     *         username/
     */
    void listBupRepo(const QString &driveUdi, const QString &repoPath);
    void listBupRepoBackups(const QString &driveUdi, const QString &repoPath, const QString &name);
    void listBupRepoDate(const QString &driveUdi, const QString &repoPath, const QString &name, const QString &datestamp, QDateTime date);
    void
    addHomeBackup(const QString &driveUdi, const QString &repoPath, const QString &name, const QString &datestamp, QDateTime date, const QString &username);

    void driveMounted(const QString &udi, const QString &mountPath);
};
