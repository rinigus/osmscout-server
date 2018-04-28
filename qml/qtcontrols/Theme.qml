pragma Singleton
import QtQuick 2.0

Item {
    readonly property int horizontalPageMargin: 10

    readonly property int paddingLarge: 10
    readonly property int paddingMedium: 7
    readonly property int paddingSmall: 5

    readonly property int iconSizeMedium: 20

    property int fontSizeNormal: Qt.application.font.pointSize
    property int fontSizeLarge: fontSizeNormal*1.2
    property int fontSizeSmall: fontSizeNormal*0.9
    property int fontSizeExtraSmall: fontSizeNormal*0.8
    property int fontSizeTiny: fontSizeNormal*0.7
}
