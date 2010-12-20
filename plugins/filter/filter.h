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

#ifndef FILTER_H
#define FILTER_H

#include <QObject>

class KConfigGroup;
class Filter : public QObject
{
    Q_OBJECT
public:
    enum FilterType{ Contain = 0, ExactMatch, RegExp, DoesNotContain };
    enum FilterField{ Content = 0, AuthorUsername, ReplyToUsername, Source };

    /**
        Just use this constructor when filter is new
    */
    explicit Filter(const QString &filterText, FilterField field = Content,
                    FilterType type = Contain, bool dontHide = false, QObject* parent = 0);
    explicit Filter( const KConfigGroup& config, QObject* parent = 0);
    virtual ~Filter();

    QString filterText() const;
    void setFilterText( const QString &text );

    FilterField filterField() const;
    void setFilterField( FilterField field );

    FilterType filterType() const;
    void setFilterType( FilterType type );

    bool dontHideReplies() const;
    void setDontHideReplies(bool dontHide);

    void writeConfig();

private:
    class Private;
    Private * const d;
};

#endif // FILTER_H
