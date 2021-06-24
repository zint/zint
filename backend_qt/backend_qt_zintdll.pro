TEMPLATE = lib

CONFIG += staticlib

TARGET = QtZintDLL

INCLUDEPATH += ../backend

LIBS += -L"../win32/Release" -lzint

HEADERS += qzint.h

SOURCES += qzint.cpp
