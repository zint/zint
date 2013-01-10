# Linux makefile for libzint
#
# make			compiles
# make install		copies to /usr/local/lib
# make uninstall	removes library
# make clean		cleans up a previous compilation and any object or editor files
#

ZINT_VERSION:=-DZINT_VERSION=\"2.4.4\"


CC := gcc
INCLUDE := -I/usr/include
CFLAGS := -g

prefix := /usr
includedir := $(prefix)/include
libdir := $(prefix)/lib
DESTDIR :=

COMMON:= common.c render.c png.c library.c ps.c large.c reedsol.c gs1.c svg.c
COMMON_OBJ:= common.o render.o png.o library.o ps.o large.o reedsol.o gs1.o svg.o
ONEDIM:= code.c code128.c 2of5.c upcean.c telepen.c medical.c plessey.c rss.c
ONEDIM_OBJ:= code.o code128.o 2of5.o upcean.o telepen.o medical.o plessey.o rss.o
POSTAL:= postal.c auspost.c imail.c
POSTAL_OBJ:= postal.o auspost.o imail.o
TWODIM:= code16k.c dmatrix.c pdf417.c qr.c maxicode.c composite.c aztec.c code49.c code1.c gridmtx.c
TWODIM_OBJ:= code16k.o dmatrix.o pdf417.o qr.o maxicode.o composite.o aztec.o code49.o code1.o gridmtx.o
OBJS:=$(COMMON_OBJ) $(ONEDIM_OBJ) $(TWODIM_OBJ) $(POSTAL_OBJ)
LIBS:= -lz -lm -lpng

all: libzint.a libzint.so

libzint.a: $(OBJS)
	$(AR) rcs libzint.a $(OBJS)
 
libzint.so.2.4.4: $(OBJS)
	$(CC) $(CFLAGS) $(ZINT_VERSION) -shared -Wl,-soname,libzint.so -o libzint.so.2.4.4 $(INCLUDE) $(OBJS) $(LIBS)

libzint.so: libzint.so.2.4.4
	rm -f libzint.so
	ln -s libzint.so.* libzint.so

%.o: %.c
	$(CC) -Wall -fPIC $(CFLAGS) $(ZINT_VERSION) -c -o $@ $<

.PHONY: install uninstall clean dist

install:
	test "$(UID)" = "0" && ldconfig -n $(PWD) || true
	install -d $(DESTDIR)$(libdir)
	mv libzint.* $(DESTDIR)$(libdir)
	install -D -p --mode=0644 zint.h $(DESTDIR)$(includedir)/zint.h

uninstall:
	rm $(DESTDIR)$(libdir)/libzint.*
	rm $(DESTDIR)$(includedir)/zint.h

clean:
	rm -f libzint.* *.o *.a *~


