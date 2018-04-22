import QtQuick 2.9
import QtQuick.Controls 2.2
import osmscout.theme 1.0
import "."

ScrollView {
    id: rootPage
    property string title: qsTr("OSM Scout Server")
    width: parent.width
    height: parent.height

    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: ScrollBar.AsNeeded

    Column {
        id: column

        x: Theme.horizontalPageMargin
        width: rootPage.width - 2*Theme.horizontalPageMargin
        spacing: Theme.paddingLarge

        //////////////////////////////////////////////////////////////////////////////////
        //// Check for installed modules
        Column {
            id: modulesNotAvailable
            width: rootPage.width
            spacing: Theme.paddingLarge

            property string dname: ""

            SectionHeader {
                text: qsTr("Missing modules")
            }

            Label {
                id: noModuleText
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }

            function checkModules() {
                modulesNotAvailable.visible = ( !modules.fonts || !modules.valhallaRoute )
                noModuleText.text =
                        qsTr("<i>OSM Scout Server</i> uses several modules that have to be installed separately " +
                             "for full functionality.<br><br>Your device is missing the following module(s) " +
                             "that are required by the current profile:<ul>")

                if (!modules.fonts)
                    noModuleText.text += qsTr("<li>OSM Scout Server Module: Fonts</li>")
                if (!modules.valhallaRoute)
                    noModuleText.text += qsTr("<li>OSM Scout Server Module: Route</li>")

                noModuleText.text += qsTr("</ul><br>Please install missing modules via Harbour or OpenRepos. " +
                                          "After installation of the module(s), please restart OSM Scout Server.")
            }

            Component.onCompleted: checkModules()

            Connections {
                target: modules
                onModulesChanged: modulesNotAvailable.checkModules()
            }

            Rectangle { // just extra space to highlight the message
                height: Theme.paddingLarge*3
                width: parent.width
                color: "transparent"
            }
        }


        //////////////////////////////////////////////////////////////////////////////////
        //// Welcome messages for new users

        Column {
            id: storageNotAvailable
            width: rootPage.width
            spacing: Theme.paddingLarge

            property string dname: ""

            SectionHeader {
                text: qsTr("Welcome")
            }

            Label {
                id: notAvailableText
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }

            Connections {
                target: manager
                onStorageAvailableChanged: storageNotAvailable.visible = !(manager.storageAvailable)
            }

            Button {
                text: qsTr("Create default directory")
                //preferredWidth: Theme.buttonWidthLarge
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    if (manager.createDirectory(storageNotAvailable.dname)) {
                        settings.setValue(settingsMapManagerPrefix + "root", storageNotAvailable.dname)
                        mainFlickable.scrollToTop()
                    }
                }
            }

            Label {
                id: notAvailableDirCreation
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }

            Component.onCompleted: {
                storageNotAvailable.visible = !(manager.storageAvailable)
                dname = manager.defaultStorageDirectory()
                notAvailableDirCreation.text = qsTr("Creates directory<br>%1<br>and configures it for storing maps").arg(dname)
                notAvailableText.text =
                        qsTr("<i>OSM Scout Server</i> is expected to be used with the " +
                             "downloaded maps. To manage the maps, the Server requires a separate " +
                             "folder. The files within that folder should be managed by the Server only. " +
                             "This includes deleting all files within that folder when requested by you during cleanup or " +
                             "map updates.<br><br>" +
                             "Please <b>allocate separate, empty folder</b> for OSM Scout Server. " +
                             "For that, create a new folder in a file manager or using command line and then select this folder " +
                             "in <i>Settings</i> (pulley menu).<br><br>" +
                             "Alternatively, the directory can be created and setup automatically at <br>" +
                             "%1<br>by pressing a button below").arg(dname)

            }
        }

        Column {
            id: noSubscriptions
            width: rootPage.width
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
            }

            Rectangle { // just extra space to highlight the message
                height: Theme.paddingLarge*3
                width: parent.width
                color: "transparent"
            }

            function checkVisible() {
                noSubscriptions.visible = (manager.storageAvailable &&
                                           JSON.parse(manager.getRequestedCountries()).children.length == 0)
            }

            Component.onCompleted: noSubscriptions.checkVisible()
            Connections {
                target: manager
                onSubscriptionChanged: noSubscriptions.checkVisible()
                onStorageAvailableChanged: noSubscriptions.checkVisible()
                onAvailabilityChanged: noSubscriptions.checkVisible()
            }
        }

        Column {
            id: noMapsAvailable
            width: rootPage.width
            spacing: Theme.paddingLarge

            SectionHeader {
                text: qsTr("Welcome")
            }

            Label {
                text: qsTr("There are no maps available yet. After subscribing them, you have to start downloads. " +
                           "Downloads can be started using <i>Map Manager</i>.")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }

            Rectangle { // just extra space to highlight the message
                height: Theme.paddingLarge*3
                width: parent.width
                color: "transparent"
            }

            function checkVisible() {
                var subs = JSON.parse(manager.getRequestedCountries())
                var avail = JSON.parse(manager.getAvailableCountries())
                if (subs.children.length != 0 &&
                        avail.countries.length == 0 &&
                        manager.ready)
                    noMapsAvailable.visible = true
                else
                    noMapsAvailable.visible = false
            }

            Component.onCompleted: noMapsAvailable.checkVisible()
            Connections {
                target: manager
                onSubscriptionChanged: noMapsAvailable.checkVisible()
                onDownloadingChanged: noMapsAvailable.checkVisible()
                onReadyChanged: noMapsAvailable.checkVisible()
                onAvailabilityChanged: noMapsAvailable.checkVisible()
            }
        }

        //// Welcome messages: done
        //////////////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////////////
        /// Warnings

        /// Warn if no language has been specified
        Column {
            id: noGeocoderLangSpecified
            width: rootPage.width
            spacing: Theme.paddingLarge

            // that will be false when maps storage is not available
            visible: geocoder.warnLargeRamLangNotSpecified

            SectionHeader {
                text: qsTr("Warning")
            }

            Label {
                text: qsTr("You have not specified languages used for parsing addresses by Geocoder-NLP. " +
                           "As a result, all known languages are used and " +
                           "you could experience very large RAM consumption. Such large RAM usage could " +
                           "lead to the OSM Scout Server being killed by the kernel. <br><br>" +
                           "To specify languages used for address parsing, either select languages below or " +
                           "go to <i>Settings</i> (pulley menu) and " +
                           "select languages as a part of <i>Geocoder-NLP</i> settings.")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
            }

            Rectangle { // just extra space to highlight the message
                height: Theme.paddingLarge*3
                width: parent.width
                color: "transparent"
            }
        }

        /// Warnings: done
        //////////////////////////////////////////////////////////////////////////////////

        Row {
            id: mapSelectorContainer
            spacing: Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x

            Text {
                id: comboText
                text: qsTr("Map")
                anchors.verticalCenter: mapSelection.verticalCenter
            }

            ComboBox {
                id: mapSelection
                width: mapSelectorContainer.width - comboText.width - mapSelectorContainer.spacing*2

                property int ncountries: 0
                property var countries: []

                function updateData()
                {
                    var ret = JSON.parse(manager.getAvailableCountries())
                    mapSelection.countries = ret.countries
                    mapSelection.ncountries = mapSelection.countries.length
                    var names = []
                    for (var i=0; i < mapSelection.ncountries; ++i)
                        names.push(mapSelection.countries[i].name)
                    mapSelection.model = names
                    mapSelection.currentIndex = ret.current

                    if (mapSelection.ncountries < 1)
                        mapSelection.visible = false
                    else
                        mapSelection.visible = true
                }

                onCurrentIndexChanged: settings.setValue(settingsMapManagerPrefix + "map_selected",
                                                         mapSelection.countries[currentIndex].id)

                Component.onCompleted: updateData()

                Connections {
                    target: manager
                    onAvailabilityChanged: mapSelection.updateData()
                }
            }
        }

        ElementLanguageSelector {
            id: eGeoLanguages
            key: settingsGeomasterPrefix + "languages"
            autoApply: true
            visible: manager.storageAvailable
        }

        SectionHeader {
            text: qsTr("Status")
            visible: manager.storageAvailable
        }

        Label {
            id: queueLength
            x: Theme.horizontalPageMargin
            width: parent.width-2*Theme.horizontalPageMargin
            visible: manager.storageAvailable

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
            visible: manager.storageAvailable
        }

        SectionHeader {
            text: qsTr("Events")
            visible: manager.storageAvailable
        }

        Label {
            id: log
            x: Theme.horizontalPageMargin
            width: parent.width-2*Theme.horizontalPageMargin
            wrapMode: Text.Wrap
            visible: manager.storageAvailable

            text: ""

            function setText() {
                text = logger.log
            }

            Component.onCompleted: { setText() }
            Connections { target: logger; onLogChanged: log.setText() }
        }
    }


    //    Connections {
    //        target: manager

    //        onErrorMessage: {
    //            pageStack.completeAnimation()
    //            pageStack.push( Qt.resolvedUrl("MessagePage.qml"),
    //                           {"header": qsTr("Error"), "message": info} )
    //        }
    //    }

    //        ///////////////////////////////////////////////////////////
    //        /// welcome wizard
    //        Component {
    //            id: firstWelcomeWizardPage

    //            Dialog {

    //                acceptDestination: secondWelcomeWizardPage

    //                SilicaFlickable {
    //                    anchors.fill: parent
    //                    contentHeight: column.height + Theme.paddingLarge

    //                    Column {
    //                        id: column
    //                        width: parent.width

    //                        DialogHeader {
    //                            title: qsTr("Welcome")
    //                            acceptText: qsTr("Next")
    //                            cancelText: qsTr("Skip")
    //                        }

    //                        Label {
    //                            text: qsTr("OSM Scout Server is a part of the solution allowing you to have offline maps on device. " +
    //                                       "With this server, you could download the maps to your device and use the " +
    //                                       "downloaded data to locally render maps, search for addresses and POIs, and " +
    //                                       "calculate the routes. Such operations requires server and an additional client " +
    //                                       "accessing the server to run simultaneously on the device.<br><br>" +
    //                                       "This wizard will help you to select the backends used by the server and " +
    //                                       "the specify languages for parsing your search requests.<br><br>" +
    //                                       "Please choose 'Next' to start configuration."
    //                                       )
    //                            x: Theme.horizontalPageMargin
    //                            width: parent.width-2*x
    //                            wrapMode: Text.WordWrap
    //                            color: Theme.highlightColor
    //                        }
    //                    }

    //                    VerticalScrollDecorator {}
    //                }
    //            }
    //        }

    //        Component {
    //            id: secondWelcomeWizardPage

    //            ProfilesPage {
    //                acceptDestination: thirdWelcomeWizardPage
    //            }
    //        }

    //        Component {
    //            id: thirdWelcomeWizardPage

    //            LanguageSelector {
    //                acceptDestination: fourthWelcomeWizardPage

    //                value: eGeoLanguages.value
    //                callback: eGeoLanguages.setValue
    //                title: eGeoLanguages.mainLabel
    //                comment: eGeoLanguages.selectorComment
    //                note: eGeoLanguages.selectorNote
    //            }
    //        }

    //        Component {
    //            id: fourthWelcomeWizardPage

    //            SystemdActivationPage {
    //                acceptDestination: rootPage
    //                acceptDestinationAction: PageStackAction.Pop
    //            }
    //        }

    //        function openWelcomeWizard()
    //        {
    //            if (status == PageStatus.Active)
    //            {
    //                rootPage.statusChanged.disconnect(openWelcomeWizard)
    //                pageStack.push(firstWelcomeWizardPage)
    //            }
    //        }

    //        function openSystemdActivation()
    //        {
    //            if (status == PageStatus.Active)
    //            {
    //                rootPage.statusChanged.disconnect(openSystemdActivation)
    //                pageStack.push(fourthWelcomeWizardPage)
    //            }
    //        }

    //        Component.onCompleted: {
    //            if (settings.firstTime)
    //                rootPage.statusChanged.connect(openWelcomeWizard)
    //            else if (settings.lastRunVersion == 0)
    //                rootPage.statusChanged.connect(openSystemdActivation)
    //        }
}
