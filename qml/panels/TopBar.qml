import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Rectangle {
    id: root
    height: 52
    color: "#0B111EE6"
    border.color: "#1E293B"
    border.width: 1

    signal openNewDialog()
    signal saveRequested()
    signal openWorldLibrary()
    signal openNationLibrary()
    signal resetZoomRequested()
    signal zoomInRequested()
    signal zoomOutRequested()
    signal toggleUiRequested()

    property bool is3DMode: false
    property string zoomText: "100%"

    Flickable {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: btnHideUi.left
        anchors.rightMargin: 8
        contentWidth: topBarRow.implicitWidth + 24
        contentHeight: parent.height
        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        RowLayout {
            id: topBarRow
            height: parent.height
            width: Math.max(parent.width - 12, implicitWidth)
            anchors.left: parent.left
            anchors.leftMargin: 12
            spacing: 8

            // Logo & Title
            Row {
                spacing: 8
                Layout.alignment: Qt.AlignVCenter

                Rectangle {
                    width: 36
                    height: 36
                    radius: 10
                    color: "#1E293B"
                    border.color: "#3B82F6"
                    border.width: 1.5

                    Text {
                        anchors.centerIn: parent
                        text: "🗺️"
                        font.pixelSize: 18
                    }
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 0
                    Text {
                        text: worldEditor.worldName
                        font.pixelSize: 14
                        font.bold: true
                        color: "#F8FAFC"
                    }
                    Text {
                        text: "Editor 8K"
                        font.pixelSize: 10
                        font.bold: true
                        color: "#38BDF8"
                    }
                }
            }

            // Separator
            Rectangle {
                width: 1
                height: 28
                color: "#1E293B"
                Layout.alignment: Qt.AlignVCenter
            }

            // Action buttons (Nuevo / Guardar / Mundos)
            Row {
                spacing: 6
                Layout.alignment: Qt.AlignVCenter

                MatalasButton {
                    iconText: "✨"
                    text: "Nuevo"
                    height: 38
                    variant: "primary"
                    onClicked: root.openNewDialog()
                }

                MatalasButton {
                    iconText: "💾"
                    text: "Guardar"
                    height: 38
                    variant: "success"
                    onClicked: root.saveRequested()
                }

                MatalasButton {
                    iconText: "📁"
                    text: "Mundos"
                    height: 38
                    variant: "default"
                    onClicked: root.openWorldLibrary()
                }
            }

            Item { Layout.fillWidth: true }

            // 2D vs 3D Globe Switcher
            Rectangle {
                width: 132
                height: 38
                radius: 19
                color: "#162032"
                border.color: "#334155"
                border.width: 1
                Layout.alignment: Qt.AlignVCenter

                Row {
                    anchors.centerIn: parent
                    spacing: 2

                    Rectangle {
                        width: 63
                        height: 32
                        radius: 16
                        color: !root.is3DMode ? "#2563EB" : "transparent"
                        Behavior on color { ColorAnimation { duration: 120 } }
                        Text {
                            anchors.centerIn: parent
                            text: "🗺️ 2D"
                            font.bold: true
                            font.pixelSize: 12
                            color: !root.is3DMode ? "#FFFFFF" : "#94A3B8"
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.is3DMode = false
                        }
                    }

                    Rectangle {
                        width: 63
                        height: 32
                        radius: 16
                        color: root.is3DMode ? "#8B5CF6" : "transparent"
                        Behavior on color { ColorAnimation { duration: 120 } }
                        Text {
                            anchors.centerIn: parent
                            text: "🌐 3D"
                            font.bold: true
                            font.pixelSize: 12
                            color: root.is3DMode ? "#FFFFFF" : "#94A3B8"
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.is3DMode = true
                        }
                    }
                }
            }

            // Mode Switcher: Terreno vs Político
            Rectangle {
                width: 216
                height: 38
                radius: 19
                color: "#162032"
                border.color: "#334155"
                border.width: 1
                Layout.alignment: Qt.AlignVCenter

                Row {
                    anchors.centerIn: parent
                    spacing: 2

                    Rectangle {
                        width: 104
                        height: 32
                        radius: 16
                        color: (worldEditor.activeMode === 0) ? "#059669" : "transparent"
                        Behavior on color { ColorAnimation { duration: 120 } }
                        Text {
                            anchors.centerIn: parent
                            text: "🌍 Terreno"
                            font.bold: true
                            font.pixelSize: 12
                            color: (worldEditor.activeMode === 0) ? "#FFFFFF" : "#94A3B8"
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: worldEditor.activeMode = 0
                        }
                    }

                    Rectangle {
                        width: 104
                        height: 32
                        radius: 16
                        color: (worldEditor.activeMode === 1) ? "#2563EB" : "transparent"
                        Behavior on color { ColorAnimation { duration: 120 } }
                        Text {
                            anchors.centerIn: parent
                            text: "🏛️ Político"
                            font.bold: true
                            font.pixelSize: 12
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

            // Display Mode Switcher: Colores vs Banderas (Visible only in Political Mode)
            Rectangle {
                width: 180
                height: 38
                radius: 19
                color: "#162032"
                border.color: "#334155"
                border.width: 1
                visible: (worldEditor.activeMode === 1)
                Layout.alignment: Qt.AlignVCenter

                Row {
                    anchors.centerIn: parent
                    spacing: 2

                    Rectangle {
                        width: 86
                        height: 32
                        radius: 16
                        color: (worldEditor.displayMode === 0) ? "#DB2777" : "transparent"
                        Behavior on color { ColorAnimation { duration: 120 } }
                        Text {
                            anchors.centerIn: parent
                            text: "🎨 Colores"
                            font.bold: true
                            font.pixelSize: 11
                            color: (worldEditor.displayMode === 0) ? "#FFFFFF" : "#94A3B8"
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: worldEditor.displayMode = 0
                        }
                    }

                    Rectangle {
                        width: 86
                        height: 32
                        radius: 16
                        color: (worldEditor.displayMode === 1) ? "#D97706" : "transparent"
                        Behavior on color { ColorAnimation { duration: 120 } }
                        Text {
                            anchors.centerIn: parent
                            text: "🚩 Banderas"
                            font.bold: true
                            font.pixelSize: 11
                            color: (worldEditor.displayMode === 1) ? "#FFFFFF" : "#94A3B8"
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: worldEditor.displayMode = 1
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }

            // Nations Library Button
            MatalasButton {
                iconText: "📜"
                text: "Biblioteca"
                height: 38
                variant: "warning"
                onClicked: root.openNationLibrary()
            }

            // Zoom Actions
            Row {
                spacing: 5
                Layout.alignment: Qt.AlignVCenter

                MatalasButton {
                    iconText: "🔍 −"
                    height: 38
                    onClicked: root.zoomOutRequested()
                }

                Rectangle {
                    height: 38
                    width: 58
                    radius: 12
                    color: "#162032"
                    border.color: "#334155"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: root.zoomText
                        color: "#38BDF8"
                        font.bold: true
                        font.pixelSize: 11
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.resetZoomRequested()
                    }
                }

                MatalasButton {
                    iconText: "🔍 +"
                    height: 38
                    onClicked: root.zoomInRequested()
                }

                MatalasButton {
                    iconText: "⛶"
                    text: "Ajustar"
                    height: 38
                    onClicked: root.resetZoomRequested()
                }
            }
        }
    }

    // Pinned Hide UI / Zen Canvas Mode Button on the top right
    MatalasButton {
        id: btnHideUi
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        iconText: "👁️"
        text: "Ocultar"
        height: 38
        variant: "ghost"
        onClicked: root.toggleUiRequested()
    }
}
