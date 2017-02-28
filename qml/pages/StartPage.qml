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
                id: mmMenu
                text: qsTr("Map Manager")
                enabled: manager.storageAvailable
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

            //////////////////////////////////////////////////////////////////////////////////
            //// Welcome messages for new users

            Column {
                id: storageNotAvailable
                width: page.width
                spacing: Theme.paddingLarge

                SectionHeader {
                    text: qsTr("Welcome")
                }

                Label {
                    text: qsTr("<i>OSM Scout Server</i> is expected to be used with the " +
                               "downloaded maps. To manage the maps, the Server requires a use of a separate " +
                               "folder with the files within that folder managed by the Server only. " +
                               "For that, please <b>allocate separate, empty folder</b> that OSM Scout Server could use. " +
                               "This includes deleting all files within that folder when requested by you during cleanup or " +
                               "map updates.<br><br>" +
                               "Please, create a new folder in a file manager or using command line and then select this folder " +
                               "in <i>Settings</i> (pulley menu).")
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                }

                Component.onCompleted: storageNotAvailable.visible = !(manager.storageAvailable)
                Connections {
                    target: manager
                    onStorageAvailableChanged: storageNotAvailable.visible = !(manager.storageAvailable)
                }
            }

            Column {
                id: noSubscriptions
                width: page.width
                spacing: Theme.paddingLarge

                SectionHeader {
                    text: qsTr("Welcome")
                }

                Label {
                    text: qsTr("With the storage folder selected and available, the next step is to get some maps. " +
                               "For that, you can select and download maps using <i>Map Manager</i>  (pulley menu).")
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                }

                function checkVisible() {
                    noSubscriptions.visible = (JSON.parse(manager.getRequestedCountries()).children.length == 0)
                }

                Component.onCompleted: noSubscriptions.checkVisible()
                Connections {
                    target: manager
                    onSubscriptionChanged: noSubscriptions.checkVisible()
                }
            }

            Column {
                id: noMapsAvailable
                width: page.width
                spacing: Theme.paddingLarge

                SectionHeader {
                    text: qsTr("Welcome")
                }

                Label {
                    text: qsTr("There are no maps available yet. After subscribing them, you have to start downloads. " +
                               "Downloads can be started using <i>Map Manager</i>  (pulley menu).")
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                }

                function checkVisible() {
                    var subs = JSON.parse(manager.getRequestedCountries())
                    var avail = JSON.parse(manager.getAvailableCountries())
                    if (subs.children.length != 0 &&
                            avail.countries.length == 0 &&
                            !manager.downloading)
                        noMapsAvailable.visible = true
                    else
                        noMapsAvailable.visible = false
                }

                Component.onCompleted: noMapsAvailable.checkVisible()
                Connections {
                    target: manager
                    onSubscriptionChanged: noMapsAvailable.checkVisible()
                    onDownloadingChanged: noMapsAvailable.checkVisible()
                    onAvailibilityChanged: noMapsAvailable.checkVisible()
                }
            }

            //// Welcome messages: done
            //////////////////////////////////////////////////////////////////////////////////

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

    Connections {
        target: manager
        onStorageAvailableChanged: {
            if (!available)
            {
                console.log("Busy: ")
                console.log(pageStack.busy)
                pageStack.completeAnimation()
                pageStack.push(Qt.resolvedUrl("SetupChooseStorageDir.qml"))
            }
        }
    }

}
