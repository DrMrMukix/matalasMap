import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property int currentRadius: 16
    signal radiusSelected(int radius)

    width: 290
    height: 56
    radius: 16
    color: "#1E293B"
    border.color: "#334155"
    border.width: 1

    Row {
        anchors.centerIn: parent
        spacing: 8

        // Quick Preset Dots
        Repeater {
            model: [
                { name: "Puntito", radius: 2, dotSize: 6 },
                { name: "S", radius: 8, dotSize: 12 },
                { name: "M", radius: 24, dotSize: 20 },
                { name: "L", radius: 64, dotSize: 28 },
                { name: "XL", radius: 140, dotSize: 34 }
            ]

            delegate: Rectangle {
                width: 40
                height: 40
                radius: 10
                color: (root.currentRadius === modelData.radius) ? "#3B82F6" : (bMouse.containsMouse ? "#334155" : "transparent")
                border.color: (root.currentRadius === modelData.radius) ? "#60A5FA" : "#475569"
                border.width: (root.currentRadius === modelData.radius) ? 2 : 1

                scale: bMouse.pressed ? 0.9 : (bMouse.containsMouse ? 1.08 : 1.0)
                Behavior on scale { NumberAnimation { duration: 80 } }

                Rectangle {
                    anchors.centerIn: parent
                    width: modelData.dotSize
                    height: modelData.dotSize
                    radius: width / 2
                    color: (root.currentRadius === modelData.radius) ? "#FFFFFF" : "#CBD5E1"
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

        // Fine tuning button with radius text
        Rectangle {
            width: 58
            height: 40
            radius: 10
            color: sliderPopup.visible ? "#2563EB" : "#0F172A"
            border.color: "#334155"
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: 2
                Text {
                    text: root.currentRadius + "px"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#F8FAFC"
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: sliderPopup.visible = !sliderPopup.visible
            }
        }
    }

    // Floating Fine-tuning Slider Popup
    Popup {
        id: sliderPopup
        y: -100
        x: (root.width - width) / 2
        width: 260
        height: 80
        padding: 12
        background: Rectangle {
            color: "#0F172A"
            radius: 14
            border.color: "#334155"
            border.width: 2
        }

        Column {
            anchors.fill: parent
            spacing: 4

            Row {
                width: parent.width
                Text {
                    text: "Tamaño exacto del pincel:"
                    font.pixelSize: 11
                    color: "#94A3B8"
                }
                Item { width: 10 }
                Text {
                    text: sizeSlider.value + " px"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#38BDF8"
                }
            }

            Slider {
                id: sizeSlider
                width: parent.width
                from: 1
                to: 160
                stepSize: 1
                value: root.currentRadius
                onMoved: {
                    root.radiusSelected(Math.round(value))
                }
            }
        }
    }
}
