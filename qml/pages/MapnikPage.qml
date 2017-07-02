import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {

    id: dialog
    allowedOrientations : Orientation.All

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: parent.width
            spacing: Theme.paddingLarge
            anchors.margins: Theme.horizontalPageMargin

            DialogHeader {
                title: qsTr("Mapnik")
            }

            Label {
                text: qsTr("Mapnik can be used as a rendering backend to provide map tiles to the client programs")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                //font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            ElementEntry {
                id: eScale
                key: settingsMapnikPrefix + "scale"
                mainLabel: qsTr("Scale factor")
                secondaryLabel: qsTr("Scale factor used to draw features on a map. On high resolution displays, such as " +
                                     "phones, it is advantageous to use scale factors larger than 1. As a first estimate, " +
                                     "it is suggested to use scale factor of 3-5 on mobile devices.")
                validator: DoubleValidator { bottom: 0.999; decimals: 1; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
                id: eBuffer
                key: settingsMapnikPrefix + "buffer_size_in_pixels"
                mainLabel: qsTr("Buffer size in pixels")
                secondaryLabel: qsTr("By increasing the size of the area which is used to load the data while rendering a tile, " +
                                     "the renderer can improve label rendering by taking into account data from neighboring tiles. " +
                                     "If you see too many cut labels, try to " +
                                     "increase the buffer size. Note that large buffer sizes would slow down the rendering. " +
                                     "The buffer size is scaled by the scaling factor to adjust for the increase in labels size.")
                validator: IntValidator { bottom: 0; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
        }

        VerticalScrollDecorator {}
    }

    onAccepted: {
        eScale.apply()
        eBuffer.apply()
        eStyle.apply()
    }
}
