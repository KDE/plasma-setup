// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.plasmasetup.time.private as Time

import org.kde.plasmasetup.components as PlasmaSetupComponents

PlasmaSetupComponents.SetupModule {
    id: root

    contentItem: ColumnLayout {
        id: mainColumn
        spacing: Kirigami.Units.gridUnit

        Label {
            id: titleLabel
            Layout.leftMargin: Kirigami.Units.gridUnit
            Layout.rightMargin: Kirigami.Units.gridUnit
            Layout.topMargin: Kirigami.Units.gridUnit
            Layout.fillWidth: true

            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            text: i18n("Select your time zone and preferred time format.")
        }

        FormCard.FormCard {
            id: timeFormatCard

            Layout.fillWidth: true

            FormCard.FormSwitchDelegate {
                Layout.fillWidth: true
                text: i18n("24-Hour Format")
                checked: Time.TimeUtil.is24HourTime
                onCheckedChanged: {
                    if (checked !== Time.TimeUtil.is24HourTime) {
                        Time.TimeUtil.is24HourTime = checked;
                    }
                }
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter

            Kirigami.SearchField {
                id: searchField
                Layout.fillWidth: true

                onTextChanged: {
                    Time.TimeUtil.timeZones.filterString = text;
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
                    id: listView

                    clip: true
                    model: Time.TimeUtil.timeZones
                    currentIndex: -1 // ensure focus is not on the listview

                    bottomMargin: 2

                    delegate: FormCard.FormRadioDelegate {
                        required property string timeZoneId

                        width: ListView.view.width
                        text: timeZoneId
                        checked: Time.TimeUtil.currentTimeZone === timeZoneId
                        onToggled: {
                            if (checked && timeZoneId !== Time.TimeUtil.currentTimeZone) {
                                Time.TimeUtil.currentTimeZone = timeZoneId;
                                checked = Qt.binding(() => Time.TimeUtil.currentTimeZone === timeZoneId);
                            }
                        }
                    }
                }
            }
        }
    }
}
