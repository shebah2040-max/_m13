// SPDX-License-Identifier: GPL-3.0-or-later
//
// Tuning Console — live MPC / MHE parameter view. Pillar 5 ships the table
// and a read-only parameter browser; live editing with safety bounds lands
// with Pillar 7 (uses Vehicle.parameterManager once the firmware side
// publishes the M130 parameter set).

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

    ListModel {
        id: demoParams
        ListElement { name: "MPC_W_ALT";        value: 1.0;    min: 0.0; max: 10.0;  unit: "—";   live: false }
        ListElement { name: "MPC_W_AIRSPEED";   value: 0.5;    min: 0.0; max: 10.0;  unit: "—";   live: false }
        ListElement { name: "MPC_W_ATTITUDE";   value: 2.0;    min: 0.0; max: 10.0;  unit: "—";   live: false }
        ListElement { name: "MPC_R_FIN";        value: 0.1;    min: 0.0; max: 1.0;   unit: "—";   live: false }
        ListElement { name: "MHE_Q_POS";        value: 0.25;   min: 0.0; max: 10.0;  unit: "m²";  live: false }
        ListElement { name: "MHE_Q_VEL";        value: 0.10;   min: 0.0; max: 10.0;  unit: "m²/s²"; live: false }
        ListElement { name: "ENV_Q_MAX";        value: 80000;  min: 0;   max: 150000; unit: "Pa"; live: false }
        ListElement { name: "ENV_ALT_MAX";      value: 30000;  min: 0;   max: 50000;  unit: "m";  live: false }
    }

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
            QGCLabel { text: qsTr("Parameters"); font.bold: true; Layout.fillWidth: true }
            QGCButton { text: qsTr("Snapshot") }
            QGCButton { text: qsTr("Rollback…") }
            QGCButton { text: qsTr("Export…") }
        }

        // Header.
        RowLayout {
            Layout.fillWidth: true
            spacing: m
            QGCLabel { text: qsTr("Name");  color: "#5B8DB8"; Layout.preferredWidth: m * 22 }
            QGCLabel { text: qsTr("Value"); color: "#5B8DB8"; Layout.preferredWidth: m * 10 }
            QGCLabel { text: qsTr("Range"); color: "#5B8DB8"; Layout.preferredWidth: m * 16 }
            QGCLabel { text: qsTr("Unit");  color: "#5B8DB8"; Layout.preferredWidth: m * 8  }
            QGCLabel { text: qsTr("Editable"); color: "#5B8DB8"; Layout.fillWidth: true }
        }

        ListView {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            clip: true
            model: demoParams
            spacing: 2
            delegate: Rectangle {
                width: ListView.view.width
                height: ScreenTools.defaultFontPixelHeight * 1.8
                color: index % 2 === 0 ? "#0A1220" : "transparent"
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: m * 0.5
                    spacing: m
                    QGCLabel { text: name;  Layout.preferredWidth: m * 22 }
                    QGCLabel { text: value.toString(); Layout.preferredWidth: m * 10; font.bold: true }
                    QGCLabel { text: min + " … " + max; color: "#5B8DB8"; Layout.preferredWidth: m * 16 }
                    QGCLabel { text: unit; color: "#5B8DB8"; Layout.preferredWidth: m * 8 }
                    QGCLabel {
                        text: live ? qsTr("LIVE") : qsTr("read-only")
                        color: live ? "#00FF87" : "#FFB300"
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
