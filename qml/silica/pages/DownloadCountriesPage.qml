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

Page {
    id: page

    property var countries: []
    property string title: ""
    property string path: ""
    property int nCountries: 1
    property bool root: false
    property bool fullpath_visible: true

    allowedOrientations : Orientation.All

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaListView {
        anchors.fill: parent

        header: Column {
            width: parent.width

            PageHeader {
                id: head
                title: page.title
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Label {
                id: fullpath
                text: page.path
                x: Theme.horizontalPageMargin
                width: page.width-2*x
                horizontalAlignment: Text.AlignRight
                truncationMode: TruncationMode.Elide
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.highlightColor
                visible: page.fullpath_visible
            }
        }

        model: nCountries

        delegate: ElementCountryListItem {
            country: countries.children[index]
        }
    }

    VerticalScrollDecorator {}

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
