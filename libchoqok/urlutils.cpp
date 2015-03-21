/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
Copyright (C) 2014 Andrea Scarpino <scarpino@kde.org>

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
#include "urlutils.h"

#include <QRegExp>

const QString protocols = "((https?|ftps?)://)";
const QString subdomains = "(([a-z0-9\\-_]{1,}\\.)?)";
const QString auth = "(([a-z0-9\\-_]{1,})((:[\\S]{1,})?)@)";
const QString domains = "(([a-z0-9\\-\\x0080-\\xFFFF_]){1,63}\\.)+";
const QString port = "(:(6553[0-5]|655[0-2][0-9]|65[0-4][\\d]{2}|6[0-4][\\d]{3}|[1-5][\\d]{4}|[1-9][\\d]{0,3}))";
const QString zone("((a[cdefgilmnoqrstuwxz])|(b[abdefghijlmnorstvwyz])|(c[acdfghiklmnoruvxyz])|(d[ejkmoz])|(e[ceghrstu])|\
(f[ijkmor])|(g[abdefghilmnpqrstuwy])|(h[kmnrtu])|(i[delmnoqrst])|(j[emop])|(k[eghimnprwyz])|(l[abcikrstuvy])|\
(m[acdefghklmnopqrstuvwxyz])|(n[acefgilopruz])|(om)|(p[aefghklnrstwy])|(qa)|(r[eosuw])|(s[abcdeghijklmnortuvyz])|\
(t[cdfghjkmnoprtvwz])|(u[agksyz])|(v[aceginu])|(w[fs])|(ye)|(z[amrw])\
|(asia|com|info|net|org|biz|name|pro|aero|cat|coop|edu|jobs|mobi|museum|tel|travel|gov|int|mil|local|xxx)|(中国)|(公司)|(网络)|(صر)|(امارات)|(рф))");
const QString ip = "(25[0-5]|[2][0-4][0-9]|[0-1]?[\\d]{1,2})(\\.(25[0-5]|[2][0-4][0-9]|[0-1]?[\\d]{1,2})){3}";
const QString params = "(((\\/)[\\w:/\\?#\\[\\]@!\\$&\\(\\)\\*%\\+,;=\\._~\\x0080-\\xFFFF\\-\\|]{1,}|%[0-9a-f]{2})?)";
const QString excludingCharacters = QString::fromLatin1("[^\\s`!()\\[\\]{};:'\".,<>?%1%2%3%4%5%6]")
                                    .arg(QChar(0x00AB)).arg(QChar(0x00BB)).arg(QChar(0x201C)).arg(QChar(0x201D)).arg(QChar(0x2018)).arg(QChar(0x2019));

const QRegExp UrlUtils::mUrlRegExp("(((((" + protocols + auth + "?)?)" +
                                   subdomains +
                                   '(' + domains +
                                   zone + "(?!(\\w))))|(" + protocols + '(' + ip + ")+))" +
                                   '(' + port + "?)" + "((\\/)?)"  +
                                   params + ')' + excludingCharacters, Qt::CaseInsensitive);

const QRegExp UrlUtils::mEmailRegExp('^' + auth + subdomains + domains + zone);
const QString hrefTemplate("<a href='%1' title='%1'>%2</a>");

UrlUtils::UrlUtils()
{
}

UrlUtils::~UrlUtils()
{
}

QStringList UrlUtils::detectUrls(const QString &text)
{
    QStringList detectedUrls;

    int pos = 0;
    while (((pos = mUrlRegExp.indexIn(text, pos)) != -1)) {
        const QString link = mUrlRegExp.cap(0);
        if ((pos - 1 > -1 && (text.at(pos - 1) != '@' &&
                              text.at(pos - 1) != '#' && text.at(pos - 1) != '!')) ||
                (pos == 0)) {
            detectedUrls << link;
        }
        pos += link.length();
    }

    return detectedUrls;
}

QString UrlUtils::detectEmails(const QString &text)
{
    QString mailtoText(text);

    int pos = 0;
    while (((pos = mEmailRegExp.indexIn(mailtoText, pos)) != -1)) {
        QString link = mEmailRegExp.cap(0);
        QString tmplink = link;
        if ((pos - 1 > -1 && (mailtoText.at(pos - 1) != '@' &&
                              mailtoText.at(pos - 1) != '#' && mailtoText.at(pos - 1) != '!')) ||
                pos == 0) {
            tmplink.prepend("mailto:");
            mailtoText.remove(pos, link.length());
            tmplink = hrefTemplate.arg(tmplink, link);
            mailtoText.insert(pos, tmplink);
        }
        pos += tmplink.length();
    }

    return mailtoText;
}

