# Linux makefile for libzint
#
# make			compiles
# make install		copies to /usr/local/lib
# make uninstall	removes library
# make clean		cleans up a previous compilation and any object or editor files
#

ZINT_VERSION:=-DZINT_VERSION=\"2.4.2\"


CC := gcc
INCLUDE := -I/usr/include
CFLAGS := -g

prefix := /usr
includedir := $(prefix)/include
libdir := $(prefix)/lib
DESTDIR :=

COMMON:= common.c render.c png.c pdf.c library.c ps.c large.c reedsol.c gs1.c svg.c
COMMON_OBJ:= common.o render.o png.o library.o ps.o large.o reedsol.o gs1.o svg.o
ONEDIM:= code.c code128.c 2of5.c upcean.c telepen.c medical.c plessey.c rss.c
ONEDIM_OBJ:= code.o code128.o 2of5.o upcean.o telepen.o medical.o plessey.o rss.o
POSTAL:= postal.c auspost.c imail.c
POSTAL_OBJ:= postal.o auspost.o imail.o
TWODIM:= code16k.c dmatrix.c pdf417.c qr.c maxicode.c composite.c aztec.c code49.c code1.c gridmtx.c
TWODIM_OBJ:= code16k.o dmatrix.o pdf417.o qr.o maxicode.o composite.o aztec.o code49.o code1.o gridmtx.o
LIBS:= `libpng12-config --I_opts --L_opts --ldflags` -lz -lm
 
libzint: code.c code128.c 2of5.c upcean.c medical.c telepen.c plessey.c postal.c auspost.c imail.c code16k.c dmatrix.c reedsol.c pdf417.c maxicode.c rss.c common.c render.c png.c library.c ps.c qr.c large.c composite.c aztec.c gs1.c svg.c code49.c code1.c gridmtx.c pdf.c
	$(CC) -Wall -fPIC $(CFLAGS) $(ZINT_VERSION) -c $(ONEDIM)
	$(CC) -Wall -fPIC $(CFLAGS) $(ZINT_VERSION) -c $(POSTAL)
	$(CC) -Wall -fPIC $(CFLAGS) $(ZINT_VERSION) -c $(TWODIM)
	$(CC) -Wall -fPIC $(CFLAGS) $(ZINT_VERSION) -c $(COMMON)
	$(CC) $(CFLAGS) $(ZINT_VERSION) -shared -Wl,-soname,libzint.so -o libzint.so.2.3.2 $(INCLUDE) $(COMMON_OBJ) $(ONEDIM_OBJ) $(TWODIM_OBJ) $(POSTAL_OBJ) $(LIBS)
	ln -s libzint.so.* libzint.so

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


