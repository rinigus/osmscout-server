import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {

    id: dialog
    allowedOrientations : Orientation.All

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: parent.width
            spacing: Theme.paddingLarge
            anchors.margins: Theme.horizontalPageMargin

            DialogHeader {
                title: qsTr("Settings")
            }

            ElementSelector {
                id: eMap
                key: settingsOsmPrefix + "map"
                mainLabel: qsTr("Map")
                secondaryLabel: qsTr("Location of the folder with a map imported by libosmscout Import utility")
                directory: true
                directory_file: "types.dat"
            }

            Column {
                width: parent.width
                spacing: Theme.paddingMedium
                anchors.margins: Theme.horizontalPageMargin

                ComboBox {
                    id: unitsBox
                    label: qsTr("Units")
                    menu: ContextMenu {
                        MenuItem { text: qsTr("Metric") }
                        MenuItem { text: qsTr("Imperial") }
                    }
                    Component.onCompleted: {
                        unitsBox.currentIndex = settings.unitIndex();
                    }
                }
                Label {
                    text: qsTr("Units used in the graphical user interface of the server. The units will change only after you apply the settings.")
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                }
            }


            SectionHeader {
                text: qsTr("Rendering")
                font.pixelSize: Theme.fontSizeMedium
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
                font.pixelSize: Theme.fontSizeMedium
            }

            Label {
                text: qsTr("Routing is calculated among the possible routes that comply with the cost limitation")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
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

            SectionHeader {
                text: qsTr("Advanced rendering settings")
                font.pixelSize: Theme.fontSizeMedium
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

            SectionHeader {
                text: qsTr("Miscellaneous")
                font.pixelSize: Theme.fontSizeMedium
            }

            ElementSwitch {
                id: eLogInfo
                key: settingsOsmPrefix + "logInfo"
                mainLabel: qsTr("Log info messages")
                secondaryLabel: qsTr( "When disabled, INFO messages will not be logged in Events log. " +
                                     "Exception is loading of the database which is shown always" )
            }

            ElementEntry {
                id: eRollSize
                key: settingsOsmPrefix + "rollingLoggerSize"
                mainLabel: qsTr("Events log size")
                secondaryLabel: qsTr("Number of events shown in the main page")
                validator: IntValidator { bottom: 3; top: 25; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementSwitch {
                id: eLogSession
                key: settingsOsmPrefix + "logSession"
                mainLabel: qsTr("Log messages into session log file")
                secondaryLabel: qsTr("When enabled, the messages are logged into a session log file." +
                                     "The log file is at .cache/harbour-osmscout-server directory. " +
                                     "Use this setting only for debugging purposes. With this setting enabled, " +
                                     "info messages will be logged and shown on events irrespective to the " +
                                     "settings above. This allows you to temporary enable full logging and disable it " +
                                     "when the required session log file was produced." )
            }
        }

        VerticalScrollDecorator {}
    }

    onAccepted: {
        eLogInfo.apply()
        eRollSize.apply()
        eLogSession.apply()
        eMap.apply()
        eStyle.apply()
        eIcons.apply()
        eFontSize.apply()
        eRenderSea.apply()
        eDrawBackground.apply()
        eDataLookupArea.apply()
        eTileBordersZoomCutoff.apply()
        eRoutingCostFactor.apply()
        eRoutingCostDistance.apply()

        /// units are done by combo box, have to apply manually
        /// units are changed the last
        settings.setValue(settingsOsmPrefix + "units", unitsBox.currentIndex)
    }
}


