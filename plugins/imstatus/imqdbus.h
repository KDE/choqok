/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Andrey Esin <gmlastik@gmail.com>

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

#ifndef IMQDBUS_H
#define IMQDBUS_H

#include <QString>

#include <TelepathyQt/Types>

#include "config-imstatus.h"

#if TELEPATHY_FOUND
namespace Tp {
    class PendingOperation;
}
#endif

/**
  @author Andrey Esin \<gmlastik@gmail.com\>
*/

class IMQDBus : public QObject
{
    Q_OBJECT
public:
    IMQDBus (QObject *parent = 0);
    ~IMQDBus();

    void updateStatusMessage(const QString &im, const QString &statusMessage);
    static QStringList scanForIMs();

private:
    void usePsi(const QString &statusMessage);
    void useKopete(const QString &statusMessage);
    void useSkype(const QString &statusMessage);
    void usePidgin(const QString &statusMessage);

#if TELEPATHY_FOUND
private slots:
    void slotFinished(Tp::PendingOperation *po);

private:
    void useTelepathy(const QString &statusMessage);
    Tp::AccountManagerPtr m_accountManager;
#endif
};

#endif // IMQDBUS_H
