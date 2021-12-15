/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

#ifndef TWITGOO_H
#define TWITGOO_H

#include "uploader.h"

#include <QMap>

class KJob;

class Twitgoo : public Choqok::Uploader
{
    Q_OBJECT
public:
    Twitgoo(QObject *parent, const QList< QVariant > &args);
    ~Twitgoo();

    virtual void upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType) override;

protected Q_SLOTS:
    void slotUpload(KJob *job);

private:
    QMap<KJob *, QUrl> mUrlMap;
};

#endif // TWITGOO_H
