/*
    Zint Barcode Generator - the open source barcode generator
    Copyright (C) 2009 - 2021 Robin Stuart <rstuart114@gmail.com>

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
/* vim: set ts=4 sw=4 et : */

//#include <QDebug>
#include <QFile>
#include <QUiLoader>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "sequencewindow.h"
#include "exportwindow.h"

SequenceWindow::SequenceWindow()
{
    setupUi(this);
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    QValidator *intvalid = new QIntValidator(this);

    linStartVal->setText(settings.value("studio/sequence/start_value", "1").toString());
    linEndVal->setText(settings.value("studio/sequence/end_value", "10").toString());
    linIncVal->setText(settings.value("studio/sequence/increment", "1").toString());
    linFormat->setText(settings.value("studio/sequence/format", "$$$$$$").toString());

    linStartVal->setValidator(intvalid);
    linEndVal->setValidator(intvalid);
    linIncVal->setValidator(intvalid);
    connect(btnClose, SIGNAL( clicked( bool )), SLOT(quit_now()));
    connect(btnReset, SIGNAL( clicked( bool )), SLOT(reset_preview()));
    connect(btnCreate, SIGNAL( clicked( bool )), SLOT(create_sequence()));
    connect(txtPreview, SIGNAL( textChanged()), SLOT(check_generate()));
    connect(btnImport, SIGNAL( clicked( bool )), SLOT(import()));
    connect(btnExport, SIGNAL( clicked( bool )), SLOT(generate_sequence()));
}

SequenceWindow::~SequenceWindow()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif

    settings.setValue("studio/sequence/start_value", linStartVal->text());
    settings.setValue("studio/sequence/end_value", linEndVal->text());
    settings.setValue("studio/sequence/increment", linIncVal->text());
    settings.setValue("studio/sequence/format", linFormat->text());
}

void SequenceWindow::quit_now()
{
    close();
}

void SequenceWindow::reset_preview()
{
    txtPreview->clear();
}

QString SequenceWindow::apply_format(QString raw_number)
{
    QString adjusted, reversed;
    QString format;
    int format_len, input_len, i, inpos;
    QChar format_qchar;

    format = linFormat->text();
    input_len = raw_number.length();
    format_len = format.length();

    inpos = input_len;

    for(i = format_len; i > 0; i--) {
        format_qchar = format[i - 1];
        char format_char = format_qchar.toLatin1();
        switch(format_char) {
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

    for(i = format_len; i > 0; i--) {
        reversed += adjusted[i - 1];
    }

    return reversed;
}

void SequenceWindow::create_sequence()
{
    QString startval, endval, incval, part, outputtext;
    int start, stop, step, i;
    bool ok;

    startval = linStartVal->text();
    endval = linEndVal->text();
    incval = linIncVal->text();
    start = startval.toInt(&ok, 10);
    stop = endval.toInt(&ok, 10);
    step = incval.toInt(&ok, 10);

    if((stop <= start) || (step <= 0)) {
        QMessageBox::critical(this, tr("Sequence Error"), tr("One or more of the input values is incorrect."));
        return;
    }

    for(i = start; i <= stop; i += step) {
        part = apply_format(QString::number(i, 10));
        part += '\n';
        outputtext += part;
    }

    txtPreview->setPlainText(outputtext);
}

void SequenceWindow::check_generate()
{
    QString preview_copy;

    preview_copy = txtPreview->toPlainText();
    if(preview_copy.isEmpty()) {
        btnExport->setEnabled(false);
        lblExport->setEnabled(false);
    } else {
        btnExport->setEnabled(true);
        lblExport->setEnabled(true);
    }
}

void SequenceWindow::import()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    QFileDialog import_dialog;
    QString filename;
    QFile file;
    QByteArray outstream;

    import_dialog.setWindowTitle("Import File");
    import_dialog.setDirectory(settings.value("studio/default_dir", QDir::toNativeSeparators(QDir::homePath())).toString());

    if (import_dialog.exec()) {
        filename = import_dialog.selectedFiles().at(0);
    } else {
        return;
    }

    file.setFileName(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Open Error"), tr("Could not open selected file."));
        return;
    }

    outstream = file.readAll();

    txtPreview->setPlainText(QString(outstream));
    file.close();

    settings.setValue("studio/default_dir", filename.mid(0, filename.lastIndexOf(QDir::separator())));
}

void SequenceWindow::generate_sequence()
{
    ExportWindow dlg;
    dlg.barcode = barcode;
    dlg.output_data = txtPreview->toPlainText();
    dlg.exec();
}
