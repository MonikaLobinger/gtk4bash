CC=gcc
#gcc `pkg-config --cflags gtk4` pr1.c `pkg-config --libs gtk4`
INCLUDES=-I/usr/include/gtk-4.0 -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/harfbuzz -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/fribidi -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/x86_64-linux-gnu -I/usr/include/graphene-1.0 -I/usr/lib/x86_64-linux-gnu/graphene-1.0/include -mfpmath=sse -msse -msse2 
LIBS=-pthread -lgtk-4 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -lgdk_pixbuf-2.0 -lcairo-gobject -lcairo -lgraphene-1.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 
CFLAGS=-c -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED
LFLAGS=-g -ggdb -Wall -Wl,--export-dynamic -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED

all: gtk4bash

gtk4bash : out/gtk4bash.o
	${CC} ${LFLAGS} -o gtk4bash out/gtk4bash.o ${LIBS}

out/gtk4bash.o : gtk4bash.c
	${CC} ${CFLAGS} -o out/gtk4bash.o gtk4bash.c ${INCLUDES}

clean :
	rm gtk4bash *.o

