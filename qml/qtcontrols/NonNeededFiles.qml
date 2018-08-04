/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
 * 
 * This file is part of OSM Scout Server.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."

Dialog {
    id: page

    dialogue: false
    title: qsTr("Unused files")

    property bool activeState: false
    property bool anythingToDelete: false
    property int nFiles: 0
    property int nDirs: 0
    property var fileNames: []
    property var dirNames: []

    contentHeight: column.height + Theme.paddingLarge

    Column {
        id: column

        x: Theme.paddingLarge
        width: page.width-2*x

        spacing: Theme.paddingLarge

        Label {
            id: mainLabel
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Label {
            id: warning
            width: parent.width
            visible: anythingToDelete
            text: qsTr("WARNING: While care has been taken to ensure deleting of the found unused files only, " +
                       "the software has not been tested by many users yet. Use at your own risk and please " +
                       "report if there are any problems.")
            wrapMode: Text.WordWrap
        }

        SectionHeader {
            text: qsTr("Directories")
            visible: anythingToDelete
        }

        Label {
            width: parent.width
            visible: anythingToDelete
            text: qsTr("Directories containing files that will be deleted during cleanup:")
            wrapMode: Text.WordWrap
        }

        Column {
            visible: anythingToDelete
            x: Theme.horizontalPageMargin
            width: parent.width - x
            spacing: Theme.paddingMedium
            Repeater {
                model: nDirs
                delegate: Label {
                    width: parent.width
                    text: dirNames[index]
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    font.pointSize: Theme.fontSizeSmall
                }
            }
        }

        SectionHeader {
            text: qsTr("Cleanup")
            visible: anythingToDelete
        }

        Column {
            visible: anythingToDelete
            width: parent.width
            spacing: Theme.paddingMedium
            anchors.margins: Theme.horizontalPageMargin

            Button {
                text: qsTr("Delete files")
                enabled: page.activeState
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    manager.deleteNonNeededFiles(fileNames)
                }
            }

            Label {
                text: qsTr("Delete unused files and free the occupied space")
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pointSize: Theme.fontSizeSmall
            }
        }

    }

    BusyIndicator {
        id: busy
        running: true
        anchors.centerIn: parent
    }

    function updatePage() {
        nDirs = 0
        nFiles = 0
        fileNames = manager.getNonNeededFilesList()
        dirNames = manager.getDirsWithNonNeededFiles()
        var size = manager.getNonNeededFilesSize()
        if (size > 0) {
            nFiles = fileNames.length
            nDirs = dirNames.length
            var s = Math.round(size/1024/1024)
            mainLabel.text = qsTr("Occupied space") + ": " + s + " " + qsTr("MB")
        }
        else {
            nFiles = 0
            if (size < 0)
                mainLabel.text = qsTr("Failed to check for non-needed files. This could be due to active downloads, for example")
            else
                mainLabel.text = qsTr("All stored files are used by OSM Scout Server. " +
                                      "There is nothing to delete.")
        }
    }

    function checkState() {
        var size = manager.getNonNeededFilesSize()
        page.activeState = (size > 0 && manager.ready)
        page.anythingToDelete = (size > 0)
        busy.running = manager.deleting
    }

    Component.onCompleted: {
        updatePage()
        checkState()
        busy.running = false
    }

    Connections {
        target: manager
        onDeletingChanged: {
            if (!state)
                updatePage()
            checkState()
        }
        onReadyChanged: checkState()
    }
}
