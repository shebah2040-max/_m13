// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pre-Launch Console — interactive checklist, countdown, weather readout,
// and GO / NO-GO roll-call. Data-binding targets:
//   - m130Checklist      (ChecklistModel QObject wrapper, landed in follow-up PR)
//   - m130Countdown      (CountdownClock QObject wrapper, ditto)
//   - m130WeatherService (Pillar 9 integration)
// Until the QObject wrappers land, this console renders a stable layout with
// illustrative static data so operators can review the UX.

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

    // Illustrative placeholder model — replaced by m130Checklist ListModel binding
    // once ChecklistQmlModel wrapper lands.
    ListModel {
        id: demoChecklist
        ListElement { title: "Range clear of personnel";    status: "done";    requiredRole: "Observer";       completedBy: "rso_alice"  }
        ListElement { title: "Weather within envelope";     status: "done";    requiredRole: "FlightDirector"; completedBy: "fd_bob"     }
        ListElement { title: "Fuel load ≥ 95 %";            status: "done";    requiredRole: "Operator";       completedBy: "ops_carol"  }
        ListElement { title: "Telemetry link PASS";         status: "done";    requiredRole: "Operator";       completedBy: "ops_carol"  }
        ListElement { title: "FTS self-test PASS";          status: "done";    requiredRole: "SafetyOfficer";  completedBy: "so_dana"    }
        ListElement { title: "GPS fix 3D, HDOP < 2.0";      status: "pending"; requiredRole: "Operator";       completedBy: ""           }
        ListElement { title: "MPC cold start converged";    status: "pending"; requiredRole: "Operator";       completedBy: ""           }
        ListElement { title: "Final GO / NO-GO poll";       status: "pending"; requiredRole: "FlightDirector"; completedBy: ""           }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: m * 2
        spacing:        ScreenTools.defaultFontPixelHeight

        // ── Header ───────────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: m * 2

            QGCLabel {
                text: qsTr("Pre-Launch Console")
                font.pointSize: ScreenTools.largeFontPointSize
                Layout.fillWidth: true
            }

            // Countdown banner.
            Rectangle {
                Layout.preferredWidth:  m * 22
                Layout.preferredHeight: ScreenTools.defaultFontPixelHeight * 2.5
                color:  "#0F2438"
                border.color: "#5B8DB8"
                radius: 4

                QGCLabel {
                    anchors.centerIn: parent
                    text:            qsTr("T-00:00:00")
                    font.pointSize:  ScreenTools.largeFontPointSize
                    color:           "#FFB300"
                }
            }

            QGCButton { text: qsTr("HOLD") }
            QGCButton { text: qsTr("RESUME") }
            QGCButton { text: qsTr("ABORT"); primary: true }
        }

        // ── Body: checklist on left, weather + go/no-go on right ─────────
        RowLayout {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            spacing: m * 2

            // Checklist panel.
            Rectangle {
                Layout.fillWidth:  true
                Layout.fillHeight: true
                color: "transparent"
                border.color: qgcPal.windowShadeDark
                radius: 4

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: m
                    spacing: m * 0.5

                    QGCLabel {
                        text: qsTr("Launch Checklist")
                        font.bold: true
                    }

                    ListView {
                        id: checklistView
                        Layout.fillWidth:  true
                        Layout.fillHeight: true
                        clip: true
                        model: demoChecklist
                        spacing: 2
                        delegate: Rectangle {
                            width:  ListView.view.width
                            height: ScreenTools.defaultFontPixelHeight * 2.2
                            color:  index % 2 === 0 ? "#0A1220" : "transparent"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: m * 0.5
                                spacing: m

                                Rectangle {
                                    Layout.preferredWidth:  m * 1.2
                                    Layout.preferredHeight: m * 1.2
                                    radius: m * 0.6
                                    color:  status === "done"    ? "#00FF87"
                                          : status === "pending" ? "#FFB300"
                                          : "#FF1744"
                                }
                                QGCLabel {
                                    text: title
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                                QGCLabel {
                                    text: requiredRole
                                    color: "#5B8DB8"
                                    font.pointSize: ScreenTools.smallFontPointSize
                                }
                                QGCLabel {
                                    text: completedBy === "" ? "—" : completedBy
                                    color: "#FFFFFF"
                                    font.pointSize: ScreenTools.smallFontPointSize
                                    Layout.preferredWidth: m * 14
                                    elide: Text.ElideRight
                                }
                                QGCButton {
                                    text: qsTr("MARK DONE")
                                    enabled: status === "pending"
                                    // hook: m130Checklist.markDone(id, currentUser, currentRole)
                                }
                            }
                        }
                    }
                }
            }

            // Right column: weather + go/no-go.
            ColumnLayout {
                Layout.preferredWidth: m * 28
                Layout.fillHeight:     true
                spacing: m

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: ScreenTools.defaultFontPixelHeight * 9
                    color: "transparent"
                    border.color: qgcPal.windowShadeDark
                    radius: 4

                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: m
                        columns: 2
                        rowSpacing:    m * 0.25
                        columnSpacing: m

                        QGCLabel { text: qsTr("Weather / METAR"); font.bold: true; Layout.columnSpan: 2 }

                        QGCLabel { text: qsTr("Wind"); color: "#5B8DB8" }
                        QGCLabel { text: "270° @ 6 kt" }

                        QGCLabel { text: qsTr("Gust"); color: "#5B8DB8" }
                        QGCLabel { text: "10 kt" }

                        QGCLabel { text: qsTr("Visibility"); color: "#5B8DB8" }
                        QGCLabel { text: "10 km" }

                        QGCLabel { text: qsTr("Ceiling"); color: "#5B8DB8" }
                        QGCLabel { text: "OVC 3500 ft" }

                        QGCLabel { text: qsTr("Temp / Dew"); color: "#5B8DB8" }
                        QGCLabel { text: "17 °C / 12 °C" }
                    }
                }

                Rectangle {
                    Layout.fillWidth:  true
                    Layout.fillHeight: true
                    color: "transparent"
                    border.color: qgcPal.windowShadeDark
                    radius: 4

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: m
                        spacing: m * 0.5

                        QGCLabel { text: qsTr("GO / NO-GO Poll"); font.bold: true }

                        Repeater {
                            model: [ "Range Safety", "Weather", "Vehicle", "Telemetry", "MPC / MHE", "Flight Director" ]
                            Row {
                                spacing: m
                                Rectangle {
                                    width: m * 1.2; height: m * 1.2
                                    radius: m * 0.6
                                    color: "#00FF87"
                                }
                                QGCLabel { text: modelData }
                            }
                        }

                        Item { Layout.fillHeight: true }

                        QGCButton {
                            text: qsTr("ARM FOR LAUNCH")
                            primary: true
                            Layout.alignment: Qt.AlignHCenter
                            enabled: false  // enabled by C++ when checklist.isReadyForLaunch()
                        }
                    }
                }
            }
        }
    }
}
