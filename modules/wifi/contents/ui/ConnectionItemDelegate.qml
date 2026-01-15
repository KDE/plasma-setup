// SPDX-FileCopyrightText: 2017 Martin Kacej <m.kacej@atlas.sk>
// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls as Controls

import org.kde.plasma.networkmanagement as PlasmaNM
import org.kde.kirigami as Kirigami

Controls.ItemDelegate {
    id: root

    contentItem: RowLayout {
        spacing: 0

        Item {
            Layout.rightMargin: Kirigami.Units.gridUnit
            implicitWidth: Kirigami.Units.iconSizes.smallMedium
            implicitHeight: Kirigami.Units.iconSizes.smallMedium

            Kirigami.Icon {
                implicitWidth: Kirigami.Units.iconSizes.smallMedium
                implicitHeight: Kirigami.Units.iconSizes.smallMedium
                visible: ConnectionState !== PlasmaNM.Enums.Activating
                anchors.centerIn: parent
                source: mobileProxyModel.showSavedMode ? "network-wireless-connected-100" : ConnectionIcon
            }

            Controls.BusyIndicator {
                anchors.fill: parent
                running: ConnectionState === PlasmaNM.Enums.Activating
            }
        }

        Controls.Label {
            id: internalTextItem
            Layout.fillWidth: true
            text: ItemUniqueName
            elide: Text.ElideRight
            font.bold: ConnectionState === PlasmaNM.Enums.Activated
            Accessible.ignored: true // base class sets this text on root already
        }

        RowLayout {
            Kirigami.Icon {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                visible: ConnectionState === PlasmaNM.Enums.Activated
                source: 'checkmark'
            }

            // ensure that the row is always of same height
            Controls.ToolButton {
                id: heightMetrics
                opacity: 0
                implicitWidth: 0
                icon.name: 'network-connect'
                enabled: false
            }
        }
    }
}
