import QtQuick 2.0

import Sailfish.Silica 1.0

Column {
    id: root

    property string key
    property string mainLabel
    property string secondaryLabel
    property bool autoApply: false
    property bool activeState: true

    signal switchChanged(bool checked)

    width: parent.width
    anchors.margins: Theme.horizontalPageMargin
    spacing: Theme.paddingSmall

    function apply()
    {
        var ret = 0;
        if (textInput.checked) ret = 1;
        settings.setValue(key, ret)
    }

    TextSwitch {
        id: textInput
        width: parent.width
        text: parent.mainLabel
        enabled: root.activeState

        Component.onCompleted: {
            checked = (settings.valueInt(parent.key) > 0)
        }

        onCheckedChanged: {
            if ( checked != (settings.valueInt(parent.key) > 0) )
            {
                if (autoApply) apply()
                root.switchChanged(checked)
            }
        }
    }

    Label {
        text: parent.secondaryLabel
        x: Theme.horizontalPageMargin
        width: parent.width-2*x
        wrapMode: Text.WordWrap
        font.pixelSize: Theme.fontSizeSmall
        color: Theme.highlightColor

        Component.onCompleted: { visible = (parent.secondaryLabel.length > 0) }
    }
}


