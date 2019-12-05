TEMPLATE = app
TARGET = qtZint
DEPENDPATH += . debug release
INCLUDEPATH += .
QT += gui
QT += uitools
QT += widgets
QT += uitools

# Input
HEADERS += barcodeitem.h \
         datawindow.h \
         exportwindow.h \
         mainwindow.h \
         sequencewindow.h

FORMS += extData.ui \
         extExport.ui \
         extSequence.ui \
         grpAztec.ui \
         grpC128.ui \
         grpC16k.ui \
         grpC39.ui \
         grpC49.ui \
         grpChannel.ui \
         grpCodeOne.ui \
         grpCodablockF.ui \
         grpDBExtend.ui \
         grpDM.ui \
         grpGrid.ui \
         grpHX.ui \
         grpMaxicode.ui \
         grpMicroPDF.ui \
         grpMQR.ui \
         grpMSICheck.ui \
         grpPDF417.ui \
         grpQR.ui \
         grpRMQR.ui \
         mainWindow.ui \
         grpDotCode.ui

SOURCES += barcodeitem.cpp \
        datawindow.cpp \
        exportwindow.cpp \
        main.cpp \
        mainwindow.cpp \
        sequencewindow.cpp
        
TRANSLATIONS = frontend_de.ts

RESOURCES += resources.qrc

# Static target following http://qt-project.org/wiki/Build_Standalone_Qt_Application_for_Windows
CONFIG += static

CONFIG += warn_on thread qt

INCLUDEPATH += ../backend_qt ../backend

LIBS += -lQtZint -lQt5Core
QMAKE_LIBDIR += ../backend_qt/release

!contains(DEFINES, NO_PNG) {
# Win
    win32:LIBS += -llibpng16 -lzlib
    win32:QMAKE_LIBDIR+="../../lpng/projects/vstudio/Release Library"
# Unix
    #unix:LIBS += -lpng -zlib
    #unix:QMAKE_LIBDIR += /usr/local/lib /usr/lib/x86_64-linux-gnu/
}
