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
import "platform"

Column {
    id: main

    width: parent.width
    anchors.margins: styler.themeHorizontalPageMargin
    spacing: styler.themePaddingSmall

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
        contentHeight: textInput.height + 2*styler.themePaddingMedium
        width: parent.width

        LabelPL {
            id: textInput
            text: ""
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            anchors.verticalCenter: parent.verticalCenter
            color: listItem.highlighted ? styler.themeHighlightColor : styler.themePrimaryColor

            Component.onCompleted: {
                setValue(settings.valueString(main.key))
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
        display_value = value
        if (value.length < 1)
            display_value = qsTr("All supported languages")
    }

    function setValue(v) {
        value = v
        display()
        if (autoApply) apply()
    }

    function select() {
        app.pushMain(Qt.resolvedUrl("LanguageSelector.qml"), {
                         value: value,
                         title: mainLabel,
                         callback: setValue,
                         comment: selectorComment,
                         note: selectorNote
                     })
    }
}

