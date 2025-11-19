// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasmasetup.timeutil as Time
import org.kde.plasma.workspace.timezoneselector as TimeZone

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
            text: i18n("Select your time zone.") // qmllint disable unqualified
        }

        Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TimeZone.TimezoneSelector {
                id: timezoneSelector

                anchors.fill: parent

                Component.onCompleted: {
                    // Initialize the selector with the current time zone.
                    //
                    // We do this in Component.onCompleted instead of in the property
                    // binding directly because otherwise the combobox is empty.
                    selectedTimeZone = Time.TimeUtil.currentTimeZone;
                }

                onSelectedTimeZoneChanged: {
                    Time.TimeUtil.currentTimeZone = selectedTimeZone;
                }
            }
        }
    }
}
