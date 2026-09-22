// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.plasmasetup.components as PlasmaSetupComponents
import org.kde.plasmasetup

PlasmaSetupComponents.SetupModule {
    id: root

    nextEnabled: true

    /*!
     * The message shown if a backup was selected to be restored.
     */
    property string backupRestoreRequestedMessage: xi18nc(
        "@info finishing setup",
        "Your device is almost ready.<nl/><nl/>After clicking <interface>Finish</interface>, your backup will be restored, and you will be able to sign in to your account."
    )

    /*!
     * The message shown if a backup is actively being restored.
     */
    property string backupRestoreInProgressMessage: xi18nc(
        "@info finishing setup; %1 is the distro name",
        "Your device is almost ready.<nl/><nl/>Your backup is being restored. Once done, you will be able to sign in to your account.<nl/><nl/>Enjoy <application>%1</application>!",
        InitialStartUtil.distroName
    )

    /*!
     * The message shown if a backup has finished being restored.
     */
    property string backupRestoreFinishedMessage: xi18nc(
        "@info finishing setup; %1 is the distro name",
        "Your device is now ready.<nl/><nl/>Enjoy <application>%1</application>!",
        InitialStartUtil.distroName
    )
    /*!
    * The message shown to users who already have an account on the system.
    */
    property string existingUserFinishedMessage: xi18nc(
        "@info finishing setup; %1 is the distro name",
        "Your device is now ready.<nl/><nl/>Enjoy <application>%1</application>!",
        InitialStartUtil.distroName
    )

    /*!
    * The message shown to users who have just created a new account.
    */
    property string newUserFinishedMessage: xi18nc(
        "@info finishing setup; %1 is the distro name",
        "Your device is now ready.<nl/><nl/>After clicking <interface>Finish</interface> you will be able to sign in to your new account.<nl/><nl/>Enjoy <application>%1</application>!",
        InitialStartUtil.distroName
    )

    contentItem: ColumnLayout {
        id: mainColumn

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter

            Label {
                id: finishedMessage
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                text: {
                    if (BackupController.restoreWanted) {
                        if (InitialStartUtil.backupRestoreRunning) {
                            return root.backupRestoreInProgressMessage;
                        }
                        return root.backupRestoreRequestedMessage;
                    }
                    return AccountController.hasExistingUsers
                        ? root.existingUserFinishedMessage
                        : root.newUserFinishedMessage;
                }
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }

            ProgressBar {
                Layout.fillWidth: true
                visible: InitialStartUtil.backupRestoreRunning
                indeterminate: true
            }

            Image {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: Kirigami.Units.gridUnit
                Layout.maximumHeight: mainColumn.height - finishedMessage.height - Kirigami.Units.gridUnit
                fillMode: Image.PreserveAspectFit
                source: "konqi-calling.png"
            }
        }
    }
}
