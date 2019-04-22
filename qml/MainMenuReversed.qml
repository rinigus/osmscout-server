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

// NB! When making changes here, make corresponding changes in
// MainMenu.qml

MenuDrawerPL {
    banner: "icons/banner.jpg"
    title: "OSM Scout Server"
    titleIcon: "osm-scout-server"

    MenuDrawerItemPL {
        text: qsTr("About OSM Scout Server")
        onClicked: app.pushMain(Qt.resolvedUrl("AboutPage.qml"))
    }

    MenuDrawerItemPL {
        text: qsTr("Settings")
        onClicked: app.pushMain(Qt.resolvedUrl("SettingsPage.qml"))
    }

    MenuDrawerItemPL {
        text: qsTr("Profile")
        onClicked: app.pushMain(Qt.resolvedUrl("ProfilesPage.qml"))
    }

    MenuDrawerItemPL {
        text: qsTr("Map Manager")
        enabled: manager.storageAvailable
        onClicked: app.pushMain(Qt.resolvedUrl("MapManagerPage.qml"))
    }
}

