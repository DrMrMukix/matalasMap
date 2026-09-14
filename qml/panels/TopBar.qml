import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    height: 70
    color: "#0F172A"
    border.color: "#1E293B"
    border.width: 1

    signal openNewDialog()
    signal saveRequested()
    signal resetZoomRequested()
    signal zoomInRequested()
    signal zoomOutRequested()

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 16

        // Logo & World Title
        Row {
            spacing: 10
            Text {
                text: "🗺️"
                font.pixelSize: 28
                anchors.verticalCenter: parent.verticalCenter
            }
            Column {
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    text: worldEditor.worldName
                    font.pixelSize: 17
                    font.bold: true
                    color: "#F8FAFC"
                }
                Text {
                    text: "Editor Creativo 8K"
                    font.pixelSize: 11
                    color: "#94A3B8"
                }
            }
        }

        // Action buttons (Nuevo / Guardar)
        Row {
            spacing: 8

            Button {
                text: "✨ Nuevo"
                font.bold: true
                onClicked: root.openNewDialog()
            }

            Button {
                id: saveBtn
                text: "💾 Guardar"
                font.bold: true
                onClicked: root.saveRequested()
            }
        }

        Item { Layout.fillWidth: true }

        // Mode Switcher: Terreno vs Político
        Rectangle {
            width: 260
            height: 46
            radius: 23
            color: "#1E293B"
            border.color: "#334155"
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: 4

                // Terrain Mode Tab
                Rectangle {
                    width: 124
                    height: 38
                    radius: 19
                    color: (worldEditor.activeMode === 0) ? "#10B981" : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }

                    Text {
                        anchors.centerIn: parent
                        text: "🌍 Terreno"
                        font.bold: true
                        font.pixelSize: 14
                        color: (worldEditor.activeMode === 0) ? "#FFFFFF" : "#94A3B8"
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: worldEditor.activeMode = 0
                    }
                }

                // Political Mode Tab
                Rectangle {
                    width: 124
                    height: 38
                    radius: 19
                    color: (worldEditor.activeMode === 1) ? "#3B82F6" : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }

                    Text {
                        anchors.centerIn: parent
                        text: "🏛️ Político"
                        font.bold: true
                        font.pixelSize: 14
                        color: (worldEditor.activeMode === 1) ? "#FFFFFF" : "#94A3B8"
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: worldEditor.activeMode = 1
                    }
                }
            }
        }

        Item { Layout.fillWidth: true }

        // Undo / Redo
        Row {
            spacing: 6

            Button {
                text: "↩️"
                enabled: worldEditor.canUndo
                onClicked: worldEditor.undo()
            }

            Button {
                text: "↪️"
                enabled: worldEditor.canRedo
                onClicked: worldEditor.redo()
            }
        }

        // Zoom Navigation
        Row {
            spacing: 6

            Button {
                text: "🔍 -"
                onClicked: root.zoomOutRequested()
            }

            Button {
                text: "🔍 +"
                onClicked: root.zoomInRequested()
            }

            Button {
                text: "⛶ Ajustar"
                onClicked: root.resetZoomRequested()
            }
        }
    }
}
