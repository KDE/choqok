/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef INDICATORMANAGER_H
#define INDICATORMANAGER_H

#include <qindicateserver.h>
#include <qindicateindicator.h>

#include "account.h"
#include "choqok_export.h"

namespace Choqok
{

class CHOQOK_EXPORT MessageIndicatorManager : public QObject
{
    Q_OBJECT
public:
    static MessageIndicatorManager *self();
    ~MessageIndicatorManager();
    void newPostInc(int unread, const QString &alias, const QString &timeline);
    QIndicate::Server *iServer;
    QIndicate::Indicator *iIndicator;

private:
    MessageIndicatorManager();
    static MessageIndicatorManager *mSelf;
    QMap<QString, int> showList;
    QMap<QString, QIndicate::Indicator *> iList;
    QImage getIconByAlias(const QString &alias);

public Q_SLOTS:
    void slotDisplay(QIndicate::Indicator *);
    void slotShowMainWindow();
    void slotCanWorkWithAccs();
    void slotupdateUnreadCount(int change, int sum);
    void slotConfigChanged();
};
}
#endif // INDICATORMANAGER_H
