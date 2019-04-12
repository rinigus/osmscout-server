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

DialogPL {
    id: dialog
    title: qsTr("Automatic activation")

    Column {
        id: column

        width: parent.width
        spacing: styler.themePaddingLarge

        LabelPL {
            color: styler.themeHighlightColor
            text: qsTr("OSM Scout Server can be activated automatically when the clients access it. " +
                       "When started automatically, it will be stopped if idle after given period of time.")
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        LabelPL {
            color: styler.themeHighlightColor
            text: qsTr("Do you want to enable automatic activation?")
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        TextSwitchPL {
            id: systemdEnable
            text: qsTr("Automatic activation")
            Component.onCompleted: {
                checked = systemd_service.enabled
            }
        }

        LabelPL {
            color: styler.themeHighlightColor
            font.pixelSize: styler.themeFontSizeSmall
            text: qsTr("It is recommended to enable automatic activation to simplify the access to the server.")
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        LabelPL {
            color: styler.themeHighlightColor
            font.pixelSize: styler.themeFontSizeSmall
            text: qsTr("Note that when automatically activated, the server runs without any user interface. " +
                       "Automatic activation and the corresponding idle timeout can be later configured in Settings.")
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        LabelPL {
            color: styler.themeHighlightColor
            font.pixelSize: styler.themeFontSizeSmall
            linkColor: styler.themePrimaryColor
            text: qsTr("For technical details, created files, and how to remove them if needed, see corresponding section " +
                       "of the <a href='https://rinigus.github.io/osmscout-server/en/#implementation-of-automatic-activation'>User's Guide</a>.")
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }

    onAccepted: {
        systemd_service.enabled = systemdEnable.checked
    }
}
