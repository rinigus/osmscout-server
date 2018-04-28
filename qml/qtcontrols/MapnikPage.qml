import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."

Dialog {

    id: dialog
    title: qsTr("Mapnik")

    contentHeight: column.height + Theme.paddingLarge

    Column {
        id: column

        x: Theme.paddingLarge
        width: dialog.width-2*x

        Label {
            text: qsTr("Mapnik can be used as a rendering backend to provide map tiles to the client programs")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
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

    function onAccepted() {
        eScale.apply()
        eBuffer.apply()
    }
}
