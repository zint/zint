/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra <bogdan@licentia.eu>               *
 *   Copyright (C) 2009-2020 by Robin Stuart <rstuart114@gmail.com>        *
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

class QLabel;

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
        DAFT             = 93,
        DATAMATRIX       = 71,
        DPIDENT          = 22,
        DPLEIT           = 21,
        DOTCODE          = 115,
        DPD              = 96,
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
        ISBNX            = 69,
        ITF14            = 89,
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
        ULTRA            = 144,
        UPCA             = 34,
        UPCE             = 37,
        UPNQR            = 143,
        ONECODE          = 85,
        VIN              = 73
    };

public:
    MainWindow(QWidget* parent = 0, Qt::WindowFlags fl = Qt::WindowFlags());
    ~MainWindow();


public slots:
    void update_preview();
    void change_options();
    void on_fgcolor_clicked();
    void on_bgcolor_clicked();
    void composite_ui_set();
    void composite_ean_check();
    void maxi_primary();
    void msi_plessey_ui_set();
    void change_print_scale();
    void change_cmyk();
    void autoheight_ui_set();
    void HRTShow_ui_set();
    void dotty_ui_set();
    void on_encoded();
	void filter_symbologies();

protected:
    void resizeEvent(QResizeEvent *event);
	void combobox_item_enabled(QComboBox *comboBox, int index, bool enabled);
    void upcean_addon_gap(QComboBox *comboBox, QLabel *label, int base);
    void set_gs1_mode(bool gs1_mode);

	const char *get_setting_name(int symbology);

	int get_button_group_index(const QStringList &children);
	void set_radiobutton_from_setting(QSettings &settings, const QString &setting, const QStringList &children, int default_val = 0);
	int get_combobox_index(const QString &child);
	void set_combobox_from_setting(QSettings &settings, const QString &setting, const QString &child, int default_val = 0);
	int get_checkbox_val(const QString &child);
	void set_checkbox_from_setting(QSettings &settings, const QString &setting, const QString &child, int default_val = 0);
	double get_doublespinbox_val(const QString &child);
	void set_doublespinbox_from_setting(QSettings &settings, const QString &setting, const QString &child, float default_val = 0.0f);
	QString get_lineedit_val(const QString &child);
	void set_lineedit_from_setting(QSettings &settings, const QString &setting, const QString &child, const char *default_val = "");
	int get_spinbox_val(const QString &child);
	void set_spinbox_from_setting(QSettings &settings, const QString &setting, const QString &child, int default_val = 0);

	void save_sub_settings(QSettings &settings, int symbology);
	void load_sub_settings(QSettings &settings, int symbology);

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
	int m_symbology;
};

#endif
