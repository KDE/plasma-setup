// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

import org.kde.coreaddons
import org.kde.kirigami as Kirigami
import org.kde.plasmasetup
import org.kde.plasmasetup.components as PlasmaSetupComponents
import org.kde.plasmasetup.selectbackuputil as SelectBackupUtil

PlasmaSetupComponents.SetupModule {
    id: root

    property var selectedDrive: null

    SelectBackupUtil.ExternalDriveBackupsModel {
        id: backupsModel
    }

    DelegateModel {
        id: driveBackupsModel
        model: backupsModel
        rootIndex: selectedDrive
    }

    available: true
    nextEnabled: true

    Binding {
        target: BackupController
        property: "restoreWanted"
        value: !dontRestoreCheck.checked && backupsListView.currentIndex !== -1
    }

    contentItem: ColumnLayout {
        anchors.fill: parent

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter

            RadioButton {
                id: dontRestoreCheck
                checked: true
                text: i18n("Do not restore from a backup")
            }

            RadioButton {
                id: doRestoreCheck
                text: i18n("Restore from this backup:")
            }


            RowLayout {
                id: drivesBackupsLayout
                enabled: doRestoreCheck.checked

                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                Layout.minimumHeight: Kirigami.Units.gridUnit * 12
                Layout.maximumHeight: Kirigami.Units.gridUnit * 20
                Layout.maximumWidth: Kirigami.Units.gridUnit * 35

                spacing: Kirigami.Units.smallSpacing

                Item {
                    visible: drivesListView.count === 0
                    Layout.alignment: Qt.AlignCenter
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: parent.width - (Kirigami.Units.largeSpacing * 4)

                        text: i18nc("@info:usagetip", "If you’d like to restore from a backup on an external disk that was made using Plasma’s built-in backup system, plug the disk in now.")
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
                                    ListView.view.currentIndex = -1;
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
                            ListView.onRemove: {
                                if (ListView.view.currentIndex === index) {
                                    ListView.view.currentIndex = -1;
                                }
                            }
                        }

                        onCurrentIndexChanged: {
                            if (backupsListView.currentIndex === -1) {
                                BackupController.username = "";
                                BackupController.sourceDir = "";
                                BackupController.sourceName = "";
                                BackupController.sourceRevision = "";
                            }
                            const backup = backupsListView.currentItem;
                            BackupController.username = backup.username;
                            BackupController.sourceDir = backup.fsPath;
                            BackupController.sourceName = backup.name;
                            BackupController.sourceRevision = backup.revision;
                        }
                    }
                }
            }

            Label {
                Layout.maximumWidth: Kirigami.Units.gridUnit * 35
                Layout.minimumHeight: Kirigami.Units.gridUnit * 3  // prevent layout shift
                wrapMode: Text.Wrap
                text: {
                    const backup = backupsListView.currentItem;
                    if (!backup || !BackupController.restoreWanted) {
                        return "\n\n";
                    }
                    return i18n("User <b>‘%1’</b> will be restored from the backup taken at %2 (located at %3).",
                           backup.username,
                           Format.formatRelativeDateTime(backup.date, Locale.LongFormat),
                           backup.relativeFsPath);
                }
            }
        }
    }
}
