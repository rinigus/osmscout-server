/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
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
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."

Dialog {

    id: page
    dialogue: false
    title: qsTr("Map Manager")

    property bool activeState: false
    property bool backendSelectionPossible: false

    contentHeight: column.height + Theme.paddingLarge

    Column {
        id: column

        x: Theme.horizontalPageMargin
        width: page.width-2*x

        spacing: Theme.paddingLarge

        Label {
            text: qsTr("Map Manager handles the storage of offline maps and " +
                       "related datasets on the device. Here you can select the " +
                       "features and countries that will be stored, updated or removed from the device")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
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
            id: start_button
            text: qsTr("Start download")
            enabled: page.activeState
            anchors.horizontalCenter: parent.horizontalCenter
            visible: manager.missing
            onClicked: {
                manager.getCountries()
            }

            function setVisibility() {
                visible = (manager.missing && !manager.downloading)
            }

            Component.onCompleted: {
                setVisibility()
            }

            Connections {
                target: manager
                onDownloadingChanged: start_button.setVisibility()
                onMissingChanged: start_button.setVisibility()
            }
        }

        Button {
            text: qsTr("Stop download")
            enabled: manager.downloading
            anchors.horizontalCenter: parent.horizontalCenter
            visible: manager.downloading
            onClicked: {
                manager.stopDownload()
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
            }
        }

        SectionHeader {
            text: qsTr("Storage settings")
        }

        Label {
            text: qsTr("Storage settings are set by the profile. " +
                       "If you wish to change storage settings, please set the corresponding profile " +
                       "or set profile to <i>Custom</i>.")
            visible: settings.profilesUsed
            x: Theme.horizontalPageMargin
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
                                 "These datasets consist of World coastlines (about 700 MB) and country-specific datasets used for rendering")

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
            secondaryLabel: qsTr("When selected, datasets with vector tiles in Mabox GL format will be stored on device after downloading them. " +
                                 "These datasets consist of World coastlines (about 12 MB), fonts (about 80 MB), and country-specific datasets")
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
    }


    function checkState()
    {
        page.activeState = manager.ready
        page.backendSelectionPossible = (page.activeState && !settings.profilesUsed)
    }

    Component.onCompleted: {
        checkState()
        if (manager.ready && !manager.checkProvidedAvailable())
            manager.updateProvided()
    }

    Connections {
        target: manager
        onReadyChanged: checkState()
        onUpdatesForDataFound: {
            var clist = JSON.parse( info )
            pageStack.push(Qt.resolvedUrl("UpdatesFound.qml"),
                           {"foundUpdates": clist})
        }
    }

    Connections {
        target: settings
        onProfilesUsedChanged: checkState()
    }
}
