/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UPLOADER_H
#define UPLOADER_H

#include <QUrl>

#include "plugin.h"

namespace Choqok
{

/**
@brief The base class for Medium uploader plugins.

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT Uploader : public Plugin
{
    Q_OBJECT
public:
    virtual ~Uploader();

    /*virtual void upload( const QString &localUrl, const QByteArray &mediumType,
                            const QString &optionalMessage = QString() );*/
    virtual void upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType) = 0;

Q_SIGNALS:
    void mediumUploaded(const QUrl &localUrl, const QString &remoteUrl);
    void uploadingFailed(const QUrl &localUrl, const QString &errorMessage);

protected:
    Uploader(const QString &componentName, QObject *parent);
};

}

#endif // UPLOADER_H
