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

ListItemPL {
    id: listItem
    contentHeight: Math.max(icon.height,clist.height) + styler.themePaddingLarge
    width: parent.width

    property var country
    property string iconSource: ""
    property bool active: true

    Row {
        anchors.verticalCenter: parent.verticalCenter
        spacing: styler.themePaddingMedium
        x: styler.themeHorizontalPageMargin
        width: parent.width-2*x

        Rectangle {
            id: icon
            color: "transparent"
            height: styler.themeItemSizeSmall*0.75
            width: styler.themeItemSizeSmall*0.75

            IconPL {
                anchors.centerIn: parent
                iconHeight: parent.height
                iconName: listItem.iconSource
            }
        }

        Column {
            id: clist
            spacing: styler.themePaddingSmall
            width: parent.width - icon.width - parent.spacing

            LabelPL {
                id: label
                color: listItem.highlighted || !listItem.active ? styler.themeHighlightColor : styler.themePrimaryColor
                font.pixelSize: styler.themeFontSizeMedium
                width: parent.width
                wrapMode: Text.WordWrap
            }

            LabelPL {
                id: prop
                color: listItem.highlighted || !listItem.active ? styler.themeHighlightColor : styler.themePrimaryColor
                horizontalAlignment: Text.AlignRight
                font.pixelSize: styler.themeFontSizeSmall
                width: parent.width
                wrapMode: Text.WordWrap
            }
        }
    }

    Connections {
        target: manager
        onSubscriptionChanged: listItem.updateData()
        onAvailabilityChanged: listItem.updateData()
    }

    Component.onCompleted: updateData()
    onCountryChanged: updateData()
    onClicked: {
        if (active) {
            var c = country
            if (c.type === "dir") {
                var newpath = ""
                if (!page.root) {
                    if (page.path.length > 0)
                        newpath = page.path + " / " + c.name
                    else
                        newpath = page.title + " / " + c.name
                }
                app.push(Qt.resolvedUrl("DownloadCountriesPage.qml"),
                         { "countries": c, "path": newpath, "title": c.name } )
            }
            else
                app.push(Qt.resolvedUrl("CountryDetailsPage.qml"),
                         { "countryId": c.id } )
        }
    }

    function updateData() {
        var c = country

        label.text = c.name
        if (c.type === "dir") {
            prop.text = qsTr("%1 territories").arg(c.children.length)
            iconSource = styler.iconFolder
        }
        else {
            var info = ""
            if (manager.isCountryAvailable(c.id) && c.id==="postal/global") {
                iconSource = styler.iconCountryAvailable
                info = qsTr("Available") + "; "
            }
            else if (manager.isCountryAvailable(c.id) && c.id==="mapnik/global") {
                iconSource = styler.iconMapnikGlobal
                info = qsTr("Available") + "; "
            }
            else if (manager.isCountryAvailable(c.id)) {
                iconSource = styler.iconCountryAvailable
                info = qsTr("Available") + "; "
            }
            else if (manager.isCountryRequested(c.id) &&
                     !manager.isCountryCompatible(c.id)) {
                iconSource = styler.iconIncompatibleVersion
                info = qsTr("Incompatible version") + "; "
            }
            else if (manager.isCountryRequested(c.id)) {
                iconSource = styler.iconCountryRequested
                info = qsTr("Subscribed") + "; "
            }
            else
                iconSource = styler.iconRegion

            prop.text = info + qsTr("Size: %1 MB").arg( c.size )
        }
    }
}
