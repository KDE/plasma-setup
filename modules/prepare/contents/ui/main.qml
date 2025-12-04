// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.plasmasetup.prepareutil as Prepare
import org.kde.plasmasetup.components as PlasmaSetupComponents

PlasmaSetupComponents.SetupModule {
    id: root

    cardWidth: Math.min(Kirigami.Units.gridUnit * 30, root.contentItem.width - Kirigami.Units.gridUnit * 2)

    nextEnabled: true

    contentItem: ColumnLayout {

        ColumnLayout {
            Layout.maximumWidth: root.cardWidth
            Layout.alignment: Qt.AlignCenter
            spacing: Kirigami.Units.smallSpacing

            // TODO: Fix scaling support as seen here: https://invent.kde.org/plasma/plasma-setup/-/merge_requests/56

            // Label {
            // Layout.leftMargin: Kirigami.Units.gridUnit
            // Layout.rightMargin: Kirigami.Units.gridUnit
            // Layout.bottomMargin: Kirigami.Units.gridUnit
            // Layout.alignment: Qt.AlignTop
            // Layout.fillWidth: true

            // wrapMode: Text.Wrap
            // horizontalAlignment: Text.AlignHCenter
            // text: i18n("Adjust the size of elements on the screen.")
            // }

            // FormCard.FormCard {
            // id: scalingCard
            // maximumWidth: root.cardWidth

            // Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

            // FormCard.FormComboBoxDelegate {
            // id: displayScaling
            // text: i18n("Display Scaling")
            // displayMode: FormCard.FormComboBoxDelegate.Dialog
            // model: Prepare.PrepareUtil.scalingOptions

            // Connections {
            // target: Prepare.PrepareUtil

            // function onScalingChanged(): void {
            // displayScaling.currentIndex = Prepare.PrepareUtil.scalingOptions.indexOf(Prepare.PrepareUtil.scaling.toString() + "%");
            // }
            // }

            // // remove % suffix
            // onCurrentValueChanged: Prepare.PrepareUtil.scaling = parseInt(currentValue.substring(0, currentValue.length - 1))
            // }
            // }

                FormCard.FormCard {
                    id: darkThemeCard
                    maximumWidth: root.cardWidth

                    Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

                    FormCard.FormSwitchDelegate {
                        id: darkThemeSwitch
                        text: i18n("Dark Theme")
                        checked: Prepare.PrepareUtil.usingDarkTheme
                        onCheckedChanged: {
                            if (checked !== Prepare.PrepareUtil.usingDarkTheme) {
                                Prepare.PrepareUtil.usingDarkTheme = checked;
                        }
                    }
                }
            }
        }
    }
}
