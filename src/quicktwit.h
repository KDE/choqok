//
// C++ Interface: quicktwit
//
// Description: 
//
//
// Author:  Mehrdad Momeny <mehrdad.momeny@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QUICKTWIT_H
#define QUICKTWIT_H


#include <kdialog.h>
class StatusTextEdit;
class Backend;
/**
Widget for Quik twitting

	@author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class QuickTwit : public KDialog
{
	Q_OBJECT
public:
    QuickTwit(QWidget* parent=0);

    ~QuickTwit();
public slots:
	void showNearMouse();
	void checkNewStatusCharactersCount(int numOfChars);
	void slotPostNewStatus(QString &newStatus);
	void slotPostNewStatusDone(bool isError);
	void sltAccepted();

protected:
// 	void keyPressEvent(QKeyEvent *e);
	
signals:
	void sigStatusUpdated();
	void sigNotify( const QString &title, const QString &message, const QString &iconUrl);
	
private:
	StatusTextEdit *txtStatus;
	Backend *twitter;
};

#endif
