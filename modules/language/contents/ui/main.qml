// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
// SPDX-FileCopyrightText: 2026 Tiziano Gaia <ti.gaia@proton.me>
// SPDX-License-Identifier: LGPL-2.1-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasmasetup.languageutil as Language

import org.kde.plasmasetup.components as PlasmaSetupComponents

/**
* @brief Module for selecting system language
*
* This module provides a user interface for selecting the system language.
* It displays a scrollable list of available languages with search functionality
* to help users easily find and select their preferred language.
*/
PlasmaSetupComponents.SetupModule {
    id: root

    function onPageActivated(): void {
        searchField.forceActiveFocus();
    }

    contentItem: ColumnLayout {
        id: mainColumn

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: Kirigami.Units.smallSpacing

            Label {
                id: titleLabel
                Layout.leftMargin: Kirigami.Units.gridUnit
                Layout.rightMargin: Kirigami.Units.gridUnit
                Layout.bottomMargin: Kirigami.Units.gridUnit
                Layout.fillWidth: true

                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                text: i18n("Please select your preferred language.")
            }

            Kirigami.SearchField {
                id: searchField
                Layout.fillWidth: true
                placeholderText: i18n("Search languages…")

                onTextChanged: {
                    Language.LanguageUtil.languageFilter = text;
                }
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Component.onCompleted: {
                    if (background) {
                        background.visible = true;
                    }
                }

                ListView {
                    id: languageListView
                    clip: true
                    model: Language.LanguageUtil.languageModel

                    currentIndex: -1 // Ensure focus is not on the listview

                    delegate: RadioDelegate {
                        required property string languageCode

                        width: ListView.view.width

                        // Get language name from locale code (e.g., "en_US" -> "English (United States)")
                        text: {
                            const locale = Qt.locale(languageCode);
                            const localeName = locale.nativeLanguageName;

                            // First letter to uppercase
                            return localeName.charAt(0).toUpperCase()
                                + localeName.slice(1)
                                + " ("
                                + locale.nativeCountryName
                                + ")";
                        }

                        checked: Language.LanguageUtil.currentLanguage === languageCode

                        onToggled: {
                            if (checked && languageCode !== Language.LanguageUtil.currentLanguage) {
                                Language.LanguageUtil.currentLanguage = languageCode;
                                Language.LanguageUtil.applyLanguage();
                                checked = Qt.binding(() => Language.LanguageUtil.currentLanguage === languageCode);
                            }
                        }
                    }

                    function scrollToCurrentLanguage() {
                        // Find the index of the current language
                        const currentLang = Language.LanguageUtil.currentLanguage;

                        for (let i = 0; i < languageListView.count; i++) {
                            const item = languageListView.itemAtIndex(i);

                            if (item && item.languageCode === currentLang) {
                                // Position the view at the current language with some offset
                                languageListView.positionViewAtIndex(i, ListView.Center);
                                break;
                            }
                        }
                    }

                    Component.onCompleted: {
                        // Scroll to the current language when the view is ready
                        Qt.callLater(() => {
                            if (Language.LanguageUtil.currentLanguage) {
                                scrollToCurrentLanguage();
                            }
                        });
                    }

                    Connections {
                        target: Language.LanguageUtil

                        // Scroll to the correct language if an initial override was applied.
                        function onInitialLanguageOverrideApplied() {
                            languageListView.scrollToCurrentLanguage();
                        }
                    }
                }
            }
        }
    }
}
