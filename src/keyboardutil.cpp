// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "keyboardutil.h"

#include "initialstartutil.h"
#include "plasmasetup_debug.h"

#include <KConfig>
#include <KConfigGroup>

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QTextStream>

KeyboardUtil::KeyboardUtil(QObject *parent)
    : QObject(parent)
{
    readCurrentKeyboardLayout();
}

QString KeyboardUtil::layoutName() const
{
    return m_layoutName;
}

void KeyboardUtil::setLayoutName(const QString &layoutName)
{
    if (m_layoutName == layoutName) {
        return;
    }

    m_layoutName = layoutName;
    Q_EMIT layoutNameChanged();
}

QString KeyboardUtil::layoutVariant() const
{
    return m_layoutVariant;
}

void KeyboardUtil::setLayoutVariant(const QString &layoutVariant)
{
    if (m_layoutVariant == layoutVariant) {
        return;
    }

    m_layoutVariant = layoutVariant;
    Q_EMIT layoutVariantChanged();
}

QString KeyboardUtil::layoutOptions() const
{
    return m_layoutOptions;
}

void KeyboardUtil::setLayoutOptions(const QString &layoutOptions)
{
    if (m_layoutOptions == layoutOptions) {
        return;
    }

    m_layoutOptions = layoutOptions;
    Q_EMIT layoutOptionsChanged();
}

void KeyboardUtil::applyLayout()
{
    if (m_layoutName.isEmpty()) {
        qCWarning(PlasmaSetup) << "No keyboard layout set.";
        return;
    }

    if (!InitialStartUtil::runningAsPlasmaSetupUser()) {
        qCInfo(PlasmaSetup) << "Not running as plasma-setup user; skipping keyboard layout application. Would have applied layout:" << m_layoutName
                            << "with variant:" << m_layoutVariant << "and options:" << m_layoutOptions;
        return;
    }

    qCInfo(PlasmaSetup) << "Applying keyboard layout:" << m_layoutName << "with variant:" << m_layoutVariant << "and options:" << m_layoutOptions;

    applyLayoutForCurrentUser();
    applyLayoutAsSystemDefault();
}

void KeyboardUtil::applyLayoutForCurrentUser()
{
    auto config = new KConfig(QStringLiteral("kxkbrc"), KConfig::NoGlobals);
    KConfigGroup group = config->group(QStringLiteral("Layout"));
    group.writeEntry(QStringLiteral("DisplayNames"), "", KConfig::Notify);
    group.writeEntry(QStringLiteral("LayoutList"), m_layoutName, KConfig::Notify);
    group.writeEntry(QStringLiteral("VariantList"), m_layoutVariant, KConfig::Notify);
    config->sync();
    delete config;
}

void KeyboardUtil::applyLayoutAsSystemDefault()
{
    const QString locale1Service = QStringLiteral("org.freedesktop.locale1");
    const QString locale1Path = QStringLiteral("/org/freedesktop/locale1");

    QDBusMessage message = QDBusMessage::createMethodCall( //
        locale1Service,
        locale1Path,
        QStringLiteral("org.freedesktop.locale1"),
        QStringLiteral("SetX11Keyboard") //
    );

    // Default model is hardcoded, since we don't have a way to detect the actual model for now.
    const QString model = QStringLiteral("pc105");
    // Convert option allows the layout to be applied everywhere with a single command.
    const bool convert = true;
    const bool interactive = false;

    message << m_layoutName << model << m_layoutVariant << m_layoutOptions << convert << interactive;

    QDBusMessage resultMessage = QDBusConnection::systemBus().call(message);

    if (resultMessage.type() == QDBusMessage::ErrorMessage) {
        qCWarning(PlasmaSetup) << "Failed to set system default keyboard layout:" << resultMessage.errorMessage();
    } else {
        qCInfo(PlasmaSetup) << "Successfully set system default keyboard layout.";
    }
}

void KeyboardUtil::readCurrentKeyboardLayout()
{
    const QString locale1Service = QStringLiteral("org.freedesktop.locale1");
    const QString locale1Path = QStringLiteral("/org/freedesktop/locale1");

    QDBusInterface interface(locale1Service, locale1Path, QStringLiteral("org.freedesktop.locale1"), QDBusConnection::systemBus());
    if (!interface.isValid()) {
        qCWarning(PlasmaSetup) << "Failed to connect to dbus interface for reading current keyboard layout:" << interface.lastError().message();
        return;
    }

    m_layoutName = interface.property("X11Layout").toString();
    m_layoutVariant = interface.property("X11Variant").toString();
    m_layoutOptions = interface.property("X11Options").toString();
}

#include "moc_keyboardutil.cpp"
