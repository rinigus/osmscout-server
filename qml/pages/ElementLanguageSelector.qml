import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    id: main

    property string key
    property string mainLabel
    property string secondaryLabel

    property string value
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
        display_value = value
        if (value.length < 1)
            display_value = qsTr("All supported languages")
    }

    function setValue(v)
    {
        value = v
        display()
    }

    function select()
    {
        pageStack.push(Qt.resolvedUrl("LanguageSelector.qml"), {
                           value: value,
                           title: mainLabel,
                           callback: setValue,
                           comment: secondaryLabel
                       })
    }

    Label {
        text: parent.mainLabel
        x: Theme.horizontalPageMargin
        width: parent.width-2*x
        wrapMode: Text.WordWrap
        font.pixelSize: Theme.fontSizeMedium
        color: Theme.highlightColor
    }

    ListItem {
        id: listItem
        width: parent.width

        Label {
            id: textInput
            text: ""
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor

            Component.onCompleted: {
                setValue(settings.valueString(main.key))
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

    Rectangle {
        height: Theme.paddingMedium
        width: parent.width
        color: "transparent"
    }
}

