/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2011  Farhad Hedayati-Fard <hf.farhad@gmail.com>

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

#ifndef QUICKFILTER_H
#define QUICKFILTER_H
#include <QPointer>
#include <plugin.h>
#include <QList>

class KAction;
namespace Choqok {
    class ShortenManager;
    class Account;
    namespace UI {
        class PostWidget;
        class MicroBlogWidget;
    }
}

class KLineEdit;
class QToolBar;

class QuickFilter : public Choqok::Plugin
{
    Q_OBJECT
public:
    QuickFilter(QObject* parent, const QList< QVariant >& args);
    ~QuickFilter();
    
public slots:
    void createUiInterface();
    void showAuthorFilterUiInterface(bool);
    void showContentFilterUiInterface(bool);
    void showAllPosts();
    
protected slots:
    void filterByAuthor();
    void filterByContent();
    void filterNewPost(Choqok::UI::PostWidget*, Choqok::Account*, QString);
    
private slots:
    void updateUser(QString user);
    void updateContent(QString text);
    
private:
    QString m_filterUser;
    QString m_filterText;
    KLineEdit *m_aledit;
    KLineEdit *m_tledit;
    QToolBar *m_authorToolbar;
    QToolBar *m_textToolbar;
    KAction *m_authorAction;
    KAction *m_textAction;
};

#endif // QUICKFILTER_H
