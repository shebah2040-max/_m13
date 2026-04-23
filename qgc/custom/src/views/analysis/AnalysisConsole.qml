// SPDX-License-Identifier: GPL-3.0-or-later
//
// Analysis Console (skeleton). Pillar 7 adds live plotting, FFT, cost
// breakdown and MHE innovation monitoring.

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
            text: qsTr("Analysis Console — Foundation skeleton")
            font.pointSize: ScreenTools.largeFontPointSize
        }
        QGCLabel {
            text: qsTr("Live plotting, FFT, cost breakdown, innovation monitoring land in Pillar 7.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
