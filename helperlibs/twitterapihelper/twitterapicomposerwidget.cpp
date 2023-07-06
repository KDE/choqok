/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "twitterapicomposerwidget.h"

#include <QCompleter>
#include <QStringListModel>

#include "choqokbehaviorsettings.h"
#include "choqokuiglobal.h"
#include "postwidget.h"

#include "twitterapiaccount.h"
#include "twitterapidebug.h"
#include "twitterapitextedit.h"

class TwitterApiComposerWidget::Private
{
public:
    Private()
        : model(nullptr)
    {}
    QStringListModel *model;
};

TwitterApiComposerWidget::TwitterApiComposerWidget(Choqok::Account *account, QWidget *parent)
    : Choqok::UI::ComposerWidget(account, parent), d(new Private)
{
    qCDebug(CHOQOK);
    d->model = new QStringListModel(qobject_cast<TwitterApiAccount *>(account)->friendsList(), this);
//     d->index = new QModelIndex(d->model->rowCount(), 0, 0, d->model);
//     qCDebug(CHOQOK)<<d->index;
    TwitterApiTextEdit *edit = new TwitterApiTextEdit(account, this);
    QCompleter *completer = new QCompleter(d->model, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    edit->setCompleter(completer);
    setEditor(edit);
    connect(Choqok::UI::Global::SessionManager::self(), &Choqok::UI::Global::SessionManager::newPostWidgetAdded,
            this, &TwitterApiComposerWidget::slotNewPostReady);
}

TwitterApiComposerWidget::~TwitterApiComposerWidget()
{
    delete d;
}

void TwitterApiComposerWidget::slotNewPostReady(Choqok::UI::PostWidget *widget, Choqok::Account *theAccount)
{
    if (theAccount == currentAccount()) {
        QString name = widget->currentPost()->author.userName;
        if (!name.isEmpty() && !d->model->stringList().contains(name)) {
            int row = d->model->rowCount();
            d->model->insertRow(row);
            d->model->setData(d->model->index(row), name);
        }
    }
}

#include "moc_twitterapicomposerwidget.cpp"
