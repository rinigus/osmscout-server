import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    id: main

    property string key
    property string mainLabel: qsTr("Languages")
    property string secondaryLabel: qsTr("List of languages used for parsing addresses")
    property string selectorComment: qsTr("Please select languages used for parsing addresses.")
    property string selectorNote: qsTr("When none of the "+
                                       "languages are selected in this form, it is assumed that all supported languages " +
                                       "should be used for parsing.<br>" +
                                       "Note that when all supported languages are used, RAM requirements could be large.")

    property string value
    property alias display_value: textInput.text

    property bool autoApply: false

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
        if (autoApply) apply()
    }

    function select()
    {
        pageStack.push(Qt.resolvedUrl("LanguageSelector.qml"), {
                           value: value,
                           title: mainLabel,
                           callback: setValue,
                           comment: selectorComment,
                           note: selectorNote
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
            anchors.verticalCenter: parent.verticalCenter
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

