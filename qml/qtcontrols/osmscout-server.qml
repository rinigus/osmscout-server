/*
  Copyright (C) 2018 rinigus <rinigus.git@gmail.com>
  License: LGPL
*/

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."

ApplicationWindow {
    id: appWindow
    visible: true
    width: 640
    height: 480
    title: qsTr("OSM Scout Server")

    header: ToolBar {
        contentHeight: toolButton.implicitHeight

        RowLayout {
            anchors.fill: parent

            ToolButton {
                id: toolButton
                text: pageStack.depth > 1 ? "\u25C0" : "\u2630"
                font.pixelSize: Qt.application.font.pixelSize * 1.6
                onClicked: {
                    if (pageStack.depth > 1) {
                        pageStack.pop()
                    } else {
                        drawer.open()
                    }
                }
            }

            Label {
                text: pageStack.currentItem.title
                anchors.centerIn: parent
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }

            ToolButton {
                id: acceptButton
                text: "Accept"
                font.pixelSize: Qt.application.font.pixelSize * 1.6
                visible: pageStack.depth > 1 && pageStack.currentItem.dialogue === true
                onClicked: {
                    pageStack.currentItem.onAccepted()
                    if (pageStack.depth > 1) {
                        pageStack.pop()
                    }
                }
            }
        }
    }

    Drawer {
        id: drawer
        width: appWindow.width * 0.66
        height: appWindow.height

        Column {
            anchors.fill: parent

            ItemDelegate {
                text: qsTr("Home")
                width: parent.width
                onClicked: {
                    pageStack.pop(null)
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Map Manager")
                width: parent.width
                onClicked: {
                    pageStack.push(".qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Profile")
                width: parent.width
                onClicked: {
                    pageStack.push(".qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Settings")
                width: parent.width
                onClicked: {
                    pageStack.push(".qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("About")
                width: parent.width
                onClicked: {
                    pageStack.push(".qml")
                    drawer.close()
                }
            }
        }
    }

    StackView {
        id: pageStack
        initialItem: StartPage { }
        anchors.fill: parent
        anchors.topMargin: Theme.paddingLarge
    }
}
