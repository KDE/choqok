/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2010-2011 Emanuele Bigiarini <pulmro@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/

*/

#ifndef DBUSHANDLER_H
#define DBUSHANDLER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QTextDocument>
#include <kio/job.h>
#include <choqok_export.h>


namespace Choqok
{
class DbusHandler;

CHOQOK_EXPORT Choqok::DbusHandler* ChoqokDbus();

class CHOQOK_EXPORT DbusHandler : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.choqok")

    /**
     * Retrieve the DBus handler instance if it exists or build up a new one
     */
    friend Choqok::DbusHandler* ChoqokDbus();

public:
    DbusHandler();
    ~DbusHandler();

public Q_SLOTS:
    /** Methods exported by the D-Bus interface org.choqok.kde:
     *   shareUrl: if you want to share an url with the html page title set bool title true;
     *   getShortening: return a bool for the active configuration of ShortenOnPaste option;
     *   setShortening: Control ShortenOnPaste option;
     */

    void shareUrl( const QString &url, bool title = false);
    void uploadFile( const QString &filename );
    void postText( const QString &text );
    void updateTimelines();
    void setShortening( bool flag);
    bool getShortening();

private:
    static DbusHandler *m_self;
    QString m_textToPost;
    QTextDocument m_doc;

    QString prepareUrl(const QString &url);

private Q_SLOTS:
    void slotcreatedQuickPost();
    void slotTitleUrl( KJob* job );

};

}

#endif // DBUSHANDLER_H
