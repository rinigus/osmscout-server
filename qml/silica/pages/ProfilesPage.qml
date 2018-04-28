import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {

    id: dialog
    allowedOrientations : Orientation.All
    canAccept: manager.ready

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
                title: qsTr("Profile")
            }

            Label {
                text: qsTr("OSM Scout Server uses profiles to simplify the selection " +
                           "of backends and the sets of downloaded databases.<br><br>" +
                           "Please select active profile."
                           )
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.highlightColor
            }

            ComboBox {
                id: profileSelection
                label: qsTr("Profile")
                enabled: manager.ready

                menu: ContextMenu {
                    MenuItem { text: qsTr("Default") }
                    MenuItem { text: qsTr("Recommended for raster tiles maps") }
                    MenuItem { text: qsTr("Recommended for vector and raster tiles maps") }
                    MenuItem { text: qsTr("<i>libosmscout</i> with <i>Geocoder-NLP</i>") }
                    MenuItem { text: qsTr("<i>libosmscout</i>") }
                    MenuItem { text: qsTr("Custom") }
                }

                Component.onCompleted: {
                    currentIndex = settings.valueInt(settingsGeneralPrefix + "profile")
                }
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor

                text: qsTr("<i>Default</i> profile supports map applications using <i>Mapbox GL</i> plugin, uses " +
                           "<i>Geocoder-NLP</i> to search for locations, " +
                           "and <i>Valhalla</i> to calculate the routes. " +
                           "This profile is a recommended one for new vector tile map clients.<br><br>" +

                           "For users with map applications using traditional raster tiles, " +
                           "the recommended profile for raster tiles maps includes <i>Mapnik</i> datasets and uses " +
                           "the same search and routing plugins as the <i>Default</i> profile.<br><br>" +

                           "For users wishing to use raster and vector tile map clients, " +
                           "the recommended profile combining vector and raster tiles includes <i>Mapbox GL</i> datasets, supports " +
                           "map rendering into raster tiles by <i>Mapnik</i>, and uses " +
                           "the same search and routing plugins as the <i>Default</i> profile. Since this profile includes " +
                           " data for multiple rendering backends, it has the largest storage requirements.<br><br>" +

                           "The profile where <i>libosmscout</i> is combined with <i>Geocoder-NLP</i> " +
                           "has smaller storage requirements when compared to the default one. " +
                           "However, rendering of the maps and routing would be limited only to one territory. " +
                           "In addition, rendering quality is inferior and routing speed is slower when compared to " +
                           "the default profile.<br><br>" +

                           "<i>libosmscout</i> profile has the smallest storage requirements among all profiles. " +
                           "However, rendering of the maps, search, and routing would be limited only to one territory. " +
                           "In addition, rendering quality and search is inferior as well as " +
                           "routing speed is slower  when compared to " +
                           "the default profile.<br><br>" +

                           "When using <i>Custom</i> profile, Settings and Map Manager Storage are not set by profiles " +
                           "and should be specified by user. " +
                           "This profile allows to select rendering, search, and routing components individually. Note that " +
                           "the user is responsible for adjusting the settings to make them consistent between requirements of " +
                           "the used backends and storage." )
            }
        }

        VerticalScrollDecorator {}
    }

    onAccepted: {
        if (manager.ready)
            settings.setValue(settingsGeneralPrefix + "profile", profileSelection.currentIndex)
    }
}
