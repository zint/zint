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

#ifndef DATAWINDOW_H
#define DATAWINDOW_H

#include "ui_extData.h"

class DataWindow : public QDialog, private Ui::DataDialog
{
    Q_OBJECT

public:
    DataWindow(const QString &input, bool isEscaped, int seg_no);
    ~DataWindow();

    bool Valid;
    bool Escaped;
    QString DataOutput;

signals:
    void dataChanged(const QString& text, bool escaped, int seg_no);

private slots:
    void clear_data();
    void text_changed();
    void okay();
    void from_file();

private:
    QString escapedData(bool &escaped);

    bool m_isEscaped;
    int m_seg_no;
};

/* vim: set ts=4 sw=4 et : */
#endif
