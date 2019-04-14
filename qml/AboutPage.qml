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
import "."
import "platform"

PagePL {
    id: page
    title: qsTr("About") + " " + programName

    property string mainText: "<p>" +
                              qsTr("The server has to be used with the client program that is able to display maps, query for searches and " +
                                   "display selected routes. The server is a thin layer exposing functionality of <i>libosmscout</i>, " +
                                   "<i>Mapnik</i>, <i>Valhalla</i>, <i>libpostal</i>, <i>Geocoder-NLP</i> and providing data for " +
                                   "<i>Mapbox GL</i> through web service. " +
                                   "This server can be used as a drop-in replacement for " +
                                   "online services providing maps.") + "</p><br>" +
                              "<p>" + qsTr("Copyright:") + " 2016-2018 <a href='https://github.com/rinigus'>Rinigus</a><br></p>" +
                              qsTr("License: GPLv3") + "<br><br>" +

                              "Donations: " +
                              "<a href='http://rinigus.github.io/donate'>http://rinigus.github.io/donate</a>" +
                              "<br><br>" +

                              "<b>" + qsTr("Translations") + "</b><br><br>" +
                              "Carmen F. B. @carmenfdezb: Spanish<br>" +
                              "Lukáš Karas @Karry: Czech<br>" +
                              "Åke Engelbrektson @eson57: Swedish<br>" +
                              "Ricardo Breitkopf @monkeyisland: German<br>" +
                              "Nathan Follens @pljmn: Dutch (NL and BE)<br>" +
                              "@Sagittarii: French<br>" +
                              "Oleg Artobolevsky @XOleg: Russian<br>" +
                              "A @atlochowski: Polish<br>" +
                              "Peer-Atle Motland @Pam: Norwegian Bokmål<br>" +
                              "Matti Lehtimäki @mal: Finnish<br>" +
                              "@Watchmaker: Italian<br>" +

                              "<br><b>" + qsTr("Maps") + "</b><br>" +
                              "<p>" +
                              qsTr("Maps are hosted by " +
                                   "Natural Language Processing Centre (Faculty of Informatics, Masaryk University, Brno, Czech Republic) " +
                                   "through modRana.org data repository.") +
                              "</p><br>" +
                              "<p>" +
                              qsTr("Map data from OpenStreetMap, Open Database License 1.0. Maps are converted to a suitable format " +
                                   "from downloaded extracts and/or using polygons as provided by Geofabrik GmbH.") +
                              "</p><br>" +
                              "<p>" +
                              qsTr("UK postal codes downloaded from " +
                                   "<a href='https://www.freemaptools.com/download-uk-postcode-lat-lng.htm'>FreeMapTools</a>. These " +
                                   "data contain: Ordnance Survey data © Crown copyright and database " +
                                   "right 2017; Royal Mail data © Royal Mail copyright and database right " +
                                   "2017; National Statistics data © Crown copyright and database right 2017.") +
                              "</p>" +

                              "<br><b>" + qsTr("Libpostal") + "</b><br>" +
                              "<p>" +
                              qsTr("Please note that <i>libpostal</i> is developed to be used with the fine tuned " +
                                   "model covering the World. The country-based models were developed to use " +
                                   "<i>libpostal</i> on mobile devices and have not been specifically tuned. Please submit the issues " +
                                   "with libpostal performance on country-based models to <i>OSM Scout Server</i> or " +
                                   "<i>geocoder-nlp</i> github projects.") +
                              "</p>" +

                              "<br><b>" + qsTr("Acknowledgments") + "</b><br><br>" +
                              "Used external projects:<br>" +
                              "<a href='http://libosmscout.sourceforge.net/'>libosmscout</a><br>" +
                              "<a href='https://github.com/openvenues/libpostal'>libpostal</a><br>" +
                              "<a href='http://mapnik.org'>Mapnik</a><br>" +
                              "<a href='https://github.com/valhalla/valhalla'>Valhalla</a><br>" +
                              "<a href='https://github.com/rinigus/geocoder-nlp'>Geocoder-NLP</a><br>" +
                              "<a href='https://www.gnu.org/software/libmicrohttpd'>GNU Libmicrohttpd</a><br>" +
                              "<a href='https://github.com/LuminosoInsight/langcodes'>langcodes</a><br><br>" +
                              "Hosting of maps: Natural Language Processing Centre (Faculty of Informatics, " +
                              "Masaryk University, Brno, Czech Republic) through <a href='http://modrana.org'>modRana.org</a><br><br>" +
                              "Fonts: <a href='https://www.google.com/get/noto/'>Noto fonts</a> (SIL Open Font License, Version 1.1)<br>"


    Item {
        height: column.height
        width: page.width

        Column {
            id: column
            spacing: styler.themePaddingLarge
            width: parent.width

            LabelPL {
                text: qsTr("version") + ": " + programVersion
                anchors.horizontalCenter: parent.horizontalCenter
                color: styler.themeHighlightColor
            }

            ListItemLabel {
                text: programName + " " + qsTr("is a server that provides offline map services")
            }

            ListItemLabel {
                text: qsTr("User's guide is available <a href='https://rinigus.github.io/osmscout-server'>online</a>")
            }

            ListItemLabel {
                text: mainText
                font.pixelSize: styler.themeFontSizeSmall
            }
        }
    }
}

