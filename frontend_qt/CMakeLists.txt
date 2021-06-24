# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2021 Robin Stuart <rstuart114@gmail.com>
# vim: set ts=4 sw=4 et :

project(zint-qt)

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/backend" "${CMAKE_SOURCE_DIR}/backend_qt")

set(zint-qt_SRCS barcodeitem.cpp main.cpp mainwindow.cpp datawindow.cpp sequencewindow.cpp exportwindow.cpp)

if(USE_QT6)
    qt6_wrap_cpp(zint-qt_SRCS mainwindow.h datawindow.h sequencewindow.h exportwindow.h)
    qt6_wrap_ui(zint-qt_SRCS mainWindow.ui extData.ui extSequence.ui extExport.ui)
    qt6_add_resources(zint-qt_SRCS resources.qrc)
else()
    qt5_wrap_cpp(zint-qt_SRCS mainwindow.h datawindow.h sequencewindow.h exportwindow.h)
    qt5_wrap_ui(zint-qt_SRCS mainWindow.ui extData.ui extSequence.ui extExport.ui)
    qt5_add_resources(zint-qt_SRCS resources.qrc)
endif()
#               grpAztec.ui  grpC39.ui         grpCodeOne.ui   grpGrid.ui      grpMQR.ui       grpUltra.ui
#               grpC11.ui    grpC49.ui         grpDAFT.ui      grpHX.ui        grpMSICheck.ui  grpUPCA.ui
#               grpC128.ui   grpChannel.ui     grpDBExtend.ui  grpLOGMARS.ui   grpPDF417.ui    grpUPCEAN.ui
#               grpC16k.ui   grpCodabar.ui     grpDM.ui        grpMaxicode.ui  grpQR.ui        grpVIN.ui
#               grpC25.ui    grpCodablockF.ui  grpDotCode.ui   grpMicroPDF.ui  grpRMQR.ui

add_executable(zint-qt ${zint-qt_SRCS})
add_dependencies(zint-qt zint QZint)

link_directories("${CMAKE_BINARY_DIR}/backend" "${CMAKE_BINARY_DIR}/backend_qt")

if(USE_QT6)
    target_link_libraries(zint-qt zint QZint Qt6::UiTools Qt6::Xml Qt6::Gui Qt6::Core)
else()
    target_link_libraries(zint-qt zint QZint Qt5::UiTools Qt5::Xml Qt5::Gui Qt5::Core)
endif()

install(TARGETS zint-qt DESTINATION "${BIN_INSTALL_DIR}" RUNTIME)
