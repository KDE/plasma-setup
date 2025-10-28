// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <QProcess>
#include <QTemporaryDir>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingReply>

#include <KAuth/HelperSupport>
#include <KConfigGroup>
#include <KSharedConfig>

#include "authhelper.h"
#include "config-plasma-setup.h"

#include <pwd.h>
#include <sys/stat.h>

const QString SDDM_AUTOLOGIN_CONFIG_PATH = QStringLiteral("/etc/sddm.conf.d/99-plasma-setup.conf");

/**
 * Minimum UID for regular (non-system) users.
 */
constexpr int MIN_REGULAR_USER_UID = 1000;

class PrivilegeGuard
{
public:
    PrivilegeGuard(const UserInfo &userInfo)
    {
        // Drop privileges to specified user
        if (setegid(userInfo.gid) != 0 || seteuid(userInfo.uid) != 0) {
            throw std::runtime_error("Failed to drop privileges to user " + userInfo.username.toStdString() + ": error " + std::to_string(errno));
        }
    }

    ~PrivilegeGuard()
    {
        // Automatically restore admin privileges when going out of scope
        if (setegid(0) != 0 || seteuid(0) != 0) {
            qWarning() << "Failed to restore admin privileges: error" << errno;
            // This path is unlikely, however if we failed to restore privileges, there's not much we can do.
            // Just terminate this helper process to avoid any further issues.
            std::terminate();
        }
    }

private:
    // Prevent copying
    PrivilegeGuard(const PrivilegeGuard &) = delete;
    PrivilegeGuard &operator=(const PrivilegeGuard &) = delete;
};

ActionReply PlasmaSetupAuthHelper::createnewuserautostarthook(const QVariantMap &args)
{
    ActionReply reply;

    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        return makeErrorReply(QStringLiteral("Username argument is missing or invalid."));
    }

    QString username = args[QStringLiteral("username")].toString();
    std::optional<UserInfo> userInfoMaybe = getUserInfo(username, reply);
    if (!userInfoMaybe.has_value()) {
        // getUserInfo already set the error in reply
        return reply;
    }
    UserInfo userInfo = userInfoMaybe.value();
    QString homePath = userInfo.homePath;

    QString autostartDirPath = QDir::cleanPath(homePath + QStringLiteral("/.config/autostart"));
    QDir autostartDir(autostartDirPath);

    // Ensure the autostart directory exists
    if (!autostartDir.exists() && !autostartDir.mkpath(QStringLiteral("."))) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to create autostart directory: ") + autostartDirPath);
        return reply;
    }

    PrivilegeGuard guard(userInfo);

    // Create the desktop entry file
    QString desktopFilePath = autostartDir.filePath(QStringLiteral("remove-autologin.desktop"));
    QFile desktopFile(desktopFilePath);
    if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to open file for writing: ") + desktopFilePath + QStringLiteral(" error:")
                                  + desktopFile.errorString());
        return reply;
    }

    QString plasmaSetupExecutablePath = QStringLiteral(PLASMA_SETUP_LIBEXECDIR) + QStringLiteral("/plasma-setup");
    if (plasmaSetupExecutablePath.isEmpty()) {
        desktopFile.close();
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to find the Plasma Setup executable path."));
        return reply;
    }

    QTextStream stream(&desktopFile);
    stream << "[Desktop Entry]\n";
    stream << "Type=Application\n";
    stream << "Name=Remove Plasma Setup Autologin\n";
    stream << "Exec=sh -c \"" << plasmaSetupExecutablePath << " --remove-autologin && rm --force '" << desktopFilePath << "'\"\n";
    stream << "X-KDE-StartupNotify=false\n";
    stream << "NoDisplay=true\n";
    desktopFile.close();

    reply = ActionReply::SuccessReply();
    reply.setData({{QStringLiteral("autostartFilePath"), desktopFilePath}});
    return reply;
}

ActionReply PlasmaSetupAuthHelper::disablesystemdunit(const QVariantMap &args)
{
    Q_UNUSED(args);

    ActionReply actionReply;

    QDBusInterface systemdInterface(QStringLiteral("org.freedesktop.systemd1"),
                                    QStringLiteral("/org/freedesktop/systemd1"),
                                    QStringLiteral("org.freedesktop.systemd1.Manager"),
                                    QDBusConnection::systemBus());

    QStringList unitFiles = {QStringLiteral("plasma-setup.service")};
    bool runtime = false; // Disable permanently, not just for runtime

    QDBusPendingReply<> dbusReply = systemdInterface.call(QStringLiteral("DisableUnitFiles"), unitFiles, runtime);
    dbusReply.waitForFinished();

    if (dbusReply.isError()) {
        actionReply = ActionReply::HelperErrorReply();
        actionReply.setErrorDescription(QStringLiteral("Unable to disable systemd unit: ") + dbusReply.error().message());
        return actionReply;
    }

    return ActionReply::SuccessReply();
}

