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

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Frame {
                anchors.fill: parent
                visible: !Kirigami.Settings.isMobile
            }

            Label {
                anchors {
                    bottom: tzSelector.top
                    bottomMargin: Kirigami.Units.gridUnit
                    horizontalCenter: parent.horizontalCenter
                }
                width: Math.min(parent.width, Kirigami.Units.gridUnit * 24) - Kirigami.Units.gridUnit * 2
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                text: i18n("Select your time zone.")
            }

            TimeZone.TimezoneSelector {
                id: tzSelector
                anchors.centerIn: parent
                width: Kirigami.Settings.isMobile
                    ? Math.min(parent.width, Kirigami.Units.gridUnit * 24)
                    : parent.width
                height: Kirigami.Settings.isMobile ? implicitHeight : parent.height
                Component.onCompleted: {
                    // Initialize the selector with the current time zone.
                    //
                    // We do this in Component.onCompleted instead of in the property
                    // binding directly because otherwise the combobox is empty.
                    selectedTimeZone = Time.TimeUtil.currentTimeZone
                }
                onSelectedTimeZoneChanged: Time.TimeUtil.currentTimeZone = selectedTimeZone
            }
        }
    }
}
