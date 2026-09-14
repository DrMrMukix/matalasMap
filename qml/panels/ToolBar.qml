import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"

Rectangle {
    id: root

    signal toggleCountryPanel()

    height: 84
    radius: 24
    color: "#0F172A"
    border.color: "#334155"
    border.width: 2

    RowLayout {
        anchors.centerIn: parent
        spacing: 14

        // Tool Buttons
        BigToolButton {
            iconText: "✏️"
            labelText: (worldEditor.activeMode === 0) ? "Tierra" : "Pintar"
            isSelected: (worldEditor.activeTool === 0)
            activeColor: (worldEditor.activeMode === 0) ? "#10B981" : "#3B82F6"
            onClicked: worldEditor.activeTool = 0
        }

        BigToolButton {
            iconText: "🧽"
            labelText: (worldEditor.activeMode === 0) ? "Océano" : "Borrar"
            isSelected: (worldEditor.activeTool === 1)
            activeColor: "#EF4444"
            onClicked: worldEditor.activeTool = 1
        }

        BigToolButton {
            iconText: "🪣"
            labelText: "Relleno"
            isSelected: (worldEditor.activeTool === 2)
            activeColor: "#F59E0B"
            onClicked: worldEditor.activeTool = 2
        }

        BigToolButton {
            iconText: "🎯"
            labelText: "Selector"
            isSelected: (worldEditor.activeTool === 3)
            activeColor: "#8B5CF6"
            onClicked: worldEditor.activeTool = 3
        }

        // Divider
        Rectangle {
            width: 2
            height: 48
            color: "#334155"
        }

        // Brush Size
        BrushSizeSelector {
            currentRadius: worldEditor.brushRadius
            onRadiusSelected: function(r) {
                worldEditor.brushRadius = r
            }
        }

        // Country Chip (Visible in Political Mode)
        CountryChip {
            visible: (worldEditor.activeMode === 1)
            countryName: {
                var c = worldEditor.countries.data(worldEditor.countries.index(0, 0), 258);
                // Search active country in model
                for (var i = 0; i < worldEditor.countries.rowCount(); ++i) {
                    var idx = worldEditor.countries.index(i, 0);
                    if (worldEditor.countries.data(idx, 257) === worldEditor.activeCountryId) {
                        return worldEditor.countries.data(idx, 258);
                    }
                }
                return "Elegir País";
            }
            countryColor: {
                for (var i = 0; i < worldEditor.countries.rowCount(); ++i) {
                    var idx = worldEditor.countries.index(i, 0);
                    if (worldEditor.countries.data(idx, 257) === worldEditor.activeCountryId) {
                        return worldEditor.countries.data(idx, 259);
                    }
                }
                return "#FF5722";
            }
            onClicked: root.toggleCountryPanel()
        }
    }
}
