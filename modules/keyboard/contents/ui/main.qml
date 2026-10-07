/*
SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>
SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels

import org.kde.plasmasetup
import org.kde.plasmasetup.components as PlasmaSetupComponents

import org.kde.plasma.private.kcm_keyboard as KCMKeyboard

PlasmaSetupComponents.SetupModule {
    id: root

    available: !Kirigami.Settings.isMobile

    function onPageActivated(): void {
        // Temporarily disable the highlight move duration so the correct
        // layout is preselected when the page is activated.
        layoutsView.highlightMoveDuration = 0;
        variantView.highlightMoveDuration = 0;
        // Set the initial current item to the system's current layout.
        layoutsView.currentIndex = layoutsProxy.findCurrentLayoutIndex();
        // Set the initial current variant to the system's current variant.
        variantView.currentIndex = variantProxy.findCurrentVariantIndex();
        // Reset the highlight move duration to the default value.
        layoutsView.highlightMoveDuration = -1;
        variantView.highlightMoveDuration = -1;
        searchField.forceActiveFocus();
    }

    KCMKeyboard.VariantsModel {
        id: variantProxy
        sourceModel: layoutModel
        searchString: searchField.text
        shortName: layoutsView.currentItem?.shortName ?? ""

        // Returns the index matching the system's current keyboard layout variant, or -1 if not found.
        function findCurrentVariantIndex(): int {
            const layoutIndex = layoutsView.currentIndex;
            if (layoutIndex === -1) {
                return -1; // No layout selected, so no variant to find
            }

            const layoutModelIndex = layoutsProxy.index(layoutIndex, 0);
            const layoutShortName = layoutsProxy.data(layoutModelIndex, KItemModels.KRoleNames.role("shortName"));

            // The system's current layout variant, if any (e.g. "dvorak" in "us(dvorak)").
            //
            // If there are multiple variants configured (e.g. a layout of "us,ru(bak)") we only
            // want to consider the second one for variant selection — in such a case the first one
            // should be a blank variant for the latin fallback layout, while the second one is for
            // the actual non-latin layout we care about showing in the picker.
            const currentVariantName = KeyboardUtil.layoutVariant.split(",").pop().trim();

            // Iterate through the rows to find the variant that matches the system's current variant for the selected layout.
            for (let i = 0; i < rowCount(); i++) {
                const proxyIndex = index(i, 0);
                const shortName = data(proxyIndex, KItemModels.KRoleNames.role("shortName"));
                const variantName = data(proxyIndex, KItemModels.KRoleNames.role("variantName"));

                if (shortName === layoutShortName && variantName === currentVariantName) {
                    console.warn("Found keyboard variant matching system default. Layout:", shortName, "Variant:", variantName);
                    return i;
                }
            }

            console.warn("No keyboard variant matching system default found for layout:", layoutShortName, "and variant name:", currentVariantName);
            return -1; // Not found
        }
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
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true

                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                text: i18n("Select your keyboard layout and language.")
            }

            Kirigami.SearchField {
                id: searchField
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                implicitHeight: mainColumn.height - titleLabel.height - searchField.height - Kirigami.Units.smallSpacing

                ScrollView {
                    clip: true
                    implicitWidth: Math.round(mainColumn.width / 2)
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Component.onCompleted: {
                        if (background) {
                            background.visible = true;
                        }
                    }

                    contentItem: ListView {
                        id: layoutsView
                        model: KCMKeyboard.LayoutSearchModel {
                            id: layoutsProxy
                            sourceModel: KCMKeyboard.LayoutModel {
                                id: layoutModel
                            }
                            searchString: searchField.text

                            // Returns the index matching the system's current keyboard layout, or -1 if not found.
                            function findCurrentLayoutIndex(): int {
                                // If there are multiple layouts configured (e.g. "us,ru") we only want to consider the
                                // second one for layout selection — in such a case the first one should be `us` to
                                // pair with a non-latin layout.
                                const layoutName = KeyboardUtil.layoutName.split(",").pop().trim();

                                // Iterate through the rows to find the layout that matches the system's current layout.
                                for (let i = 0; i < layoutsProxy.rowCount(); i++) {
                                    const proxyIndex = layoutsProxy.index(i, 0);
                                    const shortName = layoutsProxy.data(proxyIndex, KItemModels.KRoleNames.role("shortName"));
                                    const description = layoutsProxy.data(proxyIndex, KItemModels.KRoleNames.role("description"));

                                    if (shortName === layoutName) {
                                        console.warn("Found keyboard layout matching system default:", shortName, description);
                                        return i;
                                    }
                                }

                                console.warn("No keyboard layout matching system default found for layout name:", layoutName);
                                return -1; // Not found
                            }
                        }
                        delegate: LayoutDelegate {}
                        keyNavigationEnabled: true
                        activeFocusOnTab: true
                    }
                }

                ScrollView {
                    clip: true
                    implicitWidth: Math.round(mainColumn.width / 2)
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Component.onCompleted: {
                        if (background) {
                            background.visible = true;
                        }
                    }

                    contentItem: ListView {
                        id: variantView
                        model: layoutsView.currentItem ? variantProxy : []
                        delegate: LayoutDelegate {}
                        keyNavigationEnabled: true
                        activeFocusOnTab: true
                    }
                }
            }
        }
    }

    component LayoutDelegate: ItemDelegate {
        id: delegate

        width: ListView.view.width
        highlighted: ListView.isCurrentItem

        required property string shortName
        required property string description
        required property string variantName
        required property int index

        onClicked: {
            ListView.view.currentIndex = index;
            KeyboardUtil.layoutName = shortName;
            KeyboardUtil.layoutVariant = variantName;
            KeyboardUtil.applyLayout();
        }

        Accessible.name: delegate.description

        contentItem: Kirigami.TitleSubtitle {
            title: delegate.description
            wrapMode: Text.Wrap
        }
    }
}
