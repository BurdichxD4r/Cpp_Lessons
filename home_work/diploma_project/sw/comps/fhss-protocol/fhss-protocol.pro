TEMPLATE = lib
CONFIG += staticlib
CONFIG += resources_big
CONFIG -= debug_and_release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

QT += widgets
QT += xml
QT += network

INCLUDEPATH += inc
INCLUDEPATH += $$PWD/../../comps/fhss-types/inc


SOURCES += \
    src/logmaker.cpp \
    src/protocolRTP.cpp \
    src/protocolRadio.cpp \
    src/protocolServ.cpp \
    src/protocolClient.cpp \
    src/packet.cpp \
    src/transportBase.cpp \
    src/transportClient.cpp \
    src/transportServ.cpp \
    src/verificationBase.cpp \
    src/translatorXml.cpp \
    src/verificationClient.cpp \
    src/verificationServ.cpp \
    src/transportUDP.cpp


HEADERS += \
    inc/protocolClient.h \
    inc/enum_list.h \
    inc/packet.h \
    inc/protocolRadio.h \
    inc/transportBase.h \
    inc/struct_list.h \
    inc/translatorXml.h \
    inc/command_list.txt \
    inc/struct_list.h \
    inc/transportClient.h \
    inc/transportServ.h \
    inc/verificationBase.h \
    inc/verificationClient.h \
    inc/verificationServ.h \
    inc/protocolServ.h \
    inc/transportUDP.h \
    inc/protocolRTP.h \
    inc/logmaker.h


RESOURCES += \
    com_list.qrc




