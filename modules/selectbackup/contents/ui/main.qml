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
        spacing: Kirigami.Units.largeSpacing

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            RadioButton {
                id: dontRestoreCheck
                checked: true
                text: i18nc("@option:radio", "Do not restore from a backup")
            }

            RadioButton {
                id: doRestoreCheck
                text: i18nc("@option:radio this element is followed by a list view of backups", "Restore from this backup:")
            }
        }

        RowLayout {
            id: drivesBackupsLayout
            enabled: doRestoreCheck.checked

            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true
            Layout.fillHeight: true

            spacing: Kirigami.Units.smallSpacing

            ScrollView {
                id: drivesScrollView

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.maximumWidth: selectedDrive === null ? -1 : Math.round(drivesBackupsLayout.width / 2)

                Kirigami.StyleHints.showFramedBackground: true

                ListView {
                    id: drivesListView

                    Kirigami.PlaceholderMessage {
                        visible: drivesListView.count === 0

                        anchors.centerIn: parent
                        width: parent.width - (Kirigami.Units.largeSpacing * 4)

                        text: i18nc("@info:usagetip", "If you’d like to restore from a backup on an external disk that was made using Plasma’s built-in backup system, plug the disk in now.")
                    }

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
                        onClicked: ListView.view.currentIndex = index
                    }
                }
            }

            ScrollView {
                id: backupsScrollView

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.maximumWidth: Math.round(drivesBackupsLayout.width / 2)

                visible: selectedDrive !== null

                Kirigami.StyleHints.showFramedBackground: true

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
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true
            Layout.minimumHeight: Kirigami.Units.gridUnit * 3  // prevent layout shift
            wrapMode: Text.Wrap
            text: {
                const backup = backupsListView.currentItem;
                if (!backup || !BackupController.restoreWanted) {
                    return "";
                }
                return xi18nc("@info %1 is username, %2 is a relative date and time, %3 is a partial filesystem path",
                              "User ‘<emphasis strong=\"yes\">%1</emphasis>’ will be restored from the backup taken at %2 (located at <filename>%3</filename>).",
                        backup.username,
                        Format.formatRelativeDateTime(backup.date, Locale.LongFormat),
                        backup.relativeFsPath);
            }
        }
    }
}
