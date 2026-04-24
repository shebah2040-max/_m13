// SPDX-License-Identifier: GPL-3.0-or-later
//
// M130 GCS — Fly View root gate (R1.3).
//
// Replaces QGroundControl's default FlyViewCustomLayer. Routes the view
// depending on session state published by AccessController:
//
//   - m130Access.loggedIn === false  → LoginScreen (qrc:/qml/views/login/LoginScreen.qml)
//   - m130Access.loggedIn === true   → ConsoleSwitcher (qrc:/qml/ConsoleSwitcher.qml)
//
// The legacy full-screen HUD that previously lived here was extracted
// verbatim to qrc:/qml/views/hud/LegacyHud.qml and is now loaded on
// demand by the Operations console (see OperationsConsole.qml).

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.Palette

Item {
    id: _root
    anchors.fill: parent

    // Forwarded by FlyView.qml; kept as declared properties so existing
    // bindings in FlyView.qml continue to type-check. The gate itself
    // does not use them, but LegacyHud (via OperationsConsole) does.
    property var parentToolInsets
    property var totalToolInsets
    property var mapControl

    readonly property bool _loggedIn: (typeof m130Access !== "undefined")
                                      && m130Access
                                      && m130Access.loggedIn

    Loader {
        id: _gateLoader
        anchors.fill: parent
        source: _root._loggedIn
                ? "qrc:/qml/ConsoleSwitcher.qml"
                : "qrc:/qml/views/login/LoginScreen.qml"
        active: true
        asynchronous: false
    }
}
