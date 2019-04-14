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

    id: dialog
    title: qsTr("Valhalla")

    Column {
        spacing: styler.themePaddingLarge
        width: dialog.width

        ListItemLabel {
            text: qsTr("Valhalla can be used as a routing engine to calculate routes and the routing instructions")
        }

        SectionHeaderPL {
            text: qsTr("Memory")
        }

        ElementEntry {
            id: eCache
            key: settingsValhallaPrefix + "cache_in_mb"
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            mainLabel: qsTr("Cache for storing tiles, MB")
            secondaryLabel: qsTr("Memory cache that is used to keep Valhalla's tiles in RAM.")
            validator: DoubleValidator { bottom: 16; top: 1024; decimals: 0; }
        }

        SectionHeaderPL {
            text: qsTr("Limits")
        }

        ListItemLabel {
            font.pixelSize: styler.themeFontSizeSmall
            text: qsTr("Distance limits for route calculations. Only routes with the distances between locations " +
                       "that are smaller than the given limits will be calculated. By limiting the maximal distances " +
                       "between locations, you can prevent accidental calculations that would lead to too large " +
                       "routes and would use excessive amounts of RAM on your device.")
        }

        FormLayoutPL {
            spacing: styler.themePaddingLarge

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
    }

    onAccepted: {
        eCache.apply()
        eCar.apply()
        eBicycle.apply()
        ePedestrian.apply()
    }
}
