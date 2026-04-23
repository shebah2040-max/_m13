// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pre-Launch Console (skeleton).
// Pillar 5 fills in checklist cells, weather panel, countdown timer.

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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: ScreenTools.defaultFontPixelWidth * 2
        spacing:          ScreenTools.defaultFontPixelHeight

        QGCLabel {
            text: qsTr("Pre-Launch Console — Foundation skeleton")
            font.pointSize: ScreenTools.largeFontPointSize
        }

        QGCLabel {
            text: qsTr("Checklist, weather, countdown and GO/NO-GO poll are wired in Pillar 5.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
