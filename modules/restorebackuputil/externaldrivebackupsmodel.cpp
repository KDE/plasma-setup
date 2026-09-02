// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "externaldrivebackupsmodel.h"

#include <QDirIterator>
#include <QString>
#include <QtConcurrentRun>

#include <KIO/ListJob>

#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>

using namespace Qt::StringLiterals;

bool isBackup(const QString &dirPath)
{
    return QFileInfo::exists(QDir(dirPath).filePath("bupindex"_L1));
}

void scanDirectory(QPromise<QString> &promise, const QString &rootPath)
{
    if (isBackup(rootPath)) {
        promise.addResult(rootPath);
    }

    QDirIterator it(rootPath, QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        if (promise.isCanceled()) {
            return;
        }

        QString currentDir = it.next();
        if (isBackup(currentDir)) {
            promise.addResult(currentDir);
        }
    }
}

ExternalDriveBackupsModel::ExternalDriveBackupsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    const auto deviceList = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive);
    for (const auto &device : deviceList) {
        addDrive(device.udi());
    }

    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &ExternalDriveBackupsModel::addDrive);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, &ExternalDriveBackupsModel::removeDrive);
}

ExternalDriveBackupsModel::~ExternalDriveBackupsModel()
{
    // TODO cancel futures here
}

void ExternalDriveBackupsModel::addDrive(const QString &udi)
{
    Solid::Device device(udi);
    QList<Solid::Device> volumes;

    if (device.is<Solid::StorageDrive>()) {
        const auto *drive = device.as<Solid::StorageDrive>();
        if (!drive->isHotpluggable() && !drive->isRemovable()) {
            return;
        }
        volumes << Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume, udi);
        if (device.is<Solid::StorageVolume>()) {
            volumes << device;
        }
    } else if (device.is<Solid::StorageVolume>()) {
        const Solid::Device parentDevice = device.parent();
        if (!parentDevice.is<Solid::StorageDrive>()) {
            return;
        }
        const auto *drive = parentDevice.as<Solid::StorageDrive>();
        if (!drive->isHotpluggable() && !drive->isRemovable()) {
            return;
        }
        volumes << device;
    }

    QStringList validVolumes;
    for (const auto &volumeDevice : volumes) {
        if (drives.contains(volumeDevice.udi())) {
            continue;
        }
        const auto *volume = volumeDevice.as<Solid::StorageVolume>();
        if (volume->isIgnored()) {
            continue;
        }
        if (volume->usage() == Solid::StorageVolume::FileSystem || volume->usage() == Solid::StorageVolume::Encrypted) {
            validVolumes << volumeDevice.udi();
        }
    }

    beginInsertRows({}, drives.size(), drives.size() + validVolumes.size() - 1);
    drives << validVolumes;
    endInsertRows();
}

void ExternalDriveBackupsModel::removeDrive(const QString &udi)
{
    const auto idx = drives.indexOf(udi);
    if (idx == -1) {
        return;
    }

    // beginRemoveRows(createIndex(idx, 0, quintptr(0)), 0, driveBackups.value(udi, {}).size() - 1);
    // driveBackups.remove(udi);
    // endRemoveRows();

    beginRemoveRows({}, idx, idx);
    drives.removeAt(idx);
    endRemoveRows();

    driveBackups.remove(udi);
    auto *watcher = driveSearchWatchers.value(udi, nullptr);
    if (watcher) {
        watcher->cancel();
        watcher->deleteLater();
    }
    driveSearchWatchers.remove(udi);
}

int ExternalDriveBackupsModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return drives.size();
    }

    if (parent.internalId() == 0) {
        const auto parentIdx = parent.row();
        if (parentIdx < drives.size()) {
            const auto &udi = drives.at(parentIdx);
            return driveBackups.value(udi, {}).size();
        }
    }

    return 0;
}

int ExternalDriveBackupsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QModelIndex ExternalDriveBackupsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (!parent.isValid()) {
        return createIndex(row, column, quintptr(0));
    }

    if (parent.internalId() == 0) {
        return createIndex(row, column, static_cast<quintptr>(parent.row() + 1));
    }

    return QModelIndex();
}

