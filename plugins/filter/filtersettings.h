/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FILTERSETTINGS_H
#define FILTERSETTINGS_H

#include <QMap>
#include <QObject>

#include "filter.h"

class KConfigGroup;

class FilterSettings : public QObject
{
    Q_OBJECT
public:
    static FilterSettings *self();
    virtual ~FilterSettings();

    QList<Filter *> filters() const;
    void setFilters(const QList< Filter * > &filters);
    void writeConfig();
    void readConfig();

    static QMap<Filter::FilterField, QString> filterFieldsMap();
    static QMap<Filter::FilterType, QString> filterTypesMap();
    static QMap<Filter::FilterAction, QString> filterActionsMap();

    static QString filterFieldName(Filter::FilterField field);
    static Filter::FilterField filterFieldFromName(const QString &name);
    static QString filterTypeName(Filter::FilterType type);
    static Filter::FilterType filterTypeFromName(const QString &name);
    static QString filterActionName(Filter::FilterAction action);
    static Filter::FilterAction filterActionFromName(const QString &name);

    static bool hideNoneFriendsReplies();
    static void setHideNoneFriendsReplies(bool enable = true);
    static bool hideRepliesNotRelatedToMe();
    static void setHideRepliesNotRelatedToMe(bool enable = true);

private:
    FilterSettings();
    static FilterSettings *_self;

    QList<Filter *> _filters;
    static QMap<Filter::FilterField, QString> _filterFieldName;
    static QMap<Filter::FilterType, QString> _filterTypeName;
    static QMap<Filter::FilterAction, QString> _filterActionName;

    static bool _hideNoneFriendsReplies;
    static bool _hideRepliesNotRelatedToMe;

    KConfigGroup *conf;
};

#endif // FILTERSETTINGS_H
