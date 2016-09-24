import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
        }

        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge
            anchors.margins: Theme.horizontalPageMargin

            PageHeader {
                title: qsTr("OSMScout Server")
            }

//            SectionHeader {
//                text: qsTr("Statistics")
//                font.pixelSize: Theme.fontSizeLarge
//            }

            SectionHeader {
                text: qsTr("Settings")
                font.pixelSize: Theme.fontSizeLarge
            }

            SectionHeader {
                text: qsTr("Map and style")
                font.pixelSize: Theme.fontSizeMedium
            }

            ElementEntry {
                key: "map"
                mainLabel: qsTr("Folder containing Maps")
                secondaryLabel: qsTr("This folder should contain maps imported by libosmscout Import utility")
            }

            ElementEntry {
                key: "style"
                mainLabel: qsTr("Style sheet")
                secondaryLabel: qsTr("Style sheet used to render the map tiles")
            }

            ElementEntry {
                key: "icons"
                mainLabel: qsTr("Folder with icons")
                secondaryLabel: qsTr("Icons used to mark features on the map")
            }

            SectionHeader {
                text: qsTr("Rendering")
                font.pixelSize: Theme.fontSizeMedium
            }

            ElementEntry {
                key: "fontSize"
                mainLabel: qsTr("Font size")
                validator: DoubleValidator { bottom: 0; decimals: 2; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementSwitch {
                key: "renderSea"
                mainLabel: qsTr("Render sea")
            }

            ElementSwitch {
                key: "drawBackground"
                mainLabel: qsTr("Draw background")
            }
        }

        VerticalScrollDecorator {}
    }
}