QModelIndex ExternalDriveBackupsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    if (index.internalId() == 0) {
        return QModelIndex();
    }

    int parentRow = static_cast<int>(index.internalId()) - 1;
    if (parentRow < 0 || parentRow >= drives.size()) {
        return QModelIndex();
    }

    return createIndex(parentRow, 0, quintptr(0));
}

bool ExternalDriveBackupsModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.internalId() == 0 && parent.row() < drives.size()) {
        const QString &udi = drives.at(parent.row());
        const auto *watcher = driveSearchWatchers.value(udi, nullptr);
        if (watcher && watcher->isFinished() && driveBackups.value(udi, {}).size() == 0) {
            return false;
        }
        return true;
    }

    return false;
}

bool ExternalDriveBackupsModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.internalId() == 0 && parent.row() < drives.size()) {
        const QString &udi = drives.at(parent.row());
        return !driveSearchWatchers.contains(udi);
    }

    return false;
}

void ExternalDriveBackupsModel::fetchMore(const QModelIndex &parent)
{
    if (!parent.isValid() || parent.internalId() != 0 || parent.row() >= drives.size()) {
        return;
    }

    const QString &udi = drives.at(parent.row());
    Solid::Device device(udi);
    auto *access = device.as<Solid::StorageAccess>();
    if (access->isAccessible()) {
        driveMounted(udi, access->filePath());
    } else {
        connect(access, &Solid::StorageAccess::setupDone, this, [this, udi, access](Solid::ErrorType err) {
            if (err != Solid::ErrorType::NoError) {
                return;
            }
            driveMounted(udi, access->filePath());
        });
        access->setup();
    }
}

void ExternalDriveBackupsModel::driveMounted(const QString &udi, const QString &mountPath)
{
    auto watcher = new QFutureWatcher<QString>();
    connect(watcher, &QFutureWatcher<QString>::resultReadyAt, this, [this, watcher, udi](int index) {
        QString res = watcher->resultAt(index);
        listBupRepo(udi, res);
    });

    watcher->setFuture(QtConcurrent::run(&scanDirectory, mountPath));
    driveSearchWatchers.insert(udi, std::move(watcher));
}

QVariant ExternalDriveBackupsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.internalId() == 0) {
        if (index.row() >= drives.size()) {
            return {};
        }

        const QString &udi = drives.at(index.row());
        Solid::Device device(udi);
        const auto *access = device.as<Solid::StorageAccess>();

        switch (role) {
        case Qt::DisplayRole:
        case Roles::DriveDescriptionRole:
            return device.displayName();
        case Roles::DriveUDIRole:
            return udi;
        case Roles::DriveIsMountedRole:
            return access->isAccessible();
        case Roles::DriveMountPathRole:
            return access->filePath();
        default:
            return {};
        }
    }

    int parentRow = static_cast<int>(index.internalId()) - 1;
    if (parentRow < 0 || parentRow >= drives.size()) {
        return {};
    }

    const QString &driveUdi = drives.at(parentRow);
    const auto &backups = driveBackups.value(driveUdi, {});

    if (index.row() >= backups.size()) {
        return {};
    }

    const auto &backup = backups.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        return backup.date.toString();
    case Roles::BackupUsernameRole:
        return backup.username;
    case Roles::BackupDateRole:
        return backup.date;
    case Roles::BackupFSPathRole:
        return backup.fsPath;
    case Roles::BackupRelativeFSPathRole:
        Solid::Device device(driveUdi);
        const auto *access = device.as<Solid::StorageAccess>();
        return QDir(access->filePath()).relativeFilePath(backup.fsPath);
    }

    return {};
}

QHash<int, QByteArray> ExternalDriveBackupsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"_ba},
        {Roles::DriveDescriptionRole, "description"_ba},
        {Roles::DriveUDIRole, "udi"_ba},
        {Roles::DriveIsMountedRole, "isMounted"_ba},
        {Roles::DriveMountPathRole, "mountPath"_ba},
        {Roles::BackupUsernameRole, "username"_ba},
        {Roles::BackupDateRole, "date"_ba},
        {Roles::BackupFSPathRole, "fsPath"_ba},
        {Roles::BackupRelativeFSPathRole, "relativeFsPath"_ba},
    };
}

