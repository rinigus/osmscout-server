import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property string mainText: "<p>" +
                              qsTr("The server has to be used with the client program that is able to display maps, query for searches and " +
                                   "display selected routes. The server is a thin layer exposing functionality of <i>libosmscout</i>, " +
                                   "<i>libpostal</i> and <i>geocoder-nlp</i> through http. " +
                                   "This server can be used as a drop-in replacement for " +
                                   "online services providing maps.") + "</p><br>" +
                              "<p>" + qsTr("Copyright:") + " 2016, 2017 <br>rinigus https://github.com/rinigus<br></p>" +
                              qsTr("License: LGPL") + "<br><br>" +

                              "<b>" + qsTr("Translations") + "</b><br><br>" +
                              "Carmen F. B. @carmenfdezb: Spanish<br>" +
                              "Lukáš Karas @Karry: Czech<br>" +
                              "Åke Engelbrektson @eson57: Swedish<br>" +
                              "Ricardo Breitkopf @monkeyisland: German<br>" +
                              "Nathan Follens @pljmn: Dutch<br>" +

                              "<br><b>" + qsTr("Libpostal") + "</b><br>" +
                              "<p>" +
                              qsTr("Please note that <i>libpostal</i> is developed to be used with the fine tuned " +
                                   "model covering the World. The country-based models were developed to use " +
                                   "<i>libpostal</i> on mobile devices and have not been specifically tuned. Please submit the issues " +
                                   "with libpostal performance on country-based models to <i>OSM Scout Server</i> or " +
                                   "<i>geocoder-nlp</i> github projects.") +
                              "</p>" +

                              "<br><b>" + qsTr("Acknowledgments") + "</b><br><br>" +
                              "libosmscout: http://libosmscout.sourceforge.net/<br>" +
                              "libpostal: https://github.com/openvenues/libpostal<br>" +
                              "osmscout-sailfish: https://github.com/Karry/osmscout-sailfish<br>" +
                              "GNU Libmicrohttpd: https://www.gnu.org/software/libmicrohttpd<br>" +
                              "langcodes: https://github.com/LuminosoInsight/langcodes<br>"


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
                title: qsTr("About") + " " + programName
            }

            Label {
                text: qsTr("version") + ": " + programVersion
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label {
                text: "<center>" + programName + " " + qsTr("is a server that provides offline map services") + "</center>"
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
