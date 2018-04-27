pragma Singleton
import QtQuick 2.0

Item {
    readonly property int horizontalPageMargin: 10

    readonly property int paddingLarge: 10
    readonly property int paddingMedium: 7
    readonly property int paddingSmall: 5

    readonly property int iconSizeMedium: 20

    Text {
        id: referenceFont
        text: "Test"
    }

    property int fontSizeNormal: referenceFont.font.pointSize
    property int fontSizeLarge: fontSizeNormal*1.2
    property int fontSizeSmall: fontSizeNormal*0.9
    property int fontSizeExtraSmall: fontSizeNormal*0.75
    property int fontSizeTiny: fontSizeNormal*0.65
}
