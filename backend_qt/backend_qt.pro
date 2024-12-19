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
DEFINES += ZINT_VERSION="\\\"2.13.0.9\\\""

!contains(DEFINES, ZINT_NO_PNG) {
    INCLUDEPATH += ../../lpng
    INCLUDEPATH += ../../zlib
}

HEADERS +=  ../backend/aztec.h \
            ../backend/big5.h \
            ../backend/bmp.h \
            ../backend/channel_precalcs.h \
            ../backend/code1.h \
            ../backend/code128.h \
            ../backend/code49.h \
            ../backend/common.h \
            ../backend/composite.h \
            ../backend/dmatrix.h \
            ../backend/dmatrix_trace.h \
            ../backend/eci.h \
            ../backend/eci_sb.h \
            ../backend/emf.h \
            ../backend/raster_font.h \
            ../backend/gb18030.h \
            ../backend/gb2312.h \
            ../backend/gbk.h \
            ../backend/filemem.h \
            ../backend/general_field.h \
            ../backend/gridmtx.h \
            ../backend/gs1.h \
            ../backend/gs1_lint.h \
            ../backend/hanxin.h \
            ../backend/iso3166.h \
            ../backend/iso4217.h \
            ../backend/ksx1001.h \
            ../backend/large.h \
            ../backend/maxicode.h \
            ../backend/output.h \
            ../backend/pcx.h \
            ../backend/pdf417.h \
            ../backend/pdf417_tabs.h \
            ../backend/pdf417_trace.h \
            ../backend/qr.h \
            ../backend/reedsol.h \
            ../backend/reedsol_logs.h \
            ../backend/rss.h \
            ../backend/sjis.h \
            ../backend/tif.h \
            ../backend/tif_lzw.h \
            ../backend/zfiletypes.h \
            ../backend/zintconfig.h \
            ../backend/zint.h \
            qzint.h

SOURCES += ../backend/2of5.c \
           ../backend/auspost.c \
           ../backend/aztec.c \
           ../backend/bc412.c \
           ../backend/bmp.c \
           ../backend/codablock.c \
           ../backend/code.c \
           ../backend/code1.c \
           ../backend/code128.c \
           ../backend/code16k.c \
           ../backend/code49.c \
           ../backend/common.c \
           ../backend/composite.c \
           ../backend/dmatrix.c \
           ../backend/dotcode.c \
           ../backend/dxfilmedge.c \
           ../backend/eci.c \
           ../backend/emf.c \
           ../backend/filemem.c \
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
           ../backend/output.c \
           ../backend/pcx.c \
           ../backend/pdf417.c \
           ../backend/plessey.c \
           ../backend/png.c \
           ../backend/postal.c \
           ../backend/ps.c \
           ../backend/qr.c \
           ../backend/raster.c \
           ../backend/reedsol.c \
           ../backend/rss.c \
           ../backend/svg.c \
           ../backend/telepen.c \
           ../backend/tif.c \
           ../backend/ultra.c \
           ../backend/upcean.c \
           ../backend/vector.c \
           ../backend/dllversion.c \
           qzint.cpp

VERSION = 2.13.0.9

#DESTDIR = .

#include.path = $$[ZINT_INSTALL_HEADERS]
include.path = inst/include
include.files = ../backend/zint.h qzint.h

#target.path = $$[ZINT_INSTALL_LIBS]
target.path = inst/lib

INSTALLS += target include
