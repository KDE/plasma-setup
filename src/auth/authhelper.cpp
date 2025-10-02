// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <QProcess>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingReply>

#include <KAuth/HelperSupport>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include "authhelper.h"
#include "config-plasma-setup.h"

#include <pwd.h>

const QString SDDM_AUTOLOGIN_CONFIG_PATH = QStringLiteral("/etc/sddm.conf.d/99-plasma-setup.conf");

/**
 * Minimum UID for regular (non-system) users.
 */
constexpr int MIN_REGULAR_USER_UID = 1000;

ActionReply PlasmaSetupAuthHelper::createnewuserautostarthook(const QVariantMap &args)
{
    ActionReply reply;

    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(i18n("Username argument is missing or invalid."));
        return reply;
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
        reply.setErrorDescription(i18nc("%1 is a directory path", "Failed to create autostart directory: %1", autostartDirPath));
        return reply;
    }

    // Create the desktop entry file
    QString desktopFilePath = autostartDir.filePath(QStringLiteral("remove-autologin.desktop"));
    QFile desktopFile(desktopFilePath);
    if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(i18nc("%1 is a file path", "Failed to open file for writing: %1", desktopFilePath));
        return reply;
    }

    QString plasmaSetupExecutablePath = QStringLiteral(PLASMA_SETUP_LIBEXECDIR) + QStringLiteral("/plasma-setup");
    if (plasmaSetupExecutablePath.isEmpty()) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(i18n("Failed to find the Plasma Setup executable path."));
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
        actionReply.setErrorDescription(i18nc("%1 is an error message", "Failed to disable systemd unit: %1", dbusReply.error().message()));
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

    if (!QFile::remove(fileInfo.filePath())) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(i18nc("%1 is a file path", "Failed to remove file %1", fileInfo.filePath()));
        return reply;
    }

    return ActionReply::SuccessReply();
}

ActionReply PlasmaSetupAuthHelper::setnewuserglobaltheme(const QVariantMap &args)
{
    ActionReply reply;

    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(i18n("Username argument is missing or invalid."));
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
        reply.setErrorDescription(i18nc("%1 is a directory path", "Failed to create .config directory: %1", configDirPath));
        return reply;
    }

    // Set the global theme for the new user
    QString sourceFilePath = QStringLiteral("/run/plasma-setup/.config/kdeglobals");
    QString destFilePath = QDir::cleanPath(configDirPath + QStringLiteral("/kdeglobals"));

    if (!QFile::copy(sourceFilePath, destFilePath)) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(i18nc("%1 is a file path", "Failed to copy file %1", sourceFilePath));
        return reply;
    }

    return ActionReply::SuccessReply();
}

ActionReply PlasmaSetupAuthHelper::setnewuserdisplayscaling(const QVariantMap &args)
{
    ActionReply reply;

    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(i18n("Username argument is missing or invalid."));
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

    // Copy display scaling configuration files to the new user
    QString sourceBasePath = QDir::cleanPath(QStringLiteral("/run/plasma-setup/.config"));
    QString destBasePath = QDir::cleanPath(homePath + QStringLiteral("/.config"));
    QDir destDir(destBasePath);
    if (!destDir.exists()) {
        destDir.mkpath(destBasePath);
    }

    QStringList filesToCopy = {
        QStringLiteral("kwinoutputconfig.json"),
        QStringLiteral("kwinrc"),
    };
    for (const QString &fileName : filesToCopy) {
        QString sourceFilePath = QDir::cleanPath(sourceBasePath + QStringLiteral("/") + fileName);
        QString destFilePath = QDir::cleanPath(destBasePath + QStringLiteral("/") + fileName);
        if (!QFile::copy(sourceFilePath, destFilePath)) {
            reply = ActionReply::HelperErrorReply();
            reply.setErrorDescription(i18nc("%1 is a file path", "Failed to copy file %1", sourceFilePath));
            return reply;
        }
    }

    return ActionReply::SuccessReply();
}

ActionReply PlasmaSetupAuthHelper::setnewuserhomedirectoryownership(const QVariantMap &args)
{
    ActionReply reply;

    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(i18n("Username argument is missing or invalid."));
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

    // Recursively set ownership of the home directory to the new user
    int exitCode = QProcess::execute(QStringLiteral("chown"), {QStringLiteral("-R"), username + QStringLiteral(":") + username, homePath});

    if (exitCode != 0) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(i18nc("%1 is a directory path, %2 an exit code", "Failed to set ownership for home directory %1: exit code %2", homePath, exitCode));
        return reply;
    }

    return ActionReply::SuccessReply();
}

ActionReply PlasmaSetupAuthHelper::setnewusertempautologin(const QVariantMap &args)
{
    ActionReply reply;

    if (!args.contains(QStringLiteral("username")) || !args[QStringLiteral("username")].canConvert<QString>()) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(i18n("Username argument is missing or invalid."));
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
        reply.setErrorDescription(i18nc("%1 is a file path", "Failed to open file %1 for writing.", SDDM_AUTOLOGIN_CONFIG_PATH));
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
            reply.setErrorDescription(i18nc("%1 is a username", "User does not exist: %1", username));
        } else {
            reply.setErrorDescription(i18nc("%1 is a username, %2 is an error code", "System error while looking up user %1: error code %2", username, ret));
        }
        return std::nullopt;
    }

    if (pwd.pw_uid < MIN_REGULAR_USER_UID) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(i18nc("%1 is a username", "Refusing to perform action for system user: %1", username));
        return std::nullopt;
    }

    UserInfo userInfo;
    userInfo.username = QString::fromLocal8Bit(pwd.pw_name);
    userInfo.homePath = QString::fromLocal8Bit(pwd.pw_dir);
    userInfo.uid = pwd.pw_uid;
    userInfo.gid = pwd.pw_gid;

    return userInfo;
}

KAUTH_HELPER_MAIN("org.kde.plasmasetup", PlasmaSetupAuthHelper)
