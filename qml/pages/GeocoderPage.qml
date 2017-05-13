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
                title: qsTr("Geocoder-NLP")
            }

            Label {
                text: qsTr("Geocoder is responsible for resolving search requests. " +
                           "For that, it parses the search string and finds the corresponoding objects on a map.")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            Label {
                text: qsTr("Disclaimer: Please see About regarding reporting of the issues with the address parsing.")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            ElementLanguageSelector {
                id: eGeoLanguages
                key: settingsGeomasterPrefix + "languages"
                mainLabel: qsTr("Languages")
                secondaryLabel: qsTr("List of languages used for parsing addresses. " +
                                     "Note that when all supported languages are used, RAM requirements could be large")
            }

            ElementSwitch {
                id: eGeoSearchAllMaps
                key: settingsGeomasterPrefix + "search_all_maps"
                mainLabel: qsTr("Search all available maps")
                secondaryLabel: qsTr("When enabled, each search will be performed using all available maps on the device")
            }

            ElementSwitch {
                id: eGeoPrimitive
                key: settingsGeomasterPrefix + "use_primitive"
                mainLabel: qsTr("Use primitive parser")
                secondaryLabel: qsTr("In addition to <i>libpostal</i>, primitive parser allows you to specify " +
                                     "administrative hierarchy of a searched object by separating components with a comma. " +
                                     "Sometimes, when libpostal fails to parse the request correctly, this parser allows " +
                                     "you to overcome the issue.<br>Example: house_number, street, town.")
            }

            ElementSwitch {
                id: eGeoInitEveryCall
                key: settingsGeomasterPrefix + "initialize_every_call"
                mainLabel: qsTr("Load <i>libpostal</i> on every call")
                secondaryLabel: qsTr("When selected, <i>libpostal</i> databases will be loaded to the memory only " +
                                     "while parsing the request. As a result, while search would take longer time, " +
                                     "the memory overhead of libpostal is small in idle.")
            }
        }

        VerticalScrollDecorator {}
    }

    onAccepted: {
        eGeoInitEveryCall.apply()
        eGeoSearchAllMaps.apply()
        eGeoPrimitive.apply()
        eGeoLanguages.apply()
    }
}
