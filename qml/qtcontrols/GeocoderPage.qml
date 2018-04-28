import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."

Dialog {

    id: dialog

    title: qsTr("Geocoder-NLP")

    contentHeight: column.height + Theme.paddingLarge

    Column {
        id: column

        x: Theme.horizontalPageMargin
        width: dialog.width - 2*x
        spacing: Theme.paddingLarge

        Label {
            text: qsTr("Geocoder is responsible for resolving search requests. " +
                       "For that, it parses the search string and finds the corresponding objects on a map.")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        Label {
            text: qsTr("Disclaimer: Please see About regarding reporting of the issues with the address parsing.")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
            font.pointSize: Theme.fontSizeSmall
        }

        ElementLanguageSelector {
            id: eGeoLanguages
            key: settingsGeomasterPrefix + "languages"
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

    function onAccepted() {
        eGeoInitEveryCall.apply()
        eGeoSearchAllMaps.apply()
        eGeoPrimitive.apply()
        eGeoLanguages.apply()
    }
}
