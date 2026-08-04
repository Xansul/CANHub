import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 1024
    height: 640
    visible: true
    title: qsTr("CANHub")

    //map NodeState's enum to display text
    function nmtStateLabel(value) {
        switch (value) {
        case 1: return qsTr("Boot-up")
        case 2: return qsTr("Stopped")
        case 3: return qsTr("Pre-operational")
        case 4: return qsTr("Operational")
        case 5: return qsTr("Unknown")
        }
    }

    ListView {
        id: nodeList
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        height: 180
        spacing: 4
        model: nodeListModel

        delegate: Rectangle {
            width: nodeList.width
            height: 48
            radius: 6
            color: alive ? "#1f3a1f" : "#3a1f1f"

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 12
                spacing: 16

                Text {
                    text: qsTr("Node %1").arg(nodeId)
                    color: "white"
                    font.bold: true
                }

                Text {
                    text: root.nmtStateLabel(NMTState)
                    color: "white"
                }

                Text {
                    text: alive ? qsTr("Alive") : qsTr("No heartbeat")
                    color: alive ? "#8fd98f" : "#d98f8f"
                }
            }
        }

        Label {
            anchors.centerIn: parent
            visible: parent.count === 0
            text: qsTr("No nodes seen yet")
        }
    }

    ListView {
        id: traceView
        anchors.top: nodeList.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        anchors.topMargin: 4
        clip: true
        model: traceModel

        //auto scroll logic - auto scroll if view is at bottom
        property bool stickToBottom: true
        onCountChanged: if (stickToBottom) positionViewAtEnd()
        onMovementStarted: stickToBottom = atYEnd

        delegate: Rectangle {
            width: traceView.width
            color: "#2b2b2b"
            height: 24
            radius: 6

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                Text {
                    text: timestamp.toFixed(3)
                    color: "#888"
                    width: 70
                }

                Text {
                    text: direction
                    color: direction === "TX" ? "#8fb8d9" : "#d9c88f"
                    width: 30
                }

                Text {
                    text: cobId
                    color: "white"
                    width: 60
                    font.family: "monospace"
                }

                Text {
                    text: dataHex
                    color: "#cccccc"
                    font.family: "monospace"
                }
            }
        }
    }
}
