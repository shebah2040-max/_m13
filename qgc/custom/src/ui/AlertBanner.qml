// SPDX-License-Identifier: GPL-3.0-or-later
//
// Alert Banner — scrollable list of active alerts ordered by severity.
// Bound to `m130SafetyKernel.activeAlerts` (an AlertListModel registered as
// a QML context property in CustomPlugin::createQmlApplicationEngine).
// Each row is an Alert with roles: alertId, level, levelText, title,
// detail, raisedMs, ackedMs, ackUser.

import QtQuick          2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts  1.15

import QGroundControl.ScreenTools   1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controls      1.0

Rectangle {
    id: root
    color: qgcPal.window
    border.color: qgcPal.text
    border.width: 1
    radius: ScreenTools.defaultFontPixelHeight * 0.2

    readonly property QGCPalette qgcPal: QGCPalette { colorGroupEnabled: true }
    property var kernel: (typeof m130SafetyKernel !== "undefined") ? m130SafetyKernel : null
    property var alerts: kernel ? kernel.activeAlerts : null
    property string ackUser: "operator"

    implicitHeight: ScreenTools.defaultFontPixelHeight * 6

    ListView {
        id: list
        anchors.fill: parent
        anchors.margins: ScreenTools.defaultFontPixelWidth
        model: root.alerts
        spacing: ScreenTools.defaultFontPixelHeight * 0.2
        clip: true

        delegate: Rectangle {
            width: list.width
            height: label.implicitHeight + ScreenTools.defaultFontPixelHeight * 0.4
            radius: 2
            color: {
                switch (level) {
                case 1:  return "#404040"  // Advisory
                case 2:  return "#ffb000"  // Caution
                case 3:  return "#ff6000"  // Warning
                case 4:  return "#c00000"  // Emergency
                }
                return qgcPal.button
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: ScreenTools.defaultFontPixelWidth * 0.5

                QGCLabel {
                    id: label
                    text: title + " — " + detail
                    color: level >= 3 ? "white" : qgcPal.text
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                QGCLabel {
                    text: levelText
                    color: level >= 3 ? "white" : qgcPal.text
                    font.bold: true
                }
                QGCButton {
                    text: qsTr("Ack")
                    enabled: root.kernel !== null
                    onClicked: if (root.kernel) root.kernel.acknowledge(alertId, root.ackUser)
                }
            }
        }
    }

    QGCLabel {
        anchors.centerIn: parent
        visible: (!root.alerts) || (root.alerts.rowCount !== undefined
                                     ? root.alerts.rowCount() === 0
                                     : root.alerts.length === 0)
        text: qsTr("No active alerts")
        opacity: 0.5
    }
}
