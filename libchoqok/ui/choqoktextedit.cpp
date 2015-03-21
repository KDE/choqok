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
#include "choqoktextedit.h"

#include <QAction>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QTimer>

#include "libchoqokdebug.h"
#include <KLocalizedString>
#include <sonnet/speller.h>

#include "choqokbehaviorsettings.h"
#include "shortenmanager.h"

namespace Choqok
{
namespace UI
{

class TextEdit::Private
{
public:
    Private(uint charLmt)
        : langActions(new QMenu), charLimit(charLmt)
    {}
    QMenu *langActions;
    QMap<QString, QAction *> langActionMap;
    uint charLimit;
    QString prevStr;
    QChar firstChar;
    QString curLang;
};

TextEdit::TextEdit(uint charLimit /*= 0*/, QWidget *parent /*= 0*/)
    : KTextEdit(parent), d(new Private(charLimit))
{
    qCDebug(CHOQOK) << charLimit;
    connect(this, SIGNAL(textChanged()), this, SLOT(updateRemainingCharsCount()));
    setAcceptRichText(false);
    this->setToolTip(i18n("<b>Note:</b><br/><i>Ctrl+S</i> to enable/disable auto spell checker."));

    enableFindReplace(false);
    QFont counterF;
    counterF.setBold(true);
    counterF.setPointSize(10);
    lblRemainChar = new QLabel(this);
    lblRemainChar->resize(50, 50);
    lblRemainChar->setFont(counterF);
    QGridLayout *layout = new QGridLayout(this);
    layout->setRowStretch(0, 100);
    layout->setColumnStretch(5, 100);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(lblRemainChar, 1, 0);
    this->setLayout(layout);
    setTabChangesFocus(true);
    settingsChanged();
    connect(BehaviorSettings::self(), SIGNAL(configChanged()), SLOT(settingsChanged()));

    QTimer::singleShot(1000, this, SLOT(setupSpeller()));
    connect(this, SIGNAL(aboutToShowContextMenu(QMenu*)),
            SLOT(slotAboutToShowContextMenu(QMenu*)));
}

TextEdit::~TextEdit()
{
    BehaviorSettings::setSpellerLanguage(d->curLang);
    delete d;
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    if ((e->key() == Qt::Key_Return) || (e->key() == Qt::Key_Enter)) {
        if (e->modifiers() == Qt::ShiftModifier) {
            KTextEdit::keyPressEvent(e);
        } else {
            QString txt = toPlainText();
            Q_EMIT returnPressed(txt);
        }
        e->accept();
    } else if (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_S) {
        this->setCheckSpellingEnabled(!this->checkSpellingEnabled());
        e->accept();
    } else if (e->key() == Qt::Key_Escape) {
        if (!this->toPlainText().isEmpty()) {
            this->clear();
            Q_EMIT cleared();
            e->accept();
        } else {
            KTextEdit::keyPressEvent(e);
        }
    } else {
        KTextEdit::keyPressEvent(e);
    }
}

void TextEdit::clear()
{
    if (toPlainText().isEmpty()) {
        return;
    } else {
        undoableClear();
        Q_EMIT cleared();
    }
}

void TextEdit::undoableClear()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.endEditBlock();
}

void TextEdit::insertFromMimeData(const QMimeData *source)
{
    if (Choqok::BehaviorSettings::shortenOnPaste()) {
        KTextEdit::insertPlainText(ShortenManager::self()->parseText(source->text()));
    } else {
        KTextEdit::insertPlainText(source->text());
    }
}

void TextEdit::updateRemainingCharsCount()
{
    QString txt = this->toPlainText();
    int count = txt.count();
    if (count) {
        lblRemainChar->show();
        if (d->charLimit) {
            int remain = d->charLimit - count;
            if (remain < 0) {
                lblRemainChar->setStyleSheet("QLabel {color: red;}");
            } else if (remain < 30) {
                lblRemainChar->setStyleSheet("QLabel {color: rgb(242, 179, 19);}");
            } else {
                lblRemainChar->setStyleSheet("QLabel {color: green;}");
            }
            lblRemainChar->setText(QString::number(remain));
        } else {
            lblRemainChar->setText(QString::number(count));
            lblRemainChar->setStyleSheet("QLabel {color: blue;}");
        }
        txt.remove(QRegExp("@([^\\s\\W]+)"));
        txt = txt.trimmed();
        if (d->firstChar != txt[0]) {
            d->firstChar = txt[0];
            txt.prepend(' ');
            QTextBlockFormat f;
            f.setLayoutDirection((Qt::LayoutDirection) txt.isRightToLeft());
            textCursor().mergeBlockFormat(f);
        }
    } else {
        lblRemainChar->hide();
    }
}

void TextEdit::slotAboutToShowContextMenu(QMenu *menu)
{
    if (menu) {
        qCDebug(CHOQOK);
        QAction *act = new QAction(i18n("Set spell check language"), menu);
        act->setMenu(d->langActions);
        menu->addAction(act);

        QAction *shorten = new QAction(i18nc("Replace URLs by a shortened URL", "Shorten URLs"), menu);
        connect(shorten, SIGNAL(triggered(bool)), SLOT(shortenUrls()));
        menu->addAction(shorten);
    }
}

void TextEdit::shortenUrls()
{
    qCDebug(CHOQOK);
    QTextCursor cur = textCursor();
    if (!cur.hasSelection()) {
        cur.select(QTextCursor::BlockUnderCursor);
    }
    QString shortened = ShortenManager::self()->parseText(cur.selectedText());
    cur.removeSelectedText();
    cur.insertText(shortened);
    setTextCursor(cur);
}

void TextEdit::slotChangeSpellerLanguage()
{
    QAction *act = qobject_cast<QAction *>(sender());
    if (act) {
        QString lang = act->data().toString();
        setSpellCheckingLanguage(lang);
        d->langActionMap.value(d->curLang)->setChecked(false);
        d->curLang = lang;
    }
}

uint TextEdit::charLimit()
{
    return d->charLimit;
}

QChar TextEdit::firstChar()
{
    return d->firstChar;
}

void TextEdit::setFirstChar(const QChar &firstChar)
{
    d->firstChar = firstChar;
}

void TextEdit::setCharLimit(uint charLimit /*= 0*/)
{
    d->charLimit = charLimit;
    updateRemainingCharsCount();
}

void TextEdit::setPlainText(const QString &text)
{
    if (Choqok::BehaviorSettings::shortenOnPaste()) {
        KTextEdit::setPlainText(ShortenManager::self()->parseText(text));
    } else {
        KTextEdit::setPlainText(text);
    }
    moveCursor(QTextCursor::End);
    setEnabled(true);
}

void TextEdit::setText(const QString &text)
{
    KTextEdit::setPlainText(text);
    moveCursor(QTextCursor::End);
    setEnabled(true);
}

void TextEdit::prependText(const QString &text)
{
    QString tmp = text;
    tmp.append(' ' + toPlainText());
    setPlainText(tmp);
}

void TextEdit::appendText(const QString &text)
{
    QString tmp = toPlainText();
    if (tmp.isEmpty()) {
        tmp = text + ' ';
    } else {
        tmp.append(' ' + text);
    }
    setPlainText(tmp);
}

void TextEdit::settingsChanged()
{
    setCheckSpellingEnabled(BehaviorSettings::enableSpellChecker());
}

void TextEdit::setupSpeller()
{
    BehaviorSettings::self()->readConfig();
    d->curLang = BehaviorSettings::spellerLanguage();
    Sonnet::Speller s;
    if (d->curLang.isEmpty()) {
        d->curLang = s.defaultLanguage();
    }
    qCDebug(CHOQOK) << "Current LANG: " << d->curLang;
    QMap<QString, QString> list = s.availableDictionaries();
    QMap<QString, QString>::const_iterator it = list.constBegin(), endIt = list.constEnd();
    for (; it != endIt; ++it) {
        QAction *act = new QAction(it.key(), d->langActions);
        act->setData(it.value());
        act->setCheckable(true);
        if (d->curLang == it.value()) {
            act->setChecked(true);
        }
        connect(act, SIGNAL(triggered(bool)), SLOT(slotChangeSpellerLanguage()));
        d->langActions->addAction(act);
        d->langActionMap.insert(it.value(), act);
    }
}

QSize TextEdit::minimumSizeHint() const
{
    const QSize size = QTextEdit::minimumSizeHint();
    return QSize(size.width(), qMax(fontMetrics().height() * 3, size.height()));
}

}
}
