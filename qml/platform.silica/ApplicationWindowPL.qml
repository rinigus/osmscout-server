/* -*- coding: utf-8-unix -*-
 *
 * Copyright (C) 2018 Rinigus
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0
import "."

ApplicationWindow {
    allowedOrientations: defaultAllowedOrientations
    _defaultPageOrientations: defaultAllowedOrientations // required to fix issue #219
    cover: Cover {}
    initialPage: null

    property string menuPageUrl
    property var    pages: StackPL { }
    property string title

    Component.onCompleted: {
        pages.ps = pageStack;
        updateOrientation()
    }


    function clearPages() {
        // not used in the platforms with menu shown
        // as a page in a stack
    }

    function initPages() {
    }

    function showMainMenu() {
        console.log("Not used in Silica platform")
    }
}
