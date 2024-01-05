/*
    Zint Barcode Generator - the open source barcode generator
    Copyright (C) 2009-2024 Robin Stuart <rstuart114@gmail.com>

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
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QUiLoader>

#include "sequencewindow.h"
#include "exportwindow.h"

// Shorthand
#define QSL     QStringLiteral
#define QSEmpty QLatin1String("")

SequenceWindow::SequenceWindow(BarcodeItem *bc) : m_bc(bc)
{
    setupUi(this);
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif

    QByteArray geometry = settings.value(QSL("studio/sequence/window_geometry")).toByteArray();
    restoreGeometry(geometry);

    spnSeqStartVal->setValue(settings.value(QSL("studio/sequence/start_value"), 1).toInt());
    spnSeqEndVal->setValue(settings.value(QSL("studio/sequence/end_value"), 10).toInt());
    spnSeqIncVal->setValue(settings.value(QSL("studio/sequence/increment"), 1).toInt());
    linSeqFormat->setText(settings.value(QSL("studio/sequence/format"), QSL("$$$$$$")).toString());
    txtSeqPreview->setPlainText(settings.value(QSL("studio/sequence/preview"), QSEmpty).toString());

    QIcon closeIcon(QIcon::fromTheme(QSL("window-close"), QIcon(QSL(":res/x.svg"))));
    QIcon clearIcon(QSL(":res/delete.svg"));
    btnSeqClose->setIcon(closeIcon);
    btnSeqClear->setIcon(clearIcon);
    check_generate();

    connect(btnSeqClose, SIGNAL(clicked(bool)), SLOT(close()));
    connect(btnSeqClear, SIGNAL(clicked(bool)), SLOT(clear_preview()));
    connect(btnSeqCreate, SIGNAL(clicked(bool)), SLOT(create_sequence()));
    connect(txtSeqPreview, SIGNAL(textChanged()), SLOT(check_generate()));
    connect(btnSeqFromFile, SIGNAL(clicked(bool)), SLOT(from_file()));
    connect(btnSeqExport, SIGNAL(clicked(bool)), SLOT(generate_sequence()));
}

SequenceWindow::~SequenceWindow()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    settings.setValue(QSL("studio/sequence/window_geometry"), saveGeometry());

    settings.setValue(QSL("studio/sequence/start_value"), spnSeqStartVal->value());
    settings.setValue(QSL("studio/sequence/end_value"), spnSeqEndVal->value());
    settings.setValue(QSL("studio/sequence/increment"), spnSeqIncVal->value());
    settings.setValue(QSL("studio/sequence/format"), linSeqFormat->text());
    settings.setValue(QSL("studio/sequence/preview"), txtSeqPreview->toPlainText());
}

void SequenceWindow::clear_preview()
{
    txtSeqPreview->clear();
}

QString SequenceWindow::apply_format(const QString& raw_number)
{
    QString adjusted, reversed;
    QString format;
    int format_len, input_len, i, inpos;
    QChar format_qchar;

    format = linSeqFormat->text();
    input_len = raw_number.length();
    format_len = format.length();

    inpos = input_len;

    for (i = format_len; i > 0; i--) {
        format_qchar = format[i - 1];
        char format_char = format_qchar.toLatin1();
        switch (format_char) {
            case '#':
                if (inpos > 0) {
                    adjusted += raw_number[inpos - 1];
                    inpos--;
                } else {
                    adjusted += ' ';
                }
                break;
            case '$':
                if (inpos > 0) {
                    adjusted += raw_number[inpos - 1];
                    inpos--;
                } else {
                    adjusted += '0';
                }
                break;
            case '*':
                if (inpos > 0) {
                    adjusted += raw_number[inpos - 1];
                    inpos--;
                } else {
                    adjusted += '*';
                }
                break;
            default:
                adjusted += format_char;
                break;
        }
    }

    for (i = format_len; i > 0; i--) {
        reversed += adjusted[i - 1];
    }

    return reversed;
}

void SequenceWindow::create_sequence()
{
    QStringList outputtext;
    int start, stop, step, i;

    start = spnSeqStartVal->value();
    stop = spnSeqEndVal->value();
    step = spnSeqIncVal->value();

    if (stop < start) {
        QMessageBox::critical(this, tr("Sequence Error"),
                                tr("End Value must be greater than or equal to Start Value."));
        return;
    }
    if ((stop - start) / step > 10000) {
        QMessageBox::critical(this, tr("Sequence Error"), tr("The maximum sequence allowed is 10,000 items."));
        return;
    }

    for (i = start; i <= stop; i += step) {
        outputtext << apply_format(QString::number(i, 10));
    }

    txtSeqPreview->setPlainText(outputtext.join('\n'));
}

void SequenceWindow::check_generate()
{
    QString preview_copy;

    preview_copy = txtSeqPreview->toPlainText();
    if (preview_copy.isEmpty()) {
        btnSeqExport->setEnabled(false);
        btnSeqClear->setEnabled(false);
    } else {
        btnSeqExport->setEnabled(true);
        btnSeqClear->setEnabled(true);
    }
}

void SequenceWindow::from_file()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    QFileDialog import_dialog;
    QString filename;
    QFile file;
    QByteArray outstream;

    import_dialog.setWindowTitle(tr("Import File"));
    import_dialog.setDirectory(settings.value(QSL("studio/default_dir"),
                                QDir::toNativeSeparators(QDir::homePath())).toString());

    if (import_dialog.exec()) {
        filename = import_dialog.selectedFiles().at(0);
    } else {
        return;
    }

    file.setFileName(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Open Error"), tr("Could not open selected file."));
        return;
    }

    outstream = file.readAll();

    txtSeqPreview->setPlainText(QString(outstream));
    file.close();

    settings.setValue(QSL("studio/default_dir"), filename.mid(0, filename.lastIndexOf(QDir::separator())));
}

void SequenceWindow::generate_sequence()
{
    ExportWindow dlg(m_bc, txtSeqPreview->toPlainText());
    dlg.exec();
}

/* vim: set ts=4 sw=4 et : */
