win32 {
	TEMPLATE = vclib
	CONFIG += staticlib debug-and-release
}

TARGET = QtZint2
VERSION = 2.13.0.9

QMAKE_CFLAGS += /TP /wd4018 /wd4244 /wd4305
QMAKE_CXXFLAGS += /TP /wd4018 /wd4244 /wd4305

INCLUDEPATH += ../backend d:\\opt\\include

DEFINES +=  _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS ZINT_VERSION=\\\"$$VERSION\\\" include=""

!contains(DEFINES, ZINT_NO_PNG) {
    SOURCES += ../backend/png.c
}

HEADERS +=  ../backend/aztec.h \
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
            ../backend/emf.h \
            ../backend/raster_font.h \
            ../backend/gb18030.h \
            ../backend/gb2312.h \
            ../backend/gbk.h \
            ../backend/filemem.h \
            ../backend/general_field.h \
            ../backend/gridmtx.h \
            ../backend/gs1.h \
            ../backend/hanxin.h \
            ../backend/large.h \
            ../backend/maxicode.h \
            ../backend/output.h \
            ../backend/pcx.h \
            ../backend/pdf417.h \
            ../backend/pdf417_tabs.h \
            ../backend/pdf417_trace.h \
            ../backend/qr.h \
            ../backend/reedsol.h \
            ../backend/rss.h \
            ../backend/sjis.h \
            ../backend/tif.h \
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
           ../backend/gridmtx.c \
           ../backend/filemem.c \
           ../backend/general_field.c \
           ../backend/gif.c \
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
           qzint.cpp
