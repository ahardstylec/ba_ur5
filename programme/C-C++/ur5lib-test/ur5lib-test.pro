TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    ../libs/ur_kin.c \
    ../libs/ur5lib.c

HEADERS += \
    ../libs/ur_kin.h \
    ../libs/ur5lib.h \

