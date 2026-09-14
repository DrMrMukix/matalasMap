import QtQuick 2.15

Rectangle {
    id: root
    property string countryName: "Nuevo País"
    property color countryColor: "#FF5722"
    signal clicked()

    width: 180
    height: 60
    radius: 16
    color: "#2C3E50"
    border.color: "#4A6278"
    border.width: 1

    scale: chipMouse.pressed ? 0.95 : (chipMouse.containsMouse ? 1.04 : 1.0)
    Behavior on scale { NumberAnimation { duration: 100 } }

    Row {
        anchors.centerIn: parent
        spacing: 10

        Rectangle {
            width: 32
            height: 32
            radius: 16
            color: root.countryColor
            border.color: "#FFFFFF"
            border.width: 2
            anchors.verticalCenter: parent.verticalCenter
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            Text {
                text: "País Activo"
                font.pixelSize: 10
                color: "#90A4AE"
                font.bold: true
            }
            Text {
                text: root.countryName
                font.pixelSize: 14
                font.bold: true
                color: "#FFFFFF"
                elide: Text.ElideRight
                width: 110
            }
        }
    }

    MouseArea {
        id: chipMouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