void ExternalDriveBackupsModel::listBupRepo(const QString &driveUdi, const QString &repoPath)
{
    QUrl bupUrl;
    bupUrl.setScheme("bup"_L1);
    bupUrl.setPath(repoPath);
    KIO::ListJob *job = KIO::listDir(bupUrl, KIO::HideProgressInfo, KIO::ListJob::ListFlag::ExcludeDotAndDotDot);
    QObject::connect(job, &KIO::ListJob::entries, this, [this, repoPath, driveUdi](KIO::Job *, const KIO::UDSEntryList &entries) {
        for (const KIO::UDSEntry &entry : entries) {
            listBupRepoBackups(driveUdi, repoPath, entry.stringValue(KIO::UDSEntry::UDS_NAME));
        }
    });
}

void ExternalDriveBackupsModel::listBupRepoBackups(const QString &driveUdi, const QString &repoPath, const QString &name)
{
    QUrl bupUrl;
    bupUrl.setScheme("bup"_L1);
    bupUrl.setPath(QDir::cleanPath(repoPath + "/"_L1 + name));
    KIO::ListJob *job = KIO::listDir(bupUrl, KIO::HideProgressInfo, KIO::ListJob::ListFlag::ExcludeDotAndDotDot);
    QObject::connect(job, &KIO::ListJob::entries, this, [this, repoPath, name, driveUdi](KIO::Job *, const KIO::UDSEntryList &entries) {
        for (const KIO::UDSEntry &entry : entries) {
            const QString datestamp = entry.stringValue(KIO::UDSEntry::UDS_NAME);
            const QDateTime date = QDateTime::fromSecsSinceEpoch(entry.numberValue(KIO::UDSEntry::UDS_MODIFICATION_TIME));
            listBupRepoDate(driveUdi, repoPath, name, datestamp, date);
        }
    });
}

void ExternalDriveBackupsModel::listBupRepoDate(const QString &driveUdi, const QString &repoPath, const QString &name, const QString &datestamp, QDateTime date)
{
    QUrl bupUrl;
    bupUrl.setScheme("bup"_L1);
    bupUrl.setPath(QDir::cleanPath(repoPath + "/%1/%2/home"_L1.arg(name, datestamp)));
    KIO::ListJob *job = KIO::listDir(bupUrl, KIO::HideProgressInfo, KIO::ListJob::ListFlag::ExcludeDotAndDotDot);
    QObject::connect(job, &KIO::ListJob::entries, this, [this, repoPath, name, datestamp, date, driveUdi](KIO::Job *, const KIO::UDSEntryList &entries) {
        for (const KIO::UDSEntry &entry : entries) {
            const QString username = entry.stringValue(KIO::UDSEntry::UDS_NAME);
            addHomeBackup(driveUdi, repoPath, name, datestamp, date, username);
        }
    });
}

void ExternalDriveBackupsModel::addHomeBackup(const QString &driveUdi,
                                              const QString &repoPath,
                                              const QString &name,
                                              const QString &datestamp,
                                              QDateTime date,
                                              const QString &username)
{
    struct HomeBackup backupInfo;
    backupInfo.fsPath = repoPath;
    backupInfo.bupName = name;
    backupInfo.datestamp = datestamp;
    backupInfo.date = date;
    backupInfo.username = username;

    int parentRow = drives.indexOf(driveUdi);
    if (parentRow != -1) {
        if (!driveBackups.contains(driveUdi)) {
            driveBackups.insert(driveUdi, {});
        }
        auto &backups = driveBackups[driveUdi];
        // the backups are listed as KIO::listDir signals, so they may be in arbitrary order
        // so we insert them in sorted order here
        auto it = std::lower_bound(backups.begin(), backups.end(), backupInfo.date.toMSecsSinceEpoch(), [](const struct HomeBackup &backup, qint64 newVal) {
            return backup.date.toMSecsSinceEpoch() < newVal;
        });
        auto idx = std::distance(backups.begin(), it);
        if (backups.size() > idx && backups.at(idx) == backupInfo) {
            return;
        }
        beginInsertRows(createIndex(parentRow, 0, quintptr(0)), idx, idx);
        backups.insert(it, backupInfo);
        endInsertRows();
    }
}
