/*
 * Copyright (C) 2016-2019 Rinigus https://github.com/rinigus
 *                    2019 Purism SPC
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
import "."
import "platform"

DialogPL {
    id: dialog
    title: qsTr("Settings")

    property bool backendSelectionPossible: false

    Column {
        spacing: styler.themePaddingLarge
        width: dialog.width

        FormLayoutPL {
            spacing: styler.themePaddingLarge

            ComboBoxPL {
                id: unitsBox
                description: qsTr("Units used in the graphical user interface of the server. The units will change only after you apply the settings.")
                label: qsTr("Units")
                model: [ qsTr("Metric"), qsTr("Imperial") ]
                Component.onCompleted: unitsBox.currentIndex = settings.unitIndex();
            }

            ComboBoxPL {
                id: preferredLanguageSelection
                description: qsTr("Preferred language for location names shown in rendered maps or in the returned search results. " +
                                  "When possible, this language will be used. When set to <i>Default</i>, OpenStreetMap name will be used " +
                                  "which usually defaults to local language of the displayed country.")
                enabled: manager.ready
                label: qsTr("Language")
                model: [ qsTr("Default"), qsTr("English") ]
                Component.onCompleted: currentIndex = settings.valueInt(settingsGeneralPrefix + "language")
            }
        }

        ElementSelector {
            id: eMapsRoot
            directory: true
            key: settingsMapManagerPrefix + "root"
            mainLabel: qsTr("Maps storage")
            secondaryLabel: qsTr("Folder to store maps.<br><b>NB!</b> This folder will be fully managed by OSM Scout Server. " +
                                 "Please <b>allocate separate, empty folder</b> that OSM Scout Server could use. " +
                                 "This includes deleting all files within that folder when requested by you during cleanup or " +
                                 "map updates.<br>" +
                                 "<i>Example:</i> Maps under <i>~nemo</i>")
        }

        ///////////////////////////////////////
        /// systemd support
        Column {
            spacing: styler.themePaddingMedium
            visible: settings.hasBackendSystemD
            width: parent.width

            TextSwitchPL {
                id: systemdEnable
                enabled: manager.ready
                text: qsTr("Automatic activation")
                Component.onCompleted: checked = systemd_service.enabled
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("When enabled, OSM Scout Server will be activated automatically by any client accessing it. " +
                           "Automatically started server will work in the background."
                           )
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("It is recommended to enable automatic activation to simplify the access to the server.")
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("For technical details, created files, and how to remove them if needed, see corresponding section " +
                           "of the <a href='https://rinigus.github.io/osmscout-server/en/#implementation-of-automatic-activation'>User's Guide</a>.")
            }
        }

        Column {
            spacing: styler.themePaddingMedium
            visible: (!settings.hasBackendSystemD || systemdEnable.checked)
            width: parent.width

            ComboBoxPL {
                id: idleTimeout
                enabled: manager.ready
                label: qsTr("Idle timeout")
                model: timeouts.map(function (t) { return t.desc; } )

                property var timeouts: [
                    {"value": 900, "desc": qsTr("15 minutes") },
                    {"value": 1800, "desc": qsTr("30 minutes") },
                    {"value": 3600, "desc": qsTr("1 hour")},
                    {"value": 7200, "desc": qsTr("2 hours")},
                    {"value": 14400, "desc": qsTr("4 hours")},
                    {"value": 28800, "desc": qsTr("8 hours")},
                    {"value": 86400, "desc": qsTr("24 hours")},
                    {"value": -1, "desc": qsTr("No timeout")}
                ]

                Component.onCompleted: {
                    var timeout = settings.valueInt(settingsRequestMapperPrefix + "idle_timeout")
                    if (timeout < 0) {
                        currentIndex = 7
                        return
                    }

                    for (var i = 0; i < timeouts.length-2; i++)
                        if ( timeout <= timeouts[i]["value"] ) {
                            currentIndex = i
                            return
                        }

                    currentIndex = timeouts.length-2
                }

                function apply() {
                    settings.setValue(settingsRequestMapperPrefix + "idle_timeout", timeouts[currentIndex]["value"])
                }
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("When started automatically, the server will shutdown itself after " +
                           "not receiving any requests for longer than the idle timeout")
            }
        }

        ///////////////////////////////////////

        SectionHeaderPL {
            text: qsTr("Profiles")
            visible: settings.profilesUsed
        }

        ListItemLabel {
            text: qsTr("Active backends are set by the profile. " +
                       "If you wish to change the backend selection, please set the corresponding profile " +
                       "or set profile to <i>Custom</i>.")
            visible: settings.profilesUsed
        }

        Column {
            spacing: styler.themePaddingLarge
            visible: !settings.profilesUsed
            width: dialog.width

            SectionHeaderPL {
                text: qsTr("Rendering")
                visible: cbMapnik.visible
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Rendering backend is responsible for drawing the maps on raster tiles.")
                visible: cbMapnik.visible
            }

            ComboBoxPL {
                id: cbMapnik
                enabled: dialog.backendSelectionPossible
                label: qsTr("Renderer")
                model: [
                    qsTr("Mapnik (default)"),
                    qsTr("libosmscout")
                ]
                visible: settings.hasBackendMapnik && settings.hasBackendOsmScout
                Component.onCompleted: {
                    if (settings.valueInt(settingsMapnikPrefix + "use_mapnik") > 0)
                        currentIndex = 0;
                    else
                        currentIndex = 1;
                }
            }

            SectionHeaderPL {
                text: qsTr("Geocoder")
                visible: cbGeocoder.visible
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Geocoder is responsible for resolving search requests. " +
                           "For that, it parses the search string and finds the corresponding objects on a map.")
                visible: cbGeocoder.visible
            }

            ComboBoxPL {
                id: cbGeocoder
                enabled: dialog.backendSelectionPossible
                label: qsTr("Geocoder")
                model: [
                    qsTr("Geocoder-NLP (default)"),
                    qsTr("libosmscout")
                ]
                visible: settings.hasBackendOsmScout
                Component.onCompleted: {
                    if (settings.valueInt(settingsGeomasterPrefix + "use_geocoder_nlp") > 0)
                        currentIndex = 0;
                    else
                        currentIndex = 1;
                }
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("NB! If you select <i>Geocoder-NLP</i>, please specify languages that should be used for " +
                           "address parsing in the backend settings below. Otherwise, the server could use large amounts of RAM.")
                visible: cbGeocoder.currentIndex === 0 && cbGeocoder.visible
            }

            SectionHeaderPL {
                text: qsTr("Routing Engine")
                visible: cbRouter.visible
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Routing engine is responsible for calculating routes between origin and destination.")
                visible: cbRouter.visible
            }

            ComboBoxPL {
                id: cbRouter
                enabled: dialog.backendSelectionPossible
                label: qsTr("Routing Engine")
                model: [
                    qsTr("Valhalla (default)"),
                    qsTr("libosmscout")
                ]
                visible: settings.hasBackendMapnik && settings.hasBackendOsmScout
                Component.onCompleted: {
                    if (settings.valueInt(settingsValhallaPrefix + "use_valhalla") > 0)
                        currentIndex = 0;
                    else
                        currentIndex = 1;
                }
            }
        }

        SectionHeaderPL {
            text: qsTr("Backend settings")
        }

        Column {
            width: parent.width
            spacing: styler.themePaddingMedium
            visible: settings.hasBackendMapnik

            ButtonPL {
                anchors.horizontalCenter: parent.horizontalCenter
                preferredWidth: styler.themeButtonWidthLarge
                text: qsTr("Mapnik")
                onClicked: app.push(Qt.resolvedUrl("MapnikPage.qml"))
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Map rendering settings for <i>mapnik</i> backend")
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
                preferredWidth: styler.themeButtonWidthLarge
                text: qsTr("Geocoder-NLP")
                onClicked: app.push(Qt.resolvedUrl("GeocoderPage.qml"))
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Selection of languages used for address parsing and other settings of <i>geocoder-nlp</i> backend")
            }
        }

        Column {
            width: parent.width
            spacing: styler.themePaddingMedium
            visible: settings.hasBackendValhalla

            Rectangle {
                color: "transparent"
                height: styler.themePaddingLarge
                width: parent.width
            }

            ButtonPL {
                anchors.horizontalCenter: parent.horizontalCenter
                preferredWidth: styler.themeButtonWidthLarge
                text: qsTr("Valhalla")
                onClicked: app.push(Qt.resolvedUrl("ValhallaPage.qml"))
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Routing engine settings for <i>Valhalla</i> backend")
            }
        }

        Column {
            width: parent.width
            spacing: styler.themePaddingMedium
            visible: settings.hasBackendOsmScout

            Rectangle {
                width: parent.width
                height: styler.themePaddingLarge
                color: "transparent"
            }

            ButtonPL {
                anchors.horizontalCenter: parent.horizontalCenter
                preferredWidth: styler.themeButtonWidthLarge
                text: qsTr("OSM Scout library")
                onClicked: app.push(Qt.resolvedUrl("OSMScoutPage.qml"))
            }

            ListItemLabel {
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("OSM Scout library (<i>libosmscout</i>) settings. " +
                           "The library can be used for drawing maps, search, and routing.")
            }
        }

        SectionHeaderPL {
            text: qsTr("Miscellaneous")
        }

        ElementSwitch {
            id: eLogInfo
            key: settingsGeneralPrefix + "logInfo"
            mainLabel: qsTr("Log info messages")
            secondaryLabel: qsTr( "When disabled, INFO messages will not be logged in Events log. " +
                                 "Exception is loading of the database which is shown always." )
        }

        ElementEntry {
            id: eRollSize
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            key: settingsGeneralPrefix + "rollingLoggerSize"
            mainLabel: qsTr("Events log size")
            secondaryLabel: qsTr("Number of events shown in the main page")
            validator: IntValidator { bottom: 3; top: 25; }
        }

        ElementSwitch {
            id: eLogSession
            key: settingsGeneralPrefix + "logSession"
            mainLabel: qsTr("Log messages into session log file")
            secondaryLabel: qsTr("When enabled, the messages are logged into a session log file. " +
                                 "The log file is at .cache/harbour-osmscout-server directory. " +
                                 "Use this setting only for debugging purposes. With this setting enabled, " +
                                 "info messages will be logged and shown on events irrespective to the " +
                                 "settings above. This allows you to temporary enable full logging and disable it " +
                                 "when the required session log file was produced." )
        }

        Connections {
            target: settings
            onProfilesUsedChanged: checkState()
        }
    }

    Component.onCompleted: {
        checkState()
    }

    onAccepted: {
        eLogInfo.apply()
        eRollSize.apply()
        eLogSession.apply()

        /// preferred languages are done by combo box, have to apply manually
        settings.setValue(settingsGeneralPrefix + "language", preferredLanguageSelection.currentIndex)

        eMapsRoot.apply()
        if (cbMapnik.visible)
            settings.setValue(settingsMapnikPrefix + "use_mapnik", cbMapnik.currentIndex ? 0 : 1)
        if (cbGeocoder.visible)
            settings.setValue(settingsGeomasterPrefix + "use_geocoder_nlp", cbGeocoder.currentIndex ? 0 : 1)
        if (cbRouter.visible)
            settings.setValue(settingsValhallaPrefix + "use_valhalla", cbRouter.currentIndex ? 0 : 1)

        /// units are changed the last
        settings.setValue(settingsGeneralPrefix + "units", unitsBox.currentIndex)

        if (settings.hasBackendSystemD) {
            systemd_service.enabled = systemdEnable.checked
            idleTimeout.apply()
        }
    }

    function checkState() {
        dialog.backendSelectionPossible = !settings.profilesUsed
    }
}
