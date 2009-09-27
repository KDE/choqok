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

#include "twitterapisearchdialog.h"
#include "twitterapiaccount.h"
#include <kcombobox.h>
#include <klineedit.h>
#include <QGridLayout>
#include <QLabel>
#include <klocalizedstring.h>
#include "twitterapisearch.h"
#include "twitterapimicroblog.h"
#include <KDebug>

class TwitterApiSearchDialog::Private
{
public:
    Private(TwitterApiAccount* theAccount)
        :account(theAccount)
    {
        kDebug();
        mBlog = qobject_cast<TwitterApiMicroBlog*>(account->microblog());
        if(!mBlog)
            kError()<<"microblog is not a TwitterApiMicroBlog";
        Q_ASSERT(mBlog);
    }
    KComboBox *searchTypes;
    KLineEdit *searchQuery;
    TwitterApiAccount* account;
    TwitterApiMicroBlog *mBlog;
};

TwitterApiSearchDialog::TwitterApiSearchDialog(TwitterApiAccount* theAccount, QWidget* parent)
: KDialog(parent), d(new Private(theAccount))
{
    kDebug();
    setAttribute(Qt::WA_DeleteOnClose);
    createUi();
    d->searchQuery->setFocus();
    connect(d->searchTypes, SIGNAL(currentIndexChanged(int)), SLOT(slotSearchTypeChanged(int)) );
}

TwitterApiSearchDialog::~TwitterApiSearchDialog()
{
    delete d;
}

void TwitterApiSearchDialog::createUi()
{
    kDebug();
    QWidget *wd = new QWidget(this);
    setMainWidget(wd);
    QVBoxLayout *layout = new QVBoxLayout(wd);
    d->searchTypes = new KComboBox(wd);
    fillSearchTypes();
    kDebug();
    layout->addWidget(d->searchTypes);

    QHBoxLayout *queryLayout = new QHBoxLayout;
    layout->addLayout(queryLayout);
    QLabel *lblType = new QLabel(i18nc("Search query", "Query:"), wd);
    lblType->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    queryLayout->addWidget(lblType);

    d->searchQuery = new KLineEdit(this);
    queryLayout->addWidget(d->searchQuery);

    setButtonText( Ok, i18n("Search") );
}

void TwitterApiSearchDialog::fillSearchTypes()
{
    kDebug();
    QMap<int, QPair<QString, bool> > searchTypes = d->mBlog->searchBackend()->getSearchTypes();
    int count = searchTypes.count();
    for(int i = 0; i < count; ++i){
        d->searchTypes->insertItem(i, searchTypes[i].first);
    }
}

void TwitterApiSearchDialog::slotButtonClicked(int button)
{
    if(button == Ok) {
        bool isB = d->mBlog->searchBackend()->getSearchTypes()[d->searchTypes->currentIndex()].second;
        SearchInfo info(d->account, d->searchQuery->text(), d->searchTypes->currentIndex(), isB);
        d->mBlog->searchBackend()->requestSearchResults(info);
        accept();
    } else
        KDialog::slotButtonClicked(button);
}

void TwitterApiSearchDialog::slotSearchTypeChanged(int )
{
    d->searchQuery->setFocus();
}

#include "twitterapisearchdialog.moc"
