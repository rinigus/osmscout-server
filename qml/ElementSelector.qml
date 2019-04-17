/*
 * Copyright (C) 2016-2019 Rinigus https://github.com/rinigus
 *                    2019 Purism SPC
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
import harbour.osmscout.server.FileManager 1.0
import "platform"

Column {
    id: main
    anchors.margins: styler.themeHorizontalPageMargin
    spacing: styler.themePaddingSmall
    width: parent.width

    property string key
    property string mainLabel
    property string secondaryLabel
    property bool   directory: false

    property alias value: fullPath.text
    property alias displayValue: textInput.text

    LabelPL {
        text: parent.mainLabel
        x: styler.themeHorizontalPageMargin
        width: parent.width-2*x
        wrapMode: Text.WordWrap
        font.pixelSize: styler.themeFontSizeMedium
        color: styler.themeHighlightColor
    }

    ListItemPL {
        id: listItem
        contentHeight: col.height

        Column {
            id: col
            width: parent.width
            spacing: styler.themePaddingSmall

            Rectangle {
                width: parent.width
                height: styler.themePaddingSmall
                color: "transparent"
            }

            LabelPL {
                id: textInput
                text: ""
                x: styler.themeHorizontalPageMargin
                width: parent.width-2*x
                color: listItem.highlighted ? styler.themeHighlightColor : styler.themePrimaryColor

                Component.onCompleted: {
                    main.value = settings.valueString(main.key)
                    setPath(main.value)
                }
            }

            LabelPL {
                id: fullPath
                text: ""
                x: styler.themeHorizontalPageMargin
                width: parent.width-2*x
                color: listItem.highlighted ? styler.themeHighlightColor : styler.themePrimaryColor
                font.pixelSize: styler.themeFontSizeExtraSmall
                truncMode: truncModes.fade
            }

            Rectangle {
                width: parent.width
                height: styler.themePaddingSmall
                color: "transparent"
            }
        }

        onClicked: select()
    }

    LabelPL {
        id: secLabel
        text: parent.secondaryLabel
        x: styler.themeHorizontalPageMargin
        width: parent.width-2*x
        wrapMode: Text.WordWrap
        font.pixelSize: styler.themeFontSizeSmall
        color: styler.themeHighlightColor

        Component.onCompleted: { visible = (parent.secondaryLabel.length > 0) }
    }

    Rectangle {
        height: styler.themePaddingMedium
        width: parent.width
        color: "transparent"
    }

    function apply() {
        settings.setValue(key, value)
    }

    function display() {
        displayValue = value.split("/").pop()
        if (value.length < 1)
            displayValue = qsTr("<Not selected>")
    }

    function setPath(path) {
        value = path
        display()
    }

    function select() {
        app.push(Qt.resolvedUrl("platform/FileSelectorPL.qml"), {
                     homePath: value,
                     title: "Select " + mainLabel,
                     callback: setPath,
                     directory: directory
                 })
    }
}

