// SPDX-License-Identifier: GPL-3.0-or-later
//
// Admin Console — bound to m130Access (AccessController). Left column shows
// the currently authenticated operator plus session state; right column is
// the live audit-log viewer backed by AuditTailModel.

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

            // Current session.
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

                    QGCLabel { text: qsTr("Session"); font.bold: true }

                    GridLayout {
                        columns: 2
                        rowSpacing: m * 0.25
                        columnSpacing: m * 2

                        QGCLabel { text: qsTr("status");   color: "#5B8DB8" }
                        QGCLabel {
                            text: m130Access && m130Access.loggedIn ? qsTr("Logged in") : qsTr("Logged out")
                            color: m130Access && m130Access.loggedIn ? "#00FF87" : "#FFB300"
                        }
                        QGCLabel { text: qsTr("user");     color: "#5B8DB8" }
                        QGCLabel { text: m130Access ? m130Access.currentUser : "" }
                        QGCLabel { text: qsTr("role");     color: "#5B8DB8" }
                        QGCLabel { text: m130Access ? m130Access.currentRole : "" }
                        QGCLabel { text: qsTr("session");  color: "#5B8DB8" }
                        QGCLabel {
                            text: m130Access && m130Access.sessionId ? m130Access.sessionId.substring(0, 8) : "—"
                            font.family: "monospace"
                        }
                        QGCLabel { text: qsTr("step-up");  color: "#5B8DB8" }
                        QGCLabel {
                            text: m130Access && !m130Access.stepUpRequired ? qsTr("fresh") : qsTr("required")
                            color: m130Access && !m130Access.stepUpRequired ? "#00FF87" : "#FFB300"
                        }
                        QGCLabel { text: qsTr("lastError"); color: "#5B8DB8" }
                        QGCLabel { text: m130Access ? m130Access.lastError : ""; color: "#FF2D55" }
                    }

                    Item { Layout.fillHeight: true }

                    RowLayout {
                        Layout.fillWidth: true
                        QGCButton {
                            text: qsTr("Logout")
                            enabled: m130Access && m130Access.loggedIn
                            onClicked: m130Access.logout()
                        }
                    }
                }
            }

            // Audit log viewer — live.
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
                        QGCLabel { text: qsTr("Audit log (live, in-memory tail)"); font.bold: true; Layout.fillWidth: true }
                        QGCButton {
                            text: qsTr("Clear")
                            onClicked: if (m130Access && m130Access.auditTail) m130Access.auditTail.clear()
                        }
                    }

                    ListView {
                        Layout.fillWidth:  true
                        Layout.fillHeight: true
                        clip: true
                        model: m130Access ? m130Access.auditTail : null
                        spacing: 2
                        verticalLayoutDirection: ListView.BottomToTop

                        delegate: Rectangle {
                            width:  ListView.view.width
                            height: ScreenTools.defaultFontPixelHeight * 1.8
                            color:  index % 2 === 0 ? "#0A1220" : "transparent"
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: m * 0.5
                                spacing: m
                                QGCLabel { text: ts;     color: "#5B8DB8"; Layout.preferredWidth: m * 8 }
                                QGCLabel { text: user;   Layout.preferredWidth: m * 12 }
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
