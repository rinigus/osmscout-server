/*
 * Copyright (C) 2016 Jolla Ltd.
 * Contact: Joona Petrell <joona.petrell@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Jolla Ltd. nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include "filemodel.h"
#include "synchronizelists.h"

#include <QDateTime>
#include <QDebug>
#include <QGuiApplication>
#include <QMimeType>
#include <QQmlInfo>
#include <QFileInfo>

#include <unistd.h>

namespace {

enum {
    FileNameRole = Qt::UserRole + 1,
    MimeTypeRole = Qt::UserRole + 2,
    SizeRole = Qt::UserRole + 3,
    LastModifiedRole = Qt::UserRole + 4,
    CreatedRole = Qt::UserRole + 5,
    IsDirRole = Qt::UserRole + 6,
    IsLinkRole = Qt::UserRole + 7,
    SymLinkTargetRole = Qt::UserRole + 8,
    IsSelectedRole = Qt::UserRole + 9,
    ExtensionRole = Qt::UserRole + 10,
    AbsolutePathRole = Qt::UserRole + 11,
    LastAccessedRole = Qt::UserRole + 12,
    BaseNameRole = Qt::UserRole + 13
};

int access(QString fileName, int how)
{
    QByteArray fab = fileName.toUtf8();
    char *fn = fab.data();
    return ::access(fn, how);
}

QVector<StatFileInfo> directoryEntries(const QDir &dir)
{
    QVector<StatFileInfo> rv;

    QStringList fileList = dir.entryList();
    rv.reserve(fileList.count());

    foreach (const QString &fileName, fileList) {
        if (fileName.startsWith("qt_temp.")) {
            // Workaround for QFile::copy() creating intermediate qt_temp.* file (see QTBUG-27601)
            continue;
        }
        QString fullpath = dir.absoluteFilePath(fileName);
        StatFileInfo info(fullpath);
        rv.append(info);
    }

    return rv;
}

}

FileModel::FileModel(QObject *parent) :
    QAbstractListModel(parent),
    m_sortBy(SortByName),
    m_directorySort(SortDirectoriesBeforeFiles),
    m_sortOrder(Qt::AscendingOrder),
    m_caseSensitivity(Qt::CaseSensitive),
    m_includeDirectories(true),
    m_includeParentDirectory(false),
    m_includeHiddenFiles(false),
    m_includeSystemFiles(false),
    m_includeFiles(true),
    m_active(false),
    m_dirty(false),
    m_populated(false),
    m_selectedCount(0)
{
    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(refresh()));
    connect(m_watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(refresh()));
}

FileModel::~FileModel()
{
}

int FileModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_files.count();
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() > m_files.size()-1)
        return QVariant();

    const StatFileInfo &info = m_files.at(index.row());
    switch (role) {

    case Qt::DisplayRole:
    case FileNameRole:
        return info.fileName();

    case MimeTypeRole:
        return m_mimeDatabase.mimeTypeForFile(info.absoluteFilePath()).name();

    case SizeRole:
        return info.size();

    case LastModifiedRole:
        return info.lastModified();

    case CreatedRole:
        return info.created();

    case IsDirRole:
        return info.isDirAtEnd();

    case IsLinkRole:
        return info.isSymLink();

    case SymLinkTargetRole:
        return info.symLinkTarget();

    case IsSelectedRole:
        return info.isSelected();

    case ExtensionRole:
        return info.extension();

    case AbsolutePathRole:
        return info.absoluteFilePath();

    case LastAccessedRole:
        return info.lastAccessed();

    case BaseNameRole:
        return info.baseName();

    default:
        return QVariant();
    }
}

QHash<int, QByteArray> FileModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(FileNameRole, QByteArray("fileName"));
    roles.insert(MimeTypeRole, QByteArray("mimeType"));
    roles.insert(SizeRole, QByteArray("size"));
    roles.insert(LastModifiedRole, QByteArray("modified"));
    roles.insert(CreatedRole, QByteArray("created"));
    roles.insert(IsDirRole, QByteArray("isDir"));
    roles.insert(IsLinkRole, QByteArray("isLink"));
    roles.insert(SymLinkTargetRole, QByteArray("symLinkTarget"));
    roles.insert(IsSelectedRole, QByteArray("isSelected"));
    roles.insert(ExtensionRole, QByteArray("extension"));
    roles.insert(AbsolutePathRole, QByteArray("absolutePath"));
    roles.insert(LastAccessedRole, QByteArray("accessed"));
    roles.insert(BaseNameRole, QByteArray("baseName"));
    return roles;
}

int FileModel::count() const
{
    return m_files.count();
}

void FileModel::setPath(QString path)
{
    {
        QDir _d(path);
        path = _d.absolutePath();
    }

    if (m_path == path)
        return;

    if (m_populated) {
        m_populated = false;
        emit populatedChanged();
    }

    // set path to existing directory
    QFileInfo ftest(path);
    while (!ftest.exists() || ftest.isFile())
    {
        path = ftest.absoluteDir().absolutePath();
        ftest.setFile(path);
    }

    // update watcher to watch the new directory
    if (!m_path.isEmpty())
        m_watcher->removePath(m_path);

    if (!path.isEmpty())
        m_watcher->addPath(path);

    m_path = path;
    m_absolutePath = QString();
    m_directory = QString();
    m_parentPath = QString();
    scheduleUpdate(PathChanged);
}

void FileModel::setSortBy(Sort sortBy)
{
    if (m_sortBy == sortBy)
        return;

    m_sortBy = sortBy;
    scheduleUpdate(SortByChanged | ContentChanged);
}

void FileModel::setSortOrder(Qt::SortOrder order)
{
    if (m_sortOrder == order)
        return;

    m_sortOrder = order;
    scheduleUpdate(SortOrderChanged | ContentChanged);
}

void FileModel::setCaseSensitivity(Qt::CaseSensitivity sensitivity)
{
    if (m_caseSensitivity == sensitivity)
        return;

    m_caseSensitivity = sensitivity;
    scheduleUpdate(CaseSensitivityChanged | ContentChanged);
}

void FileModel::setIncludeDirectories(bool include)
{
    if (m_includeDirectories == include)
        return;

    m_includeDirectories = include;
    scheduleUpdate(IncludeDirectoriesChanged | ContentChanged);
}

void FileModel::setIncludeParentDirectory(bool include)
{
    if (m_includeParentDirectory == include)
        return;

    m_includeParentDirectory = include;
    scheduleUpdate(IncludeParentDirectoryChanged | ContentChanged);
}

void FileModel::setIncludeHiddenFiles(bool include)
{
    if (m_includeHiddenFiles == include)
        return;

    m_includeHiddenFiles = include;
    scheduleUpdate(IncludeHiddenFilesChanged | ContentChanged);
}

void FileModel::setIncludeSystemFiles(bool include)
{
    if (m_includeSystemFiles == include)
        return;

    m_includeSystemFiles = include;
    scheduleUpdate(IncludeSystemFilesChanged | ContentChanged);
}

void FileModel::setIncludeFiles(bool include)
{
    if (m_includeFiles == include)
        return;

    m_includeFiles = include;
    scheduleUpdate(IncludeFilesChanged | ContentChanged);
}

void FileModel::setDirectorySort(DirectorySort sort)
{
    if (m_directorySort == sort)
        return;

    m_directorySort = sort;
    scheduleUpdate(DirectorySortChanged | ContentChanged);
}

void FileModel::setNameFilters(const QStringList &filters)
{
    if (m_nameFilters == filters)
        return;

    m_nameFilters = filters;
    scheduleUpdate(NameFiltersChanged | ContentChanged);
}

void FileModel::setActive(bool active)
{
    if (m_active == active)
        return;

    m_active = active;
    scheduleUpdate(ActiveChanged | ContentChanged);
}

QString FileModel::appendPath(QString pathName)
{
    return QDir::cleanPath(QDir(m_path).absoluteFilePath(pathName));
}

QString FileModel::parentPath()
{
    return QDir::cleanPath(QDir(m_path).absoluteFilePath(".."));
}

QString FileModel::fileNameAt(int fileIndex)
{
    if (fileIndex < 0 || fileIndex >= m_files.count())
        return QString();

    return m_files.at(fileIndex).absoluteFilePath();
}

void FileModel::toggleSelectedFile(int fileIndex)
{
    if (!m_files.at(fileIndex).isSelected()) {
        StatFileInfo info = m_files.at(fileIndex);
        info.setSelected(true);
        m_files[fileIndex] = info;
        m_selectedCount++;
    } else {
        StatFileInfo info = m_files.at(fileIndex);
        info.setSelected(false);
        m_files[fileIndex] = info;
        m_selectedCount--;
    }
    // emit signal for views
    QModelIndex topLeft = index(fileIndex, 0);
    QModelIndex bottomRight = index(fileIndex, 0);
    emit dataChanged(topLeft, bottomRight);

    emit selectedCountChanged();
}

void FileModel::clearSelectedFiles()
{
    QMutableVectorIterator<StatFileInfo> iter(m_files);
    int row = 0;
    while (iter.hasNext()) {
        StatFileInfo &info = iter.next();
        info.setSelected(false);
        // emit signal for views
        QModelIndex topLeft = index(row, 0);
        QModelIndex bottomRight = index(row, 0);
        emit dataChanged(topLeft, bottomRight);
        row++;
    }
    m_selectedCount = 0;
    emit selectedCountChanged();
}

void FileModel::selectAllFiles()
{
    QMutableVectorIterator<StatFileInfo> iter(m_files);
    int row = 0;
    while (iter.hasNext()) {
        StatFileInfo &info = iter.next();
        info.setSelected(true);
        // emit signal for views
        QModelIndex topLeft = index(row, 0);
        QModelIndex bottomRight = index(row, 0);
        emit dataChanged(topLeft, bottomRight);
        row++;
    }
    m_selectedCount = m_files.count();
    emit selectedCountChanged();
}

QStringList FileModel::selectedFiles() const
{
    if (m_selectedCount == 0)
        return QStringList();

    QStringList fileNames;
    foreach (const StatFileInfo &info, m_files) {
        if (info.isSelected())
            fileNames.append(info.absoluteFilePath());
    }
    return fileNames;
}

void FileModel::refresh()
{
    if (!m_active) {
        m_dirty = true;
        return;
    }

    scheduleUpdate(ContentChanged);
}

void FileModel::refreshFull()
{
    if (!m_active) {
        m_dirty = true;
        return;
    }

    if (m_populated) {
        m_populated = false;
        emit populatedChanged();
    }
    scheduleUpdate();
}

void FileModel::readDirectory()
{
    // wrapped in reset model methods to get views notified
    beginResetModel();

    m_files.clear();
    if (!m_path.isEmpty())
        readAllEntries();

    endResetModel();

    recountSelectedFiles();

    m_populated = true;
    m_changedFlags |= (PopulatedChanged | CountChanged);
}

void FileModel::recountSelectedFiles()
{
    int count = 0;
    foreach (const StatFileInfo &info, m_files) {
        if (info.isSelected())
            count++;
    }
    if (m_selectedCount != count) {
        m_selectedCount = count;
        m_changedFlags |= SelectedCountChanged;
    }
}

void FileModel::readAllEntries()
{
    QDir dir(directory());
    if (!dir.exists()) {
        qmlInfo(this) << "Path " << dir.path() << " not found";
        return;
    }

    if (access(dir.path(), R_OK) == -1) {
        qmlInfo(this) << "No permissions to access " << dir.path();
        emit error(ErrorReadNoPermissions, dir.path());
        return;
    }

    m_absolutePath = dir.absolutePath();
    m_directory = dir.isRoot() ? QStringLiteral("/") : dir.dirName();
    m_parentPath = dir.isRoot() ? QString() : QDir::cleanPath(dir.absoluteFilePath(QStringLiteral("..")));
    m_files = directoryEntries(dir);
}

void FileModel::refreshEntries()
{
    int oldCount = m_files.count();

    if (m_path.isEmpty()) {
        clearModel();
    } else {
        QDir dir(directory());
        if (!dir.exists()) {
            clearModel();

            qmlInfo(this) << "Path " << dir.path() << " not found";
            return;
        }

        if (access(dir.path(), R_OK) == -1) {
            clearModel();
            qmlInfo(this) << "No permissions to access " << dir.path();
            emit error(ErrorReadNoPermissions, dir.path());
            return;
        }

        // read all files
        QVector<StatFileInfo> newFiles = directoryEntries(dir);
        ::synchronizeList(this, m_files, newFiles);
    }

    recountSelectedFiles();

    if (m_files.count() != oldCount) {
        m_changedFlags |= CountChanged;
    }
}

int FileModel::insertRange(int index, int count, const QVector<StatFileInfo> &source, int sourceIndex)
{
    m_files.reserve(m_files.count() + count);

    beginInsertRows(QModelIndex(), index, index + count - 1);

    for (int i = 0; i < count; ++i)
        m_files.insert(index + i, source.at(sourceIndex + i));

    endInsertRows();
    return count;
}

int FileModel::removeRange(int index, int count)
{
    beginRemoveRows(QModelIndex(), index, index + count - 1);

    QVector<StatFileInfo>::iterator it = m_files.begin() + index;
    m_files.erase(it, it + count);

    endRemoveRows();
    return 0;
}

void FileModel::clearModel()
{
    if (!m_files.isEmpty()) {
        beginResetModel();
        m_files.clear();
        endResetModel();
    }
}

QDir FileModel::directory() const
{
    QDir dir(m_path);
    if (dir.exists()) {
        QDir::Filters filters(QDir::NoDot | QDir::System);

        if (m_includeDirectories) {
            filters |= QDir::AllDirs;
            if (!m_includeParentDirectory || dir.isRoot()) {
                filters |= QDir::NoDotDot;
            }
        }

        if (m_includeHiddenFiles) {
            filters |= QDir::Hidden;
        }
        if (m_includeSystemFiles) {
            filters |= QDir::System;
        }

        if (m_includeFiles) {
            filters |= QDir::Files;
        }

        QDir::SortFlags sortFlags(QDir::LocaleAware);

        if (m_sortBy == SortByName) {
            sortFlags |= QDir::Name;
        } else if (m_sortBy == SortByModified) {
            sortFlags |= QDir::Time;
        } else if (m_sortBy == SortBySize) {
            sortFlags |= QDir::Size;
        } else if (m_sortBy == SortByExtension) {
            sortFlags |= QDir::Type;
        }

        if (m_sortOrder == Qt::DescendingOrder) {
            sortFlags |= QDir::Reversed;
        }
        if (m_caseSensitivity == Qt::CaseInsensitive) {
            sortFlags |= QDir::IgnoreCase;
        }

        if (m_directorySort == SortDirectoriesBeforeFiles) {
            sortFlags |= QDir::DirsFirst;
        } else if (m_directorySort == SortDirectoriesAfterFiles) {
            sortFlags |= QDir::DirsLast;
        }

        dir.setFilter(filters);
        dir.setSorting(sortFlags);

        if (!m_nameFilters.isEmpty()) {
            dir.setNameFilters(m_nameFilters);
        }
    }

    return dir;
}

void FileModel::scheduleUpdate(ChangedFlags flags)
{
    m_changedFlags |= flags;
    if (!m_timer.isActive()) {
        m_timer.start(0, this);
    }
}

void FileModel::update()
{
    if (!m_populated || (m_changedFlags & SortOrderChanged)) {
        // Do a complete refresh
        readDirectory();
    } else if (m_changedFlags & ContentChanged) {
        // Do an incremental update
        refreshEntries();
    }

    // Report any changes that have occurred
    if (m_changedFlags & PathChanged) {
        emit pathChanged();
    }
    if (m_changedFlags & SortByChanged) {
        emit sortByChanged();
    }
    if (m_changedFlags & SortOrderChanged) {
        emit sortOrderChanged();
    }
    if (m_changedFlags & CaseSensitivityChanged) {
        emit caseSensitivityChanged();
    }
    if (m_changedFlags & IncludeDirectoriesChanged) {
        emit includeDirectoriesChanged();
    }
    if (m_changedFlags & IncludeParentDirectoryChanged) {
        emit includeParentDirectoryChanged();
    }
    if (m_changedFlags & IncludeHiddenFilesChanged) {
        emit includeHiddenFilesChanged();
    }
    if (m_changedFlags & IncludeSystemFilesChanged) {
        emit includeSystemFilesChanged();
    }
    if (m_changedFlags & IncludeFilesChanged) {
        emit includeFilesChanged();
    }
    if (m_changedFlags & DirectorySortChanged) {
        emit directorySortChanged();
    }
    if (m_changedFlags & NameFiltersChanged) {
        emit nameFiltersChanged();
    }
    if (m_changedFlags & PopulatedChanged) {
        emit populatedChanged();
    }
    if (m_changedFlags & CountChanged) {
        emit countChanged();
    }
    if (m_changedFlags & ActiveChanged) {
        emit activeChanged();
    }
    if (m_changedFlags & SelectedCountChanged) {
        emit selectedCountChanged();
    }

    m_changedFlags = 0;
    m_dirty = false;
}

void FileModel::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();
        update();
    }
}

bool FileModel::hasFile(QString filename) const
{
    QDir dir(m_path);
    if (dir.exists() && dir.exists(filename))
        return true;
    return false;
}
