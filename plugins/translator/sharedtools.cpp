/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

