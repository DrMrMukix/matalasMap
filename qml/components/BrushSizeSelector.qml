import QtQuick 2.15

Rectangle {
    id: root
    property int currentRadius: 16
    signal radiusSelected(int radius)

    width: 220
    height: 60
    radius: 16
    color: "#2C3E50"
    border.color: "#4A6278"
    border.width: 1

    Row {
        anchors.centerIn: parent
        spacing: 12

        Repeater {
            model: [
                { name: "S", radius: 4, dotSize: 10 },
                { name: "M", radius: 16, dotSize: 18 },
                { name: "L", radius: 48, dotSize: 26 },
                { name: "XL", radius: 128, dotSize: 34 }
            ]

            delegate: Rectangle {
                id: btn
                width: 44
                height: 44
                radius: 12
                color: (root.currentRadius === modelData.radius) ? "#FF9800" : (bMouse.containsMouse ? "#34495E" : "transparent")
                border.color: (root.currentRadius === modelData.radius) ? "#FFE0B2" : "#5D6D7E"
                border.width: (root.currentRadius === modelData.radius) ? 2 : 1

                scale: bMouse.pressed ? 0.9 : (bMouse.containsMouse ? 1.08 : 1.0)
                Behavior on scale { NumberAnimation { duration: 100 } }

                Rectangle {
                    anchors.centerIn: parent
                    width: modelData.dotSize
                    height: modelData.dotSize
                    radius: width / 2
                    color: (root.currentRadius === modelData.radius) ? "#FFFFFF" : "#ECEFF1"
                }

                MouseArea {
                    id: bMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.radiusSelected(modelData.radius)
                }
            }
        }
    }
}
