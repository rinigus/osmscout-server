/*
 * Copyright (C) 2016-2019 Rinigus https://github.com/rinigus
 *                    2019 Purism SPC
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
import QtQuick.Dialogs 1.3

FileDialog {
    id: fs

    folder: 'file://' + homePath

    property var    callback
    property alias  directory: fs.selectFolder
    property string directoryFile: "" // not used in this implementation
    property string homePath

    onAccepted: {
        if (callback) {
            var f = String(fileUrl);
            if (f.startsWith('file://')) {
                f = f.substr(7);
            }
            callback(f);
        }
    }
}
