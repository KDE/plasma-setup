// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <qqmlintegration.h>

class AccountController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString fullName READ fullName WRITE setFullName NOTIFY fullNameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)

    /**
     * Indicates whether at least one regular (non-system) user already exists on the system.
     */
    Q_PROPERTY(bool hasExistingUsers READ hasExistingUsers NOTIFY hasExistingUsersChanged)

public:
    ~AccountController() override;

    /**
     * Returns the singleton instance of AccountController.
     *
     * This is intended to be used automatically by the QML engine.
     */
    static AccountController *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

    /**
     * Returns the singleton instance of AccountController.
     *
     * If it doesn't exist, it will create one.
     */
    static AccountController *instance();

    QString username() const;
    void setUsername(const QString &username);

    QString fullName() const;
    void setFullName(const QString &fullName);

    QString password() const;
    void setPassword(const QString &password);

    /**
     * Creates a new user account with the current username, full name, and password.
     *
     * @return true if the user was created successfully, false otherwise.
     */
    Q_INVOKABLE bool createUser();

    /**
     * Validates the provided username according to system rules.
     *
     * @param username The username to validate.
     * @return true if the username is valid, false otherwise.
     */
    Q_INVOKABLE bool isUsernameValid(const QString &username) const;

    /**
     * Provides a user-friendly validation message for the given username.
     *
     * @param username The username to validate.
     * @return A localized validation message if invalid, or an empty string if valid.
     */
    Q_INVOKABLE QString usernameValidationMessage(const QString &username) const;

    /**
     * Returns whether the controller detected any pre-existing regular users.
     */
    bool hasExistingUsers() const;

Q_SIGNALS:
    void usernameChanged();
    void fullNameChanged();
    void passwordChanged();
    void hasExistingUsersChanged();

private:
    /**
     * Private constructor to enforce singleton pattern.
     *
     * Use `instance()` to get the singleton instance.
     */
    explicit AccountController(QObject *parent = nullptr);

    /**
     * Static instance of AccountController.
     */
    static AccountController *s_instance;

    QString m_username;
    QString m_fullName;
    QString m_password;

    /**
     * Cached result of the existing-user detection. Defaults to false so the account page shows.
     */
    bool m_hasExistingUsers = false;

    /**
     * Runs the detection routine once during construction and emits when the value changes.
     */
    void initializeExistingUserFlag();

    /**
     * Checks if overriding account creation behavior via environment variable is requested.
     *
     * Reads the PLASMA_SETUP_USER_CREATION_OVERRIDE environment variable, which can be set to
     * "enable" to force account creation to be enabled regardless of existing users. Useful for testing.
     *
     * @return true if an override was applied, false otherwise.
     */
    bool isAccountCreationOverrideEnabled();

    /**
     * Parses /etc/login.defs for UID_MIN, falling back to a sensible default.
     */
    int regularUserUidThreshold() const;

    /**
     * Enumerates passwd entries to determine if a regular user already exists.
     */
    bool detectExistingUsers(int minUid) const;
};
