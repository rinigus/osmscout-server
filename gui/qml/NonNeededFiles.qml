/*
 * Copyright (C) 2016-2019 Rinigus https://github.com/rinigus
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
import "."
import "platform"

PagePL {
    id: page
    title: qsTr("Unused files")

    property bool activeState: false
    property bool anythingToDelete: false
    property int nFiles: 0
    property int nDirs: 0
    property var fileNames: []
    property var dirNames: []

    Item {
        height: Math.max(column.height, page.height*0.75)
        width: page.width

        Column {
            id: column
            spacing: styler.themePaddingLarge
            width: page.width

            ListItemLabel {
                id: mainLabel
            }

            SectionHeaderPL {
                text: qsTr("Directories")
                visible: anythingToDelete
            }

            ListItemLabel {
                text: qsTr("Directories containing files that will be deleted during cleanup:")
                visible: anythingToDelete
            }

            Column {
                spacing: styler.themePaddingMedium
                visible: anythingToDelete
                x: styler.themeHorizontalPageMargin + styler.themePaddingLarge
                width: parent.width - 2*x
                Repeater {
                    model: nDirs
                    delegate: ListItemLabel {
                        font.pixelSize: styler.themeFontSizeSmall
                        text: dirNames[index]
                    }
                }
            }

            SectionHeaderPL {
                text: qsTr("Cleanup")
                visible: anythingToDelete
            }

            Column {
                spacing: styler.themePaddingMedium
                visible: anythingToDelete
                width: parent.width

                ButtonPL {
                    anchors.horizontalCenter: parent.horizontalCenter
                    enabled: page.activeState
                    preferredWidth: styler.themeButtonWidthLarge
                    text: qsTr("Delete files")
                    onClicked: {
                        remorse.execute(qsTr("Deleting"),
                                        function () {
                                            manager.deleteNonNeededFiles(fileNames)
                                        }
                                        )
                    }
                }

                ListItemLabel {
                    font.pixelSize: styler.themeFontSizeSmall
                    text: qsTr("Delete unused files and free the occupied space")
                }
            }
        }

        RemorsePopupPL { id: remorse }

        BusyIndicatorPL {
            id: busy
            anchors.centerIn: parent
            running: true
        }
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

    Component.onCompleted: {
        updatePage()
        checkState()
        busy.running = false
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
}
