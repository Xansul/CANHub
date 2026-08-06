import QtQuick
import QtQuick.Controls

Button {
    id: control
    palette.buttonText: "#222"
    background: Rectangle {
        color: control.pressed ? "#333" : (control.hovered ? "#444" : "#555")
        radius: 4
    }
}
