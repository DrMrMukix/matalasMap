import QtQuick 2.15
import QtQuick.Controls 2.15

Dialog {
    id: root
    title: "Crear Nuevo Mundo"
    modal: true
    anchors.centerIn: parent
    width: Math.min(540, parent.width - 40)
    standardButtons: Dialog.Cancel

    background: Rectangle {
        color: "#1E293B"
        radius: 20
        border.color: "#334155"
        border.width: 2
    }

    header: Rectangle {
        color: "transparent"
        height: 60
        Text {
            anchors.centerIn: parent
            text: "🌟 ¡Crea tu Mundo!"
            font.pixelSize: 22
            font.bold: true
            color: "#F8FAFC"
        }
    }

    contentItem: Column {
        spacing: 20
        padding: 10

        Text {
            text: "¿Cómo quieres empezar tu mapa?"
            font.pixelSize: 15
            color: "#94A3B8"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            spacing: 20
            anchors.horizontalCenter: parent.horizontalCenter

            // Earth Option
            Rectangle {
                id: earthCard
                width: 200
                height: 180
                radius: 18
                color: eMouse.containsMouse ? "#2E4057" : "#0F172A"
                border.color: eMouse.containsMouse ? "#38BDF8" : "#334155"
                border.width: 2

                scale: eMouse.pressed ? 0.95 : (eMouse.containsMouse ? 1.03 : 1.0)
                Behavior on scale { NumberAnimation { duration: 100 } }

                Column {
                    anchors.centerIn: parent
                    spacing: 12

                    Text {
                        text: "🌎"
                        font.pixelSize: 48
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "La Tierra"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#FFFFFF"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Mapa real 8K\nlisto para pintar"
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
                        worldEditor.newWorld("La Tierra", true);
                        root.accept();
                    }
                }
            }

            // Empty World Option
            Rectangle {
                id: emptyCard
                width: 200
                height: 180
                radius: 18
                color: wMouse.containsMouse ? "#2E4057" : "#0F172A"
                border.color: wMouse.containsMouse ? "#38BDF8" : "#334155"
                border.width: 2

                scale: wMouse.pressed ? 0.95 : (wMouse.containsMouse ? 1.03 : 1.0)
                Behavior on scale { NumberAnimation { duration: 100 } }

                Column {
                    anchors.centerIn: parent
                    spacing: 12

                    Text {
                        text: "🌊"
                        font.pixelSize: 48
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Mundo Vacío"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#FFFFFF"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Océano completo\ncrea tus continentes"
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
                        worldEditor.newWorld("Mi Mundo", false);
                        root.accept();
                    }
                }
            }
        }
    }
}
