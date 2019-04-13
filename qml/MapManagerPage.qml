/*
 * Copyright (C) 2016-2019 Rinigus https://github.com/rinigus
 *
 * This file is part of OSM Scout Server.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import "platform"

PagePL {

    id: page
    title: qsTr("Map Manager")

    property bool activeState: false
    property bool backendSelectionPossible: false

    Column {
        id: column

        width: parent.width
        spacing: styler.themePaddingLarge

        LabelPL {
            color: styler.themeHighlightColor
            text: qsTr("Map Manager handles the storage of offline maps and " +
                       "related datasets on the device. Here you can select the " +
                       "features and countries that will be stored, updated or removed from the device.")
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        SectionHeaderPL {
            text: qsTr("Subscribed")
        }

        Column {
            id: subscolumn
            width: parent.width
            spacing: styler.themePaddingMedium

            property int ncountries: 0
            property var countries: []

            signal updateData();

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

        SectionHeaderPL {
            text: qsTr("Downloads")
        }

        ElementDownloads {
        }

        LabelPL {
            id: missingInfo

            color: styler.themeHighlightColor
            font.pixelSize: styler.themeFontSizeSmall
            visible: manager.missing
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap

            Connections {
                target: manager
                onMissingInfoChanged: {
                    missingInfo.updateText(info)
                }
            }

            Component.onCompleted: {
                updateText(manager.missingInfo())
            }

            function updateText(info) {
                text = qsTr("Missing data:<br>") + info
            }
        }

        ButtonPL {
            id: start_button
            anchors.horizontalCenter: parent.horizontalCenter
            enabled: page.activeState
            preferredWidth: styler.themeButtonWidthLarge
            text: qsTr("Start download")
            visible: manager.missing

            Connections {
                target: manager
                onDownloadingChanged: start_button.setVisibility()
                onMissingChanged: start_button.setVisibility()
            }

            Component.onCompleted: setVisibility()
            onClicked: manager.getCountries()

            function setVisibility() {
                visible = (manager.missing && !manager.downloading)
            }
        }

        ButtonPL {
            anchors.horizontalCenter: parent.horizontalCenter
            enabled: manager.downloading
            preferredWidth: styler.themeButtonWidthLarge
            text: qsTr("Stop download")
            visible: manager.downloading
            onClicked: manager.stopDownload()
        }

        SectionHeaderPL {
            text: qsTr("Provided maps")
        }

        Column {
            width: parent.width
            spacing: styler.themePaddingMedium

            ButtonPL {
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: page.activeState
                preferredWidth: styler.themeButtonWidthLarge
                text: qsTr("Subscribe")
                onClicked: {
                    var clist = JSON.parse( manager.getProvidedCountries() )
                    app.push(Qt.resolvedUrl("DownloadCountriesPage.qml"),
                             { "countries": clist } )
                }
            }

            LabelPL {
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Select the countries or territories that you want to keep on device")
                x: styler.themeHorizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }
        }

        Column {
            width: parent.width
            spacing: styler.themePaddingMedium

            Rectangle {
                width: parent.width
                height: styler.themePaddingLarge
                color: "transparent"
            }

            ButtonPL {
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: page.activeState
                preferredWidth: styler.themeButtonWidthLarge
                text: qsTr("Check for updates")
                onClicked: manager.updateProvided()
            }

            LabelPL {
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Update the list of currently available maps and datasets and check " +
                           "if the installed maps can be updated")
                x: styler.themeHorizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }
        }

        SectionHeaderPL {
            text: qsTr("Storage")
        }

        Column {
            width: parent.width
            spacing: styler.themePaddingMedium

            ButtonPL {
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: page.activeState
                preferredWidth: styler.themeButtonWidthLarge
                text: qsTr("Start cleanup")
                onClicked: {
                    var clist = JSON.parse( manager.getProvidedCountries() )
                    app.push(Qt.resolvedUrl("NonNeededFiles.qml"))
                }
            }

            LabelPL {
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Check whether there are files that are not used anymore from the " +
                           "earlier subscriptions")
                x: styler.themeHorizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }
        }

        SectionHeaderPL {
            text: qsTr("Storage settings")
        }

        LabelPL {
            color: styler.themeHighlightColor
            text: qsTr("Storage settings are set by the profile. " +
                       "If you wish to change storage settings, please set the corresponding profile " +
                       "or set profile to <i>Custom</i>.")
            visible: settings.profilesUsed
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        ElementSwitch {
            id: eManagerMapnik
            activeState: page.backendSelectionPossible
            key: settingsMapManagerPrefix + "mapnik"
            autoApply: true
            mainLabel: qsTr("Store datasets for Mapnik")
            secondaryLabel: qsTr("When selected, datasets allowing rendering of maps with Mapnik will be stored on device after downloading them. " +
                                 "These datasets consist of World coastlines (about 700 MB) and country-specific datasets used for rendering.")

            onSwitchChanged: {
                // ensure that we have the same value for geocoder-nlp as postal
                // on mobile device
                settings.setValue( settingsMapManagerPrefix + "mapnik",
                                  settings.valueInt(settingsMapManagerPrefix + "mapnik") )
            }
        }

        ElementSwitch {
            id: eManagerMapbox
            activeState: page.backendSelectionPossible
            key: settingsMapManagerPrefix + "mapboxgl"
            autoApply: true
            mainLabel: qsTr("Store datasets for Mapbox GL")
            secondaryLabel: qsTr("When selected, datasets with vector tiles in Mapbox GL format will be stored on device after downloading them. " +
                                 "These datasets consist of World coastlines (about 12 MB), fonts (about 80 MB) and country-specific datasets.")
        }

        ElementSwitch {
            id: eManagerGeocoderNLP
            activeState: page.backendSelectionPossible
            key: settingsMapManagerPrefix + "geocoder_nlp"
            autoApply: true
            mainLabel: qsTr("Store datasets for geocoder-nlp with libpostal")
            secondaryLabel: qsTr("When selected, libpostal-based geocoder datasets will be stored on device after downloading them. " +
                                 "These datasets consist of language parsing dataset (about 700 MB) and country-specific datasets used for " +
                                 "address parsing and lookup.")
        }

        ElementSwitch {
            id: eManagerValhalla
            activeState: page.backendSelectionPossible
            key: settingsMapManagerPrefix + "valhalla"
            autoApply: true
            mainLabel: qsTr("Store datasets for Valhalla routing engine")
            secondaryLabel: qsTr("When selected, Valhalla datasets will be stored on device after downloading them. " +
                                 "These datasets are required for using Valhalla as a routing engine.")
        }

        ElementSwitch {
            id: eManagerOSMScout
            activeState: page.backendSelectionPossible
            key: settingsMapManagerPrefix + "osmscout"
            autoApply: true
            mainLabel: qsTr("Store datasets for libosmscout")
            secondaryLabel: qsTr("When selected, libosmscout datasets will be stored on device after downloading them. " +
                                 "These datasets are required for rendering, search, or routing by libosmscout backend.")
        }

        Connections {
            target: manager
            onReadyChanged: checkState()
            onUpdatesForDataFound: {
                var clist = JSON.parse( info )
                app.push(Qt.resolvedUrl("UpdatesFound.qml"),
                         {"foundUpdates": clist})
            }
        }

        Connections {
            target: settings
            onProfilesUsedChanged: checkState()
        }
    }

    Component.onCompleted: {
        checkState()
        if (manager.ready && !manager.checkProvidedAvailable())
            manager.updateProvided()
    }

    function checkState() {
        page.activeState = manager.ready
        page.backendSelectionPossible = (page.activeState && !settings.profilesUsed)
    }
}
