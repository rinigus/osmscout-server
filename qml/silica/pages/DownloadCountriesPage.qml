import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property var countries: []
    property string title: ""
    property string path: ""
    property int nCountries: 1
    property bool root: false
    property bool fullpath_visible: true

    allowedOrientations : Orientation.All

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaListView {
        anchors.fill: parent

        header: Column {
            width: parent.width

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
                visible: page.fullpath_visible
            }
        }

        model: nCountries

        delegate: ElementCountryListItem {
            country: countries.children[index]
        }
    }

    VerticalScrollDecorator {}

    Component.onCompleted: {
        nCountries = countries.children.length

        if (page.title.length < 1)
        {
            page.title = qsTr("Select country or territory")
            page.root = true
        }

        if (page.path.length < 1)
            page.fullpath_visible = false
    }
}
