import QtQuick 2.0

Label {
    id: activeDownloadText
    text: ""
    x: Theme.horizontalPageMargin
    width: parent.width-2*x
    wrapMode: Text.WordWrap

    function setText(state)
    {
        if (!state) activeDownloadText.text = qsTr("No downloads active")
        else activeDownloadText.text = qsTr("Download active")
    }

    Connections {
        target: manager
        onDownloadProgress: activeDownloadText.text = info
    }

    Connections {
        target: manager
        onDownloadingChanged: {
            if (!state) activeDownloadText.setText(state)
        }
    }

    Component.onCompleted: activeDownloadText.setText(manager.downloading)
}
