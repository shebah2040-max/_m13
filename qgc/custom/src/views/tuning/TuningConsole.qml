// SPDX-License-Identifier: GPL-3.0-or-later
//
// Tuning Console — bound to m130Tuning (TuningController). Shows each MPC
// weight with its hard bounds and current value; an edit dialog routes
// writes through TuningController::setParameter(), which enforces the
// mission-phase + step-up safety gate. A rejected write surfaces as
// "safety gated" in the status row and requires StepUpDialog first.

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
    anchors.fill: parent

    readonly property QGCPalette qgcPal: QGCPalette { colorGroupEnabled: true }
    readonly property real       m:      ScreenTools.defaultFontPixelWidth

    signal stepUpRequested()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: m * 2
        spacing: ScreenTools.defaultFontPixelHeight

        QGCLabel {
            text: qsTr("Tuning Console")
            font.pointSize: ScreenTools.largeFontPointSize
        }

        RowLayout {
            Layout.fillWidth: true
            QGCLabel { text: qsTr("MPC weights"); font.bold: true; Layout.fillWidth: true }

            QGCLabel {
                text: m130Access && !m130Access.stepUpRequired
                      ? qsTr("Step-up fresh")
                      : qsTr("Step-up required")
                color: m130Access && !m130Access.stepUpRequired ? "#00FF87" : "#FFB300"
            }

            QGCButton {
                text: qsTr("Re-authenticate")
                enabled: m130Access && m130Access.loggedIn
                onClicked: root.stepUpRequested()
            }

            QGCButton {
                text: qsTr("Reset defaults")
                onClicked: m130Tuning.resetToDefaults()
            }
        }

        // Header.
        RowLayout {
            Layout.fillWidth: true
            spacing: m
            QGCLabel { text: qsTr("Parameter"); color: "#5B8DB8"; Layout.preferredWidth: m * 22 }
            QGCLabel { text: qsTr("Value");     color: "#5B8DB8"; Layout.preferredWidth: m * 10 }
            QGCLabel { text: qsTr("Range");     color: "#5B8DB8"; Layout.preferredWidth: m * 20 }
            QGCLabel { text: qsTr("Unit");      color: "#5B8DB8"; Layout.preferredWidth: m * 8  }
            QGCLabel { text: qsTr("Action");    color: "#5B8DB8"; Layout.fillWidth: true }
        }

        ListView {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            clip: true
            model: m130Tuning ? m130Tuning.parameters : null
            spacing: 2

            delegate: Rectangle {
                width:  ListView.view.width
                height: ScreenTools.defaultFontPixelHeight * 2.0
                color:  index % 2 === 0 ? "#0A1220" : "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: m * 0.5
                    spacing: m

                    QGCLabel { text: paramName; Layout.preferredWidth: m * 22 }
                    QGCLabel { text: value.toFixed(4); Layout.preferredWidth: m * 10; font.bold: true }
                    QGCLabel { text: minValue.toFixed(2) + " … " + maxValue.toFixed(2); color: "#5B8DB8"; Layout.preferredWidth: m * 20 }
                    QGCLabel { text: unit; color: "#5B8DB8"; Layout.preferredWidth: m * 8 }
                    QGCTextField {
                        id: edit
                        Layout.preferredWidth: m * 10
                        placeholderText: qsTr("new value")
                        validator: DoubleValidator { bottom: minValue; top: maxValue }
                    }
                    QGCButton {
                        text: qsTr("Set")
                        enabled: edit.text.length > 0
                        onClicked: {
                            const rc = m130Tuning.setParameter(paramName, parseFloat(edit.text));
                            if (rc !== 0) {
                                status.text = qsTr("Rejected: ") + m130Tuning.lastError;
                            } else {
                                status.text = qsTr("Applied");
                                edit.text = "";
                            }
                        }
                    }
                }
            }
        }

        QGCLabel {
            id: status
            text: ""
            color: "#FFB300"
        }
    }
}
