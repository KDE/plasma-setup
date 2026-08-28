// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>

struct Drive {
    QString udi;
};

class ExternalDrivesModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        DescriptionRole = Qt::UserRole + 1,
        IsMountedRole,
        MountPathRole,
        UDIRole,
    };
    Q_ENUM(Roles)

    explicit ExternalDrivesModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void addDrive(const QString &udi);
    void removeDrive(const QString &udi);
    QList<QString> drives;
};
