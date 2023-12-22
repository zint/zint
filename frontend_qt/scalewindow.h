/*
    Zint Barcode Generator - the open source barcode generator
    Copyright (C) 2022-2023 Robin Stuart <rstuart114@gmail.com>

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

#ifndef SCALEWINDOW_H
#define SCALEWINDOW_H

#include "ui_extScale.h"
#include "barcodeitem.h"

class ScaleWindow : public QDialog, private Ui::ScaleDialog
{
    Q_OBJECT

private:
    BarcodeItem *m_bc;

public:
    ScaleWindow(BarcodeItem *bc, struct Zint::QZintXdimDpVars *vars, double originalScale);
    ~ScaleWindow();

    bool Valid;

    struct Zint::QZintXdimDpVars m_vars;

signals:
    void scaleChanged(double scale);

public slots:
    void size_msg_ui_set();
private slots:
    void unset_scale();
    void okay();
    void update_scale();
    void x_dim_units_change();
    void x_dim_default();
    void resolution_units_change();
    void resolution_default();

private:
    float get_x_dim_mm() const;
    float get_dpmm() const;
    const char *getFileType() const;
    void set_maxima();
    double update_vars();
    double m_originalScale;
    bool m_unset;
};

/* vim: set ts=4 sw=4 et : */
#endif
