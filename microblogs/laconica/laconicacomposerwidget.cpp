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

#include "laconicacomposerwidget.h"
#include <KPushButton>
#include <QLabel>
#include <KFileDialog>
#include <klocalizedstring.h>
#include <QPointer>
#include <qlayout.h>
#include <QHBoxLayout>
#include <shortenmanager.h>
#include <notifymanager.h>
#include <account.h>
#include <microblog.h>
#include "laconicamicroblog.h"
#include <KDebug>
#include <choqoktextedit.h>

class LaconicaComposerWidget::Private{
public:
    Private()
    :btnAttach(0), mediumName(0), btnCancel(0), mediumLayout(0)
    {}
    QString mediumToAttach;
    KPushButton *btnAttach;
    QPointer<QLabel> mediumName;
    QPointer<KPushButton> btnCancel;
    QHBoxLayout *mediumLayout;
};


LaconicaComposerWidget::LaconicaComposerWidget(Choqok::Account* account, QWidget* parent)
    : ComposerWidget(account, parent), d(new Private)
{
    d->btnAttach = new KPushButton(this);
    d->btnAttach->setIcon(KIcon("mail-attachment"));
    d->btnAttach->setToolTip(i18n("Attach a file"));
    d->btnAttach->setMaximumWidth(d->btnAttach->height());
    connect(d->btnAttach, SIGNAL(clicked(bool)), this, SLOT(selectMediumToAttach()));
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(d->btnAttach);
    vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
    editorLayout()->addItem(vLayout);
}

LaconicaComposerWidget::~LaconicaComposerWidget()
{

}

void LaconicaComposerWidget::submitPost(const QString& txt)
{
    if( d->mediumToAttach.isEmpty() ){
        Choqok::UI::ComposerWidget::submitPost(txt);
    } else {
        kDebug();
        editorLayout()->setEnabled(false);
        QString text = txt;
        if( currentAccount()->microblog()->postCharLimit() &&
            text.size() > (int)currentAccount()->microblog()->postCharLimit() )
            text = Choqok::ShortenManager::self()->parseText(text);
        setPostToSubmit(0L);
        setPostToSubmit( new Choqok::Post );
        postToSubmit()->content = text;
        if( !replyToId.isEmpty() ) {
            postToSubmit()->replyToPostId = replyToId;
        }
        connect( currentAccount()->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
                SLOT(slotPostMediaSubmitted(Choqok::Account*,Choqok::Post*)) );
        connect(currentAccount()->microblog(),
                SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,
                                         QString,Choqok::MicroBlog::ErrorLevel)),
                SLOT(slotErrorPost(Choqok::Account*,Choqok::Post*)));
        btnAbort = new KPushButton(KIcon("dialog-cancel"), i18n("Abort"), this);
        layout()->addWidget(btnAbort);
        connect( btnAbort, SIGNAL(clicked(bool)), SLOT(abort()) );
        LaconicaMicroBlog *mBlog = qobject_cast<LaconicaMicroBlog*>(currentAccount()->microblog());
        mBlog->createPostWithAttachment( currentAccount(), postToSubmit(), d->mediumToAttach );
    }
}

void LaconicaComposerWidget::slotPostMediaSubmitted(Choqok::Account* theAccount, Choqok::Post* post)
{
    kDebug();
    if( currentAccount() == theAccount && post == postToSubmit() ) {
        kDebug()<<"Accepted";
        disconnect(currentAccount()->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
                   this, SLOT(slotPostMediaSubmitted(Choqok::Account*,Choqok::Post*)) );
        disconnect(currentAccount()->microblog(),
                    SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,
                                    QString,Choqok::MicroBlog::ErrorLevel)),
                    this, SLOT(slotErrorPost(Choqok::Account*,Choqok::Post*)));
        if(btnAbort){
            btnAbort->deleteLater();
        }
        Choqok::NotifyManager::success(i18n("New post submitted successfully"));
        editor()->clear();
        replyToId.clear();
        editorLayout()->setEnabled(true);
        setPostToSubmit( 0L );
        cancelAttachMedium();
        currentAccount()->microblog()->updateTimelines(currentAccount());
    }
}

void LaconicaComposerWidget::selectMediumToAttach()
{
    kDebug();
    d->mediumToAttach = KFileDialog::getOpenFileName( KUrl("kfiledialog:///image?global"),
                                                      QString(), this,
                                                      i18n("Select Media to Upload") );
    if( d->mediumToAttach.isEmpty() )
        return;
    QString fileName = KUrl(d->mediumToAttach).fileName();
    if( !d->mediumName ){
        kDebug()<<fileName;
        d->mediumLayout = new QHBoxLayout;
        d->mediumName = new QLabel(this);
        d->btnCancel = new KPushButton(this);
        d->btnCancel->setIcon(KIcon("list-remove"));
        d->btnCancel->setToolTip(i18n("Discard"));
        d->btnCancel->setMaximumWidth(d->btnCancel->height());
        connect( d->btnCancel, SIGNAL(clicked(bool)), SLOT(cancelAttachMedium()) );

        d->mediumLayout->addWidget(d->mediumName);
        d->mediumLayout->addWidget(d->btnCancel);
        qobject_cast<QVBoxLayout*>(layout())->addLayout(d->mediumLayout);
    }
    d->mediumName->setText(fileName);
}

void LaconicaComposerWidget::cancelAttachMedium()
{
    kDebug();
    delete d->mediumName;
    delete d->mediumLayout;
    delete d->btnCancel;
    d->mediumToAttach.clear();
}

#include "laconicacomposerwidget.moc"
