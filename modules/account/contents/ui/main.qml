// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore as Core

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.plasmasetup
import org.kde.plasmasetup.components as PlasmaSetupComponents

PlasmaSetupComponents.SetupModule {
    id: root

    nextEnabled: usernameField.text.length > 0 && paswordField.text.length > 0 && repeatField.text === paswordField.text

    contentItem: ScrollView {
        anchors.centerIn: parent
        width: Math.min(mainColumn.implicitWidth + (Kirigami.Units.gridUnit * 2), parent.width)
        height: Math.min(mainColumn.implicitHeight, parent.height)

        ColumnLayout {
            id: mainColumn

            width: root.cardWidth

            KirigamiComponents.AvatarButton {
                id: avatar

                property FileDialog fileDialog: null

                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                // Square button
                implicitWidth: Kirigami.Units.gridUnit * 5
                implicitHeight: implicitWidth

                padding: 0

                name: fullNameField.text

                onClicked: {
                    if (fileDialog) {
                        return;
                    }
                    fileDialog = openFileDialog.createObject(this);
                    fileDialog.chosen.connect(receivedSource => {
                                                  if (!receivedSource) {
                                                      return;
                                                  }
                                                  source = receivedSource;
                                              });
                    fileDialog.open();
                }

                Button {
                    anchors {
                        bottom: parent.bottom
                        right: parent.right
                    }
                    visible: avatar.source.toString().length === 0
                    icon.name: "cloud-upload"
                    text: i18n("Upload new avatar")
                    display: AbstractButton.IconOnly

                    onClicked: parent.clicked()

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                Button {
                    anchors {
                        bottom: parent.bottom
                        right: parent.right
                    }
                    visible: avatar.source.toString().length !== 0
                    icon.name: "edit-clear"
                    text: i18n("Remove current avatar")
                    display: AbstractButton.IconOnly

                    onClicked: avatar.source = ""

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                }
                Component {
                    id: openFileDialog

                    FileDialog {
                        currentFolder: Core.StandardPaths.standardLocations(Core.StandardPaths.PicturesLocation)[0]
                        parentWindow: root.Window.window

                        onAccepted: destroy()
                        onRejected: destroy()
                    }
                }
            }

            // Padding around the text
            Item {
                Layout.topMargin: Kirigami.Units.gridUnit
            }

            Label {
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                text: i18n("We need a few details to complete the setup.")

                Layout.leftMargin: Kirigami.Units.gridUnit
                Layout.rightMargin: Kirigami.Units.gridUnit
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter

                Label {
                    text: i18nc("@info", "This user will be an administrator.")
                }

                Kirigami.ContextualHelpButton {
                    toolTipText: xi18nc("@info", "This user will have administrative privileges on the system.<nl/><nl/>This means that they can change system settings, install software, and access all files on the system.<nl/><nl/>Choose a strong password for this user.")
                }
            }

            // Padding around the text
            Item {
                Layout.bottomMargin: Kirigami.Units.gridUnit
            }

            Kirigami.FormLayout {
                TextField {
                    id: fullNameField

                    Kirigami.FormData.label: i18nc("@label:textbox", "Full Name") // qmllint disable unqualified
                    property string previousText: ''
                    onTextChanged: {
                        if (usernameField.text.length === 0 || usernameField.text === previousText) {
                            usernameField.text = previousText = fullNameField.text.toLowerCase().replace(/\s/g, '');
                        }
                    }

                    Binding {
                        target: AccountController
                        property: 'fullName'
                        value: fullNameField.text
                    }
                }

                TextField {
                    id: usernameField

                    Kirigami.FormData.label: i18nc("@label:textbox", "Username") // qmllint disable unqualified

                    Binding {
                        target: AccountController
                        property: 'username'
                        value: usernameField.text
                    }
                }

                Kirigami.PasswordField {
                    id: paswordField

                    Kirigami.FormData.label: i18nc("@label:textbox", "Password") // qmllint disable unqualified

                    onTextChanged: {
                        showPasswordQuality = text.length > 0;
                    }
                }

                Kirigami.PasswordField {
                    id: repeatField

                    Kirigami.FormData.label: i18nc("@label:textbox", "Confirm Password") // qmllint disable unqualified

                    function setPasswordMatchError() {
                        repeatField.statusMessage = i18nc("@info:status", "Passwords don’t match"); // qmllint disable unqualified
                        repeatField.status = Kirigami.MessageType.Error;
                    }

                    function clearPasswordMatchError() {
                        repeatField.statusMessage = '';
                        repeatField.status = Kirigami.MessageType.Information;
                    }

                    // Timer delays validation while the user is typing.
                    Timer {
                        id: validationTimer
                        interval: Kirigami.Units.humanMoment
                        running: false
                        repeat: false

                        onTriggered: {
                            if (repeatField.text.length > 0 && repeatField.text !== paswordField.text) {
                                repeatField.setPasswordMatchError();
                            } else {
                                repeatField.clearPasswordMatchError();
                            }
                        }
                    }

                    // Reset timer when the user types.
                    onTextChanged: {
                        // If passwords match, immediately clear the error message.
                        if (text.length > 0 && text === paswordField.text) {
                            repeatField.clearPasswordMatchError();
                        } else {
                            validationTimer.restart();
                        }
                    }

                    onEditingFinished: function () {
                        if (text.length < 1 || text !== paswordField.text) {
                            repeatField.setPasswordMatchError();
                        } else {
                            repeatField.clearPasswordMatchError();
                            AccountController.password = text;
                            return;
                        }
                    }
                }
            }
        }
    }
}
