// Based on @coderus implementation of the dialog:
// https://github.com/CODeRUS/splashscreen-changer/blob/master/settings/SecondPage.qml
//
// few changes introduced: visual and support for directory selection

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.osmscout.server.FileManager 1.0

Page {
    id: page
    allowedOrientations : Orientation.All

    property alias path: fileModel.path
    property string homePath
    property string title
    property bool showFormat
    property bool directory: false
    property string directory_file: ""

    signal formatClicked

    property var callback

    FileModel {
        id: fileModel

        path: homePath
        includeDirectories: true
        includeParentDirectory: true
        includeFiles: !directory
        active: page.status === PageStatus.Active
        onError: {
            console.log("Error ###", fileName, error)
        }
    }

    SilicaListView {
        id: fileList

        anchors.fill: parent
        model: fileModel

        header: Column {
//            height: ( head.height + fullpath.height + Theme.paddingSmall + Theme.paddingLarge +
//                      helpText.visible ? (helpText.height+Theme.paddingSmall) : 0 )
            spacing: Theme.paddingSmall

            PageHeader {
                id: head
                title: page.title
                wrapMode: Text.WordWrap
                width: page.width
            }

            Label {
                id: fullpath
                text: page.path
                x: Theme.horizontalPageMargin
                width: page.width-2*x
                horizontalAlignment: Text.AlignRight
                truncationMode: TruncationMode.Elide
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.highlightColor
            }

            Label {
                id: helpText
                text: qsTr("To select a folder, press and hold the folder and choose 'Select' in the context menu")
                x: Theme.horizontalPageMargin
                width: page.width-2*x
                visible: page.directory && page.directory_file.length < 1
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                wrapMode: Text.WordWrap
            }
        }

        delegate: ListItem {
            id: fileItem

            width: ListView.view.width
            contentHeight: Theme.itemSizeMedium

            Row {
                anchors.fill: parent
                spacing: Theme.paddingLarge
                Rectangle {
                    width: height
                    height: parent.height
                    color: "transparent"

                    Image {
                        anchors.centerIn: parent
                        source: {
                            var iconSource
                            if (model.isDir) {
                                iconSource = "image://theme/icon-m-file-folder"
                            } else {
                                var iconType = "other"
                                switch (model.mimeType) {
                                case "application/vnd.android.package-archive":
                                    iconType = "apk"
                                    break
                                case "application/x-rpm":
                                    iconType = "rpm"
                                    break
                                case "text/vcard":
                                    iconType = "vcard"
                                    break
                                case "text/plain":
                                case "text/x-vnote":
                                    iconType = "note"
                                    break
                                case "application/pdf":
                                    iconType = "pdf"
                                    break
                                case "application/vnd.oasis.opendocument.spreadsheet":
                                case "application/x-kspread":
                                case "application/vnd.ms-excel":
                                case "text/csv":
                                case "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet":
                                case "application/vnd.openxmlformats-officedocument.spreadsheetml.template":
                                    iconType = "spreadsheet"
                                    break
                                case "application/vnd.oasis.opendocument.presentation":
                                case "application/vnd.oasis.opendocument.presentation-template":
                                case "application/x-kpresenter":
                                case "application/vnd.ms-powerpoint":
                                case "application/vnd.openxmlformats-officedocument.presentationml.presentation":
                                case "application/vnd.openxmlformats-officedocument.presentationml.template":
                                    iconType = "presentation"
                                    break
                                case "application/vnd.oasis.opendocument.text-master":
                                case "application/vnd.oasis.opendocument.text":
                                case "application/vnd.oasis.opendocument.text-template":
                                case "application/msword":
                                case "application/rtf":
                                case "application/x-mswrite":
                                case "application/vnd.openxmlformats-officedocument.wordprocessingml.document":
                                case "application/vnd.openxmlformats-officedocument.wordprocessingml.template":
                                case "application/vnd.ms-works":
                                    iconType = "formatted"
                                    break
                                default:
                                    if (mimeType.indexOf("audio/") == 0) {
                                        iconType = "audio"
                                    } else if (mimeType.indexOf("image/") == 0) {
                                        iconType = "image"
                                    } else if (mimeType.indexOf("video/") == 0) {
                                        iconType = "video"
                                    }
                                }
                                iconSource = "image://theme/icon-m-file-" + iconType
                            }
                            return iconSource + (highlighted ? "?" + Theme.highlightColor : "")
                        }
                    }
                }
                Column {
                    width: parent.width - parent.height - parent.spacing - Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: -Theme.paddingSmall
                    Label {
                        text: model.fileName
                        width: parent.width
                        font.pixelSize: Theme.fontSizeLarge
                        truncationMode: TruncationMode.Fade
                        color: highlighted ? Theme.highlightColor : Theme.primaryColor
                    }
                    Label {
                        property string dateString: Format.formatDate(model.modified, Formatter.DateLong)
                        text: model.isDir ? dateString
                                            //: Shows size and modification date, e.g. "15.5MB, 02/03/2016"
                                            //% "%1, %2"
                                          : "%1, %2".arg(Format.formatFileSize(model.size)).arg(dateString)
                        width: parent.width
                        truncationMode: TruncationMode.Fade
                        font.pixelSize: Theme.fontSizeSmall
                        color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                    }
                }
            }

            onClicked: {
                if (!model.isDir ||
                        (page.directory && page.directory_file.length > 0 && fileModel.hasFile(model.fileName + "/" + page.directory_file) ) )
                {
                    var filePath = fileModel.appendPath(model.fileName)
                    if (typeof callback == "function")
                    {
                        callback(filePath);
                        pageStack.pop()
                    }
                }
                else
                {
                    fileModel.path = fileModel.appendPath(model.fileName)
                }
            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Select")
                    onClicked: {
                        var filePath = fileModel.appendPath(model.fileName)
                        if (typeof callback == "function")
                        {
                            callback(filePath);
                            pageStack.pop()
                        }
                    }
                    enabled: (page.directory && model.isDir && model.fileName!==".." ) || (!page.directory && !model.isDir)
                }
            }
        }

        ViewPlaceholder {
            enabled: fileModel.count === 0
            text: qsTr("Empty directory")
        }

        VerticalScrollDecorator {}
    }

    Component.onCompleted: {
        if (directory && (fileModel.hasFile(directory_file) || directory_file.length<1))
            fileModel.path = fileModel.appendPath("..")
    }
}
