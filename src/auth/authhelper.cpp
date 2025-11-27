// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "authhelper.h"
#include "../usernamevalidator.h"

#include "config-plasma-setup.h"

#include <KAuth/HelperSupport>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingReply>

#include <algorithm>
#include <cerrno>
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

/**
 * Find a system executable by searching custom paths first, then system PATH.
 *
 * @param executableName Name of the executable to find (e.g., "useradd")
 * @return Full path to the executable, or empty string if not found
 */
static QString findExecutable(const QString &executableName)
{
    const QStringList searchPaths = {
        QStringLiteral("/usr/sbin"),
        QStringLiteral("/usr/bin"),
        QStringLiteral("/sbin"),
        QStringLiteral("/bin"),
    };

    QString result = QStandardPaths::findExecutable(executableName, searchPaths);
    if (result.isEmpty()) {
        // Fallback to default PATH search
        result = QStandardPaths::findExecutable(executableName);
    }

    return result;
}

/**
 * Validate that a group name uses only safe characters and exists on the system.
 *
 * @return Empty string if the group is valid, or an error description otherwise.
 */
static QString validateGroupName(const QString &group)
{
    static const QRegularExpression allowedPattern(QStringLiteral("^[A-Za-z0-9_]+$"));
    if (!allowedPattern.match(group).hasMatch()) {
        return QStringLiteral("Invalid group name: %1").arg(group);
    }

    struct group grp;
    struct group *result = nullptr;
    QByteArray groupBytes = group.toLocal8Bit();
    long bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (bufsize <= 0) {
        bufsize = 16384; // fallback size
    }
    if (bufsize > INT_MAX) {
        return QStringLiteral("System group buffer size too large: %1").arg(bufsize);
    }
    QByteArray buf(static_cast<int>(bufsize), 0);

    const int ret = getgrnam_r(groupBytes.constData(), &grp, buf.data(), buf.size(), &result);
    if (ret != 0) {
        return QStringLiteral("System error while looking up group %1: error code %2").arg(group).arg(ret);
    }

    if (result == nullptr) {
        return QStringLiteral("Unknown group: %1").arg(group);
    }

    return QString();
}

