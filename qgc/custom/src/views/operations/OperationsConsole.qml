// SPDX-License-Identifier: GPL-3.0-or-later
//
// Operations Console — primary flight display.
// Pillar 5 (Views Completion) replaces the body of this file with the full
// Attitude Director Indicator / Horizontal Situation Indicator / tape layout.
// Foundation keeps the existing FlyViewCustomLayer content accessible via
// Loader so the HUD continues to work while the split is staged.

import QtQuick          2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts  1.15

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Palette       1.0

Item {
    id: root
    anchors.fill: parent

    property var  vehicle:    QGroundControl.multiVehicleManager.activeVehicle
    readonly property real   m:          ScreenTools.defaultFontPixelWidth
    readonly property QGCPalette qgcPal: QGCPalette { colorGroupEnabled: true }

    Loader {
        id:        legacyHudLoader
        anchors.fill: parent
        source:    "qrc:/qml/FlyViewCustomLayer.qml"
        active:    true
        asynchronous: false
    }
}
