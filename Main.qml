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
        default: return qsTr("Unknown")
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
        anchors.bottom: sdoConsolePanel.top
        anchors.margins: 12
        anchors.topMargin: 4
        clip: true
        model: traceModel

        delegate: Rectangle {
            width: traceView.width
            color: "#222"
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

    Rectangle {
        id: sdoConsolePanel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        height: 170
        radius: 6
        color: "#222"

        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 12

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("SDO Console")
                color: "white"
                font.bold: true
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 0

                TextField {
                    id: nodeIdField
                    placeholderText: qsTr("Node ID")
                    width: 80
                    validator: IntValidator { bottom: 1; top: 127 }
                }

                TextField {
                    id: indexField
                    placeholderText: qsTr("Index (hex)")
                    width: 100
                }

                TextField {
                    id: subIndexField
                    placeholderText: qsTr("Sub Index")
                    width: 80
                    validator: IntValidator { bottom: 0; top: 255 }
                }

                TextField {
                    id: dataField
                    placeholderText: qsTr("Data (hex, write only)")
                    width: 170
                }
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 20

                Button {
                    text: qsTr("Read")
                    palette.buttonText: "#222"
                    enabled: !sdoConsole.busy
                    onClicked: sdoConsole.readValue(parseInt(nodeIdField.text),
                                                    parseInt(indexField.text, 16),
                                                    parseInt(subIndexField.text))
                }

                Button {
                    text: qsTr("Write")
                    palette.buttonText: "#222"
                    enabled: !sdoConsole.busy
                    onClicked: sdoConsole.writeValue(parseInt(nodeIdField.text),
                                                    parseInt(indexField.text, 16),
                                                    parseInt(subIndexField.text),
                                                    dataField.text)
                }
            }

            BusyIndicator {
                anchors.horizontalCenter: parent.horizontalCenter
                running: sdoConsole.busy
                width: 24
                height: 24
                palette.dark: "white"
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: sdoConsole.lastResult
                color: "#cccccc"
                wrapMode: Text.Wrap
                width: parent.width
                horizontalAlignment: Text.AlignHCenter;
            }
        }
    }
}
