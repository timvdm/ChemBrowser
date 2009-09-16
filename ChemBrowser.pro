#-------------------------------------------------
#
# Project created by QtCreator 2009-09-13T23:57:31
#
#-------------------------------------------------

QT       += network opengl webkit

INCLUDEPATH = /usr/local/include/openbabel-2.0 /usr/local/molsketch /usr/local/include/eigen2
LIBS += -L/usr/local/lib -lopenbabel -lmolsketch -lavogadro

TARGET = ChemBrowser
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
