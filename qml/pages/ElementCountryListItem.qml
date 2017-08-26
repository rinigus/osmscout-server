import QtQuick 2.0
import Sailfish.Silica 1.0

ListItem {
    id: listItem
    contentHeight: clist.height + Theme.paddingLarge
    width: parent.width

    property var country
    property string iconSource: ""
    property bool active: true

    function updateData() {
        var c = country

        label.text = c.name
        if (c.type === "dir") {
            prop.text = qsTr("%1 territories").arg(c.children.length)
            iconSource = "image://theme/icon-m-file-folder"
        }
        else {
            var info = ""
            if (manager.isCountryAvailable(c.id) && c.id==="postal/global") {
                iconSource = "image://theme/icon-m-chat"
                info = qsTr("Available") + "; "
            }
            else if (manager.isCountryAvailable(c.id) && c.id==="mapnik/global") {
                iconSource = "image://theme/icon-m-image"
                info = qsTr("Available") + "; "
            }
            else if (manager.isCountryAvailable(c.id)) {
                iconSource = "image://theme/icon-m-location"
                info = qsTr("Available") + "; "
            }
            else if (manager.isCountryRequested(c.id) &&
                     !manager.isCountryCompatible(c.id)) {
                iconSource = "image://theme/icon-m-sync"
                info = qsTr("Incompatible version") + "; "
            }
            else if (manager.isCountryRequested(c.id)) {
                iconSource = "image://theme/icon-m-cloud-download"
                info = qsTr("Subscribed") + "; "
            }
            else
                iconSource = "image://theme/icon-m-region"

            prop.text = info + qsTr("Size: %1 MB").arg( c.size )
        }
    }

    Row {
        spacing: Theme.paddingLarge
        x: Theme.horizontalPageMargin
        width: parent.width-2*x
        anchors.verticalCenter: parent.verticalCenter

        Rectangle {
            width: Theme.iconSizeMedium
            height: Theme.iconSizeMedium
            color: "transparent"

            Image {
                anchors.centerIn: parent
                source: {
                    return iconSource + (highlighted || !active ? "?" + Theme.highlightColor : "")
                }
            }
        }

        Column {
            id: clist
            width: parent.width - Theme.iconSizeMedium - Theme.paddingLarge
            spacing: Theme.paddingSmall

            Label {
                id: label
                width: parent.width
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeMedium
                color: listItem.highlighted || !listItem.active ? Theme.highlightColor : Theme.primaryColor
            }

            Label {
                id: prop
                width: parent.width
                horizontalAlignment: Text.AlignRight
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: listItem.highlighted || !listItem.active ? Theme.highlightColor : Theme.secondaryColor
            }
        }
    }

    Component.onCompleted: updateData()
    onCountryChanged: updateData()

    Connections {
        target: manager
        onSubscriptionChanged: listItem.updateData()
        onAvailabilityChanged: listItem.updateData()
    }

    onClicked: {
        if (active) {
            var c = country
            if (c.type === "dir")
            {
                var newpath = ""
                if (!page.root)
                {
                    if (page.path.length > 0)
                        newpath = page.path + " / " + c.name
                    else
                        newpath = page.title + " / " + c.name
                }
                pageStack.push(Qt.resolvedUrl("DownloadCountriesPage.qml"),
                               { "countries": c, "path": newpath, "title": c.name } )
            }
            else
                pageStack.push(Qt.resolvedUrl("CountryDetailsPage.qml"),
                               { "countryId": c.id } )
        }
    }
}
