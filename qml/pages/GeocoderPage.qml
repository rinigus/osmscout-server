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
                title: qsTr("Geocoder")
            }

            Label {
                text: qsTr("Geocoder is responsible for responding to search requests. " +
                           "For that, it parses the search string and finds the corresponoding objects on a map.")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            ElementSwitch {
                id: eGeocoderNLP
                key: settingsGeomasterPrefix + "use_geocoder_nlp"
                mainLabel: qsTr("Use Geocoder-NLP with libpostal as a geocoder")
                secondaryLabel: qsTr("When selected, a libpostal-based geocoder will be used to resolve all search requests. " +
                                     "Note that it requires additional databases for language, user input parsing, and geocoding. " +
                                     "The additional storage and RAM requirements could be significant, depending on selected " +
                                     "languages and country.")
            }

            SectionHeader {
                text: qsTr("Geocoder-NLP and libpostal")
                font.pixelSize: Theme.fontSizeMedium
            }

            ElementSelector {
                id: ePostalGeneral
                key: settingsGeomasterPrefix + "postal_main_dir"
                mainLabel: qsTr("Libpostal language parser")
                secondaryLabel: qsTr("Location of the folder with a language parser databases of libpostal, such as language_classifier")
                directory: true
                directory_file: "language_classifier"
            }

            ElementSelector {
                id: ePostalCountry
                key: settingsGeomasterPrefix + "postal_country_dir"
                mainLabel: qsTr("Country-specific database")
                secondaryLabel: qsTr("Location of the folder with a country-specific databases of libpostal, such as address_parser")
                directory: true
                directory_file: "address_parser"
            }

            ElementSelector {
                id: eGeoDatabase
                key: settingsGeomasterPrefix + "geocoder_path"
                mainLabel: qsTr("Geocoder database")
                secondaryLabel: qsTr("Location of the SQLite geocoder-nlp database")
            }

            ElementEntry {
                id: eGeoLanguages
                key: settingsGeomasterPrefix + "languages"
                mainLabel: qsTr("Languages")
                secondaryLabel: qsTr("Comma-separated list of two-letter language codes (ISO 639-1)")
            }

            ElementSwitch {
                id: eGeoPrimitive
                key: settingsGeomasterPrefix + "use_primitive"
                mainLabel: qsTr("Use primitive parser")
                secondaryLabel: qsTr("In addition to libpostal, primitive parser allows you to specify " +
                                     "administrative hierarchy of a searched object by separating components with a comma. " +
                                     "Sometimes, when libpostal fails to parse the request correctly, this parser allows " +
                                     "you to overcome the issue. Example: house_number, street, town.")
            }

            ElementSwitch {
                id: eGeoInitEveryCall
                key: settingsGeomasterPrefix + "initialize_every_call"
                mainLabel: qsTr("Load libpostal on every call")
                secondaryLabel: qsTr("When selected, libpostal databases will be loaded to the memory only " +
                                     "while parsing the request. As a result, while search would take longer time, " +
                                     "the memory overhead of libpostal is small in idle.")
            }
        }

        VerticalScrollDecorator {}
    }

    onAccepted: {
        eGeoInitEveryCall.apply()
        eGeoPrimitive.apply()
        eGeoLanguages.apply()
        eGeoDatabase.apply()
        ePostalCountry.apply()
        ePostalGeneral.apply()
        eGeocoderNLP.apply()
    }
}


