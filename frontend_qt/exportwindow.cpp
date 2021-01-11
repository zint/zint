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
#include <QUiLoader>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "exportwindow.h"

ExportWindow::ExportWindow()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    setupUi(this);

    linDestPath->setText(settings.value("studio/export/destination", QDir::toNativeSeparators(QDir::homePath())).toString());
    linPrefix->setText(settings.value("studio/export/file_prefix", "bcs_").toString());
    cmbFileName->setCurrentIndex(settings.value("studio/export/name_format", 0).toInt());
    cmbFileFormat->setCurrentIndex(settings.value("studio/export/filetype", 0).toInt());

    connect(btnCancel, SIGNAL( clicked( bool )), SLOT(quit_now()));
    connect(btnOK, SIGNAL( clicked( bool )), SLOT(process()));
    connect(btnDestPath, SIGNAL( clicked( bool )), SLOT(get_directory()));
}

ExportWindow::~ExportWindow()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif

    settings.setValue("studio/export/destination", linDestPath->text());
    settings.setValue("studio/export/file_prefix", linPrefix->text());
    settings.setValue("studio/export/name_format", cmbFileName->currentIndex());
    settings.setValue("studio/export/filetype", cmbFileFormat->currentIndex());
}

void ExportWindow::quit_now()
{
    close();
}

void ExportWindow::get_directory()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    QString directory;
    QFileDialog fdialog;

    fdialog.setFileMode(QFileDialog::Directory);
    fdialog.setDirectory(settings.value("studio/default_dir", QDir::toNativeSeparators(QDir::homePath())).toString());

    if(fdialog.exec()) {
        directory = fdialog.selectedFiles().at(0);
    } else {
        return;
    }

    linDestPath->setText(QDir::toNativeSeparators(directory));
    settings.setValue("studio/default_dir", directory);
}

void ExportWindow::process()
{
    QString fileName;
    QString dataString;
    QString suffix;
    QString Feedback;
    int lines, i, j, inputpos;

    lines = output_data.count(QChar('\n'), Qt::CaseInsensitive);
    inputpos = 0;

    switch(cmbFileFormat->currentIndex()) {
#ifdef NO_PNG
        case 0: suffix = ".eps"; break;
        case 1: suffix = ".gif"; break;
        case 2: suffix = ".svg"; break;
        case 3: suffix = ".bmp"; break;
        case 4: suffix = ".pcx"; break;
        case 5: suffix = ".emf"; break;
        case 6: suffix = ".tif"; break;
#else
        case 0: suffix = ".png"; break;
        case 1: suffix = ".eps"; break;
        case 2: suffix = ".gif"; break;
        case 3: suffix = ".svg"; break;
        case 4: suffix = ".bmp"; break;
        case 5: suffix = ".pcx"; break;
        case 6: suffix = ".emf"; break;
        case 7: suffix = ".tif"; break;
#endif
    }
    txtFeedback->clear();
    Feedback = "";

    for(i = 0; i < lines; i++) {
        int datalen = 0;
        for(j = inputpos; ((j < output_data.length()) && (output_data[j] != '\n') ); j++) {
            datalen++;
        }
        dataString = output_data.mid(inputpos, datalen);
        switch(cmbFileName->currentIndex()) {
            case 0: { /* Same as Data (URL Escaped) */
                    QString url_escaped;
                    int m;
                    QChar name_qchar;

                    for(m = 0; m < dataString.length(); m++) {
                        name_qchar = dataString[m];
                        char name_char = name_qchar.toLatin1();

                        switch(name_char) {
                            case '\\': url_escaped += "%5C"; break;
                            case '/': url_escaped += "%2F"; break;
                            case ':': url_escaped += "%3A"; break;
                            case '*': url_escaped += "%2A"; break;
                            case '?': url_escaped += "%3F"; break;
                            case '"': url_escaped += "%22"; break;
                            case '<': url_escaped += "%3C"; break;
                            case '>': url_escaped += "%3E"; break;
                            case '|': url_escaped += "%7C"; break;
                            case '%': url_escaped += "%25"; break;
                            default: url_escaped += name_qchar; break;
                        }
                    }
                    fileName = linDestPath->text() + QDir::separator() + linPrefix->text() + url_escaped + suffix;
                }
                break;
            case 1: { /* Formatted Serial Number */
                    QString biggest, this_val, outnumber;
                    int number_size, val_size, m;

                    biggest = QString::number(lines + 1);
                    number_size = biggest.length();
                    this_val = QString::number(i + 1);
                    val_size = this_val.length();

                    for(m = 0; m < (number_size - val_size); m++) {
                        outnumber += QChar('0');
                    }

                    outnumber += this_val;

                    fileName = linDestPath->text() + QDir::separator() + linPrefix->text() + outnumber + suffix;
                }
                break;
        }
        barcode->bc.setText(dataString.toLatin1().data());
        barcode->bc.save_to_file(fileName.toLatin1().data());
        Feedback += "Line ";
        Feedback += QString::number(i + 1);
        Feedback += ": ";
        if (barcode->bc.hasErrors()) {
            Feedback += barcode->bc.error_message();
            Feedback += "\n";
        } else {
            Feedback += "Success\n";
        }
        txtFeedback->document()->setPlainText(Feedback);
        inputpos += datalen + 1;
    }
}
