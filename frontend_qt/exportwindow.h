/*
    Zint Barcode Generator - the open source barcode generator
    Copyright (C) 2009-2022 Robin Stuart <rstuart114@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef Z_EXPORTWINDOW_H
#define Z_EXPORTWINDOW_H

#include "ui_extExport.h"
#include "barcodeitem.h"

class ExportWindow : public QDialog, private Ui::ExportDialog
{
    Q_OBJECT

public:
    ExportWindow(BarcodeItem *bc, const QString& output_data);
    ~ExportWindow();

private slots:
    void process();
    void get_directory();

protected:
    BarcodeItem *m_bc;
    QString m_output_data;
    QStringList m_dataList;
    int m_lines;
};

/* vim: set ts=4 sw=4 et : */
#endif
