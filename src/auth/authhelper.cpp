// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <QProcess>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingReply>

#include <KAuth/HelperSupport>
#include <KConfigGroup>
#include <KSharedConfig>

#include "authhelper.h"
#include "config-plasma-setup.h"

#include <exception>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * Minimum UID for regular (non-system) users.
 */
constexpr int MIN_REGULAR_USER_UID = 1000;

/**
 * Path to the Plasma Setup home directory.
 */
const QString PLASMA_SETUP_HOMEDIR = QStringLiteral("/run/plasma-setup");

/**
 * Path to the SDDM autologin configuration file.
 */
const QString SDDM_AUTOLOGIN_CONFIG_PATH = QStringLiteral("/etc/sddm.conf.d/99-plasma-setup.conf");

/**
 * RAII guard for temporarily dropping privileges to a specific user.
 *
 * This class uses the RAII (Resource Acquisition Is Initialization) pattern to
 * safely drop privileges from root to a specified user and automatically restore
 * root privileges when the guard goes out of scope.
 *
 * @note This class is not copyable to prevent accidental privilege state duplication.
 *
 * Example usage:
 * @code
 * {
 *     PrivilegeGuard guard(userInfo);
 *     // Code here runs with user privileges
 *     // ...
 * }   // Privileges automatically restored to root when the guard goes out of scope
 * @endcode
 */
class PrivilegeGuard
{
public:
    PrivilegeGuard(const UserInfo &userInfo)
    {
        // Clear supplementary groups that root may belong to. Not likely to be necessary, but just in case.
        if (setgroups(0, nullptr) != 0) {
            throw std::runtime_error("Failed to clear supplementary groups that root may belong to: error " + std::to_string(errno));
        }

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
    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        return makeErrorReply(QStringLiteral("Username argument is missing or invalid."));
    }

    QString username = args[QStringLiteral("username")].toString();

    UserInfo userInfo;
    try {
        userInfo = getUserInfo(username);
    } catch (const std::runtime_error &e) {
        return makeErrorReply(QStringLiteral("Failed to get user info: ") + QString::fromStdString(e.what()));
    }

    QString homePath = userInfo.homePath;

    QString autostartDirPath = QDir::cleanPath(homePath + QStringLiteral("/.config/autostart"));
    QDir autostartDir(autostartDirPath);

    try {
        PrivilegeGuard guard(userInfo);

        // Ensure the autostart directory exists
        if (!autostartDir.exists() && !autostartDir.mkpath(QStringLiteral("."))) {
            return makeErrorReply(QStringLiteral("Unable to create autostart directory: ") + autostartDirPath);
        }

        // Create the desktop entry file
        QString desktopFilePath = autostartDir.filePath(QStringLiteral("remove-autologin.desktop"));
        QFile desktopFile(desktopFilePath);
        if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return makeErrorReply(QStringLiteral("Unable to open file for writing: ") + desktopFilePath + QStringLiteral(" error:")
                                  + desktopFile.errorString());
        }

        QString plasmaSetupExecutablePath = QStringLiteral(PLASMA_SETUP_LIBEXECDIR) + QStringLiteral("/plasma-setup");
        if (plasmaSetupExecutablePath.isEmpty()) {
            desktopFile.close();
            return makeErrorReply(QStringLiteral("Unable to find the Plasma Setup executable path."));
        }

        QTextStream stream(&desktopFile);
        stream << "[Desktop Entry]\n";
        stream << "Type=Application\n";
        stream << "Name=Remove Plasma Setup Autologin\n";
        stream << "Exec=sh -c \"" << plasmaSetupExecutablePath << " --remove-autologin && rm --force '" << desktopFilePath << "'\"\n";
        stream << "X-KDE-StartupNotify=false\n";
        stream << "NoDisplay=true\n";
        desktopFile.close();

