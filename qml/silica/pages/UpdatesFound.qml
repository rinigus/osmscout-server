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

import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: page

    property var foundUpdates: []
    property int nUpdates: 0

    canAccept: manager.ready

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            anchors.left: parent.left
            anchors.right: parent.right

            spacing: Theme.paddingLarge

            DialogHeader {
                title: qsTr("Updates")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
                text: qsTr("Do you want to start the update?")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
                text: qsTr("Please note that as soon as you start the updates, the maps will not be available until the update is finished")
                font.pixelSize: Theme.fontSizeSmall
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
                text: qsTr("List of countries and features with the found updates")
            }

            Column {
                width: parent.width
                spacing: Theme.paddingMedium
                Repeater {
                    width: parent.width
                    model: nUpdates
                    delegate: ElementCountryListItem {
                        country: foundUpdates[index]
                        active: false
                    }
                }
            }

            VerticalScrollDecorator {}
        }
    }

    Component.onCompleted: {
        nUpdates = foundUpdates.length
    }

    onAccepted: manager.getUpdates()
}
