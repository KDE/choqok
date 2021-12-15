/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UI_CHOQOK_MAINWINDOW_H
#define UI_CHOQOK_MAINWINDOW_H

#include <QHideEvent>

#include <KXmlGuiWindow>

#include "choqok_export.h"

class QTimer;
class QTabWidget;

namespace Choqok
{
namespace UI
{
class MicroBlogWidget;

class CHOQOK_EXPORT MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

    /**
    @return current active microblog widget
    */
    Choqok::UI::MicroBlogWidget *currentMicroBlog();
    QList<Choqok::UI::MicroBlogWidget *> microBlogsWidgetsList();
    void activateTab(int k);

public Q_SLOTS:
    void showStatusMessage(const QString &message, bool isPermanent = false);
    void activateChoqok();

Q_SIGNALS:
    void updateTimelines();
    void markAllAsRead();
    void removeOldPosts();
    void quickPostCreated();
    void currentMicroBlogWidgetChanged(Choqok::UI::MicroBlogWidget *widget);

protected:
    virtual void hideEvent(QHideEvent *event) override;
    virtual QSize sizeHint() const override;

    QTabWidget *mainWidget;
    QTimer *timelineTimer;
};

}

}

#endif // UI_CHOQOK_MAINWINDOW_H
