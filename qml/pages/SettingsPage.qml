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

            ElementSelector {
                id: eMapsRoot
                key: settingsMapManagerPrefix + "root"
                mainLabel: qsTr("Maps storage")
                secondaryLabel: qsTr("Folder to store maps.<br><b>NB!</b> This folder will be fully managed by OSM Scout Server. " +
                                     "Please <b>allocate separate, empty folder</b> that OSM Scout Server could use. " +
                                     "This includes deleting all files within that folder when requested by you during cleanup or " +
                                     "map updates.<br>" +
                                     "<i>Example:</i> /home/nemo/Maps")
                directory: true
            }

            SectionHeader {
                text: qsTr("Geocoder")
                font.pixelSize: Theme.fontSizeMedium
            }

            Label {
                text: qsTr("Geocoder is responsible for resolving search requests. " +
                           "For that, it parses the search string and finds the corresponoding objects on a map." +
                           "This server allows you to select between two geocoder backends: <i>geocoder-nlp</i> and " +
                           "<i>libosmscout</i>. ")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            ElementSwitch {
                id: eGeocoderNLP
                key: settingsGeomasterPrefix + "use_geocoder_nlp"
                mainLabel: qsTr("Use geocoder-nlp with libpostal as a geocoder")
                secondaryLabel: qsTr("When selected, a libpostal-based geocoder will be used to resolve all search requests. " +
                                     "Note that it requires additional databases for language, user input parsing, and geocoding.")
            }

            SectionHeader {
                text: qsTr("Backend settings")
                font.pixelSize: Theme.fontSizeMedium
            }

            Column {
                width: parent.width
                spacing: Theme.paddingMedium
                anchors.margins: Theme.horizontalPageMargin

                Button {
                    text: qsTr("OSM Scout backend")
                    onClicked: pageStack.push(Qt.resolvedUrl("OSMScoutPage.qml"))
                    preferredWidth: Theme.buttonWidthLarge
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Label {
                    text: qsTr("OSM Scout library (<i>libosmscout</i>) settings. The library is used for drawing maps, search, and routing.")
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                }
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
                    text: qsTr("Geocoder-NLP backend")
                    onClicked: pageStack.push(Qt.resolvedUrl("GeocoderPage.qml"))
                    preferredWidth: Theme.buttonWidthLarge
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Label {
                    text: qsTr("Selection of languages used for address parsing and other settings of <i>geocoder-nlp</i> backend")
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                }
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
                                     "Exception is loading of the database which is shown always." )
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

        /// units are done by combo box, have to apply manually
        /// units are changed the last
        settings.setValue(settingsOsmPrefix + "units", unitsBox.currentIndex)

        eMapsRoot.apply()
        eGeocoderNLP.apply()
    }
}
