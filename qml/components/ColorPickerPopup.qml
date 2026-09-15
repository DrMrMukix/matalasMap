import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: root
    width: 340
    height: 480
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property color currentColor: "#E65100"
    property real hue: 0.05
    property real saturation: 0.8
    property real brightness: 0.9

    signal colorAccepted(color col)

    background: Rectangle {
        color: "#0F172A"
        radius: 18
        border.color: "#334155"
        border.width: 2
    }

    function setColor(col) {
        currentColor = col;
        var r = col.r, g = col.g, b = col.b;
        var max = Math.max(r, g, b), min = Math.min(r, g, b);
        var d = max - min;
        var h = 0, s = (max === 0 ? 0 : d / max), v = max;
        if (d !== 0) {
            if (max === r) h = (g - b) / d + (g < b ? 6 : 0);
            else if (max === g) h = (b - r) / d + 2;
            else h = (r - g) / d + 4;
            h /= 6;
        }
        hue = h;
        saturation = s;
        brightness = v;
    }

    function updateColorFromHsv() {
        currentColor = Qt.hsva(hue, saturation, brightness, 1.0)
    }

    contentItem: ColumnLayout {
        spacing: 12
        anchors.fill: parent
        anchors.margins: 14

        // Title
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "🎨 Selector de Color"
                font.pixelSize: 16
                font.bold: true
                color: "#F8FAFC"
                Layout.fillWidth: true
            }

            Button {
                text: "✕"
                flat: true
                onClicked: root.close()
            }
        }

        // 2D Saturation & Brightness picker
        Rectangle {
            id: svPicker
            Layout.fillWidth: true
            height: 160
            radius: 8
            clip: true

            // Base hue color
            Rectangle {
                anchors.fill: parent
                color: Qt.hsva(root.hue, 1.0, 1.0, 1.0)
            }

            // White gradient (horizontal for saturation)
            Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: "#FFFFFF" }
                    GradientStop { position: 1.0; color: "transparent" }
                }
            }

            // Black gradient (vertical for brightness)
            Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop { position: 0.0; color: "transparent" }
                    GradientStop { position: 1.0; color: "#000000" }
                }
            }

            // Cursor Indicator
            Rectangle {
                x: root.saturation * svPicker.width - 9
                y: (1.0 - root.brightness) * svPicker.height - 9
                width: 18
                height: 18
                radius: 9
                color: "transparent"
                border.color: "#FFFFFF"
                border.width: 2

                Rectangle {
                    anchors.centerIn: parent
                    width: 14
                    height: 14
                    radius: 7
                    color: "transparent"
                    border.color: "#000000"
                    border.width: 1
                }
            }

            MouseArea {
                anchors.fill: parent
                function updateSV(mouse) {
                    var s = Math.max(0.0, Math.min(1.0, mouse.x / svPicker.width))
                    var v = Math.max(0.0, Math.min(1.0, 1.0 - (mouse.y / svPicker.height)))
                    root.saturation = s
                    root.brightness = v
                    root.updateColorFromHsv()
                }
                onPressed: updateSV(mouse)
                onPositionChanged: updateSV(mouse)
            }
        }

        // Hue Slider (Rainbow)
        Rectangle {
            id: hueBar
            Layout.fillWidth: true
            height: 24
            radius: 12
            clip: true

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.00; color: "#FF0000" }
                GradientStop { position: 0.17; color: "#FFFF00" }
                GradientStop { position: 0.33; color: "#00FF00" }
                GradientStop { position: 0.50; color: "#00FFFF" }
                GradientStop { position: 0.67; color: "#0000FF" }
                GradientStop { position: 0.83; color: "#FF00FF" }
                GradientStop { position: 1.00; color: "#FF0000" }
            }

            // Hue Handle
            Rectangle {
                x: root.hue * (hueBar.width - 16)
                anchors.verticalCenter: parent.verticalCenter
                width: 16
                height: 24
                radius: 8
                color: "#FFFFFF"
                border.color: "#000000"
                border.width: 1
            }

            MouseArea {
                anchors.fill: parent
                function updateH(mouse) {
                    var h = Math.max(0.0, Math.min(1.0, mouse.x / hueBar.width))
                    root.hue = h
                    root.updateColorFromHsv()
                }
                onPressed: updateH(mouse)
                onPositionChanged: updateH(mouse)
            }
        }

        // Color Preview & Hex Display
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // Big Preview Swatch
            Rectangle {
                width: 48
                height: 48
                radius: 12
                color: root.currentColor
                border.color: "#FFFFFF"
                border.width: 2
            }

            Column {
                Layout.fillWidth: true
                Text {
                    text: root.currentColor.toString().toUpperCase()
                    font.pixelSize: 14
                    font.bold: true
                    font.family: "Monospace"
                    color: "#F8FAFC"
                }
                Text {
                    text: "R: " + Math.round(root.currentColor.r * 255) +
                          "  G: " + Math.round(root.currentColor.g * 255) +
                          "  B: " + Math.round(root.currentColor.b * 255)
                    font.pixelSize: 11
                    color: "#94A3B8"
                }
            }

            Button {
                text: "✓ Elegir"
                font.bold: true
                onClicked: {
                    root.colorAccepted(root.currentColor)
                    root.close()
                }
            }
        }

        // Favorites & Recents
        Text {
            text: "⭐ Colores Sugeridos"
            font.pixelSize: 12
            font.bold: true
            color: "#94A3B8"
        }

        Flow {
            Layout.fillWidth: true
            spacing: 6

            Repeater {
                model: [
                    "#E53935", "#D81B60", "#8E24AA", "#3949AB", "#1E88E5",
                    "#00897B", "#43A047", "#FDD835", "#FB8C00", "#F4511E",
                    "#6D4C41", "#546E7A", "#263238", "#FFFFFF", "#FFD700"
                ]

                delegate: Rectangle {
                    width: 24
                    height: 24
                    radius: 12
                    color: modelData
                    border.color: (root.currentColor === modelData) ? "#FFFFFF" : "#475569"
                    border.width: (root.currentColor === modelData) ? 2 : 1

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.setColor(modelData)
                        }
                    }
                }
            }
        }
    }
}
