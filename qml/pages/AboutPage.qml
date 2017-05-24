import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property string mainText: "<p>" +
                              qsTr("The server has to be used with the client program that is able to display maps, query for searches and " +
                                   "display selected routes. The server is a thin layer exposing functionality of <i>libosmscout</i>, " +
                                   "<i>mapnik</i>, <i>valhalla</i>, <i>libpostal</i> and <i>geocoder-nlp</i> through http. " +
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
                              "@Sagittarii: French<br>" +
                              "Oleg Artobolevsky @XOleg: Russian<br>" +
                              "A @atlochowski: Polish<br>" +
                              "Peer-Atle Motland @Pam: Norwegian Bokmål<br>" +

                              "<br><b>" + qsTr("Maps") + "</b><br>" +
                              "<p>" +
                              qsTr("Maps are hosted by " +
                                   "Natural Language Processing Centre (Faculty of Informatics, Masaryk University, Brno, Czech Republic) " +
                                   "through modRana.org data repository.") +
                              "</p><br>" +
                              "<p>" +
                              qsTr("Map data from OpenStreetMap, Open Database License 1.0. Maps are converted to a suitable format " +
                                   "from downloaded extracts and/or using polygons as provided by Geofabrik GmbH.") +
                              "</p>" +

                              "<br><b>" + qsTr("Libpostal") + "</b><br>" +
                              "<p>" +
                              qsTr("Please note that <i>libpostal</i> is developed to be used with the fine tuned " +
                                   "model covering the World. The country-based models were developed to use " +
                                   "<i>libpostal</i> on mobile devices and have not been specifically tuned. Please submit the issues " +
                                   "with libpostal performance on country-based models to <i>OSM Scout Server</i> or " +
                                   "<i>geocoder-nlp</i> github projects.") +
                              "</p>" +

                              "<br><b>" + qsTr("Acknowledgments") + "</b><br><br>" +
                              "libosmscout: http://libosmscout.sourceforge.net/<br><br>" +
                              "libpostal: https://github.com/openvenues/libpostal<br><br>" +
                              "mapnik: http://mapnik.org/<br><br>" +
                              "valhalla: https://github.com/valhalla/valhalla<br><br>" +
                              "geocoder-nlp: https://github.com/rinigus/geocoder-nlp<br><br>" +
                              "osmscout-sailfish: https://github.com/Karry/osmscout-sailfish<br><br>" +
                              "GNU Libmicrohttpd: https://www.gnu.org/software/libmicrohttpd<br><br>" +
                              "langcodes: https://github.com/LuminosoInsight/langcodes<br><br>" +
                              "Hosting of maps: Natural Language Processing Centre (Faculty of Informatics, " +
                              "Masaryk University, Brno, Czech Republic) through modRana.org<br><br>" +
                              "Fonts: Noto fonts (SIL Open Font License, Version 1.1)<br>"


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
                color: Theme.highlightColor
            }

            Label {
                text: "<center>" + programName + " " + qsTr("is a server that provides offline map services") + "</center>"
                wrapMode: Text.WordWrap
                width: column.width
                textFormat: Text.RichText
                color: Theme.highlightColor
            }

            Label {
                text: mainText
                wrapMode: Text.WordWrap
                width: column.width
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                //textFormat: Text.RichText
            }
        }

        VerticalScrollDecorator {}
    }
}
