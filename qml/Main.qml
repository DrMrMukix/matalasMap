import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Matalas 1.0

import "panels"
import "dialogs"
import "components"

Window {
    id: appWindow
    width: 1280
    height: 800
    visible: true
    title: "matalasMap — Editor Creativo de Mapas Históricos"
    color: "#0B132B"

    Item {
        id: root
        anchors.fill: parent

        property bool uiVisible: true

        // 2D Map Canvas (Direct Rust 8K Renderer integration)
        MapCanvas {
            id: canvas
            anchors.fill: parent
            bridge: worldEditor
            visible: !topBar.is3DMode

            onCursorWorldCoordsChanged: function(wx, wy) {
                coordBadge.text = "X: " + wx + "  Y: " + wy;
            }
        }

        // 3D Interactive Globe
        Globe3D {
            id: globe
            anchors.fill: parent
            bridge: worldEditor
            visible: topBar.is3DMode

            onCursorWorldCoordsChanged: function(wx, wy) {
                coordBadge.text = "X: " + wx + "  Y: " + wy + " (3D)";
            }
        }

        // Top Navigation & Control Bar (Streamlined to 52px)
        TopBar {
            id: topBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            z: 100
            visible: root.uiVisible
            zoomText: (!topBar.is3DMode ? Math.round(canvas.zoom * 100) : Math.round(globe.zoom * 100)) + "%"

            onToggleUiRequested: root.uiVisible = false
            onOpenNewDialog: newDialog.open()
            onOpenWorldLibrary: worldLibraryDialog.open()
            onOpenNationLibrary: nationLibraryDialog.open()

            onSaveRequested: saveWorldDialog.open()
            onResetZoomRequested: {
                if (!topBar.is3DMode) {
                    canvas.resetView();
                } else {
                    globe.resetView();
                }
            }
            onZoomInRequested: {
                if (!topBar.is3DMode) {
                    canvas.zoomIn();
                } else {
                    globe.zoomIn();
                }
            }
            onZoomOutRequested: {
                if (!topBar.is3DMode) {
                    canvas.zoomOut();
                } else {
                    globe.zoomOut();
                }
            }
        }

        // Left Vertical Tool Rail (Ergonomic layout for widescreen tablets)
        LeftToolRail {
            id: leftToolRail
            anchors.left: parent.left
            anchors.leftMargin: 12
            y: root.uiVisible
               ? Math.max(topBar.bottom + 12, (parent.height + topBar.height - height) / 2)
               : (parent.height - height) / 2
            z: 100
            visible: root.uiVisible

            onToggleCountryPanel: {
                countryPanel.visible = !countryPanel.visible;
            }
        }

        // Slide-out Right Sidebar (Country & Nation Manager)
        CountryPanel {
            id: countryPanel
            anchors.right: parent.right
            anchors.rightMargin: 14
            anchors.top: root.uiVisible ? topBar.bottom : parent.top
            anchors.topMargin: 14
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 14
            z: 100
            visible: root.uiVisible && (worldEditor.activeMode === 1)

            onOpenHistoricalLibrary: nationLibraryDialog.open()
            onOpenColorPicker: colorPickerPopup.open()
        }

        // Floating Button to restore bars when in Zen Canvas Mode
        Rectangle {
            id: btnRestoreUi
            visible: !root.uiVisible
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 14
            width: 140
            height: 40
            radius: 20
            color: "#0F172AF0"
            border.color: "#38BDF8"
            border.width: 1.5
            z: 200

            Row {
                anchors.centerIn: parent
                spacing: 6
                Text {
                    text: "👁️"
                    font.pixelSize: 15
                }
                Text {
                    text: "Mostrar Barras"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#F8FAFC"
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.uiVisible = true
            }
        }

        // Dialog for New World
        NewWorldDialog {
            id: newDialog
        }

        // Dialog for Saved Worlds
        WorldLibraryDialog {
            id: worldLibraryDialog
        }

        // Dialog for Saving World with custom name & overwrite
        SaveWorldDialog {
            id: saveWorldDialog
        }

        // Dialog for Historical Nations (1650-2026)
        NationLibraryDialog {
            id: nationLibraryDialog
            onNationSelected: function(nation) {
                var newId = worldEditor.createCountry(nation.name, nation.color);
                if (nation.flagPath && nation.flagPath.length > 0) {
                    worldEditor.setCountryFlag(newId, nation.flagPath);
                }
                worldEditor.activeCountryId = newId;
                toast.show("¡" + nation.name + " lista para pintar!");
            }
        }

        // Enhanced Color Picker Popup
        ColorPickerPopup {
            id: colorPickerPopup
            anchors.centerIn: parent
            onColorAccepted: function(col) {
                countryPanel.selectedColor = col;
            }
        }

        // Coordinate & Status Dock (Docked neatly in bottom-left corner)
        Rectangle {
            id: infoBadge
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 12
            anchors.left: (root.uiVisible && leftToolRail.visible) ? leftToolRail.right : parent.left
            anchors.leftMargin: 12
            width: coordBadge.implicitWidth + 30
            height: 32
            radius: 10
            color: "#0B111ECC"
            border.color: "#334155"
            border.width: 1
            z: 90
            opacity: root.uiVisible ? 1.0 : 0.65

            Row {
                anchors.centerIn: parent
                spacing: 6
                Text {
                    text: "📍"
                    font.pixelSize: 12
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    id: coordBadge
                    text: "X: 0  Y: 0"
                    color: "#38BDF8"
                    font.pixelSize: 11
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        Connections {
            target: worldEditor
            function onCountryPicked(id, name, color, flagPath) {
                toast.show("🎯 País seleccionado: " + name)
            }
        }

        // Feedback Toast
        Rectangle {
            id: toast
            anchors.top: root.uiVisible ? topBar.bottom : parent.top
            anchors.topMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            width: toastText.implicitWidth + 40
            height: 44
            radius: 22
            color: "#0F172ACC"
            border.color: "#38BDF8"
            border.width: 1.5
            z: 300
            opacity: 0.0
            visible: opacity > 0.0

            Behavior on opacity { NumberAnimation { duration: 200 } }

            Text {
                id: toastText
                anchors.centerIn: parent
                text: ""
                color: "#F8FAFC"
                font.bold: true
                font.pixelSize: 13
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

        // Fullscreen Interactive Loading Overlay
        LoadingOverlay {
            id: loadingOverlay
        }
    }
}
