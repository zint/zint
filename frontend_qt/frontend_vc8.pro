TEMPLATE = vcapp
TARGET = qtZint
CONFIG += 	warn_on \
			thread \
			qt \
			uitools

FORMS = mainWindow.ui \
		extCLI.ui \
		extScale.ui \
		extSequence.ui \
		extExport.ui \
		extData.ui

HEADERS = 	mainwindow.h \
			barcodeitem.h \
			cliwindow.h \
			datawindow.h \
			exportwindow.h \
			scalewindow.h \
			sequencewindow.h \
			qzint.h

SOURCES = 	main.cpp \
			mainwindow.cpp \
			barcodeitem.cpp \
			cliwindow.cpp \
			datawindow.cpp \
			exportwindow.cpp \
			scalewindow.cpp \
			sequencewindow.cpp \
			qzint.cpp

RESOURCES = resources.qrc

INCLUDEPATH += ../backend

RC_FILE = ./res/qtZint.rc

LIBS = QtZint2.lib
