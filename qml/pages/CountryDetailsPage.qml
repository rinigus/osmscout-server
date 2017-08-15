import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property var countryId: ""
    property int nFeatures: 0
    property var features: []
    property bool activeState: false

    signal dataChanged

    allowedOrientations : Orientation.All

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
        pageHead.title = c.name
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

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: parent.width
            spacing: Theme.paddingLarge
            //anchors.margins: Theme.horizontalPageMargin

            Column {
                width: parent.width

                PageHeader {
                    id: pageHead
                    title: ""
                    wrapMode: Text.WordWrap
                    width: page.width
                }

                Label {
                    id: fullName
                    text: ""
                    x: Theme.horizontalPageMargin
                    width: page.width-2*x
                    horizontalAlignment: Text.AlignRight
                    truncationMode: TruncationMode.Elide
                    font.pixelSize: Theme.fontSizeTiny
                    color: Theme.highlightColor
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
                color: Theme.highlightColor
            }

            Label {
                id: available
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
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
                    color: Theme.highlightColor
                }

                Label {
                    id: size
                    width: parent.width/3 - Theme.paddingLarge/2
                    color: Theme.highlightColor
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
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                }

                Label {
                    id: size_full
                    width: parent.width/3 - Theme.paddingLarge/2
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
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
                            font.pixelSize: Theme.fontSizeExtraSmall
                            color: Theme.secondaryHighlightColor
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
                            font.pixelSize: Theme.fontSizeExtraSmall
                            horizontalAlignment: Text.AlignRight
                            color: Theme.secondaryHighlightColor
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
                preferredWidth: Theme.buttonWidthLarge
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    manager.addCountry(countryId)
                    checkSubs()
                }
            }

            Button {
                id: unsubscribe
                text: qsTr("Unsubscribe")
                preferredWidth: Theme.buttonWidthLarge
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    manager.rmCountry(countryId)
                    checkSubs()
                }
            }
        }

        VerticalScrollDecorator {}
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
