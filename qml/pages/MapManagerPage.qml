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
                font.pixelSize: Theme.fontSizeMedium
            }

            ElementSwitch {
                id: eManagerGeocoderNLP
                enabled: page.activeState
                key: settingsMapManagerPrefix + "geocoder_nlp"
                autoApply: true
                mainLabel: qsTr("Store datasets for geocoder-nlp with libpostal")
                secondaryLabel: qsTr("When selected, a libpostal-based geocoder datasets will be stored on device after downloading them. " +
                                     "These datasets consist of language parsing dataset (about 700 MB) and country-specific datasets used for " +
                                     "address parsing and lookup.")

                onSwitchChanged: {
                    // ensure that we have the same value for geocoder-nlp as postal
                    // on mobile device
                    settings.setValue( settingsMapManagerPrefix + "postal_country",
                                      settings.valueInt(settingsMapManagerPrefix + "geocoder_nlp") )
                }
            }

            SectionHeader {
                text: qsTr("Subscribed")
                font.pixelSize: Theme.fontSizeMedium
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
                font.pixelSize: Theme.fontSizeMedium
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
                font.pixelSize: Theme.fontSizeMedium
            }

            Column {
                width: parent.width
                spacing: Theme.paddingMedium
                anchors.margins: Theme.horizontalPageMargin

                Button {
                    text: qsTr("Update list")
                    enabled: page.activeState
                    preferredWidth: Theme.buttonWidthLarge
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClicked: {
                        if (!manager.updateProvided())
                            console.log("Could not start the download. Perhaps you are downloading something already. " +
                                        "Please wait till the current download is finished")
                    }
                }

                Label {
                    text: qsTr("List of currently available maps and datasets")
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
        }

        VerticalScrollDecorator {}
    }

    Component.onCompleted: {
        page.activeState = !manager.downloading
    }

    Connections {
        target: manager
        onDownloadingChanged: page.activeState = !manager.downloading
    }
}
