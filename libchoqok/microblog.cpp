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

#include "microblog.h"
#include <klocalizedstring.h>
#include <KDebug>
#include "account.h"
#include <QTimer>

namespace Choqok
{
class MicroBlog::Private
{
public:
    QString serviceName;
//     int error;
//     QString errorString;
    Capabilities capabilities;
    QString homepage;
    uint charLimit;
    QStringList timelineTypes;
    Account *account;
};

MicroBlog::MicroBlog( const KComponentData &instance, QObject *parent )
    :Plugin(instance, parent), d(new Private)
{
    kDebug();
}

MicroBlog::~MicroBlog()
{
    kDebug();
    delete d;
}

QString MicroBlog::serviceName() const
{
    return d->serviceName;
}

QString MicroBlog::homepageUrl() const
{
    return d->homepage;
}

QString MicroBlog::errorString( ErrorType type )
{
    switch(type){
        case ServerError:
            return i18n("Server returned error.");
            break;
        case CommunicationError:
            return i18n("Error on communication with server.");
            break;
        case ParsingError:
            return i18n("Error on parsing results");
            break;
        case AuthenticationError:
            return i18n("Authentication error.");
            break;
        case NotSupportedError:
            return i18n("Server do not support this feature.");
            break;
        case OtherError:
            return i18n("Unknown error.");
            break;
    };
    return QString();
}

void MicroBlog::setCapabilities( MicroBlog::Capabilities cap )
{
    d->capabilities = cap;
}

void MicroBlog::setServiceName(const QString& serviceName)
{
    d->serviceName = serviceName;
}

void MicroBlog::setServiceHomepageUrl(const QString& homepage)
{
    d->homepage = homepage;
}

uint MicroBlog::postCharLimit() const
{
    return d->charLimit;
}

void MicroBlog::setCharLimit(uint limit)
{
    d->charLimit = limit;
}

QStringList MicroBlog::timelineTypes() const
{
    return d->timelineTypes;
}
void MicroBlog::setTimelineTypes(const QStringList &types)
{
    d->timelineTypes = types;
}

bool MicroBlog::isValidTimeline( const QString &timeline )
{
    return d->timelineTypes.contains( timeline );
}

/*
void MicroBlog::aboutToUnload()
{
    kDebug();
    emit saveTimelines();
    QTimer::singleShot(0, this, SIGNAL( readyForUnload() ));
}
*/

}