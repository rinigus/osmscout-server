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
                title: qsTr("Routing speeds")
            }

            SectionHeader {
                text: qsTr("Modes of transportation")
                font.pixelSize: Theme.fontSizeMedium
            }

            ElementEntry {
                id:  libosmscout_speed_Car
                key: settingsSpeedPrefix + "Car"
                mainLabel: qsTr("Maximal car speed, km/h")
                secondaryLabel: qsTr("This is a speed that is assumed to be never exceeded during calculations of the route")
                validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
                id:  libosmscout_speed_Bicycle
                key: settingsSpeedPrefix + "Bicycle"
                mainLabel: qsTr("Expected speed of a bicycle, km/h")
                validator: DoubleValidator { bottom: 0.1; top: 1.079e+9; decimals: 0; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
                id:  libosmscout_speed_Foot
                key: settingsSpeedPrefix + "Foot"
                mainLabel: qsTr("Expected speed while walking, km/h")
                validator: DoubleValidator { bottom: 0.1; top: 1.079e+9; decimals: 0; }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            SectionHeader {
                text: qsTr("Car speed on different roads")
                font.pixelSize: Theme.fontSizeMedium
            }

            ElementEntry {
             id:  libosmscout_speed_highway_living_street
             key:  settingsSpeedPrefix +  "highway_living_street"
             mainLabel: qsTr( "living street, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_motorway
             key:  settingsSpeedPrefix +  "highway_motorway"
             mainLabel: qsTr( "motorway, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_motorway_junction
             key:  settingsSpeedPrefix +  "highway_motorway_junction"
             mainLabel: qsTr( "motorway junction, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_motorway_link
             key:  settingsSpeedPrefix +  "highway_motorway_link"
             mainLabel: qsTr( "motorway link, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_motorway_primary
             key:  settingsSpeedPrefix +  "highway_motorway_primary"
             mainLabel: qsTr( "motorway primary, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_motorway_trunk
             key:  settingsSpeedPrefix +  "highway_motorway_trunk"
             mainLabel: qsTr( "motorway trunk, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_primary
             key:  settingsSpeedPrefix +  "highway_primary"
             mainLabel: qsTr( "primary, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_primary_link
             key:  settingsSpeedPrefix +  "highway_primary_link"
             mainLabel: qsTr( "primary link, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_residential
             key:  settingsSpeedPrefix +  "highway_residential"
             mainLabel: qsTr( "residential, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_road
             key:  settingsSpeedPrefix +  "highway_road"
             mainLabel: qsTr( "road, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_roundabout
             key:  settingsSpeedPrefix +  "highway_roundabout"
             mainLabel: qsTr( "roundabout, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_secondary
             key:  settingsSpeedPrefix +  "highway_secondary"
             mainLabel: qsTr( "secondary, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_secondary_link
             key:  settingsSpeedPrefix +  "highway_secondary_link"
             mainLabel: qsTr( "secondary link, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_service
             key:  settingsSpeedPrefix +  "highway_service"
             mainLabel: qsTr( "service, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_tertiary
             key:  settingsSpeedPrefix +  "highway_tertiary"
             mainLabel: qsTr( "tertiary, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_tertiary_link
             key:  settingsSpeedPrefix +  "highway_tertiary_link"
             mainLabel: qsTr( "tertiary link, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_trunk
             key:  settingsSpeedPrefix +  "highway_trunk"
             mainLabel: qsTr( "trunk, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_trunk_link
             key:  settingsSpeedPrefix +  "highway_trunk_link"
             mainLabel: qsTr( "trunk link, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            ElementEntry {
             id:  libosmscout_speed_highway_unclassified
             key:  settingsSpeedPrefix +  "highway_unclassified"
             mainLabel: qsTr( "unclassified, km/h" )
             validator: DoubleValidator { bottom: 1; top: 1.079e+9; decimals: 0; }
             inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
        }

        VerticalScrollDecorator {}
    }

    onAccepted: {
        libosmscout_speed_Bicycle.apply() ;
        libosmscout_speed_Car.apply() ;
        libosmscout_speed_Foot.apply() ;
        libosmscout_speed_highway_living_street.apply() ;
        libosmscout_speed_highway_motorway.apply() ;
        libosmscout_speed_highway_motorway_junction.apply() ;
        libosmscout_speed_highway_motorway_link.apply() ;
        libosmscout_speed_highway_motorway_primary.apply() ;
        libosmscout_speed_highway_motorway_trunk.apply() ;
        libosmscout_speed_highway_primary.apply() ;
        libosmscout_speed_highway_primary_link.apply() ;
        libosmscout_speed_highway_residential.apply() ;
        libosmscout_speed_highway_road.apply() ;
        libosmscout_speed_highway_roundabout.apply() ;
        libosmscout_speed_highway_secondary.apply() ;
        libosmscout_speed_highway_secondary_link.apply() ;
        libosmscout_speed_highway_service.apply() ;
        libosmscout_speed_highway_tertiary.apply() ;
        libosmscout_speed_highway_tertiary_link.apply() ;
        libosmscout_speed_highway_trunk.apply() ;
        libosmscout_speed_highway_trunk_link.apply() ;
        libosmscout_speed_highway_unclassified.apply() ;
    }
}


