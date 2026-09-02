// SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

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

            Item {
                Layout.fillWidth: true
                Layout.bottomMargin: Kirigami.Units.gridUnit
                Layout.minimumHeight: Kirigami.Units.gridUnit * 12

                RowLayout {
                    id: drivesBackupsLayout
                    anchors.fill: parent
                    spacing: Kirigami.Units.smallSpacing

                    ScrollView {
                        id: drivesScrollView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.maximumWidth: selectedDrive === null ? -1 : drivesBackupsLayout.width / 2

                        Component.onCompleted: if (background) background.visible = true

                        ListView {
                            id: drivesListView
                            Layout.fillWidth: true
                            clip: true

                            model: backupsModel
                            delegate: ExternalDriveDelegate {
                                implicitWidth: drivesListView.width
                                onClicked: {
                                    drivesListView.currentIndex = index;
                                    const modelIndex = backupsModel.index(index, 0);
                                    console.warn("calling canFetchMore...")
                                    if (backupsModel.canFetchMore(modelIndex)) {
                                        console.warn("couldFetchMore!")
                                        backupsModel.fetchMore(modelIndex);
                                    } else {
                                        console.warn("couldntFetchMore...")
                                    }
                                    selectedDrive = modelIndex;
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

                            model: driveBackupsModel
                            activeFocusOnTab: true
                            delegate: FoundBackupDelegate {
                                implicitWidth: backupsListView.width
                                onClicked: {
                                    backupsListView.currentIndex = index;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
