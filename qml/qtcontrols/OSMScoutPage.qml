import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."

Dialog {

    id: dialog
    title: qsTr("OSM Scout <i>libosmscout</i>")

    // To enable PullDownMenu, place our content in a SilicaFlickable
    contentHeight: column.height + Theme.paddingLarge

    Column {
        id: column

        x: Theme.paddingLarge
        width: dialog.width-2*x
        spacing: Theme.paddingLarge

        Label {
            text: qsTr("OSM Scout library (<i>libosmscout</i>) can be used for drawing map, search, and routing")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
            //font.pointSize: Theme.fontSizeSmall
        }

        SectionHeader {
            text: qsTr("Rendering")
        }

        ElementEntry {
            id: eFontSize
            key: settingsOsmPrefix + "fontSize"
            mainLabel: qsTr("Font size")
            validator: DoubleValidator { bottom: 0; decimals: 1; }
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }

        ElementSelector {
            id: eStyle
            key: settingsOsmPrefix + "style"
            mainLabel: qsTr("Style Sheet")
            secondaryLabel: qsTr("Style sheet used to render the map tiles")
        }

        ElementSelector {
            id: eIcons
            key: settingsOsmPrefix + "icons"
            mainLabel: qsTr("Icons")
            secondaryLabel: qsTr("Icons used to mark features on the map")
            directory: true
            directory_file: "parking.png"
        }

        ElementSwitch {
            id: eRenderSea
            key: settingsOsmPrefix + "renderSea"
            mainLabel: qsTr("Render sea")
        }

        ElementSwitch {
            id: eDrawBackground
            key: settingsOsmPrefix + "drawBackground"
            mainLabel: qsTr("Draw background")
        }

        SectionHeader {
            text: qsTr("Routing")
        }

        Label {
            text: qsTr("Routing is calculated among the possible routes that comply with the cost limitation")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
            font.pointSize: Theme.fontSizeSmall
        }

        ElementEntry {
            id: eRoutingCostFactor
            key: settingsOsmPrefix + "routingCostLimitFactor"
            mainLabel: qsTr("Cost factor")
            secondaryLabel: qsTr("Cost factor is a component of a cost limit that is proportional to the geodesic distance " +
                                 "between the route origin and the target")
            validator: DoubleValidator { bottom: 1.0; decimals: 1; }
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }

        ElementEntry {
            id: eRoutingCostDistance
            key: settingsOsmPrefix + "routingCostLimitDistance"
            mainLabel: qsTr("Cost distance")
            secondaryLabel: qsTr("Cost distance is an offset of a cost limit")
            validator: DoubleValidator { bottom: 1.0; decimals: 0 }
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }

        Column {
            width: parent.width
            spacing: Theme.paddingMedium
            anchors.margins: Theme.horizontalPageMargin

            Rectangle {
                width: parent.width
                height: Theme.paddingLarge
                color: "transparent"
            }

            Button {
                text: qsTr("Routing speeds")
                onClicked: pageStack.push(Qt.resolvedUrl("OSMSpeedPage.qml"))
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label {
                text: qsTr("Speeds for different transportation modes (car, bicycle, walk) and " +
                           "at different road surfaces used when finding an optimal route")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pointSize: Theme.fontSizeSmall
            }
        }

        SectionHeader {
            text: qsTr("Advanced rendering settings")
        }

        ElementEntry {
            id: eDataLookupArea
            key: settingsOsmPrefix + "dataLookupArea"
            mainLabel: qsTr("Loaded area factor")
            secondaryLabel: qsTr("By increasing the size of the area which is used to load the data while rendering a tile, " +
                                 "the renderer can improve label rendering by taking into account data from neighboring tiles. " +
                                 "Factor value of 1.0 corresponds to loading data that originates from the rendered tile only. " +
                                 "The large value of the factor would decrease the rendering speed and increase RAM footprint " +
                                 "of the server. Due to its impact on RAM, depending on your tile size, factors as small as 2 " +
                                 "it may lead to filling up RAM of the device.")
            validator: DoubleValidator { bottom: 1.0; decimals: 1; }
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }

        ElementEntry {
            id: eTileBordersZoomCutoff
            key: settingsOsmPrefix + "tileBordersZoomCutoff"
            mainLabel: qsTr("Cutoff zoom level")
            secondaryLabel: qsTr("At the specified zoom level, the map rendering algorithm will stop checking " +
                                 "for labels in the neighboring tiles. Frequently, 14 is used as a cutoff zoom level. " +
                                 "By setting it to a larger value, you could get more accurate label rendering at larger zoom levels. " +
                                 "Note that the label rendering could significantly slow down the map rendering.")
            validator: IntValidator { bottom: 1;  }
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }
    }

    function onAccepted() {
        eStyle.apply()
        eIcons.apply()
        eFontSize.apply()
        eRenderSea.apply()
        eDrawBackground.apply()
        eDataLookupArea.apply()
        eTileBordersZoomCutoff.apply()
        eRoutingCostFactor.apply()
        eRoutingCostDistance.apply()
    }
}
