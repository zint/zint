TEMPLATE = app
TARGET = qtZint
DEPENDPATH += . debug release
INCLUDEPATH += .
INCLUDEPATH += ../backend
INCLUDEPATH += ../backend_qt
!contains(DEFINES, NO_PNG) {
    INCLUDEPATH += ../../lpng
    INCLUDEPATH += ../../zlib
}
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
         grpC11.ui \
         grpC128.ui \
         grpC16k.ui \
         grpC25.ui \
         grpC39.ui \
         grpC49.ui \
         grpChannel.ui \
         grpCodabar.ui \
         grpCodablockF.ui \
         grpCodeOne.ui \
         grpDAFT.ui \
         grpDBExtend.ui \
         grpDM.ui \
         grpDotCode.ui \
         grpGrid.ui \
         grpHX.ui \
         grpLOGMARS.ui \
         grpMaxicode.ui \
         grpMicroPDF.ui \
         grpMQR.ui \
         grpMSICheck.ui \
         grpPDF417.ui \
         grpQR.ui \
         grpRMQR.ui \
         grpUltra.ui \
         grpUPCA.ui \
         grpUPCEAN.ui \
         grpVIN.ui \
         mainWindow.ui

SOURCES += barcodeitem.cpp \
        datawindow.cpp \
        exportwindow.cpp \
        main.cpp \
        mainwindow.cpp \
        sequencewindow.cpp
        
TRANSLATIONS = frontend_de.ts

RESOURCES += resources.qrc

RC_FILE += res/qtZint.rc

# Static target following http://qt-project.org/wiki/Build_Standalone_Qt_Application_for_Windows
CONFIG += static

CONFIG += warn_on thread qt

LIBS += -lQtZint
QMAKE_LIBDIR += ../backend_qt/release
LIBS += -lQt5Core
QMAKE_LIBDIR += C:/qt/5.15.2static/lib

!contains(DEFINES, NO_PNG) {
# Win
    win32:LIBS += -llibpng16 -lzlib
    win32:QMAKE_LIBDIR+="../../lpng/projects/vstudio/Release Library"
# Unix
    #unix:LIBS += -lpng -zlib
    #unix:QMAKE_LIBDIR += /usr/local/lib /usr/lib/x86_64-linux-gnu/
}
