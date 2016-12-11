TEMPLATE = vcapp
TARGET = qtZint
CONFIG += 	warn_on \
			thread \
			qt \
			uitools

FORMS = mainWindow.ui \
		extSequence.ui \ 
		extExport.ui \
		extData.ui

HEADERS = 	mainwindow.h \
			barcodeitem.h \
			datawindow.h \
			exportwindow.h \
			sequencewindow.h

SOURCES = 	main.cpp \ 
			mainwindow.cpp \
			barcodeitem.cpp \
			datawindow.cpp \
			exportwindow.cpp \
			sequencewindow.cpp

RESOURCES = resources.qrc

INCLUDEPATH += ../backend_qt4 ../backend

CONFIG(debug, debug|release) {
LIBPATH +=	../backend_qt4/debug
}

CONFIG(release, debug|release) {
LIBPATH +=	../backend_qt4/release
}

RC_FILE = ./res/qtZint.rc
         
LIBS = QtZint2.lib
