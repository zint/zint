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
#include <QFileDialog>
#include <QUiLoader>
#include <QStringList>
#include <QMessageBox>
#include <QSettings>

#include "datawindow.h"

DataWindow::DataWindow()
{
    setupUi(this);

    connect(btnCancel, SIGNAL( clicked( bool )), SLOT(quit_now()));
    connect(btnReset, SIGNAL( clicked( bool )), SLOT(clear_data()));
    connect(btnOK, SIGNAL( clicked( bool )), SLOT(okay()));
}

DataWindow::DataWindow(const QString &input)
{
    setupUi(this);
    txtDataInput->setPlainText(input);
    txtDataInput->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);

    connect(btnCancel, SIGNAL( clicked( bool )), SLOT(quit_now()));
    connect(btnReset, SIGNAL( clicked( bool )), SLOT(clear_data()));
    connect(btnOK, SIGNAL( clicked( bool )), SLOT(okay()));
    connect(btnFromFile, SIGNAL( clicked( bool )), SLOT(from_file()));
}

DataWindow::~DataWindow()
{
}

void DataWindow::quit_now()
{
    Valid = 0;
    close();
}

void DataWindow::clear_data()
{
    txtDataInput->clear();
}

void DataWindow::okay()
{
    Valid = 1;
    DataOutput = txtDataInput->toPlainText();
    close();
}

void DataWindow::from_file()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    QFileDialog open_dialog;
    QString filename;
    QFile file;
    QByteArray outstream;
    QString escape_string;

    open_dialog.setWindowTitle("Open File");
    open_dialog.setDirectory(settings.value("studio/default_dir", QDir::toNativeSeparators(QDir::homePath())).toString());

    if (open_dialog.exec()) {
        filename = open_dialog.selectedFiles().at(0);
    } else {
        return;
    }

    file.setFileName(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Open Error"), tr("Could not open selected file."));
        return;
    }

    outstream = file.readAll();

    /* Allow some non-printing (control) characters to be read from file
       by converting them to escape sequences */
    escape_string.clear();
    escape_string.append(QString(outstream));

    escape_string.replace((QChar)'\\', (QString)"\\\\", Qt::CaseInsensitive);
    escape_string.replace((QChar)0x04, (QString)"\\E", Qt::CaseInsensitive); /* End of Transmission */
    escape_string.replace((QChar)'\a', (QString)"\\a", Qt::CaseInsensitive); /* Bell */
    escape_string.replace((QChar)'\b', (QString)"\\b", Qt::CaseInsensitive); /* Backspace */
    escape_string.replace((QChar)'\t', (QString)"\\t", Qt::CaseInsensitive); /* Horizontal tab */
    escape_string.replace((QChar)'\v', (QString)"\\v", Qt::CaseInsensitive); /* Vertical tab */
    escape_string.replace((QChar)'\f', (QString)"\\f", Qt::CaseInsensitive); /* Form feed */
    escape_string.replace((QChar)'\r', (QString)"\\r", Qt::CaseInsensitive); /* Carriage return */
    escape_string.replace((QChar)0x1b, (QString)"\\e", Qt::CaseInsensitive); /* Escape */
    escape_string.replace((QChar)0x1d, (QString)"\\G", Qt::CaseInsensitive); /* Group Separator */
    escape_string.replace((QChar)0x1e, (QString)"\\R", Qt::CaseInsensitive); /* Record Separator */

    txtDataInput->setPlainText(QString(escape_string));
    file.close();

    settings.setValue("studio/default_dir", filename.mid(0, filename.lastIndexOf(QDir::separator())));
}
