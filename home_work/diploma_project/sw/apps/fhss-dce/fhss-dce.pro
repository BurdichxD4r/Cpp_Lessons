QT       += core gui
QT       += network
QT       += xml
CONFIG += resources_big

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fhss-dce
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
t
t
CONFIG += c++11
CONFIG -= debug_and_release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \ 
    src/main.cpp \
    src/mainwindDCE.cpp


HEADERS += \
    src/mainwindDCE.h

FORMS += \
   src/mainwindDCE.ui

INCLUDEPATH += $$PWD/../../comps/qt-simple-plot/inc
INCLUDEPATH += $$PWD/../../comps/fhss-protocol/inc
INCLUDEPATH += $$PWD/../../comps/qt-widget-mess/inc
INCLUDEPATH += $$PWD/../../comps/qt-widget-dce/inc
INCLUDEPATH += $$PWD/../../comps/qt-widget-exch-IQ/inc
INCLUDEPATH += $$PWD/../../comps/eml/inc
INCLUDEPATH += $$PWD/../../comps/qt-dce-panel/inc
INCLUDEPATH += $$PWD/../../comps/qt-dce-plot/inc
INCLUDEPATH += $$PWD/../../comps/fhss-logger/inc
INCLUDEPATH += $$PWD/../../comps/prsgen/inc

PRE_TARGETDEPS += \
        $$OUT_PWD/../../comps/fhss-protocol/libfhss-protocol.a
PRE_TARGETDEPS += \
        $$OUT_PWD/../../comps/qt-widget-mess/libqt-widget-mess.a
PRE_TARGETDEPS += \
        $$OUT_PWD/../../comps/qt-widget-dce/libqt-widget-dce.a
PRE_TARGETDEPS += \
         $$OUT_PWD/../../comps/qt-widget-exch-IQ/libqt-widget-exch-IQ.a


PRE_TARGETDEPS += \
        $$OUT_PWD/../../comps/eml/libeml.a \
        $$OUT_PWD/../../comps/fhss-logger/libfhss-logger.a \
        $$OUT_PWD/../../comps/prsgen/libprsgen.a \
        $$OUT_PWD/../../comps/qt-dce-plot/libqt-dce-plot.a \
        $$OUT_PWD/../../comps/qt-simple-plot/libqt-simple-plot.a \
        $$OUT_PWD/../../comps/qt-customplot/libqt-customplot.a \
        $$OUT_PWD/../../comps/qt-dce-panel/libqt-dce-panel.a

LIBS += -L$$OUT_PWD/../../comps/qt-simple-plot
LIBS += -L$$OUT_PWD/../../comps/qt-dce-plot/  -L$$OUT_PWD/../../comps/qt-customplot/ -L$$OUT_PWD/../../comps/fhss-logger/ -L$$OUT_PWD/../../comps/prsgen/

LIBS += -L$$OUT_PWD/../../comps/eml  -L$$OUT_PWD/../../comps/qt-dce-panel -L$$OUT_PWD/../../comps/qt-channel-panel

LIBS += -L$$OUT_PWD/../../comps/fhss-protocol
LIBS += -L$$OUT_PWD/../../comps/qt-widget-mess
LIBS += -L$$OUT_PWD/../../comps/qt-widget-dce
LIBS += -L$$OUT_PWD/../../comps/qt-widget-exch-IQ
LIBS += -L$$OUT_PWD/../../comps/qt-dce-panel


LIBS += -lqt-widget-dce
LIBS += -lqt-widget-exch-IQ
LIBS += -lfhss-protocol
LIBS += -lqt-widget-mess
LIBS += -lqt-simple-plot

LIBS += -lqt-dce-panel  -leml
LIBS += -lqt-dce-plot -lqt-customplot
LIBS += -lfhss-logger
LIBS += -lprsgen

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    pict.qrc


