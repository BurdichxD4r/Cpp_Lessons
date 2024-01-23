TEMPLATE = lib
CONFIG += staticlib
CONFIG += resources_big
CONFIG -= debug_and_release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

QT += widgets

INCLUDEPATH += inc
INCLUDEPATH += $$PWD/../../comps/fhss-types/inc


SOURCES += \
    src/basecommand.cpp \
    src/exchagedce_dial.cpp \
    src/netconnect_dial.cpp \
    src/netdisconnect_dial.cpp \
    src/netsync_dial.cpp \
    src/disabledce_dial.cpp \
    src/enabledce_dial.cpp \ 
    src/widgetrc.cpp \
    src/tablefreq.cpp     
    

HEADERS += \
    inc/basecommand.h \
    inc/disabledce_dial.h \
    inc/enabledce_dial.h \
    inc/exchagedce_dial.h \
    inc/widgetrc.h \
    inc/mode1.txt \
    inc/tablefreq.h \
    inc/netsync_dial.h \
    inc/netconnect_dial.h \
    inc/netdisconnect_dial.h
    
FORMS += \
    src/exchagedce_dial.ui \
    src/netconnect_dial.ui \
    src/netdisconnect_dial.ui \
    src/netsync_dial.ui \
    src/disabledce_dial.ui \
    src/enabledce_dial.ui    

RESOURCES += \
    mode.qrc
