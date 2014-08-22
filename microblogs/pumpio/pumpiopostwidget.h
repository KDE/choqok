/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2013-2014 Andrea Scarpino <scarpino@kde.org>

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

#ifndef PUMPIOPOSTWIDGET_H
#define PUMPIOPOSTWIDGET_H

#include "postwidget.h"

class PumpIOPostWidget : public Choqok::UI::PostWidget
{
    Q_OBJECT
public:
    explicit PumpIOPostWidget(Choqok::Account* account, Choqok::Post* post, QWidget* parent = 0);
    virtual ~PumpIOPostWidget();

    virtual void checkAnchor(const QUrl& url);

    virtual QString generateSign();

    virtual void initUi();

protected Q_SLOTS:
    virtual void slotPostError(Choqok::Account* theAccount, Choqok::Post* post,
                               Choqok::MicroBlog::ErrorType error, const QString& errorMessage);

    virtual void slotResendPost();

    void slotReplyTo();

    void slotToggleFavorite(Choqok::Account*, Choqok::Post*);

    void toggleFavorite();

protected:
    virtual bool isResendAvailable();

    static const KIcon unFavIcon;

private:
    void updateFavStat();
    bool isReplyAvailable();

    class Private;
    Private * const d;

};

#endif // PUMPIOPOSTWIDGET_H