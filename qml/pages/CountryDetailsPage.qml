import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property var countryId: ""
    property int nFeatures: 0
    property var features: []
    property bool activeState: false

    allowedOrientations : Orientation.All

    function checkSubs()
    {
        if (manager.isCountryRequested(countryId))
        {
            subscribe.enabled = false
            unsubscribe.enabled = (true && page.activeState)
            subscribed.text = qsTr("Subscribed")
            available.visible = true

            if (manager.isCountryAvailable(countryId))
                available.text = qsTr("All datasets covering selected features are available")
            else
                available.text = qsTr("Some datasets are missing, you would have to start downloads to get them")
        }
        else
        {
            subscribe.enabled = (true && page.activeState)
            unsubscribe.enabled = false
            subscribed.text = qsTr("No subscription")

            available.text = ""
            available.visible = false
        }
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
                //height: pageHead.height + fullName.height + Theme.paddingSmall + Theme.paddingLarge
                width: parent.width
                //spacing: Theme.paddingSmall

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
                text: qsTr("Availibility on device")
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
                            text: features[index].name
                            width: parent.width*split_prop - Theme.paddingMedium/2
                            font.pixelSize: Theme.fontSizeExtraSmall
                            color: Theme.secondaryHighlightColor
                            wrapMode: Text.WordWrap
                        }

                        Label {
                            id: feature_size
                            width: parent.width*(1-split_prop) - Theme.paddingMedium/2
                            font.pixelSize: Theme.fontSizeExtraSmall
                            horizontalAlignment: Text.AlignRight
                            color: Theme.secondaryHighlightColor
                            wrapMode: Text.WordWrap
                            Component.onCompleted: {
                                var txt = features[index].size + " " + qsTr("MB")
                                if (!features[index].enabled)
                                    txt = txt + " [" + qsTr("disabled") + "]"
                                feature_size.text = txt
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
        var c = JSON.parse(manager.getCountryDetails(countryId))
        pageHead.title = c.name
        page.activeState = !manager.downloading
        fullName.text = c.name_full
        size.text = qsTr("%1 MB").arg(c.size)
        size_full.text = qsTr("%1 MB").arg(c.size_total)

        features = c.features
        nFeatures = c.features.length

        checkSubs()
    }

    Connections {
        target: manager
        onDownloadingChanged: {
            page.activeState = !manager.downloading
            checkSubs()
        }
    }

}
