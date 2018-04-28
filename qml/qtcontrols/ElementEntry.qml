import QtQuick 2.0
import QtQuick.Controls 1.4
import "."

Column {
    id: main

    property string key
    property string mainLabel
    property string secondaryLabel
    property bool hasUnits: false
    property double displayFactor: 1.0

    property alias value: textInput.text
    property alias validator: textInput.validator
    property alias inputMethodHints: textInput.inputMethodHints

    width: parent.width
    anchors.margins: Theme.horizontalPageMargin
    spacing: Theme.paddingSmall

    function apply()
    {
        if (textInput.acceptableInput)
        {
            var rep = value
            if (main.hasUnits)
                rep = parseFloat(value) / main.displayFactor

            settings.setValue(key, rep)
        }
    }

    Row {
        spacing: Theme.horizontalPageMargin
        x: Theme.horizontalPageMargin
        width: main.width - 2*x

        Text {
            id: textInputLabel
            text: main.mainLabel
            anchors.verticalCenter: textInput.verticalCenter
        }


        TextField {
            id: textInput
            width: parent.width - textInputLabel.width - 2*Theme.horizontalPageMargin
            text: ""
            placeholderText: main.mainLabel

            Component.onCompleted: {
                text = settings.valueString(main.key)
                main.hasUnits = settings.hasUnits(main.key)
                if (main.hasUnits)
                {
                    main.displayFactor = settings.unitFactor();
                    validator.decimals = settings.unitDisplayDecimals()

                    var v = parseFloat(text) * main.displayFactor
                    text = v.toFixed(settings.unitDisplayDecimals())

                    textInputLabel.text = main.mainLabel + ", " + settings.unitName(main.key)
                }
            }

            //        EnterKey.enabled: text.length > 0
            //        EnterKey.onClicked: {
            //            /// commented out since we apply settings separately
            //            // settings.setValue(parent.key, parent.value)
            //            focus = false
            //        }

            //        onFocusChanged: {
            //            if (!focus)
            //            {
            //                if (settings.valueString(parent.key) !== text)
            //                {
            //                    secLabel.text = "<b>Setting not applied.</b> " + parent.secondaryLabel
            //                    secLabel.visible = true
            //                }
            //                else
            //                {
            //                    secLabel.text = parent.secondaryLabel
            //                    secLabel.visible = (parent.secondaryLabel.length > 0)
            //                }
            //            }
            //        }
        }
    }

    Label {
        id: secLabel
        text: main.secondaryLabel
        x: Theme.horizontalPageMargin
        width: main.width-2*x
        wrapMode: Text.WordWrap
        font.pointSize: Theme.fontSizeSmall

        Component.onCompleted: { visible = (parent.secondaryLabel.length > 0) }
    }
}

