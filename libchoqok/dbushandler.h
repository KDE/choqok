/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2010-2011 Emanuele Bigiarini <pulmro@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DBUSHANDLER_H
#define DBUSHANDLER_H

#include <QObject>
#include <QString>
#include <QTextDocument>
#include <KJob>

#include "choqok_export.h"

namespace Choqok
{
class DbusHandler;

CHOQOK_EXPORT Choqok::DbusHandler *ChoqokDbus();

class CHOQOK_EXPORT DbusHandler : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.choqok")

    /**
     * Retrieve the DBus handler instance if it exists or build up a new one
     */
    friend Choqok::DbusHandler *ChoqokDbus();

public:
    DbusHandler();
    ~DbusHandler();

public Q_SLOTS:
    /** Methods exported by the D-Bus interface org.choqok.kde:
     *   shareUrl: if you want to share an url with the html page title set bool title true;
     *   getShortening: return a bool for the active configuration of ShortenOnPaste option;
     *   setShortening: Control ShortenOnPaste option;
     */

    void shareUrl(const QString &url, bool title = false);
    void uploadFile(const QString &filename);
    void postText(const QString &text);
    void updateTimelines();
    void setShortening(bool flag);
    bool getShortening();

private:
    static DbusHandler *m_self;
    QString m_textToPost;
    QTextDocument m_doc;

    QString prepareUrl(const QString &url);

private Q_SLOTS:
    void slotcreatedQuickPost();
    void slotTitleUrl(KJob *job);

};

}

#endif // DBUSHANDLER_H
