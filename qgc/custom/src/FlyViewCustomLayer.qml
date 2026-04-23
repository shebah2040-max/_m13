import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls

// ============================================================================
//  M130 GCS — Professional Aerospace HUD
//  Design Standard : Inspired by MIL-STD-1472G / RTCA DO-178C color model
//  Color Hierarchy : White=Primary Data  Cyan=Commanded  Green=Nominal
//                    Amber=Caution        Red=Warning      Dim=Inactive
// ============================================================================

Item {
    id: _root

    property var parentToolInsets
    property var totalToolInsets: _totalToolInsets
    property var mapControl

    // ── Vehicle ──────────────────────────────────────────────────────────────
    property var _vehicle: QGroundControl.multiVehicleManager.activeVehicle
    property var _rkt:     _vehicle ? _vehicle.getFactGroup("rocket") : null

    // ── Design Tokens (DO-178C compliant color model) ────────────────────────
    readonly property color _c_bg:       "#E6060A10"   // panel background
    readonly property color _c_bg_solid: "#060A10"
    readonly property color _c_border:   "#0F2438"     // panel border
    readonly property color _c_sep:      "#152030"     // separator lines

    // Primary data — always white
    readonly property color _c_white:    "#FFFFFF"
    // Labels — steel-blue, uppercase
    readonly property color _c_label:    "#5B8DB8"
    // Commanded / reference values — cyan
    readonly property color _c_cyan:     "#00E5FF"
    // Nominal — bright green
    readonly property color _c_green:    "#00FF87"
    // Caution — amber (FAA standard)
    readonly property color _c_amber:    "#FFB300"
    // Warning — red
    readonly property color _c_red:      "#FF1744"
    // Inactive / no-data
    readonly property color _c_dim:      "#1E3550"
    // Active accent (teal primary branding)
    readonly property color _c_teal:     "#00D4AA"

    // ── Typography scale ─────────────────────────────────────────────────────
    readonly property real _fs:    ScreenTools.defaultFontPointSize
    readonly property real _fw:    ScreenTools.defaultFontPixelWidth
    readonly property real _fh:    ScreenTools.defaultFontPixelHeight
    readonly property real _m:     _fw * 0.7         // base margin
    readonly property real _r:     _fw * 0.35        // border radius

    // ── Stage helpers ─────────────────────────────────────────────────────────
    function stageName(s) {
        switch (Math.round(s)) {
        case 0: return "IDLE"
        case 1: return "BOOST"
        case 2: return "CRUISE"
        case 3: return "TERMINAL"
        default: return "---"
        }
    }
    function stageColor(s) {
        switch (Math.round(s)) {
        case 1: return _c_amber
        case 2: return _c_cyan
        case 3: return _c_red
        default: return _c_dim
        }
    }
    // Value → color cascade  (green / amber / red)
    function valColor(v, warnLo, dangerLo, warnHi, dangerHi) {
        if (v === undefined || v === null) return _c_dim
        if (v <= dangerLo || v >= dangerHi) return _c_red
        if (v <= warnLo  || v >= warnHi)  return _c_amber
        return _c_green
    }
    function solverColor(ms, okBelow, warnBelow) {
        if (ms >= warnBelow) return _c_red
        if (ms >= okBelow)   return _c_amber
        return _c_green
    }

    // ── Tool Insets ───────────────────────────────────────────────────────────
    QGCToolInsets {
        id: _totalToolInsets
        leftEdgeTopInset:      _panelFlight.x + _panelFlight.width + _m
        leftEdgeCenterInset:   parentToolInsets.leftEdgeCenterInset
        leftEdgeBottomInset:   parentToolInsets.leftEdgeBottomInset
        rightEdgeTopInset:     _root.width - _panelGNC.x + _m
        rightEdgeCenterInset:  parentToolInsets.rightEdgeCenterInset
        rightEdgeBottomInset:  parentToolInsets.rightEdgeBottomInset
        topEdgeLeftInset:      parentToolInsets.topEdgeLeftInset
        topEdgeCenterInset:    _topBar.height + _m
        topEdgeRightInset:     parentToolInsets.topEdgeRightInset
        bottomEdgeLeftInset:   parentToolInsets.bottomEdgeLeftInset
        bottomEdgeCenterInset: _panelFin.visible ? (_root.height - _panelFin.y + _m) : parentToolInsets.bottomEdgeCenterInset
        bottomEdgeRightInset:  parentToolInsets.bottomEdgeRightInset
    }

    // =========================================================================
    //  TOP MISSION STATUS BAR
    // =========================================================================
    Rectangle {
        id: _topBar
        anchors.top:       parent.top
        anchors.left:      parent.left
        anchors.right:     parent.right
        anchors.topMargin: parentToolInsets.topEdgeCenterInset
        height:            _fh * 1.8
        color:             "#F0060A10"
        border.color:      _c_border
        border.width:      1

        Row {
            anchors.fill:    parent
            anchors.margins: _m
            spacing:         _fw * 2

            // ── Phase dot ─────────────────────────────────────────────────
            Rectangle {
                width:  _fh * 0.65; height: width; radius: width / 2
                anchors.verticalCenter: parent.verticalCenter
                color: _rkt ? stageColor(_rkt.stage.rawValue) : _c_dim
            }

            // ── Stage label ───────────────────────────────────────────────
            QGCLabel {
                text:  _rkt ? stageName(_rkt.stage.rawValue) : "OFFLINE"
                color: _rkt ? stageColor(_rkt.stage.rawValue) : _c_dim
                font.pointSize: _fs * 0.85
                font.bold:      true
                font.family:    ScreenTools.monoFontFamily
                font.letterSpacing: 1.5
                anchors.verticalCenter: parent.verticalCenter
            }

            // ── Divider ───────────────────────────────────────────────────
            Rectangle { width: 1; height: parent.height * 0.6; color: _c_sep; anchors.verticalCenter: parent.verticalCenter }

            // ── Mission timer ─────────────────────────────────────────────
            Row {
                spacing: _m * 0.4
                anchors.verticalCenter: parent.verticalCenter
                QGCLabel { text: "T+"; color: _c_label; font.pointSize: _fs * 0.75; font.letterSpacing: 1 }
                QGCLabel {
                    text:  _rkt ? _rkt.tFlight.rawValue.toFixed(2) + "s" : "---.--s"
                    color: _c_white
                    font.pointSize:  _fs * 0.9
                    font.bold:       true
                    font.family:     ScreenTools.monoFontFamily
                }
            }

            Rectangle { width: 1; height: parent.height * 0.6; color: _c_sep; anchors.verticalCenter: parent.verticalCenter }

            // ── Dynamic pressure ──────────────────────────────────────────
            Row {
                spacing: _m * 0.4
                anchors.verticalCenter: parent.verticalCenter
                QGCLabel { text: "q"; color: _c_label; font.pointSize: _fs * 0.75; font.italic: true }
                QGCLabel {
                    text:  _rkt ? _rkt.qDyn.rawValue.toFixed(0) + " Pa" : "---- Pa"
                    color: _rkt ? valColor(_rkt.qDyn.rawValue, 500, 100, 8000, 15000) : _c_dim
                    font.pointSize: _fs * 0.85
                    font.bold:      true
                    font.family:    ScreenTools.monoFontFamily
                }
            }

            Rectangle { width: 1; height: parent.height * 0.6; color: _c_sep; anchors.verticalCenter: parent.verticalCenter }

            // ── MPC solver status (critical alert) ────────────────────────
            Row {
                spacing: _m * 0.4
                anchors.verticalCenter: parent.verticalCenter
                visible: _rkt ? _rkt.mpcFailCount.rawValue > 0 : false
                Rectangle {
                    width: _fh * 0.55; height: width; radius: 2
                    color: _c_red
                    anchors.verticalCenter: parent.verticalCenter
                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.2; duration: 500 }
                        NumberAnimation { to: 1.0; duration: 500 }
                    }
                }
                QGCLabel {
                    text:  "MPC FAIL " + (_rkt ? Math.round(_rkt.mpcFailCount.rawValue) : 0)
                    color: _c_red
                    font.pointSize: _fs * 0.8
                    font.bold:      true
                    font.letterSpacing: 1
                }
            }

            // ── Fin clamp alert ───────────────────────────────────────────
            Row {
                spacing: _m * 0.4
                anchors.verticalCenter: parent.verticalCenter
                visible: _rkt ? _rkt.finClampCount.rawValue > 0 : false
                Rectangle {
                    width: _fh * 0.55; height: width; radius: 2
                    color: _c_amber
                    anchors.verticalCenter: parent.verticalCenter
                }
                QGCLabel {
                    text:  "FIN CLMP " + (_rkt ? Math.round(_rkt.finClampCount.rawValue) : 0)
                    color: _c_amber
                    font.pointSize: _fs * 0.8
                    font.bold:      true
                    font.letterSpacing: 1
                }
            }
        }
    }

    // =========================================================================
    //  LEFT PANEL — PRIMARY FLIGHT DATA
    // =========================================================================
    Rectangle {
        id: _panelFlight
        anchors.left:      parent.left
        anchors.top:       _topBar.bottom
        anchors.leftMargin: _m
        anchors.topMargin:  _m
        width:  _fw * 19
        color:  _c_bg
        radius: _r
        border.color: _c_border
        border.width: 1
        height: _colFlight.implicitHeight + _m * 2

        // ── Corner brackets ────────────────────────────────────────────────
        Canvas {
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.strokeStyle = _c_teal
                ctx.lineWidth = 1.5
                var s = 10
                // TL
                ctx.beginPath(); ctx.moveTo(s,0); ctx.lineTo(0,0); ctx.lineTo(0,s); ctx.stroke()
                // TR
                ctx.beginPath(); ctx.moveTo(width-s,0); ctx.lineTo(width,0); ctx.lineTo(width,s); ctx.stroke()
                // BL
                ctx.beginPath(); ctx.moveTo(0,height-s); ctx.lineTo(0,height); ctx.lineTo(s,height); ctx.stroke()
                // BR
                ctx.beginPath(); ctx.moveTo(width-s,height); ctx.lineTo(width,height); ctx.lineTo(width,height-s); ctx.stroke()
            }
        }

        Column {
            id: _colFlight
            anchors { left:parent.left; right:parent.right; top:parent.top; margins: _m }
            spacing: _fh * 0.18

            // ── Header ──────────────────────────────────────────────────
            QGCLabel {
                text: "PRIMARY FLIGHT"
                color: _c_label
                font.pointSize:  _fs * 0.7
                font.letterSpacing: 2
            }
            Rectangle { width: parent.width; height: 1; color: _c_sep }

            // ── Velocity (large) ─────────────────────────────────────────
            Column {
                width: parent.width
                spacing: 0
                QGCLabel { text: "AIRSPEED"; color: _c_label; font.pointSize: _fs * 0.65; font.letterSpacing: 1.5 }
                Row {
                    spacing: _m * 0.4
                    QGCLabel {
                        text: _rkt ? _rkt.airspeed.rawValue.toFixed(1) : "---.--"
                        color: _c_white
                        font.pointSize: _fs * 1.5
                        font.bold:      true
                        font.family:    ScreenTools.monoFontFamily
                    }
                    QGCLabel { text: "m/s"; color: _c_label; font.pointSize: _fs * 0.75; anchors.bottom: parent.bottom; bottomPadding: _m * 0.4 }
                }
                // Velocity bar
                Rectangle {
                    width: parent.width; height: 3; color: _c_dim; radius: 2
                    Rectangle {
                        width: {
                            if (!_rkt) return 0
                            return Math.min(1.0, _rkt.airspeed.rawValue / 600.0) * parent.width
                        }
                        height: parent.height; radius: 2
                        color: _rkt ? valColor(_rkt.airspeed.rawValue, 50, 10, 550, 600) : _c_dim
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: _c_sep }

            // ── Flight path angle ────────────────────────────────────────
            DataRow {
                label: "FLT PATH  γ"
                valueText: _rkt ? ((_rkt.gammaRad.rawValue * 180.0 / Math.PI) >= 0 ? "+" : "") +
                                  (_rkt.gammaRad.rawValue * 180.0 / Math.PI).toFixed(2) + "°" : "---.--°"
                valColor: _rkt ? valColor(Math.abs(_rkt.gammaRad.rawValue * 180.0 / Math.PI), 0, 0, 45, 85) : _c_dim
            }

            // ── Heading ──────────────────────────────────────────────────
            DataRow {
                label: "HEADING   χ"
                valueText: _rkt ? _rkt.bearingDeg.rawValue.toFixed(1) + "°" : "---.--°"
                valColor: _c_white
            }

            // ── Alpha ─────────────────────────────────────────────────────
            DataRow {
                label: "AoA       α"
                valueText: _rkt ? ((_rkt.alphaEst.rawValue * 180.0 / Math.PI) >= 0 ? "+" : "") +
                                  (_rkt.alphaEst.rawValue * 180.0 / Math.PI).toFixed(2) + "°" : "---.--°"
                valColor: _rkt ? valColor(Math.abs(_rkt.alphaEst.rawValue * 180.0 / Math.PI), 5, 0, 12, 18) : _c_dim
            }

            Rectangle { width: parent.width; height: 1; color: _c_sep }

            // ── Altitude ──────────────────────────────────────────────────
            DataRow {
                label: "ALTITUDE  h"
                valueText: _rkt ? _rkt.altitude.rawValue.toFixed(1) + " m" : "-----.-  m"
                valColor: _c_white
            }

            // ── Downrange ─────────────────────────────────────────────────
            DataRow {
                label: "DOWNRANGE"
                valueText: _rkt ? _rkt.posDownrange.rawValue.toFixed(0) + " m" : "-----  m"
                valColor: _c_white
            }

            // ── To target ─────────────────────────────────────────────────
            DataRow {
                label: "TO TARGET"
                valueText: _rkt ? _rkt.targetRangeRem.rawValue.toFixed(0) + " m" : "-----  m"
                valColor: _rkt ? (
                    _rkt.targetRangeRem.rawValue < 100 ? _c_red :
                    _rkt.targetRangeRem.rawValue < 300 ? _c_amber :
                    _c_cyan
                ) : _c_dim
                bold: true
            }

            // ── Crossrange ────────────────────────────────────────────────
            DataRow {
                label: "CROSSRANGE"
                valueText: _rkt ? ((_rkt.posCrossrange.rawValue >= 0 ? "+" : "") + _rkt.posCrossrange.rawValue.toFixed(1) + " m") : "-----  m"
                valColor: _rkt ? valColor(Math.abs(_rkt.posCrossrange.rawValue), 5, 0, 30, 60) : _c_dim
            }
        }
    }

    // =========================================================================
    //  RIGHT PANEL — GNC HEALTH MONITOR
    // =========================================================================
    Rectangle {
        id: _panelGNC
        anchors.right:      parent.right
        anchors.top:        _topBar.bottom
        anchors.rightMargin: _m
        anchors.topMargin:   _m
        width:  _fw * 22
        color:  _c_bg
        radius: _r
        border.color: _c_border
        border.width: 1
        height: _colGNC.implicitHeight + _m * 2

        Canvas {
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.strokeStyle = _c_teal
                ctx.lineWidth = 1.5
                var s = 10
                ctx.beginPath(); ctx.moveTo(s,0); ctx.lineTo(0,0); ctx.lineTo(0,s); ctx.stroke()
                ctx.beginPath(); ctx.moveTo(width-s,0); ctx.lineTo(width,0); ctx.lineTo(width,s); ctx.stroke()
                ctx.beginPath(); ctx.moveTo(0,height-s); ctx.lineTo(0,height); ctx.lineTo(s,height); ctx.stroke()
                ctx.beginPath(); ctx.moveTo(width-s,height); ctx.lineTo(width,height); ctx.lineTo(width,height-s); ctx.stroke()
            }
        }

        Column {
            id: _colGNC
            anchors { left:parent.left; right:parent.right; top:parent.top; margins: _m }
            spacing: _fh * 0.18

            // ── Header ───────────────────────────────────────────────────
            QGCLabel {
                text: "GNC HEALTH MONITOR"
                color: _c_label
                font.pointSize:  _fs * 0.7
                font.letterSpacing: 2
            }
            Rectangle { width: parent.width; height: 1; color: _c_sep }

            // ── MPC Solver ───────────────────────────────────────────────
            SolverRow {
                label:     "MPC SOLVER"
                valueMs:   _rkt ? _rkt.mpcSolveMs.rawValue : 0
                okBelow:   15
                warnBelow: 25
                status:    _rkt ? _rkt.mpcSolverStatus.rawValue : -1
                sqpIter:   _rkt ? _rkt.mpcSqpIter.rawValue : 0
            }

            // ── MHE Estimator ────────────────────────────────────────────
            SolverRow {
                label:     "MHE ESTIMATOR"
                valueMs:   _rkt ? _rkt.mheSolveMs.rawValue : 0
                okBelow:   20
                warnBelow: 35
                status:    _rkt ? _rkt.mheStatus.rawValue : -1
                sqpIter:   0
            }

            Rectangle { width: parent.width; height: 1; color: _c_sep }

            // ── Blend alpha ───────────────────────────────────────────────
            Column {
                width: parent.width
                spacing: _fh * 0.1
                Row {
                    width: parent.width
                    QGCLabel {
                        text:  "BLEND RATIO α"
                        color: _c_label
                        font.pointSize: _fs * 0.75
                        width: _fw * 11
                    }
                    QGCLabel {
                        text:  _rkt ? _rkt.blendAlpha.rawValue.toFixed(3) : "-.---"
                        color: _rkt ? (_rkt.blendAlpha.rawValue > 0.5 ? _c_green : _c_amber) : _c_dim
                        font.pointSize: _fs * 0.9
                        font.bold:      true
                        font.family:    ScreenTools.monoFontFamily
                    }
                }
                // Blend bar with threshold marker
                Item {
                    width: parent.width; height: _fh * 0.45
                    Rectangle { anchors.fill: parent; color: _c_dim; radius: 2 }
                    Rectangle {
                        width:  _rkt ? _rkt.blendAlpha.rawValue * parent.width : 0
                        height: parent.height; radius: 2
                        color:  _rkt ? (_rkt.blendAlpha.rawValue > 0.5 ? _c_green : _c_amber) : _c_dim
                    }
                    // Threshold marker at 0.5
                    Rectangle { x: parent.width * 0.5; width: 1; height: parent.height * 1.6; anchors.verticalCenter: parent.verticalCenter; color: _c_white; opacity: 0.4 }
                }
            }

            // ── MHE Quality ───────────────────────────────────────────────
            Row {
                width: parent.width
                QGCLabel {
                    text:  "MHE QUALITY"
                    color: _c_label
                    font.pointSize: _fs * 0.75
                    width: _fw * 11
                }
                QGCLabel {
                    text:  _rkt ? _rkt.mheQuality.rawValue.toFixed(4) : "--.----"
                    color: _rkt ? (_rkt.mheValid.rawValue > 0 ? _c_green : _c_dim) : _c_dim
                    font.pointSize: _fs * 0.9
                    font.bold:      true
                    font.family:    ScreenTools.monoFontFamily
                }
                QGCLabel {
                    text:  _rkt ? (_rkt.mheValid.rawValue > 0 ? "  VALID" : "  NO-DATA") : "  NO-DATA"
                    color: _rkt ? (_rkt.mheValid.rawValue > 0 ? _c_green : _c_dim) : _c_dim
                    font.pointSize: _fs * 0.7
                    font.letterSpacing: 1
                }
            }

            Rectangle { width: parent.width; height: 1; color: _c_sep }

            // ── GPS constellation ─────────────────────────────────────────
            Row {
                width: parent.width
                spacing: _m
                QGCLabel {
                    text:  "GPS"
                    color: _c_label
                    font.pointSize: _fs * 0.75
                    font.letterSpacing: 1
                    width: _fw * 4
                    anchors.verticalCenter: parent.verticalCenter
                }
                // Strength dots
                Row {
                    spacing: 3
                    anchors.verticalCenter: parent.verticalCenter
                    Repeater {
                        model: 5
                        Rectangle {
                            property int filled: _rkt ? Math.min(5, Math.round(_rkt.gpsSatellites.rawValue / 2.4)) : 0
                            width:  _fw * 1.2; height: _fh * (0.3 + index * 0.12)
                            anchors.bottom: parent.bottom
                            radius: 1
                            color:  index < filled ? _c_green : _c_dim
                        }
                    }
                }
                QGCLabel {
                    text:  _rkt ? Math.round(_rkt.gpsSatellites.rawValue) + " SAT" : "-- SAT"
                    color: _rkt ? (_rkt.gpsSatellites.rawValue >= 6 ? _c_green : _c_amber) : _c_dim
                    font.pointSize: _fs * 0.85
                    font.family:    ScreenTools.monoFontFamily
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // ── Servo health matrix ───────────────────────────────────────
            Row {
                width: parent.width
                spacing: _m
                QGCLabel {
                    text:  "SRV"
                    color: _c_label
                    font.pointSize: _fs * 0.75
                    font.letterSpacing: 1
                    width: _fw * 4
                    anchors.verticalCenter: parent.verticalCenter
                }
                Row {
                    spacing: _m * 0.6
                    anchors.verticalCenter: parent.verticalCenter
                    Repeater {
                        model: 4
                        Item {
                            width:  _fw * 2.8
                            height: _fh * 1.1
                            property int mask: _rkt ? Math.round(_rkt.servoOnlineMask.rawValue) : 0
                            property bool online: (mask & (1 << index)) !== 0
                            Rectangle {
                                anchors.fill: parent
                                radius: 2
                                color: online ? "#1A3D2A" : "#3D1A1A"
                                border.color: online ? _c_green : _c_red
                                border.width: 1
                            }
                            QGCLabel {
                                anchors.centerIn: parent
                                text:  "F" + (index+1)
                                color: parent.online ? _c_green : _c_red
                                font.pointSize: _fs * 0.7
                                font.bold:      true
                                font.family:    ScreenTools.monoFontFamily
                            }
                        }
                    }
                }
                QGCLabel {
                    property int mask: _rkt ? Math.round(_rkt.servoOnlineMask.rawValue) : 0
                    function popcnt(n) { var c=0; while(n){c+=n&1;n>>=1}; return c }
                    text:  popcnt(mask) + "/4"
                    color: mask === 15 ? _c_green : _c_red
                    font.pointSize: _fs * 0.9
                    font.bold:      true
                    font.family:    ScreenTools.monoFontFamily
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Rectangle { width: parent.width; height: 1; color: _c_sep }

            // ── Cross-validation ──────────────────────────────────────────
            Row {
                width: parent.width; spacing: _m
                QGCLabel { text: "XVAL γ-ERR"; color: _c_label; font.pointSize: _fs * 0.7; width: _fw * 10 }
                QGCLabel {
                    text:  _rkt ? ((_rkt.xvalGammaErr.rawValue * 180.0/Math.PI).toFixed(3) + "°") : "---.---°"
                    color: _rkt ? valColor(Math.abs(_rkt.xvalGammaErr.rawValue * 180.0/Math.PI), 1, 0, 5, 10) : _c_dim
                    font.pointSize: _fs * 0.85
                    font.family:    ScreenTools.monoFontFamily
                }
            }
            Row {
                width: parent.width; spacing: _m
                QGCLabel { text: "XVAL h-ERR"; color: _c_label; font.pointSize: _fs * 0.7; width: _fw * 10 }
                QGCLabel {
                    text:  _rkt ? _rkt.xvalAltErr.rawValue.toFixed(2) + " m" : "---.-- m"
                    color: _rkt ? valColor(Math.abs(_rkt.xvalAltErr.rawValue), 5, 0, 20, 50) : _c_dim
                    font.pointSize: _fs * 0.85
                    font.family:    ScreenTools.monoFontFamily
                }
            }
            Row {
                width: parent.width; spacing: _m
                QGCLabel { text: "XVAL PENALTY"; color: _c_label; font.pointSize: _fs * 0.7; width: _fw * 10 }
                QGCLabel {
                    text:  _rkt ? _rkt.xvalPenalty.rawValue.toFixed(4) : "-.----"
                    color: _rkt ? valColor(_rkt.xvalPenalty.rawValue, 0.5, 0, 2.0, 5.0) : _c_dim
                    font.pointSize: _fs * 0.85
                    font.family:    ScreenTools.monoFontFamily
                }
            }
        }
    }

    // =========================================================================
    //  BOTTOM PANEL — FIN DEFLECTIONS + AERO DATA
    // =========================================================================
    Rectangle {
        id: _panelFin
        visible:                  _rkt !== null && _rkt !== undefined
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom:           parent.bottom
        anchors.bottomMargin:     parentToolInsets.bottomEdgeCenterInset + _m
        width:                    _finContent.implicitWidth + _m * 4
        height:                   _finContent.implicitHeight + _m * 2.5
        color:                    _c_bg
        radius:                   _r
        border.color:             _c_border
        border.width:             1

        Canvas {
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.strokeStyle = _c_teal
                ctx.lineWidth = 1.5
                var s = 10
                ctx.beginPath(); ctx.moveTo(s,0); ctx.lineTo(0,0); ctx.lineTo(0,s); ctx.stroke()
                ctx.beginPath(); ctx.moveTo(width-s,0); ctx.lineTo(width,0); ctx.lineTo(width,s); ctx.stroke()
                ctx.beginPath(); ctx.moveTo(0,height-s); ctx.lineTo(0,height); ctx.lineTo(s,height); ctx.stroke()
                ctx.beginPath(); ctx.moveTo(width-s,height); ctx.lineTo(width,height); ctx.lineTo(width,height-s); ctx.stroke()
            }
        }

        Column {
            id: _finContent
            anchors.centerIn: parent
            spacing: _fh * 0.25

            // Header row
            Row {
                spacing: _fw * 6
                QGCLabel {
                    text: "FIN DEFLECTIONS"
                    color: _c_label; font.pointSize: _fs * 0.7; font.letterSpacing: 2
                }
                QGCLabel {
                    text: "AERODYNAMIC  PARAMETERS"
                    color: _c_label; font.pointSize: _fs * 0.7; font.letterSpacing: 2
                }
            }

            Row {
                spacing: _m * 3

                // ── 4 fin bars ───────────────────────────────────────────
                Row {
                    spacing: _m * 2
                    Repeater {
                        model: 4
                        Column {
                            spacing: _fh * 0.15

                            QGCLabel {
                                text: "FIN " + (index+1)
                                color: _c_label
                                font.pointSize: _fs * 0.7
                                font.letterSpacing: 1
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            // Fin bar with scale ticks
                            Item {
                                width:  _fw * 3
                                height: _fh * 5
                                anchors.horizontalCenter: parent.horizontalCenter

                                property real finVal: {
                                    if (!_rkt) return 0
                                    switch(index) {
                                    case 0: return _rkt.fin1.rawValue
                                    case 1: return _rkt.fin2.rawValue
                                    case 2: return _rkt.fin3.rawValue
                                    case 3: return _rkt.fin4.rawValue
                                    }
                                    return 0
                                }
                                property real normalized: Math.max(-1.0, Math.min(1.0, finVal / 20.0))
                                property color barColor: Math.abs(finVal) > 16 ? _c_red
                                                       : Math.abs(finVal) > 10 ? _c_amber
                                                       : _c_teal

                                // Background track
                                Rectangle {
                                    anchors.fill: parent
                                    color: _c_bg_solid
                                    border.color: _c_border; border.width: 1; radius: 2
                                }
                                // Scale ticks (±5°, ±10°, ±15°)
                                Repeater {
                                    model: 7
                                    Rectangle {
                                        property real tickPos: (index - 3) / 3.0  // -1 to +1
                                        x: 0; y: parent.height/2 - tickPos * parent.height/2 - height/2
                                        width:  index % 3 === 0 ? parent.width : parent.width * 0.4
                                        height: 1
                                        color: _c_sep
                                    }
                                }
                                // Deflection bar
                                Rectangle {
                                    property real norm: parent.normalized
                                    height: Math.abs(norm) * (parent.height / 2)
                                    width:  parent.width - 4
                                    x:      2
                                    y:      norm >= 0 ? (parent.height/2 - height) : (parent.height/2)
                                    color:  parent.barColor
                                    radius: 2
                                }
                                // Zero line
                                Rectangle {
                                    y: parent.height / 2
                                    width: parent.width; height: 1
                                    color: _c_white; opacity: 0.5
                                }
                            }

                            // Numerical value
                            QGCLabel {
                                property real fv: {
                                    if (!_rkt) return 0
                                    switch(index) {
                                    case 0: return _rkt.fin1.rawValue
                                    case 1: return _rkt.fin2.rawValue
                                    case 2: return _rkt.fin3.rawValue
                                    case 3: return _rkt.fin4.rawValue
                                    }
                                    return 0
                                }
                                text:  (fv >= 0 ? "+" : "") + fv.toFixed(1) + "°"
                                color: Math.abs(fv) > 16 ? _c_red : Math.abs(fv) > 10 ? _c_amber : _c_white
                                font.pointSize: _fs * 0.85
                                font.bold:      true
                                font.family:    ScreenTools.monoFontFamily
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                }

                // ── Divider ──────────────────────────────────────────────
                Rectangle {
                    width: 1; height: _fh * 6.5
                    color: _c_sep
                    anchors.verticalCenter: parent.verticalCenter
                }

                // ── Aero parameters column ────────────────────────────────
                Column {
                    spacing: _fh * 0.22
                    anchors.verticalCenter: parent.verticalCenter

                    AeroRow { label: "q_DYN"; value: _rkt ? _rkt.qDyn.rawValue.toFixed(0)   + " Pa"     : "---- Pa"    }
                    AeroRow { label: "ρ";     value: _rkt ? _rkt.rho.rawValue.toFixed(4)     + " kg/m³"  : "-.---- kg/m³" }
                    AeroRow { label: "Δroll"; value: _rkt ? ((_rkt.deltaRoll.rawValue  >= 0 ? "+" : "") + _rkt.deltaRoll.rawValue.toFixed(3)  + " rad/s²") : "---.--- rad/s²" }
                    AeroRow { label: "Δpitch";value: _rkt ? ((_rkt.deltaPitch.rawValue >= 0 ? "+" : "") + _rkt.deltaPitch.rawValue.toFixed(3) + " rad/s²") : "---.--- rad/s²" }
                    AeroRow { label: "Δyaw";  value: _rkt ? ((_rkt.deltaYaw.rawValue   >= 0 ? "+" : "") + _rkt.deltaYaw.rawValue.toFixed(3)   + " rad/s²") : "---.--- rad/s²" }
                    AeroRow { label: "φ";     value: _rkt ? ((_rkt.phi.rawValue   * 180.0/Math.PI) >= 0 ? "+" : "") + (_rkt.phi.rawValue   * 180.0/Math.PI).toFixed(2) + "°" : "---.--°" }
                    AeroRow { label: "θ";     value: _rkt ? ((_rkt.theta.rawValue * 180.0/Math.PI) >= 0 ? "+" : "") + (_rkt.theta.rawValue * 180.0/Math.PI).toFixed(2) + "°" : "---.--°" }
                    AeroRow { label: "ψ";     value: _rkt ? ((_rkt.psi.rawValue   * 180.0/Math.PI) >= 0 ? "+" : "") + (_rkt.psi.rawValue   * 180.0/Math.PI).toFixed(2) + "°" : "---.--°" }
                }
            }
        }
    }

    // =========================================================================
    //  INLINE COMPONENTS
    // =========================================================================

    component DataRow: Item {
        property string label:     ""
        property string valueText: "---"
        property color  valColor:  _c_white
        property bool   bold:      false

        width:  parent ? parent.width : 0
        height: _fh * 1.05

        QGCLabel {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text:           label
            color:          _c_label
            font.pointSize: _fs * 0.72
            font.letterSpacing: 0.5
            width:          _fw * 9
        }
        QGCLabel {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text:           valueText
            color:          valColor
            font.pointSize: _fs * 0.9
            font.bold:      bold
            font.family:    ScreenTools.monoFontFamily
            horizontalAlignment: Text.AlignRight
        }
    }

    component SolverRow: Item {
        property string label:     ""
        property real   valueMs:   0
        property real   okBelow:   15
        property real   warnBelow: 25
        property real   status:    0
        property real   sqpIter:   0

        width:  parent ? parent.width : 0
        height: _fh * 1.6

        readonly property color dotColor: solverColor(valueMs, okBelow, warnBelow)

        // Status dot
        Rectangle {
            id: _solverDot
            width:  _fh * 0.55; height: width; radius: width / 2
            color:  parent.dotColor
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
        }

        // Label
        QGCLabel {
            anchors.left: _solverDot.right
            anchors.leftMargin: _m * 0.8
            anchors.verticalCenter: parent.verticalCenter
            text:           parent.label
            color:          _c_label
            font.pointSize: _fs * 0.72
            font.letterSpacing: 0.5
        }

        // Solve time
        QGCLabel {
            anchors.right: _errLabel.visible ? _errLabel.left : _iterLabel.visible ? _iterLabel.left : parent.right
            anchors.rightMargin: _m * 0.5
            anchors.verticalCenter: parent.verticalCenter
            text:           valueMs.toFixed(2) + " ms"
            color:          parent.dotColor
            font.pointSize: _fs * 0.9
            font.bold:      valueMs >= warnBelow
            font.family:    ScreenTools.monoFontFamily
        }

        // SQP iter
        QGCLabel {
            id: _iterLabel
            visible: parent.sqpIter > 0
            anchors.right: _errLabel.visible ? _errLabel.left : parent.right
            anchors.rightMargin: _m * 0.5
            anchors.verticalCenter: parent.verticalCenter
            text:  "i=" + Math.round(parent.sqpIter)
            color: _c_dim
            font.pointSize: _fs * 0.7
            font.family:    ScreenTools.monoFontFamily
        }

        // Error badge
        QGCLabel {
            id: _errLabel
            visible: parent.status !== 0
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text:  "[E" + Math.round(parent.status) + "]"
            color: _c_red
            font.pointSize: _fs * 0.75
            font.bold:      true
            font.family:    ScreenTools.monoFontFamily
        }

        // Timing bar
        Rectangle {
            anchors.bottom:      parent.bottom
            anchors.left:        parent.left
            anchors.right:       parent.right
            height:              3
            color:               _c_dim
            radius:              2
            Rectangle {
                width:  Math.min(1.0, parent.parent.valueMs / parent.parent.warnBelow) * parent.width
                height: parent.height
                radius: 2
                color:  parent.parent.dotColor
            }
            // Threshold marker
            Rectangle {
                x:      (parent.parent.okBelow / parent.parent.warnBelow) * parent.width
                width:  1; height: parent.height * 2.0
                anchors.verticalCenter: parent.verticalCenter
                color:  _c_amber; opacity: 0.6
            }
        }
    }

    component AeroRow: Row {
        property string label: ""
        property string value: ""

        spacing: _m * 0.5
        QGCLabel {
            text:           label
            color:          _c_label
            font.pointSize: _fs * 0.72
            font.letterSpacing: 0.5
            width:          _fw * 5.5
        }
        QGCLabel {
            text:           value
            color:          _c_white
            font.pointSize: _fs * 0.82
            font.family:    ScreenTools.monoFontFamily
        }
    }
}
