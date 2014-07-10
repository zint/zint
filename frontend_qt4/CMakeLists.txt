# (c) 2008 by BogDan Vatra < bogdan@licentia.eu >

project(zint-qt)

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/backend"  "${CMAKE_SOURCE_DIR}/backend_qt4")

set(zint-qt_SRCS barcodeitem.cpp  main.cpp  mainwindow.cpp datawindow.cpp sequencewindow.cpp exportwindow.cpp)
QT4_WRAP_CPP(zint-qt_SRCS mainwindow.h datawindow.h sequencewindow.h exportwindow.h)

QT4_WRAP_UI(zint-qt_SRCS mainWindow.ui extData.ui extSequence.ui extExport.ui)
#				grpAztec.ui  grpC39.ui        grpDM.ui        grpMSICheck.ui
#				grpC128.ui   grpChannel.ui    grpMicroPDF.ui  grpPDF417.ui
#				grpC16k.ui   grpCodablock.ui  grpMQR.ui       grpQR.ui
#				grpMaxicode.ui)

QT4_ADD_RESOURCES(zint-qt_SRCS resources.qrc)

add_executable(zint-qt ${zint-qt_SRCS})

target_link_libraries(zint-qt zint QZint ${QT_QTUITOOLS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY}
     ${QT_QTCORE_LIBRARY} )

install(TARGETS zint-qt DESTINATION "${BIN_INSTALL_DIR}" RUNTIME)
