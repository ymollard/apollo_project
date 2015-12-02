TEMPLATE = app
TARGET = main_mass

include(../libs.pri)

# Input
HEADERS += ../mainframe_converter/mainframe_converter.h
SOURCES += ./main_mass.cpp ../mainframe_converter/mainframe_converter.cpp
