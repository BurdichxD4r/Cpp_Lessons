QT       += core gui
QT       += xml
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fhss-dte
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


CONFIG += c++11
CONFIG -= debug_and_release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \         
    src/main.cpp \
    src/mainwindDTE.cpp
   

HEADERS += \  
    src/mainwindDTE.h 
    

FORMS += \
    src/mainwindDTE.ui 
    
INCLUDEPATH += $$PWD/../../comps/fhss-types/inc
INCLUDEPATH += $$PWD/../../comps/fhss-protocol/inc
INCLUDEPATH += $$PWD/../../comps/qt-widget-command/inc
INCLUDEPATH += $$PWD/../../comps/qt-widget-dte/inc
INCLUDEPATH += $$PWD/../../comps/qt-widget-mess/inc

PRE_TARGETDEPS += \
        $$OUT_PWD/../../comps/fhss-types/libfhss-types.a
PRE_TARGETDEPS += \
        $$OUT_PWD/../../comps/fhss-protocol/libfhss-protocol.a
PRE_TARGETDEPS += \
        $$OUT_PWD/../../comps/qt-widget-command/libqt-widget-command.a
PRE_TARGETDEPS += \
        $$OUT_PWD/../../comps/qt-widget-dte/libqt-widget-dte.a
        PRE_TARGETDEPS += \
                $$OUT_PWD/../../comps/qt-widget-mess/libqt-widget-mess.a


LIBS += -L$$OUT_PWD/../../comps/fhss-types
LIBS += -L$$OUT_PWD/../../comps/fhss-protocol
LIBS += -L$$OUT_PWD/../../comps/qt-widget-command
LIBS += -L$$OUT_PWD/../../comps/qt-widget-mess
LIBS += -L$$OUT_PWD/../../comps/qt-widget-dte


LIBS += -lqt-widget-dte
LIBS += -lfhss-protocol
LIBS += -lqt-widget-command
LIBS += -lqt-widget-mess
LIBS += -lfhss-types


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    pict.qrc