        ActionReply reply = ActionReply::SuccessReply();
        reply.setData({{QStringLiteral("autostartFilePath"), desktopFilePath}});
        return reply;
    } catch (const std::runtime_error &e) {
        return makeErrorReply(QStringLiteral("Failed to drop privileges: ") + QString::fromStdString(e.what()));
    }
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
        return makeErrorReply(QStringLiteral("Unable to disable systemd unit: ") + dbusReply.error().message());
    }

    return ActionReply::SuccessReply();
}

ActionReply PlasmaSetupAuthHelper::removeautologin(const QVariantMap &args)
{
    Q_UNUSED(args);

    QFileInfo fileInfo(SDDM_AUTOLOGIN_CONFIG_PATH);

    if (!fileInfo.exists()) {
        return ActionReply::SuccessReply();
    }

    QFile file(fileInfo.filePath());
    if (!file.remove()) {
        QString errorDetails = file.errorString();
        return makeErrorReply(QStringLiteral("Unable to remove file ") + fileInfo.filePath() + QStringLiteral(": ") + errorDetails);
    }

    return ActionReply::SuccessReply();
}

ActionReply PlasmaSetupAuthHelper::setnewuserglobaltheme(const QVariantMap &args)
{
    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        return makeErrorReply(QStringLiteral("Username argument is missing or invalid."));
    }

    QString username = args[QStringLiteral("username")].toString();

    UserInfo userInfo;
    try {
        userInfo = getUserInfo(username);
    } catch (const std::runtime_error &e) {
        return makeErrorReply(QStringLiteral("Failed to get user info: ") + QString::fromStdString(e.what()));
    }

    QString homePath = userInfo.homePath;

    // Copy the file to a temp file while we still have privileges
    QString sourceFilePath = PLASMA_SETUP_HOMEDIR + QStringLiteral("/.config/kdeglobals");
    std::unique_ptr<QTemporaryFile> tempFile;
    try {
        tempFile = copyToTempFile(sourceFilePath);
    } catch (const std::runtime_error &e) {
        return makeErrorReply(QStringLiteral("Error copying file to temporary location: ") + QString::fromStdString(e.what()));
    }

    try {
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
            return makeErrorReply(QStringLiteral("Unable to create .config directory: ") + configDirPath);
        }

        // Set the global theme for the new user
        QString destFilePath = QDir::cleanPath(configDirPath + QStringLiteral("/kdeglobals"));

        if (!tempFile->copy(destFilePath)) {
            return makeErrorReply(QStringLiteral("Unable to copy file to destination: ") + tempFile->fileName() + QStringLiteral(" to ") + destFilePath
                                  + QStringLiteral(" -- Error message: ") + tempFile->errorString());
        }

        return ActionReply::SuccessReply();
    } catch (const std::runtime_error &e) {
        return makeErrorReply(QStringLiteral("Failed to drop privileges: ") + QString::fromStdString(e.what()));
    }
}

ActionReply PlasmaSetupAuthHelper::setnewuserdisplayscaling(const QVariantMap &args)
{
    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        return makeErrorReply(QStringLiteral("Username argument is missing or invalid."));
    }

    QString username = args[QStringLiteral("username")].toString();

    UserInfo userInfo;
    try {
        userInfo = getUserInfo(username);
    } catch (const std::runtime_error &e) {
        return makeErrorReply(QStringLiteral("Failed to get user info: ") + QString::fromStdString(e.what()));
    }

    QString homePath = userInfo.homePath;

    QStringList filesToCopy = {
        QStringLiteral("kwinoutputconfig.json"),
        QStringLiteral("kwinrc"),
    };

    QString sourceBasePath = PLASMA_SETUP_HOMEDIR + QStringLiteral("/.config");

    // Copy files to temp files while we still have privileges
    std::map<QString, std::unique_ptr<QTemporaryFile>> tempFiles;
    for (const QString &fileName : filesToCopy) {
        QString sourceFilePath = QDir::cleanPath(sourceBasePath + QStringLiteral("/") + fileName);
        try {
            tempFiles[fileName] = copyToTempFile(sourceFilePath);
        } catch (const std::runtime_error &e) {
            return makeErrorReply(QStringLiteral("Error copying file to temporary location: ") + QString::fromStdString(e.what()));
        }
    }

    try {
        PrivilegeGuard guard(userInfo);

        // Ensure the config directory exists
        QString destBasePath = QDir::cleanPath(homePath + QStringLiteral("/.config"));
        QDir destDir(destBasePath);
        if (!destDir.exists()) {
            destDir.mkpath(destBasePath);
        }

        // Copy display scaling configuration files to the new user from temp files
        for (const QString &fileName : filesToCopy) {
            QString destFilePath = QDir::cleanPath(destBasePath + QStringLiteral("/") + fileName);

            auto tempFile = tempFiles[fileName].get();
            if (!tempFile->copy(destFilePath)) {
                return makeErrorReply(QStringLiteral("Unable to copy file to destination: ") + tempFile->fileName() + QStringLiteral(" to ") + destFilePath
                                      + QStringLiteral(" -- Error message: ") + tempFile->errorString());
            }
        }

        return ActionReply::SuccessReply();
    } catch (const std::runtime_error &e) {
        return makeErrorReply(QStringLiteral("Failed to drop privileges: ") + QString::fromStdString(e.what()));
    }
}

