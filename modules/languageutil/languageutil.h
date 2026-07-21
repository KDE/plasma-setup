// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
// SPDX-FileCopyrightText: 2026 Tiziano Gaia <ti.gaia@proton.me>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QStringListModel>

#include "languagesortfilterproxymodel.h"

/**
 * Handles language choice.
 *
 * This class provides functionality to load available languages,
 * set the current language, and apply the selected language.
 *
 * Language choice applies to this application, the current session, and as the system default.
 */
class LanguageUtil : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    /**
     * List of available languages in the system.
     *
     * A list of language codes that are available for use in the application.
     * Each entry in the list represents a language that can be selected by the user.
     */
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages NOTIFY availableLanguagesChanged)

    /**
     * Model containing the available languages.
     *
     * This model supports filtering through languageFilter and is exposed
     * directly to QML views that display the available language choices.
     */
    Q_PROPERTY(QAbstractItemModel *languageModel READ languageModel CONSTANT)

    /**
     * Search filter applied to the language model.
     *
     * Updating this value filters languages by their code, native name,
     * or English name.
     */
    Q_PROPERTY(QString languageFilter READ languageFilter WRITE setLanguageFilter NOTIFY languageFilterChanged)

    /**
     * The language code of the currently selected language.
     */
    Q_PROPERTY(QString currentLanguage READ currentLanguage WRITE setCurrentLanguage NOTIFY currentLanguageChanged)

public:
    explicit LanguageUtil(QObject *parent = nullptr);

    QStringList availableLanguages() const;
    QAbstractItemModel *languageModel();
    QString languageFilter() const;
    void setLanguageFilter(const QString &filter);

    QString currentLanguage() const;
    void setCurrentLanguage(const QString &language);

    /**
     * Applies the chosen language.
     */
    Q_INVOKABLE void applyLanguage();

Q_SIGNALS:
    void availableLanguagesChanged();
    void languageFilterChanged();
    void currentLanguageChanged();
    void initialLanguageOverrideApplied();

private:
    /**
     * Applies the current language setting for the current user session.
     */
    void applyLanguageForCurrentSession();

    /**
     * Applies the selected language as the system default.
     *
     * Sets the selected language as the default for the entire system, so that
     * it will apply to new users, the login screen, and other system components.
     */
    void applyLanguageAsSystemDefault();

    /**
     * Loads the available languages from the system.
     *
     * This function populates the availableLanguages list with the languages
     * that are supported by Plasma.
     */
    void loadAvailableLanguages();

    /**
     * Overrides the initial language if it is not available.
     *
     * If the current system language at boot is not in the list of available languages
     * the picker uses, this function defaults to English and emits a signal to
     * notify that an override has been applied so the UI can show the correct selection.
     *
     * This ensures that the language picker always starts with a valid default selection.
     */
    void overrideInitialLanguageIfNeeded();

    QStringList m_availableLanguages;
    QStringListModel m_languageModel;
    LanguageSortFilterProxyModel m_languageProxyModel;
    QString m_languageFilter;
    QString m_currentLanguage;
};
