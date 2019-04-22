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

PageListPL {
    id: page
    delegate: ElementCountryListItem {
        country: countries.children[index]
    }
    headerExtra: Component {
        LabelPL {
            id: fullpath
            color: styler.themeHighlightColor
            font.pixelSize: styler.themeFontSizeExtraSmall
            horizontalAlignment: Text.AlignRight
            text: page.path
            truncMode: truncModes.elide
            visible: page.fullpath_visible
            x: styler.themeHorizontalPageMargin
            width: page.width-2*x
        }
    }
    model: nCountries

    property var countries: []
    property string path: ""
    property int nCountries: 1
    property bool root: false
    property bool fullpath_visible: true

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
