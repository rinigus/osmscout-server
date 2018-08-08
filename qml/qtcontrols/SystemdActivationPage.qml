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
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."

Dialog {

    id: dialog
    title: qsTr("Automatic activation")
    contentHeight: column.height + Theme.paddingLarge

    Column {
        id: column

        x: Theme.horizontalPageMargin
        width: dialog.width-2*x
        spacing: Theme.paddingLarge

        Label {
            text: qsTr("OSM Scout Server can be activated automatically when the clients access it. " +
                       "When started automatically, it will be stopped if idle after given period of time.")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        Label {
            text: qsTr("Do you want to enable automatic activation?")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        Switch {
            id: systemdEnable
            text: qsTr("Automatic activation")
            Component.onCompleted: {
                checked = systemd_service.enabled
            }
        }

        Label {
            text: qsTr("It is recommended to enable automatic activation to simplify the access to the server.")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
            font.pointSize: Theme.fontSizeSmall
        }

        Label {
            text: qsTr("Note that when automatically activated, the server runs without any user interface. " +
                       "Automatic activation and the corresponding idle timeout can be later configured in Settings.")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
            font.pointSize: Theme.fontSizeSmall
        }

        Label {
            text: qsTr("For technical details, created files, and how to remove them if needed, see corresponding section " +
                       "of the <a href='https://rinigus.github.io/osmscout-server/en/#implementation-of-automatic-activation'>User's Guide</a>.")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
            font.pointSize: Theme.fontSizeSmall
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }

    function onAccepted() {
        systemd_service.enabled = systemdEnable.checked
    }
}
