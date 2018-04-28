import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."

Dialog {

    id: dialog
    title: qsTr("Valhalla")

    contentHeight: column.height + Theme.paddingLarge

    Column {
        id: column

        x: Theme.horizontalPageMargin
        width: dialog.width-2*x
        spacing: Theme.paddingLarge

        Label {
            text: qsTr("Valhalla can be used as a routing engine to calculate routes and the routing instructions")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
        }

        SectionHeader {
            text: qsTr("Memory")
        }

        ElementEntry {
            id: eCache
            key: settingsValhallaPrefix + "cache_in_mb"
            mainLabel: qsTr("Cache for storing tiles, MB")
            secondaryLabel: qsTr("Memory cache that is used to keep Valhalla's tiles in RAM.")
            validator: DoubleValidator { bottom: 16; top: 1024; decimals: 0; }
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }

        SectionHeader {
            text: qsTr("Limits")
        }

        Label {
            text: qsTr("Distance limits for route calculations. Only routes with the distances between locations " +
                       "that are smaller than the given limits will be calculated. By limiting the maximal distances " +
                       "between locations, you can prevent accidental calculations that would lead to too large " +
                       "routes and would use excessive amounts of RAM on your device.")
            x: Theme.horizontalPageMargin
            width: parent.width-2*x
            wrapMode: Text.WordWrap
            font.pointSize: Theme.fontSizeSmall
        }

        ElementEntry {
            id:  eCar
            key: settingsValhallaPrefix + "limit_max_distance_auto"
            mainLabel: qsTr("Limit for route by car")
            validator: DoubleValidator { bottom: 10; top:10000; decimals: 0; }
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }

        ElementEntry {
            id:  eBicycle
            key: settingsValhallaPrefix + "limit_max_distance_bicycle"
            mainLabel: qsTr("Limit for route by bicycle")
            validator: DoubleValidator { bottom: 10; top: 1000; decimals: 0; }
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }

        ElementEntry {
            id:  ePedestrian
            key: settingsValhallaPrefix + "limit_max_distance_pedestrian"
            mainLabel: qsTr("Limit for route by foot")
            validator: DoubleValidator { bottom: 10; top: 1000; decimals: 0; }
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }
    }

    function onAccepted() {
        eCache.apply()
        eCar.apply()
        eBicycle.apply()
        ePedestrian.apply()
    }
}
