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

    id: dialog
    allowedOrientations : Orientation.All

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: parent.width
            spacing: Theme.paddingLarge
            anchors.margins: Theme.horizontalPageMargin

            DialogHeader {
                title: qsTr("Valhalla")
            }

            Label {
                text: qsTr("Valhalla can be used as a routing engine to calculate routes and the routing instructions")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                //font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            SectionHeader {
                text: qsTr("Memory")
            }

            ElementEntry {
                id: eCache
                key: settingsValhallaPrefix + "cache_in_mb"
                mainLabel: qsTr("Cache for storing tiles, MB")
                secondaryLabel: qsTr("Memory cache that is used to keep Valhalla's tiles in RAM.")
                validator: DoubleValidator { bottom: 16; top: 1024; decimals: 0; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            SectionHeader {
                text: qsTr("Limits")
            }

            Label {
                text: qsTr("Distance limits for route calculations. Only routes with the distances between locations " +
                           "that are smaller than the given limits will be calculated. By limiting the maximal distances " +
                           "between locations, you can prevent accidental calculations that would lead to too large " +
                           "routes and would use excessive amounts of RAM on your device.")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            ElementEntry {
                id:  eCar
                key: settingsValhallaPrefix + "limit_max_distance_auto"
                mainLabel: qsTr("Limit for route by car")
                validator: DoubleValidator { bottom: 10; top:10000; decimals: 0; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
                id:  eBicycle
                key: settingsValhallaPrefix + "limit_max_distance_bicycle"
                mainLabel: qsTr("Limit for route by bicycle")
                validator: DoubleValidator { bottom: 10; top: 1000; decimals: 0; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
                id:  ePedestrian
                key: settingsValhallaPrefix + "limit_max_distance_pedestrian"
                mainLabel: qsTr("Limit for route by foot")
                validator: DoubleValidator { bottom: 10; top: 1000; decimals: 0; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
        }

        VerticalScrollDecorator {}
    }

    onAccepted: {
        eCache.apply()
        eCar.apply()
        eBicycle.apply()
        ePedestrian.apply()
    }
}
