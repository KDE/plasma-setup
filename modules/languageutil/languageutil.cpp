// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "languageutil.h"
#include "plasmasetup_languageutil_debug.h"

#include <KLocalizedString>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusReply>
#include <QTimer>

LanguageUtil::LanguageUtil(QObject *parent)
    : QObject(parent)
{
    loadAvailableLanguages();
    m_currentLanguage = QLocale::system().name();
    qCInfo(PlasmaSetupLanguageUtil) << "System language detected as:" << m_currentLanguage;
    overrideInitialLanguageIfNeeded();
}

QStringList LanguageUtil::availableLanguages() const
{
    return m_availableLanguages;
}

QString LanguageUtil::currentLanguage() const
{
    return m_currentLanguage;
}

void LanguageUtil::setCurrentLanguage(const QString &language)
{
    if (m_currentLanguage != language) {
        m_currentLanguage = language;
        Q_EMIT currentLanguageChanged();
    }
}

void LanguageUtil::applyLanguage()
{
    if (m_currentLanguage.isEmpty()) {
        return;
    }

    applyLanguageForCurrentSession();
    applyLanguageAsSystemDefault();

    Q_EMIT currentLanguageChanged();
}

void LanguageUtil::applyLanguageForCurrentSession()
{
    qputenv("LANGUAGE", m_currentLanguage.toUtf8());
    qputenv("LANG", m_currentLanguage.toUtf8());
    QLocale::setDefault(QLocale(m_currentLanguage));

    QEvent localeChangeEvent(QEvent::LocaleChange);
    QCoreApplication::sendEvent(QCoreApplication::instance(), &localeChangeEvent);
    QEvent languageChangeEvent(QEvent::LanguageChange);
    QCoreApplication::sendEvent(QCoreApplication::instance(), &languageChangeEvent);
}

void LanguageUtil::applyLanguageAsSystemDefault()
{
    const QString locale1Service = QStringLiteral("org.freedesktop.locale1");
    const QString locale1Path = QStringLiteral("/org/freedesktop/locale1");

    QDBusMessage message = QDBusMessage::createMethodCall( //
        locale1Service,
        locale1Path,
        QStringLiteral("org.freedesktop.locale1"),
        QStringLiteral("SetLocale") //
    );

    const QLocale locale = QLocale(m_currentLanguage);
    const QString lang = QStringLiteral("LANG=") + locale.name() + QStringLiteral(".UTF-8"); // e.g. "LANG=en_US.UTF-8"
    const bool interactive = false;
    message << QStringList{lang} << interactive;

    QDBusPendingReply<> reply = QDBusConnection::systemBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=]() {
        watcher->deleteLater();

        if (watcher->isError()) {
            qCWarning(PlasmaSetupLanguageUtil) << "Failed to set system default language:" << watcher->error().message();
        } else {
            qCInfo(PlasmaSetupLanguageUtil) << "Successfully set system default language.";
        }
    });
}

void LanguageUtil::loadAvailableLanguages()
{
    m_availableLanguages = KLocalizedString::availableDomainTranslations("plasmashell").values();

    // Ensure we at least have English available
    if (!m_availableLanguages.contains(QStringLiteral("en_US"))) {
        m_availableLanguages.append(QStringLiteral("en_US"));
    }

    m_availableLanguages.sort();

    Q_EMIT availableLanguagesChanged();
}

void LanguageUtil::overrideInitialLanguageIfNeeded()
{
    if (m_availableLanguages.contains(m_currentLanguage)) {
        // Current language is available; no override needed.
        return;
    }

    // If the current language has an underscore variant, grab the base language.
    // e.g. "de_DE" becomes "de"
    QString baseLanguage = m_currentLanguage.split(QStringLiteral("_")).first();

    // The language list didn't contain the full locale, but maybe it has the base language.
    if (m_availableLanguages.contains(baseLanguage)) {
        qCInfo(PlasmaSetupLanguageUtil) << "Current language" << m_currentLanguage << "is not available. Overriding to base language" << baseLanguage << ".";
        m_currentLanguage = baseLanguage;
    } else {
        // The base language is also not available, so we'll fall back to English.
        qCWarning(PlasmaSetupLanguageUtil) << "Current language" << m_currentLanguage << "is not available. Defaulting to en_US.";
        m_currentLanguage = QStringLiteral("en_US");
    }

    applyLanguage();

    // Small delay because otherwise the QML side won't see the change and scroll to the new language.
    QTimer::singleShot(0, this, [this]() {
        Q_EMIT initialLanguageOverrideApplied();
    });
}

#include "moc_languageutil.cpp"
