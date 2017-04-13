//-----------------------------------------------------------------------------
// Copyright (C) 2009 Michael Gernoth <michael at gernoth.net>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// GUI (QT)
//-----------------------------------------------------------------------------

#include <QApplication>
#include <QPushButton>
#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QtGui>

#include "ui/ui_overlays.h"
/**
 * @brief The actual plot, black area were we paint the graph
 */
class Plot: public QWidget
{
private:
	int GraphStart;
	double GraphPixelsPerPoint;
	int CursorAPos;
	int CursorBPos;
	void PlotGraph(int *buffer, int len, QRect r,QRect r2, QPainter* painter, int graphNum);
	void PlotDemod(uint8_t *buffer, size_t len, QRect r,QRect r2, QPainter* painter, int graphNum, int plotOffset);
	void plotGridLines(QPainter* painter,QRect r);
	int xCoordOf(int i, QRect r );
	int yCoordOf(int v, QRect r, int maxVal);
	int valueOf_yCoord(int y, QRect r, int maxVal);
	QColor getColor(int graphNum);
public:
	Plot(QWidget *parent = 0);

protected:
	void paintEvent(QPaintEvent *event);
	void closeEvent(QCloseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event) { mouseMoveEvent(event); }
	void keyPressEvent(QKeyEvent *event);

};
class ProxGuiQT;

/**
 * The window with plot and controls
 */
class ProxWidget : public QWidget
{
	Q_OBJECT;

	private:
		Plot *plot;
		Ui::Form *opsController;
		ProxGuiQT *master;
	
	public:
		ProxWidget(QWidget *parent = 0, ProxGuiQT *master = NULL);

	//protected:
	//	void paintEvent(QPaintEvent *event);
	//	void closeEvent(QCloseEvent *event);
	//	void mouseMoveEvent(QMouseEvent *event);
	//	void mousePressEvent(QMouseEvent *event) { mouseMoveEvent(event); }
	//	void keyPressEvent(QKeyEvent *event);
	public slots:
		void applyOperation();
		void stickOperation();
		void vchange_autocorr(int v);
		void vchange_askedge(int v);
		void vchange_dthr_up(int v);
		void vchange_dthr_down(int v);
};

class ProxGuiQT : public QObject
{
	Q_OBJECT;

	private:
		QApplication *plotapp;
		ProxWidget *plotwidget;
		int argc;
		char **argv;
		void (*main_func)(void);
	
	public:
		ProxGuiQT(int argc, char **argv);
		~ProxGuiQT(void);
		void ShowGraphWindow(void);
		void RepaintGraphWindow(void);
		void HideGraphWindow(void);
		void MainLoop(void);
	
	private slots:
		void _ShowGraphWindow(void);
		void _RepaintGraphWindow(void);
		void _HideGraphWindow(void);

	signals:
		void ShowGraphWindowSignal(void);
		void RepaintGraphWindowSignal(void);
		void HideGraphWindowSignal(void);
};
