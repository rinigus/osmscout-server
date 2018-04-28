import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."

Dialog {

    id: page
    title: qsTr("Updates")

    contentHeight: column.height + Theme.paddingLarge

    property var foundUpdates: []
    property int nUpdates: 0

    canAccept: manager.ready

    Column {
        id: column

        x: Theme.horizontalPageMargin
        width: page.width-2*x

        spacing: Theme.paddingLarge

        Label {
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
            text: qsTr("Do you want to start the update?")
        }

        Label {
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
            text: qsTr("Please note that as soon as you start the updates, the maps will not be available until the update is finished")
            font.pointSize: Theme.fontSizeSmall
        }

        Label {
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
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

    }

    Component.onCompleted: {
        nUpdates = foundUpdates.length
    }

    function onAccepted() {
        manager.getUpdates()
    }
}
