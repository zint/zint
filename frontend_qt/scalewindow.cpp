/*
    Zint Barcode Generator - the open source barcode generator
    Copyright (C) 2022-2024 Robin Stuart <rstuart114@gmail.com>

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
#include <QSettings>
#include <QUiLoader>

#include <math.h>
#include "scalewindow.h"

// Shorthand
#define QSL QStringLiteral

/* "Standard" dpmm/dpi equivalents */
static int resolution_standard(int inch, int val)
{
    /* Note toss-up between rounding 2400 dpi to 94 or 95 (~94.48818897637795275591) */
    static int standards[9][2] = {
        {  4, 100 }, {  6,  150 }, {  8,  200 }, {  12,  300 }, { 16, 400 },
        { 24, 600 }, { 47, 1200 }, { 94, 2400 }, { 189, 4800 },
    };
    for (int i = 0; i < 8; i++) {
        if (standards[i][inch] == val) {
            return standards[i][inch ? 0 : 1];
        }
    }
    return 0;
}

ScaleWindow::ScaleWindow(BarcodeItem *bc, Zint::QZintXdimDpVars *vars, double originalScale)
        : m_bc(bc), Valid(false), m_vars(*vars), m_originalScale(originalScale), m_unset(false)
{
    setupUi(this);

    if (m_bc->bc.noPng()) {
        cmbFileType->setItemText(0, QSL("Raster (BMP/GIF/PCX/TIF)"));
    }
    if (m_bc->bc.symbol() != BARCODE_MAXICODE) {
        cmbFileType->setItemText(1, QSL("Vector (EMF/EPS/SVG)"));
        cmbFileType->removeItem(2); // EMF
    }

    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif

    QByteArray geometry = settings.value(QSL("studio/scale/window_geometry")).toByteArray();
    restoreGeometry(geometry);

    spnXdim->setValue(m_vars.x_dim);
    cmbXdimUnits->setCurrentIndex(m_vars.x_dim_units);
    spnResolution->setValue(m_vars.resolution);
    cmbResolutionUnits->setCurrentIndex(m_vars.resolution_units);

    if (m_bc->bc.symbol() == BARCODE_MAXICODE) {
        cmbFileType->setCurrentIndex(std::min(m_vars.filetype_maxicode, cmbFileType->count() - 1));
    } else {
        cmbFileType->setCurrentIndex(std::min(m_vars.filetype, cmbFileType->count() - 1));
    }

    if (cmbXdimUnits->currentIndex() == 1) { // Inches
        spnXdim->setSingleStep(0.001);
    } else {
        spnXdim->setSingleStep(0.01);
    }
    if (cmbResolutionUnits->currentIndex() == 1) { // Inches
        spnResolution->setSingleStep(50);
    } else {
        spnResolution->setSingleStep(1);
    }
    set_maxima();

    size_msg_ui_set();

    QIcon closeIcon(QIcon::fromTheme(QSL("window-close"), QIcon(QSL(":res/x.svg"))));
    QIcon unsetIcon(QSL(":res/delete.svg"));
    QIcon okIcon(QIcon(QSL(":res/check.svg")));
    btnCancel->setIcon(closeIcon);
    btnScaleUnset->setIcon(unsetIcon);
    btnScaleUnset->setEnabled(m_vars.set);
    btnOK->setIcon(okIcon);

    connect(btnCancel, SIGNAL(clicked(bool)), SLOT(close()));
    connect(btnScaleUnset, SIGNAL(clicked(bool)), SLOT(unset_scale()));
    connect(btnOK, SIGNAL(clicked(bool)), SLOT(okay()));
    connect(spnXdim, SIGNAL(valueChanged(double)), SLOT(update_scale()));
    connect(cmbXdimUnits, SIGNAL(currentIndexChanged(int)), SLOT(x_dim_units_change()));
    connect(btnXdimDefault, SIGNAL(clicked(bool)), SLOT(x_dim_default()));
    connect(spnResolution, SIGNAL(valueChanged(int)), SLOT(update_scale()));
    connect(cmbResolutionUnits, SIGNAL(currentIndexChanged(int)), SLOT(resolution_units_change()));
    connect(btnResolutionDefault, SIGNAL(clicked(bool)), SLOT(resolution_default()));
    connect(cmbFileType, SIGNAL(currentIndexChanged(int)), SLOT(update_scale()));
}

ScaleWindow::~ScaleWindow()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    settings.setValue(QSL("studio/scale/window_geometry"), saveGeometry());
}

void ScaleWindow::size_msg_ui_set()
{
    msgPrintingScale->setText(QString::asprintf("%.2f", m_bc->bc.scale()));
    float width_x_dim, height_x_dim;
    if (m_bc->bc.getWidthHeightXdim((float) m_vars.x_dim, width_x_dim, height_x_dim)) {
        const char *fmt = cmbXdimUnits->currentIndex() == 1 ? "%.3f x %.3f in" : "%.2f x %.2f mm";
        msgWidthHeight->setText(QString::asprintf(fmt, width_x_dim, height_x_dim));
    } else {
        msgWidthHeight->clear();
    }

    bool defaultDisable;
    QWidget *focus = QApplication::focusWidget();

    // This doesn't work well due to rounding errors TODO: fix
    defaultDisable = m_bc->bc.defaultXdim() == (float) get_x_dim_mm();
    btnXdimDefault->setEnabled(!defaultDisable);
    if (focus == btnXdimDefault && defaultDisable) {
        spnXdim->setFocus();
    }

    defaultDisable = (cmbResolutionUnits->currentIndex() == 0 && spnResolution->value() == 12)
                        || (cmbResolutionUnits->currentIndex() == 1 && spnResolution->value() == 300);
    btnResolutionDefault->setEnabled(!defaultDisable);
    if (focus == btnResolutionDefault && defaultDisable) {
        spnResolution->setFocus();
    }
}

