/*
 * Copyright (C) 2016-2019 Rinigus https://github.com/rinigus
 *               2019 Purism SPC
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

ApplicationWindowPL {
    id: app
    initialPage: PagePL {
        title: "OSM Scout Server"
        Item { height: 10; width: 10; }
    }
    menuPageUrl: Qt.resolvedUrl("MainMenu.qml")
    title: qsTr("OSM Scout Server")

    property var rootPage

    StylerPL {
        id: styler
    }

    TruncationModes {
        id: truncModes
    }

    Component.onCompleted: {
        app.pages.replace( Qt.resolvedUrl("StartPage.qml") )
    }

    function createObject(page, options) {
        var pc = Qt.createComponent(page);
        if (pc.status === Component.Error) {
            console.log('Error while creating component');
            console.log(pc.errorString());
            return null;
        }
        return pc.createObject(app, options ? options : {})
    }

    function push(pagefile, options, clearAll) {
        return app.pages.push(pagefile, options);
    }

    function pushMain(pagefile, options) {
        // replace the current main with the new stack
        app.pages.pop(app.rootPage);
        return app.pages.push(pagefile, options);
    }
}
