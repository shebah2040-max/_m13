// SPDX-License-Identifier: GPL-3.0-or-later
//
// Attitude Director Indicator — simple sky/ground ball with roll and pitch.
// For Operations HUD. Pitch in degrees positive nose-up, roll in degrees
// positive right-wing-down.

import QtQuick 2.15
import QGroundControl.ScreenTools 1.0

Item {
    id: root
    property real pitchDeg: 0
    property real rollDeg:  0
    property real pixelsPerDeg: ScreenTools.defaultFontPixelHeight * 0.4

    implicitWidth:  ScreenTools.defaultFontPixelWidth * 20
    implicitHeight: ScreenTools.defaultFontPixelWidth * 20

    Rectangle {
        anchors.fill: parent
        color: "#060A10"
        border.color: "#152030"
        radius: width / 2
        clip: true

        Rectangle {
            id: horizon
            anchors.centerIn: parent
            width:  parent.width  * 1.5
            height: parent.height * 1.5
            rotation: -root.rollDeg
            transformOrigin: Item.Center

            Rectangle {
                width: parent.width
                height: parent.height / 2 + root.pitchDeg * root.pixelsPerDeg
                color: "#2B6CB0"  // sky
            }
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: parent.height / 2 - root.pitchDeg * root.pixelsPerDeg
                color: "#724C1A"  // ground
            }

            // Horizon line.
            Rectangle {
                anchors.left:  parent.left
                anchors.right: parent.right
                height: 2
                color: "#FFFFFF"
                y: parent.height / 2 + root.pitchDeg * root.pixelsPerDeg - 1
            }
        }

        // Aircraft reference symbol.
        Rectangle {
            anchors.centerIn: parent
            width:  ScreenTools.defaultFontPixelWidth * 8
            height: 2
            color: "#FFB300"
        }
        Rectangle {
            anchors.centerIn: parent
            width:  2
            height: ScreenTools.defaultFontPixelWidth * 4
            color: "#FFB300"
        }

        // Roll scale markers at ±10°, ±20°, ±30°, ±45°, ±60°.
        Repeater {
            model: [-60, -45, -30, -20, -10, 10, 20, 30, 45, 60]
            Rectangle {
                x: parent.width / 2 +
                   (parent.width / 2 - 4) * Math.sin(modelData * Math.PI / 180)
                y: parent.height / 2 -
                   (parent.height / 2 - 4) * Math.cos(modelData * Math.PI / 180)
                width: 2
                height: 6
                color: "#5B8DB8"
            }
        }
    }
}
