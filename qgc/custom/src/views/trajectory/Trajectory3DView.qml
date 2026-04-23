// SPDX-License-Identifier: GPL-3.0-or-later
//
// 3D trajectory view — draws the flown ENU track and IIP point using a
// simple orthographic top-down canvas projection. Qt Quick 3D / Cesium
// is planned for Phase 8b; the 2D canvas here is intentionally
// lightweight so range safety can read a live top-down of the flight at
// a glance.

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
    border.color: qgcPal.windowShadeDark
    border.width: 1

    readonly property QGCPalette qgcPal: QGCPalette { colorGroupEnabled: true }

    // Bound externally: m130Trajectory controller.
    property QtObject controller: m130Trajectory

    // Auto-fit padding (px around the track bounding box).
    property real padding: ScreenTools.defaultFontPixelHeight

    Connections {
        target: root.controller
        function onTrackChanged() { canvas.requestPaint() }
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d");
            ctx.reset();

            if (!root.controller || !root.controller.track ||
                root.controller.track.count === 0) {
                ctx.fillStyle = qgcPal.windowShade;
                ctx.fillRect(0, 0, width, height);
                ctx.fillStyle = qgcPal.text;
                ctx.font = "14px sans-serif";
                ctx.fillText(qsTr("No trajectory samples"),
                             width / 2 - 80, height / 2);
                return;
            }

            // Scan for bounds.
            const track = root.controller.track;
            let minE = Number.POSITIVE_INFINITY, maxE = Number.NEGATIVE_INFINITY;
            let minN = Number.POSITIVE_INFINITY, maxN = Number.NEGATIVE_INFINITY;
            for (let i = 0; i < track.count; i++) {
                const idx = track.index(i, 0);
                const e = track.data(idx, Qt.UserRole + 1);
                const n = track.data(idx, Qt.UserRole + 2);
                if (e < minE) minE = e;
                if (e > maxE) maxE = e;
                if (n < minN) minN = n;
                if (n > maxN) maxN = n;
            }
            if (maxE === minE) maxE = minE + 1;
            if (maxN === minN) maxN = minN + 1;

            const w = width  - 2 * padding;
            const h = height - 2 * padding;
            const sx = w / (maxE - minE);
            const sy = h / (maxN - minN);
            const s  = Math.min(sx, sy);

            // Background + cross-hair at origin.
            ctx.fillStyle = qgcPal.windowShade;
            ctx.fillRect(0, 0, width, height);

            const ox = padding - minE * s;
            const oy = height - padding + minN * s;

            ctx.strokeStyle = qgcPal.windowShadeDark;
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.moveTo(ox - 10, oy); ctx.lineTo(ox + 10, oy);
            ctx.moveTo(ox, oy - 10); ctx.lineTo(ox, oy + 10);
            ctx.stroke();

            // Draw track.
            ctx.strokeStyle = qgcPal.colorGreen;
            ctx.lineWidth = 2;
            ctx.beginPath();
            for (let j = 0; j < track.count; j++) {
                const idx = track.index(j, 0);
                const e = track.data(idx, Qt.UserRole + 1);
                const n = track.data(idx, Qt.UserRole + 2);
                const x = padding + (e - minE) * s;
                const y = height - padding - (n - minN) * s;
                if (j === 0) ctx.moveTo(x, y);
                else         ctx.lineTo(x, y);
            }
            ctx.stroke();

            // Draw latest position as a dot.
            if (track.count > 0) {
                const last = track.index(track.count - 1, 0);
                const e = track.data(last, Qt.UserRole + 1);
                const n = track.data(last, Qt.UserRole + 2);
                const x = padding + (e - minE) * s;
                const y = height - padding - (n - minN) * s;
                ctx.fillStyle = qgcPal.colorRed;
                ctx.beginPath();
                ctx.arc(x, y, 5, 0, Math.PI * 2);
                ctx.fill();
            }
        }
    }

    QGCLabel {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: ScreenTools.defaultFontPixelWidth
        text: controller && controller.sampleCount > 0
              ? qsTr("Trajectory · %1 samples").arg(controller.sampleCount)
              : qsTr("Trajectory · waiting for GNC")
        font.pointSize: ScreenTools.smallFontPointSize
    }
}
