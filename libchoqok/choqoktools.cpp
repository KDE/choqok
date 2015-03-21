/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "choqoktools.h"

#include <KProcess>
#include <KToolInvocation>

#include <QtOAuth/qoauth_namespace.h>

#include "choqokbehaviorsettings.h"

void Choqok::openUrl(const QUrl &url)
{
    if (Choqok::BehaviorSettings::useCustomBrowser()) {
        QStringList args = Choqok::BehaviorSettings::customBrowser().split(' ');
        args.append(url.toString());
        if (KProcess::startDetached(args) == 0) {
            KToolInvocation::invokeBrowser(url.toString());
        }
    } else {
        KToolInvocation::invokeBrowser(url.toString());
    }
}

QString Choqok::qoauthErrorText(int code)
{
    switch (code) {
    case QOAuth::NoError:
        return i18n("No Error");
    case QOAuth::BadRequest:
        return i18n("Bad request");
    case QOAuth::ConsumerKeyEmpty:
    case QOAuth::ConsumerSecretEmpty:
        return i18n("Consumer Key or Secret has not been provided");
    case QOAuth::Forbidden:
        return i18n("Forbidden");
    case QOAuth::Timeout:
        return i18n("Timeout on server");
    case QOAuth::Unauthorized:
        return i18n("Authorization Error");
    case QOAuth::UnsupportedHttpMethod:
        return i18n("Internal Error");
    case QOAuth::OtherError:
    case QOAuth::RSADecodingError:
    case QOAuth::RSAKeyFileError:
    case QOAuth::RSAPrivateKeyEmpty:
        return i18n("Unknown Error");
    default:
        return QString();
    }
}

QString Choqok::getColorString(const QColor &color)
{
    return "rgb(" + QString::number(color.red()) + ',' + QString::number(color.green()) + ',' +
           QString::number(color.blue()) + ')';
}
