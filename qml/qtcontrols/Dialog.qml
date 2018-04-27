import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."

ScrollView {

    id: main

    property bool dialogue: true
    property bool canAccept: true

    property string title

    width: parent ? parent.width : 100
    height: parent ? parent.height : 100
}