ActionReply PlasmaSetupAuthHelper::setnewusertempautologin(const QVariantMap &args)
{
    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        return makeErrorReply(QStringLiteral("Username argument is missing or invalid."));
    }

    QString username = args[QStringLiteral("username")].toString();

    // Validate the username. We don't actually need the home directory here,
    // but this function performs the necessary security checks.
    try {
        UserInfo userInfo = getUserInfo(username);
    } catch (const std::runtime_error &e) {
        return makeErrorReply(QStringLiteral("Failed to get user info: ") + QString::fromStdString(e.what()));
    }

    QFile file(SDDM_AUTOLOGIN_CONFIG_PATH);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return makeErrorReply(QStringLiteral("Unable to open file ") + SDDM_AUTOLOGIN_CONFIG_PATH + QStringLiteral(" for writing: ") + file.errorString());
    }

    QTextStream stream(&file);
    stream << "[Autologin]\n";
    stream << "User=" << username << "\n";
    stream << "Session=plasma\n";
    stream << "Relogin=true\n"; // Set Relogin to true for temporary autologin
    file.close();

    return ActionReply::SuccessReply();
}

std::unique_ptr<QTemporaryFile> PlasmaSetupAuthHelper::copyToTempFile(const QString &sourceFilePath)
{
    // Create a temporary file
    auto tempFile = std::make_unique<QTemporaryFile>();

    if (!tempFile->open()) {
        throw std::runtime_error("Unable to create temporary file: " + tempFile->errorString().toStdString());
    }

    // Copy the source file to the temp file
    QFile sourceFile(sourceFilePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Unable to open source file: " + sourceFilePath.toStdString() + " -- Error: " + sourceFile.errorString().toStdString());
    }

    QByteArray data = sourceFile.readAll();
    sourceFile.close();

    if (tempFile->write(data) != data.size()) {
        throw std::runtime_error("Unable to write to temporary file: " + tempFile->errorString().toStdString());
    }

    tempFile->flush();

    // Set file permissions to be readable by everyone, so the new user can access it.
    QString tempFilePath = tempFile->fileName();
    if (chmod(tempFilePath.toUtf8().constData(), 0644) != 0) {
        throw std::runtime_error("Unable to set permissions on temporary file: error code " + std::to_string(errno));
    }

    return tempFile;
}

UserInfo PlasmaSetupAuthHelper::getUserInfo(const QString &username)
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
        if (ret == 0) {
            throw std::runtime_error("User does not exist: " + username.toStdString());
        } else {
            throw std::runtime_error("System error while looking up user " + username.toStdString() + ": error code " + std::to_string(ret));
        }
    }

    if (pwd.pw_uid < MIN_REGULAR_USER_UID) {
        throw std::runtime_error("Refusing to perform action for system user: " + username.toStdString());
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

#include "moc_authhelper.cpp"
