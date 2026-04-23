// SPDX-License-Identifier: GPL-3.0-or-later
//
// Master Caution / Warning / Emergency lamp (REQ-M130-GCS-SAFE-004, UI-008).
//
// Bound to `masterAlertLevel` (0..4) which is driven by the C++ AlertManager
// exposed as a context property "m130AlertManager". Until the property is
// wired (Pillar 3), the binding defaults to None.

import QtQuick          2.15
import QtQuick.Controls 2.15

import QGroundControl.ScreenTools   1.0
import QGroundControl.Palette       1.0

Item {
    id: root
    readonly property int levelNone      : 0
    readonly property int levelAdvisory  : 1
    readonly property int levelCaution   : 2
    readonly property int levelWarning   : 3
    readonly property int levelEmergency : 4

    property int level: (typeof m130AlertManager !== "undefined" && m130AlertManager !== null)
                          ? m130AlertManager.masterLevel : levelNone
    property string text: qsTr("OK")

    readonly property QGCPalette qgcPal: QGCPalette { colorGroupEnabled: true }

    implicitWidth:  ScreenTools.defaultFontPixelWidth  * 16
    implicitHeight: ScreenTools.defaultFontPixelHeight * 2

    Rectangle {
        id:         lamp
        anchors.fill: parent
        radius:     ScreenTools.defaultFontPixelHeight * 0.3
        border.width: 1
        border.color: qgcPal.text

        color: {
            switch (root.level) {
            case root.levelNone:      return qgcPal.button
            case root.levelAdvisory:  return "#808080"
            case root.levelCaution:   return "#ffb000"
            case root.levelWarning:   return "#ff6000"
            case root.levelEmergency: return "#c00000"
            }
            return qgcPal.button
        }

        // Use a shape overlay (for colorblind users — HAZ-009 mitigation).
        Text {
            anchors.centerIn: parent
            font.family: ScreenTools.normalFontFamily
            font.pixelSize: ScreenTools.defaultFontPixelHeight
            color: root.level >= root.levelWarning ? "white" : qgcPal.text
            text: {
                switch (root.level) {
                case root.levelNone:      return qsTr("OK")
                case root.levelAdvisory:  return qsTr("◇ ADVISORY")
                case root.levelCaution:   return qsTr("△ CAUTION")
                case root.levelWarning:   return qsTr("▲ WARNING")
                case root.levelEmergency: return qsTr("■ EMERGENCY")
                }
                return ""
            }
        }
    }

    SequentialAnimation on opacity {
        running:  root.level === root.levelEmergency
        loops:    Animation.Infinite
        NumberAnimation { from: 1.0; to: 0.35; duration: 400 }
        NumberAnimation { from: 0.35; to: 1.0; duration: 400 }
    }
}
