import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.osmscout.server.FileManager 1.0

Column {
    id: main

    property string key
    property string mainLabel
    property string secondaryLabel
    property bool directory: false
    property string directory_file: ""

    property alias value: fullPath.text
    property alias display_value: textInput.text

    width: parent.width
    anchors.margins: Theme.horizontalPageMargin
    spacing: Theme.paddingSmall

    function apply()
    {
        settings.setValue(key, value)
    }

    function display()
    {
        display_value = value.split("/").pop()
    }

    function setPath(path)
    {
        value = path
        display()
    }

    function select()
    {
        pageStack.push(Qt.resolvedUrl("FileSelector.qml"), {
                           homePath: value,
                           showFormat: true,
                           title: "Select " + mainLabel,
                           callback: setPath,
                           directory: directory,
                           directory_file: directory_file
                       })
    }

    SectionHeader {
        text: parent.mainLabel
        font.pixelSize: Theme.fontSizeSmall
    }

    ListItem {
        id: listItem

        Column {
            width: parent.width
            height: textInput.height + fullPath.height + Theme.paddingSmall
            spacing: Theme.paddingSmall

            Label {
                id: textInput
                text: ""
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor

                Component.onCompleted: {
                    main.value = settings.valueString(main.key)
                    setPath(main.value)
                }
            }

            Label {
                id: fullPath
                text: ""
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                font.pixelSize: Theme.fontSizeTiny
                truncationMode: TruncationMode.Fade
            }
        }

        onClicked: select()
    }

    Label {
        id: secLabel
        text: parent.secondaryLabel
        x: Theme.horizontalPageMargin
        width: parent.width-2*x
        wrapMode: Text.WordWrap
        font.pixelSize: Theme.fontSizeSmall
        color: Theme.highlightColor

        Component.onCompleted: { visible = (parent.secondaryLabel.length > 0) }
    }
}

