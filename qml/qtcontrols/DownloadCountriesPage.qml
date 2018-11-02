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
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import osmscout.theme 1.0
import "."

Dialog {

    id: page
    dialogue: false

    property var countries: []
    property string path: ""
    property int nCountries: 1
    property bool root: false
    property bool fullpath_visible: true

    // To enable PullDownMenu, place our content in a SilicaFlickable
    ListView {
        anchors.fill: page

        header: Column {
            x: Theme.horizontalPageMargin
            width: page.width-2*x

            Label {
                id: head
                text: page.title
                wrapMode: Text.WordWrap
                width: parent.width
                horizontalAlignment: Text.AlignRight
                font.bold: true
                font.pointSize: Theme.fontSizeLarge
            }

            Label {
                id: fullpath
                text: page.path
                wrapMode: Text.WordWrap
                width: parent.width
                horizontalAlignment: Text.AlignRight
                font.pointSize: Theme.fontSizeTiny
                visible: page.fullpath_visible
            }
        }

        model: nCountries

        delegate: ElementCountryListItem {
            country: countries.children[index]
        }
    }

    Component.onCompleted: {
        nCountries = countries.children.length

        if (page.title.length < 1)
        {
            page.title = qsTr("Select country or territory")
            page.root = true
        }

        if (page.path.length < 1)
            page.fullpath_visible = false
    }
}
