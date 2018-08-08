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
    id: root

    property string key
    property string mainLabel
    property string secondaryLabel
    property bool autoApply: false
    property bool activeState: true

    signal switchChanged(bool checked)

    width: parent.width
    anchors.margins: Theme.horizontalPageMargin
    spacing: Theme.paddingSmall

    function apply()
    {
        var ret = 0;
        if (textInput.checked) ret = 1;
        settings.setValue(key, ret)
    }

    TextSwitch {
        id: textInput
        width: parent.width
        text: parent.mainLabel
        enabled: root.activeState

        Component.onCompleted: {
            checked = (settings.valueInt(parent.key) > 0)
        }

        onCheckedChanged: {
            if ( checked != (settings.valueInt(parent.key) > 0) )
            {
                if (autoApply) apply()
                root.switchChanged(checked)
            }
        }
    }

    Label {
        text: parent.secondaryLabel
        x: Theme.horizontalPageMargin
        width: parent.width-2*x
        wrapMode: Text.WordWrap
        font.pixelSize: Theme.fontSizeSmall
        color: Theme.highlightColor

        Component.onCompleted: { visible = (parent.secondaryLabel.length > 0) }
    }
}