ActionReply PlasmaSetupAuthHelper::createuser(const QVariantMap &args)
{
    // Extract and validate input arguments.
    //
    // The password is handled directly as a QByteArray to allow secure memory clearing after use,
    // which is not possible with QString and QVariant due to their internal memory management.
    const QVariant usernameVariant = args.value(QStringLiteral("username"));
    const QVariant fullNameVariant = args.value(QStringLiteral("fullName"));

    // Ensure required arguments are present and can be converted
    if (!usernameVariant.canConvert<QString>() || !args.value(QStringLiteral("password")).canConvert<QByteArray>()) {
        return makeErrorReply(QStringLiteral("Username or password argument is missing or invalid."));
    }

    const QString username = usernameVariant.toString().trimmed();
    const QString fullName = fullNameVariant.canConvert<QString>() ? fullNameVariant.toString().trimmed() : QString();

    const auto validationResult = PlasmaSetupValidation::Account::validateUsername(username);
    if (validationResult != PlasmaSetupValidation::Account::UsernameValidationResult::Valid) {
        return makeErrorReply(PlasmaSetupValidation::Account::usernameValidationMessage(validationResult));
    }

    QByteArray password = args.value(QStringLiteral("password")).toByteArray();

    // Validate password is not empty
    if (password.isEmpty()) {
        // Clear password data from memory for security
        std::fill(password.begin(), password.end(), '\0');
        return makeErrorReply(QStringLiteral("Password cannot be empty."));
    }

    // Build useradd command arguments
    QStringList useraddArguments;

    // -m: Create a home directory for the new user
    useraddArguments << QStringLiteral("-m");

    // -U: Create a group with the same name as the user and add the user to this group
    useraddArguments << QStringLiteral("-U");

    // -c: Set the user's full name (comment field)
    if (!fullName.isEmpty()) {
        useraddArguments << QStringLiteral("-c") << fullName;
    }

    // The username to create
    useraddArguments << username;

    // Locate the useradd executable
    const QString useraddBinary = findExecutable(QStringLiteral("useradd"));
    if (useraddBinary.isEmpty()) {
        return makeErrorReply(QStringLiteral("Could not locate useradd executable."));
    }

    // Execute useradd to create the user account
    QProcess useraddProcess;
    useraddProcess.start(useraddBinary, useraddArguments);

    if (!useraddProcess.waitForStarted()) {
        return makeErrorReply(QStringLiteral("Failed to start useradd: ") + useraddProcess.errorString());
    }

    if (!useraddProcess.waitForFinished()) {
        useraddProcess.kill();
        useraddProcess.waitForFinished();
        return makeErrorReply(QStringLiteral("useradd timed out."));
    }

    // Check if useradd completed successfully
    if (useraddProcess.exitStatus() != QProcess::NormalExit || useraddProcess.exitCode() != 0) {
        const QString stderrOutput = QString::fromLocal8Bit(useraddProcess.readAllStandardError()).trimmed();
        const QString stdoutOutput = QString::fromLocal8Bit(useraddProcess.readAllStandardOutput()).trimmed();
        return makeErrorReply(QStringLiteral("useradd failed with exit code %1: %2 %3").arg(useraddProcess.exitCode()).arg(stderrOutput).arg(stdoutOutput));
    }

    const ActionReply extraGroupReply = addUserToExtraGroups(username, args.value(QStringLiteral("extraGroups")));
    if (extraGroupReply.type() != ActionReply::SuccessType) {
        return extraGroupReply;
    }

    // We set the password separately using chpasswd to avoid exposing it in command-line arguments
    // that could be listened to in process listings.
    //
    // Locate the chpasswd executable to set the user's password
    const QString chpasswdBinary = findExecutable(QStringLiteral("chpasswd"));
    if (chpasswdBinary.isEmpty()) {
        return makeErrorReply(QStringLiteral("User created but could not locate chpasswd executable."));
    }

    // Start chpasswd process
    QProcess chpasswdProcess;
    chpasswdProcess.start(chpasswdBinary);

    if (!chpasswdProcess.waitForStarted()) {
        return makeErrorReply(QStringLiteral("Failed to start chpasswd: ") + chpasswdProcess.errorString());
    }

    // Prepare password data in the format "username:password\n"
    QByteArray passwordData = username.toUtf8();
    passwordData.append(':');
    passwordData.append(password);
    passwordData.append('\n');

    // Write password data to chpasswd's stdin
    if (chpasswdProcess.write(passwordData) != passwordData.size()) {
        // Writing to chpasswd failed.
        //
        // Clear password data from memory for security
        std::fill(password.begin(), password.end(), '\0');
        std::fill(passwordData.begin(), passwordData.end(), '\0');
        // Kill the process and return an error
        chpasswdProcess.kill();
        chpasswdProcess.waitForFinished();
        return makeErrorReply(QStringLiteral("Failed to write password to chpasswd: ") + chpasswdProcess.errorString());
    }

    // Close stdin to signal we're done writing
    chpasswdProcess.closeWriteChannel();

    // Clear password data from memory for security
    std::fill(password.begin(), password.end(), '\0');
    std::fill(passwordData.begin(), passwordData.end(), '\0');

    // Wait for chpasswd to complete
    if (!chpasswdProcess.waitForFinished()) {
        chpasswdProcess.kill();
        chpasswdProcess.waitForFinished();
        return makeErrorReply(QStringLiteral("chpasswd timed out."));
    }

    // Check if chpasswd completed successfully
    if (chpasswdProcess.exitStatus() != QProcess::NormalExit || chpasswdProcess.exitCode() != 0) {
        const QString stderrOutput = QString::fromLocal8Bit(chpasswdProcess.readAllStandardError()).trimmed();
        return makeErrorReply(QStringLiteral("chpasswd failed with exit code %1: %2").arg(chpasswdProcess.exitCode()).arg(stderrOutput));
    }

    // Retrieve and return the newly created user's information
    try {
        UserInfo userInfo = getUserInfo(username);
        ActionReply reply = ActionReply::SuccessReply();
        reply.setData({
            {QStringLiteral("username"), userInfo.username},
            {QStringLiteral("homePath"), userInfo.homePath},
            {QStringLiteral("uid"), userInfo.uid},
            {QStringLiteral("gid"), userInfo.gid},
        });
        return reply;
    } catch (const std::runtime_error &e) {
        return makeErrorReply(QStringLiteral("User created but failed to retrieve user info: ") + QString::fromStdString(e.what()));
    }
}

