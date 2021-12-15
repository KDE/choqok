/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOKTEXTEDIT_H
#define CHOQOKTEXTEDIT_H

#include <KTextEdit>

#include "choqok_export.h"

class QLabel;

namespace Choqok
{
namespace UI
{
/**

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT TextEdit : public KTextEdit
{
    Q_OBJECT
public:
    /**
    \brief Constructor
    @param charLimit Character limit for current account. 0 means no limit
    */
    explicit TextEdit(uint charLimit = 0, QWidget *parent = nullptr);
    virtual ~TextEdit();
    void clear();
    void setCharLimit(uint charLimit = 0);
    void setPlainText(const QString &text);
    void setText(const QString &text);
    void prependText(const QString &text);
    void appendText(const QString &text);

protected:
    virtual void keyPressEvent(QKeyEvent *) override;
    virtual void insertFromMimeData(const QMimeData *source) override;
    virtual QSize minimumSizeHint() const override;

Q_SIGNALS:
    void returnPressed(const QString &txt);
//     void charsRemain( int count );
    void cleared();

protected Q_SLOTS:
    virtual void updateRemainingCharsCount();
    void settingsChanged();
    void slotChangeSpellerLanguage();
    void setupSpeller();
    void slotAboutToShowContextMenu(QMenu *menu);
    void shortenUrls();

protected:
    uint charLimit();
    QChar firstChar();
    void setFirstChar(const QChar &firstChar);
    void undoableClear();
    QLabel *lblRemainChar;

private:
    class Private;
    Private *const d;
};
}
}
#endif // CHOQOKTEXTEDIT_H
