import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    property string key
    property string mainLabel
    property string secondaryLabel

    property alias value: textInput.text
    property alias validator: textInput.validator
    property alias inputMethodHints: textInput.inputMethodHints

    width: parent.width
    anchors.margins: Theme.horizontalPageMargin
    spacing: Theme.paddingSmall

    TextField {
        id: textInput
        width: parent.width
        text: ""
        label: parent.mainLabel
        placeholderText: parent.mainLabel

        Component.onCompleted: {
            text = settings.valueString(settingsOsmPrefix + parent.key)
        }

        EnterKey.enabled: text.length > 0
        EnterKey.onClicked: {
            settings.setValue(settingsOsmPrefix + parent.key, parent.value)
            focus = false
        }

        onFocusChanged: {
            if (!focus)
            {
                if (settings.valueString(settingsOsmPrefix + parent.key) !== text)
                {
                    secLabel.text = "<b>Setting not applied.</b> " + parent.secondaryLabel
                    secLabel.visible = true
                }
                else
                {
                    secLabel.text = parent.secondaryLabel
                    secLabel.visible = (parent.secondaryLabel.length > 0)
                }
            }
        }
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

