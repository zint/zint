
TEMPLATE = lib


# My qt is configured for static build:
# http://qt-project.org/wiki/Build_Standalone_Qt_Application_for_Windows
CONFIG += staticlib

# for dynamic build enable this:
#CONFIG += dll

macx{
	CONFIG -= dll
	CONFIG += lib_bundle
}

TARGET = QtZint

INCLUDEPATH += ../backend

#EDIT THIS !!!!
DEFINES += ZINT_VERSION="\\\"2.7.1\\\""

!contains(DEFINES, NO_PNG) {
    INCLUDEPATH += ../../lpng
    INCLUDEPATH += ../../zlib
}

contains(DEFINES, QR_SYSTEM){
    LIBS += -lqrencode
}

contains(DEFINES, QR){

INCLUDEPATH += qrencode

HEADERS += qrencode/bitstream.h \
           qrencode/mask.h \
           qrencode/qrencode.h \
           qrencode/qrencode_inner.h \
           qrencode/qrinput.h \
           qrencode/qrspec.h \
           qrencode/rscode.h \
           qrencode/split.h 

SOURCES += qrencode/bitstream.c \
           qrencode/mask.c \
           qrencode/qrencode.c \
           qrencode/qrinput.c \
           qrencode/qrspec.c \
           qrencode/rscode.c \
           qrencode/split.c 
}

HEADERS +=  ../backend/aztec.h \
            ../backend/bmp.h \
            ../backend/code49.h \
            ../backend/common.h \
            ../backend/composite.h \
            ../backend/dmatrix.h \
            ../backend/eci.h \
            ../backend/font.h \
            ../backend/gb18030.h \
            ../backend/gb2312.h \
            ../backend/gridmtx.h \
            ../backend/gs1.h \
            ../backend/hanxin.h \
            ../backend/large.h \
            ../backend/maxicode.h \
            ../backend/pcx.h \
            ../backend/pdf417.h \
            ../backend/reedsol.h \
            ../backend/rss.h \
            ../backend/sjis.h \
            ../backend/stdint_msvc.h \
            ../backend/zint.h \
            qzint.h

SOURCES += ../backend/2of5.c \
           ../backend/auspost.c \
           ../backend/aztec.c \
           ../backend/bmp.c \
           ../backend/codablock.c \
           ../backend/code.c \
           ../backend/code128.c \
           ../backend/code16k.c \
           ../backend/code49.c \
           ../backend/common.c \
           ../backend/composite.c \
           ../backend/dmatrix.c \
           ../backend/dotcode.c \
           ../backend/eci.c \
           ../backend/emf.c \
           ../backend/gb18030.c \
           ../backend/gb2312.c \
           ../backend/general_field.c \
           ../backend/gif.c \
           ../backend/gridmtx.c \
           ../backend/gs1.c \
           ../backend/hanxin.c \
           ../backend/imail.c \
           ../backend/large.c \
           ../backend/library.c \
           ../backend/mailmark.c \
           ../backend/maxicode.c \
           ../backend/medical.c \
           ../backend/pcx.c \
           ../backend/pdf417.c \
           ../backend/plessey.c \
           ../backend/postal.c \
           ../backend/ps.c \
           ../backend/raster.c \
           ../backend/reedsol.c \
           ../backend/rss.c \
           ../backend/sjis.c \
           ../backend/svg.c \
           ../backend/telepen.c \
           ../backend/tif.c \
           ../backend/ultra.c \
           ../backend/upcean.c \
           ../backend/vector.c \
           ../backend/qr.c \
           ../backend/dllversion.c \
           ../backend/code1.c \
           ../backend/png.c \
           qzint.cpp

VERSION = 2.7.1

#DESTDIR = .

#include.path = $$[ZINT_INSTALL_HEADERS]
include.path = inst/include
include.files = ../backend/zint.h qzint.h

#target.path = $$[ZINT_INSTALL_LIBS]
target.path = inst/lib

INSTALLS += target include

