import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Matalas 1.0

Dialog {
    id: root
    title: ""
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent.width - 20, 1020)
    height: Math.min(parent.height - 20, 840)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    signal nationSelected(var nation)

    HistoricalNationsModel {
        id: nationsModel
    }

    Overlay.modal: Rectangle {
        color: "#AA000814"
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

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
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#38BDF8" }
                GradientStop { position: 0.5; color: "#3B82F6" }
                GradientStop { position: 1.0; color: "#8B5CF6" }
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 14

        // Top App Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Rectangle {
                width: 44
                height: 44
                radius: 14
                color: "#1E293B"
                border.color: "#38BDF8"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "🏛️"
                    font.pixelSize: 22
                }
            }

            Column {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    text: "Biblioteca Histórica de Naciones y Estados"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#F8FAFC"
                }
                Text {
                    text: "Colección completa 1650 – 2026 · Wikipedia & Wikidata (100% Offline)"
                    font.pixelSize: 12
                    color: "#94A3B8"
                }
            }

            Rectangle {
                width: 36
                height: 36
                radius: 18
                color: closeBtnMouse.containsMouse ? "#EF444433" : "#1E293B"
                border.color: closeBtnMouse.containsMouse ? "#EF4444" : "#334155"

                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    color: closeBtnMouse.containsMouse ? "#EF4444" : "#94A3B8"
                    font.bold: true
                    font.pixelSize: 15
                }

                MouseArea {
                    id: closeBtnMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.close()
                }
            }
        }

        // Search Field Card
        Rectangle {
            Layout.fillWidth: true
            height: 48
            radius: 14
            color: "#1E293B"
            border.color: searchField.activeFocus ? "#38BDF8" : "#334155"
            border.width: searchField.activeFocus ? 2 : 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 10

                Text {
                    text: "🔍"
                    font.pixelSize: 16
                }

                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    placeholderText: "Buscar por nombre (ej. España, Imperio Otomano) o año (ej. 1810, 1945)..."
                    placeholderTextColor: "#64748B"
                    color: "#F8FAFC"
                    font.pixelSize: 14
                    background: null
                    onTextChanged: nationsModel.filterText = text
                }

                Rectangle {
                    visible: searchField.text.length > 0
                    width: 24
                    height: 24
                    radius: 12
                    color: "#334155"
                    Text {
                        anchors.centerIn: parent
                        text: "✕"
                        color: "#FFFFFF"
                        font.pixelSize: 11
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            searchField.text = ""
                            searchField.forceActiveFocus()
                        }
                    }
                }

                Rectangle {
                    height: 26
                    width: countText.implicitWidth + 16
                    radius: 13
                    color: "#0F172A"
                    border.color: "#38BDF8"

                    Text {
                        id: countText
                        anchors.centerIn: parent
                        text: nationsModel.count + " estados"
                        color: "#38BDF8"
                        font.bold: true
                        font.pixelSize: 11
                    }
                }
            }
        }

        // Epoch Timeline Filter Chips (Flickable for Touch & Tablet)
        Flickable {
            Layout.fillWidth: true
            height: 34
            contentWidth: epochRow.width
            flickableDirection: Flickable.HorizontalFlick
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            Row {
                id: epochRow
                spacing: 8

                Repeater {
                    model: [
                        { label: "🌍 Todas las Épocas", query: "" },
                        { label: "⚔️ Siglo XVII (1650)", query: "1650" },
                        { label: "👑 Siglo XVIII (1750)", query: "1750" },
                        { label: "🦅 Era Napoleónica (1804)", query: "1804" },
                        { label: "🚂 Siglo XIX (1870)", query: "1870" },
                        { label: "🎖️ Guerras Mundiales (1940)", query: "1940" },
                        { label: "🚀 Guerra Fría (1975)", query: "1975" },
                        { label: "🌐 Mundo Actual (2026)", query: "2026" }
                    ]

                    delegate: Rectangle {
                        height: 32
                        width: chipLabel.implicitWidth + 24
                        radius: 16
                        color: (searchField.text === modelData.query) ? "#2563EB" : "#1E293B"
                        border.color: (searchField.text === modelData.query) ? "#60A5FA" : "#334155"
                        border.width: (searchField.text === modelData.query) ? 2 : 1

                        Text {
                            id: chipLabel
                            anchors.centerIn: parent
                            text: modelData.label
                            font.pixelSize: 12
                            font.bold: (searchField.text === modelData.query)
                            color: (searchField.text === modelData.query) ? "#FFFFFF" : "#CBD5E1"
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                searchField.text = modelData.query
                            }
                        }
                    }
                }
            }
        }

        // Sorting Row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "Ordenar por:"
                font.pixelSize: 11
                font.bold: true
                color: "#64748B"
            }

            Repeater {
                model: [
                    { label: "🔤 A-Z", mode: 1 },
                    { label: "🔡 Z-A", mode: 2 },
                    { label: "⏳ Más Antiguo", mode: 3 },
                    { label: "🚀 Moderno", mode: 4 },
                    { label: "📜 Original", mode: 0 }
                ]

                delegate: Rectangle {
                    height: 26
                    width: sLabel.implicitWidth + 16
                    radius: 13
                    color: (nationsModel.sortMode === modelData.mode)
                           ? "#2563EB"
                           : (sMa.containsMouse ? "#334155" : "#1E293B")
                    border.color: (nationsModel.sortMode === modelData.mode)
                                  ? "#60A5FA"
                                  : "#334155"

                    Text {
                        id: sLabel
                        anchors.centerIn: parent
                        text: modelData.label
                        font.pixelSize: 11
                        font.bold: (nationsModel.sortMode === modelData.mode)
                        color: (nationsModel.sortMode === modelData.mode) ? "#FFFFFF" : "#94A3B8"
                    }

                    MouseArea {
                        id: sMa
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: nationsModel.sortMode = modelData.mode
                    }
                }
            }

            Item { Layout.fillWidth: true }
        }

        // Nations List View
        ListView {
            id: nationsView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: nationsModel
            spacing: 10

            ScrollBar.vertical: ScrollBar {
                active: true
                policy: ScrollBar.AsNeeded
            }

            delegate: Rectangle {
                id: itemCard
                width: nationsView.width
                height: 76
                radius: 16
                color: dMouse.containsMouse ? "#27354A" : "#1E293B"
                border.color: dMouse.containsMouse ? "#38BDF8" : "#334155"
                border.width: dMouse.containsMouse ? 2 : 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 14
                    anchors.topMargin: 8
                    anchors.bottomMargin: 8
                    spacing: 14

                    // Authentic National Flag Card Preview
                    Rectangle {
                        width: 82
                        height: 54
                        radius: 8
                        color: "#0F172A"
                        border.color: "#475569"
                        border.width: 1.5
                        clip: true

                        Image {
                            id: flagImg
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectFit
                            source: model.flagPath.length > 0 ? worldEditor.resolveAssetUrl(model.flagPath) : ""
                            sourceSize: Qt.size(164, 108)
                            asynchronous: true
                            cache: true
                        }

                        // Fallback color card if image not found or loading
                        Rectangle {
                            anchors.fill: parent
                            color: model.color
                            visible: flagImg.status !== Image.Ready
                            opacity: 0.85

                            Text {
                                anchors.centerIn: parent
                                text: "🚩"
                                font.pixelSize: 22
                            }
                        }
                    }

                    // Nation Info
                    Column {
                        Layout.fillWidth: true
                        spacing: 4

                        Row {
                            spacing: 8
                            Text {
                                text: model.name
                                font.pixelSize: 16
                                font.bold: true
                                color: "#FFFFFF"
                                elide: Text.ElideRight
                            }

                            // Period badge
                            Rectangle {
                                height: 22
                                width: periodText.implicitWidth + 14
                                radius: 11
                                color: "#0F172A"
                                border.color: "#38BDF8"
                                anchors.verticalCenter: parent.verticalCenter

                                Text {
                                    id: periodText
                                    anchors.centerIn: parent
                                    text: model.period
                                    font.pixelSize: 11
                                    font.bold: true
                                    color: "#38BDF8"
                                }
                            }
                        }

                        Row {
                            spacing: 8
                            // Color circle preview
                            Rectangle {
                                width: 14
                                height: 14
                                radius: 7
                                color: model.color
                                border.color: "#FFFFFF"
                                border.width: 1.5
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: model.wikidataId.length > 0 ? ("Wikidata: " + model.wikidataId) : "Histórico"
                                font.pixelSize: 12
                                color: "#94A3B8"
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: "· " + model.license
                                font.pixelSize: 11
                                color: "#64748B"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }

                    // Action Button: Use in map
                    Rectangle {
                        width: 120
                        height: 42
                        radius: 21
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: btnMouse.containsMouse ? "#2563EB" : "#3B82F6" }
                            GradientStop { position: 1.0; color: btnMouse.containsMouse ? "#1D4ED8" : "#2563EB" }
                        }
                        border.color: "#60A5FA"
                        border.width: 1

                        Row {
                            anchors.centerIn: parent
                            spacing: 6
                            Text {
                                text: "✏️"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: "¡Pintar!"
                                color: "#FFFFFF"
                                font.bold: true
                                font.pixelSize: 13
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            id: btnMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                var nation = nationsModel.getNation(index)
                                root.nationSelected(nation)
                                root.close()
                            }
                        }
                    }
                }

                MouseArea {
                    id: dMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    acceptedButtons: Qt.NoButton
                }
            }
        }
    }
}
