import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property bool activeState: false
    property bool anythingToDelete: false
    property int nFiles: 0
    property int nDirs: 0
    property var fileNames: []
    property var dirNames: []

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        RemorsePopup { id: remorse }

        Column {
            id: column

            anchors.margins: Theme.horizontalPageMargin
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("Unused files")
            }

            Label {
                id: mainLabel
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
            }

            Label {
                id: warning
                width: parent.width
                visible: anythingToDelete
                text: qsTr("WARNING: While care has been taken to ensure deleting of the found unused files only, " +
                           "the software has not been tested by many users yet. Use at your own risk and please " +
                           "report if there are any problems.")
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            SectionHeader {
                text: qsTr("Directories")
                visible: anythingToDelete
            }

            Label {
                width: parent.width
                visible: anythingToDelete
                text: qsTr("Directories containing files that will be deleted during cleanup:")
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
            }

            Column {
                visible: anythingToDelete
                x: Theme.horizontalPageMargin
                width: parent.width - x
                spacing: Theme.paddingMedium
                Repeater {
                    model: nDirs
                    delegate: Label {
                        width: parent.width
                        text: dirNames[index]
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        color: Theme.highlightColor
                        font.pixelSize: Theme.fontSizeSmall
                    }
                }
            }

            SectionHeader {
                text: qsTr("Cleanup")
                visible: anythingToDelete
            }

            Column {
                visible: anythingToDelete
                width: parent.width
                spacing: Theme.paddingMedium
                anchors.margins: Theme.horizontalPageMargin

                Button {
                    text: qsTr("Delete files")
                    enabled: page.activeState
                    preferredWidth: Theme.buttonWidthLarge
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClicked: {
                        remorse.execute(qsTr("Deleting"),
                                        function () {
                                            manager.deleteNonNeededFiles(fileNames)
                                        }
                                        )
                    }
                }

                Label {
                    text: qsTr("Delete unused files and free the occupied space")
                    x: Theme.horizontalPageMargin
                    width: parent.width-2*x
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                }
            }

            VerticalScrollDecorator {}
        }
    }

    BusyIndicator {
        id: busy
        running: true
        size: BusyIndicatorSize.Large
        anchors.centerIn: parent
    }

    function updatePage() {
        fileNames = manager.getNonNeededFilesList()
        dirNames = manager.getDirsWithNonNeededFiles()
        var size = manager.getNonNeededFilesSize()
        if (size > 0) {
            nFiles = fileNames.length
            nDirs = dirNames.length
            var s = Math.round(size/1024/1024)
            mainLabel.text = qsTr("Occupied space") + ": " + s + " " + qsTr("MB")
        }
        else {
            nFiles = 0
            if (size < 0)
                mainLabel.text = qsTr("Failed to check for non-needed files. This could be due to active downloads, for example")
            else
                mainLabel.text = qsTr("All stored files are used by OSM Scout Server. " +
                                      "There is nothing to delete.")
        }
    }

    function checkState() {
        var size = manager.getNonNeededFilesSize()
        page.activeState = (size > 0 && manager.ready)
        page.anythingToDelete = (size > 0)
        busy.running = manager.deleting
    }

    Component.onCompleted: {
        updatePage()
        checkState()
        busy.running = false
    }

    Connections {
        target: manager
        onDeletingChanged: {
            if (!state)
                updatePage()
            checkState()
        }
        onReadyChanged: checkState()
    }
}
