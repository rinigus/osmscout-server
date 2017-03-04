import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property bool activeState: false
    property int nFiles: 0
    property var fileNames: []

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
                title: qsTr("Non-needed files")
            }

            Label {
                id: sizeLabel
                width: parent.width
                horizontalAlignment: Text.AlignRight
                color: Theme.highlightColor
            }

            Label {
                width: parent.width
                text: qsTr("Files:")
                color: Theme.highlightColor
            }

            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - x
                spacing: Theme.paddingMedium
                Repeater {
                    model: nFiles
                    delegate: Label {
                        width: parent.width
                        text: fileNames[index]
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        color: Theme.highlightColor
                        font.pixelSize: Theme.fontSizeSmall
                    }
                }
            }

            Column {
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
                                            pageStack.pop()
                                        }
                                        )
                    }
                }

                Label {
                    text: qsTr("Delete non needed files")
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
    }

    Component.onCompleted: {
        fileNames = manager.getNonNeededFilesList()
        var size = manager.getNonNeededFilesSize()
        if (size >= 0) {
            nFiles = fileNames.length
            var s = Math.round(size/1024/1024)
            sizeLabel.text = qsTr("Occupied space") + ": " + s + " " + qsTr("MB")
        }
        else {
            nFiles = 0
            sizeLabel.text = qsTr("Failed to check for non-needed files. This could be due to active downloads, for example")
        }

        page.activeState = (size > 0 && !manager.downloading)

        busy.running = false
    }
}
