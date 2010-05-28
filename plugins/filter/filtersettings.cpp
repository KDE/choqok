/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "filtersettings.h"
#include <QApplication>
#include "filter.h"
#include <KGlobal>
#include <ksharedptr.h>
#include <KSharedConfig>
#include <QStringList>
#include <KLocalizedString>

FilterSettings *FilterSettings::_self = new FilterSettings;

FilterSettings* FilterSettings::self()
{
    return _self;
}

FilterSettings::FilterSettings(): QObject(qApp)
{
    reloadFilters();

    _filterFieldName[Filter::AuthorUsername] = i18n("Author Username");
    _filterFieldName[Filter::Content] = i18n("Post Text");
    _filterFieldName[Filter::Source] = i18n("Author Client");
    _filterFieldName[Filter::ReplyToUsername] = i18n("Reply to User");

    _filterTypeName[Filter::Contain] = i18n("Contain");
    _filterTypeName[Filter::ExactMatch] = i18n("Exact Match");
    _filterTypeName[Filter::RegExp] = i18n("Regular Expression");
}

FilterSettings::~FilterSettings()
{

}

QList< Filter* > FilterSettings::availableFilters() const
{
    return _filters;
}

void FilterSettings::reloadFilters()
{
    _filters.clear();
    //Filter group names are start with Filter_%Text%%Field%%Type%
    QStringList groups = KGlobal::config()->groupList();
    foreach(const QString &grp, groups){
        if(grp.startsWith("Filter_")){
            Filter *f = new Filter(KGlobal::config()->group(grp), this);
            _filters << f;
        }
    }
}

void FilterSettings::setFilters(QList< Filter* > filters)
{
    _filters = filters;
}

void FilterSettings::saveFilters()
{
    QStringList groups = KGlobal::config()->groupList();
    foreach(const QString &grp, groups){
        if(grp.startsWith("Filter_")){
            KGlobal::config()->deleteGroup(grp);
        }
    }

    foreach(Filter *f, _filters){
        f->writeConfig();
    }
}

QMap< Filter::FilterField, QString > FilterSettings::filterFieldName()
{
    return _filterFieldName;
}

QMap< Filter::FilterType, QString > FilterSettings::filterTypeName()
{
    return _filterTypeName;
}

