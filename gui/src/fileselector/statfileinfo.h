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

#ifndef STATFILEINFO_H
#define STATFILEINFO_H

#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <sys/stat.h>

/**
 * @brief The StatFileInfo class is like QFileInfo, but has more detailed information about file types.
 */
class StatFileInfo
{
public:
    explicit StatFileInfo();
    explicit StatFileInfo(QString fileName);
    ~StatFileInfo();

    void setFile(QString fileName);
    QString fileName() const { return m_fileInfo.fileName(); }

    // these inspect the file itself without following symlinks

    // directory
    bool isDir() const { return S_ISDIR(m_lstat.st_mode); }
    // symbolic link
    bool isSymLink() const { return S_ISLNK(m_lstat.st_mode); }
    // block special file
    bool isBlk() const { return S_ISBLK(m_lstat.st_mode); }
    // character special file
    bool isChr() const { return S_ISCHR(m_lstat.st_mode); }
    // pipe of FIFO special file
    bool isFifo() const { return S_ISFIFO(m_lstat.st_mode); }
    // socket
    bool isSocket() const { return S_ISSOCK(m_lstat.st_mode); }
    // regular file
    bool isFile() const { return S_ISREG(m_lstat.st_mode); }
    // system file (not a dir, regular file or symlink)
    bool isSystem() const { return !S_ISDIR(m_lstat.st_mode) && !S_ISREG(m_lstat.st_mode) &&
                                   !S_ISLNK(m_lstat.st_mode); }

    // these inspect the file or if it is a symlink, then its target end point

    // directory
    bool isDirAtEnd() const { return S_ISDIR(m_stat.st_mode); }
    // block special file
    bool isBlkAtEnd() const { return S_ISBLK(m_stat.st_mode); }
    // character special file
    bool isChrAtEnd() const { return S_ISCHR(m_stat.st_mode); }
    // pipe of FIFO special file
    bool isFifoAtEnd() const { return S_ISFIFO(m_stat.st_mode); }
    // socket
    bool isSocketAtEnd() const { return S_ISSOCK(m_stat.st_mode); }
    // regular file
    bool isFileAtEnd() const { return S_ISREG(m_stat.st_mode); }
    // system file (not a dir or regular file)
    bool isSystemAtEnd() const { return !S_ISDIR(m_stat.st_mode) && !S_ISREG(m_stat.st_mode); }

    // these inspect the file or if it is a symlink, then its target end point

    QFile::Permissions permissions() const { return m_fileInfo.permissions(); }
    QString group() const { return m_fileInfo.group(); }
    uint groupId() const { return m_fileInfo.groupId(); }
    QString owner() const { return m_fileInfo.owner(); }
    uint ownerId() const { return m_fileInfo.ownerId(); }
    qint64 size() const { return m_fileInfo.size(); }
    QDateTime lastModified() const { return m_fileInfo.lastModified(); }
    QDateTime lastAccessed() const { return m_fileInfo.lastRead(); }
    QDateTime created() const { return m_fileInfo.created(); }
    QString extension() const;
    QString baseName() const;
    bool exists() const;
    bool isSafeToRead() const;

    // path accessors

    QDir absoluteDir() const { return m_fileInfo.absoluteDir(); }
    QString absolutePath() const { return m_fileInfo.absolutePath(); }
    QString absoluteFilePath() const { return m_fileInfo.absoluteFilePath(); }
    QString suffix() const { return m_fileInfo.suffix(); }
    QString symLinkTarget() const { return m_fileInfo.symLinkTarget(); }
    bool isSymLinkBroken() const;

    // selection
    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }

    void refresh();

private:
    QString m_fileName;
    QFileInfo m_fileInfo;
    struct stat m_stat; // after following possible symlinks
    struct stat m_lstat; // file itself without following symlinks
    bool m_selected;
};

bool operator==(const StatFileInfo &lhs, const StatFileInfo &rhs);
bool operator!=(const StatFileInfo &lhs, const StatFileInfo &rhs);

#endif // STATFILEINFO_H
