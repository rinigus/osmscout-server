/*
 * Copyright (C) 2016-2019 Rinigus https://github.com/rinigus
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
import "."
import "platform"

DialogPL {
    id: page
    canAccept: manager.ready
    title: qsTr("Updates")

    property var foundUpdates: []
    property int nUpdates: 0

    Column {
        id: column

        anchors.left: page.left
        anchors.right: page.right

        spacing: styler.themePaddingLarge

        ListItemLabel {
            text: qsTr("Do you want to start the update?")
        }

        ListItemLabel {
            font.pixelSize: styler.themeFontSizeSmall
            text: qsTr("Please note that as soon as you start the updates, the maps will not be available until the update is finished")
        }

        ListItemLabel {
            text: qsTr("List of countries and features with the found updates")
        }

        Column {
            spacing: styler.themePaddingMedium
            width: parent.width
            Repeater {
                model: nUpdates
                delegate: ElementCountryListItem {
                    country: foundUpdates[index]
                    active: false
                    width: parent.width
                }
                width: parent.width
            }
        }
    }

    Component.onCompleted: {
        nUpdates = foundUpdates.length
    }

    onAccepted: manager.getUpdates()
}
