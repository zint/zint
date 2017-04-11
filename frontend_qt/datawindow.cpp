/*
    Zint Barcode Generator - the open source barcode generator
    Copyright (C) 2009-2016 Robin Stuart <rstuart114@gmail.com>

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

#include <QDebug>
#include <QFileDialog>
#include <QUiLoader>
#include <QStringList>
#include <QMessageBox>

#include "datawindow.h"
#include <stdio.h>

DataWindow::DataWindow()
{
	setupUi(this);

	connect(btnCancel, SIGNAL( clicked( bool )), SLOT(quit_now()));
	connect(btnReset, SIGNAL( clicked( bool )), SLOT(clear_data()));
	connect(btnOK, SIGNAL( clicked( bool )), SLOT(okay()));
}

DataWindow::DataWindow(QString input)
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
	//QString fileName;
	//QFileDialog fdialog;
	QFile file;
	
	//fdialog.setFileMode(QFileDialog::ExistingFile);
	//
	//if(fdialog.exec()) {
	//	fileName = fdialog.selectedFiles().at(0);
	//} else {
	//	return;
	//}

	QString fileName = QFileDialog::getOpenFileName(this,
                                 tr("Open File"),
                                 "./",
                                 tr("All Files (*);;Text Files (*.txt)"));
     if (fileName.isEmpty())
         return;

	file.setFileName(fileName);
	if(!file.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(this, tr("Open Error"), tr("Could not open selected file."));
		return;
	}
	
	QByteArray outstream = file.readAll();
	
	txtDataInput->setPlainText(QString(outstream));
	file.close();
}