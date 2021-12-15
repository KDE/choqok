/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2011-2012 Bardia Daneshvar <bardia.daneshvar@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOKTABBAR_H
#define CHOQOKTABBAR_H

#include <QIcon>
#include <QString>
#include <QWidget>

#include "choqok_export.h"

namespace Choqok
{

namespace UI
{

class ChoqokTabBarPrivate;
class CHOQOK_EXPORT ChoqokTabBar : public QWidget
{
    Q_OBJECT
public:
    ChoqokTabBar(QWidget *parent = nullptr);
    ~ChoqokTabBar();

    enum TabPosition {
        North,
        South,
        West,
        East
    };

    enum SelectionBehavior {
        SelectLeftTab,
        SelectRightTab,
        SelectPreviousTab
    };

    enum ExtraWidgetPosition {
        Top,
        Bottom,
        Left,
        Right
    };

    void setTabPosition(TabPosition position);
    TabPosition tabPosition() const;

    void setTabsClosable(bool closeable);
    bool tabsClosable() const;

    void setCornerWidget(QWidget *w , Qt::Corner corner = Qt::TopRightCorner);
    QWidget *cornerWidget(Qt::Corner corner = Qt::TopRightCorner) const;

    void setExtraWidget(QWidget *widget , ExtraWidgetPosition position);
    QWidget *extraWidget(ExtraWidgetPosition position);

    void setTabAlongsideWidget(QWidget *widget);
    QWidget *tabAlongsideWidget() const;

    void setTabCloseActivatePrevious(bool stt);
    bool tabCloseActivatePrevious() const;

    SelectionBehavior selectionBehaviorOnRemove() const;
    void setSelectionBehaviorOnRemove(SelectionBehavior behavior);

    QWidget *currentWidget() const;
    QWidget *widget(int index) const;

    int currentIndex() const;
    int indexOf(QWidget *widget) const;

    int addTab(QWidget *widget , const QString &name);
    int addTab(QWidget *widget , const QIcon &icon , const QString &name);

    int insertTab(int index , QWidget *widget , const QString &name);
    int insertTab(int index , QWidget *widget , const QIcon &icon , const QString &name);

    void moveTab(int from , int to);
    void removeTab(int index);

    void removePage(QWidget *widget);

    void setTabIcon(int index , const QIcon &icon);
    QIcon tabIcon(int index) const;

    void setTabText(int index , const QString &text);
    QString tabText(int index) const;

    /*! With LinkedAllTabBars function, all linked tabbars
     *  using unique Settings .
     */
    void setLinkedTabBar(bool stt);
    bool linkedTabBar() const;

    void setTabBarHidden(bool stt);
    bool isTabBarHidden() const;

    QSize iconSize() const;
    void setIconSize(const QSize &size);

    int count() const;
    Qt::ToolButtonStyle toolButtonStyle() const;

    void setStyledTabBar(bool stt);
    bool styledTabBar() const;

    void refreshTabBar();

public Q_SLOTS:
    void setCurrentIndex(int index);
    void setCurrentWidget(QWidget *widget);

    void setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle);

Q_SIGNALS:
    void currentChanged(int index);
    void tabCloseRequested(int index);
    void tabMoved(int from , int to);

    void contextMenu(const QPoint &point);
    void contextMenu(QWidget *widget , const QPoint &point);

    void tabPositionChanged(TabPosition position);
    void styledPanelSignal(bool stt);
    void iconSizeChanged(const QSize &size);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

private Q_SLOTS:
    void action_triggered(QAction *action);
    void contextMenuRequest(const QPoint &point);
    void widget_destroyed(QObject *obj);

private: // Private Functions
    void init_style();
    void init_position(TabPosition position);
    void init_extra_widget(const QSize &size);
    void init_alongside_widget(const QSize &size);

private:
    ChoqokTabBarPrivate *p;
};

}
}

#endif // CHOQOKTABBAR_H
