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
        anchors.fill: parent
        anchors.margins: 12
        spacing: 4
        model: nodeListModel

        delegate: Rectangle {
            width: ListView.view.width
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
}
