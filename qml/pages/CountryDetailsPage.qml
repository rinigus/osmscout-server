import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property var countryId: ""
    property int nFeatures: 0
    property var features: []

    allowedOrientations : Orientation.All

    function checkSubs()
    {
        if (manager.isCountryRequested(countryId))
        {
            subscribe.enabled = false
            unsubscribe.enabled = true
            subscribed.text = qsTr("Subscribed")
            available.visible = true

            if (manager.isCountryAvailable(countryId))
                available.text = qsTr("All datasets covering selected features are available")
            else
                available.text = qsTr("Some datasets are missing, you would have to start downloads to get them")
        }
        else
        {
            subscribe.enabled = true
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
                height: pageHead.height + fullName.height + Theme.paddingSmall + Theme.paddingLarge
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

            ListView {
                width: parent.width
                height: nFeatures * (Theme.fontSizeExtraSmall +  Theme.paddingMedium)
                model: nFeatures
                delegate: Row {
                    spacing: Theme.paddingLarge
                    height: Theme.fontSizeExtraSmall +  Theme.paddingMedium
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x

                    Label {
                        text: features[index].name
                        width: parent.width*2/3 - Theme.paddingLarge/2
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: Theme.secondaryHighlightColor
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: feature_size
                        width: parent.width*1/3 - Theme.paddingLarge/2
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
//            ListView {
//                width: parent.width
//                height: nFeatures * (2*(Theme.fontSizeExtraSmall +  Theme.paddingSmall) +  Theme.paddingMedium)
//                model: nFeatures
//                delegate: Column {
//                    spacing: Theme.paddingSmall
//                    height: 2*(Theme.fontSizeExtraSmall +  Theme.paddingSmall) +  Theme.paddingMedium
//                    width: parent.width

//                    Label {
//                        x: Theme.horizontalPageMargin
//                        width: parent.width-2*x
//                        font.pixelSize: Theme.fontSizeExtraSmall
//                        color: Theme.secondaryHighlightColor
//                        text: features[index].name
//                    }
//                    Label {
//                        id: feature_size
//                        x: Theme.horizontalPageMargin
//                        width: parent.width-2*x
//                        font.pixelSize: Theme.fontSizeExtraSmall
//                        horizontalAlignment: Text.AlignRight
//                        color: Theme.secondaryHighlightColor
//                        Component.onCompleted: {
//                            var txt = features[index].size + " " + qsTr("MB")
//                            if (!features[index].enabled)
//                                txt = txt + " [" + qsTr("disabled") + "]"
//                            feature_size.text = txt
//                        }
//                    }
//                }
//            }

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
        fullName.text = c.name_full
        size.text = qsTr("%1 MB").arg(c.size)
        size_full.text = qsTr("%1 MB").arg(c.size_total)

        features = c.features
        nFeatures = c.features.length

        checkSubs()
    }
}
