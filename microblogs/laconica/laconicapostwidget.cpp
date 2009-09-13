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

#include "laconicapostwidget.h"
#include <twitterapihelper/twitterapiaccount.h>
#include <KDebug>

const QRegExp LaconicaPostWidget::mGroupRegExp("([\\s]|^)!([^\\s\\W]+)");

LaconicaPostWidget::LaconicaPostWidget(Choqok::Account* account, const Choqok::Post& post, QWidget* parent): TwitterApiPostWidget(account, post, parent)
{

}

// void LaconicaPostWidget::initUi()
// {
//     TwitterApiPostWidget::initUi();
// }

QString LaconicaPostWidget::prepareStatus(const QString& text)
{
    QString res = TwitterApiPostWidget::prepareStatus(text);
    QString homepage = qobject_cast<TwitterApiAccount*>(currentAccount())->homepageUrl().prettyUrl(KUrl::RemoveTrailingSlash);
    res.replace(mGroupRegExp,"\\1!<a href='group://\\2'>\\2</a> <a href='"+ homepage +
    "/group/\\2'>"+ webIconText +"</a>");
    res.replace(mHashtagRegExp,"\\1#<a href='tag://\\2'>\\2</a> <a href='"+ homepage +
    "/tag/\\1'>"+ webIconText +"</a>");
    return res;
}

void LaconicaPostWidget::checkAnchor(const QUrl& url)
{
    TwitterApiPostWidget::checkAnchor(url);
}

