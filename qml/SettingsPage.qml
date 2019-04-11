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
import "platform"

DialogPL {
    id: dialog
    title: qsTr("Settings")

    property bool backendSelectionPossible: false

    Column {
        id: column

        spacing: styler.themePaddingLarge
        width: dialog.width

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
            width: parent.width
            spacing: styler.themePaddingMedium

            TextSwitchPL {
                id: systemdEnable
                enabled: manager.ready
                text: qsTr("Automatic activation")
                Component.onCompleted: checked = systemd_service.enabled
            }

            LabelPL {
                text: qsTr("When enabled, OSM Scout Server will be activated automatically by any client accessing it. " +
                           "Automatically started server will work in the background."
                           )
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                x: styler.themeHorizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }

            LabelPL {
                text: qsTr("It is recommended to enable automatic activation to simplify the access to the server.")
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                x: styler.themeHorizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }

            LabelPL {
                text: qsTr("For technical details, created files, and how to remove them if needed, see corresponding section " +
                           "of the <a href='https://rinigus.github.io/osmscout-server/en/#implementation-of-automatic-activation'>User's Guide</a>.")
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                linkColor: styler.themePrimaryColor
                x: styler.themeHorizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                onLinkActivated: Qt.openUrlExternally(link)
            }

            ComboBoxPL {
                id: idleTimeout
                enabled: manager.ready && systemdEnable.checked
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

            LabelPL {
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("When started automatically, the server will shutdown itself after " +
                           "not receiving any requests for longer than the idle timeout")
                x: styler.themeHorizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }
        }

        ///////////////////////////////////////

        SectionHeaderPL {
            text: qsTr("Profiles")
            visible: settings.profilesUsed
        }

        LabelPL {
            color: styler.themeHighlightColor
            text: qsTr("Active backends are set by the profile. " +
                       "If you wish to change the backend selection, please set the corresponding profile " +
                       "or set profile to <i>Custom</i>.")
            visible: settings.profilesUsed
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        SectionHeaderPL {
            text: qsTr("Rendering")
        }

        LabelPL {
            color: styler.themeHighlightColor
            font.pixelSize: styler.themeFontSizeSmall
            text: qsTr("This server allows you to select between two backends to draw the maps: <i>libosmscout</i> and <i>mapnik</i>.")
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        ElementSwitch {
            id: eMapnik
            activeState: dialog.backendSelectionPossible
            key: settingsMapnikPrefix + "use_mapnik"
            mainLabel: qsTr("Use Mapnik for rendering maps")
            secondaryLabel: qsTr("When selected, Mapnik will be used to render maps. " +
                                 "Note that it requires additional databases for World coastlines and countries.<br>")
        }

        SectionHeaderPL {
            text: qsTr("Geocoder")
        }

        LabelPL {
            color: styler.themeHighlightColor
            font.pixelSize: styler.themeFontSizeSmall
            text: qsTr("Geocoder is responsible for resolving search requests. " +
                       "For that, it parses the search string and finds the corresponding objects on a map. " +
                       "This server allows you to select between two geocoder backends: <i>geocoder-nlp</i> and " +
                       "<i>libosmscout</i>. ")
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        ElementSwitch {
            id: eGeocoderNLP
            activeState: dialog.backendSelectionPossible
            key: settingsGeomasterPrefix + "use_geocoder_nlp"
            mainLabel: qsTr("Use geocoder-nlp with libpostal as a geocoder")
            secondaryLabel: qsTr("When selected, a libpostal-based geocoder will be used to resolve all search requests. " +
                                 "Note that it requires additional databases for language, user input parsing, and geocoding.<br>" +
                                 "NB! If you select <i>geocoder-nlp</i>, please specify languages that should be used for " +
                                 "address parsing in the backend settings below. Otherwise, the server could use large amounts of RAM.")
        }

        SectionHeaderPL {
            text: qsTr("Routing Engine")
        }

        LabelPL {
            color: styler.themeHighlightColor
            font.pixelSize: styler.themeFontSizeSmall
            text: qsTr("Routing engine is responsible for calculating routes between origin and destination. " +
                       "This server allows you to select between two routing engines: <i>Valhalla</i> and " +
                       "<i>libosmscout</i>. ")
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        ElementSwitch {
            id: eValhalla
            activeState: dialog.backendSelectionPossible
            key: settingsValhallaPrefix + "use_valhalla"
            mainLabel: qsTr("Use Valhalla as routing engine")
            secondaryLabel: qsTr("When selected, Valhalla will be used to calculate the routing instructions.")
        }

        SectionHeaderPL {
            text: qsTr("Backend settings")
        }

        Column {
            width: parent.width
            spacing: styler.themePaddingMedium

            ButtonPL {
                anchors.horizontalCenter: parent.horizontalCenter
                preferredWidth: styler.themeButtonWidthLarge
                text: qsTr("Mapnik")
                onClicked: app.push(Qt.resolvedUrl("MapnikPage.qml"))
            }

            LabelPL {
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Map rendering settings for <i>mapnik</i> backend")
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
                preferredWidth: styler.themeButtonWidthLarge
                text: qsTr("Geocoder-NLP")
                onClicked: app.push(Qt.resolvedUrl("GeocoderPage.qml"))
            }

            LabelPL {
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Selection of languages used for address parsing and other settings of <i>geocoder-nlp</i> backend")
                x: styler.themeHorizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }
        }

        Column {
            width: parent.width
            spacing: styler.themePaddingMedium

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

            LabelPL {
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("Routing engine settings for <i>Valhalla</i> backend")
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
                preferredWidth: styler.themeButtonWidthLarge
                text: qsTr("OSM Scout library")
                onClicked: app.push(Qt.resolvedUrl("OSMScoutPage.qml"))
            }

            LabelPL {
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("OSM Scout library (<i>libosmscout</i>) settings. " +
                           "The library can be used for drawing maps, search, and routing.")
                x: styler.themeHorizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
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
    }

    Component.onCompleted: {
        checkState()
    }

    Connections {
        target: settings
        onProfilesUsedChanged: checkState()
    }

    onAccepted: {
        eLogInfo.apply()
        eRollSize.apply()
        eLogSession.apply()

        /// preferred languages are done by combo box, have to apply manually
        settings.setValue(settingsGeneralPrefix + "language", preferredLanguageSelection.currentIndex)

        eMapsRoot.apply()
        eGeocoderNLP.apply()
        eMapnik.apply()
        eValhalla.apply()

        /// units are done by combo box, have to apply manually
        /// units are changed the last
        settings.setValue(settingsGeneralPrefix + "units", unitsBox.currentIndex)

        systemd_service.enabled = systemdEnable.checked
        idleTimeout.apply()
    }

    function checkState() {
        dialog.backendSelectionPossible = !settings.profilesUsed
    }
}
