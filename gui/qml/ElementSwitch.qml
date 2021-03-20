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

TextSwitchPL {
    id: root

    description: secondaryLabel
    text: mainLabel
    enabled: root.activeState

    property string key
    property string mainLabel
    property string secondaryLabel
    property bool autoApply: false
    property bool activeState: true

    signal switchChanged(bool checked)

    Component.onCompleted: checked = (settings.valueInt(key) > 0)

    onCheckedChanged: {
        if ( checked != (settings.valueInt(key) > 0) ) {
            if (autoApply) apply()
            root.switchChanged(checked)
        }
    }

    function apply() {
        var ret = 0;
        if (root.checked) ret = 1;
        settings.setValue(key, ret)
    }
}


