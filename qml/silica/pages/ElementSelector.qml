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
import harbour.osmscout.server.FileManager 1.0

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

    function select()
    {
        pageStack.push(Qt.resolvedUrl("FileSelector.qml"), {
                           homePath: value,
                           showFormat: true,
                           title: "Select " + mainLabel,
                           callback: setPath,
                           directory: directory,
                           directory_file: directory_file
                       })
    }

//    SectionHeader {
//        text: parent.mainLabel
//        font.pixelSize: Theme.fontSizeSmall
//    }

    Label {
        text: parent.mainLabel
        x: Theme.horizontalPageMargin
        width: parent.width-2*x
        wrapMode: Text.WordWrap
        font.pixelSize: Theme.fontSizeMedium
        color: Theme.highlightColor
    }

    ListItem {
        id: listItem

        Column {
            width: parent.width
            spacing: Theme.paddingSmall

            Label {
                id: textInput
                text: ""
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor

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
                color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                font.pixelSize: Theme.fontSizeTiny
                truncationMode: TruncationMode.Fade
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
        font.pixelSize: Theme.fontSizeSmall
        color: Theme.highlightColor

        Component.onCompleted: { visible = (parent.secondaryLabel.length > 0) }
    }

    Rectangle {
        height: Theme.paddingMedium
        width: parent.width
        color: "transparent"
    }
}