ActionReply PlasmaSetupAuthHelper::removeautologin(const QVariantMap &args)
{
    Q_UNUSED(args);

    ActionReply reply;
    QFileInfo fileInfo(SDDM_AUTOLOGIN_CONFIG_PATH);

    if (!fileInfo.exists()) {
        return ActionReply::SuccessReply();
    }

    QFile file(fileInfo.filePath());
    if (!file.remove()) {
        reply = ActionReply::HelperErrorReply();
        QString errorDetails = file.errorString();
        reply.setErrorDescription(QStringLiteral("Unable to remove file ") + fileInfo.filePath() + QStringLiteral(": ") + errorDetails);
        return reply;
    }

    return ActionReply::SuccessReply();
}

ActionReply PlasmaSetupAuthHelper::setnewuserglobaltheme(const QVariantMap &args)
{
    ActionReply reply;

    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Username argument is missing or invalid."));
        return reply;
    }

    QString username = args[QStringLiteral("username")].toString();
    auto userInfoMaybe = getUserInfo(username, reply);
    if (!userInfoMaybe.has_value()) {
        // getUserInfo already set the error in reply
        return reply;
    }
    UserInfo userInfo = userInfoMaybe.value();
    QString homePath = userInfo.homePath;

    // Create a temporary directory accessible to the new user
    QString tempDirPath;
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to create temporary directory."));
        return reply;
    }
    tempDirPath = tempDir.path();

    // Copy the file to the temp directory while we still have privileges
    QString sourceFilePath = QStringLiteral("/run/plasma-setup/.config/kdeglobals");
    QString tempFilePath = tempDirPath + QStringLiteral("/kdeglobals");
    QFile sourceFile(sourceFilePath);
    if (!sourceFile.copy(tempFilePath)) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to copy file to temp directory: ") + sourceFilePath + QStringLiteral(" to ") + tempFilePath
                                  + QStringLiteral(" -- Error message: ") + sourceFile.errorString());
        return reply;
    }

    // Set permissions on the temp directory and file so the new user can access them
    if (chmod(tempDirPath.toLocal8Bit().constData(), 0755) != 0) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to set directory permissions: error code ") + QString::number(errno));
        return reply;
    }

    // Set file permissions to be readable by everyone
    if (chmod(tempFilePath.toLocal8Bit().constData(), 0644) != 0) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to set file permissions: error code ") + QString::number(errno));
        return reply;
    }

    PrivilegeGuard guard(userInfo);

    // Ensure the .config directory exists in the new user's home
    //
    // TODO: Make creating the .config directory (or just "createNewUserDirs" or whatever) a separate
    // action that is called explicitly before all others, since it will definitely be needed for
    // any other configuration actions. This will also avoid redundant error handling code in each
    // action.
    QString configDirPath = QDir::cleanPath(homePath + QStringLiteral("/.config"));
    QDir configDir(configDirPath);

    if (!configDir.exists() && !configDir.mkpath(QStringLiteral("."))) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to create .config directory: ") + configDirPath);
        return reply;
    }

    // Set the global theme for the new user
    QString destFilePath = QDir::cleanPath(configDirPath + QStringLiteral("/kdeglobals"));

    QFile tempFile(tempFilePath);
    if (!tempFile.copy(destFilePath)) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to copy file to destination: ") + tempFilePath + QStringLiteral(" to ") + destFilePath
                                  + QStringLiteral(" -- Error message: ") + tempFile.errorString());
        return reply;
    }

    return ActionReply::SuccessReply();
}

