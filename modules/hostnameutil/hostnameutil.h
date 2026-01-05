// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "hostname1_interface.h"

#include <QObject>
#include <qqmlintegration.h>

/**
 * Handles the system hostname.
 *
 * This class provides functionality to manage the system hostname,
 * including retrieving and setting the hostname.
 */
class HostnameUtil : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    /**
     * The system's hostname.
     */
    Q_PROPERTY(QString hostname READ hostname WRITE setHostname NOTIFY hostnameChanged)

public:
    /**
     * Default constructor.
     */
    explicit HostnameUtil(QObject *parent = nullptr);

    /**
     * Returns the current hostname.
     */
    QString hostname() const;

    /**
     * Checks if the current hostname is the system default.
     *
     * The hostname is considered the default if it matches the
     * default hostname provided by hostnamed, or if it begins with "localhost".
     *
     * @return True if the hostname is the default, false otherwise.
     */
    Q_INVOKABLE bool hostnameIsDefault() const;

    /**
     * Checks if the provided hostname is valid.
     *
     * @param hostname The hostname to validate.
     * @return True if the hostname is valid, false otherwise.
     */
    Q_INVOKABLE bool isHostnameValid(const QString &hostname) const;

    /**
     * Returns a user-facing validation message for the provided hostname.
     *
     * @param hostname The hostname to validate.
     * @return A validation message if invalid, or an empty string if valid.
     */
    Q_INVOKABLE QString hostnameValidationMessage(const QString &hostname) const;

    /**
     * Sets the system hostname.
     *
     * If the provided hostname is invalid, the request is silently ignored
     * and the current hostname is left unchanged. Callers should use
     * isHostnameValid() or hostnameValidationMessage() before invoking this
     * function to ensure the hostname will be accepted.
     *
     * @param hostname The new hostname to set.
     */
    void setHostname(const QString &hostname);

Q_SIGNALS:
    /**
     * Emitted when the hostname changes.
     */
    void hostnameChanged();

private:
    /**
     * Reads the current hostname from the system.
     */
    void loadHostname();

    /**
     * Attempts to read the hostname via D-Bus.
     *
     * @param propertyName The D-Bus property name to read, e.g., "StaticHostname" or "Hostname".
     * @return The retrieved value, or an empty string on failure.
     */
    QString readHostnameViaDBus(const QString &propertyName);

    /**
     * Applies the requested hostname to the system.
     *
     * @param hostname The hostname to set.
     */
    void setHostnameOnSystem(const QString &hostname);

    /**
     * Sets the static hostname via D-Bus.
     *
     * @param hostname The static hostname to set.
     */
    void setStaticHostname(const QString &hostname);

    /**
     * Sets the transient hostname via D-Bus.
     *
     * @param hostname The transient hostname to set.
     */
    void setTransientHostname(const QString &hostname);

    /**
     * D-Bus interface for org.freedesktop.hostname1.
     */
    OrgFreedesktopHostname1Interface *const m_dbusInterface;

    /**
     * Cached hostname value.
     */
    QString m_hostname;
};
