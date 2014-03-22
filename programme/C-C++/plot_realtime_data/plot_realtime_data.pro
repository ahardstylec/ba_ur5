TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    ../libs/ur5_realtime_ci.c

HEADERS += \
    ../libs/ur5_realtime_ci.h

