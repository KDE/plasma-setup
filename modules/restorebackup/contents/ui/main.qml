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
            Layout.alignment: Qt.AlignCenter
            spacing: Kirigami.Units.gridUnit

            RowLayout {
                id: drivesBackupsLayout

                Layout.fillWidth: true
                Layout.minimumHeight: Kirigami.Units.gridUnit * 12

                spacing: Kirigami.Units.smallSpacing

                Item {
                    visible: drivesListView.count === 0
                    Layout.alignment: Qt.AlignCenter
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: parent.width - (Kirigami.Units.largeSpacing * 4)

                        text: i18n("If you have an external drive with backups, you can plug it in to restore from it now.")
                    }
                }

                ScrollView {
                    id: drivesScrollView

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.maximumWidth: selectedDrive === null ? -1 : drivesBackupsLayout.width / 2

                    visible: drivesListView.count !== 0

                    Component.onCompleted: if (background) background.visible = true

                    ListView {
                        id: drivesListView

                        Layout.fillWidth: true

                        clip: true
                        currentIndex: -1

                        model: backupsModel
                        onCurrentIndexChanged: {
                            if (currentIndex === -1) {
                                selectedDrive = null;
                                return;
                            }
                            const modelIndex = backupsModel.index(currentIndex, 0);
                            if (backupsModel.canFetchMore(modelIndex)) {
                                backupsModel.fetchMore(modelIndex);
                            }
                            selectedDrive = modelIndex;
                        }
                        delegate: ExternalDriveDelegate {
                            ListView.onRemove: {
                                if (selectedDrive !== null && selectedDrive.row === index) {
                                    selectedDrive = null;
                                    currentIndex = -1;
                                }
                            }
                            onClicked: {
                                ListView.view.currentIndex = index;
                            }
                        }
                    }
                }

                ScrollView {
                    id: backupsScrollView

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.maximumWidth: drivesBackupsLayout.width / 2

                    visible: selectedDrive !== null

                    Component.onCompleted: if (background) background.visible = true

                    ListView {
                        id: backupsListView
                        clip: true
                        currentIndex: -1

                        BusyIndicator {
                            anchors.centerIn: parent
                            running: backupsListView.count === 0 && drivesListView.currentItem && drivesListView.currentItem.isScanning
                        }

                        model: driveBackupsModel
                        activeFocusOnTab: true
                        delegate: FoundBackupDelegate {
                            onClicked: {
                                ListView.view.currentIndex = index;
                            }
                        }
                    }
                }
            }

            CheckBox {
                visible: backupsListView.currentIndex !== -1
                Layout.fillWidth: true
                text: {
                    if (backupsListView.currentIndex === -1) {
                        return "";
                    }
                    const backup = backupsListView.currentItem
                    i18n("Restore from backup of user ‘%1’ taken on %2 (located at %3)?",
                         backup.username,
                         Format.formatRelativeDateTime(backup.date, Locale.LongFormat),
                         backup.relativeFsPath)
                }
                checked: true
            }
        }
    }
}
