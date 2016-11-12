import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page
    property string mainText: "<p>" +
                              "The server has to be used with the client program that is able to display maps, query for searches and " +
                              "display selected routes. The server is a thin layer exposing functionality of <i>libosmscout</i> through http. " +
                              "This server can be used as a drop-in replacement for " +
                              "online services providing maps.</p><br>" +
                              "<p>Copyright: 2016 <br>rinigus https://github.com/rinigus<br></p>" +
                              "License: LGPL<br><br>" +
                              "<b>Acknowledgments:</b><br><br>" +
                              "libosmscout: http://libosmscout.sourceforge.net/<br>" +
                              "osmscout-sailfish: https://github.com/Karry/osmscout-sailfish<br>" +
                              "GNU Libmicrohttpd: https://www.gnu.org/software/libmicrohttpd<br>"


    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            anchors.margins: Theme.horizontalPageMargin
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("About " + programName)
            }

            Label {
                text: qsTr("version: ") + programVersion
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label {
                text: "<center>" + programName + " is a server that provides offline map services</center>"
                wrapMode: Text.WordWrap
                width: column.width
                textFormat: Text.RichText
            }

            Label {
                text: mainText
                wrapMode: Text.WordWrap
                width: column.width
                font.pixelSize: Theme.fontSizeSmall
                //textFormat: Text.RichText
            }
        }

        VerticalScrollDecorator {}
    }
}
