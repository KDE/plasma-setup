// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <qqmlintegration.h>

class AccountController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString fullName READ fullName WRITE setFullName NOTIFY fullNameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)

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

Q_SIGNALS:
    void usernameChanged();
    void fullNameChanged();
    void passwordChanged();

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
     * Retrieves from the configuration the list of groups for newly created users.
     *
     * If no groups are specified by the configuration, falls back to the `wheel`
     * group which most distributions use for admin/sudo access.
     *
     * @return A list of group names.
     */
    QStringList userGroupsFromConfig() const;
};
