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

CoverBackground {

    Image {
        id: download
        source: "image://theme/icon-cover-transfers"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.margins: Theme.paddingLarge
        fillMode: Image.PreserveAspectFit
        visible: manager.downloading
    }

    Image {
        id: icon
        source: "/usr/share/icons/hicolor/256x256/apps/harbour-osmscout-server.png"
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
        width: parent.width/2
    }

    Label {
        id: label
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.margins: Theme.paddingLarge
        text: qsTr("OSM Scout Server")

        function setText(q) {
            if (q > 0) text = qsTr("Jobs") + ": " + q
            else text = qsTr("Idle")
        }

        Connections {
            target: infohub;
            onQueueChanged: {
                label.setText(queue)
            }
        }

        Component.onCompleted: label.setText(infohub.queue)

    }


    //    CoverActionList {
    //        id: coverAction

    //        CoverAction {
    //            iconSource: "image://theme/icon-cover-next"
    //        }

    //        CoverAction {
    //            iconSource: "image://theme/icon-cover-pause"
    //        }
    //    }
}


