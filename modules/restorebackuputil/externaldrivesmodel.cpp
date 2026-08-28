// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "externaldrivesmodel.h"

#include <QString>

#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>

using namespace Qt::StringLiterals;

ExternalDrivesModel::ExternalDrivesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const auto deviceList = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive);
    for (const auto &device : deviceList) {
        addDrive(device.udi());
    }

    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &ExternalDrivesModel::addDrive);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, &ExternalDrivesModel::removeDrive);
}

void ExternalDrivesModel::addDrive(const QString &udi)
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

void ExternalDrivesModel::removeDrive(const QString &udi)
{
    const auto idx = drives.indexOf(udi);
    if (idx == -1) {
        return;
    }

    beginRemoveRows({}, idx, idx);
    drives.removeAt(idx);
    endRemoveRows();
}

int ExternalDrivesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return drives.size();
}

QVariant ExternalDrivesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= drives.size()) {
        return {};
    }

    const QString &udi = drives.at(index.row());
    Solid::Device device(udi);
    const auto *access = device.as<Solid::StorageAccess>();

    switch (role) {
    case Qt::DisplayRole:
    case Roles::DescriptionRole:
        return device.displayName();
    case Roles::UDIRole:
        return udi;
    case Roles::IsMountedRole:
        return access->isAccessible();
    case Roles::MountPathRole:
        return access->filePath();
    }

    return {};
}

QHash<int, QByteArray> ExternalDrivesModel::roleNames() const
{
    return {
        {Roles::DescriptionRole, "description"_ba},
        {Roles::UDIRole, "udi"_ba},
        {Roles::IsMountedRole, "isMounted"_ba},
        {Roles::MountPathRole, "mountPath"_ba},
    };
}
