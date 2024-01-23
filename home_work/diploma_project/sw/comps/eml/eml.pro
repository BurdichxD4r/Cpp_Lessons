TEMPLATE = lib
CONFIG += staticlib
CONFIG -= debug_and_release

INCLUDEPATH += inc

SOURCES += \
        src/eml.cpp \
        src/fastfourier.cpp \
        src/math_big.cpp \
        src/treedsol.cpp

HEADERS += \
        inc/eml.h \
        inc/fastfourier.h \
        inc/math_big.h \
        inc/treedsol.h
