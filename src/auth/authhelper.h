// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <KAuth/ActionReply>

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
     * Disables the systemd unit for Plasma Setup.
     *
     * This action is used to disable the systemd unit which runs Plasma Setup
     * once the first user has been created.
     *
     * @param args The arguments passed to the action (not used here).
     * @return An ActionReply indicating success or failure.
     */
    ActionReply disablesystemdunit(const QVariantMap &args);

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
     * Changes the effective user and group ID of the current process back to root.
     *
     * This function is used to resume privileges after temporarily dropping them to those of
     * another user with `becomeUser()`.
     *
     * @param reply Reference to an ActionReply object to set error information if needed.
     * @return True if the operation was successful, false otherwise.
     */
    bool becomeAdminAgain(ActionReply &reply);

    /**
     * Changes the effective user and group ID of the current process to that of the specified user.
     *
     * This function is used to drop privileges to those of a specified user, allowing
     * file operations and other actions to be performed with the permissions of that user.
     *
     * Calling `becomeAdminAgain()` will restore the original privileges.
     *
     * @param userInfo A UserInfo struct containing the UID and GID of the target user.
     * @param reply Reference to an ActionReply object to set error information if needed.
     * @return True if the operation was successful, false otherwise.
     */
    bool becomeUser(const UserInfo &userInfo, ActionReply &reply);

    /**
     * Validates the given username and retrieves information about the user.
     *
     * This function performs important security checks to ensure that the username
     * is valid and that the user exists on the system. It also prevents actions from being performed
     * on invalid users (e.g., system users or non-existent users).
     *
     * @param username The username to look up.
     * @param reply Reference to an ActionReply object to set error information if needed.
     * @return A UserInfo struct containing information about the user, or null if the user is invalid.
     */
    std::optional<UserInfo> getUserInfo(const QString &username, ActionReply &reply);
};
