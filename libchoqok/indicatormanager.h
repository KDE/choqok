/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Andrey Esin <gmlastik@gmail.com>

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

#ifndef INDICATORMANAGER_H
#define INDICATORMANAGER_H

#include <KDE/KNotification>
#include <klocalizedstring.h>
#include "choqok_export.h"
#include <account.h>
#include <qindicateserver.h>
#include <qindicateindicator.h>

namespace Choqok
{

class CHOQOK_EXPORT MessageIndicatorManager : public QObject
{
    Q_OBJECT
public:
    static MessageIndicatorManager* self();
    ~MessageIndicatorManager();
    void newPostInc ( int unread, const QString& alias, const QString& timeline );
    QIndicate::Server *iServer;
    QIndicate::Indicator *iIndicator;

private:
    MessageIndicatorManager();
    static MessageIndicatorManager *mSelf;
    QMap<QString, int> showList;
    QMap<QString, QIndicate::Indicator *> iList;
    QImage getIconByAlias( const QString& alias );

public slots:
    void slotDisplay ( QIndicate::Indicator* );
    void slotShowMainWindow();
    void slotCanWorkWithAccs();
    void slotupdateUnreadCount(int change, int sum);
    void slotConfigChanged();
};
}
#endif // INDICATORMANAGER_H
