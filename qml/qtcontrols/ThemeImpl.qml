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

pragma Singleton
import QtQuick 2.0

Item {
    readonly property int horizontalPageMargin: 10

    readonly property int paddingLarge: 10
    readonly property int paddingMedium: 7
    readonly property int paddingSmall: 5

    readonly property int iconSizeMedium: 20

    property int fontSizeNormal: Qt.application.font.pointSize
    property int fontSizeLarge: fontSizeNormal*1.2
    property int fontSizeSmall: fontSizeNormal*0.9
    property int fontSizeExtraSmall: fontSizeNormal*0.8
    property int fontSizeTiny: fontSizeNormal*0.7
}