ActionReply PlasmaSetupAuthHelper::setnewuserdisplayscaling(const QVariantMap &args)
{
    ActionReply reply;

    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Username argument is missing or invalid."));
        return reply;
    }

    QString username = args[QStringLiteral("username")].toString();
    auto userInfoMaybe = getUserInfo(username, reply);
    if (!userInfoMaybe.has_value()) {
        // getUserInfo already set the error in reply
        return reply;
    }
    UserInfo userInfo = userInfoMaybe.value();
    QString homePath = userInfo.homePath;

    // Create a temporary directory accessible to the new user
    QString tempDirPath;
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to create temporary directory."));
        return reply;
    }
    tempDirPath = tempDir.path();

    QStringList filesToCopy = {
        QStringLiteral("kwinoutputconfig.json"),
        QStringLiteral("kwinrc"),
    };

    QString sourceBasePath = QDir::cleanPath(QStringLiteral("/run/plasma-setup/.config"));

    // Copy files to temp directory while we still have privileges
    for (const QString &fileName : filesToCopy) {
        QString sourceFilePath = QDir::cleanPath(sourceBasePath + QStringLiteral("/") + fileName);
        QString tempFilePath = QDir::cleanPath(tempDirPath + QStringLiteral("/") + fileName);

        QFile sourceFile(sourceFilePath);
        if (!sourceFile.copy(tempFilePath)) {
            reply = ActionReply::HelperErrorReply();
            reply.setErrorDescription(QStringLiteral("Unable to copy file to temp directory: ") + sourceFilePath + QStringLiteral(" to ") + tempFilePath
                                      + QStringLiteral(" -- Error message: ") + sourceFile.errorString());
            return reply;
        }

        // Make sure the user can read the file
        if (chmod(tempFilePath.toLocal8Bit().constData(), 0644) != 0) {
            reply = ActionReply::HelperErrorReply();
            reply.setErrorDescription(QStringLiteral("Unable to set permissions on ") + tempFilePath + QStringLiteral(": error code ")
                                      + QString::number(errno));
            return reply;
        }
    }

    // Set permissions on the temp directory so the new user can access it
    if (chmod(tempDirPath.toLocal8Bit().constData(), 0755) != 0) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to set directory permissions: error code ") + QString::number(errno));
        return reply;
    }

    // Ensure the config directory exists
    QString destBasePath = QDir::cleanPath(homePath + QStringLiteral("/.config"));
    QDir destDir(destBasePath);
    if (!destDir.exists()) {
        destDir.mkpath(destBasePath);
    }

    PrivilegeGuard guard(userInfo);

    // Copy display scaling configuration files to the new user from temp directory
    for (const QString &fileName : filesToCopy) {
        QString tempFilePath = QDir::cleanPath(tempDirPath + QStringLiteral("/") + fileName);
        QString destFilePath = QDir::cleanPath(destBasePath + QStringLiteral("/") + fileName);

        QFile tempFile(tempFilePath);
        if (!tempFile.copy(destFilePath)) {
            reply = ActionReply::HelperErrorReply();
            reply.setErrorDescription(QStringLiteral("Unable to copy file to destination: ") + tempFilePath + QStringLiteral(" to ") + destFilePath
                                      + QStringLiteral(" -- Error message: ") + tempFile.errorString());
            return reply;
        }
    }

    return ActionReply::SuccessReply();
}

ActionReply PlasmaSetupAuthHelper::setnewusertempautologin(const QVariantMap &args)
{
    ActionReply reply;

    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Username argument is missing or invalid."));
        return reply;
    }

    QString username = args[QStringLiteral("username")].toString();

    // Validate the username. We don't actually need the home directory here,
    // but this function performs the necessary security checks.
    auto userInfoMaybe = getUserInfo(username, reply);
    if (!userInfoMaybe.has_value()) {
        // getUserInfo already set the error in reply
        return reply;
    }
    UserInfo userInfo = userInfoMaybe.value();

    QFile file(SDDM_AUTOLOGIN_CONFIG_PATH);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Unable to open file ") + SDDM_AUTOLOGIN_CONFIG_PATH + QStringLiteral(" for writing: ") + file.errorString());
        return reply;
    }

    QTextStream stream(&file);
    stream << "[Autologin]\n";
    stream << "User=" << username << "\n";
    stream << "Session=plasma\n";
    stream << "Relogin=true\n"; // Set Relogin to true for temporary autologin
    file.close();

    return ActionReply::SuccessReply();
}

std::optional<UserInfo> PlasmaSetupAuthHelper::getUserInfo(const QString &username, ActionReply &reply)
{
    struct passwd pwd;
    struct passwd *result = nullptr;
    QByteArray usernameBytes = username.toLocal8Bit();
    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize <= 0) {
        bufsize = 16384; // fallback size
    }
    QByteArray buf(static_cast<int>(bufsize), 0);

    int ret = getpwnam_r(usernameBytes.constData(), &pwd, buf.data(), buf.size(), &result);

    if (result == nullptr) {
        reply = ActionReply::HelperErrorReply();
        if (ret == 0) {
            reply.setErrorDescription(QStringLiteral("User does not exist: ") + username);
        } else {
            reply.setErrorDescription(QStringLiteral("System error while looking up user ") + username + QStringLiteral(": error code ")
                                      + QString::number(ret));
        }
        return std::nullopt;
    }

    if (pwd.pw_uid < MIN_REGULAR_USER_UID) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(QStringLiteral("Refusing to perform action for system user: ") + username);
        return std::nullopt;
    }

    UserInfo userInfo;
    userInfo.username = QString::fromLocal8Bit(pwd.pw_name);
    userInfo.homePath = QString::fromLocal8Bit(pwd.pw_dir);
    userInfo.uid = pwd.pw_uid;
    userInfo.gid = pwd.pw_gid;

    return userInfo;
}

ActionReply PlasmaSetupAuthHelper::makeErrorReply(const QString &errorDescription)
{
    ActionReply reply = ActionReply::HelperErrorReply();
    reply.setErrorDescription(errorDescription);
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.plasmasetup", PlasmaSetupAuthHelper)
