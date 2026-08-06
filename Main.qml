import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 2056
    height: 1024
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

    ColumnLayout {
        id: consolePanels
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: 500
        spacing: 12

        Rectangle {
            id: pdoConsolePanel
            Layout.fillHeight: true
            Layout.fillWidth: true
            radius: 6
            color: "#222"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("PDO Console")
                    color: "white"
                    font.bold: true
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 8

                    TextField {
                        id: pdoCobIdField
                        placeholderText: qsTr("COB-ID (hex)")
                        Layout.preferredWidth: 100
                    }

                    TextField {
                        id: pdoNodeIdField
                        placeholderText: qsTr("Node")
                        Layout.preferredWidth: 90
                        validator: IntValidator { bottom: 0; top: 127 }
                    }

                    TextField {
                        id: pdoDataField
                        placeholderText: qsTr("Data (hex, send only)")
                        Layout.preferredWidth: 160
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 8

                    Button {
                        text: qsTr("Subscribe")
                        palette.buttonText: "#222"
                        onClicked: pdoConsoleModel.addSubscription(
                                       parseInt(pdoCobIdField.text, 16),
                                       parseInt(pdoNodeIdField.text))
                    }

                    Button {
                        text: qsTr("Send")
                        palette.buttonText: "#222"
                        onClicked: pdoConsoleModel.sendPDO(
                                       parseInt(pdoCobIdField.text, 16),
                                       pdoDataField.text)
                    }

                    Button {
                        text: qsTr("Poll (not always supported")
                        palette.buttonText: "#222"
                        onClicked: pdoConsoleModel.pollPDO(parseInt(pdoCobIdField.text, 16))
                    }
                }

                ListView {
                    id: pdoList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: pdoConsoleModel

                    delegate: Row {
                        width: ListView.view.width
                        spacing: 12
                        padding: 2

                        Text {
                            text: cobId
                            color: "white"
                            width: 60
                            font.family: "monospace"
                        }

                        Text {
                            text: qsTr("node %1").arg(nodeId)
                            color: "#888"
                            width: 60
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

        Rectangle {
            id: sdoConsolePanel
            Layout.preferredHeight: 300
            Layout.fillWidth: true
            radius: 6
            color: "#222"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("SDO Console")
                    color: "white"
                    font.bold: true
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 0

                    TextField {
                        id: sdoNodeIdField
                        placeholderText: qsTr("Node ID")
                        Layout.preferredWidth: 80
                        validator: IntValidator { bottom: 1; top: 127 }
                    }

                    TextField {
                        id: sdoIndexField
                        placeholderText: qsTr("Index (hex)")
                        Layout.preferredWidth: 100
                    }

                    TextField {
                        id: sdoSubIndexField
                        placeholderText: qsTr("Sub Index")
                        Layout.preferredWidth: 80
                        validator: IntValidator { bottom: 0; top: 255 }
                    }

                    TextField {
                        id: sdoDataField
                        placeholderText: qsTr("Data (hex, write only)")
                        Layout.preferredWidth: 170
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 20

                    Button {
                        text: qsTr("Read")
                        palette.buttonText: "#222"
                        enabled: !sdoConsole.busy
                        onClicked: sdoConsole.readValue(parseInt(sdoNodeIdField.text),
                                                        parseInt(sdoIndexField.text, 16),
                                                        parseInt(sdoSubIndexField.text))
                    }

                    Button {
                        text: qsTr("Write")
                        palette.buttonText: "#222"
                        enabled: !sdoConsole.busy
                        onClicked: sdoConsole.writeValue(parseInt(sdoNodeIdField.text),
                                                        parseInt(sdoIndexField.text, 16),
                                                        parseInt(sdoSubIndexField.text),
                                                        sdoDataField.text)
                    }
                }

                BusyIndicator {
                    Layout.alignment: Qt.AlignHCenter
                    running: sdoConsole.busy
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    palette.dark: "white"
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: parent.width
                    Layout.fillHeight: true
                    horizontalAlignment: Text.AlignHCenter;
                    text: sdoConsole.lastResult
                    color: "#cccccc"
                    wrapMode: Text.Wrap
                }
            }
        }
    }

    ColumnLayout {
        id: nodePanels
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 500
        spacing: 12

        Rectangle {
            id: nodePanel
            Layout.fillHeight: true
            Layout.fillWidth: true
            radius: 6
            color: "#222"

            ColumnLayout {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 20
                spacing: 12

                ListView {
                    id: nodeList
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    clip: true
                    model: nodeListModel

                    delegate: Rectangle {
                        width: nodeList.width
                        height: 48
                        radius: 6
                        color: alive ? "#1f3a1f" : "#3a1f1f"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
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
        }
    }

    ColumnLayout {
        id: tracePanel
        anchors.top: parent.top
        anchors.left: nodePanels.right
        anchors.right: consolePanels.left
        anchors.bottom: parent.bottom
        anchors.margins: 12

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            radius: 6
            color: "#222"

            Text {
                anchors.centerIn: parent
                text: qsTr("Trace Log")
                color: "white"
                font.bold: true
            }
        }

        ListView {
            id: traceView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: traceModel

            delegate: Rectangle {
                width: traceView.width
                color: "#222"
                height: 24
                radius: 6

                RowLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 12

                    Text {
                        text: timestamp.toFixed(3)
                        color: "#888"
                        Layout.preferredWidth: 70
                    }

                    Text {
                        text: direction
                        color: direction === "TX" ? "#8fb8d9" : "#d9c88f"
                        Layout.preferredWidth: 30
                    }

                    Text {
                        text: cobId
                        color: "white"
                        Layout.preferredWidth: 60
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
}
