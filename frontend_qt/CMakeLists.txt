# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2021 Robin Stuart <rstuart114@gmail.com>
# vim: set ts=4 sw=4 et :

project(zint-qt)

set(CMAKE_AUTORCC ON)

set(${PROJECT_NAME}_SRCS barcodeitem.cpp main.cpp mainwindow.cpp datawindow.cpp sequencewindow.cpp exportwindow.cpp)

if(USE_QT6)
    qt6_wrap_cpp(zint-qt_SRCS mainwindow.h datawindow.h sequencewindow.h exportwindow.h)
    qt6_wrap_ui(zint-qt_SRCS mainWindow.ui extData.ui extSequence.ui extExport.ui)
else()
    qt5_wrap_cpp(zint-qt_SRCS mainwindow.h datawindow.h sequencewindow.h exportwindow.h)
    qt5_wrap_ui(zint-qt_SRCS mainWindow.ui extData.ui extSequence.ui extExport.ui)
endif()

#               grpAztec.ui  grpC39.ui         grpCodeOne.ui   grpGrid.ui      grpMQR.ui       grpUltra.ui
#               grpC11.ui    grpC49.ui         grpDAFT.ui      grpHX.ui        grpMSICheck.ui  grpUPCA.ui
#               grpC128.ui   grpChannel.ui     grpDBExtend.ui  grpLOGMARS.ui   grpPDF417.ui    grpUPCEAN.ui
#               grpC16k.ui   grpCodabar.ui     grpDM.ui        grpMaxicode.ui  grpQR.ui        grpVIN.ui
#               grpC25.ui    grpCodablockF.ui  grpDotCode.ui   grpMicroPDF.ui  grpRMQR.ui

add_executable(${PROJECT_NAME} ${zint-qt_SRCS} resources.qrc)

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE res/qtZint.rc)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/backend" "${CMAKE_SOURCE_DIR}/backend_qt")

target_link_libraries(${PROJECT_NAME} zint QZint Qt${QT_VERSION_MAJOR}::UiTools Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Core)


install(TARGETS ${PROJECT_NAME} DESTINATION "${BIN_INSTALL_DIR}" RUNTIME)
