/*
    Zint Barcode Generator - the open source barcode generator
    Copyright (C) 2021-2024 Robin Stuart <rstuart114@gmail.com>

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

//#include <QDebug>
#include <QClipboard>
#include <QMimeData>
#include <QSettings>

#include "cliwindow.h"

// Shorthand
#define QSL     QStringLiteral
#define QSEmpty QLatin1String("")

static const int tempMessageTimeout = 2000;

CLIWindow::CLIWindow(BarcodeItem *bc, const bool autoHeight, const double heightPerRow,
                        const struct Zint::QZintXdimDpVars* xdimdpVars)
    : m_bc(bc), m_autoHeight(autoHeight), m_heightPerRow(heightPerRow), m_xdimdpVars(xdimdpVars)
{
    setupUi(this);
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif

    QByteArray geometry = settings.value(QSL("studio/cli/window_geometry")).toByteArray();
    restoreGeometry(geometry);

#ifdef _WIN32
    const int index = settings.value(QSL("studio/cli/rad_unix_win"), 1).toInt();
#else
    const int index = settings.value(QSL("studio/cli/rad_unix_win"), 0).toInt();
#endif
    if (index == 1) {
        radCLIWin->setChecked(true);
        chkCLINoEXE->setEnabled(true);
    } else {
        radCLIUnix->setChecked(true);
        chkCLINoEXE->setEnabled(false);
    }
    chkCLILongOpts->setChecked(settings.value(QSL("studio/cli/chk_long_opts"), 0).toInt() ? true : false);
    chkCLIBarcodeName->setChecked(settings.value(QSL("studio/cli/chk_barcode_name"), 0).toInt() ? true : false);
    chkCLINoEXE->setChecked(settings.value(QSL("studio/cli/chk_no_exe"), 0).toInt() ? true : false);

    QIcon copyIcon(QIcon::fromTheme(QSL("edit-copy"), QIcon(QSL(":res/copy.svg"))));
    QIcon closeIcon(QIcon::fromTheme(QSL("window-close"), QIcon(QSL(":res/x.svg"))));
    btnCLICopy->setIcon(copyIcon);
    btnCLIClose->setIcon(closeIcon);

    connect(btnCLIClose, SIGNAL(clicked(bool)), SLOT(close()));
    connect(btnCLICopy, SIGNAL(clicked(bool)), SLOT(copy_to_clipboard()));
    connect(radCLIUnix, SIGNAL(toggled(bool)), SLOT(generate_cli()));
    connect(radCLIWin, SIGNAL(toggled(bool)), SLOT(generate_cli()));
    connect(chkCLILongOpts, SIGNAL(toggled(bool)), SLOT(generate_cli()));
    connect(chkCLIBarcodeName, SIGNAL(toggled(bool)), SLOT(generate_cli()));
    connect(chkCLINoEXE, SIGNAL(toggled(bool)), SLOT(generate_cli()));

    generate_cli();
}

CLIWindow::~CLIWindow()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    settings.setValue(QSL("studio/cli/window_geometry"), saveGeometry());
    settings.setValue(QSL("studio/cli/rad_unix_win"), radCLIWin->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/cli/chk_long_opts"), chkCLILongOpts->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/cli/chk_barcode_name"), chkCLIBarcodeName->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/cli/chk_no_exe"), chkCLINoEXE->isChecked() ? 1 : 0);
}

void CLIWindow::copy_to_clipboard()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QMimeData *mdata = new QMimeData;
    mdata->setData("text/plain", txtCLICmd->toPlainText().toUtf8());
    clipboard->setMimeData(mdata, QClipboard::Clipboard);
    statusBarCLI->showMessage(tr("Copied to clipboard"), tempMessageTimeout);
}

void CLIWindow::generate_cli()
{
    bool noEXE = false;
    if (radCLIWin->isChecked()) {
        noEXE = chkCLINoEXE->isChecked();
        chkCLINoEXE->setEnabled(true);
    } else {
        chkCLINoEXE->setEnabled(false);
    }
    QString cmd = m_bc->bc.getAsCLI(radCLIWin->isChecked(), chkCLILongOpts->isChecked(),
                    chkCLIBarcodeName->isChecked(), noEXE, m_autoHeight, m_heightPerRow, QSEmpty /*outfile*/,
                    m_xdimdpVars);

    txtCLICmd->setPlainText(cmd);
    statusBarCLI->clearMessage();
}

/* vim: set ts=4 sw=4 et : */
