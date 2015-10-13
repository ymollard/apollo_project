PATH_TO_CTADEV=/home/ymollard/ctadev

all: preprocess apollo15

preprocess: remove_blocks.c
	gcc remove_blocks.c -o preprocess

apollo15: apollo15.cc jpleph.o apollo15.hpp
	g++ -O0 -c -DUSE_DAL apollo15.cc -I$(PATH_TO_CTADEV)/include/ -I$(PATH_TO_CTADEV)/include/utils/src -I$(PATH_TO_CTADEV)/include/taskmain/lib -I$(PATH_TO_CTADEV)/include/param/src -I$(PATH_TO_CTADEV)/include/barycen/src -I$(PATH_TO_CTADEV)/include/error/src -Wall -g
	g++ -o apollo15 apollo15.o jpleph.o -L$(PATH_TO_CTADEV)/lib -ldal -lcfitsio -lutils -lerror -lmetatask -lparam -Wall -g

jpleph: jpleph.cc jpleph.h jpl_int.h
	g++ -c -g jpleph.cc

clean:
	rm *.o preprocess apollo15

flush:
	rm fits/*.fits DR005893.F02
