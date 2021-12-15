/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <plugin.h>
#include <qqueue.h>
#include <QPointer>

class QMenu;
class QAction;
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

protected Q_SLOTS:
    void translate();
    void slotTranslated(KJob*);
    void slotConfigureTranslator();
    void slotUpdateMenu();

private:
    QMenu *setupTranslateMenu();

    QMap<KJob*, Choqok::UI::PostWidget*> mJobPostWidget;
    QStringList langs;
    QAction *translateAction;
};

#endif //TRANSLATOR_H
