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
                title: qsTr("Settings")
            }

            SectionHeader {
                text: qsTr("Map and style")
                font.pixelSize: Theme.fontSizeMedium
            }

            ElementEntry {
                id: eMap
                key: "map"
                mainLabel: qsTr("Folder containing Maps")
                secondaryLabel: qsTr("This folder should contain maps imported by libosmscout Import utility")
            }

            ElementEntry {
                id: eStyle
                key: "style"
                mainLabel: qsTr("Style sheet")
                secondaryLabel: qsTr("Style sheet used to render the map tiles")
            }

            ElementEntry {
                id: eIcons
                key: "icons"
                mainLabel: qsTr("Folder with icons")
                secondaryLabel: qsTr("Icons used to mark features on the map")
            }

            SectionHeader {
                text: qsTr("Rendering")
                font.pixelSize: Theme.fontSizeMedium
            }

            ElementEntry {
                id: eFontSize
                key: "fontSize"
                mainLabel: qsTr("Font size")
                validator: DoubleValidator { bottom: 0; decimals: 2; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementSwitch {
                id: eRenderSea
                key: "renderSea"
                mainLabel: qsTr("Render sea")
            }

            ElementSwitch {
                id: eDrawBackground
                key: "drawBackground"
                mainLabel: qsTr("Draw background")
            }

            SectionHeader {
                text: qsTr("Miscellaneous")
                font.pixelSize: Theme.fontSizeMedium
            }

            ElementSwitch {
                id: eLogInfo
                key: "logInfo"
                mainLabel: qsTr("Log info messages")
                secondaryLabel: qsTr( "When disabled, INFO messages will not be logged in Events log. " +
                                      "Exception is loading of the database which is shown always" )
            }

            ElementEntry {
                id: eRollSize
                key: "rollingLoggerSize"
                mainLabel: qsTr("Events log size")
                secondaryLabel: qsTr("Number of events shown in the main page")
                validator: IntValidator { bottom: 3; top: 25; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
        }

        VerticalScrollDecorator {}
    }

    onAccepted: {
        eLogInfo.apply()
        eRollSize.apply()
        eMap.apply()
        eStyle.apply()
        eIcons.apply()
        eFontSize.apply()
        eRenderSea.apply()
        eDrawBackground.apply()
    }
}


