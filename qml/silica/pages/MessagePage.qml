import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property string header: ""
    property string message: ""

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            anchors.margins: Theme.horizontalPageMargin
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: Theme.paddingLarge

            PageHeader {
                title: header
            }

            Label {
                text: message
                wrapMode: Text.WordWrap
                width: column.width
                color: Theme.highlightColor
                textFormat: Text.RichText
            }
        }
    }
}
