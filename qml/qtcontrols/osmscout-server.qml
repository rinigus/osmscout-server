/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
 * 
 * This file is part of OSM Scout Server.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import osmscout.theme 1.0
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
                font.pointSize: Qt.application.font.pointSize * 1.6
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
                font.pointSize: Qt.application.font.pointSize * 1.6
            }

            ToolButton {
                id: acceptButton
                text: "Accept"
                font.pixelSize: Qt.application.font.pixelSize * 1.6
                visible: pageStack.depth > 1 && pageStack.currentItem.dialogue === true
                enabled: pageStack.depth > 1 && pageStack.currentItem.canAccept === true
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
        height: appWindow.height

        Column {
            anchors.fill: parent

            ItemDelegate {
                text: qsTr("Home")
                onClicked: {
                    pageStack.pop(null)
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Map Manager")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("MapManagerPage.qml"))
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Profile")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("ProfilesPage.qml"))
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("Settings")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("About")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
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

    Settings {
        property alias x: appWindow.x
        property alias y: appWindow.y
        property alias width: appWindow.width
        property alias height: appWindow.height
    }
}
