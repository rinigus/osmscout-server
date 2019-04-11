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
import "platform"

DialogPL {
    id: dialog
    title: qsTr("Geocoder-NLP")

    FormLayoutPL {
        id: column

        width: parent.width
        spacing: styler.themePaddingLarge

        LabelPL {
            color: styler.themeHighlightColor
            text: qsTr("Geocoder is responsible for resolving search requests. " +
                       "For that, it parses the search string and finds the corresponding objects on a map.")
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        LabelPL {
            color: styler.themeHighlightColor
            font.pixelSize: styler.themeFontSizeSmall
            text: qsTr("Disclaimer: Please see About regarding reporting of the issues with the address parsing.")
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        ElementLanguageSelector {
            id: eGeoLanguages
            key: settingsGeomasterPrefix + "languages"
            secondaryLabel: qsTr("List of languages used for parsing addresses. " +
                                 "Note that when all supported languages are used, RAM requirements could be large")
        }

        ElementSwitch {
            id: eGeoSearchAllMaps
            key: settingsGeomasterPrefix + "search_all_maps"
            mainLabel: qsTr("Search all available maps")
            secondaryLabel: qsTr("When enabled, each search will be performed using all available maps on the device")
        }

        ElementSwitch {
            id: eGeoPostal
            key: settingsGeomasterPrefix + "use_postal"
            mainLabel: qsTr("Use libpostal parser")
            secondaryLabel: qsTr("This is the main address parser of the geocoder. It is recommended to enable " +
                                 "<i>libpostal</i> parser, unless the hardware does not allow to use it and the limitations " +
                                 "primitive parser are taken into account. Note that at least one parser has to be specified.")
        }

        ElementSwitch {
            id: eGeoPrimitive
            key: settingsGeomasterPrefix + "use_primitive"
            mainLabel: qsTr("Use primitive parser")
            secondaryLabel: qsTr("In addition to <i>libpostal</i>, primitive parser allows you to specify " +
                                 "administrative hierarchy of a searched object by separating components with a comma. " +
                                 "Sometimes, when libpostal fails to parse the request correctly, this parser allows " +
                                 "you to overcome the issue.<br>Example: house_number, street, town.")
        }

        ElementSwitch {
            id: eGeoInitEveryCall
            key: settingsGeomasterPrefix + "initialize_every_call"
            mainLabel: qsTr("Load <i>libpostal</i> on every call")
            secondaryLabel: qsTr("When selected, <i>libpostal</i> databases will be loaded to the memory only " +
                                 "while parsing the request. As a result, while search would take longer time, " +
                                 "the memory overhead of libpostal is small in idle.")
        }
    }

    onAccepted: {
        eGeoInitEveryCall.apply();
        eGeoSearchAllMaps.apply();
        eGeoPostal.apply();
        eGeoPrimitive.apply();
        eGeoLanguages.apply();
    }
}
