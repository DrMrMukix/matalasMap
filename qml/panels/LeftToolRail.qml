import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Rectangle {
    id: root

    signal toggleCountryPanel()

    width: 68
    implicitHeight: mainColumn.implicitHeight + 24
    height: Math.min(parent ? (parent.height - (root.visible ? 76 : 24)) : implicitHeight, implicitHeight)
    radius: 24
    color: "#0B111ECC"
    border.color: "#334155"
    border.width: 1.5
    clip: true

    Flickable {
        anchors.fill: parent
        anchors.topMargin: 12
        anchors.bottomMargin: 12
        contentHeight: mainColumn.implicitHeight
        contentWidth: width
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ColumnLayout {
            id: mainColumn
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

        // Tool 0: Land (Terrain) / Paint (Political)
        Rectangle {
            id: btnDraw
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            radius: 14
            color: (worldEditor.activeTool === 0)
                   ? ((worldEditor.activeMode === 0) ? "#059669" : "#2563EB")
                   : (maDraw.pressed ? "#334155" : (maDraw.containsMouse ? "#27354A" : "#1E293B"))
            border.color: (worldEditor.activeTool === 0) ? "#6EE7B7" : "#334155"
            border.width: (worldEditor.activeTool === 0) ? 2 : 1

            Text {
                anchors.centerIn: parent
                text: (worldEditor.activeMode === 0) ? "✏️" : "🖌️"
                font.pixelSize: 22
            }

            MouseArea {
                id: maDraw
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: worldEditor.activeTool = 0
            }

            ToolTip.visible: maDraw.containsMouse
            ToolTip.text: (worldEditor.activeMode === 0) ? "Pintar Tierra" : "Pintar País"
        }

        // Tool 1: Ocean (Terrain) / Erase (Political)
        Rectangle {
            id: btnErase
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            radius: 14
            color: (worldEditor.activeTool === 1)
                   ? "#DC2626"
                   : (maErase.pressed ? "#334155" : (maErase.containsMouse ? "#27354A" : "#1E293B"))
            border.color: (worldEditor.activeTool === 1) ? "#FCA5A5" : "#334155"
            border.width: (worldEditor.activeTool === 1) ? 2 : 1

            Text {
                anchors.centerIn: parent
                text: (worldEditor.activeMode === 0) ? "🌊" : "🧽"
                font.pixelSize: 22
            }

            MouseArea {
                id: maErase
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: worldEditor.activeTool = 1
            }

            ToolTip.visible: maErase.containsMouse
            ToolTip.text: (worldEditor.activeMode === 0) ? "Borrar a Océano" : "Borrar País"
        }

        // Tool 2: Bucket Fill
        Rectangle {
            id: btnFill
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            radius: 14
            color: (worldEditor.activeTool === 2)
                   ? "#D97706"
                   : (maFill.pressed ? "#334155" : (maFill.containsMouse ? "#27354A" : "#1E293B"))
            border.color: (worldEditor.activeTool === 2) ? "#FDE68A" : "#334155"
            border.width: (worldEditor.activeTool === 2) ? 2 : 1

            Text {
                anchors.centerIn: parent
                text: "🪣"
                font.pixelSize: 22
            }

            MouseArea {
                id: maFill
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: worldEditor.activeTool = 2
            }

            ToolTip.visible: maFill.containsMouse
            ToolTip.text: "Relleno Rápido"
        }

        // Tool 3: Color / Country Picker
        Rectangle {
            id: btnPicker
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            radius: 14
            color: (worldEditor.activeTool === 3)
                   ? "#7C3AED"
                   : (maPicker.pressed ? "#334155" : (maPicker.containsMouse ? "#27354A" : "#1E293B"))
            border.color: (worldEditor.activeTool === 3) ? "#C4B5FD" : "#334155"
            border.width: (worldEditor.activeTool === 3) ? 2 : 1

            Text {
                anchors.centerIn: parent
                text: "🎯"
                font.pixelSize: 22
            }

            MouseArea {
                id: maPicker
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: worldEditor.activeTool = 3
            }

            ToolTip.visible: maPicker.containsMouse
            ToolTip.text: "Gotero / Selector"
        }

        // Tool 4: Hand (Pan / Rotate)
        Rectangle {
            id: btnHand
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            radius: 14
            color: (worldEditor.activeTool === 4)
                   ? "#0D9488"
                   : (maHand.pressed ? "#334155" : (maHand.containsMouse ? "#27354A" : "#1E293B"))
            border.color: (worldEditor.activeTool === 4) ? "#99F6E4" : "#334155"
            border.width: (worldEditor.activeTool === 4) ? 2 : 1

            Text {
                anchors.centerIn: parent
                text: "✋"
                font.pixelSize: 22
            }

            MouseArea {
                id: maHand
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: worldEditor.activeTool = 4
            }

            ToolTip.visible: maHand.containsMouse
            ToolTip.text: "Mover / Navegar"
        }

        // Divider
        Rectangle {
            Layout.preferredWidth: 42
            Layout.preferredHeight: 1.5
            color: "#334155"
            Layout.alignment: Qt.AlignHCenter
        }

        // Brush Size Detailed Selector
        Rectangle {
            id: btnBrushSize
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            radius: 14
            color: brushPopup.visible ? "#3B82F6" : (maBrush.pressed ? "#334155" : (maBrush.containsMouse ? "#1E293B" : "#0B111E"))
            border.color: brushPopup.visible ? "#60A5FA" : "#475569"
            border.width: brushPopup.visible ? 2 : 1

            Column {
                anchors.centerIn: parent
                spacing: 2

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.max(6, Math.min(22, worldEditor.brushRadius * 0.75))
                    height: width
                    radius: width / 2
                    color: (worldEditor.activeMode === 0) ? "#10B981" : worldEditor.activeCountryColor
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: worldEditor.brushRadius + "px"
                    font.pixelSize: 10
                    font.bold: true
                    color: brushPopup.visible ? "#FFFFFF" : "#94A3B8"
                }
            }

            MouseArea {
                id: maBrush
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (brushPopup.visible) {
                        brushPopup.close()
                    } else {
                        brushPopup.open()
                    }
                }
            }

            ToolTip.visible: maBrush.containsMouse && !brushPopup.visible
            ToolTip.text: "Pincel: " + worldEditor.brushRadius + "px (Clic para ajustar al detalle)"
        }

        // Divider
        Rectangle {
            Layout.preferredWidth: 42
            Layout.preferredHeight: 1.5
            color: "#334155"
            Layout.alignment: Qt.AlignHCenter
        }

        // Undo
        Rectangle {
            Layout.preferredWidth: 50
            Layout.preferredHeight: 44
            radius: 12
            color: maUndo.pressed ? "#334155" : (maUndo.containsMouse ? "#27354A" : "#1E293B")
            border.color: "#334155"
            border.width: 1
            opacity: worldEditor.canUndo ? 1.0 : 0.4

            Text {
                anchors.centerIn: parent
                text: "↩️"
                font.pixelSize: 20
            }

            MouseArea {
                id: maUndo
                anchors.fill: parent
                enabled: worldEditor.canUndo
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: worldEditor.undo()
            }

            ToolTip.visible: maUndo.containsMouse
            ToolTip.text: "Deshacer"
        }

        // Redo
        Rectangle {
            Layout.preferredWidth: 50
            Layout.preferredHeight: 44
            radius: 12
            color: maRedo.pressed ? "#334155" : (maRedo.containsMouse ? "#27354A" : "#1E293B")
            border.color: "#334155"
            border.width: 1
            opacity: worldEditor.canRedo ? 1.0 : 0.4

            Text {
                anchors.centerIn: parent
                text: "↪️"
                font.pixelSize: 20
            }

            MouseArea {
                id: maRedo
                anchors.fill: parent
                enabled: worldEditor.canRedo
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: worldEditor.redo()
            }

            ToolTip.visible: maRedo.containsMouse
            ToolTip.text: "Rehacer"
        }

        // Country Panel Toggle Badge (Only in Political Mode)
        Rectangle {
            id: countryToggleBadge
            visible: (worldEditor.activeMode === 1)
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            radius: 14
            color: maCountry.pressed ? "#334155" : (maCountry.containsMouse ? "#27354A" : "#1E293B")
            border.color: worldEditor.activeCountryColor
            border.width: 2.5

            Column {
                anchors.centerIn: parent
                spacing: 2
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "🏛️"
                    font.pixelSize: 16
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Países"
                    font.pixelSize: 9
                    font.bold: true
                    color: "#F1F5F9"
                }
            }

            MouseArea {
                id: maCountry
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.toggleCountryPanel()
            }

            ToolTip.visible: maCountry.containsMouse
            ToolTip.text: "Abrir Lista de Países: " + worldEditor.activeCountryName
        }
    }
}

    // Detailed Brush Size Flyout Popup
    Popup {
        id: brushPopup
        x: root.width + 12
        y: Math.max(10, Math.min(parent ? parent.height - height - 10 : 300, btnBrushSize.mapToItem(root, 0, 0).y - 100))
        width: 290
        height: 290
        padding: 16
        modal: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#0F172A"
            radius: 20
            border.color: "#334155"
            border.width: 1.5

            // Left arrow accent indicator
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: -5
                width: 10
                height: 10
                rotation: 45
                color: "#0F172A"
                border.color: "#334155"
                border.width: 1.5
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            // Header: Title & Close
            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "🖌️ Tamaño de Píxeles"
                    font.pixelSize: 14
                    font.bold: true
                    color: "#F8FAFC"
                    Layout.fillWidth: true
                }

                Rectangle {
                    width: 24
                    height: 24
                    radius: 12
                    color: "#1E293B"
                    Text {
                        anchors.centerIn: parent
                        text: "✕"
                        font.pixelSize: 11
                        color: "#94A3B8"
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: brushPopup.close()
                    }
                }
            }

            // Current Size Preview & Nudge Buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                // -5 button
                Rectangle {
                    width: 32
                    height: 32
                    radius: 8
                    color: mMinus5.containsMouse ? "#334155" : "#1E293B"
                    border.color: "#475569"
                    Text { anchors.centerIn: parent; text: "-5"; font.pixelSize: 11; font.bold: true; color: "#CBD5E1" }
                    MouseArea {
                        id: mMinus5; anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: worldEditor.brushRadius = Math.max(1, worldEditor.brushRadius - 5)
                    }
                }

                // -1 button
                Rectangle {
                    width: 32
                    height: 32
                    radius: 8
                    color: mMinus1.containsMouse ? "#334155" : "#1E293B"
                    border.color: "#475569"
                    Text { anchors.centerIn: parent; text: "-1"; font.pixelSize: 12; font.bold: true; color: "#CBD5E1" }
                    MouseArea {
                        id: mMinus1; anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: worldEditor.brushRadius = Math.max(1, worldEditor.brushRadius - 1)
                    }
                }

                // Center visual preview & number
                Rectangle {
                    Layout.fillWidth: true
                    height: 48
                    radius: 12
                    color: "#162032"
                    border.color: "#38BDF8"
                    border.width: 1

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 8

                        Rectangle {
                            width: Math.max(6, Math.min(26, worldEditor.brushRadius * 0.7))
                            height: width
                            radius: width / 2
                            color: (worldEditor.activeMode === 0) ? "#10B981" : worldEditor.activeCountryColor
                        }

                        Text {
                            text: worldEditor.brushRadius + " px"
                            font.pixelSize: 16
                            font.bold: true
                            color: "#38BDF8"
                        }
                    }
                }

                // +1 button
                Rectangle {
                    width: 32
                    height: 32
                    radius: 8
                    color: mPlus1.containsMouse ? "#334155" : "#1E293B"
                    border.color: "#475569"
                    Text { anchors.centerIn: parent; text: "+1"; font.pixelSize: 12; font.bold: true; color: "#CBD5E1" }
                    MouseArea {
                        id: mPlus1; anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: worldEditor.brushRadius = Math.min(100, worldEditor.brushRadius + 1)
                    }
                }

                // +5 button
                Rectangle {
                    width: 32
                    height: 32
                    radius: 8
                    color: mPlus5.containsMouse ? "#334155" : "#1E293B"
                    border.color: "#475569"
                    Text { anchors.centerIn: parent; text: "+5"; font.pixelSize: 11; font.bold: true; color: "#CBD5E1" }
                    MouseArea {
                        id: mPlus5; anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: worldEditor.brushRadius = Math.min(100, worldEditor.brushRadius + 5)
                    }
                }
            }

            // Slider
            Slider {
                id: brushSlider
                Layout.fillWidth: true
                from: 1
                to: 64
                stepSize: 1
                value: worldEditor.brushRadius
                onMoved: worldEditor.brushRadius = Math.round(value)
            }

            // Quick Preset Chips (1px, 2px, 4px, 8px, 12px, 16px, 24px, 32px, 48px, 64px)
            Text {
                text: "Atajos rápidos:"
                font.pixelSize: 11
                color: "#64748B"
                font.bold: true
            }

            Flow {
                Layout.fillWidth: true
                spacing: 6

                Repeater {
                    model: [1, 2, 4, 8, 12, 16, 24, 32, 48, 64]

                    delegate: Rectangle {
                        width: 44
                        height: 26
                        radius: 8
                        color: (worldEditor.brushRadius === modelData) ? "#2563EB" : (pMouse.containsMouse ? "#334155" : "#1E293B")
                        border.color: (worldEditor.brushRadius === modelData) ? "#60A5FA" : "#334155"

                        Text {
                            anchors.centerIn: parent
                            text: modelData + "px"
                            font.pixelSize: 11
                            font.bold: (worldEditor.brushRadius === modelData)
                            color: (worldEditor.brushRadius === modelData) ? "#FFFFFF" : "#94A3B8"
                        }

                        MouseArea {
                            id: pMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: worldEditor.brushRadius = modelData
                        }
                    }
                }
            }
        }
    }
}
