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

    RestoreBackupUtil.ExternalDriveBackupsModel {
        id: backupsModel
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
                visible: backupsView.rows == 0
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                text: i18n("If you have an external drive with backups, you can plug it in to restore from it now.")
            }

            ScrollView {
                id: backupsScrollView
                Layout.fillWidth: true
                Layout.bottomMargin: Kirigami.Units.gridUnit

                visible: backupsView.rows > 0

                Component.onCompleted: {
                    if (background) {
                        background.visible = true;
                    }
                }

                TreeView {
                    id: backupsView
                    model: backupsModel
                    columnWidthProvider: function(_col) { return backupsView.width }
                    Layout.fillWidth: true
                    delegate: TreeViewDelegate {
                        hoverEnabled: false
                        implicitWidth: backupsView.width
                        contentItem: Label {
                            text: depth === 0
                                  ? model.display
                                  : i18n("User ‘%1’ on %2 (in %3)",
                                         model.username,
                                         Format.formatRelativeDateTime(model.date, Locale.ShortFormat),
                                         model.relativeFsPath)
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
