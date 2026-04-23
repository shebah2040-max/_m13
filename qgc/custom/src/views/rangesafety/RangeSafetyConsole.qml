// SPDX-License-Identifier: GPL-3.0-or-later
//
// Range Safety Console — Instantaneous Impact Point read-out, flight corridor
// status, and the dual-authorisation FTS panel. Lives behind RBAC — only
// users with Role >= RangeSafety may arm the commit button (enforced by the
// Qt wrapper around FlightTerminationService in CustomPlugin).

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

    // Placeholder — Qt wrapper will expose:
    //   m130Iip.downrangeM, m130Iip.crossrangeM, m130Iip.timeToImpactS, m130Iip.inCorridor
    property real _iipDown:     1420.0
    property real _iipCross:     215.0
    property real _iipTti:        12.4
    property bool _inCorridor:   true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: m * 2
        spacing: ScreenTools.defaultFontPixelHeight

        QGCLabel {
            text: qsTr("Range Safety Console")
            font.pointSize: ScreenTools.largeFontPointSize
        }

        RowLayout {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            spacing: m * 2

            // ── IIP + corridor panel ─────────────────────────────────────
            Rectangle {
                Layout.fillWidth:  true
                Layout.fillHeight: true
                color: "transparent"
                border.color: qgcPal.windowShadeDark
                radius: 4

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: m
                    spacing: m

                    QGCLabel { text: qsTr("Instantaneous Impact Point"); font.bold: true }

                    GridLayout {
                        columns: 2
                        columnSpacing: m * 2
                        rowSpacing:    m * 0.5

                        QGCLabel { text: qsTr("Downrange");   color: "#5B8DB8" }
                        QGCLabel { text: _iipDown.toFixed(0)  + " m" }

                        QGCLabel { text: qsTr("Crossrange"); color: "#5B8DB8" }
                        QGCLabel { text: _iipCross.toFixed(0) + " m" }

                        QGCLabel { text: qsTr("Time to impact"); color: "#5B8DB8" }
                        QGCLabel { text: _iipTti.toFixed(1) + " s" }

                        QGCLabel { text: qsTr("Corridor"); color: "#5B8DB8" }
                        QGCLabel {
                            text:  _inCorridor ? qsTr("WITHIN") : qsTr("OUT OF BOUNDS")
                            color: _inCorridor ? "#00FF87" : "#FF1744"
                            font.bold: true
                        }
                    }

                    // Corridor visualisation placeholder — Pillar 5b will
                    // overlay the actual polygon + live IIP dot.
                    Rectangle {
                        Layout.fillWidth:  true
                        Layout.fillHeight: true
                        color: "#0A1220"
                        border.color: "#152030"

                        QGCLabel {
                            anchors.centerIn: parent
                            text: qsTr("Corridor map — wired to MapView in Pillar 5b")
                            color: "#5B8DB8"
                        }
                    }
                }
            }

            // ── FTS dual-auth panel ──────────────────────────────────────
            Rectangle {
                Layout.preferredWidth: m * 28
                Layout.fillHeight: true
                color: "#220A0A"
                border.color: "#FF1744"
                border.width: 2
                radius: 6

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: m
                    spacing: m

                    QGCLabel {
                        text: qsTr("FLIGHT TERMINATION")
                        font.pointSize: ScreenTools.largeFontPointSize
                        color: "#FF1744"
                        font.bold: true
                    }

                    QGCLabel {
                        text: qsTr("Requires dual authorisation (RSO + Safety Officer) per REQ-SAFE-009.")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        color: "#FFB300"
                    }

                    GridLayout {
                        columns: 2
                        columnSpacing: m
                        rowSpacing:    m * 0.5
                        Layout.fillWidth: true

                        QGCLabel { text: qsTr("RSO user") }
                        QGCTextField { placeholderText: qsTr("rso_alice"); Layout.fillWidth: true }

                        QGCLabel { text: qsTr("RSO TOTP") }
                        QGCTextField { echoMode: TextInput.Password; Layout.fillWidth: true }

                        QGCLabel { text: qsTr("Safety user") }
                        QGCTextField { placeholderText: qsTr("so_dana"); Layout.fillWidth: true }

                        QGCLabel { text: qsTr("Safety TOTP") }
                        QGCTextField { echoMode: TextInput.Password; Layout.fillWidth: true }

                        QGCLabel { text: qsTr("Reason") }
                        QGCComboBox {
                            Layout.fillWidth: true
                            model: [ "Manual RSO", "Envelope violation", "Loss of control",
                                     "Link loss", "IIP out of corridor", "Battery critical" ]
                        }
                    }

                    Item { Layout.fillHeight: true }

                    QGCButton {
                        Layout.fillWidth: true
                        text: qsTr("ARM & FIRE FTS")
                        primary: true
                        // Hook: m130Fts.armAndFire({ primary_user, primary_totp, secondary_user, secondary_totp, reason })
                        enabled: false
                    }

                    QGCLabel {
                        text: qsTr("Attempt: 0  •  Armed: 0")
                        color: "#5B8DB8"
                        font.pointSize: ScreenTools.smallFontPointSize
                    }
                }
            }
        }
    }
}
