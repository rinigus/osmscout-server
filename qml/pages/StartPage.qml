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

            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
        }

        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: page.width-Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("OSMScout Server")
            }

            SectionHeader {
                text: qsTr("Database")
                font.pixelSize: Theme.fontSizeLarge
            }

            Label {
                id: database
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
                text: ""

                function setText() {
                    text = settings.valueString(settingsOsmPrefix + "map")
                }

                Component.onCompleted: { setText() }
                Connections { target: settings; onOsmScoutSettingsChanged: database.setText() }
            }

//            Label {
//                id: status
//                width: parent.width
//                color: Theme.highlightColor
//                text: ""

//                function setText() {
//                    if (infohub.error) text = qsTr("Status: Error")
//                    else text = qsTr("Status: Running")
//                }

//                Component.onCompleted: { setText() }
//                Connections { target: infohub; onErrorChanged: status.setText() }
//            }

            SectionHeader {
                text: qsTr("Last Events")
                font.pixelSize: Theme.fontSizeMedium
            }

            Label {
                id: log
                width: parent.width
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeTiny
                wrapMode: Text.WordWrap

                text: ""

                function setText() {
                    text = logger.log
                }

                Component.onCompleted: { setText() }
                Connections { target: logger; onLogChanged: log.setText() }
            }
        }

        VerticalScrollDecorator {}
    }
}


