/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "sharedtools.h"
#include <KLocalizedString>
#include <kstandarddirs.h>
#include <QFile>

SharedTools* SharedTools::_self = 0;

SharedTools::SharedTools()
{
    _missingLangs.insert("zh-CN", i18nc("Translation Language", "Chinese Simplified"));
    _missingLangs.insert("zh-TW", i18nc("Translation Language", "Chinese Traditional"));
    _missingLangs.insert("tl", i18nc("Translation Language", "Filipino"));
    _missingLangs.insert("ht", i18nc("Translation Language", "Haitian Creole"));
    _missingLangs.insert("iw", i18nc("Translation Language", "Hebrew"));
    _missingLangs.insert("no", i18nc("Translation Language", "Norwegian"));
    
    _languageCodes  << "af" << "sq" << "ar" << "be" << "bg" << "ca" << "zh-CN" << "zh-TW" << "hr" << "cs" << "da" << "nl" << "en" << "et" << "tl" << "fi" << "fr" << "gl" << "de" << "el" << "ht" << "iw" << "hi" << "hu" << "is" << "id" << "ga" << "it" << "ja" << "lv" << "lt" << "mk" << "ms" << "mt" << "no" << "fa" << "pl" << "pt" << "ro" << "ru" << "sr" << "sk" << "sl" << "es" << "sw" << "sv" << "th" << "tr" << "uk" << "vi" << "cy" << "yi";
}

SharedTools::~SharedTools()
{

}

SharedTools* SharedTools::self()
{
    if(!_self)
        _self = new SharedTools;
    return _self;
}

QMap< QString, QString > SharedTools::missingLangs() const
{
    return _missingLangs;
}

QStringList SharedTools::languageCodes() const
{
    return _languageCodes;
}

QString SharedTools::languageFlag(const QString& languageCode) const
{
    return KStandardDirs::locate( "locale", QString( "l10n/%1/flag.png" ).arg( languageCode.toLower() ) );
}

#include "sharedtools.moc"
