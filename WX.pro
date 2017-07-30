#-------------------------------------------------
#
# Project created by QtCreator 2017-04-24T15:55:26
#
#-------------------------------------------------

QT       += core gui opengl widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_LFLAGS_WINDOWS= /SUBSYSTEM:WINDOWS,5.01

TARGET = WX
TEMPLATE = app
RC_FILE =  app.rc


SOURCES += main.cpp\
        mainwindow.cpp \
    viewerwidget.cpp \
    pickhandler.cpp \
    measureanalysis.cpp \
    vpbterrain.cpp \
    travelmanipulator.cpp \
    mytravelmanipulator.cpp \
    compositeimage.cpp \
    imageresample.cpp \
    cprocessdlg.cpp

HEADERS  += mainwindow.h \
    viewerwidget.h \
    pickhandler.h \
    measureanalysis.h \
    vpbterrain.h \
    travelmanipulator.h \
    mytravelmanipulator.h \
    compositeimage.h \
    imageresample.h \
    cprocessbase.h \
    cprocessdlg.h

FORMS    += mainwindow.ui \
    vpbterrain.ui \
    compositeimage.ui \
    imageresample.ui

3RDPATH = ../3rd
OSGPATH = ../OSG
VPBPATH = ../VPB

LIBS += -L"../bin"
LIBS += -L"$$OSGPATH/lib"

INCLUDEPATH += $$OSGPATH/include
#INCLUDEPATH += $$3RDPATH/GDAL/include

#DLLDESTDIR = $$PWD/bin
DESTDIR = ../bin
#DLLDESTDIR=../bin

#设置Obj输出目录与Target
SUFFIX_STR =

 CONFIG(debug, debug|release) {
     message(Building $$TARGET with debug mode.)
     SUFFIX_STR =d
     DEFINES += _DEBUG

 }else {
}
TARGET              = $$TARGET$$SUFFIX_STR

win32: LIBS += -L$$PWD/../3rd/GDAL210/lib/ -lgdal_i

INCLUDEPATH += $$PWD/../3rd/GDAL210/include
DEPENDPATH += $$PWD/../3rd/GDAL210/include

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../3rd/GDAL210/lib/gdal_i.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../3rd/GDAL210/lib/libgdal_i.a

CONFIG(debug, debug|release){
    LIBS += -lOpenThreadsd \
        -losgd \
        -losgDBd \
        -losgUtild \
        -losgGAd \
        -losgViewerd \
        -losgQtd \
        -losgTerraind\
        -losgShadowd\
        -lglew32\
}else{
    LIBS += -losg \
        -losgDB \
        -losgUtil \
        -losgGA \
        -losgViewer \
        -losgQt \
        -lOpenThreads \
        -losgTerrain \
        -losgShadow \
        -lglew32 \
}

RESOURCES += \
    icon.qrc


