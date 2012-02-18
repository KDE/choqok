/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2010-2011 Ramin Gomari <ramin.gomari@gmail.com>

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

#ifndef NOWLISTENING_H
#define NOWLISTENING_H

#include <plugin.h>

class KConfigGroup;
/**
Now Listening

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
@author Ramin Gomari \<ramin.gomari@gmail.com\>
*/
class NowListening : public Choqok::Plugin
{
    Q_OBJECT
public:
    NowListening( QObject* parent, const QList< QVariant >& args );
    ~NowListening();

protected slots:
    void slotPrepareNowListening();

};

#endif //NOWLISTENING_H
