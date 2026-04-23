// SPDX-License-Identifier: GPL-3.0-or-later
//
// Step-up re-authentication dialog — requested by any console that tries
// to perform a critical operation (FTS, MPC weight change, admin action)
// when AccessController::stepUpRequired is true.

import QtQuick          2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts  1.15

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Palette       1.0

Dialog {
    id: root
    modal: true
    title: qsTr("Step-up re-authentication")
    standardButtons: Dialog.NoButton

    readonly property real m: ScreenTools.defaultFontPixelWidth

    signal confirmed()

    onOpened: totp.forceActiveFocus()
    onClosed: totp.text = ""

    ColumnLayout {
        spacing: ScreenTools.defaultFontPixelHeight * 0.5
        width: m * 40

        QGCLabel {
            text: qsTr("This operation requires fresh credentials.\nEnter your TOTP code to continue.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        QGCTextField {
            id: totp
            Layout.fillWidth: true
            placeholderText: "000000"
            inputMethodHints: Qt.ImhDigitsOnly
            validator: RegularExpressionValidator { regularExpression: /\d{6,8}/ }
            onAccepted: okButton.clicked()
        }

        QGCLabel {
            Layout.fillWidth: true
            text: m130Access ? m130Access.lastError : ""
            color: "#FF2D55"
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            QGCButton {
                text: qsTr("Cancel")
                onClicked: root.close()
            }
            QGCButton {
                id: okButton
                text: qsTr("Verify")
                enabled: totp.text.length >= 6
                onClicked: {
                    if (m130Access.stepUp(totp.text)) {
                        root.confirmed();
                        root.close();
                    }
                }
            }
        }
    }
}
