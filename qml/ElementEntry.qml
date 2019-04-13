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

TextFieldPL {
    id: main

    description: secondaryLabel
    text: ""
    label: mainLabel
    placeholderText: mainLabel
    width: parent.width

    property string key
    property string mainLabel
    property string secondaryLabel
    property bool   hasUnits: false
    property double displayFactor: 1.0

    property alias value: main.text

    Component.onCompleted: {
        text = settings.valueString(main.key)
        main.hasUnits = settings.hasUnits(main.key)
        if (main.hasUnits)
        {
            main.displayFactor = settings.unitFactor();
            validator.decimals = settings.unitDisplayDecimals()

            var v = parseFloat(text) * main.displayFactor
            text = v.toFixed(settings.unitDisplayDecimals())

            label = label + ", " + settings.unitName(main.key)
        }
    }

    onEnter: {
        focus = false
    }

    function apply() {
        if (main.acceptableInput)
        {
            var rep = value
            if (main.hasUnits)
                rep = parseFloat(value) / main.displayFactor

            settings.setValue(key, rep)
        }
    }
}

