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
                text: qsTr("Map Manager")
                onClicked: pageStack.push(Qt.resolvedUrl("MapManagerPage.qml"))
            }

            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
        }

        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("OSM Scout Server")
            }

            SectionHeader {
                text: qsTr("Map")
                font.pixelSize: Theme.fontSizeLarge
            }

            ListItem {
                id: listItem
                //contentHeight: Theme.itemSizeSmall

                Column {
                    width: parent.width
                    height: database.height + database_full.height + Theme.paddingSmall
                    spacing: Theme.paddingSmall

                    Label {
                        id: database
                        x: Theme.horizontalPageMargin
                        width: parent.width-2*Theme.horizontalPageMargin
                        wrapMode: Text.WordWrap
                        color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                        text: ""

                        function setText() {
                            text = settings.valueString(settingsOsmPrefix + "map").split("/").pop()
                        }

                        Component.onCompleted: { setText() }
                        Connections { target: settings; onOsmScoutSettingsChanged: database.setText() }
                    }

                    Label {
                        id: database_full
                        x: Theme.horizontalPageMargin
                        width: parent.width-2*Theme.horizontalPageMargin
                        wrapMode: Text.WordWrap
                        color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                        text: ""
                        font.pixelSize: Theme.fontSizeTiny
                        truncationMode: TruncationMode.Fade

                        function setText() {
                            text = settings.valueString(settingsOsmPrefix + "map")
                        }

                        Component.onCompleted: { setText() }
                        Connections { target: settings; onOsmScoutSettingsChanged: database_full.setText() }
                    }
                }

                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }

            SectionHeader {
                text: qsTr("Status")
                font.pixelSize: Theme.fontSizeMedium
            }

            Label {
                id: queueLength
                x: Theme.horizontalPageMargin
                width: parent.width-2*Theme.horizontalPageMargin
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall

                text: ""

                function setText(q) {
                    if (q > 0) text = qsTr("Jobs in a queue") + ": " + q
                    else text = qsTr("Idle")
                }

                Connections {
                    target: infohub;
                    onQueueChanged: {
                        queueLength.setText(queue)
                    }
                }

                Component.onCompleted: queueLength.setText(infohub.queue)
            }

            ElementDownloads {
            }

            SectionHeader {
                text: qsTr("Events")
                font.pixelSize: Theme.fontSizeMedium
            }

            Label {
                id: log
                x: Theme.horizontalPageMargin
                width: parent.width-2*Theme.horizontalPageMargin
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.Wrap

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
