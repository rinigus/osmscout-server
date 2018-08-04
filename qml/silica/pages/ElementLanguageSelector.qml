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

Column {
    id: main

    property string key
    property string mainLabel: qsTr("Languages")
    property string secondaryLabel: qsTr("List of languages used for parsing addresses")
    property string selectorComment: qsTr("Please select languages used for parsing addresses.")
    property string selectorNote: qsTr("When none of the "+
                                       "languages are selected in this form, it is assumed that all supported languages " +
                                       "should be used for parsing.<br>" +
                                       "Note that when all supported languages are used, RAM requirements could be large.")

    property string value
    property alias display_value: textInput.text

    property bool autoApply: false

    width: parent.width
    anchors.margins: Theme.horizontalPageMargin
    spacing: Theme.paddingSmall

    function apply()
    {
        settings.setValue(key, value)
    }

    function display()
    {
        display_value = value
        if (value.length < 1)
            display_value = qsTr("All supported languages")
    }

    function setValue(v)
    {
        value = v
        display()
        if (autoApply) apply()
    }

    function select()
    {
        pageStack.push(Qt.resolvedUrl("LanguageSelector.qml"), {
                           value: value,
                           title: mainLabel,
                           callback: setValue,
                           comment: selectorComment,
                           note: selectorNote
                       })
    }

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
        width: parent.width

        Label {
            id: textInput
            text: ""
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            anchors.verticalCenter: parent.verticalCenter
            color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor

            Component.onCompleted: {
                setValue(settings.valueString(main.key))
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

