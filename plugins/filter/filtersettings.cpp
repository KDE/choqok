/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include <QStringList>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include "filter.h"

FilterSettings *FilterSettings::_self = 0L;
QMap<Filter::FilterField, QString> FilterSettings::_filterFieldName;
QMap<Filter::FilterType, QString> FilterSettings::_filterTypeName;
QMap<Filter::FilterAction, QString> FilterSettings::_filterActionName;
bool FilterSettings::_hideNoneFriendsReplies = false;
bool FilterSettings::_hideRepliesNotRelatedToMe = false;

FilterSettings *FilterSettings::self()
{
    if (!_self) {
        _self = new FilterSettings;
        _filterFieldName[Filter::AuthorUsername] = i18n("Author Username");
        _filterFieldName[Filter::Content] = i18n("Post Text");
        _filterFieldName[Filter::Source] = i18n("Author Client");
        _filterFieldName[Filter::ReplyToUsername] = i18n("Reply to User");

        _filterTypeName[Filter::Contain] = i18n("Contain");
        _filterTypeName[Filter::DoesNotContain] = i18n("Does Not Contain");
        _filterTypeName[Filter::ExactMatch] = i18n("Exact Match");
        _filterTypeName[Filter::RegExp] = i18n("Regular Expression");

        _filterActionName[Filter::Remove] = i18nc("@action", "Hide Posts");
        _filterActionName[Filter::Highlight] = i18nc("@action", "Highlight Posts");
    }
    return _self;
}

FilterSettings::FilterSettings(): QObject()
{
    conf = new KConfigGroup(KSharedConfig::openConfig(), QLatin1String("Filter Plugin"));
    readConfig();
}

FilterSettings::~FilterSettings()
{

}

QList< Filter * > FilterSettings::filters() const
{
    return _filters;
}

void FilterSettings::readConfig()
{
    _filters.clear();
    //Filter group names are start with Filter_%Text%%Field%%Type%
    KSharedConfig::openConfig()->sync();
    QStringList groups = KSharedConfig::openConfig()->groupList();
    for (const QString &grp: groups) {
        if (grp.startsWith(QLatin1String("Filter_"))) {
            Filter *f = new Filter(KSharedConfig::openConfig()->group(grp), this);
            if (f->filterText().isEmpty()) {
                continue;
            }
            _filters << f;
            //qDebug() << "REEADING A FILTER";
        }
    }
    //qDebug() << filters().count();

    _hideNoneFriendsReplies = conf->readEntry("hideNoneFriendsReplies", false);
    _hideRepliesNotRelatedToMe = conf->readEntry("hideRepliesNotRelatedToMe", false);
}

void FilterSettings::setFilters(const QList< Filter * > &filters)
{
    _filters = filters;
}

void FilterSettings::writeConfig()
{
    QStringList groups = KSharedConfig::openConfig()->groupList();
    for (const QString &grp: groups) {
        if (grp.startsWith(QLatin1String("Filter_"))) {
            KSharedConfig::openConfig()->deleteGroup(grp);
        }
    }
    conf->writeEntry("hideNoneFriendsReplies", _hideNoneFriendsReplies);
    conf->writeEntry("hideRepliesNotRelatedToMe", _hideRepliesNotRelatedToMe);
    KSharedConfig::openConfig()->sync();

    for (Filter *f: _filters) {
        f->writeConfig();
    }

    readConfig();
}

QString FilterSettings::filterFieldName(Filter::FilterField field)
{
    return _filterFieldName.value(field);
}

QString FilterSettings::filterTypeName(Filter::FilterType type)
{
    return _filterTypeName.value(type);
}

Filter::FilterField FilterSettings::filterFieldFromName(const QString &name)
{
    return _filterFieldName.key(name);
}

Filter::FilterType FilterSettings::filterTypeFromName(const QString &name)
{
    return _filterTypeName.key(name);
}

Filter::FilterAction FilterSettings::filterActionFromName(const QString &name)
{
    return _filterActionName.key(name);
}

QString FilterSettings::filterActionName(Filter::FilterAction action)
{
    return _filterActionName.value(action);
}

QMap< Filter::FilterField, QString > FilterSettings::filterFieldsMap()
{
    return _filterFieldName;
}

QMap< Filter::FilterType, QString > FilterSettings::filterTypesMap()
{
    return _filterTypeName;
}

QMap< Filter::FilterAction, QString > FilterSettings::filterActionsMap()
{
    return _filterActionName;
}

bool FilterSettings::hideNoneFriendsReplies()
{
    return _hideNoneFriendsReplies;
}

void FilterSettings::setHideNoneFriendsReplies(bool enable)
{
    _hideNoneFriendsReplies = enable;
}

bool FilterSettings::hideRepliesNotRelatedToMe()
{
    return _hideRepliesNotRelatedToMe;
}

void FilterSettings::setHideRepliesNotRelatedToMe(bool enable)
{
    _hideRepliesNotRelatedToMe = enable;
}

