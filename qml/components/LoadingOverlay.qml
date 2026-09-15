import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    anchors.fill: parent
    z: 9999

    visible: opacity > 0.0
    opacity: (typeof worldEditor !== "undefined" && worldEditor.isLoading) ? 1.0 : 0.0
    color: "#070D1AE8"

    Behavior on opacity {
        NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
    }

    // Modal click shield: blocks all mouse, hover, and touch events
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true
        cursorShape: Qt.BusyCursor
        acceptedButtons: Qt.AllButtons
        onWheel: function(wheel) { wheel.accepted = true; }
    }

    // Center Card
    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 32, 420)
        height: 240
        radius: 24
        color: "#0F172AF8"
        border.color: "#38BDF8"
        border.width: 1.5

        // Outer glow accent bar
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 4
            radius: 2
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#38BDF8" }
                GradientStop { position: 0.5; color: "#818CF8" }
                GradientStop { position: 1.0; color: "#06B6D4" }
            }
        }

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 16
            width: parent.width - 48

            // Animated Rotating & Pulsing Orb Indicator
            Item {
                Layout.alignment: Qt.AlignHCenter
                width: 64
                height: 64

                // Outer rotating gradient ring
                Rectangle {
                    id: outerRing
                    anchors.fill: parent
                    radius: 32
                    color: "transparent"
                    border.color: "#38BDF8"
                    border.width: 3

                    RotationAnimation on rotation {
                        from: 0
                        to: 360
                        duration: 1200
                        loops: Animation.Infinite
                        running: root.visible
                    }
                }

                // Inner pulsing icon
                Text {
                    anchors.centerIn: parent
                    text: "🌍"
                    font.pixelSize: 28

                    SequentialAnimation on scale {
                        loops: Animation.Infinite
                        running: root.visible
                        NumberAnimation { from: 0.9; to: 1.15; duration: 800; easing.type: Easing.InOutSine }
                        NumberAnimation { from: 1.15; to: 0.9; duration: 800; easing.type: Easing.InOutSine }
                    }
                }
            }

            // Status message
            Column {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: (typeof worldEditor !== "undefined" && worldEditor.loadingMessage.length > 0)
                          ? worldEditor.loadingMessage
                          : "Procesando mapa 8K..."
                    font.pixelSize: 16
                    font.bold: true
                    color: "#F8FAFC"
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Núcleo Rust de alto rendimiento · 33.5M píxeles"
                    font.pixelSize: 11
                    color: "#94A3B8"
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            // Indeterminate Progress Bar
            Rectangle {
                Layout.fillWidth: true
                height: 6
                radius: 3
                color: "#1E293B"
                clip: true

                Rectangle {
                    id: progressGlow
                    width: parent.width * 0.4
                    height: parent.height
                    radius: 3
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "transparent" }
                        GradientStop { position: 0.5; color: "#38BDF8" }
                        GradientStop { position: 1.0; color: "#818CF8" }
                    }

                    SequentialAnimation on x {
                        loops: Animation.Infinite
                        running: root.visible
                        NumberAnimation {
                            from: -progressGlow.width
                            to: progressGlow.parent.width
                            duration: 1100
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
            }
        }
    }
}
