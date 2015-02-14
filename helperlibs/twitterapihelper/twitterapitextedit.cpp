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

#include "twitterapitextedit.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QApplication>
#include <QCompleter>
#include <QKeyEvent>
#include <QLabel>
#include <QModelIndex>
#include <QScrollBar>

#include <KIO/Job>
#include <KDebug>

#include <QtOAuth/qoauth_namespace.h>

#include <qjson/parser.h>

#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"
#include "urlutils.h"

class TwitterApiTextEdit::Private
{
public:
    Private(Choqok::Account *theAccount)
    :c(0), acc(theAccount), tCoMaximumLength(0), tCoMaximumLengthHttps(0)
    {}
    Choqok::Account *acc;
    QCompleter *c;
    int tCoMaximumLength;
    int tCoMaximumLengthHttps;
};

TwitterApiTextEdit::TwitterApiTextEdit(Choqok::Account* theAccount, QWidget* parent)
: TextEdit(theAccount->postCharLimit(), parent), d(new Private(theAccount))
{
    kDebug();
    setTabChangesFocus(false);
    fetchTCoMaximumLength();
}

TwitterApiTextEdit::~TwitterApiTextEdit()
{
    delete d;
}

void TwitterApiTextEdit::setCompleter(QCompleter *completer)
{
    if (d->c)
        QObject::disconnect(d->c, 0, this, 0);

    d->c = completer;

    if (!d->c)
        return;

    d->c->setWidget(this);
    d->c->setCompletionMode(QCompleter::PopupCompletion);
    d->c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(d->c, SIGNAL(activated(const QString&)),
                     this, SLOT(insertCompletion(const QString&)));
}

QCompleter *TwitterApiTextEdit::completer() const
{
    return d->c;
}

void TwitterApiTextEdit::insertCompletion(const QString& completion)
{
    if (d->c->widget() != this)
        return;
    QString textToInsert = completion + ' ';
    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::EndOfWord);
    tc.select(QTextCursor::WordUnderCursor);
    bool startWithAt;
    if(QString(qVersion()) >= "4.7.0")
        startWithAt = toPlainText()[tc.selectionStart()-1] != '@';
    else
        startWithAt = !completion.startsWith('@');
    if(startWithAt)
        textToInsert.prepend('@');
    tc.insertText(textToInsert);
    setTextCursor(tc);
}

// QString TwitterApiTextEdit::textUnderCursor() const
// {
//     QTextCursor tc = textCursor();
//     tc.select(QTextCursor::WordUnderCursor);
//     return tc.selectedText();
// }

void TwitterApiTextEdit::focusInEvent(QFocusEvent *e)
{
    if (d->c)
        d->c->setWidget(this);
    KTextEdit::focusInEvent(e);
}

void TwitterApiTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (d->c && d->c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
//             case Qt::Key_Backtab:
                e->ignore();
                return; // let the completer do default behavior
            default:
//                 Choqok::UI::TextEdit::keyPressEvent(e);
                break;
        }
    } else if(e->text().isEmpty()){
        Choqok::UI::TextEdit::keyPressEvent(e);
        return;
    }
    if(e->key() == Qt::Key_Tab){
        e->ignore();
        return;
    }
//     bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+E
//     if (!d->c )// || !isShortcut) // don't process the shortcut when we have a completer
    Choqok::UI::TextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier |
                                                Qt::MetaModifier);
    if (!d->c || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()+{}|:\"<>?,./;'[]\\-= "); // end of word
