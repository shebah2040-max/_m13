// SPDX-License-Identifier: GPL-3.0-or-later
//
// Replay Console — plays back a `.m130raw` session file through
// ReplayController (Pillar 4 + Pillar 5 views). Controls: open file,
// play / pause / rewind, speed slider (0.1× .. 100×), and a live frame
// counter. The Qt wrapper publishes:
//   m130Replay.state   ("Empty" | "Paused" | "Playing" | "Finished")
//   m130Replay.speed   (real, clamped)
//   m130Replay.frames  (qulonglong)
//   m130Replay.openFile(path) / play() / pause() / rewind() / setSpeed(x)

import QtQuick          2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts  1.15
import QtQuick.Dialogs  6.5

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

    property string _path:   ""
    property string _state:  "Empty"
    property real   _speed:  1.0
    property int    _frames: 0

    FileDialog {
        id: chooseFile
        nameFilters: [ "M130 FDR raw (*.m130raw)", "All files (*)" ]
        onAccepted: {
            _path  = selectedFile
            _state = "Paused"
            _frames = 0
            // Hook: m130Replay.openFile(selectedFile)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: m * 2
        spacing: ScreenTools.defaultFontPixelHeight

        QGCLabel {
            text: qsTr("Replay Console")
            font.pointSize: ScreenTools.largeFontPointSize
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: m

            QGCButton { text: qsTr("Open…"); onClicked: chooseFile.open() }

            QGCLabel {
                text: _path === "" ? qsTr("No file loaded") : _path
                Layout.fillWidth: true
                elide: Text.ElideMiddle
                color: "#5B8DB8"
            }

            Rectangle {
                Layout.preferredWidth:  m * 10
                Layout.preferredHeight: ScreenTools.defaultFontPixelHeight * 1.6
                radius: 4
                color:  _state === "Playing"  ? "#00FF87"
                      : _state === "Paused"   ? "#FFB300"
                      : _state === "Finished" ? "#5B8DB8"
                      : "#1E3550"
                QGCLabel {
                    anchors.centerIn: parent
                    text: _state.toUpperCase()
                    color: "#060A10"
                    font.bold: true
                }
            }
        }

        // ── Transport ──────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: ScreenTools.defaultFontPixelHeight * 6
            color: "transparent"
            border.color: qgcPal.windowShadeDark
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.margins: m
                spacing: m * 2

                QGCButton {
                    text: qsTr("◀◀  Rewind")
                    enabled: _state !== "Empty"
                    onClicked: { _state = "Paused"; _frames = 0 /* m130Replay.rewind() */ }
                }
                QGCButton {
                    text: _state === "Playing" ? qsTr("⏸  Pause") : qsTr("▶  Play")
                    primary: true
                    enabled: _state !== "Empty" && _state !== "Finished"
                    onClicked: {
                        _state = _state === "Playing" ? "Paused" : "Playing"
                        // Hook: _state === "Playing" ? m130Replay.play() : m130Replay.pause()
                    }
                }

                QGCLabel { text: qsTr("Speed"); color: "#5B8DB8" }

                Slider {
                    id: speedSlider
                    Layout.fillWidth: true
                    from: 0.1
                    to:   100.0
                    value: _speed
                    stepSize: 0.1
                    onMoved: { _speed = value /* m130Replay.setSpeed(value) */ }
                }

                QGCLabel {
                    text: _speed.toFixed(1) + "×"
                    font.bold: true
                    Layout.preferredWidth: m * 6
                }
            }
        }

        // ── Telemetry readout ──────────────────────────────────────────
        Rectangle {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            color: "transparent"
            border.color: qgcPal.windowShadeDark
            radius: 4

            GridLayout {
                anchors.fill: parent
                anchors.margins: m
                columns: 2
                columnSpacing: m * 4
                rowSpacing:    m * 0.5

                QGCLabel { text: qsTr("Frames emitted"); color: "#5B8DB8" }
                QGCLabel { text: _frames.toString(); font.bold: true }

                QGCLabel { text: qsTr("State"); color: "#5B8DB8" }
                QGCLabel { text: _state }

                QGCLabel { text: qsTr("Speed"); color: "#5B8DB8" }
                QGCLabel { text: _speed.toFixed(2) + "×" }

                QGCLabel { text: qsTr("File"); color: "#5B8DB8" }
                QGCLabel { text: _path === "" ? "—" : _path; elide: Text.ElideMiddle; Layout.fillWidth: true }

                Item { Layout.columnSpan: 2; Layout.fillHeight: true }
            }
        }
    }
}
