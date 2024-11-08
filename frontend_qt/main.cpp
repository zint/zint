/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra <bogdan@licentia.eu>               *
 *   Copyright (C) 2009-2024 by Robin Stuart <rstuart114@gmail.com>        *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

#if defined(__linux__) && QT_VERSION >= 0x50100 /* `qEnvironmentVariableIsEmpty()` introduced Qt 5.1 */
    /* Not compatible with Wayland for some reason(s) so use X11 unless overridden */
    if (QGuiApplication::platformName() != "xcb" && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("xcb"));
    }
#endif

#if QT_VERSION >= 0x50600 && QT_VERSION < 0x60100
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#if QT_VERSION >= 0x50400
    /* Suppresses "Qt WebEngine seems to be initialized from a plugin" warning */
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#endif
#if defined(_WIN32) && QT_VERSION >= 0x50A01 && QT_VERSION < 0x60000
    /* Suppresses help question mark in dialogs */
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

    QApplication app(argc, argv);

    MainWindow w;
    // Seem to need to do this before showing the window
    w.setWindowTitle(w.windowTitle() + ' ' + MainWindow::get_zint_version());
    w.show();
    return app.exec();
}

/* vim: set ts=4 sw=4 et : */
