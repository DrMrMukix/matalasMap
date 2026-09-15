import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Dialog {
    id: root
    title: ""
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent.width * 0.9, 640)
    height: Math.min(parent.height * 0.9, 580)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property var existingWorlds: []
    property bool fileAlreadyExists: false

    function refreshSaves() {
        existingWorlds = worldEditor.getSavedWorldsList();
        checkNameExists();
    }

    function checkNameExists() {
        if (typeof worldEditor !== "undefined") {
            fileAlreadyExists = worldEditor.checkSaveExists(nameField.text.trim());
        }
    }

    onOpened: {
        nameField.text = worldEditor.worldName || "Mi_Mundo";
        nameField.selectAll();
        nameField.forceActiveFocus();
        refreshSaves();
    }

    background: Rectangle {
        color: "#0F172A"
        radius: 24
        border.color: "#334155"
        border.width: 2

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 5
            radius: 2.5
            color: root.fileAlreadyExists ? "#F59E0B" : "#10B981"
            Behavior on color { ColorAnimation { duration: 150 } }
        }
    }

    contentItem: ColumnLayout {
        spacing: 16

        // 1. Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Rectangle {
                width: 44
                height: 44
                radius: 14
                color: "#1E293B"
                border.color: root.fileAlreadyExists ? "#F59E0B" : "#10B981"
                border.width: 1.5

                Text {
                    anchors.centerIn: parent
                    text: "💾"
                    font.pixelSize: 22
                }
            }

            Column {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    text: "Guardar Partida de Mundo"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#F8FAFC"
                }
                Text {
                    text: "Archivo binario comprimido de alta fidelidad (.matalas)"
                    font.pixelSize: 12
                    color: "#94A3B8"
                }
            }

            MatalasButton {
                iconText: "✕"
                iconOnly: true
                height: 36
                width: 36
                variant: "ghost"
                onClicked: root.close()
            }
        }

        // 2. Name Input Field Card
        Rectangle {
            Layout.fillWidth: true
            height: 80
            radius: 16
            color: "#162032"
            border.color: nameField.activeFocus ? "#38BDF8" : "#334155"
            border.width: 1.5

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 4

                Text {
                    text: "Nombre del archivo / partida:"
                    font.pixelSize: 11
                    font.bold: true
                    color: "#94A3B8"
                }

                TextField {
                    id: nameField
                    Layout.fillWidth: true
                    font.pixelSize: 14
                    font.bold: true
                    color: "#FFFFFF"
                    placeholderText: "Escribe el nombre de tu mundo..."
                    placeholderTextColor: "#64748B"
                    background: null
                    onTextChanged: root.checkNameExists()
                    onAccepted: executeSave()
                }
            }
        }

        // 3. Status Alert Banner (Overwrite Warning vs Ready)
        Rectangle {
            Layout.fillWidth: true
            height: 42
            radius: 12
            color: root.fileAlreadyExists ? "#F59E0B22" : "#10B98122"
            border.color: root.fileAlreadyExists ? "#F59E0B" : "#10B981"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                Text {
                    text: root.fileAlreadyExists ? "⚠️" : "✓"
                    font.pixelSize: 15
                }

                Text {
                    Layout.fillWidth: true
                    text: root.fileAlreadyExists
                          ? "Ya existe una partida con este nombre. Se sobreescribirá el archivo."
                          : "Nombre libre. Se creará un nuevo archivo de partida."
                    font.pixelSize: 12
                    font.bold: true
                    color: root.fileAlreadyExists ? "#FDE68A" : "#A7F3D0"
                }
            }
        }

        // 4. Existing Saves Picker Header
        Text {
            text: "O selecciona una partida existente para actualizar:"
            font.pixelSize: 12
            font.bold: true
            color: "#64748B"
        }

        // 5. Existing Saves List
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 14
            color: "#0B111E88"
            border.color: "#1E293B"
            border.width: 1
            clip: true

            ListView {
                id: savesView
                anchors.fill: parent
                anchors.margins: 6
                model: root.existingWorlds
                spacing: 6
                clip: true

                ScrollBar.vertical: ScrollBar {
                    active: true
                    policy: ScrollBar.AsNeeded
                }

                delegate: Rectangle {
                    width: savesView.width - 12
                    height: 44
                    radius: 10
                    color: (nameField.text.trim() === modelData.name)
                           ? "#2563EB44"
                           : (sMouse.containsMouse ? "#1E293B" : "#162032")
                    border.color: (nameField.text.trim() === modelData.name)
                                  ? "#38BDF8"
                                  : "#334155"
                    border.width: (nameField.text.trim() === modelData.name) ? 1.5 : 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 10

                        Text {
                            text: "🗺️"
                            font.pixelSize: 16
                        }

                        Text {
                            Layout.fillWidth: true
                            text: modelData.name
                            font.pixelSize: 13
                            font.bold: true
                            color: "#F8FAFC"
                            elide: Text.ElideRight
                        }

                        Text {
                            text: modelData.modified || ""
                            font.pixelSize: 11
                            color: "#94A3B8"
                        }
                    }

                    MouseArea {
                        id: sMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            nameField.text = modelData.name;
                            root.checkNameExists();
                        }
                    }
                }
            }
        }

        // 6. Action Buttons Bottom Row
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            MatalasButton {
                text: "Cancelar"
                variant: "ghost"
                height: 42
                onClicked: root.close()
            }

            Item { Layout.fillWidth: true }

            MatalasButton {
                height: 42
                iconText: root.fileAlreadyExists ? "⚠️" : "💾"
                text: root.fileAlreadyExists ? "Sobreescribir Partida" : "Guardar Partida"
                variant: root.fileAlreadyExists ? "warning" : "success"
                enabledState: nameField.text.trim().length > 0
                onClicked: executeSave()
            }
        }
    }

    function executeSave() {
        var cleanName = nameField.text.trim();
        if (cleanName.length === 0) return;

        if (worldEditor.saveWorldNamed(cleanName, true)) {
            toast.show("¡Partida \"" + cleanName + "\" guardada exitosamente!");
            root.close();
        } else {
            toast.show("Error al guardar la partida \"" + cleanName + "\"");
        }
    }
}
