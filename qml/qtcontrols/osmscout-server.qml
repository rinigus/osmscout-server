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

        ToolButton {
            id: toolButton
            text: stackView.depth > 1 ? "\u25C0" : "\u2630"
            font.pixelSize: Qt.application.font.pixelSize * 1.6
            onClicked: {
                if (stackView.depth > 1) {
                    stackView.pop()
                } else {
                    drawer.open()
                }
            }
        }

        Label {
            text: stackView.currentItem.title
            anchors.centerIn: parent
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
                    stackView.pop(null)
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Map Manager")
                width: parent.width
                onClicked: {
                    stackView.push(".qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Profile")
                width: parent.width
                onClicked: {
                    stackView.push(".qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Settings")
                width: parent.width
                onClicked: {
                    stackView.push(".qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("About")
                width: parent.width
                onClicked: {
                    stackView.push(".qml")
                    drawer.close()
                }
            }
        }
    }

    StackView {
        id: stackView
        initialItem: StartPage { }
        anchors.fill: parent
    }

//    StackLayout {
//        width: parent.width
//        height: parent.height
//        currentIndex: bar.currentIndex

//        StartPage {
//            id: startPage
//        }
//        Item {
//            id: discoverTab
//        }
//        Item {
//            id: activityTab
//        }
//    }

//    footer: TabBar {
//        id: bar

//        TabButton {
//            text: qsTr("Main")
//        }
//        TabButton {
//            text: qsTr("Map Manager")
//        }
//        TabButton {
//            text: qsTr("Profile")
//        }
//        TabButton {
//            text: qsTr("Settings")
//        }
//        TabButton {
//            text: qsTr("About")
//        }
//    }
}
