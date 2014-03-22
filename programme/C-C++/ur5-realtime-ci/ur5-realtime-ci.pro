TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ../libs/tcphelper.c \
    ../libs/ur5_realtime_ci.c \
    main.cpp \
    ../libs/interrupt_utils.c

HEADERS += \
    ../libs/tcphelper.h \
    ../libs/ur5_realtime_ci.h \
    ../libs/interrupt_utils.h

unix|win32: LIBS += -lpthread

INCLUDEPATH += /usr/include/c++/4.8.2/
