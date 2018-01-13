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
                title: qsTr("Automatic activation")
            }

            Label {
                text: qsTr("OSM Scout Server can be activated automatically when the clients access it. " +
                           "When started automatically, it will be stopped if idle after given period of time.")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                //font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            Label {
                text: qsTr("Do you want to enable automatic activation?")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                //font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            TextSwitch {
                id: systemdEnable
                text: qsTr("Automatic activation")
                Component.onCompleted: {
                    checked = systemd_service.enabled
                }
            }

            Label {
                text: qsTr("It is recommended to enable automatic activation to simplify the access to the server.")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            Label {
                text: qsTr("Note that when automatically activated, the server runs without any user interface. " +
                           "Automatic activation and the corresponding idle timeout can be later configured in Settings.")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            Label {
                text: qsTr("For technical details, created files, and how to remove them if needed, see corresponding section " +
                           "of the <a href='https://rinigus.github.io/osmscout-server/en/#implementation-of-automatic-activation'>User's Guide</a>.")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                linkColor: Theme.primaryColor
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        VerticalScrollDecorator {}
    }

    onAccepted: {
        systemd_service.enabled = systemdEnable.checked
    }
}