//     bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    //Implemented internally to get the char before selection :D
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    QString completionPrefix = tc.selectedText();
    QChar charBeforeSelection;
    if(completionPrefix.startsWith('@')){
        charBeforeSelection = completionPrefix.at(0);
        completionPrefix.remove(0,1);
    } else {
      if (!toPlainText().isEmpty() && tc.selectionStart() > 0)
        charBeforeSelection = toPlainText()[tc.selectionStart() - 1];
    }

    if ( !e->text().isEmpty() && (eow.contains(e->text().right(1)) || completionPrefix.length() < 1 ||
                                  charBeforeSelection != '@') ) {
        d->c->popup()->hide();
        return;
    } else if ((e->key() != Qt::Key_Enter) && (e->key() != Qt::Key_Return)) {
        if ( textCursor().selectedText().length() && 
             textCursor().selectedText() != completionPrefix ) {
            return;
        }

        if (completionPrefix != d->c->completionPrefix()) {
            d->c->setCompletionPrefix(completionPrefix);
            d->c->popup()->setCurrentIndex(d->c->completionModel()->index(0, 0));
        }
        QRect cr = cursorRect();
        cr.setWidth(d->c->popup()->sizeHintForColumn(0)
        + d->c->popup()->verticalScrollBar()->sizeHint().width());
        d->c->complete(cr); // popup it up!
    }
}

void TwitterApiTextEdit::updateRemainingCharsCount()
{
    QString txt = this->toPlainText();
    int count = txt.count();
    if (count) {
        lblRemainChar->show();
        if (charLimit()) {
            int remain = charLimit() - count;

            Q_FOREACH (const QString &url, UrlUtils::detectUrls(txt)) {
                // Twitter does not wrapps urls with login informations
                if (!url.contains("@")) {
                    int diff = -1;
                    if (url.startsWith("http://")) {
                        diff = url.length() - d->tCoMaximumLength;
                    } else if (url.startsWith("https://")) {
                        diff = url.length() - d->tCoMaximumLengthHttps;
                    }

                    if (diff > 0) {
                        remain += diff;
                    }
                }
            }

            if (remain < 0) {
                lblRemainChar->setStyleSheet( "QLabel {color: red;}" );
            } else if (remain < 30) {
                lblRemainChar->setStyleSheet( "QLabel {color: rgb(242, 179, 19);}" );
            } else {
                lblRemainChar->setStyleSheet( "QLabel {color: green;}" );
            }
            lblRemainChar->setText( QString::number(remain) );
        } else {
            lblRemainChar->setText( QString::number(count) );
            lblRemainChar->setStyleSheet( "QLabel {color: blue;}" );
        }
        txt.remove(QRegExp("@([^\\s\\W]+)"));
        txt = txt.trimmed();
        if (firstChar() != txt[0]) {
            setFirstChar(txt[0]);
            txt.prepend(' ');
            QTextBlockFormat f;
            f.setLayoutDirection( (Qt::LayoutDirection) txt.isRightToLeft() );
            textCursor().mergeBlockFormat(f);
        }
    } else {
        lblRemainChar->hide();
    }
}

void TwitterApiTextEdit::fetchTCoMaximumLength()
{
    TwitterApiAccount *acc = qobject_cast<TwitterApiAccount*>(d->acc);
    if (acc) {
        KUrl url("https://api.twitter.com/1.1/help/configuration.json");

        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
        if (!job) {
            kDebug() << "Cannot create an http GET request!";
            return;
        }
        TwitterApiMicroBlog *mBlog = qobject_cast<TwitterApiMicroBlog*>(acc->microblog());
        job->addMetaData("customHTTPHeader", "Authorization: " +
            mBlog->authorizationHeader(acc, url, QOAuth::GET));
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotTCoMaximumLength(KJob*)));
        job->start();
    } else {
        kDebug() << "the account is not a TwitterAPIAccount!";
    }
}

void TwitterApiTextEdit::slotTCoMaximumLength(KJob* job)
{
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
    } else {
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob* >(job);
        bool ok;
        QJson::Parser parser;
        const QVariantMap reply = parser.parse(j->data(), &ok).toMap();
        if (ok) {
            d->tCoMaximumLength = reply["short_url_length"].toInt();
            d->tCoMaximumLengthHttps = reply["short_url_length_https"].toInt();
        } else {
            kDebug() << "Cannot parse JSON reply";
        }
    }
}
