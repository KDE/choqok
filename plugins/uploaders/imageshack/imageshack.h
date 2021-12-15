/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef IMAGESHACK_H
#define IMAGESHACK_H

#include "uploader.h"

#include <QMap>

/**
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/

class KJob;

class ImageShack : public Choqok::Uploader
{
    Q_OBJECT
public:
    ImageShack(QObject *parent, const QList< QVariant > &args);
    ~ImageShack();

    virtual void upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType) override;

protected Q_SLOTS:
    void slotUpload(KJob *job);

private:
    QMap<KJob *, QUrl> mUrlMap;
};

#endif
