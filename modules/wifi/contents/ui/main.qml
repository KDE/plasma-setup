// SPDX-FileCopyrightText: 2017 Martin Kacej <m.kacej@atlas.sk>
// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.networkmanagement as PlasmaNM

import org.kde.plasmasetup.components as PlasmaSetupComponents

PlasmaSetupComponents.SetupModule {
    id: root

    available: availableDevices.wirelessDeviceAvailable

    contentItem: ColumnLayout {
        id: mainColumn

        PlasmaNM.AvailableDevices {
            id: availableDevices
        }

        PlasmaNM.Handler {
            id: handler
        }

        PlasmaNM.EnabledConnections {
            id: enabledConnections
        }

        PlasmaNM.NetworkModel {
            id: connectionModel
        }

        PlasmaNM.MobileProxyModel {
            id: mobileProxyModel
            sourceModel: connectionModel
            showSavedMode: false
        }

        ConnectDialog {
            id: connectionDialog
            handler: handler
            parent: root.contentItem.Overlay.overlay
        }

        Component.onCompleted: handler.requestScan()

        Timer {
            id: scanTimer
            interval: 10200
            repeat: true
            running: parent.visible

            onTriggered: handler.requestScan()
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter

            Label {
                id: titleLabel
                Layout.leftMargin: Kirigami.Units.gridUnit
                Layout.rightMargin: Kirigami.Units.gridUnit
                Layout.bottomMargin: Kirigami.Units.gridUnit
                Layout.fillWidth: true

                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                text: i18n("Connect to a WiFi network for network access.")
            }

            ScrollView {
                id: savedCard
                Layout.fillWidth: true
                visible: enabledConnections.wirelessEnabled && count > 0

                Component.onCompleted: {
                    if (background) {
                        background.visible = true;
                    }
                }

                // number of visible entries
                property int count: 0
                function updateCount() {
                    count = 0;
                    for (let i = 0; i < connectedRepeater.count; i++) {
                        let item = connectedRepeater.itemAt(i);
                        if (item && item.shouldDisplay) {
                            count++;
                        }
                    }
                }

                ColumnLayout {
                    id: column

                    width: savedCard.width

                    Repeater {
                        id: connectedRepeater
                        model: mobileProxyModel
                        delegate: ConnectionItemDelegate {
                            // connected or saved
                            property bool shouldDisplay: (Uuid != "") || ConnectionState === PlasmaNM.Enums.Activated
                            onShouldDisplayChanged: savedCard.updateCount()

                            // separate property for visible since visible is false when the whole card is not visible
                            visible: (Uuid != "") || ConnectionState === PlasmaNM.Enums.Activated

                            Layout.fillWidth: true

                            hoverEnabled: false
                        }
                    }
                }
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: enabledConnections.wirelessEnabled

                Component.onCompleted: {
                    if (background) {
                        background.visible = true;
                    }
                }

                ListView {
                    clip: true
                    model: mobileProxyModel

                    delegate: ConnectionItemDelegate {
                        width: ListView.view.width
                        height: visible ? implicitHeight : 0
                        visible: !((Uuid != "") || ConnectionState === PlasmaNM.Enums.Activated)

                        onClicked: {
                            changeState()
                        }

                        property bool predictableWirelessPassword: !Uuid && Type == PlasmaNM.Enums.Wireless &&
                        (SecurityType == PlasmaNM.Enums.StaticWep ||
                        SecurityType == PlasmaNM.Enums.WpaPsk ||
                        SecurityType == PlasmaNM.Enums.Wpa2Psk ||
                        SecurityType == PlasmaNM.Enums.SAE)

                        function changeState() {
                            if (Uuid || !predictableWirelessPassword) {
                                if (ConnectionState == PlasmaNM.Enums.Deactivated) {
                                    if (!predictableWirelessPassword && !Uuid) {
                                        handler.addAndActivateConnection(DevicePath, SpecificPath);
                                    } else {
                                        handler.activateConnection(ConnectionPath, DevicePath, SpecificPath);
                                    }
                                } else{
                                    //show popup
                                }
                            } else if (predictableWirelessPassword) {
                                connectionDialog.headingText = i18n("Connect to") + " " + ItemUniqueName;
                                connectionDialog.devicePath = DevicePath;
                                connectionDialog.specificPath = SpecificPath;
                                connectionDialog.securityType = SecurityType;
                                connectionDialog.openAndClear();
                            }
                        }
                    }
                }
            }
        }
    }
}