void ScaleWindow::unset_scale()
{
    m_vars.x_dim = std::min(m_bc->bc.getXdimDpFromScale(m_originalScale, get_dpmm(), getFileType()), 10.0f);
    m_vars.set = 0;

    if (cmbXdimUnits->currentIndex() == 1) { // Inches
        spnXdim->setValue(m_vars.x_dim / 25.4);
    } else {
        spnXdim->setValue(m_vars.x_dim);
    }
    m_unset = true;
    btnScaleUnset->setEnabled(false);
}

void ScaleWindow::okay()
{
    if (update_vars()) {
        Valid = true;
        m_vars.set = m_unset ? 0 : 1;
    }
    close();
}

void ScaleWindow::update_scale()
{
    float scale = update_vars();
    if (scale) {
        // Need up-to-date `vectorWidth()` and `vectorHeight()` to estimate size including borders, whitespace & text,
        // so tell main window to encode and it will update UI here via `size_msg_ui_set()`
        emit scaleChanged(scale);
        m_unset = false;
        btnScaleUnset->setEnabled(true);
        set_maxima();
    }
}

void ScaleWindow::x_dim_units_change()
{
    if (cmbXdimUnits->currentIndex() == 1) { // Inches
        spnXdim->setValue(spnXdim->value() / 25.4);
        spnXdim->setSingleStep(0.001);
    } else {
        spnXdim->setValue(spnXdim->value() * 25.4);
        spnXdim->setSingleStep(0.01);
    }
    update_scale();
}

void ScaleWindow::x_dim_default()
{
    if (cmbXdimUnits->currentIndex() == 1) { // Inches
        spnXdim->setValue(m_bc->bc.defaultXdim() / 25.4);
    } else {
        spnXdim->setValue(m_bc->bc.defaultXdim());
    }
    update_scale();
}

void ScaleWindow::resolution_units_change()
{
    int value;
    if (cmbResolutionUnits->currentIndex() == 1) { // Inches
        if ((value = resolution_standard(0, spnResolution->value()))) {
            spnResolution->setValue(value);
        } else {
            spnResolution->setValue((int) roundf(spnResolution->value() * 25.4f));
        }
        spnResolution->setSingleStep(50);
    } else {
        if ((value = resolution_standard(1, spnResolution->value()))) {
            spnResolution->setValue(value);
        } else {
            spnResolution->setValue((int) roundf(spnResolution->value() / 25.4f));
        }
        spnResolution->setSingleStep(1);
    }
    update_scale();
}

void ScaleWindow::resolution_default()
{
    spnResolution->setValue(cmbResolutionUnits->currentIndex() == 1 ? 300 : 12);
    update_scale();
}

float ScaleWindow::get_x_dim_mm() const
{
    return (float) (cmbXdimUnits->currentIndex() == 1 ? spnXdim->value() * 25.4 : spnXdim->value());

}

float ScaleWindow::get_dpmm() const
{
    return (float) (cmbResolutionUnits->currentIndex() == 1 ? spnResolution->value() / 25.4 : spnResolution->value());
}

const char *ScaleWindow::getFileType() const
{
    static const char *filetypes[3] = { "GIF", "SVG", "EMF" };
    return filetypes[std::max(std::min(cmbFileType->currentIndex(), 2), 0)];
}

void ScaleWindow::set_maxima()
{
    float maxXdim = std::min(m_bc->bc.getXdimDpFromScale(200.0f, get_dpmm(), getFileType()), 10.0f);
    if (cmbXdimUnits->currentIndex() == 1) { // Inches
        spnXdim->setMaximum(maxXdim / 25.4);
    } else {
        spnXdim->setMaximum(maxXdim);
    }
    float maxRes = m_bc->bc.getXdimDpFromScale(200.0f, get_x_dim_mm(), getFileType());
    if (cmbResolutionUnits->currentIndex() == 1) { // Inches
        spnResolution->setMaximum(maxRes * 25.4);
    } else {
        spnResolution->setMaximum(maxRes);
    }
}

double ScaleWindow::update_vars()
{
    double scale = (double) m_bc->bc.getScaleFromXdimDp(get_x_dim_mm(), get_dpmm(), getFileType());
    if (scale != 0.0) {
        m_vars.x_dim = spnXdim->value();
        m_vars.x_dim_units = cmbXdimUnits->currentIndex();
        m_vars.resolution = spnResolution->value();
        m_vars.resolution_units = cmbResolutionUnits->currentIndex();
        if (m_bc->bc.symbol() == BARCODE_MAXICODE) {
            m_vars.filetype_maxicode = cmbFileType->currentIndex();
        } else {
            m_vars.filetype = cmbFileType->currentIndex();
        }
    }
    return scale;
}

/* vim: set ts=4 sw=4 et : */
