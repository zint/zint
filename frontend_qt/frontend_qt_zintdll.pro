TEMPLATE = app
TARGET = qtZint
QT += uitools
QT += widgets

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

RESOURCES += resources.qrc

RC_FILE += res/qtZint.rc

INCLUDEPATH += ../backend_qt ../backend

LIBS += -lQtZintDLL -lzint
QMAKE_LIBDIR += ../backend_qt/release
QMAKE_LIBDIR += ../win32/Release
