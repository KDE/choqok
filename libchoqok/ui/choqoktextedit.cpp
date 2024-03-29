/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "choqoktextedit.h"

#include <QAction>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QTimer>

#include <KLocalizedString>
#include <sonnet/speller.h>

#include "choqokbehaviorsettings.h"
#include "libchoqokdebug.h"
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
    connect(this, &TextEdit::textChanged, this, &TextEdit::updateRemainingCharsCount);
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
    connect(BehaviorSettings::self(), &BehaviorSettings::configChanged,
            this, &TextEdit::settingsChanged);

    QTimer::singleShot(1000, this, SLOT(setupSpeller()));
    connect(this, &TextEdit::aboutToShowContextMenu, this,
            &TextEdit::slotAboutToShowContextMenu);
}

TextEdit::~TextEdit()
{
    disconnect(this, &TextEdit::textChanged, this,
            &TextEdit::updateRemainingCharsCount);
    disconnect(this, &TextEdit::aboutToShowContextMenu, this,
            &TextEdit::slotAboutToShowContextMenu);
    disconnect(BehaviorSettings::self(), &BehaviorSettings::configChanged,
               this, &TextEdit::settingsChanged);

    BehaviorSettings::setSpellerLanguage(d->curLang);
    d->langActions->deleteLater();
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
                lblRemainChar->setStyleSheet(QLatin1String("QLabel {color: red;}"));
            } else if (remain < 30) {
                lblRemainChar->setStyleSheet(QLatin1String("QLabel {color: rgb(242, 179, 19);}"));
            } else {
                lblRemainChar->setStyleSheet(QLatin1String("QLabel {color: green;}"));
            }
            lblRemainChar->setText(QString::number(remain));
        } else {
            lblRemainChar->setText(QString::number(count));
            lblRemainChar->setStyleSheet(QLatin1String("QLabel {color: blue;}"));
        }
        txt.remove(QRegExp(QLatin1String("@([^\\s\\W]+)")));
        txt = txt.trimmed();
        if (d->firstChar != txt[0]) {
            d->firstChar = txt[0];
            txt.prepend(QLatin1Char(' '));
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
        connect(shorten, &QAction::triggered, this, &TextEdit::shortenUrls);
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
        QAction *actOld = d->langActionMap.value(d->curLang);
        if (actOld) {
            actOld->setChecked(false);
        }
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
    tmp.append(QLatin1Char(' ') + toPlainText());
    setPlainText(tmp);
}

void TextEdit::appendText(const QString &text)
{
    QString tmp = toPlainText();
    if (tmp.isEmpty()) {
        tmp = text + QLatin1Char(' ');
    } else {
        tmp.append(QLatin1Char(' ') + text);
    }
    setPlainText(tmp);
}

void TextEdit::settingsChanged()
{
    setCheckSpellingEnabled(BehaviorSettings::enableSpellChecker());
}

void TextEdit::setupSpeller()
{
    BehaviorSettings::self()->load();
    d->curLang = BehaviorSettings::spellerLanguage();
    Sonnet::Speller s;
    if (d->curLang.isEmpty()) {
        d->curLang = s.defaultLanguage();
    }
    qCDebug(CHOQOK) << "Current LANG:" << d->curLang;
    for (const QString &dict: s.availableDictionaries().keys()) {
        const QString value = s.availableDictionaries().value(dict);
        QAction *act = new QAction(dict, d->langActions);
        act->setData(value);
        act->setCheckable(true);
        if (d->curLang == value) {
            act->setChecked(true);
        }
        connect(act, &QAction::triggered, this, &TextEdit::slotChangeSpellerLanguage);
        d->langActions->addAction(act);
        d->langActionMap.insert(value, act);
    }
}

QSize TextEdit::minimumSizeHint() const
{
    const QSize size = KTextEdit::minimumSizeHint();
    return QSize(size.width(), qMax(fontMetrics().height() * 3, size.height()));
}

}
}

#include "moc_choqoktextedit.cpp"
