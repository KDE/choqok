/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef _CHOQOK_VERSION_H_
#define _CHOQOK_VERSION_H_

#define CHOQOK_VERSION_STRING "0.6.7"
#define CHOQOK_VERSION_MAJOR 0
#define CHOQOK_VERSION_MINOR 6
#define CHOQOK_VERSION_RELEASE 7
#define CHOQOK_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

#define CHOQOK_VERSION \
  CHOQOK_MAKE_VERSION(CHOQOK_VERSION_MAJOR,CHOQOK_VERSION_MINOR,CHOQOK_VERSION_RELEASE)

#define CHOQOK_IS_VERSION(a,b,c) ( CHOQOK_VERSION >= CHOQOK_MAKE_VERSION(a,b,c) )


#endif // _CHOQOK_VERSION_H_
