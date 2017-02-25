import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property var countries: []
    property string title: ""
    property string path: ""
    property int nCountries: 1

    allowedOrientations : Orientation.All

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaListView {
        anchors.fill: parent

        header: Column {
            width: parent.width
            //height: head.height + fullpath.height + Theme.paddingSmall + Theme.paddingLarge
            spacing: Theme.paddingSmall

            PageHeader {
                id: head
                title: page.title
                wrapMode: Text.WordWrap
                width: parent.width
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
                spacing: Theme.paddingSmall
                anchors.margins: Theme.horizontalPageMargin

                Label {
                    id: label
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x
                    wrapMode: Text.WordWrap
                    text: countries.children[index].name
                    color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                }

                Label {
                    id: prop
                    text: ""
                    x: Theme.horizontalPageMargin * 2
                    width: parent.width-3*x
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: listItem.highlighted ? Theme.highlightColor : Theme.secondaryColor

                    Component.onCompleted: {
                        var c = countries.children[index]
                        if (c.type === "dir")
                            prop.text = qsTr("%1 territories").arg(c.children.length)
                        else
                            prop.text = qsTr("Size: %1 MB").arg( c.size )
                    }
                }
            }

            onClicked: {
                var c = countries.children[index]
                if (c.type === "dir")
                {
                    var newpath = ""
                    if (page.path.length > 0)
                        newpath = page.path + " / " + c.name
                    pageStack.push(Qt.resolvedUrl("DownloadCountriesPage.qml"),
                                   { "countries": c, "path": newpath, "title": c.name } )
                }
                else
                    pageStack.push(Qt.resolvedUrl("CountryDetailsPage.qml"),
                                   { "countryId": c.id } )
            }
        }
    }

    VerticalScrollDecorator {}

    Component.onCompleted: {
        nCountries = countries.children.length

        if (page.title.length < 1)
            page.title = qsTr("Select country or territory")
        if (page.path.length < 1)
            fullpath.visible = false
    }
}
