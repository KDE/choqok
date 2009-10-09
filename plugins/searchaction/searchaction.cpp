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

#include "searchaction.h"
#include <KGenericFactory>
#include <KAction>
#include "choqokuiglobal.h"
#include "twitterapihelper/twitterapimicroblog.h"
#include <twitterapihelper/twitterapiaccount.h>
#include <microblogwidget.h>
#include <KMessageBox>
#include <KActionCollection>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < SearchAction > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_untiny" ) )

SearchAction::SearchAction( QObject* parent, const QList< QVariant >& args )
    : Plugin(MyPluginFactory::componentData(), parent)
{
    KAction *action = new KAction(KIcon("edit-find"), i18n("Search..."), this);
    action->setShortcut(KShortcut(Qt::ControlModifier | Qt::Key_F));
    actionCollection()->addAction("search", action);
    connect(action, SIGNAL(triggered(bool)), SLOT(slotSearch()));
    setXMLFile("searchactionui.rc");
}

SearchAction::~SearchAction()
{

}

void SearchAction::slotSearch()
{
    TwitterApiAccount *curAccount = qobject_cast<TwitterApiAccount*>(Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentAccount());
    if(curAccount){
        TwitterApiMicroBlog *mBlog = qobject_cast<TwitterApiMicroBlog *>(curAccount->microblog());
        mBlog->showSearchDialog(curAccount);
    } else {
        KMessageBox::sorry(Choqok::UI::Global::mainWindow(),
                           i18n("The Search action plugin does not support the current microblog!"));
    }
}
