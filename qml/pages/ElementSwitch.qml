import QtQuick 2.0

import Sailfish.Silica 1.0

Column {
    property string key
    property string mainLabel
    property string secondaryLabel

    width: parent.width
    anchors.margins: Theme.horizontalPageMargin
    spacing: Theme.paddingSmall

    TextSwitch {
        id: textInput
        width: parent.width
        text: parent.mainLabel

        Component.onCompleted: {
            checked = (settings.valueInt(settingsOsmPrefix + parent.key) > 0)
        }

        onCheckedChanged: {
            var ret = 0;
            if (checked) ret = 1;
            settings.setValue(settingsOsmPrefix + parent.key, ret)
        }
    }

    Label {
        text: parent.secondaryLabel
        x: Theme.horizontalPageMargin
        width: parent.width-2*x
        wrapMode: Text.WordWrap
        font.pixelSize: Theme.fontSizeSmall

        Component.onCompleted: { visible = (parent.secondaryLabel.length > 0) }
    }
}


