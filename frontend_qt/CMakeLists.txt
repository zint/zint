# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2024 Robin Stuart <rstuart114@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later
# vim: set ts=4 sw=4 et :

cmake_minimum_required(VERSION 3.10)
project(zint-qt)

set(CMAKE_AUTORCC ON)

set(${PROJECT_NAME}_SRCS barcodeitem.cpp main.cpp mainwindow.cpp
                            cliwindow.cpp datawindow.cpp scalewindow.cpp sequencewindow.cpp exportwindow.cpp)

if(USE_QT6)
    qt6_wrap_cpp(zint-qt_SRCS mainwindow.h cliwindow.h datawindow.h scalewindow.h sequencewindow.h exportwindow.h)
    qt6_wrap_ui(zint-qt_SRCS mainWindow.ui extCLI.ui extData.ui extScale.ui extSequence.ui extExport.ui)
else()
    qt5_wrap_cpp(zint-qt_SRCS mainwindow.h cliwindow.h datawindow.h scalewindow.h sequencewindow.h exportwindow.h)
    qt5_wrap_ui(zint-qt_SRCS mainWindow.ui extCLI.ui extData.ui extScale.ui extSequence.ui extExport.ui)
endif()

#           grpAztec.ui  grpC39.ui      grpCodablockF.ui  grpDotCode.ui  grpMailmark2D.ui  grpPDF417.ui  grpUPCA.ui
#           grpC11.ui    grpC49.ui      grpCodeOne.ui     grpDPD.ui      grpMaxicode.ui    grpPZN.ui     grpUPCEAN.ui
#           grpC128.ui   grpC93.ui      grpDAFT.ui        grpGrid.ui     grpMicroPDF.ui    grpQR.ui      grpUPNQR.ui
#           grpC16k.ui   grpChannel.ui  grpDBExtend.ui    grpHX.ui       grpMQR.ui         grpRMQR.ui    grpVIN.ui
#           grpC25.ui    grpCodabar.ui  grpDM.ui          grpITF14.ui    grpMSICheck.ui    grpUltra.ui

if(APPLE)
    # https://doc.qt.io/qt-5/appicon.html
    set(MACOSX_BUNDLE_ICON_FILE zint-qt.icns)
    set(APP_ICON_MACOSX "${CMAKE_CURRENT_SOURCE_DIR}/zint-qt.icns")
    set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${zint-qt_SRCS} resources.qrc ${APP_ICON_MACOSX})
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Zint Barcode Studio"
        MACOSX_BUNDLE_BUNDLE_VERSION ${ZINT_VERSION}
        MACOSX_BUNDLE_COPYRIGHT "Copyright © 2006-2022 Robin Stuart and others"
        MACOSX_BUNDLE_GUI_IDENTIFIER "uk.org.zint.zint-qt"
        MACOSX_BUNDLE_INFO_STRING "A free barcode generator"
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${ZINT_VERSION})
else()
    add_executable(${PROJECT_NAME} ${zint-qt_SRCS} resources.qrc)
endif()

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE res/qtZint.rc)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../backend" "${CMAKE_CURRENT_SOURCE_DIR}/../backend_qt")

if(ZINT_SHARED)
    target_link_libraries(${PROJECT_NAME} zint)
else()
    target_link_libraries(${PROJECT_NAME} zint-static)
endif()
target_link_libraries(${PROJECT_NAME} QZint
                        Qt${QT_VERSION_MAJOR}::UiTools Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Svg
                        Qt${QT_VERSION_MAJOR}::Core)


install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}" RUNTIME)
