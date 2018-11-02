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
import osmscout.theme 1.0

Label {
    id: activeDownloadText
    text: ""
    x: Theme.horizontalPageMargin
    width: parent.width-2*x
    wrapMode: Text.WordWrap

    function setText(state)
    {
        if (!state) activeDownloadText.text = qsTr("No downloads active")
        else activeDownloadText.text = qsTr("Download active")
    }

    Connections {
        target: manager
        onDownloadProgress: activeDownloadText.text = info
    }

    Connections {
        target: manager
        onDownloadingChanged: {
            if (!state) activeDownloadText.setText(state)
        }
    }

    Component.onCompleted: activeDownloadText.setText(manager.downloading)
}
