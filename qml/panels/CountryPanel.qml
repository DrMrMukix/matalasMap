import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Rectangle {
    id: root

    property color selectedColor: "#E65100"
    readonly property var presetColors: [
        "#E53935", "#D81B60", "#8E24AA", "#5E35B1", "#3949AB",
        "#1E88E5", "#039BE5", "#00ACC1", "#00897B", "#43A047",
        "#7CB342", "#C0CA33", "#FDD835", "#FFB300", "#FB8C00",
        "#F4511E", "#6D4C41", "#546E7A", "#78909C", "#263238"
    ]

    signal openHistoricalLibrary()
    signal openColorPicker()

    implicitWidth: 440
    width: Math.min(parent ? parent.width - 28 : 440, 460)
    radius: 24
    color: "#0F172A"
    border.color: "#334155"
    border.width: 1.5
    clip: true

    // Top subtle gradient bar (inset inside rounded frame)
    Rectangle {
        anchors.top: parent.top
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.right: parent.right
        anchors.rightMargin: 24
        height: 4
        radius: 2
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#38BDF8" }
            GradientStop { position: 0.5; color: "#818CF8" }
            GradientStop { position: 1.0; color: "#F43F5E" }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        // 1. Header with title, count badge, and close button
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                width: 36
                height: 36
                radius: 12
                color: "#1E293B"
                border.color: "#38BDF8"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "🏛️"
                    font.pixelSize: 18
                }
            }

            Column {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    text: "Países y Naciones"
                    font.pixelSize: 17
                    font.bold: true
                    color: "#F8FAFC"
                }
                Text {
                    text: (worldEditor.countries ? worldEditor.countries.count : 0) + " estados en este mundo"
                    font.pixelSize: 11
                    color: "#94A3B8"
                }
            }

            MatalasButton {
                iconText: "✕"
                iconOnly: true
                height: 36
                width: 36
                variant: "ghost"
                onClicked: root.visible = false
            }
        }

        // 2. Button to open Historical Library
        MatalasButton {
            Layout.fillWidth: true
            height: 42
            iconText: "📜"
            text: "Biblioteca Histórica (1650 – 2026)"
            variant: "primary"
            onClicked: root.openHistoricalLibrary()
        }

        // 3. Search Bar
        Rectangle {
            Layout.fillWidth: true
            height: 42
            radius: 12
            color: "#1E293B"
            border.color: searchInput.activeFocus ? "#38BDF8" : "#334155"
            border.width: searchInput.activeFocus ? 2 : 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 10
                spacing: 8

                Text {
                    text: "🔍"
                    font.pixelSize: 14
                }

                TextField {
                    id: searchInput
                    Layout.fillWidth: true
                    placeholderText: "Buscar país por nombre..."
                    placeholderTextColor: "#64748B"
                    color: "#F8FAFC"
                    font.pixelSize: 13
                    background: null
                    onTextChanged: {
                        if (worldEditor.countries) {
                            worldEditor.countries.filterText = text
                        }
                    }
                }

                Rectangle {
                    visible: searchInput.text.length > 0
                    width: 22
                    height: 22
                    radius: 11
                    color: "#334155"

                    Text {
                        anchors.centerIn: parent
                        text: "✕"
                        color: "#FFFFFF"
                        font.pixelSize: 10
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            searchInput.text = ""
                            searchInput.forceActiveFocus()
                        }
                    }
                }
            }
        }

        // 4. Sort Options Row
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                text: "Ordenar:"
                font.pixelSize: 11
                font.bold: true
                color: "#64748B"
            }

            Repeater {
                model: [
                    { label: "🔤 A-Z", mode: 1 },
                    { label: "🔡 Z-A", mode: 2 },
                    { label: "🌍 Área", mode: 3 },
                    { label: "🔢 ID", mode: 0 }
                ]

                delegate: Rectangle {
                    height: 28
                    width: sortText.implicitWidth + 16
                    radius: 14
                    color: (worldEditor.countries && worldEditor.countries.sortMode === modelData.mode)
                           ? "#2563EB"
                           : (sortMa.containsMouse ? "#334155" : "#1E293B")
                    border.color: (worldEditor.countries && worldEditor.countries.sortMode === modelData.mode)
                                  ? "#60A5FA"
                                  : "#334155"

                    Text {
                        id: sortText
                        anchors.centerIn: parent
                        text: modelData.label
                        font.pixelSize: 11
                        font.bold: (worldEditor.countries && worldEditor.countries.sortMode === modelData.mode)
                        color: (worldEditor.countries && worldEditor.countries.sortMode === modelData.mode)
                               ? "#FFFFFF"
                               : "#94A3B8"
                    }

                    MouseArea {
                        id: sortMa
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (worldEditor.countries) {
                                worldEditor.countries.sortMode = modelData.mode
                            }
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }
        }

        // 5. Country List View
        ListView {
            id: countryList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: worldEditor.countries
            spacing: 6
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                active: true
            }

            delegate: Rectangle {
                width: countryList.width
                height: 52
                radius: 14
                color: model.isSelected ? "#1E293B" : (itemMouse.containsMouse ? "#162032" : "#0B111E")
                border.color: model.isSelected ? "#38BDF8" : "#1E293B"
                border.width: model.isSelected ? 2 : 1

                MouseArea {
                    id: itemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: worldEditor.activeCountryId = model.countryId
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 10

                    // Flag & Color Box
                    Rectangle {
                        width: 44
                        height: 30
                        radius: 6
                        color: model.color
                        border.color: "#FFFFFF"
                        border.width: 1.5
                        clip: true

                        Image {
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectCrop
                            source: (model.flagPath && model.flagPath.length > 0) ? worldEditor.resolveAssetUrl(model.flagPath) : ""
                            sourceSize: Qt.size(88, 60)
                            visible: (model.flagPath && model.flagPath.length > 0)
                            asynchronous: true
                        }
                    }

                    // Country Name & Territory Area
                    Column {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            text: model.name
                            font.pixelSize: 14
                            font.bold: true
                            color: "#F8FAFC"
                            elide: Text.ElideRight
                            width: parent.width
                        }

                        Text {
                            text: (model.pixelCount > 0) ? (model.pixelCount.toLocaleString() + " px²") : "Sin territorio"
                            font.pixelSize: 10
                            color: (model.pixelCount > 0) ? "#38BDF8" : "#64748B"
                        }
                    }

                    // Delete country button
                    MatalasButton {
                        iconText: "🗑️"
                        iconOnly: true
                        height: 32
                        width: 32
                        variant: "danger"
                        onClicked: worldEditor.deleteCountry(model.countryId)
                    }
                }
            }

            // Empty state if search has 0 results
            Item {
                anchors.centerIn: parent
                visible: countryList.count === 0
                width: 220
                height: 120

                Column {
                    anchors.centerIn: parent
                    spacing: 6
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "🔍"
                        font.pixelSize: 28
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "No se encontraron países"
                        font.pixelSize: 13
                        font.bold: true
                        color: "#94A3B8"
                    }
                }
            }
        }

        // 6. Collapsible New Country Creator
        Rectangle {
            id: creatorBox
            Layout.fillWidth: true
            height: isExpanded ? 168 : 38
            radius: 14
            color: "#162032"
            border.color: "#334155"
            border.width: 1
            clip: true

            property bool isExpanded: false
            Behavior on height { NumberAnimation { duration: 150 } }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                // Toggle Header
                Item {
                    Layout.fillWidth: true
                    implicitHeight: toggleHeaderRow.implicitHeight

                    RowLayout {
                        id: toggleHeaderRow
                        anchors.fill: parent

                        Text {
                            text: "+ Inventar Nueva Nación"
                            font.pixelSize: 12
                            font.bold: true
                            color: "#38BDF8"
                            Layout.fillWidth: true
                        }

                        Text {
                            text: creatorBox.isExpanded ? "▲" : "▼"
                            font.pixelSize: 11
                            color: "#94A3B8"
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: creatorBox.isExpanded = !creatorBox.isExpanded
                    }
                }

                TextField {
                    id: nameField
                    placeholderText: "Nombre del país..."
                    text: "Nueva Nación"
                    Layout.fillWidth: true
                    font.pixelSize: 13
                    font.bold: true
                    color: "#FFFFFF"
                    placeholderTextColor: "#64748B"
                    leftPadding: 10
                    rightPadding: 10
                    background: Rectangle {
                        color: "#0F172A"
                        radius: 8
                        border.color: nameField.activeFocus ? "#3B82F6" : "#334155"
                        border.width: 1.5
                    }
                }

                // Color Swatches
                Flow {
                    Layout.fillWidth: true
                    spacing: 6

                    Repeater {
                        model: root.presetColors
                        delegate: Rectangle {
                            width: 20
                            height: 20
                            radius: 10
                            color: modelData
                            border.color: (root.selectedColor === modelData) ? "#FFFFFF" : "#334155"
                            border.width: (root.selectedColor === modelData) ? 2.5 : 1

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

                MatalasButton {
                    Layout.fillWidth: true
                    height: 36
                    iconText: "✨"
                    text: "Crear País"
                    variant: "success"
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
