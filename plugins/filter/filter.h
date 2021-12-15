/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FILTER_H
#define FILTER_H

#include <QObject>

class KConfigGroup;
class Filter : public QObject
{
    Q_OBJECT
public:
    enum FilterType { Contain = 0, ExactMatch, RegExp, DoesNotContain };
    enum FilterField { Content = 0, AuthorUsername, ReplyToUsername, Source };
    enum FilterAction { None = 0, Remove, Highlight};

    /**
        Just use this constructor when filter is new
    */
    explicit Filter(const QString &filterText, FilterField field = Content,
                    FilterType type = Contain, FilterAction action = Remove,
                    bool dontHide = false, QObject *parent = nullptr);
    explicit Filter(const KConfigGroup &config, QObject *parent = nullptr);
    virtual ~Filter();

    QString filterText() const;
    void setFilterText(const QString &text);

    FilterField filterField() const;
    void setFilterField(FilterField field);

    FilterType filterType() const;
    void setFilterType(FilterType type);

    FilterAction filterAction() const;
    void setFilterAction(FilterAction action);

    bool dontHideReplies() const;
    void setDontHideReplies(bool dontHide);

    void writeConfig();

private:
    class Private;
    Private *const d;
};

#endif // FILTER_H
