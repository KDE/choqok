/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "choqokmainwindow.h"

#include <QStatusBar>
#include <QTabWidget>

#include "choqokbehaviorsettings.h"
#include "libchoqokdebug.h"
#include "microblogwidget.h"

#ifdef QTINDICATE_BUILD
#include "indicatormanager.h"
#endif

using namespace Choqok::UI;

static const int TIMEOUT = 5000;

Choqok::UI::MainWindow::MainWindow()
    : KXmlGuiWindow()
{
    mainWidget = new QTabWidget(this);
    mainWidget->setDocumentMode(true);
    mainWidget->setMovable(true);
#ifdef QTINDICATE_BUILD
    Choqok::MessageIndicatorManager::self();
#endif
}

Choqok::UI::MainWindow::~MainWindow()
{

}

QSize MainWindow::sizeHint() const
{
    return QSize(350, 400);
}

void MainWindow::showStatusMessage(const QString &message, bool isPermanent)
{
    if (isPermanent) {
        statusBar()->showMessage(message);
    } else {
        statusBar()->showMessage(message, TIMEOUT);
    }
}

void MainWindow::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    if (!this->isVisible()) {
        qCDebug(CHOQOK);
        if (Choqok::BehaviorSettings::markAllAsReadOnHideToSystray()) {
            Q_EMIT markAllAsRead();
        }
        Q_EMIT removeOldPosts();
    }
}

Choqok::UI::MicroBlogWidget *MainWindow::currentMicroBlog()
{
    return qobject_cast<Choqok::UI::MicroBlogWidget *>(mainWidget->currentWidget());
}

void Choqok::UI::MainWindow::activateChoqok()
{
    showNormal();
    activateWindow();
    raise();
}

QList<Choqok::UI::MicroBlogWidget *> Choqok::UI::MainWindow::microBlogsWidgetsList()
{
    QList<Choqok::UI::MicroBlogWidget *> lst;
    if (mainWidget->currentWidget())
        for (int i = 0; i < mainWidget->count(); i++) {
            lst.append(qobject_cast<Choqok::UI::MicroBlogWidget *>(mainWidget->widget(i)));
        }
    return lst;
}

void Choqok::UI::MainWindow::activateTab(int k)
{
    if (mainWidget->count() >= k) {
        mainWidget->setCurrentIndex(k);
    }
}

#include "moc_choqokmainwindow.cpp"
