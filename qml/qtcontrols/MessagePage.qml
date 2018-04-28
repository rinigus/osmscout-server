import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import osmscout.theme 1.0
import "."

Dialog {
    id: page
    dialogue: false

    property string message: ""

    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: ScrollBar.AsNeeded

    Label {
        text: message
        wrapMode: Text.WordWrap
        x: Theme.horizontalPageMargin
        width: page.width - 2*x
        textFormat: Text.RichText
    }
}
