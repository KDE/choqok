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
    explicit TextEdit(uint charLimit = 0, QWidget *parent = 0);
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
