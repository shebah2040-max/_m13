// SPDX-License-Identifier: GPL-3.0-or-later
//
// Login screen — username / password and optional TOTP follow-up. Routes
// through m130Access (AccessController). On `login()` success the panel
// emits `loggedIn`; on RequiresTotp it reveals the TOTP field and the
// caller can complete via `completeLogin()`.

import QtQuick          2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts  1.15

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Palette       1.0

Rectangle {
    id: root
    color: qgcPal.window

    readonly property QGCPalette qgcPal: QGCPalette { colorGroupEnabled: true }
    readonly property real m: ScreenTools.defaultFontPixelWidth

    property bool _requiresTotp: false

    signal loggedIn()

    Connections {
        target: m130Access
        function onSessionChanged() {
            if (m130Access.loggedIn) {
                userField.text = "";
                passField.text = "";
                totpField.text = "";
                root._requiresTotp = false;
                root.loggedIn();
            }
        }
        function onLoginFailed(reason) {
            if (reason === "RequiresTotp") {
                root._requiresTotp = true;
                totpField.forceActiveFocus();
            } else {
                root._requiresTotp = false;
            }
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: m * 60
        height: ScreenTools.defaultFontPixelHeight * 18
        color: qgcPal.windowShade
        border.color: qgcPal.windowShadeDark
        radius: 6

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: m * 2
            spacing: ScreenTools.defaultFontPixelHeight * 0.5

            QGCLabel {
                text: qsTr("M130 Ground Control — Sign in")
                font.pointSize: ScreenTools.largeFontPointSize
            }

            QGCLabel { text: qsTr("User"); color: "#5B8DB8" }
            QGCTextField {
                id: userField
                Layout.fillWidth: true
                placeholderText: qsTr("operator")
                enabled: !root._requiresTotp
            }

            QGCLabel { text: qsTr("Password"); color: "#5B8DB8" }
            QGCTextField {
                id: passField
                Layout.fillWidth: true
                echoMode: TextInput.Password
                enabled: !root._requiresTotp
                onAccepted: loginButton.clicked()
            }

            QGCLabel {
                text: qsTr("TOTP (6 digits)")
                color: "#5B8DB8"
                visible: root._requiresTotp
            }
            QGCTextField {
                id: totpField
                Layout.fillWidth: true
                visible: root._requiresTotp
                placeholderText: "000000"
                inputMethodHints: Qt.ImhDigitsOnly
                validator: RegularExpressionValidator { regularExpression: /\d{6,8}/ }
                onAccepted: totpButton.clicked()
            }

            RowLayout {
                Layout.fillWidth: true
                QGCLabel {
                    Layout.fillWidth: true
                    text: m130Access ? m130Access.lastError : ""
                    color: "#FF2D55"
                }
                QGCButton {
                    id: loginButton
                    text: qsTr("Sign in")
                    visible: !root._requiresTotp
                    enabled: userField.text.length > 0 && passField.text.length > 0
                    onClicked: m130Access.login(userField.text, passField.text)
                }
                QGCButton {
                    id: totpButton
                    text: qsTr("Verify")
                    visible: root._requiresTotp
                    enabled: totpField.text.length >= 6
                    onClicked: m130Access.completeLogin(totpField.text)
                }
            }
        }
    }
}
