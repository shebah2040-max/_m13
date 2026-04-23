// SPDX-License-Identifier: GPL-3.0-or-later
//
// Weather + NOTAM badge — small overlay widget for Operations / PreLaunch
// that shows live wind at the launch point + worst active NOTAM severity.

import QtQuick          2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts  1.15

import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Palette       1.0

Rectangle {
    id: root

    readonly property QGCPalette qgcPal: QGCPalette { colorGroupEnabled: true }
    property QtObject controller: m130Weather

    // Launch site coordinates set by the parent console.
    property real   padLat: 0.0
    property real   padLon: 0.0
    property var    weatherSample: ({ valid: false })
    property int    worstSeverity: 0

    color: qgcPal.windowShade
    border.color: severityColor(worstSeverity)
    border.width: 1
    implicitWidth:  ScreenTools.defaultFontPixelWidth  * 30
    implicitHeight: ScreenTools.defaultFontPixelHeight * 5

    function severityColor(sev) {
        if (sev >= 3) return qgcPal.colorRed;
        if (sev >= 2) return qgcPal.colorOrange;
        if (sev >= 1) return qgcPal.colorYellow;
        return qgcPal.windowShadeDark;
    }

    function refresh() {
        if (!controller) return;
        weatherSample = controller.sampleWeather(padLat, padLon);
        worstSeverity = controller.worstNotamSeverity(
            padLat, padLon, Date.now());
    }

    Connections {
        target: root.controller
        function onWeatherChanged() { root.refresh() }
        function onNotamChanged()   { root.refresh() }
    }
    Component.onCompleted: refresh()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: ScreenTools.defaultFontPixelWidth
        spacing: ScreenTools.defaultFontPixelHeight / 2

        QGCLabel {
            text: weatherSample.valid
                  ? qsTr("Wind %1 m/s from %2°")
                        .arg(Math.sqrt(weatherSample.u_east_mps * weatherSample.u_east_mps +
                                       weatherSample.v_north_mps * weatherSample.v_north_mps)
                                  .toFixed(1))
                        .arg(((Math.atan2(-weatherSample.u_east_mps,
                                          -weatherSample.v_north_mps) * 180 / Math.PI) + 360) % 360
                                  .toFixed(0))
                  : qsTr("Wind: n/a")
        }

        QGCLabel {
            color: severityColor(worstSeverity)
            text: {
                switch (worstSeverity) {
                case 3: return qsTr("NOTAM HAZARD");
                case 2: return qsTr("NOTAM warning");
                case 1: return qsTr("NOTAM advisory");
                default: return qsTr("No active NOTAM");
                }
            }
        }
    }
}
