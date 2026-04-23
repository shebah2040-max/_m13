// SPDX-License-Identifier: GPL-3.0-or-later
//
// Top-level console switcher. Shows a left-side sidebar with one button per
// console and a Loader that hosts the selected .qml. Consoles are kept in a
// list so additional consoles (Comms, Mission, Tactical…) can be added
// without touching the switcher body.

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

    property int _selected: 0

    readonly property var _consoles: [
        { name: qsTr("Operations"),   source: "qrc:/qml/views/operations/OperationsConsole.qml"  },
        { name: qsTr("Pre-Launch"),   source: "qrc:/qml/views/prelaunch/PreLaunchConsole.qml"    },
        { name: qsTr("Range Safety"), source: "qrc:/qml/views/rangesafety/RangeSafetyConsole.qml"},
        { name: qsTr("Replay"),       source: "qrc:/qml/views/replay/ReplayConsole.qml"          },
        { name: qsTr("Tuning"),       source: "qrc:/qml/views/tuning/TuningConsole.qml"          },
        { name: qsTr("Analysis"),     source: "qrc:/qml/views/analysis/AnalysisConsole.qml"      },
        { name: qsTr("Admin"),        source: "qrc:/qml/views/admin/AdminConsole.qml"            }
    ]

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Sidebar.
        Rectangle {
            Layout.preferredWidth: m * 18
            Layout.fillHeight:     true
            color: "#0A1220"
            border.color: "#152030"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: m
                spacing: m * 0.5

                QGCLabel {
                    text: qsTr("M130 GCS")
                    font.pointSize: ScreenTools.largeFontPointSize
                    font.bold: true
                    color: "#00D4AA"
                    Layout.alignment: Qt.AlignHCenter
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: "#152030" }

                Repeater {
                    model: _consoles
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: ScreenTools.defaultFontPixelHeight * 2.4
                        color: index === _selected ? "#00D4AA" : "transparent"
                        radius: 3

                        QGCLabel {
                            anchors.left: parent.left
                            anchors.leftMargin: m
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.name
                            color: index === _selected ? "#060A10" : "#FFFFFF"
                            font.bold: index === _selected
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: _selected = index
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // Content area.
        Loader {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            source: _consoles[_selected].source
            asynchronous: false
        }
    }
}
