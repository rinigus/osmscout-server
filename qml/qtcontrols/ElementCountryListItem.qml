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

ItemDelegate {
    id: listItem
    height: clist.height + Theme.paddingLarge
    width: parent.width

    property var country
    property string iconSource: ""
    property bool active: true

    function updateData() {
        var c = country

        label.text = c.name
        if (c.type === "dir") {
            prop.text = qsTr("%1 territories").arg(c.children.length)
            iconSource = "image://theme/icon-m-file-folder"
        }
        else {
            var info = ""
            if (manager.isCountryAvailable(c.id) && c.id==="postal/global") {
                iconSource = "image://theme/icon-m-chat"
                info = qsTr("Available") + "; "
            }
            else if (manager.isCountryAvailable(c.id) && c.id==="mapnik/global") {
                iconSource = "image://theme/icon-m-image"
                info = qsTr("Available") + "; "
            }
            else if (manager.isCountryAvailable(c.id)) {
                iconSource = "image://theme/icon-m-location"
                info = qsTr("Available") + "; "
            }
            else if (manager.isCountryRequested(c.id) &&
                     !manager.isCountryCompatible(c.id)) {
                iconSource = "image://theme/icon-m-sync"
                info = qsTr("Incompatible version") + "; "
            }
            else if (manager.isCountryRequested(c.id)) {
                iconSource = "image://theme/icon-m-cloud-download"
                info = qsTr("Subscribed") + "; "
            }
            else
                iconSource = "image://theme/icon-m-region"

            prop.text = info + qsTr("Size: %1 MB").arg( c.size )
        }
    }

    Row {
        spacing: Theme.paddingLarge
        x: Theme.horizontalPageMargin
        width: parent.width-2*x
        anchors.verticalCenter: parent.verticalCenter

        Rectangle {
            width: Theme.iconSizeMedium
            height: Theme.iconSizeMedium
            color: "transparent"

            Image {
                anchors.centerIn: parent
                source: {
                    return iconSource + (highlighted || !active ? "?" + Theme.highlightColor : "")
                }
            }
        }

        Column {
            id: clist
            width: parent.width - Theme.iconSizeMedium - Theme.paddingLarge
            spacing: Theme.paddingSmall

            Label {
                id: label
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Label {
                id: prop
                width: parent.width
                horizontalAlignment: Text.AlignRight
                wrapMode: Text.WordWrap
            }
        }
    }

    Component.onCompleted: updateData()
    onCountryChanged: updateData()

    Connections {
        target: manager
        onSubscriptionChanged: listItem.updateData()
        onAvailabilityChanged: listItem.updateData()
    }

    onClicked: {
        if (active) {
            var c = country
            if (c.type === "dir")
            {
                var newpath = ""
                if (!page.root)
                {
                    if (page.path.length > 0)
                        newpath = page.path + " / " + c.name
                    else
                        newpath = page.title + " / " + c.name
                }
                pageStack.push(Qt.resolvedUrl("DownloadCountriesPage.qml"),
                               { "countries": c, "path": newpath, "title": c.name } )
            }
            else
                pageStack.push(Qt.resolvedUrl("CountryDetailsPage.qml"),
                               { "countryId": c.id } )
        }
    }
}
