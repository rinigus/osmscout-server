import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {

    id: dialog
    allowedOrientations : Orientation.All

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: parent.width
            spacing: Theme.paddingLarge
            anchors.margins: Theme.horizontalPageMargin

            DialogHeader {
                title: qsTr("Valhalla")
            }

            Label {
                text: qsTr("Valhalla can be used as a routing engine to calculate routes and the routing instructions")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            ElementEntry {
                id: eCache
                key: settingsValhallaPrefix + "cache_in_mb"
                mainLabel: qsTr("Cache for storing tiles")
                secondaryLabel: qsTr("Memory cache that is used to keep Valhalla's tiles in RAM. Memory cache is given here in MB.")
                validator: DoubleValidator { bottom: 16; top: 1024; decimals: 0; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
        }

        VerticalScrollDecorator {}
    }

    onAccepted: {
        eCache.apply()
    }
}
