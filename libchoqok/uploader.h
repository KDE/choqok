/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef UPLOADER_H
#define UPLOADER_H

#include <KDE/KUrl>
#include "plugin.h"

namespace Choqok {

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
                            const QString &optionalMessage = QString() )*/;
    virtual void upload( const KUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType) = 0;

Q_SIGNALS:
    void mediumUploaded( const KUrl &localUrl, const QString &remoteUrl );
    void uploadingFailed( const KUrl &localUrl, const QString &errorMessage );

protected:
    Uploader( const KComponentData &instance, QObject *parent );
};

}

#endif // UPLOADER_H
