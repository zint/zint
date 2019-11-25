/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra <bogdan@licentia.eu>               *
 *   Copyright (C) 2009-2019 by Robin Stuart <rstuart114@gmail.com>        *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QGraphicsItem>
#include <QMainWindow>
#include <QGraphicsScene>

#include "ui_mainWindow.h"
#include "barcodeitem.h"

class QAction;
class QActionGroup;
class QLabel;
class QMenu;

class MainWindow : public QWidget, private Ui::mainWindow
{
    Q_OBJECT

    Q_ENUMS(BarcodeTypes)

public:
    enum BarcodeTypes
    {
        AUSREDIRECT      = 68,
        AUSREPLY         = 66,
        AUSROUTE         = 67,
        AUSPOST          = 63,
        AZTEC            = 92,
        AZRUNE           = 128,
        CHANNEL          = 140,
        CODABAR          = 18,
        CODABLOCK        = 74,
        CODE11           = 1,
        CODE128          = 20,
        CODE16K          = 23,
        C25LOGIC         = 6,
        C25IATA          = 4,
        C25IND           = 7,
        C25INTER         = 3,
        C25MATRIX        = 2,
        CODE32           = 129,
        CODE39           = 8,
        EXCODE39         = 9,
        CODE49           = 24,
        CODE93           = 25,
        CODE_ONE         = 141,
        DATAMATRIX       = 71,
        DPIDENT          = 22,
        DPLEIT           = 21,
        DOTCODE          = 115,
        KIX              = 90,
        EAN14            = 72,
        EANX             = 13,
        FIM              = 49,
        FLAT             = 28,
        GRIDMATRIX       = 142,
        RSS_EXP          = 31,
        RSS_EXPSTACK     = 81,
        RSS_LTD          = 30,
        RSS14            = 29,
        RSS14STACK       = 79,
        RSS14STACK_OMNI  = 80,
        HANXIN           = 116,
        ITF14            = 89,
        ISBNX            = 69,
        JAPANPOST        = 76,
        KOREAPOST        = 77,
        LOGMARS          = 50,
        MAXICODE         = 57,
        MICROPDF417      = 84,
        MICROQR          = 97,
        MSI_PLESSEY      = 47,
        NVE18            = 75,
        PDF417           = 55,
        PHARMA           = 51,
        PHARMA_TWO       = 53,
        PZN              = 52,
        PLANET           = 82,
        POSTNET          = 40,
        QRCODE           = 58,
        RMQR             = 145,
        RM4SCC           = 70,
        MAILMARK         = 121,
        TELEPEN          = 32,
        TELEPEN_NUM      = 87,
        PLESSEY          = 86,
        UPNQR            = 143,
        UPCA             = 34,
        UPCE             = 37,
        ONECODE          = 85
    };

public:
    MainWindow(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~MainWindow();


public slots:
    void update_preview();
    void change_options();
    void on_fgcolor_clicked();
    void on_bgcolor_clicked();
    void composite_enable();
    void composite_ean_check();
    void maxi_primary();
    void change_print_scale();
    void autoheight_clicked();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    bool save();
    void about();
    void quit_now();
    void reset_view();
    int open_data_dialog();
    int open_sequence_dialog();
    void copy_to_clipboard_svg();
    void copy_to_clipboard_bmp();

private:
    QColor m_fgcolor,m_bgcolor;
    BarcodeItem m_bc;
    QWidget *m_optionWidget;
    QGraphicsScene *scene;
};

#endif
