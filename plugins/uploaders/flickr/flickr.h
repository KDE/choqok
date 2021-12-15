/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FLICKR_H
#define FLICKR_H

#include "uploader.h"

#include <QMap>

/**
@author Andrey Esin \<gmlastik@gmail.com\>
*/

class KJob;

class Flickr : public Choqok::Uploader
{
    Q_OBJECT
public:
    Flickr(QObject *parent, const QList< QVariant > &args);
    ~Flickr();

    virtual void upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType) override;
    QString base58encode(quint64);
    QByteArray createSign(QByteArray);

protected Q_SLOTS:
    void slotUpload(KJob *job);

private:
    QMap<KJob *, QUrl> mUrlMap;
};

#endif //FLICKR_H
