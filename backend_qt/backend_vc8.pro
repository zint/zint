win32 {
	TEMPLATE = vclib
	CONFIG += staticlib debug-and-release
}

TARGET = QtZint2
VERSION = 2.3.0

QMAKE_CFLAGS += /TP /wd4018 /wd4244 /wd4305
QMAKE_CXXFLAGS += /TP /wd4018 /wd4244 /wd4305

INCLUDEPATH += ../backend d:\\opt\\include

DEFINES +=  _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS ZINT_VERSION=\\\"$$VERSION\\\" include=""

!contains(DEFINES, NO_PNG) {
    SOURCES += ../backend/png.c
}

HEADERS +=  ../backend/aztec.h \
            ../backend/code1.h \
            ../backend/code49.h \
            ../backend/common.h \
            ../backend/composite.h \
            ../backend/dmatrix.h \
            ../backend/font.h \
            ../backend/gb18030.h \
            ../backend/gb2312.h \
            ../backend/gridmtx.h \
            ../backend/gs1.h \
            ../backend/large.h \
            ../backend/maxicode.h \
            ../backend/maxihex.h \
            ../backend/ms_stdint.h \
            ../backend/pdf417.h \
            ../backend/qr.h \
            ../backend/reedsol.h \
            ../backend/rss.h \
            ../backend/sjis.h \
            ../backend/zint.h \
            qzint.h

SOURCES += ../backend/2of5.c \
           ../backend/auspost.c \
           ../backend/aztec.c \
           ../backend/code.c \
           ../backend/code1.c \
           ../backend/code128.c \
           ../backend/code16k.c \
           ../backend/code49.c \
           ../backend/common.c \
           ../backend/composite.c \
           ../backend/dmatrix.c \
           ../backend/gb18030.c \
           ../backend/gb2312.c \
           ../backend/gridmtx.c \
           ../backend/gs1.c \
           ../backend/imail.c \
           ../backend/large.c \
           ../backend/library.c \
           ../backend/maxicode.c \
           ../backend/medical.c \
           ../backend/pdf417.c \
           ../backend/plessey.c \
           ../backend/postal.c \
           ../backend/ps.c \
           ../backend/qr.c \
           ../backend/reedsol.c \
           ../backend/rss.c \
           ../backend/svg.c \
           ../backend/telepen.c \
           ../backend/upcean.c \
           qzint.cpp
