/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

K_PLUGIN_CLASS_WITH_JSON(SearchAction, "choqok_searchaction.json")

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
      KMessageBox::error(Choqok::UI::Global::mainWindow(),
                         i18n("The Search action plugin does not support the "
                              "current microblog."));
    }
}

#include "searchaction.moc"
