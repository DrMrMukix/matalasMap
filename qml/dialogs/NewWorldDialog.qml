import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Dialog {
    id: root
    title: ""
    modal: true
    anchors.centerIn: parent
    width: Math.min(760, parent.width - 32)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: "#0F172A"
        radius: 24
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
            color: "#3B82F6"
        }
    }

    contentItem: ColumnLayout {
        spacing: 20

        // Header
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 4

            Text {
                text: "🌟 ¡Crea un Nuevo Mundo!"
                font.pixelSize: 20
                font.bold: true
                color: "#F8FAFC"
                Layout.fillWidth: true
            }

            MatalasButton {
                iconText: "✕"
                iconOnly: true
                height: 36
                width: 36
                variant: "ghost"
                onClicked: root.reject()
            }
        }

        Text {
            text: "¿Cómo quieres empezar tu mapa creativo?"
            font.pixelSize: 14
            color: "#94A3B8"
            Layout.alignment: Qt.AlignHCenter
        }

        Flickable {
            Layout.fillWidth: true
            Layout.preferredHeight: 215
            contentWidth: Math.max(width, cardsRow.implicitWidth + 24)
            contentHeight: 215
            flickableDirection: Flickable.HorizontalFlick
            boundsBehavior: Flickable.StopAtBounds
            clip: true

            RowLayout {
                id: cardsRow
                height: 200
                anchors.centerIn: parent
                spacing: 16

            // 1. Earth Blank Option
            Rectangle {
                width: 220
                height: 200
                radius: 18
                color: eMouse.containsMouse ? "#1E293B" : "#162032"
                border.color: eMouse.containsMouse ? "#38BDF8" : "#334155"
                border.width: 2

                scale: eMouse.pressed ? 0.95 : (eMouse.containsMouse ? 1.03 : 1.0)
                Behavior on scale { NumberAnimation { duration: 100 } }

                Column {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        text: "🌍"
                        font.pixelSize: 42
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Tierra Virgen"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#FFFFFF"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Mapa real 8K\n(continentes vacíos\nlistos para pintar)"
                        font.pixelSize: 12
                        color: "#94A3B8"
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                MouseArea {
                    id: eMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        worldEditor.startAsyncNewWorld("La Tierra", 1);
                        root.accept();
                    }
                }
            }

            // 2. Earth 2026 Option
            Rectangle {
                width: 220
                height: 200
                radius: 18
                color: nMouse.containsMouse ? "#1E293B" : "#162032"
                border.color: nMouse.containsMouse ? "#38BDF8" : "#334155"
                border.width: 2

                scale: nMouse.pressed ? 0.95 : (nMouse.containsMouse ? 1.03 : 1.0)
                Behavior on scale { NumberAnimation { duration: 100 } }

                Column {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        text: "🚩"
                        font.pixelSize: 42
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Tierra 2026"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#FFFFFF"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Mapa real 8K con\npaíses actuales\ny banderas oficiales"
                        font.pixelSize: 12
                        color: "#94A3B8"
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                MouseArea {
                    id: nMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        worldEditor.startAsyncNewWorld("Tierra 2026", 2);
                        root.accept();
                    }
                }
            }

            // 3. Empty World Option
            Rectangle {
                width: 220
                height: 200
                radius: 18
                color: wMouse.containsMouse ? "#1E293B" : "#162032"
                border.color: wMouse.containsMouse ? "#38BDF8" : "#334155"
                border.width: 2

                scale: wMouse.pressed ? 0.95 : (wMouse.containsMouse ? 1.03 : 1.0)
                Behavior on scale { NumberAnimation { duration: 100 } }

                Column {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        text: "🌊"
                        font.pixelSize: 42
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Océano Total"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#FFFFFF"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Océano completo\ncrea tus continentes\ndesde cero"
                        font.pixelSize: 12
                        color: "#94A3B8"
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                MouseArea {
                    id: wMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        worldEditor.startAsyncNewWorld("Mi Mundo", 0);
                        root.accept();
                    }
                }
            }
        }
    }

    Item { height: 6 }

        MatalasButton {
            Layout.alignment: Qt.AlignHCenter
            text: "Cerrar"
            variant: "default"
            onClicked: root.reject()
        }
    }
}
