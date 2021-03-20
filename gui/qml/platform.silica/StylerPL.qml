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

QtObject {
    readonly property bool isSilica: true
    
    // font sizes and family
    property string themeFontFamily: Theme.fontFamily
    property string themeFontFamilyHeading: Theme.fontFamilyHeading
    property int  themeFontSizeHuge: Theme.fontSizeHuge
    property int  themeFontSizeExtraLarge: Theme.fontSizeExtraLarge
    property int  themeFontSizeLarge: Theme.fontSizeLarge
    property int  themeFontSizeMedium: Theme.fontSizeMedium
    property int  themeFontSizeSmall: Theme.fontSizeSmall
    property int  themeFontSizeExtraSmall: Theme.fontSizeExtraSmall
    property real themeFontSizeOnMap: themeFontSizeExtraSmall * 0.75

    // colors
    // block background (navigation, poi panel, bubble)
    property color blockBg: Theme.overlayBackgroundColor || "#e6000000"
    // variant of navigation icons
    property string navigationIconsVariant: Theme.colorScheme ? "black" : "white"
    // descriptive items
    property color themeHighlightColor: Theme.highlightColor
    // navigation items (to be clicked)
    property color themePrimaryColor: Theme.primaryColor
    // navigation items, secondary
    property color themeSecondaryColor: Theme.secondaryColor
    // descriptive items, secondary
    property color themeSecondaryHighlightColor: Theme.secondaryHighlightColor

    // button sizes
    property real themeButtonWidthLarge: Theme.buttonWidthLarge
    property real themeButtonWidthMedium: Theme.buttonWidthMedium

    // icon sizes
    property real themeIconSizeLarge: Theme.iconSizeLarge
    property real themeIconSizeMedium: Theme.iconSizeMedium
    property real themeIconSizeSmall: Theme.iconSizeSmallPlus

    // used icons
    property string iconAbout: "image://theme/icon-m-about"
    property string iconCountryAvailable: "image://theme/icon-m-location"
    property string iconCountryRequested: "image://theme/icon-m-cloud-download"
    property string iconFolder: "image://theme/icon-m-file-folder"
    property string iconIncompatibleVersion: "image://theme/icon-m-sync"
    property string iconMapnikGlobal: "image://theme/icon-m-image"
    property string iconPostalGlobal: "image://theme/icon-m-chat"
    property string iconRegion: "image://theme/icon-m-region"


    // item sizes
    property real themeItemSizeLarge: Theme.itemSizeLarge
    property real themeItemSizeSmall: Theme.itemSizeSmall
    property real themeItemSizeExtraSmall: Theme.itemSizeExtraSmall

    // paddings and page margins
    property real themeHorizontalPageMargin: Theme.horizontalPageMargin
    property real themePaddingLarge: Theme.paddingLarge
    property real themePaddingMedium: Theme.paddingMedium
    property real themePaddingSmall: Theme.paddingSmall

    property real themePixelRatio: Theme.pixelRatio
}
