// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "accountcontroller.h"

#include "accounts_interface.h"
#include "plasmasetup_debug.h"
#include "user.h"

#include <QDBusObjectPath>

using namespace Qt::StringLiterals;

AccountController::AccountController(QObject *parent)
    : QObject(parent)
    , m_dbusInterface(new OrgFreedesktopAccountsInterface(u"org.freedesktop.Accounts"_s, u"/org/freedesktop/Accounts"_s, QDBusConnection::systemBus(), this))
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

    bool isAdmin = true;
    QDBusPendingReply<QDBusObjectPath> reply = m_dbusInterface->CreateUser(m_username, m_fullName, static_cast<qint32>(isAdmin));
    reply.waitForFinished();
    if (reply.isValid()) {
        qCInfo(PlasmaSetup) << "User created with path" << reply.value().path();
        User *createdUser = new User(this);
        createdUser->setPath(reply.value());
        createdUser->setPassword(m_password);
        delete createdUser;
        return true;
    } else {
        qCWarning(PlasmaSetup) << "Failed to create user:" << reply.error().message();
        return false;
    }
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
