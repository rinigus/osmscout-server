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

            ComboBox {
                id: mapSelection
                label: qsTr("Map")

                property int ncountries: 0
                property var countries: []

                function updateData()
                {
                    var ret = JSON.parse(manager.getAvailableCountries())
                    mapSelection.countries = ret.countries
                    mapSelection.ncountries = mapSelection.countries.length
                    mapSelection.currentIndex = ret.current

                    if (mapSelection.ncountries < 1)
                        mapSelection.visible = false
                    else
                        mapSelection.visible = true
                }

                menu: ContextMenu {
                    Repeater {
                        model: mapSelection.ncountries
                        delegate: MenuItem {
                            text: index < mapSelection.ncountries ? mapSelection.countries[index].name : ""
                            onClicked: settings.setValue(settingsMapManagerPrefix + "map_selected", mapSelection.countries[index].id)
                        }
                    }
                }

                Component.onCompleted: updateData()

                Connections {
                    target: manager
                    onAvailibilityChanged: mapSelection.updateData()
                }
            }


            SectionHeader {
                text: qsTr("Status")
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
