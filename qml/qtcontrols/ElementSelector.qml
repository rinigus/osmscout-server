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
import QtQuick.Dialogs 1.0
import osmscout.theme 1.0
import "."

Column {
    id: main

    property string key
    property string mainLabel
    property string secondaryLabel
    property bool directory: false
    property string directory_file: ""

    property alias value: fullPath.text
    property alias display_value: textInput.text

    width: parent.width
    anchors.margins: Theme.horizontalPageMargin
    spacing: Theme.paddingSmall

    function apply()
    {
        settings.setValue(key, value)
    }

    function display()
    {
        display_value = value.split("/").pop()
        if (value.length < 1)
            display_value = qsTr("<Not selected>")
    }

    function setPath(path)
    {
        value = path
        display()
    }

    FileDialog {
        id: selector
        title: "Select " + mainLabel
        selectFolder: main.directory

        onAccepted: {
            var path = fileUrl.toString();
            // remove prefixed "file://"
            path = path.replace(/^(file:\/{2})/,"");
            // unescape html codes like '%23' for '#'
            var cleanPath = decodeURIComponent(path);
            setPath(cleanPath)
        }
    }

    function select()
    {
        selector.folder = 'file://' + value
        selector.open()
    }

    Label {
        text: parent.mainLabel
        x: Theme.horizontalPageMargin
        width: parent.width-2*x
        wrapMode: Text.WordWrap
    }

    ItemDelegate {
        id: listItem
        width: parent.width

        Column {
            width: parent.width
            spacing: Theme.paddingSmall

            Label {
                id: textInput
                text: ""
                x: Theme.horizontalPageMargin
                width: parent.width-2*x

                Component.onCompleted: {
                    main.value = settings.valueString(main.key)
                    setPath(main.value)
                }
            }

            Label {
                id: fullPath
                text: ""
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                font.pointSize: Theme.fontSizeTiny
            }
        }

        onClicked: select()
    }

    Label {
        id: secLabel
        text: parent.secondaryLabel
        x: Theme.horizontalPageMargin
        width: parent.width-2*x
        wrapMode: Text.WordWrap
        font.pointSize: Theme.fontSizeSmall

        Component.onCompleted: { visible = (parent.secondaryLabel.length > 0) }
    }

    Rectangle {
        height: Theme.paddingMedium
        width: parent.width
        color: "transparent"
    }
}

