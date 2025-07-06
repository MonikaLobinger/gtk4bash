CC=gcc
#gcc `pkg-config --cflags gtk4` pr1.c `pkg-config --libs gtk4`
CFLAGS=-c -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED
LFLAGS=-g -ggdb -Wall -Wl,--export-dynamic -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED 

INCLUDES=`pkg-config --cflags gtk4` `pkg-config --cflags libadwaita-1`
LIBS=`pkg-config --libs gtk4`  `pkg-config --libs libadwaita-1` -lexpat

all: gtk4bash

gtk4bash : out/gtk4bash.o
	${CC} ${LFLAGS} -o gtk4bash out/gtk4bash.o ${LIBS}

out/gtk4bash.o : gtk4bash.c
	${CC} ${CFLAGS} ${INCLUDES} -o out/gtk4bash.o gtk4bash.c 

clean :
	rm gtk4bash out/*.o

