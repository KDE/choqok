/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef SHORTENER_H
#define SHORTENER_H

#include "plugin.h"

class QWidget;

namespace Choqok{
/**
@brief The base class for a Shortener plugin main class.

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT Shortener : public Plugin
{
Q_OBJECT
public:
    virtual ~Shortener();
    /**
        Shorten the @p url and return the shortened URL
    */
    virtual QString shorten( const QString &url );

protected:
    Shortener( const KComponentData &instance, QObject *parent );
};
}//End Namespace Choqok
#endif
