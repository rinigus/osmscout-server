import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: page

    property var foundUpdates: []
    property int nUpdates: 0

    canAccept: true

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            anchors.left: parent.left
            anchors.right: parent.right

            spacing: Theme.paddingLarge

            DialogHeader {
                title: qsTr("Updates")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
                text: qsTr("Do you want to start the update?")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
                text: qsTr("Please note that as soon as you start the updates, the maps will not be available until the update is finished")
                font.pixelSize: Theme.fontSizeSmall
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
                text: qsTr("List of countries and features with the found updates")
            }

            Column {
                width: parent.width
                spacing: Theme.paddingMedium
                Repeater {
                    width: parent.width
                    model: nUpdates
                    delegate: ElementCountryListItem {
                        country: foundUpdates[index]
                        active: false
                    }
                }
            }

            VerticalScrollDecorator {}
        }
    }

    Component.onCompleted: {
        nUpdates = foundUpdates.length
        console.log(foundUpdates)
        page.canAccept = (!manager.downloading)
    }

    Connections {
        target: manager
        onDownloadingChanged: page.canAccept = !manager.downloading
    }

    onAccepted: manager.getUpdates()
}
