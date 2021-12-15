/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef POSTEROUS_H
#define POSTEROUS_H

#include "uploader.h"

#include <QMap>

class KJob;

class Posterous : public Choqok::Uploader
{
    Q_OBJECT
public:
    Posterous(QObject *parent, const QList< QVariant > &args);
    ~Posterous();

    virtual void upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType) override;
    QString getAuthToken(const QUrl &localUrl);
protected Q_SLOTS:
    void slotUpload(KJob *job);

private:
    QMap<KJob *, QUrl> mUrlMap;
};

#endif // POSTEROUS_H
