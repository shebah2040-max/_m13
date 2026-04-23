// SPDX-License-Identifier: GPL-3.0-or-later
//
// Range Safety Console (skeleton).
// Pillar 5 adds IIP visualisation, flight corridor overlay, and the
// dual-auth FTS panel wired to FlightTerminationService via a C++ bridge.

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
            text: qsTr("Range Safety Console — Foundation skeleton")
            font.pointSize: ScreenTools.largeFontPointSize
        }

        QGCLabel {
            text: qsTr("IIP, corridor overlay and FTS dual-auth panel are wired in Pillar 5.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
