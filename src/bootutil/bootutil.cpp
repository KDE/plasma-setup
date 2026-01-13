// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "bootutil.h"

#include "plasmasetup_bootutil_debug.h"

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QDBusConnection>
#include <QDBusReply>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>

/**
 * Path to the SDDM autologin configuration file.
 */
const QString SDDM_AUTOLOGIN_CONFIG_PATH = QStringLiteral("/etc/sddm.conf.d/99-plasma-setup.conf");

/**
 * Path to the PlasmaLogin autologin configuration file.
 */
const QString PLASMALOGIN_AUTOLOGIN_CONFIG_PATH = QStringLiteral("/etc/plasmalogin.conf.d/99-plasma-setup.conf");

/**
 * Path to the config to the active display manager (sddm or plasmalogin)
 */
static QString displayManagerConfigPath()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.systemd1"),
                                                      QStringLiteral("/org/freedesktop/systemd1"),
                                                      QStringLiteral("org.freedesktop.systemd1.Manager"),
                                                      QStringLiteral("GetUnitFileState"));
    msg << QStringLiteral("plasmalogin.service");

    QDBusReply<QString> usingPlasmaLoginQuery = QDBusConnection::systemBus().call(msg);
    if (usingPlasmaLoginQuery.value() == QLatin1String("enabled")) {
        return PLASMALOGIN_AUTOLOGIN_CONFIG_PATH;
    }
    return SDDM_AUTOLOGIN_CONFIG_PATH;
}

BootUtil::BootUtil(QObject *parent)
    : QObject(parent)
{
}

bool BootUtil::writeDisplayManagerAutologin(const bool autoLogin)
{
    // Make sure the directory exists
    const QString configFilePath = displayManagerConfigPath();
    QFileInfo fileInfo(configFilePath);
    QDir dir = fileInfo.dir();

    // If autologin is to be disabled, remove the file if it exists
    if (!autoLogin) {
        if (fileInfo.exists()) {
            if (!QFile::remove(fileInfo.filePath())) {
                qCWarning(PlasmaSetupBootUtil) << "Failed to remove file:" << fileInfo.filePath();
                return false;
            }
        }
        return true;
    }

    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        qCWarning(PlasmaSetupBootUtil) << "Failed to create directory:" << dir.absolutePath();
        return false;
    }

    QFile file(configFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(PlasmaSetupBootUtil) << "Failed to open file for writing:" << file.fileName();
        return false;
    }

    // Write the autologin configuration
    QTextStream stream(&file);
    stream << "[Autologin]\n";
    stream << "User=plasma-setup\n";
    stream << "Session=plasma\n";
    file.close();

    removeEmptyAutologinEntry();

    qCInfo(PlasmaSetupBootUtil) << "Display Manager autologin configuration written successfully.";
    return true;
}

void BootUtil::removeEmptyAutologinEntry()
{
    const QString configFilePath = QStringLiteral("/etc/sddm.conf.d/kde_settings.conf");

    KSharedConfigPtr config = KSharedConfig::openConfig(configFilePath);
    config->deleteGroup(QStringLiteral("Autologin"));
    config->sync();

    qCInfo(PlasmaSetupBootUtil) << "Removed empty autologin group from SDDM configuration.";
}

#include "bootutil.moc"

#include "moc_bootutil.cpp"
