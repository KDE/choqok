/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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


#ifndef CHOQOKSHORTENMANAGER_H
#define CHOQOKSHORTENMANAGER_H

#include <QObject>

#include <KUrl>

#include "shortener.h"

namespace Choqok{
  namespace UI {
    class PostWidget;
  }

class ShortenManagerPrivate;
/**
Singleton class to manage URL shortening
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT ShortenManager: public QObject
{
    friend class ShortenManagerPrivate;
    Q_OBJECT
public:
    static ShortenManager *self();
    /**
        If there is any shorten plugin loaded/enabled return Shortened URL
        else return @p url
    */
    QString shortenUrl(const QString &url);

    /**
        Parse and find Urls and then shorten them. and return result text
    */
    QString parseText(const QString &text);

    /**
        Reload configurations.
        Should call after change on shortening plugin!
    */
    void reloadConfig();

    void emitNewUnshortenedUrl( Choqok::UI::PostWidget *widget, const KUrl& fromUrl, const KUrl& toUrl);

  Q_SIGNALS:
    void newUnshortenedUrl( Choqok::UI::PostWidget *widget, const KUrl& fromUrl, const KUrl& toUrl);

private:
    ShortenManager(QObject *parent=0);
    ~ShortenManager();
};

}
#endif
