import QtQuick 2.0
import Sailfish.Silica 1.0

Page 
{
    id: page

    SilicaFlickable {
        id: mainFlickable
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge
        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("OSM Scout Server")
                wrapMode: Text.WordWrap
            }

            Label {
                 text: qsTr("Module: Fonts")
                 anchors.right: parent.right
                 anchors.leftMargin: Theme.horizontalPageMargin
                 anchors.rightMargin: Theme.horizontalPageMargin
                 wrapMode: Text.WordWrap
                 font.pixelSize: Theme.fontSizeLarge
                 color: Theme.highlightColor
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.highlightColor

                text: qsTr("This is a module for OSM Scout Server. This module provides fonts for rendering maps.<br><br>" +
                           "You can safely close this application.<br><br>Please keep this application installed if you want " +
                           "to use OSM Scout Server.")
            }
        }
    }
}
