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

public:
    MainWindow(QWidget *parent = 0, Qt::WindowFlags fl = Qt::WindowFlags());
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
    void set_smaller_font(QLabel *note);

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
