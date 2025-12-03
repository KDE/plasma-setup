// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.plasmasetup
import org.kde.plasmasetup.components as PlasmaSetupComponents

PlasmaSetupComponents.SetupModule {
    id: root

    available: !AccountController.hasExistingUsers

    /*!
    Whether the entered username is valid.
    */
    property bool usernameValid: AccountController.isUsernameValid(usernameField.text)

    /*!
    Message describing why the username is invalid, or empty if valid.
    */
    property string usernameValidationMessage: AccountController.usernameValidationMessage(usernameField.text)

    nextEnabled: root.usernameValid && passwordField.text.length > 0 && repeatField.text === passwordField.text

    contentItem: ScrollView {
        anchors.centerIn: parent
        width: Math.min(mainColumn.implicitWidth + (Kirigami.Units.gridUnit * 2), parent.width)
        height: Math.min(mainColumn.implicitHeight, parent.height)

        ColumnLayout {
            id: mainColumn

            width: root.cardWidth

            KirigamiComponents.AvatarButton {
                id: avatar

                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                // Square button
                implicitWidth: Kirigami.Units.gridUnit * 5
                implicitHeight: implicitWidth

                name: fullNameField.text

                // Disabled so it doesn't act like a button, instead we use it
                // as a simple avatar with user initial for now.
                //
                // TODO: Implement avatar selection
                enabled: false
            }

            // Padding around the text
            Item {
                Layout.topMargin: Kirigami.Units.gridUnit
            }

            Label {
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                text: i18n("We need a few details to complete the setup.") // qmllint disable unqualified

                Layout.leftMargin: Kirigami.Units.gridUnit
                Layout.rightMargin: Kirigami.Units.gridUnit
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter

                Label {
                    text: i18nc("@info", "This user will be an administrator.") // qmllint disable unqualified
                }

                Kirigami.ContextualHelpButton {
                    toolTipText: xi18nc("@info", "This user will have administrative privileges on the system.<nl/><nl/>This means that they can change system settings, install software, and access all files on the system.<nl/><nl/>Choose a strong password for this user.") // qmllint disable unqualified
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

                Kirigami.InlineMessage {
                    Layout.fillWidth: true
                    type: Kirigami.MessageType.Error
                    text: root.usernameValidationMessage
                    visible: root.usernameValidationMessage.length > 0 && usernameField.text.length > 0
                }

                Kirigami.PasswordField {
                    id: passwordField

                    Kirigami.FormData.label: i18nc("@label:textbox", "Password") // qmllint disable unqualified
                    placeholderText: "" // Form label already indicates this
                    onTextChanged: debouncer.reset()

                    Binding {
                        target: AccountController
                        property: 'password'
                        value: passwordField.text
                    }
                }

                Kirigami.PasswordField {
                    id: repeatField

                    Kirigami.FormData.label: i18nc("@label:textbox", "Confirm Password") // qmllint disable unqualified
                    placeholderText: "" // Form label already indicates this
                    onTextChanged: debouncer.reset()

                    onEditingFinished: {
                        debouncer.isTriggered = true;
                    }
                }

                Debouncer {
                    id: debouncer
                }

                Kirigami.InlineMessage {
                    id: passwordWarning

                    Layout.fillWidth: true
                    type: Kirigami.MessageType.Error
                    text: i18n("Passwords must match") // qmllint disable unqualified
                    visible: passwordField.text !== "" && repeatField.text !== "" && passwordField.text !== repeatField.text && debouncer.isTriggered
                }
            }
        }
    }
}
