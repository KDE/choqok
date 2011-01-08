/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef FILTERSETTINGS_H
#define FILTERSETTINGS_H

#include <QObject>
#include <QMap>
#include "filter.h"
#include <KConfigSkeleton>

class FilterSettings : public QObject
{
    Q_OBJECT
public:
    static FilterSettings *self();
    virtual ~FilterSettings();

    QList<Filter*> filters() const;
    void setFilters(const QList< Filter* >& filters);
    void writeConfig();
    void readConfig();

    static QMap<Filter::FilterField, QString> filterFieldsMap();
    static QMap<Filter::FilterType, QString> filterTypesMap();

    static QString filterFieldName(Filter::FilterField field);
    static Filter::FilterField filterFieldFromName( const QString &name );
    static QString filterTypeName(Filter::FilterType type);
    static Filter::FilterType filterTypeFromName( const QString &name );

    static bool hideNoneFriendsReplies();
    static void setHideNoneFriendsReplies(bool enable = true);
    static bool hideRepliesNotRelatedToMe();
    static void setHideRepliesNotRelatedToMe(bool enable = true);

private:
    FilterSettings();
    static FilterSettings *_self;

    QList<Filter*> _filters;
    static QMap<Filter::FilterField, QString> _filterFieldName;
    static QMap<Filter::FilterType, QString> _filterTypeName;

    static bool _hideNoneFriendsReplies;
    static bool _hideRepliesNotRelatedToMe;

    KConfigGroup *conf;
};

#endif // FILTERSETTINGS_H
