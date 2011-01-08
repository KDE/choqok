/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "choqokid.h"


ChoqokId::ChoqokId()
    :QString()
{

}

ChoqokId::ChoqokId(const QString& other )
    : QString(other)
{

}

ChoqokId::ChoqokId(const ChoqokId& other)
    : QString(other)
{

}

ChoqokId::ChoqokId(const QLatin1String& latin1)
    : QString(latin1)
{

}

ChoqokId::~ChoqokId()
{
}

bool ChoqokId::operator<(const ChoqokId& s) const
{
    int dif = length() - s.length();
    if(dif > 0){
        return false;
    } else if(dif < 0) {
        return true;
    }
    return QString::operator<(s);
}

bool ChoqokId::operator>(const ChoqokId& s) const
{
    int dif = length() - s.length();
    if(dif > 0){
        return true;
    } else if(dif < 0) {
        return false;
    }
    return QString::operator>(s);
}

QString ChoqokId::toString() const
{
    return QString(toLatin1());
}

// ChoqokId& ChoqokId::operator=(const QString& other)
// {
//     clear();
//     append(other);
//     return *this;
// }
