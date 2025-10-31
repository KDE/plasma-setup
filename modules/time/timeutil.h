// SPDX-FileCopyrightText: 2023 by Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>

#include <qqmlregistration.h>

class OrgFreedesktopTimedate1Interface;

class TimeUtil : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString currentTimeZone READ currentTimeZone WRITE setCurrentTimeZone NOTIFY currentTimeZoneChanged)

public:
    explicit TimeUtil(QObject *parent = nullptr);

    QString currentTimeZone() const;
    void setCurrentTimeZone(const QString &timeZone);

Q_SIGNALS:
    void currentTimeZoneChanged();

private:
    OrgFreedesktopTimedate1Interface *const m_dbusInterface;
};
