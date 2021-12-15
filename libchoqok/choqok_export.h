/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOK_EXPORT_H
#define CHOQOK_EXPORT_H

#ifndef CHOQOK_EXPORT
# if defined(MAKE_CHOQOK_LIB)
// We are building this library
# define CHOQOK_EXPORT Q_DECL_EXPORT
# else
// We are using this library
# define CHOQOK_EXPORT Q_DECL_IMPORT
# endif
#endif

#ifndef CHOQOK_HELPER_EXPORT
# if defined(MAKE_TWITTERAPIHELPER_LIB)
// We are building this library
# define CHOQOK_HELPER_EXPORT Q_DECL_EXPORT
# else
// We are using this library
# define CHOQOK_HELPER_EXPORT Q_DECL_IMPORT
# endif
#endif

#endif
