import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground 
{
    id: cover
    Image {
        id: icon
        source: "/usr/share/icons/hicolor/256x256/apps/harbour-osmscout-server-module-fonts.png"
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
        width: parent.width/2
    }

    Label {
        id: label
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.margins: Theme.paddingLarge
        text: qsTr("Module: Fonts")
    }
}
