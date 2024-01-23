TEMPLATE = lib
CONFIG += staticlib
CONFIG -= debug_and_release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

INCLUDEPATH += inc
    

HEADERS += \
    inc/structs_list.h \
    inc/enums_list.h 
    

