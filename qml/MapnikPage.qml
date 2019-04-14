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
import "."
import "platform"

DialogPL {
    id: dialog
    title: qsTr("Mapnik")

    Column {
        spacing: styler.themePaddingLarge
        width: parent.width

        ListItemLabel {
            text: qsTr("Mapnik can be used as a rendering backend to provide map tiles to the client programs")
        }

        FormLayoutPL {
            spacing: styler.themePaddingLarge

            ElementEntry {
                id: eScale
                key: settingsMapnikPrefix + "scale"
                mainLabel: qsTr("Scale factor")
                secondaryLabel: qsTr("Scale factor used to draw features on a map. On high resolution displays, such as " +
                                     "phones, it is advantageous to use scale factors larger than 1. As a first estimate, " +
                                     "it is suggested to use scale factor of 3-5 on mobile devices.")
                validator: DoubleValidator { bottom: 0.999; decimals: 1; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
                id: eBuffer
                key: settingsMapnikPrefix + "buffer_size_in_pixels"
                mainLabel: qsTr("Buffer size in pixels")
                secondaryLabel: qsTr("By increasing the size of the area which is used to load the data while rendering a tile, " +
                                     "the renderer can improve label rendering by taking into account data from neighboring tiles. " +
                                     "If you see too many cut labels, try to " +
                                     "increase the buffer size. Note that large buffer sizes would slow down the rendering. " +
                                     "The buffer size is scaled by the scaling factor to adjust for the increase in labels size.")
                validator: IntValidator { bottom: 0; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
        }
    }

    onAccepted: {
        eScale.apply()
        eBuffer.apply()
    }
}
