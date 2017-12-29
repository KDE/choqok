/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "searchaction.h"

#include <QAction>

#include <KActionCollection>
#include <KMessageBox>
#include <KLocalizedString>
#include <KPluginFactory>

#include "choqokuiglobal.h"
#include "microblogwidget.h"

#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"

K_PLUGIN_FACTORY_WITH_JSON(SearchActionFactory, "choqok_searchaction.json",
                           registerPlugin < SearchAction > ();)

SearchAction::SearchAction(QObject *parent, const QList< QVariant > &)
    : Plugin(QLatin1String("choqok_searchaction"), parent)
{
    QAction *action = new QAction(QIcon::fromTheme(QLatin1String("edit-find")), i18n("Search..."), this);
    action->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_F));
    actionCollection()->addAction(QLatin1String("search"), action);
    connect(action, &QAction::triggered, this, &SearchAction::slotSearch);
    setXMLFile(QLatin1String("searchactionui.rc"));
}

SearchAction::~SearchAction()
{

}

void SearchAction::slotSearch()
{
    TwitterApiAccount *curAccount = qobject_cast<TwitterApiAccount *>(Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentAccount());
    if (curAccount) {
        TwitterApiMicroBlog *mBlog = qobject_cast<TwitterApiMicroBlog *>(curAccount->microblog());
        mBlog->showSearchDialog(curAccount);
    } else {
        KMessageBox::sorry(Choqok::UI::Global::mainWindow(),
                           i18n("The Search action plugin does not support the current microblog."));
    }
}

#include "searchaction.moc"
