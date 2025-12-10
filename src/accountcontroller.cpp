// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "accountcontroller.h"

#include "config-plasma-setup.h"
#include "plasmasetup_debug.h"
#include "usernamevalidator.h"

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QApplication>
#include <QFile>
#include <QVariantMap>

#include <pwd.h>

/**
 * @brief Minimum UID for user accounts
 *
 * This value represents the absolute lowest user ID value possible.
 */
constexpr int MINIMUM_USER_ID = 0;
/**
 * @brief Maximum UID for local user accounts
 *
 * This value represents the typical highest user ID value possible for
 * local UNIX user accounts.
 */
constexpr int MAXIMUM_USER_ID = 65535;
/**
 * @brief Default minimum UID for regular user accounts
 *
 * This value is used as a fallback when LOGIN_DEFS_PATH cannot be read
 * or does not contain a valid UID_MIN setting. UIDs below this threshold
 * are typically reserved for system accounts.
 */
constexpr int DEFAULT_MIN_REGULAR_USER_ID = 1000;
/**
 * @brief Default maximum UID for regular user accounts
 *
 * This value is used as a fallback when LOGIN_DEFS_PATH cannot be read
 * or does not contain a valid UID_MAX setting. UIDs above this threshold
 * are typically reserved for dynamic accounts.
 */
constexpr int DEFAULT_MAX_REGULAR_USER_ID = 65000;

AccountController::AccountController(QObject *parent)
    : QObject(parent)
{
    initializeExistingUserFlag();
}

AccountController::~AccountController() = default;

AccountController *AccountController::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)

    QJSEngine::setObjectOwnership(instance(), QQmlEngine::CppOwnership);
    return instance();
}

AccountController *AccountController::s_instance = nullptr;

AccountController *AccountController::instance()
{
    if (!s_instance) {
        s_instance = new AccountController();
    }
    return s_instance;
}

QString AccountController::username() const
{
    return m_username;
}

void AccountController::setUsername(const QString &username)
{
    qCInfo(PlasmaSetup) << "Setting username to" << username;

    if (m_username == username) {
        return;
    }
    m_username = username;
    Q_EMIT usernameChanged();
}

QString AccountController::fullName() const
{
    return m_fullName;
}

void AccountController::setFullName(const QString &fullName)
{
    qCInfo(PlasmaSetup) << "Setting full name to" << fullName;

    if (m_fullName == fullName) {
        return;
    }

    m_fullName = fullName;
    Q_EMIT fullNameChanged();
}

bool AccountController::createUser()
{
    qCInfo(PlasmaSetup) << "Creating user" << m_username << "with full name" << m_fullName;

    QList<QWindow *> topLevelWindows = QGuiApplication::topLevelWindows();
    QWindow *window = topLevelWindows.isEmpty() ? nullptr : topLevelWindows.first();

    KAuth::Action action(QStringLiteral("org.kde.plasmasetup.createuser"));
    action.setParentWindow(window);
    action.setHelperId(QStringLiteral("org.kde.plasmasetup"));
    action.setArguments({
        {QStringLiteral("username"), m_username},
        {QStringLiteral("fullName"), m_fullName},
        {QStringLiteral("password"), m_password},
        {QStringLiteral("extraGroups"), userGroupsFromConfig()},
    });

    KAuth::ExecuteJob *job = action.execute();
    if (!job->exec()) {
        const QString errorMessage =
            job->errorString().isEmpty() ? QStringLiteral("Authorization or helper failure (code %1)").arg(job->error()) : job->errorString();
        qCWarning(PlasmaSetup) << "Failed to create user:" << errorMessage;
        return false;
    }

    const QVariantMap userData = job->data();
    qCInfo(PlasmaSetup) << "User created successfully. UID:" << userData.value(QStringLiteral("uid")).toLongLong()
                        << "Home:" << userData.value(QStringLiteral("homePath")).toString();

    return true;
}

QString AccountController::password() const
{
    return m_password;
}

void AccountController::setPassword(const QString &password)
{
    m_password = password;
    Q_EMIT passwordChanged();
}

bool AccountController::isUsernameValid(const QString &username) const
{
    return PlasmaSetupValidation::Account::isUsernameValid(username);
}

QString AccountController::usernameValidationMessage(const QString &username) const
{
    const auto result = PlasmaSetupValidation::Account::validateUsername(username);

    return PlasmaSetupValidation::Account::usernameValidationMessage(result);
}

