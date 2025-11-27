// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <KAuth/ActionReply>

#include <QTemporaryFile>
#include <QVariant>

using namespace KAuth;

/**
 * Struct to hold user information.
 */
struct UserInfo {
    /** The username of the user. */
    QString username;

    /** The home directory path of the user. */
    QString homePath;

    /** The user ID (UID) of the user. */
    int uid;

    /** The group ID (GID) of the user. */
    int gid;
};

/**
 * A KAuth helper class for performing privileged actions related to Plasma Setup.
 */
class PlasmaSetupAuthHelper : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    /**
     * Creates a new user account using the system's useradd utility.
     *
     * @param args The arguments passed to the action, which should include:
     * - String: "username": The username for the new user.
     * - String: "fullName": The full name for the new user.
     * - String: "password": The password for the new user.
     * @return An ActionReply indicating success or failure.
     */
    ActionReply createuser(const QVariantMap &args);

    /**
     * Creates the `/etc/plasma-setup-done` flag file when setup completes.
     *
     * @param args The arguments passed to the action (not used here).
     * @return An ActionReply indicating success or failure.
     */
    ActionReply createflagfile(const QVariantMap &args);

    /**
     * Creates an autostart hook for the newly created user to remove the autologin configuration.
     *
     * This function creates a desktop entry in the new user's autostart directory that will
     * execute a script to remove the autologin configuration when the session transitions to the new user.
     *
     * @param args The arguments passed to the action, which should include:
     * - String: "username": The username of the newly created user.
     * @return An ActionReply indicating success or failure.
     */
    ActionReply createnewuserautostarthook(const QVariantMap &args);

    /**
     * Removes the configuration file that enables autologin for Plasma Setup.
     *
     * @param args The arguments passed to the action (not used here).
     * @return An ActionReply indicating success or failure.
     */
    ActionReply removeautologin(const QVariantMap &args);

    /**
     * Sets the global theme for the newly created user.
     *
     * @param args The arguments passed to the action, which should include:
     * - String: "username": The username of the newly created user.
     * @return An ActionReply indicating success or failure.
     */
    ActionReply setnewuserglobaltheme(const QVariantMap &args);

    /**
     * Sets the display scaling for the newly created user.
     *
     * Copies a few configurations files containing the chosen display scaling options
     * from the plasma-setup user to the new user, namely:
     * - ~/.config/kwinrc
     * - ~/.config/kwinoutputconfig.json
     *
     * @param args The arguments passed to the action, which should include:
     * - String: "username": The username of the newly created user.
     * @return An ActionReply indicating success or failure.
     */
    ActionReply setnewuserdisplayscaling(const QVariantMap &args);

    /**
     * Sets the configuration for the newly created user to login automatically.
     *
     * This sets an autologin config with `Relogin=true`, so we can switch from the
     * plasma-setup user to the new user when setup is complete.
     *
     * @param args The arguments passed to the action, which should include:
     * - String: "username": The username of the newly created user.
     * @return An ActionReply indicating success or failure.
     */
    ActionReply setnewusertempautologin(const QVariantMap &args);

private:
    /**
     * Adds a user to the provided supplementary groups using usermod.
     */
    ActionReply addUserToExtraGroups(const QString &username, const QVariant &extraGroupsVariant);

    /**
     * Copies a source file to a temporary file with permissions that allow the new user to read it.
     *
     * Creates a temporary file, copies the contents from the source file, and sets permissions
     * so that the specified user can access it. The temporary file is automatically cleaned up
     * when the returned QTemporaryFile object is destroyed.
     *
     * @param sourceFilePath The path to the source file to copy.
     * @return A unique pointer to the QTemporaryFile on success.
     * @throws std::runtime_error if any operation fails.
     */
    std::unique_ptr<QTemporaryFile> copyToTempFile(const QString &sourceFilePath);

    /**
     * Validates the given username and retrieves information about the user.
     *
     * This function performs important security checks to ensure that the username
     * is valid and that the user exists on the system. It also prevents actions from being performed
     * on invalid users (e.g., system users or non-existent users).
     *
     * @param username The username to look up.
     * @return A UserInfo struct containing information about the user.
     * @throws std::runtime_error if the username is invalid or the user does not exist.
     */
    UserInfo getUserInfo(const QString &username);

    /**
     * Helper function to create an error ActionReply with the given description.
     *
     * @param errorDescription The description of the error.
     * @return An ActionReply representing the error.
     */
    ActionReply makeErrorReply(const QString &errorDescription);
};
