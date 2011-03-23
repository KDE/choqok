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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <plugin.h>
#include <qqueue.h>
#include <KUrl>
#include <QPointer>

class QMenu;
class KAction;
namespace KIO {
class Job;
}

class KJob;
namespace Choqok {
    class ShortenManager;
namespace UI {
    class PostWidget;
}
}

class KConfigGroup;

class Translator : public Choqok::Plugin
{
    Q_OBJECT
public:
    Translator( QObject* parent, const QList< QVariant >& args );
    ~Translator();

protected slots:
    void translate();
    void slotTranslated(KJob*);
    void slotConfigureTranslator();
    void slotUpdateMenu();

private:
    QMenu *setupTranslateMenu();

    QMap<KJob*, Choqok::UI::PostWidget*> mJobPostWidget;
    QStringList langs;
    KAction *translateAction;
};

#endif //TRANSLATOR_H
