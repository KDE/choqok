/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "twitterapicomposerwidget.h"
#include "twitterapitextedit.h"
#include <QtGui/QCompleter>
#include "twitterapiaccount.h"
#include <KDebug>
#include <choqokuiglobal.h>
#include <postwidget.h>
#include <choqokbehaviorsettings.h>
#include <QtGui/QStringListModel>

class TwitterApiComposerWidget::Private
{
public:
    Private()
    :model(0)
    {}
    QStringListModel *model;
};

TwitterApiComposerWidget::TwitterApiComposerWidget(Choqok::Account* account, QWidget* parent)
: Choqok::UI::ComposerWidget(account, parent), d(new Private)
{
    kDebug();
    d->model = new QStringListModel(qobject_cast<TwitterApiAccount*>(account)->friendsList(), this);
//     d->index = new QModelIndex(d->model->rowCount(), 0, 0, d->model);
//     kDebug()<<d->index;
    TwitterApiTextEdit *edit = new TwitterApiTextEdit(140, this);
    QCompleter *completer = new QCompleter(d->model, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    edit->setCompleter(completer);
    setEditor(edit);
    connect( Choqok::UI::Global::SessionManager::self(),
             SIGNAL(newPostWidgetAdded(Choqok::UI::PostWidget*,Choqok::Account*,QString)),
             SLOT(slotNewPostReady(Choqok::UI::PostWidget*,Choqok::Account*)) );
}

TwitterApiComposerWidget::~TwitterApiComposerWidget()
{
    delete d;
}

void TwitterApiComposerWidget::slotNewPostReady(Choqok::UI::PostWidget* widget, Choqok::Account* theAccount)
{
    if(theAccount == currentAccount()){
        int row = d->model->rowCount();
        d->model->insertRow(row);
        QString name = widget->currentPost()->author.userName;
        if( !name.isEmpty() && !d->model->stringList().contains(name) )
            d->model->setData(d->model->index(row), name);
    }
}

#include "twitterapicomposerwidget.moc"
