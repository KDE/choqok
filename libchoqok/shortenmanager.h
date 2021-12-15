/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOKSHORTENMANAGER_H
#define CHOQOKSHORTENMANAGER_H

#include <QObject>
#include <QUrl>

#include "shortener.h"

namespace Choqok
{
namespace UI
{
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

    void emitNewUnshortenedUrl(Choqok::UI::PostWidget *widget, const QUrl &fromUrl, const QUrl &toUrl);

Q_SIGNALS:
    void newUnshortenedUrl(Choqok::UI::PostWidget *widget, const QUrl &fromUrl, const QUrl &toUrl);

private:
    ShortenManager(QObject *parent = nullptr);
    ~ShortenManager();
};

}
#endif
