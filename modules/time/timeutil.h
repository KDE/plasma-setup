// SPDX-FileCopyrightText: 2023 by Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>

#include <qqmlregistration.h>

class OrgFreedesktopTimedate1Interface;

/**
 * Utility class for managing system timezone settings.
 *
 * TimeUtil provides a QML-accessible interface for reading and modifying
 * the system timezone using the freedesktop.org timedate1 D-Bus interface.
 */
class TimeUtil : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    /**
     * @property currentTimeZone
     *
     * The current system timezone identifier.
     *
     * This property holds the IANA timezone identifier (e.g., "America/New_York")
     * for the system's current timezone.
     */
    Q_PROPERTY(QString currentTimeZone READ currentTimeZone WRITE setCurrentTimeZone NOTIFY currentTimeZoneChanged)

public:
    explicit TimeUtil(QObject *parent = nullptr);

    /**
     * Gets the current system timezone.
     *
     * @return The IANA timezone identifier as a QString.
     */
    QString currentTimeZone() const;

    /**
     * Sets the system timezone.
     *
     * @param timeZone The IANA timezone identifier to set (e.g., "Europe/London").
     */
    void setCurrentTimeZone(const QString &timeZone);

Q_SIGNALS:
    /**
     * Emitted when the system timezone has been successfully changed.
     */
    void currentTimeZoneChanged();

private:
    /**
     * D-Bus interface for communicating with the freedesktop.org timedate1 service.
     *
     * This interface is used to query and modify system timezone settings
     * through the org.freedesktop.timedate1 D-Bus service.
     */
    OrgFreedesktopTimedate1Interface *const m_dbusInterface;
};
