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
import osmscout.theme 1.0
import "."

Dialog {

    id: page
    dialogue: false

    property var countryId: ""
    property int nFeatures: 0
    property var features: []
    property bool activeState: false

    signal dataChanged

    function checkSubs() {
        if (manager.isCountryRequested(countryId)) {
            subscribe.enabled = false
            unsubscribe.enabled = (true && page.activeState)
            subscribed.text = qsTr("Subscribed")
            available.visible = true

            if (manager.isCountryAvailable(countryId))
                available.text = qsTr("All datasets covering selected features are available")
            else if (manager.isCountryCompatible(countryId))
                available.text = qsTr("Some datasets are missing. You would have to start downloads to get them.")
            else
                available.text = qsTr("Some or all datasets are incompatible with this version. " +
                                      "You would have to update list of provided maps, the maps, and/or install " +
                                      "the latest release of OSM Scout Server.")
        }
        else {
            subscribe.enabled = (true && page.activeState)
            unsubscribe.enabled = false
            subscribed.text = qsTr("No subscription")

            available.text = ""
            available.visible = false
        }
    }

    function fillData() {
        var c = JSON.parse(manager.getCountryDetails(countryId))
        page.title = c.name
        fullName.text = c.name_full
        if (c.name === c.name_full) fullName.visible = false

        page.activeState = (manager.ready && canBeActive())
        size.text = qsTr("%1 MB").arg(c.size)
        size_full.text = qsTr("%1 MB").arg(c.size_total)

        features = c.features
        nFeatures = c.features.length

        dataChanged()
    }

    function canBeActive() {
        if (countryId === "postal/global" || countryId === "mapnik/global")
            return false;
        return true;
    }

    Column {
        id: column

        x: Theme.horizontalPageMargin
        width: page.width-2*x
        spacing: Theme.paddingLarge

        Column {
            width: parent.width

            Label {
                text: page.title
                wrapMode: Text.WordWrap
                width: column.width
                horizontalAlignment: Text.AlignRight
                font.bold: true
                font.pointSize: Theme.fontSizeLarge
            }

            Label {
                id: fullName
                text: ""
                width: column.width
                horizontalAlignment: Text.AlignRight
                font.pointSize: Theme.fontSizeSmall
            }
        }

        SectionHeader {
            text: qsTr("Availability on device")
        }

        Label {
            id: subscribed
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        Label {
            id: available
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        SectionHeader {
            text: qsTr("Size")
        }

        Row {
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            spacing: Theme.paddingLarge

            Label {
                text: qsTr("Selected datasets")
                width: parent.width*2/3 - Theme.paddingLarge/2
                wrapMode: Text.WordWrap
            }

            Label {
                id: size
                width: parent.width/3 - Theme.paddingLarge/2
                horizontalAlignment: Text.AlignRight
            }
        }

        Row {
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            spacing: Theme.paddingLarge

            Label {
                text: qsTr("All datasets")
                width: parent.width*2/3 - Theme.paddingLarge/2
                wrapMode: Text.WordWrap
                font.pointSize: Theme.fontSizeSmall
            }

            Label {
                id: size_full
                width: parent.width/3 - Theme.paddingLarge/2
                horizontalAlignment: Text.AlignRight
                font.pointSize: Theme.fontSizeSmall
            }
        }

        Column {
            width: parent.width
            spacing: Theme.paddingMedium
            Repeater {
                width: parent.width
                model: nFeatures
                delegate: Row {
                    spacing: Theme.paddingMedium
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x

                    property double split_prop: 0.6

                    Label {
                        id: feature_name
                        width: parent.width*split_prop - Theme.paddingMedium/2
                        font.pointSize: Theme.fontSizeExtraSmall
                        wrapMode: Text.WordWrap

                        function updateData() {
                            var txt = features[index].name + "<br>" + features[index].date
                            if (features[index].compatible == 0)
                                txt = txt + "<br><b>" + qsTr("incompatible version") + "</b>"
                            text = txt
                        }

                        Component.onCompleted: updateData()
                        Connections {
                            target: page
                            onDataChanged: feature_name.updateData()
                        }
                    }

                    Label {
                        id: feature_size
                        width: parent.width*(1-split_prop) - Theme.paddingMedium/2
                        font.pointSize: Theme.fontSizeExtraSmall
                        horizontalAlignment: Text.AlignRight
                        wrapMode: Text.WordWrap

                        function updateData() {
                            var txt = features[index].size + " " + qsTr("MB")
                            if (!features[index].enabled)
                                txt = txt + "<br>" + qsTr("disabled")
                            feature_size.text = txt
                        }

                        Component.onCompleted: updateData()
                        Connections {
                            target: page
                            onDataChanged: feature_name.updateData()
                        }
                    }
                }
            }
        }

        SectionHeader {
            text: qsTr("Subscription")
        }

        Button {
            id: subscribe
            text: qsTr("Subscribe")
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                manager.addCountry(countryId)
                checkSubs()
            }
        }

        Button {
            id: unsubscribe
            text: qsTr("Unsubscribe")
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                manager.rmCountry(countryId)
                checkSubs()
            }
        }
    }

    Component.onCompleted: {
        fillData()
        checkSubs()
    }

    Connections {
        target: manager
        onReadyChanged: {
            page.activeState = (manager.ready && canBeActive())
            checkSubs()
        }

        onSubscriptionChanged: {
            fillData()
            checkSubs()
        }
        onAvailabilityChanged: {
            fillData()
            checkSubs()
        }
    }
}
