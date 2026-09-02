// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

import org.kde.coreaddons
import org.kde.kirigami as Kirigami
import org.kde.plasmasetup.components as PlasmaSetupComponents
import org.kde.plasmasetup.restorebackuputil as RestoreBackupUtil

PlasmaSetupComponents.SetupModule {
    id: root

    property var selectedDrive: null

    RestoreBackupUtil.ExternalDriveBackupsModel {
        id: backupsModel
    }

    DelegateModel {
        id: driveBackupsModel
        model: backupsModel
        rootIndex: selectedDrive
    }

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
                text: i18n("If you have an external drive with backups, you can plug it in to restore from it now.")
            }

            ScrollView {
                id: backupsScrollView
                visible: selectedDrive !== null
                Layout.fillWidth: true
                Layout.bottomMargin: Kirigami.Units.gridUnit

                Component.onCompleted: {
                    if (background) {
                        background.visible = true;
                    }
                }

                ListView {
                    id: backupsListView
                    model: driveBackupsModel
                    Layout.fillWidth: true
                    delegate: FoundBackupDelegate {
                        implicitWidth: backupsListView.width
                    }
                }
            }

            ScrollView {
                id: drivesScrollView
                Layout.fillWidth: true
                Layout.bottomMargin: Kirigami.Units.gridUnit

                visible: selectedDrive === null

                Component.onCompleted: {
                    if (background) {
                        background.visible = true;
                    }
                }

                ListView {
                    id: drivesListView
                    model: backupsModel
                    Layout.fillWidth: true
                    delegate: ExternalDriveDelegate {
                        implicitWidth: drivesListView.width
                        onClicked: {
                            console.warn(backupsModel.index(index, 0))
                            backupsModel.fetchMore(backupsModel.index(index, 0))
                            selectedDrive = backupsModel.index(index, 0)
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
