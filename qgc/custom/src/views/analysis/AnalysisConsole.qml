// SPDX-License-Identifier: GPL-3.0-or-later
//
// Analysis Console — bound to m130Analysis (AnalysisController). Left side
// shows the active timeseries via a lightweight Canvas (no QtCharts
// dependency); right side shows the MHE innovation health card plus a
// spectrum plot of the active series.

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

    readonly property var seriesList: [
        "phi", "theta", "psi", "q_dyn",
        "altitude", "airspeed", "alpha_est", "mpc_solve_us"
    ]

    Connections {
        target: m130Analysis
        function onSeriesUpdated(name) {
            if (name === m130Analysis.activeSeries) {
                tsCanvas.requestPaint();
            }
        }
        function onActiveSeriesChanged() {
            tsCanvas.requestPaint();
            spectrumCanvas.requestPaint();
        }
    }

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

            QGCLabel { text: qsTr("Series:"); color: "#5B8DB8" }
            QGCComboBox {
                Layout.preferredWidth: m * 24
                model: root.seriesList
                currentIndex: Math.max(0, root.seriesList.indexOf(m130Analysis ? m130Analysis.activeSeries : ""))
                onActivated: if (m130Analysis) m130Analysis.activeSeries = root.seriesList[currentIndex]
            }

            QGCLabel { text: qsTr("Rate (Hz):"); color: "#5B8DB8" }
            QGCTextField {
                Layout.preferredWidth: m * 8
                text: m130Analysis ? m130Analysis.sampleRateHz.toFixed(1) : "20.0"
                validator: DoubleValidator { bottom: 1; top: 1000 }
                onEditingFinished: if (m130Analysis) m130Analysis.sampleRateHz = parseFloat(text)
            }

            Item { Layout.fillWidth: true }

            QGCButton {
                text: qsTr("Recompute spectrum")
                onClicked: spectrumCanvas.requestPaint()
            }
            QGCButton {
                text: qsTr("Clear")
                onClicked: m130Analysis.clearAll()
            }
        }

        RowLayout {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            spacing: m

            // Timeseries canvas.
            Rectangle {
                Layout.fillWidth:  true
                Layout.fillHeight: true
                color: "#0A1220"
                border.color: "#152030"
                radius: 4

                Canvas {
                    id: tsCanvas
                    anchors.fill: parent
                    anchors.margins: 2
                    onPaint: {
                        const ctx = getContext("2d");
                        ctx.reset();
                        ctx.fillStyle = "#0A1220";
                        ctx.fillRect(0, 0, width, height);
                        if (!m130Analysis) return;
                        const pts = m130Analysis.downsample(Math.floor(width));
                        if (pts.length < 2) {
                            ctx.fillStyle = "#5B8DB8";
                            ctx.fillText(qsTr("Waiting for data..."), 20, 20);
                            return;
                        }
                        let minV = pts[0].value, maxV = pts[0].value;
                        let minT = pts[0].tMs,   maxT = pts[0].tMs;
                        for (let i = 0; i < pts.length; ++i) {
                            if (pts[i].value < minV) minV = pts[i].value;
                            if (pts[i].value > maxV) maxV = pts[i].value;
                            if (pts[i].tMs   < minT) minT = pts[i].tMs;
                            if (pts[i].tMs   > maxT) maxT = pts[i].tMs;
                        }
                        const dv = (maxV - minV) || 1;
                        const dt = (maxT - minT) || 1;
                        ctx.strokeStyle = "#00D4AA";
                        ctx.lineWidth = 1.2;
                        ctx.beginPath();
                        for (let i = 0; i < pts.length; ++i) {
                            const x = ((pts[i].tMs - minT) / dt) * width;
                            const y = height - ((pts[i].value - minV) / dv) * height;
                            if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
                        }
                        ctx.stroke();
                        ctx.fillStyle = "#5B8DB8";
                        ctx.fillText(maxV.toFixed(3), 4, 12);
                        ctx.fillText(minV.toFixed(3), 4, height - 4);
                    }
                }
            }

            // MHE innovation health + spectrum.
            ColumnLayout {
                Layout.preferredWidth: m * 40
                Layout.fillHeight: true
                spacing: m

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: ScreenTools.defaultFontPixelHeight * 6
                    color: "transparent"
                    border.color: qgcPal.windowShadeDark
                    radius: 4

                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: m
                        columns: 2
                        rowSpacing: m * 0.25
                        columnSpacing: m * 2

                        QGCLabel { text: qsTr("MHE innovation");  font.bold: true; Layout.columnSpan: 2 }
                        QGCLabel { text: qsTr("samples");      color: "#5B8DB8" }
                        QGCLabel { text: m130Analysis ? m130Analysis.mheSamples.toString() : "—" }
                        QGCLabel { text: qsTr("mean");         color: "#5B8DB8" }
                        QGCLabel {
                            text: m130Analysis ? m130Analysis.mheRunningMean.toFixed(3) : "—"
                            color: m130Analysis && m130Analysis.mheMeanBias ? "#FF2D55" : "#E8EAF0"
                        }
                        QGCLabel { text: qsTr("variance");     color: "#5B8DB8" }
                        QGCLabel {
                            text: m130Analysis ? m130Analysis.mheRunningVar.toFixed(3) : "—"
                            color: m130Analysis && m130Analysis.mheVarianceOut ? "#FF2D55" : "#E8EAF0"
                        }
                        QGCLabel { text: qsTr("3-σ excursion"); color: "#5B8DB8" }
                        QGCLabel {
                            text: m130Analysis && m130Analysis.mheThreeSigma ? qsTr("TRIPPED") : qsTr("OK")
                            color: m130Analysis && m130Analysis.mheThreeSigma ? "#FF2D55" : "#00FF87"
                        }
                        QGCLabel { text: qsTr("reason"); color: "#5B8DB8" }
                        QGCLabel { text: m130Analysis ? m130Analysis.mheReason : ""; Layout.fillWidth: true; elide: Text.ElideRight }
                    }
                }

                Rectangle {
                    Layout.fillWidth:  true
                    Layout.fillHeight: true
                    color: "#0A1220"
                    border.color: "#152030"
                    radius: 4

                    Canvas {
                        id: spectrumCanvas
                        anchors.fill: parent
                        anchors.margins: 2
                        onPaint: {
                            const ctx = getContext("2d");
                            ctx.reset();
                            ctx.fillStyle = "#0A1220";
                            ctx.fillRect(0, 0, width, height);
                            if (!m130Analysis) return;
                            const sp = m130Analysis.currentSpectrum();
                            if (sp.length < 2) {
                                ctx.fillStyle = "#5B8DB8";
                                ctx.fillText(qsTr("Not enough samples"), 20, 20);
                                return;
                            }
                            let maxMag = 0.0;
                            for (let i = 0; i < sp.length; ++i) {
                                if (sp[i].mag > maxMag) maxMag = sp[i].mag;
                            }
                            if (maxMag <= 0) maxMag = 1.0;
                            const fmax = sp[sp.length - 1].fHz;
                            ctx.strokeStyle = "#FFB300";
                            ctx.lineWidth = 1.0;
                            ctx.beginPath();
                            for (let i = 0; i < sp.length; ++i) {
                                const x = (sp[i].fHz / (fmax || 1)) * width;
                                const y = height - (sp[i].mag / maxMag) * height;
                                if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
                            }
                            ctx.stroke();
                            const peak = m130Analysis.currentSpectrumPeak();
                            if (peak && peak.fHz !== undefined) {
                                ctx.fillStyle = "#E8EAF0";
                                ctx.fillText(qsTr("peak ") + peak.fHz.toFixed(2) + " Hz", 4, 12);
                            }
                        }
                    }
                }
            }
        }
    }
}
