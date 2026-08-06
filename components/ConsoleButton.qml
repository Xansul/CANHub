import QtQuick
import QtQuick.Controls

Button {
    id: control
    palette.buttonText: "#222"
    background: Rectangle {
        color: control.pressed ? "#222" : (control.hovered ? "#333" : "#444")
        radius: 4
    }
}
