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
import "."
import "platform"

PagePL {
    id: page

    property var countryId: ""
    property int nFeatures: 0
    property var features: []
    property bool activeState: false

    signal dataChanged

    Column {
        id: column
        spacing: styler.themePaddingLarge
        width: parent.width

        LabelPL {
            id: fullName
            color: styler.themeHighlightColor
            font.pixelSize: styler.themeFontSizeExtraSmall
            horizontalAlignment: Text.AlignRight
            text: ""
            truncMode: truncModes.elide
            x: styler.themeHorizontalPageMargin
            width: page.width-2*x
        }

        SectionHeaderPL {
            text: qsTr("Availability on device")
        }

        ListItemLabel {
            id: subscribed
        }

        ListItemLabel {
            id: available
        }

        SectionHeaderPL {
            text: qsTr("Size")
        }

        Row {
            spacing: styler.themePaddingLarge
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x

            LabelPL {
                text: qsTr("Selected datasets")
                color: styler.themeHighlightColor
                width: parent.width*2/3 - styler.themePaddingLarge/2
                wrapMode: Text.WordWrap
            }

            LabelPL {
                id: size
                color: styler.themeHighlightColor
                horizontalAlignment: Text.AlignRight
                width: parent.width/3 - styler.themePaddingLarge/2
            }
        }

        Row {
            spacing: styler.themePaddingLarge
            x: styler.themeHorizontalPageMargin
            width: parent.width-2*x

            LabelPL {
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeSmall
                text: qsTr("All datasets")
                width: parent.width*2/3 - styler.themePaddingLarge/2
                wrapMode: Text.WordWrap
            }

            LabelPL {
                id: size_full
                color: styler.themeHighlightColor
                horizontalAlignment: Text.AlignRight
                font.pixelSize: styler.themeFontSizeSmall
                width: parent.width/3 - styler.themePaddingLarge/2
            }
        }

        Column {
            width: parent.width
            spacing: styler.themePaddingMedium
            Repeater {
                width: parent.width
                model: nFeatures
                delegate: Row {
                    spacing: styler.themePaddingMedium
                    x: styler.themeHorizontalPageMargin
                    width: parent.width-2*x

                    property double split_prop: 0.6

                    LabelPL {
                        id: feature_name
                        color: styler.themeSecondaryHighlightColor
                        font.pixelSize: styler.themeFontSizeExtraSmall
                        width: parent.width*split_prop - styler.themePaddingMedium/2
                        wrapMode: Text.WordWrap

                        Connections {
                            target: page
                            onDataChanged: feature_name.updateData()
                        }

                        Component.onCompleted: updateData()

                        function updateData() {
                            var txt = features[index].name + "<br>" + features[index].date
                            if (features[index].compatible == 0)
                                txt = txt + "<br><b>" + qsTr("incompatible version") + "</b>"
                            text = txt
                        }
                    }

                    LabelPL {
                        id: feature_size
                        color: styler.themeSecondaryHighlightColor
                        horizontalAlignment: Text.AlignRight
                        font.pixelSize: styler.themeFontSizeExtraSmall
                        width: parent.width*(1-split_prop) - styler.themePaddingMedium/2
                        wrapMode: Text.WordWrap

                        Connections {
                            target: page
                            onDataChanged: feature_name.updateData()
                        }

                        Component.onCompleted: updateData()

                        function updateData() {
                            var txt = features[index].size + " " + qsTr("MB")
                            if (!features[index].enabled)
                                txt = txt + "<br>" + qsTr("disabled")
                            feature_size.text = txt
                        }
                    }
                }
            }
        }

        SectionHeaderPL {
            text: qsTr("Subscription")
        }

        ButtonPL {
            id: subscribe
            anchors.horizontalCenter: parent.horizontalCenter
            preferredWidth: styler.themeButtonWidthLarge
            text: qsTr("Subscribe")
            onClicked: {
                manager.addCountry(countryId)
                checkSubs()
            }
        }

        ButtonPL {
            id: unsubscribe
            anchors.horizontalCenter: parent.horizontalCenter
            preferredWidth: styler.themeButtonWidthLarge
            text: qsTr("Unsubscribe")
            onClicked: {
                manager.rmCountry(countryId)
                checkSubs()
            }
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

    Component.onCompleted: {
        fillData()
        checkSubs()
    }

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
}
