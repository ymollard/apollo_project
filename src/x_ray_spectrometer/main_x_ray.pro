TEMPLATE = app
TARGET = main_x_ray

include(../libs.pri)

# Input
HEADERS += ../mainframe_converter/mainframe_converter.h ../mainframe_converter/mainframe_converter.tcc
SOURCES += ./main_x_ray.cpp ../mainframe_converter/mainframe_converter.cpp
