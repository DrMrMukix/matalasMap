import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root

    property string iconText: "✏️"
    property string labelText: "Pincel"
    property bool isSelected: false
    property color activeColor: "#FF9800"
    property color baseColor: "#2C3E50"

    signal clicked()

    width: 72
    height: 72
    radius: 16
    color: isSelected ? activeColor : (mouseArea.containsMouse ? "#34495E" : baseColor)

    border.color: isSelected ? "#FFE0B2" : "#4A6278"
    border.width: isSelected ? 3 : 1

    Behavior on color { ColorAnimation { duration: 150 } }
    Behavior on scale { NumberAnimation { duration: 100 } }

    scale: mouseArea.pressed ? 0.92 : (mouseArea.containsMouse ? 1.05 : 1.0)

    Column {
        anchors.centerIn: parent
        spacing: 2

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.iconText
            font.pixelSize: 28
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.labelText
            font.pixelSize: 11
            font.bold: true
            color: root.isSelected ? "#FFFFFF" : "#ECEFF1"
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
