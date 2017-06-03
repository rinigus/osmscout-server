import QtQuick 2.0
import Sailfish.Silica 1.0

Page {

    id: page
    allowedOrientations : Orientation.All
    property bool activeState: false

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: parent.width
            spacing: Theme.paddingLarge
            anchors.margins: Theme.horizontalPageMargin

            PageHeader {
                title: qsTr("Map Manager")
            }

            Label {
                text: qsTr("Map Manager handles the storage of offline maps and " +
                           "related datasets on the device. Here you can select the " +
                           "features and countries that will be stored, updated or removed from the device")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            SectionHeader {
                text: qsTr("Storage settings")
            }

            ElementSwitch {
                id: eManagerMapnik
                activeState: page.activeState
                key: settingsMapManagerPrefix + "mapnik"
                autoApply: true
                mainLabel: qsTr("Store datasets for Mapnik")
                secondaryLabel: qsTr("When selected, datasets allowing rendering of maps with Mapnik will be stored on device after downloading them. " +
                                     "These datasets consist of World coastlines (about 700 MB) and country-specific datasets used for rendering")

                onSwitchChanged: {
                    // ensure that we have the same value for geocoder-nlp as postal
                    // on mobile device
                    settings.setValue( settingsMapManagerPrefix + "mapnik",
                                      settings.valueInt(settingsMapManagerPrefix + "mapnik") )
                }
            }

            ElementSwitch {
                id: eManagerGeocoderNLP
                activeState: page.activeState
                key: settingsMapManagerPrefix + "geocoder_nlp"
                autoApply: true
                mainLabel: qsTr("Store datasets for geocoder-nlp with libpostal")
                secondaryLabel: qsTr("When selected, libpostal-based geocoder datasets will be stored on device after downloading them. " +
                                     "These datasets consist of language parsing dataset (about 700 MB) and country-specific datasets used for " +
                                     "address parsing and lookup.")

                onSwitchChanged: {
                    // ensure that we have the same value for geocoder-nlp as postal
                    // on mobile device
                    settings.setValue( settingsMapManagerPrefix + "postal_country",
                                      settings.valueInt(settingsMapManagerPrefix + "geocoder_nlp") )
                }
            }

            ElementSwitch {
                id: eManagerValhalla
                activeState: page.activeState
                key: settingsMapManagerPrefix + "valhalla"
                autoApply: true
                mainLabel: qsTr("Store datasets for Valhalla routing engine")
                secondaryLabel: qsTr("When selected, Valhalla datasets will be stored on device after downloading them. " +
                                     "These datasets are required for using Valhalla as a routing engine.")
            }

            ElementSwitch {
                id: eManagerOSMScout
                activeState: page.activeState
                key: settingsMapManagerPrefix + "osmscout"
                autoApply: true
                mainLabel: qsTr("Store datasets for libosmscout")
                secondaryLabel: qsTr("When selected, libosmscout datasets will be stored on device after downloading them. " +
                                     "These datasets are required for rendering, search, or routing by libosmscout backend.")
            }

            SectionHeader {
                text: qsTr("Subscribed")
            }

            Column {
                id: subscolumn

                property int ncountries: 0
                property var countries: []

                signal updateData();

                width: parent.width
                spacing: Theme.paddingMedium

                Repeater {
                    width: parent.width
                    model: subscolumn.ncountries
                    delegate: ElementCountryListItem {
                        country: subscolumn.countries.children[index]

                        Connections {
                            target: subscolumn
                            onUpdateData: {
                                if (index < subscolumn.countries.children)
                                    country = subscolumn.countries.children[index]
                            }
                        }
                    }
                }

                Component.onCompleted: {
                    subscolumn.countries = JSON.parse(manager.getRequestedCountries())
                    subscolumn.ncountries = subscolumn.countries.children.length
                }

                Connections {
                    target: manager
                    onSubscriptionChanged: {
                        subscolumn.countries = JSON.parse(manager.getRequestedCountries())
                        subscolumn.ncountries = subscolumn.countries.children.length
                        subscolumn.updateData()
                    }
                }
            }

            SectionHeader {
                text: qsTr("Downloads")
            }

            ElementDownloads {
            }

            Label {
                id: missingInfo

                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                visible: manager.missing

                function updateText(info) {
                    text = qsTr("Missing data:<br>") + info
                }

                Component.onCompleted: {
                    updateText(manager.missingInfo())
                }

                Connections {
                    target: manager
                    onMissingInfoChanged: {
                        missingInfo.updateText(info)
                    }
                }
            }

            Button {
                text: qsTr("Start download")
                enabled: page.activeState
                preferredWidth: Theme.buttonWidthLarge
                anchors.horizontalCenter: parent.horizontalCenter
                visible: manager.missing
                onClicked: {
                    manager.getCountries()
                }
            }

            SectionHeader {
                text: qsTr("Provided maps")
            }

            Column {
                width: parent.width
                spacing: Theme.paddingMedium
                anchors.margins: Theme.horizontalPageMargin

                Button {
                    text: qsTr("Subscribe")
                    enabled: page.activeState
                    preferredWidth: Theme.buttonWidthLarge
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClicked: {
                        var clist = JSON.parse( manager.getProvidedCountries() )
                        pageStack.push(Qt.resolvedUrl("DownloadCountriesPage.qml"),
                                       { "countries": clist } )
                    }
                }

                Label {
                    text: qsTr("Select the countries or territories that you want to keep on device")
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
                    text: qsTr("Check for updates")
                    enabled: page.activeState
                    preferredWidth: Theme.buttonWidthLarge
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClicked: {
                        manager.updateProvided()
                    }
                }

                Label {
                    text: qsTr("Update the list of currently available maps and datasets and check " +
                               "if the installed maps can be updated")
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                }
            }

            SectionHeader {
                text: qsTr("Storage")
            }

            Column {
                width: parent.width
                spacing: Theme.paddingMedium
                anchors.margins: Theme.horizontalPageMargin

                Button {
                    text: qsTr("Start cleanup")
                    enabled: page.activeState
                    preferredWidth: Theme.buttonWidthLarge
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClicked: {
                        var clist = JSON.parse( manager.getProvidedCountries() )
                        pageStack.push(Qt.resolvedUrl("NonNeededFiles.qml"))
                    }
                }

                Label {
                    text: qsTr("Check whether there are files that are not used anymore from the " +
                               "earlier subscriptions")
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                }
            }
        }

        VerticalScrollDecorator {}
    }

    Component.onCompleted: {
        page.activeState = !manager.downloading
        if (!manager.downloading && !manager.checkProvidedAvailable())
            manager.updateProvided()
    }

    Connections {
        target: manager
        onDownloadingChanged: page.activeState = !manager.downloading
        onUpdatesForDataFound: {
            var clist = JSON.parse( info )
            pageStack.push(Qt.resolvedUrl("UpdatesFound.qml"),
                           {"foundUpdates": clist})
        }
    }
}
