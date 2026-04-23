// SPDX-License-Identifier: GPL-3.0-or-later
//
// Map corridor status overlay. Binds to m130MapController; displayed
// inside RangeSafetyConsole and PreLaunchConsole.

import QtQuick          2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts  1.15

import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Palette       1.0

Rectangle {
    id: root
    color: qgcPal.windowShade
    border.width: 1

    readonly property QGCPalette qgcPal: QGCPalette { colorGroupEnabled: true }
    property QtObject controller: m130MapController

    // State enum mirrored from nav::CorridorState.
    readonly property int stateUnknown: 0
    readonly property int stateInside:  1
    readonly property int stateWarning: 2
    readonly property int stateBreach:  3

    border.color: {
        if (!controller) return qgcPal.windowShadeDark;
        switch (controller.state) {
        case stateBreach:  return qgcPal.colorRed;
        case stateWarning: return qgcPal.colorOrange;
        case stateInside:  return qgcPal.colorGreen;
        default:           return qgcPal.windowShadeDark;
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: ScreenTools.defaultFontPixelWidth
        spacing: ScreenTools.defaultFontPixelWidth

        QGCLabel {
            Layout.fillWidth: true
            text: {
                if (!root.controller) return qsTr("Corridor: —");
                switch (root.controller.state) {
                case stateBreach:  return qsTr("CORRIDOR BREACH");
                case stateWarning: return qsTr("Corridor warning");
                case stateInside:  return qsTr("Corridor nominal");
                default:           return qsTr("Corridor: no data");
                }
            }
            font.bold: root.controller && root.controller.state >= stateWarning
            font.pointSize: ScreenTools.mediumFontPointSize
        }

        QGCLabel {
            text: root.controller
                  ? qsTr("Margin: %1 m").arg(Math.round(root.controller.marginMeters))
                  : qsTr("Margin: —")
        }
    }
}
