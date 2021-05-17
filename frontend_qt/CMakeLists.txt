# (c) 2008 by BogDan Vatra < bogdan@licentia.eu >
# vim: set ts=4 sw=4 et :

project(zint-qt)

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/backend")

set(zint-qt_SRCS barcodeitem.cpp  main.cpp  mainwindow.cpp datawindow.cpp sequencewindow.cpp exportwindow.cpp qzint.cpp)

if(USE_QT6)
    qt6_wrap_cpp(zint-qt_SRCS mainwindow.h datawindow.h sequencewindow.h exportwindow.h qzint.h)
    qt6_wrap_ui(zint-qt_SRCS mainWindow.ui extData.ui extSequence.ui extExport.ui)
    qt6_add_resources(zint-qt_SRCS resources.qrc)
else()
    qt5_wrap_cpp(zint-qt_SRCS mainwindow.h datawindow.h sequencewindow.h exportwindow.h qzint.h)
    qt5_wrap_ui(zint-qt_SRCS mainWindow.ui extData.ui extSequence.ui extExport.ui)
    qt5_add_resources(zint-qt_SRCS resources.qrc)
endif()
#               grpAztec.ui  grpC39.ui         grpCodeOne.ui   grpHX.ui        grpMSICheck.ui  grpUPCA.ui
#               grpC11.ui    grpC49.ui         grpDBExtend.ui  grpLOGMARS.ui   grpPDF417.ui    grpUPCEAN.ui
#               grpC128.ui   grpChannel.ui     grpDM.ui        grpMaxicode.ui  grpQR.ui        grpVIN.ui
#               grpC16k.ui   grpCodabar.ui     grpDotCode.ui   grpMicroPDF.ui  grpRMQR.ui
#               grpC25.ui    grpCodablockF.ui  grpGrid.ui      grpMQR.ui       grpUltra.ui

add_executable(zint-qt ${zint-qt_SRCS})
add_dependencies(zint-qt zint)

link_directories( "${CMAKE_BINARY_DIR}/backend" )

if(USE_QT6)
    target_link_libraries(zint-qt zint Qt6::UiTools Qt6::Xml Qt6::Gui Qt6::Core)
else()
    target_link_libraries(zint-qt zint Qt5::UiTools Qt5::Xml Qt5::Gui Qt5::Core)
endif()

install(TARGETS zint-qt DESTINATION "${BIN_INSTALL_DIR}" RUNTIME)
