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
#include <QRegularExpression>

#include "datawindow.h"

// Shorthand
#define QSL QStringLiteral

static const int tempMessageTimeout = 2000;

DataWindow::DataWindow(const QString &input, bool isEscaped) : Valid(false), Escaped(false)
{
    setupUi(this);
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif

    QByteArray geometry = settings.value(QSL("studio/data/window_geometry")).toByteArray();
    restoreGeometry(geometry);

    QIcon closeIcon(QIcon::fromTheme(QSL("window-close"), QIcon(QSL(":res/x.svg"))));
    QIcon clearIcon(QIcon::fromTheme(QSL("edit-clear"), QIcon(QSL(":res/delete.svg"))));
    QIcon okIcon(QIcon(QSL(":res/check.svg")));
    btnCancel->setIcon(closeIcon);
    btnDataClear->setIcon(clearIcon);
    btnOK->setIcon(okIcon);

    if (isEscaped && input.contains(QSL("\\n"))) {
        // Substitute escaped Line Feeds with actual Line Feeds
        QString out;
        out.reserve(input.length());
        int lastPosn = 0;
        QRegularExpression escRE(QSL("\\\\(?:[0EabtnvfreGR\\\\]|x[0-9A-Fa-f]{2}|u[0-9A-Fa-f]{4})"));
        QRegularExpressionMatchIterator matchI = escRE.globalMatch(input);
        while (matchI.hasNext()) {
            QRegularExpressionMatch match = matchI.next();
            if (match.captured(0) == QSL("\\n")) {
                out += input.mid(lastPosn, match.capturedStart(0) - lastPosn) + '\n';
                lastPosn = match.capturedEnd(0);
            }
        }
        out += input.mid(lastPosn);
        txtDataInput->setPlainText(out);
        statusBarData->showMessage(tr("Converted LFs"), tempMessageTimeout);
    } else {
        txtDataInput->setPlainText(input);
    }
    txtDataInput->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);

    connect(btnCancel, SIGNAL( clicked( bool )), SLOT(close()));
    connect(btnDataClear, SIGNAL( clicked( bool )), SLOT(clear_data()));
    connect(btnOK, SIGNAL( clicked( bool )), SLOT(okay()));
    connect(btnFromFile, SIGNAL( clicked( bool )), SLOT(from_file()));
}

DataWindow::~DataWindow()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    settings.setValue(QSL("studio/data/window_geometry"), saveGeometry());
}

void DataWindow::clear_data()
{
    txtDataInput->clear();
}

void DataWindow::okay()
{
    Valid = true;
    DataOutput = txtDataInput->toPlainText();
    if (DataOutput.contains('\n')) {
        // Escape Line Feeds
        DataOutput.replace('\n', QSL("\\n"));
        Escaped = true;
    }
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
    open_dialog.setWindowTitle("Import File");
    open_dialog.setDirectory(settings.value("studio/default_dir",
                QDir::toNativeSeparators(QDir::homePath())).toString());

    if (open_dialog.exec()) {
        filename = open_dialog.selectedFiles().at(0);
    } else {
        return;
    }

    file.setFileName(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Open Error"), tr("Could not open selected file."));
        return;
    }

    outstream = file.readAll();

    /* Allow some non-printing (control) characters to be read from file
       by converting them to escape sequences */
    QString escape_string(outstream); // Converts to UTF-8 (NOTE: QString can't handle embedded NULs)

    QRegularExpression escRE(QSL("[\\x04\\x07\\x08\\x09\\x0B\\x0C\\x0D\\x1B\\x1D\\x1E\\x5C]"));
    if (escape_string.contains(escRE)) {
        escape_string.replace((QChar)'\\', QSL("\\\\"));
        escape_string.replace((QChar)0x04, QSL("\\E")); /* End of Transmission */
        escape_string.replace((QChar)'\a', QSL("\\a")); /* Bell (0x07) */
        escape_string.replace((QChar)'\b', QSL("\\b")); /* Backspace (0x08) */
        escape_string.replace((QChar)'\t', QSL("\\t")); /* Horizontal tab (0x09) */
        // Leaving Line Feed (0x0A)
        escape_string.replace((QChar)'\v', QSL("\\v")); /* Vertical tab (0x0B) */
        escape_string.replace((QChar)'\f', QSL("\\f")); /* Form feed (0x0C) */
        escape_string.replace((QChar)'\r', QSL("\\r")); /* Carriage return (0x0D) */
        escape_string.replace((QChar)0x1b, QSL("\\e")); /* Escape */
        escape_string.replace((QChar)0x1d, QSL("\\G")); /* Group Separator */
        escape_string.replace((QChar)0x1e, QSL("\\R")); /* Record Separator */
        Escaped = true;
        statusBarData->showMessage(tr("Escaped data"), tempMessageTimeout);
    }

    txtDataInput->setPlainText(escape_string);
    file.close();

    settings.setValue("studio/default_dir", filename.mid(0, filename.lastIndexOf(QDir::separator())));
}
