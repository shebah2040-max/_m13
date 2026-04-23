// SPDX-License-Identifier: GPL-3.0-or-later
//
// Analysis Console — post-flight plotting, field stats, FFT / spectrum.
// Pillar 5 ships the layout; live plotting integration lands with Pillar 7
// (Tuning & Analysis) which introduces the QtCharts binding.

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

    readonly property var _fields: [
        "altitude_m", "airspeed_m_s", "q_dyn_pa",
        "phi_rad", "theta_rad", "psi_rad",
        "alpha_est_rad", "pos_downrange_m", "pos_crossrange_m",
        "mpc_solve_us", "mhe_solve_us", "stage"
    ]

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: m * 2
        spacing: ScreenTools.defaultFontPixelHeight

        QGCLabel {
            text: qsTr("Analysis Console")
            font.pointSize: ScreenTools.largeFontPointSize
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: m

            QGCLabel { text: qsTr("Field:"); color: "#5B8DB8" }
            QGCComboBox {
                Layout.preferredWidth: m * 24
                model: _fields
            }
            QGCLabel { text: qsTr("From:"); color: "#5B8DB8" }
            QGCTextField { placeholderText: "T+00:00:00"; Layout.preferredWidth: m * 12 }
            QGCLabel { text: qsTr("To:"); color: "#5B8DB8" }
            QGCTextField { placeholderText: "T+00:30:00"; Layout.preferredWidth: m * 12 }
            Item { Layout.fillWidth: true }
            QGCButton { text: qsTr("Plot") }
            QGCButton { text: qsTr("FFT") }
            QGCButton { text: qsTr("Export CSV") }
        }

        Rectangle {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            color: "#0A1220"
            border.color: "#152030"
            radius: 4

            QGCLabel {
                anchors.centerIn: parent
                text: qsTr("Chart area — QtCharts integration lands with Pillar 7.")
                color: "#5B8DB8"
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: ScreenTools.defaultFontPixelHeight * 5
            color: "transparent"
            border.color: qgcPal.windowShadeDark
            radius: 4

            GridLayout {
                anchors.fill: parent
                anchors.margins: m
                columns: 6
                rowSpacing: m * 0.25
                columnSpacing: m * 2

                QGCLabel { text: qsTr("Samples"); color: "#5B8DB8" }
                QGCLabel { text: "—" }
                QGCLabel { text: qsTr("min");  color: "#5B8DB8" }
                QGCLabel { text: "—" }
                QGCLabel { text: qsTr("max");  color: "#5B8DB8" }
                QGCLabel { text: "—" }

                QGCLabel { text: qsTr("mean"); color: "#5B8DB8" }
                QGCLabel { text: "—" }
                QGCLabel { text: qsTr("σ");    color: "#5B8DB8" }
                QGCLabel { text: "—" }
                QGCLabel { text: qsTr("last"); color: "#5B8DB8" }
                QGCLabel { text: "—" }
            }
        }
    }
}
