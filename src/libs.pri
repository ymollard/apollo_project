# Please set the paths to libs SAS and HEASOFT
# Each of them must contain include/ and lib/ directories
# Warning: PWD here is src/some_converter

SAS_LIB = ../../libs/ctadev_64
HEASOFT_LIB = ../../libs/heasoft-6.17/x86_64-unknown-linux-gnu-libc2.19-0




INCLUDEPATH += .
INCLUDEPATH += $$SAS_LIB/include/
INCLUDEPATH += $$SAS_LIB/include/param/src
INCLUDEPATH += $$SAS_LIB/include/error/src
INCLUDEPATH += $$SAS_LIB/include/utils/src
INCLUDEPATH += $$SAS_LIB/include/taskmain/lib/

# SAS dependencies

LIBS += $$SAS_LIB/lib/libcfitsio.so
LIBS += $$SAS_LIB/lib/liberror.so
LIBS += $$SAS_LIB/lib/libparam.so
LIBS += $$SAS_LIB/lib/libselector.so
LIBS += $$SAS_LIB/lib/libutils.so
LIBS += $$SAS_LIB/lib/libcaloalutils.so
LIBS += $$SAS_LIB/lib/libdal.so
LIBS += $$SAS_LIB/lib/libmetatask.so
LIBS += $$SAS_LIB/lib/libqt.so.3
LIBS += $$SAS_LIB/lib/libslatec.so

# Heasoft dependencies
LIBS += $$HEASOFT_LIB/lib/libfitstcl.so
LIBS += $$HEASOFT_LIB/lib/libtcl8.5.so
