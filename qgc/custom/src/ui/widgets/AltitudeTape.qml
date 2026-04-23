// SPDX-License-Identifier: GPL-3.0-or-later
//
// Vertical altitude tape. Shows ±N metres around a centred reading with
// tick marks every 10 m (minor) and 50 m (major). Reusable between the
// Operations and Replay consoles.

import QtQuick          2.15
import QtQuick.Controls 2.15

import QGroundControl.ScreenTools 1.0

Item {
    id: root

    property real altitudeM:    0
    property real rangeM:       200
    property real minorStepM:   10
    property real majorStepM:   50

    readonly property real _fh: ScreenTools.defaultFontPixelHeight
    readonly property real _fw: ScreenTools.defaultFontPixelWidth

    implicitWidth:  _fw * 10
    implicitHeight: _fh * 14

    Rectangle {
        anchors.fill: parent
        color: "#060A10"
        border.color: "#152030"
    }

    // Tick painter.
    Canvas {
        id: ticks
        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            ctx.strokeStyle = "#5B8DB8"
            ctx.fillStyle   = "#FFFFFF"
            ctx.font        = (ScreenTools.defaultFontPointSize - 1) + "pt sans-serif"
            const h    = height
            const cy   = h / 2
            const mPerPx = (2 * rangeM) / h
            const startM = altitudeM - rangeM
            const endM   = altitudeM + rangeM
            const firstMinor = Math.ceil(startM / minorStepM) * minorStepM
            for (let m = firstMinor; m <= endM; m += minorStepM) {
                const y = cy - (m - altitudeM) / mPerPx
                const isMajor = (Math.round(m / majorStepM) * majorStepM) === Math.round(m)
                ctx.lineWidth = isMajor ? 2 : 1
                const tickLen = isMajor ? width * 0.45 : width * 0.25
                ctx.beginPath()
                ctx.moveTo(width - tickLen, y)
                ctx.lineTo(width, y)
                ctx.stroke()
                if (isMajor) ctx.fillText(m.toString(), _fw, y + _fh * 0.35)
            }
        }
        onVisibleChanged: if (visible) requestPaint()
    }

    // Centre readout.
    Rectangle {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width:  parent.width
        height: _fh * 1.6
        color:  "#0F2438"
        border.color: "#00D4AA"

        Text {
            anchors.centerIn: parent
            text:  root.altitudeM.toFixed(0) + " m"
            color: "#FFFFFF"
            font.bold: true
            font.pointSize: ScreenTools.defaultFontPointSize
        }
    }

    onAltitudeMChanged: ticks.requestPaint()
    onRangeMChanged:    ticks.requestPaint()
}
