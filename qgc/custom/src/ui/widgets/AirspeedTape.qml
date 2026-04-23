// SPDX-License-Identifier: GPL-3.0-or-later
//
// Vertical airspeed tape (m/s). Parameterised similarly to AltitudeTape so
// the two can sit on the Operations HUD symmetrically.

import QtQuick 2.15
import QGroundControl.ScreenTools 1.0

Item {
    id: root
    property real airspeedMs: 0
    property real rangeMs:    40
    property real minorStep:  2
    property real majorStep:  10

    readonly property real _fh: ScreenTools.defaultFontPixelHeight
    readonly property real _fw: ScreenTools.defaultFontPixelWidth

    implicitWidth:  _fw * 10
    implicitHeight: _fh * 14

    Rectangle {
        anchors.fill: parent
        color: "#060A10"
        border.color: "#152030"
    }

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
            const mPerPx = (2 * rangeMs) / h
            const startV = airspeedMs - rangeMs
            const endV   = airspeedMs + rangeMs
            const firstMinor = Math.ceil(startV / minorStep) * minorStep
            for (let v = firstMinor; v <= endV; v += minorStep) {
                const y = cy - (v - airspeedMs) / mPerPx
                const isMajor = (Math.round(v / majorStep) * majorStep) === Math.round(v)
                ctx.lineWidth = isMajor ? 2 : 1
                const tickLen = isMajor ? width * 0.45 : width * 0.25
                ctx.beginPath()
                ctx.moveTo(0, y)
                ctx.lineTo(tickLen, y)
                ctx.stroke()
                if (isMajor) ctx.fillText(v.toString(), width - _fw * 4, y + _fh * 0.35)
            }
        }
        onVisibleChanged: if (visible) requestPaint()
    }

    Rectangle {
        anchors.verticalCenter: parent.verticalCenter
        width:  parent.width
        height: _fh * 1.6
        color:  "#0F2438"
        border.color: "#00D4AA"
        Text {
            anchors.centerIn: parent
            text:  root.airspeedMs.toFixed(1) + " m/s"
            color: "#FFFFFF"
            font.bold: true
            font.pointSize: ScreenTools.defaultFontPointSize
        }
    }

    onAirspeedMsChanged: ticks.requestPaint()
    onRangeMsChanged:    ticks.requestPaint()
}
