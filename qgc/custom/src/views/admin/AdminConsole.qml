// SPDX-License-Identifier: GPL-3.0-or-later
//
// Admin Console — user management, audit-log viewer, and system-health
// indicators. Wired to UserManager + SessionManager + AuditLogger via the
// Qt bridge landed in a follow-up PR.

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

    // Illustrative placeholders — replaced by m130Users / m130Audit bindings.
    ListModel {
        id: demoUsers
        ListElement { username: "rso_alice"; role: "RangeSafety";    active: true  }
        ListElement { username: "so_dana";   role: "SafetyOfficer";  active: true  }
        ListElement { username: "fd_bob";    role: "FlightDirector"; active: true  }
        ListElement { username: "ops_carol"; role: "Operator";       active: true  }
        ListElement { username: "obs_eric";  role: "Observer";       active: false }
    }
    ListModel {
        id: demoAudit
        ListElement { ts: "12:04:01"; user: "rso_alice"; action: "checklist.mark_done";  detail: "range.clear" }
        ListElement { ts: "12:04:33"; user: "so_dana";   action: "fts.self_test";        detail: "PASS"        }
        ListElement { ts: "12:05:02"; user: "fd_bob";    action: "mission.request_arm";  detail: "accepted"    }
        ListElement { ts: "12:05:40"; user: "ops_carol"; action: "telemetry.open";       detail: "udp:14550"   }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: m * 2
        spacing: ScreenTools.defaultFontPixelHeight

        QGCLabel {
            text: qsTr("Admin Console")
            font.pointSize: ScreenTools.largeFontPointSize
        }

        RowLayout {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            spacing: m * 2

            // Users.
            Rectangle {
                Layout.preferredWidth: m * 36
                Layout.fillHeight: true
                color: "transparent"
                border.color: qgcPal.windowShadeDark
                radius: 4

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: m
                    spacing: m * 0.5

                    RowLayout {
                        Layout.fillWidth: true
                        QGCLabel { text: qsTr("Users"); font.bold: true; Layout.fillWidth: true }
                        QGCButton { text: qsTr("+ Add") }
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: demoUsers
                        spacing: 2
                        delegate: Rectangle {
                            width: ListView.view.width
                            height: ScreenTools.defaultFontPixelHeight * 2.0
                            color: index % 2 === 0 ? "#0A1220" : "transparent"
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: m * 0.5
                                spacing: m

                                Rectangle {
                                    width: m * 1.0; height: m * 1.0; radius: m * 0.5
                                    color: active ? "#00FF87" : "#5B8DB8"
                                }
                                QGCLabel { text: username; Layout.fillWidth: true }
                                QGCLabel { text: role; color: "#5B8DB8"; font.pointSize: ScreenTools.smallFontPointSize }
                                QGCButton { text: qsTr("Edit") }
                            }
                        }
                    }
                }
            }

            // Audit log.
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

                    RowLayout {
                        Layout.fillWidth: true
                        QGCLabel { text: qsTr("Audit log (signed, append-only)"); font.bold: true; Layout.fillWidth: true }
                        QGCButton { text: qsTr("Export…") }
                        QGCButton { text: qsTr("Verify chain") }
                    }

                    ListView {
                        Layout.fillWidth:  true
                        Layout.fillHeight: true
                        clip: true
                        model: demoAudit
                        spacing: 2
                        delegate: Rectangle {
                            width: ListView.view.width
                            height: ScreenTools.defaultFontPixelHeight * 1.8
                            color: index % 2 === 0 ? "#0A1220" : "transparent"
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: m * 0.5
                                spacing: m
                                QGCLabel { text: ts; color: "#5B8DB8"; Layout.preferredWidth: m * 8 }
                                QGCLabel { text: user; Layout.preferredWidth: m * 12 }
                                QGCLabel { text: action; color: "#00E5FF"; Layout.preferredWidth: m * 20 }
                                QGCLabel { text: detail; Layout.fillWidth: true; elide: Text.ElideRight }
                            }
                        }
                    }
                }
            }
        }
    }
}
