/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import "."
import "platform"

PageEmptyPL {
    id: page

    property string message: ""
    property string description: ""

    BusyIndicatorPL {
        id: busy
        running: true
    }

    LabelPL {
        anchors.bottom: busy.top
        anchors.bottomMargin: Math.round(busy.height/4)
        anchors.horizontalCenter: parent.horizontalCenter
        color: styler.themeHighlightColor
        font.pixelSize: styler.themeFontSizeLarge
        horizontalAlignment: Text.AlignHCenter
        text: message
        width: parent.width - 2*styler.themeHorizontalPageMargin
        wrapMode: Text.WordWrap
    }

    LabelPL {
        anchors.top: busy.bottom
        anchors.topMargin: Math.round(busy.height/4)
        anchors.horizontalCenter: parent.horizontalCenter
        color: styler.themeHighlightColor
        horizontalAlignment: Text.AlignHCenter
        text: description
        visible: description
        width: parent.width - 2*styler.themeHorizontalPageMargin
        wrapMode: Text.WordWrap
    }
}
