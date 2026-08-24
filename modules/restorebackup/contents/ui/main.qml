// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasmasetup.components as PlasmaSetupComponents

PlasmaSetupComponents.SetupModule {
    id: root

    available: true
    nextEnabled: true

    contentItem: ColumnLayout {
        ColumnLayout {
            Layout.maximumWidth: root.cardWidth
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
                text: i18n("If you have an external drive with backups on it, you can choose to restore from it now.")
            }

            ScrollView {
                id: externalDrivesView
                Layout.fillWidth: true
                Layout.bottomMargin: Kirigami.Units.gridUnit

                Component.onCompleted: {
                    if (background) {
                        background.visible = true;
                    }
                }

                ColumnLayout {
                    width: externalDrivesView.width

                    Repeater {
                        model: ["16 GiB Pendrive", "32 GiB Pendrive"]
                        delegate: ExternalDriveDelegate {
                            Layout.fillWidth: true
                            hoverEnabled: false
                        }
                    }
                }
            }

            Button {
                Layout.alignment: Qt.AlignCenter
                text: i18n("Browse manually…")
            }
        }
    }
}
