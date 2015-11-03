TEMPLATE = app
TARGET = main_gamma_ray
INCLUDEPATH += .
INCLUDEPATH += ../ctadev/include/
INCLUDEPATH += ../ctadev/include/param/src
INCLUDEPATH += ../ctadev/include/error/src
INCLUDEPATH += ../ctadev/include/utils/src
INCLUDEPATH += ../ctadev/include/taskmain/lib/

# SAS dependencies
LIBS += ../ctadev/lib/libcfitsio.so
LIBS += ../ctadev/lib/liberror.so
LIBS += ../ctadev/lib/libparam.so
LIBS += ../ctadev/lib/libselector.so
LIBS += ../ctadev/lib/libutils.so
LIBS += ../ctadev/lib/libcaloalutils.so
LIBS += ../ctadev/lib/libdal.so
LIBS += ../ctadev/lib/libmetatask.so
LIBS += ../ctadev/lib/libqt.so.3
LIBS += ../ctadev/lib/libslatec.so

# Heasoft dependencies
LIBS += ../heasoft-6.17/x86_64-unknown-linux-gnu-libc2.5/lib/libfitstcl.so
LIBS += ../heasoft-6.17/x86_64-unknown-linux-gnu-libc2.5/lib/libtcl8.5.so

# Input
HEADERS += mainframe_converter.h
SOURCES += main_gamma_ray.cpp mainframe_converter.cpp
