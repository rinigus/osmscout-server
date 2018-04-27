import QtQuick 2.0
import QtQuick.Controls 2.2
import "."

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

    Switch {
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

        Component.onCompleted: { visible = (parent.secondaryLabel.length > 0) }
    }
}


