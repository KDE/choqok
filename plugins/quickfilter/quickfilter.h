/*
    Copyright (C) 2011  Farhad Hedayati-Fard <hf.farhad@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef QUICKFILTER_H
#define QUICKFILTER_H
#include <QPointer>
#include <plugin.h>
#include <QList>

class KAction;
namespace Choqok {
    class ShortenManager;
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
  void showTextFilterUiInterface(bool);
  void hideTextFilterbar();
  void hideAuthorFilterbar();

protected slots:
  void filterByAuthor();
  void filterByText();

private slots:
  void updateUser(QString user);
  void updateText(QString text);

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
