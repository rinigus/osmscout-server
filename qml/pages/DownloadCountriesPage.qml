import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property var countries: []
    property string title: qsTr("Select country or territory")
    property string path: ""
    property int nCountries: 1

    allowedOrientations : Orientation.All

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaListView {
        anchors.fill: parent

        header: Column {
            height: head.height + fullpath.height + Theme.paddingSmall + Theme.paddingLarge
            spacing: Theme.paddingSmall

            PageHeader {
                id: head
                title: page.title
                wrapMode: Text.WordWrap
                x: Theme.horizontalPageMargin
                width: page.width-2*x
            }

            Label {
                id: fullpath
                text: page.path
                x: Theme.horizontalPageMargin
                width: page.width-2*x
                horizontalAlignment: Text.AlignRight
                truncationMode: TruncationMode.Elide
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.highlightColor
            }
        }

        model: nCountries

        delegate: ListItem {
            id: listItem
            contentHeight: clist.height + Theme.paddingLarge
            width: parent.width

            Column {
                id: clist
                width: parent.width
                spacing: Theme.paddingMedium
                anchors.margins: Theme.horizontalPageMargin

                Label {
                    id: label
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x
                    wrapMode: Text.WordWrap
                    text: countries[index].name
                    color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                }

                Label {
                    text: qsTr("Size: %1 MB").arg( countries[index].size )
                    x: Theme.horizontalPageMargin * 2
                    width: parent.width-3*x
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: listItem.highlighted ? Theme.highlightColor : Theme.secondaryColor
                }
            }
        }
    }

    VerticalScrollDecorator {}

    Component.onCompleted: {
        nCountries = countries.length
    }
}
