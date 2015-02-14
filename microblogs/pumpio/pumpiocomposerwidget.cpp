/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2013-2014 Andrea Scarpino <scarpino@kde.org>
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

#include "pumpiocomposerwidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>

#include <KDebug>
#include <KFileDialog>
#include <KLocalizedString>

#include "account.h"
#include "choqoktextedit.h"
#include "shortenmanager.h"

#include "pumpiomicroblog.h"
#include "pumpiopost.h"

class PumpIOComposerWidget::Private
{
public:
    QString mediumToAttach;
    KPushButton *btnAttach;
    QPointer<QLabel> mediumName;
    QPointer<KPushButton> btnCancel;
    QGridLayout *editorLayout;
    QString replyToObjectType;
};

PumpIOComposerWidget::PumpIOComposerWidget(Choqok::Account* account, QWidget* parent)
                                          : ComposerWidget(account, parent)
                                          , d(new Private)
{
    d->editorLayout = qobject_cast<QGridLayout*>(editorContainer()->layout());
    d->btnAttach = new KPushButton(editorContainer());
    d->btnAttach->setIcon(KIcon("mail-attachment"));
    d->btnAttach->setToolTip(i18n("Attach a file"));
    d->btnAttach->setMaximumWidth(d->btnAttach->height());
    connect(d->btnAttach, SIGNAL(clicked(bool)), this, SLOT(attachMedia()));
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(d->btnAttach);
    vLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
    d->editorLayout->addItem(vLayout, 0, 1);
}

PumpIOComposerWidget::~PumpIOComposerWidget()
{
    delete d;
}

void PumpIOComposerWidget::submitPost(const QString& text)
{
    kDebug();
    editorContainer()->setEnabled(false);
    QString txt = text;
    if (currentAccount()->postCharLimit() &&
        txt.size() > (int) currentAccount()->postCharLimit()) {
        txt = Choqok::ShortenManager::self()->parseText(txt);
    }
    setPostToSubmit(0L);
    setPostToSubmit(new Choqok::Post);
    postToSubmit()->content = txt;
    if (!replyToId.isEmpty()) {
        postToSubmit()->replyToPostId = replyToId;
    }
    connect(currentAccount()->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
            this, SLOT(slotPostSubmited(Choqok::Account*,Choqok::Post*)));
    connect(currentAccount()->microblog(),
            SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,
                             QString,Choqok::MicroBlog::ErrorLevel)), this,
            SLOT(slotErrorPost(Choqok::Account*,Choqok::Post*)));
    btnAbort = new KPushButton(KIcon("dialog-cancel"), i18n("Abort"), this);
    layout()->addWidget(btnAbort);
    connect( btnAbort, SIGNAL(clicked(bool)), SLOT(abort()) );

    PumpIOMicroBlog *mBlog = qobject_cast<PumpIOMicroBlog* >(currentAccount()->microblog());
    if (d->mediumToAttach.isEmpty()) {
        if (replyToId.isEmpty()) {
            currentAccount()->microblog()->createPost(currentAccount(), postToSubmit());
        } else {
            // WTF? It seems we cannot cast postToSubmit to PumpIOPost and then I'm copying its attributes
            PumpIOPost *pumpPost = new PumpIOPost();
            pumpPost->content = postToSubmit()->content;
            pumpPost->replyToPostId = postToSubmit()->replyToPostId;
            pumpPost->replyToObjectType = d->replyToObjectType;
            setPostToSubmit(pumpPost);

            mBlog->createReply(currentAccount(), pumpPost);
        }
    } else {
        mBlog->createPostWithMedia(currentAccount(), postToSubmit(), d->mediumToAttach);
    }
}

void PumpIOComposerWidget::slotPostSubmited(Choqok::Account* theAccount, Choqok::Post* post)
{
    kDebug();
    if( currentAccount() == theAccount && post == postToSubmit() ) {
        kDebug()<<"Accepted";
        disconnect(currentAccount()->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
                    this, SLOT(slotPostSubmited(Choqok::Account*,Choqok::Post*)) );
        disconnect(currentAccount()->microblog(),
                    SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,
                             QString,Choqok::MicroBlog::ErrorLevel)),
                    this, SLOT(slotErrorPost(Choqok::Account*,Choqok::Post*)));
        if(btnAbort){
            btnAbort->deleteLater();
        }
        editor()->clear();
        editorCleared();
        editorContainer()->setEnabled(true);
        setPostToSubmit(0L);
        cancelAttach();
        currentAccount()->microblog()->updateTimelines(currentAccount());
    }
}

void PumpIOComposerWidget::attachMedia()
{
    kDebug();
    d->mediumToAttach = KFileDialog::getOpenFileName(KUrl("kfiledialog:///image?global"),
                                                     QString(), this,
                                                     i18n("Select Media to Upload"));
    if (d->mediumToAttach.isEmpty()) {
        kDebug() << "No file selected";
        return;
    }
    const QString fileName = KUrl(d->mediumToAttach).fileName();
    if (!d->mediumName) {
        d->mediumName = new QLabel(editorContainer());
        d->btnCancel = new KPushButton(editorContainer());
        d->btnCancel->setIcon(KIcon("list-remove"));
        d->btnCancel->setToolTip(i18n("Discard Attachment"));
        d->btnCancel->setMaximumWidth(d->btnCancel->height());
        connect(d->btnCancel, SIGNAL(clicked(bool)), SLOT(cancelAttach()));

        d->editorLayout->addWidget(d->mediumName, 1, 0);
        d->editorLayout->addWidget(d->btnCancel, 1, 1);
    }
    d->mediumName->setText(i18n("Attaching <b>%1</b>", fileName));
    editor()->setFocus();
}

void PumpIOComposerWidget::cancelAttach()
{
    kDebug();
    delete d->mediumName;
    d->mediumName = 0;
    delete d->btnCancel;
    d->btnCancel = 0;
    d->mediumToAttach.clear();
}

void PumpIOComposerWidget::slotSetReply(const QString replyToId, const QString replyToUsername, const QString replyToObjectType)
{
    kDebug();
    this->replyToId = replyToId;
    this->replyToUsername = replyToUsername;
    d->replyToObjectType = replyToObjectType;

    if(!replyToUsername.isEmpty()){
        replyToUsernameLabel()->setText(i18n("Replying to <b>%1</b>", replyToUsername));
        btnCancelReply()->show();
        replyToUsernameLabel()->show();
    }
    editor()->setFocus();
}
