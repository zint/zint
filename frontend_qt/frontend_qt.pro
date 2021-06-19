TEMPLATE = app
TARGET = qtZint
DEPENDPATH += . debug release
INCLUDEPATH += .
INCLUDEPATH += ../backend
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
         sequencewindow.h \
         qzint.h \
        ..\backend\aztec.h \
        ..\backend\big5.h \
        ..\backend\bmp.h \
        ..\backend\channel_precalcs.h \
        ..\backend\code1.h \
        ..\backend\code128.h \
        ..\backend\code49.h \
        ..\backend\common.h \
        ..\backend\composite.h \
        ..\backend\dmatrix.h \
        ..\backend\eci.h \
        ..\backend\eci_sb.h \
        ..\backend\emf.h \
        ..\backend\font.h \
        ..\backend\gb18030.h \
        ..\backend\gb2312.h \
        ..\backend\general_field.h \
        ..\backend\gridmtx.h \
        ..\backend\gs1.h \
        ..\backend\hanxin.h \
        ..\backend\ksx1001.h \
        ..\backend\large.h \
        ..\backend\maxicode.h \
        ..\backend\ms_stdint.h \
        ..\backend\output.h \
        ..\backend\pcx.h \
        ..\backend\pdf417.h \
        ..\backend\qr.h \
        ..\backend\reedsol.h \
        ..\backend\reedsol_logs.h \
        ..\backend\rss.h \
        ..\backend\sjis.h \
        ..\backend\stdint_msvc.h \
        ..\backend\tif.h \
        ..\backend\tif_lzw.h \
        ..\backend\zfiletypes.h \
        ..\backend\zint.h \
        ..\backend\zintconfig.h \

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
         grpCodeOne.ui \
         grpCodablockF.ui \
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
         grpUPCA.ui \
         grpUPCEAN.ui \
         grpVIN.ui \
         mainWindow.ui

SOURCES += barcodeitem.cpp \
        datawindow.cpp \
        exportwindow.cpp \
        main.cpp \
        mainwindow.cpp \
        sequencewindow.cpp \
        qzint.cpp \
        ..\backend\2of5.c \
        ..\backend\auspost.c \
        ..\backend\aztec.c \
        ..\backend\bmp.c \
        ..\backend\codablock.c \
        ..\backend\code.c \
        ..\backend\code1.c \
        ..\backend\code128.c \
        ..\backend\code16k.c \
        ..\backend\code49.c \
        ..\backend\common.c \
        ..\backend\composite.c \
        ..\backend\dllversion.c \
        ..\backend\dmatrix.c \
        ..\backend\dotcode.c \
        ..\backend\eci.c \
        ..\backend\emf.c \
        ..\backend\gb18030.c \
        ..\backend\gb2312.c \
        ..\backend\general_field.c \
        ..\backend\gif.c \
        ..\backend\gridmtx.c \
        ..\backend\gs1.c \
        ..\backend\hanxin.c \
        ..\backend\imail.c \
        ..\backend\large.c \
        ..\backend\library.c \
        ..\backend\mailmark.c \
        ..\backend\maxicode.c \
        ..\backend\medical.c \
        ..\backend\output.c \
        ..\backend\pcx.c \
        ..\backend\pdf417.c \
        ..\backend\plessey.c \
        ..\backend\png.c \
        ..\backend\postal.c \
        ..\backend\ps.c \
        ..\backend\qr.c \
        ..\backend\raster.c \
        ..\backend\reedsol.c \
        ..\backend\rss.c \
        ..\backend\sjis.c \
        ..\backend\svg.c \
        ..\backend\telepen.c \
        ..\backend\tif.c \
        ..\backend\ultra.c \
        ..\backend\upcean.c \
        ..\backend\vector.c
        
TRANSLATIONS = frontend_de.ts

RESOURCES += resources.qrc

RC_FILE += res/qtZint.rc

# Static target following http://qt-project.org/wiki/Build_Standalone_Qt_Application_for_Windows
CONFIG += static

CONFIG += warn_on thread qt

INCLUDEPATH += ../backend

LIBS += -lQt5Core
QMAKE_LIBDIR += C:/qt/5.15.1static/lib

!contains(DEFINES, NO_PNG) {
# Win
    win32:LIBS += -llibpng16 -lzlib
    win32:QMAKE_LIBDIR+="../../lpng/projects/vstudio/Release Library"
# Unix
    #unix:LIBS += -lpng -zlib
    #unix:QMAKE_LIBDIR += /usr/local/lib /usr/lib/x86_64-linux-gnu/
}
