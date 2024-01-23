TEMPLATE = lib
CONFIG += staticlib
CONFIG -= debug_and_release
CONFIG += resources_big
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

QT += widgets
QT += network
QT += xml

INCLUDEPATH += inc
INCLUDEPATH += $$PWD/../../comps/fhss-types/inc
INCLUDEPATH += $$PWD/../../comps/qt-widget-mess/inc
INCLUDEPATH += $$PWD/../../comps/fhss-protocol/inc
INCLUDEPATH += $$PWD/../../comps/qt-widget-command/inc


SOURCES += \
    src/stagemarkerbutton.cpp \
    src/widgetDTE.cpp   \
    src/widgetAU.cpp \
    src/widgetMAS.cpp \
    src/widgetClient.cpp
    
HEADERS += \
    inc/stagemarkerbutton.h \
    inc/widgetDTE.h \
    inc/widgetAU.h \
    inc/widgetMAS.h \
    inc/widgetClient.h

FORMS += \
    src/widgetDTE.ui \
    src/widgetAU.ui \
    src/widgetMAS.ui \
    src/widgetClient.ui




