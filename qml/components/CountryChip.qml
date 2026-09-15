import QtQuick 2.15

Rectangle {
    id: root
    property string countryName: "Nuevo País"
    property color countryColor: "#FF5722"
    property string countryFlag: ""
    signal clicked()

    width: 190
    height: 56
    radius: 16
    color: "#1E293B"
    border.color: "#334155"
    border.width: 1

    scale: chipMouse.pressed ? 0.95 : (chipMouse.containsMouse ? 1.04 : 1.0)
    Behavior on scale { NumberAnimation { duration: 100 } }

    Row {
        anchors.centerIn: parent
        spacing: 10

        // Flag or Color Circle
        Rectangle {
            width: 36
            height: 26
            radius: 4
            color: root.countryColor
            border.color: "#FFFFFF"
            border.width: 1.5
            clip: true
            anchors.verticalCenter: parent.verticalCenter

            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: root.countryFlag.length > 0 ? worldEditor.resolveAssetUrl(root.countryFlag) : ""
                sourceSize: Qt.size(72, 52)
                visible: root.countryFlag.length > 0
                asynchronous: true
            }
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            Text {
                text: "País Activo"
                font.pixelSize: 10
                color: "#38BDF8"
                font.bold: true
            }
            Text {
                text: root.countryName
                font.pixelSize: 13
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
