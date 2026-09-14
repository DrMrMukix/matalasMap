import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property color selectedColor: "#E65100"
    readonly property var presetColors: [
        "#E53935", "#D81B60", "#8E24AA", "#5E35B1", "#3949AB",
        "#1E88E5", "#039BE5", "#00ACC1", "#00897B", "#43A047",
        "#7CB342", "#C0CA33", "#FDD835", "#FFB300", "#FB8C00",
        "#F4511E", "#6D4C41", "#546E7A", "#78909C", "#263238"
    ]

    width: 320
    height: parent.height - 140
    radius: 20
    color: "#1E293B"
    border.color: "#334155"
    border.width: 2

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Header
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "🏛️ Países y Naciones"
                font.pixelSize: 18
                font.bold: true
                color: "#F8FAFC"
                Layout.fillWidth: true
            }

            Button {
                text: "✕"
                flat: true
                onClicked: root.visible = false
            }
        }

        // Country List
        ListView {
            id: countryList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: worldEditor.countries
            spacing: 8

            delegate: Rectangle {
                width: countryList.width
                height: 52
                radius: 12
                color: model.isSelected ? "#334155" : (itemMouse.containsMouse ? "#2A3749" : "#0F172A")
                border.color: model.isSelected ? "#38BDF8" : "#1E293B"
                border.width: model.isSelected ? 2 : 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Rectangle {
                        width: 28
                        height: 28
                        radius: 14
                        color: model.color
                        border.color: "#FFFFFF"
                        border.width: 2
                    }

                    Text {
                        text: model.name
                        font.pixelSize: 14
                        font.bold: true
                        color: "#FFFFFF"
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    Button {
                        text: "🗑️"
                        flat: true
                        onClicked: worldEditor.deleteCountry(model.countryId)
                    }
                }

                MouseArea {
                    id: itemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: worldEditor.activeCountryId = model.countryId
                }
            }
        }

        // New Country Creator
        Rectangle {
            Layout.fillWidth: true
            height: 160
            radius: 14
            color: "#0F172A"
            border.color: "#334155"
            border.width: 1
            clip: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                Text {
                    text: "+ Inventar País"
                    font.pixelSize: 13
                    font.bold: true
                    color: "#38BDF8"
                }

                TextField {
                    id: nameField
                    placeholderText: "Nombre del país..."
                    text: "Nueva Nación"
                    Layout.fillWidth: true
                    color: "#FFFFFF"
                    background: Rectangle {
                        color: "#1E293B"
                        radius: 8
                        border.color: "#475569"
                    }
                }

                // Color Swatches
                Flow {
                    Layout.fillWidth: true
                    spacing: 6

                    Repeater {
                        model: root.presetColors
                        delegate: Rectangle {
                            width: 22
                            height: 22
                            radius: 11
                            color: modelData
                            border.color: (root.selectedColor === modelData) ? "#FFFFFF" : "transparent"
                            border.width: (root.selectedColor === modelData) ? 2 : 0

                            scale: sMouse.pressed ? 0.85 : (sMouse.containsMouse ? 1.2 : 1.0)
                            Behavior on scale { NumberAnimation { duration: 80 } }

                            MouseArea {
                                id: sMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.selectedColor = modelData
                            }
                        }
                    }
                }

                Button {
                    Layout.fillWidth: true
                    text: "✨ Crear País"
                    font.bold: true
                    onClicked: {
                        if (nameField.text.trim().length > 0) {
                            worldEditor.createCountry(nameField.text.trim(), root.selectedColor);
                        }
                    }
                }
            }
        }
    }
}
