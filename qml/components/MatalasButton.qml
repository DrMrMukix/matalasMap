import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property string text: ""
    property string iconText: ""
    property string variant: "default" // "default", "primary", "success", "warning", "danger", "ghost"
    property bool iconOnly: false
    property bool isSelected: false
    property int pointSize: 13
    property bool bold: true
    property real customRadius: 14
    property bool enabledState: true

    signal clicked()

    implicitWidth: iconOnly ? height : Math.max(44, buttonContentRow.implicitWidth + 24)
    implicitHeight: 44
    height: 44
    radius: customRadius

    opacity: enabledState ? 1.0 : 0.45

    // Color definitions based on variant
    readonly property color colDefault: "#1E293B"
    readonly property color colDefaultHover: "#2A3749"
    readonly property color colDefaultBorder: "#334155"

    readonly property color colPrimary: "#2563EB"
    readonly property color colPrimaryHover: "#3B82F6"
    readonly property color colPrimaryBorder: "#60A5FA"

    readonly property color colSuccess: "#059669"
    readonly property color colSuccessHover: "#10B981"
    readonly property color colSuccessBorder: "#34D399"

    readonly property color colWarning: "#D97706"
    readonly property color colWarningHover: "#F59E0B"
    readonly property color colWarningBorder: "#FBBF24"

    readonly property color colDanger: "#DC2626"
    readonly property color colDangerHover: "#EF4444"
    readonly property color colDangerBorder: "#F87171"

    readonly property color colGhost: "transparent"
    readonly property color colGhostHover: "#1E293B88"
    readonly property color colGhostBorder: mouseArea.containsMouse ? "#475569" : "transparent"

    color: {
        if (isSelected) return colPrimary;
        var hover = mouseArea.containsMouse;
        switch (variant) {
            case "primary": return hover ? colPrimaryHover : colPrimary;
            case "success": return hover ? colSuccessHover : colSuccess;
            case "warning": return hover ? colWarningHover : colWarning;
            case "danger":  return hover ? colDangerHover : colDanger;
            case "ghost":   return hover ? colGhostHover : colGhost;
            default:        return hover ? colDefaultHover : colDefault;
        }
    }

    border.color: {
        if (isSelected) return "#93C5FD";
        var hover = mouseArea.containsMouse;
        switch (variant) {
            case "primary": return colPrimaryBorder;
            case "success": return colSuccessBorder;
            case "warning": return colWarningBorder;
            case "danger":  return colDangerBorder;
            case "ghost":   return hover ? "#475569" : "transparent";
            default:        return hover ? "#475569" : colDefaultBorder;
        }
    }
    border.width: isSelected ? 2 : 1.5

    // Scale animation on press for punchy tactile feedback
    scale: mouseArea.pressed && enabledState ? 0.94 : (mouseArea.containsMouse && enabledState ? 1.03 : 1.0)
    Behavior on scale { NumberAnimation { duration: 90; easing.type: Easing.OutQuad } }
    Behavior on color { ColorAnimation { duration: 120 } }
    Behavior on border.color { ColorAnimation { duration: 120 } }

    Row {
        id: buttonContentRow
        anchors.centerIn: parent
        spacing: (root.iconText.length > 0 && root.text.length > 0) ? 6 : 0

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.iconText
            font.pixelSize: root.iconOnly ? (root.pointSize + 5) : (root.pointSize + 2)
            visible: root.iconText.length > 0
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            font.pixelSize: root.pointSize
            font.bold: root.bold
            color: "#F8FAFC"
            visible: root.text.length > 0 && !root.iconOnly
            elide: Text.ElideRight
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: root.enabledState
        cursorShape: root.enabledState ? Qt.PointingHandCursor : Qt.ArrowCursor
        enabled: root.enabledState
        onClicked: root.clicked()
    }
}
