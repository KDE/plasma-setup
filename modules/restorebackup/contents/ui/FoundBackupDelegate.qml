// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.coreaddons
import org.kde.kirigami as Kirigami

ItemDelegate {
    id: root
    required property int index
    required property string username
    required property var date
    required property string relativeFsPath

    implicitWidth: ListView.view.width
    highlighted: ListView.isCurrentItem

    contentItem: RowLayout {
        spacing: 0

        Item {
            Layout.rightMargin: Kirigami.Units.smallSpacing
            implicitWidth: Kirigami.Units.iconSizes.smallMedium
            implicitHeight: Kirigami.Units.iconSizes.smallMedium

            Kirigami.Icon {
                implicitWidth: Kirigami.Units.iconSizes.smallMedium
                implicitHeight: Kirigami.Units.iconSizes.smallMedium
                anchors.centerIn: parent
                source: "backup"
            }
        }

        Label {
            Layout.fillWidth: true
            text: date !== undefined ? Format.formatRelativeDateTime(date, Locale.LongFormat) : ""
        }
    }
}