QStringList AccountController::userGroupsFromConfig() const
{
    QStringList parsedGroups;

    const QString configPath = QString::fromUtf8(PLASMA_SETUP_CONFIG_PATH);
    if (!configPath.isEmpty()) {
        KConfig config(configPath, KConfig::SimpleConfig);
        KConfigGroup accountsGroup(&config, QStringLiteral("Accounts"));
        const QString configuredGroups = accountsGroup.readEntry(QStringLiteral("UserGroups"), QString());

        if (!configuredGroups.isEmpty()) {
            const auto groupNames = configuredGroups.split(QLatin1Char(','), Qt::SkipEmptyParts);
            for (const QString &groupName : groupNames) {
                parsedGroups << groupName.trimmed();
            }
        }
    }

    if (parsedGroups.isEmpty()) {
        parsedGroups << QStringLiteral("wheel");
    }

    return parsedGroups;
}

bool AccountController::hasExistingUsers() const
{
    return m_hasExistingUsers;
}

void AccountController::initializeExistingUserFlag()
{
    if (isAccountCreationOverrideEnabled()) {
        return;
    }

    const std::pair<int, int> uidRange = regularUserUidRange();
    const bool hasExistingUsers = detectExistingUsers(uidRange);

    if (!hasExistingUsers) {
        return;
    }

    qCInfo(PlasmaSetup) << "Existing users detected, the account module will not be shown.";
    m_hasExistingUsers = hasExistingUsers;
    Q_EMIT hasExistingUsersChanged();
}

bool AccountController::isAccountCreationOverrideEnabled()
{
    if (!qEnvironmentVariableIntValue("PLASMA_SETUP_USER_CREATION_OVERRIDE")) {
        return false;
    }

    m_hasExistingUsers = false;
    Q_EMIT hasExistingUsersChanged();
    qCInfo(PlasmaSetup) << "PLASMA_SETUP_USER_CREATION_OVERRIDE is set to enable; account creation will be enabled regardless of existing users.";
    return true;
}

std::pair<int, int> AccountController::regularUserUidRange() const
{
    const QString loginDefsPath = QStringLiteral(LOGIN_DEFS_PATH);
    QFile loginDefs(loginDefsPath);
    if (!loginDefs.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(PlasmaSetup) << "Unable to open" << loginDefs.fileName() << ':' << loginDefs.errorString();
        return std::pair<int, int>{DEFAULT_MIN_REGULAR_USER_ID, DEFAULT_MAX_REGULAR_USER_ID};
    }

    QTextStream stream(&loginDefs);
    static const QRegularExpression whitespace(QStringLiteral("\\s+"));

    int uidMinValue = DEFAULT_MIN_REGULAR_USER_ID;
    int uidMaxValue = DEFAULT_MAX_REGULAR_USER_ID;
    while (!stream.atEnd()) {
        const QString trimmedLine = stream.readLine().trimmed();
        if (trimmedLine.isEmpty() || trimmedLine.startsWith(QLatin1Char('#'))) {
            continue;
        }

        const QStringList tokens = trimmedLine.split(whitespace, Qt::SkipEmptyParts);
        if (tokens.size() >= 2 && tokens.at(0) == QLatin1String("UID_MIN")) {
            bool ok = false;
            const int parsedValue = tokens.at(1).toInt(&ok);
            if (ok && parsedValue >= 0) {
                uidMinValue = parsedValue;
            } else {
                qCWarning(PlasmaSetup) << "Invalid UID_MIN value in" << loginDefs.fileName() << ':' << tokens.at(1);
            }
        }
        if (tokens.size() >= 2 && tokens.at(0) == QLatin1String("UID_MAX")) {
            bool ok = false;
            const int parsedValue = tokens.at(1).toInt(&ok);
            if (ok && parsedValue >= 0) {
                uidMaxValue = parsedValue;
            } else {
                qCWarning(PlasmaSetup) << "Invalid UID_MAX value in" << loginDefs.fileName() << ':' << tokens.at(1);
            }
        }
    }

    return std::pair<int, int>{uidMinValue, uidMaxValue};
}

bool AccountController::detectExistingUsers(std::pair<int, int> uidRange) const
{
    struct PasswdScopeGuard {
        PasswdScopeGuard()
        {
            setpwent();
        }
        ~PasswdScopeGuard()
        {
            endpwent();
        }
    } guard;

    const uid_t uidMin = static_cast<uid_t>(std::max(uidRange.first, MINIMUM_USER_ID));
    const uid_t uidMax = static_cast<uid_t>(std::min(uidRange.second, MAXIMUM_USER_ID));
    errno = 0;
    while (passwd *entry = getpwent()) {
        if (entry->pw_uid >= uidMin && entry->pw_uid <= uidMax) {
            return true;
        }
    }

    if (errno != 0) {
        qCWarning(PlasmaSetup) << "Failed while enumerating passwd entries:" << QString::fromLocal8Bit(strerror(errno));
    }

    return false;
}

#include "moc_accountcontroller.cpp"
