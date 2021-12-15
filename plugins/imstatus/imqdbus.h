/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef IMQDBUS_H
#define IMQDBUS_H

#include <QObject>
#include <QString>

#ifdef TELEPATHY_FOUND
#include <TelepathyQt/Types>

namespace Tp
{
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
    IMQDBus(QObject *parent = nullptr);
    ~IMQDBus();

    void updateStatusMessage(const QString &im, const QString &statusMessage);
    static QStringList scanForIMs();

private:
    void usePsi(const QString &statusMessage);
    void useKopete(const QString &statusMessage);
    void useSkype(const QString &statusMessage);
    void usePidgin(const QString &statusMessage);

#ifdef TELEPATHY_FOUND
private Q_SLOTS:
    void slotFinished(Tp::PendingOperation *po);

private:
    void useTelepathy(const QString &statusMessage);
    Tp::AccountManagerPtr m_accountManager;
#endif
};

#endif // IMQDBUS_H