ActionReply PlasmaSetupAuthHelper::createflagfile(const QVariantMap &args)
{
    Q_UNUSED(args);

    const QString flagFilePath = QString::fromUtf8(PLASMA_SETUP_DONE_FLAG_PATH);
    QFileInfo flagFileInfo(flagFilePath);
    QDir parentDir = flagFileInfo.dir();

    if (!parentDir.exists() && !parentDir.mkpath(QStringLiteral("."))) {
        return makeErrorReply(QStringLiteral("Unable to create parent directory for flag file: ") + parentDir.path());
    }

    QFile flagFile(flagFilePath);
    if (!flagFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return makeErrorReply(QStringLiteral("Unable to write flag file: ") + flagFile.errorString());
    }

    QTextStream stream(&flagFile);
    stream << "Plasma Setup completed at " << QDateTime::currentDateTimeUtc().toString(Qt::ISODate) << "Z\n";
    flagFile.close();

    if (chmod(flagFilePath.toUtf8().constData(), 0644) != 0) {
        return makeErrorReply(QStringLiteral("Unable to set permissions on flag file: errno ") + QString::number(errno));
    }

    return ActionReply::SuccessReply();
}

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

ActionReply PlasmaSetupAuthHelper::addUserToExtraGroups(const QString &username, const QVariant &extraGroupsVariant)
{
    if (!extraGroupsVariant.canConvert<QStringList>()) {
        return makeErrorReply(QStringLiteral("Extra groups argument is missing or invalid."));
    }

    QStringList extraGroups;

    const QStringList extraGroupsVariantList = extraGroupsVariant.toStringList();
    extraGroups.reserve(extraGroupsVariantList.size());

    for (const QString &group : extraGroupsVariantList) {
        const QString trimmedGroup = group.trimmed();
        if (!trimmedGroup.isEmpty()) {
            const QString validationResult = validateGroupName(trimmedGroup);
            if (!validationResult.isEmpty()) {
                return makeErrorReply(validationResult);
            }
            extraGroups << trimmedGroup;
        }
    }

    if (extraGroups.isEmpty()) {
        return ActionReply::SuccessReply();
    }

    const QString usermodBinary = findExecutable(QStringLiteral("usermod"));
    if (usermodBinary.isEmpty()) {
        return makeErrorReply(QStringLiteral("Could not locate usermod executable for adding groups."));
    }

    QStringList usermodArguments;
    usermodArguments << QStringLiteral("-a") << QStringLiteral("-G") << extraGroups.join(QLatin1Char(','));
    usermodArguments << username;

    QProcess usermodProcess;
    usermodProcess.start(usermodBinary, usermodArguments);

    if (!usermodProcess.waitForStarted()) {
        return makeErrorReply(QStringLiteral("Failed to start usermod: ") + usermodProcess.errorString());
    }

    if (!usermodProcess.waitForFinished()) {
        usermodProcess.kill();
        usermodProcess.waitForFinished();
        return makeErrorReply(QStringLiteral("usermod timed out when adding extra groups."));
    }

    if (usermodProcess.exitStatus() != QProcess::NormalExit || usermodProcess.exitCode() != 0) {
        const QString stderrOutput = QString::fromLocal8Bit(usermodProcess.readAllStandardError()).trimmed();
        const QString stdoutOutput = QString::fromLocal8Bit(usermodProcess.readAllStandardOutput()).trimmed();
        return makeErrorReply(QStringLiteral("usermod failed with exit code %1 while adding extra groups: stderr: %2 stdout: %3")
                                  .arg(usermodProcess.exitCode())
                                  .arg(stderrOutput)
                                  .arg(stdoutOutput));
    }

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
