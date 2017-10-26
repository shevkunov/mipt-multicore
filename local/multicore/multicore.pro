TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CFLAGS += -ansi -pedantic -Wall -Wextra
QMAKE_LFLAGS += -pthread
QMAKE_LFLAGS += -lm

SOURCES += \
    integral.c
