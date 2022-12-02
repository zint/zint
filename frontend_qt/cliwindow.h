/*
    Zint Barcode Generator - the open source barcode generator
    Copyright (C) 2021-2022 Robin Stuart <rstuart114@gmail.com>

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

#ifndef CLIWINDOW_H
#define CLIWINDOW_H

#include "ui_extCLI.h"
#include "barcodeitem.h"

class CLIWindow : public QDialog, private Ui::CLIDialog
{
    Q_OBJECT

public:
    CLIWindow(BarcodeItem *bc, const bool autoHeight, const double heightPerRow,
                const struct Zint::QZintXdimDpVars* xdimdpVars);
    virtual ~CLIWindow();

private slots:
    void copy_to_clipboard();
    void generate_cli();

protected:
    BarcodeItem *m_bc;
    bool m_autoHeight;
    double m_heightPerRow;
    const Zint::QZintXdimDpVars *m_xdimdpVars;
};

/* vim: set ts=4 sw=4 et : */
#endif
