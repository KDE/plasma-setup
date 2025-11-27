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
#include <QVariantMap>

AccountController::AccountController(QObject *parent)
    : QObject(parent)
{
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

#include "moc_accountcontroller.cpp"
