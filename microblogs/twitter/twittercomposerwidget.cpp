/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "twittercomposerwidget.h"
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
#include "twittermicroblog.h"
#include <KDebug>
#include <choqoktextedit.h>

class TwitterComposerWidget::Private{
public:
    Private()
    :btnAttach(0), mediumName(0), btnCancel(0)
    {}
    QString mediumToAttach;
    KPushButton *btnAttach;
    QPointer<QLabel> mediumName;
    QPointer<KPushButton> btnCancel;
    QGridLayout *editorLayout;
};


TwitterComposerWidget::TwitterComposerWidget(Choqok::Account* account, QWidget* parent)
    : TwitterApiComposerWidget(account, parent), d(new Private)
{
    d->editorLayout = qobject_cast<QGridLayout*>(editorContainer()->layout());
    d->btnAttach = new KPushButton(editorContainer());
    d->btnAttach->setIcon(KIcon("mail-attachment"));
    d->btnAttach->setToolTip(i18n("Attach a file"));
    d->btnAttach->setMaximumWidth(d->btnAttach->height());
    connect(d->btnAttach, SIGNAL(clicked(bool)), this, SLOT(selectMediumToAttach()));
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(d->btnAttach);
    vLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
    d->editorLayout->addItem(vLayout, 0, 1, 1, 1);
}

TwitterComposerWidget::~TwitterComposerWidget()
{
    delete d;
}

void TwitterComposerWidget::submitPost(const QString& txt)
{
    if( d->mediumToAttach.isEmpty() ){
        Choqok::UI::ComposerWidget::submitPost(txt);
    } else {
        kDebug();
        editorContainer()->setEnabled(false);
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
        TwitterMicroBlog *mBlog = qobject_cast<TwitterMicroBlog*>(currentAccount()->microblog());
        mBlog->createPostWithAttachment( currentAccount(), postToSubmit(), d->mediumToAttach );
    }
}

void TwitterComposerWidget::slotPostMediaSubmitted(Choqok::Account* theAccount, Choqok::Post* post)
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
        editorContainer()->setEnabled(true);
        setPostToSubmit( 0L );
        cancelAttachMedium();
        currentAccount()->microblog()->updateTimelines(currentAccount());
    }
}

void TwitterComposerWidget::selectMediumToAttach()
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
        d->mediumName = new QLabel(editorContainer());
        d->btnCancel = new KPushButton(editorContainer());
        d->btnCancel->setIcon(KIcon("list-remove"));
        d->btnCancel->setToolTip(i18n("Discard Attachment"));
        d->btnCancel->setMaximumWidth(d->btnCancel->height());
        connect( d->btnCancel, SIGNAL(clicked(bool)), SLOT(cancelAttachMedium()) );

        d->editorLayout->addWidget(d->mediumName, 1, 0);
        d->editorLayout->addWidget(d->btnCancel, 1, 1);
    }
    d->mediumName->setText(i18n("Attaching <b>%1</b>", fileName));
    editor()->setFocus();
}

void TwitterComposerWidget::cancelAttachMedium()
{
    kDebug();
    delete d->mediumName;
    d->mediumName = 0;
    delete d->btnCancel;
    d->btnCancel = 0;
    d->mediumToAttach.clear();
}

#include "twittercomposerwidget.moc"
