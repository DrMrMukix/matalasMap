import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Matalas 1.0

import "panels"
import "dialogs"

Window {
    id: appWindow
    width: 1280
    height: 800
    visible: true
    title: "matalasMap — Editor Creativo de Mapas Históricos"
    color: "#0B132B"

    // 2D Map Canvas (Direct Rust 8K Renderer integration)
    MapCanvas {
        id: canvas
        anchors.fill: parent
        bridge: worldEditor

        onCursorWorldCoordsChanged: function(wx, wy) {
            coordBadge.text = "X: " + wx + "  Y: " + wy;
        }
    }

    // Top Navigation & Control Bar
    TopBar {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        onOpenNewDialog: newDialog.open()
        onSaveRequested: {
            var savePath = "assets/presets/" + worldEditor.worldName.replace(/ /g, "_") + ".matalas";
            if (worldEditor.saveWorld(savePath)) {
                toast.show("¡Mundo guardado exitosamente!");
            } else {
                toast.show("Error al guardar el mundo");
            }
        }
        onResetZoomRequested: canvas.resetView()
        onZoomInRequested: canvas.zoomIn()
        onZoomOutRequested: canvas.zoomOut()
    }

    // Bottom Main ToolBar
    ToolBar {
        id: bottomToolBar
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(840, parent.width - 40)

        onToggleCountryPanel: {
            countryPanel.visible = !countryPanel.visible;
        }
    }

    // Slide-out Country Panel
    CountryPanel {
        id: countryPanel
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.top: topBar.bottom
        anchors.topMargin: 20
        visible: (worldEditor.activeMode === 1) // automatically opens in Political Mode
    }

    // Dialog for New World
    NewWorldDialog {
        id: newDialog
    }

    // Coordinate & Zoom Badge
    Rectangle {
        id: infoBadge
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        anchors.left: parent.left
        anchors.leftMargin: 24
        width: 180
        height: 36
        radius: 12
        color: "#0F172ACC"
        border.color: "#334155"
        border.width: 1

        Text {
            id: coordBadge
            anchors.centerIn: parent
            text: "X: 0  Y: 0"
            color: "#94A3B8"
            font.pixelSize: 12
            font.bold: true
        }
    }

    // Feedback Toast
    Rectangle {
        id: toast
        anchors.top: topBar.bottom
        anchors.topMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter
        width: toastText.implicitWidth + 36
        height: 44
        radius: 22
        color: "#10B981"
        opacity: 0.0
        visible: opacity > 0.0

        Behavior on opacity { NumberAnimation { duration: 250 } }

        Text {
            id: toastText
            anchors.centerIn: parent
            text: ""
            color: "#FFFFFF"
            font.bold: true
            font.pixelSize: 14
        }

        Timer {
            id: toastTimer
            interval: 2200
            onTriggered: toast.opacity = 0.0
        }

        function show(msg) {
            toastText.text = msg;
            toast.opacity = 1.0;
            toastTimer.restart();
        }
    }

    Component.onCompleted: {
        canvas.resetView();
    }
}
