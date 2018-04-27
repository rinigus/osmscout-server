import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."

Dialog {

    id: page
    dialogue: false

    property var countries: []
    property string path: ""
    property int nCountries: 1
    property bool root: false
    property bool fullpath_visible: true

    // To enable PullDownMenu, place our content in a SilicaFlickable
    ListView {
        anchors.fill: page

        header: Column {
            x: Theme.horizontalPageMargin
            width: page.width-2*x

            Label {
                id: head
                text: page.title
                wrapMode: Text.WordWrap
                width: parent.width
                horizontalAlignment: Text.AlignRight
                font.bold: true
                font.pointSize: Theme.fontSizeLarge
            }

            Label {
                id: fullpath
                text: page.path
                wrapMode: Text.WordWrap
                width: parent.width
                horizontalAlignment: Text.AlignRight
                font.pixelSize: Theme.fontSizeTiny
                visible: page.fullpath_visible
            }
        }

        model: nCountries

        delegate: ElementCountryListItem {
            country: countries.children[index]
        }
    }

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
