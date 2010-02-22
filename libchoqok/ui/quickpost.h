/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#ifndef QUICKPOST_H
#define QUICKPOST_H

#include "choqoktypes.h"
#include <kdialog.h>
#include "account.h"
#include <microblog.h>

class KComboBox;
class QCheckBox;
namespace Choqok {
namespace UI {
class TextEdit;

/**
Widget for Quick posting

    @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT QuickPost : public KDialog
{
    Q_OBJECT
public:
    QuickPost( QWidget* parent = 0 );
    virtual ~QuickPost();

public slots:
    void show();
    void submitPost( const QString &newStatus );
    void setText( const QString& text/*, Choqok::Account* account = 0, const QString& replyToId = QString()*/ );

signals:
    /**
    Emitted when a new post submitted. @p postText is the text that submitted,
    @p postText will be empty on failure!

    @param result Result of posting, Could be Success or Fail
    */
    void newPostSubmitted( Choqok::JobResult result, const QString &postText = QString() );

protected:
    void loadAccounts();

protected slots:
    void slotCurrentAccountChanged(int);
    void checkAll( bool isAll );
    virtual void slotButtonClicked(int button);
    void addAccount( Choqok::Account* account );
    void removeAccount( const QString &alias );
    void accountModified( Choqok::Account *theAccount );
    virtual void slotSubmitPost( Choqok::Account *theAccount, Choqok::Post *post );
    void postError( Choqok::Account *theAccount, Choqok::Post* post,
                    Choqok::MicroBlog::ErrorType error, const QString &errorMessage);

private:
    void setupUi();
    class Private;
    Private *d;
};

}

}
#endif
