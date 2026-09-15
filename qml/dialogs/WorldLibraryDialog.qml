import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Dialog {
    id: root
    title: ""
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent.width * 0.88, 780)
    height: Math.min(parent.height * 0.85, 560)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property var worldsList: []

    function refreshWorlds() {
        worldsList = worldEditor.getSavedWorldsList()
    }

    onOpened: refreshWorlds()

    background: Rectangle {
        color: "#0F172A"
        radius: 22
        border.color: "#334155"
        border.width: 2
        clip: true

        Rectangle {
            anchors.top: parent.top
            anchors.topMargin: 4
            anchors.left: parent.left
            anchors.leftMargin: 24
            anchors.right: parent.right
            anchors.rightMargin: 24
            height: 4
            radius: 2
            color: "#10B981"
        }
    }

    contentItem: ColumnLayout {
        spacing: 16

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Rectangle {
                width: 44
                height: 44
                radius: 14
                color: "#1E293B"
                border.color: "#10B981"
                border.width: 1.5

                Text {
                    anchors.centerIn: parent
                    text: "📁"
                    font.pixelSize: 22
                }
            }

            Column {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    text: "Mis Mundos Guardados"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#F8FAFC"
                }
                Text {
                    text: "Formatos .matalas locales de alta velocidad"
                    font.pixelSize: 12
                    color: "#94A3B8"
                }
            }

            MatalasButton {
                iconText: "✕"
                iconOnly: true
                height: 36
                width: 36
                variant: "ghost"
                onClicked: root.close()
            }
        }

        // Empty state
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.worldsList.length === 0

            Column {
                anchors.centerIn: parent
                spacing: 12
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "🗺️"
                    font.pixelSize: 48
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Aún no tienes mundos guardados"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#94A3B8"
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "¡Pinta tu primer mapa y presiona Guardar!"
                    font.pixelSize: 13
                    color: "#64748B"
                }
            }
        }

        // Worlds List
        ListView {
            id: worldsView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            visible: root.worldsList.length > 0
            model: root.worldsList
            spacing: 10

            delegate: Rectangle {
                width: worldsView.width
                height: 76
                radius: 16
                color: itemMouse.containsMouse ? "#1E293B" : "#162032"
                border.color: itemMouse.containsMouse ? "#38BDF8" : "#334155"
                border.width: 1.5

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 14

                    // Map Icon
                    Rectangle {
                        width: 52
                        height: 52
                        radius: 12
                        color: "#0F172A"
                        border.color: "#475569"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "🌍"
                            font.pixelSize: 24
                        }
                    }

                    Column {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: modelData.name
                            font.pixelSize: 16
                            font.bold: true
                            color: "#FFFFFF"
                        }

                        Row {
                            spacing: 12
                            Text {
                                text: "📅 " + (modelData.modified || modelData.date || "Reciente")
                                font.pixelSize: 12
                                color: "#94A3B8"
                            }
                            Text {
                                text: "📦 " + (modelData.size || (Math.round((modelData.sizeBytes || 0) / 1024) + " KB"))
                                font.pixelSize: 12
                                color: "#64748B"
                            }
                        }
                    }

                    // Action buttons
                    Row {
                        spacing: 8

                        MatalasButton {
                            iconText: "📂"
                            text: "Abrir"
                            variant: "success"
                            onClicked: {
                                var p = modelData.path || modelData.filePath;
                                if (p) {
                                    worldEditor.startAsyncLoadWorld(p);
                                    root.close();
                                }
                            }
                        }

                        MatalasButton {
                            iconText: "🗑️"
                            iconOnly: true
                            variant: "danger"
                            onClicked: {
                                var p = modelData.path || modelData.filePath;
                                if (p) {
                                    worldEditor.deleteSavedWorld(p);
                                    root.refreshWorlds();
                                }
                            }
                        }
                    }
                }

                MouseArea {
                    id: itemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    acceptedButtons: Qt.NoButton
                }
            }
        }
    }
}
