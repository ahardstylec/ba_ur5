TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    ../libs/ur5_primary_secondary_interface.c

HEADERS += \
    ../libs/ur5_primary_secondary_interface.h

