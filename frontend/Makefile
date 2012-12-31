# Linux makefile for zint - requires libzint
#
# make			compiles zint
# make install		copies binary to /usr/local/bin
# make uninstall	removes the binary
# make clean		cleans up a previous compilation and any object or editor files
#

ZINT_VERSION:=-DZINT_VERSION=\"2.4.4\"

CC := gcc
INCLUDE := -I/usr/include
CFLAGS := -g

prefix := /usr
bindir := $(prefix)/bin
DESTDIR :=

zint: main.c
	$(CC) -Wall $(INCLUDE) $(CFLAGS) $(ZINT_VERSION) -I../backend -L../backend main.c -o zint -lzint

.PHONY: install uninstall clean dist

clean:
	rm -f zint *.o *.a *~ *.png *.eps *.svg *.log

install:
	install -D -p  zint $(DESTDIR)$(bindir)/zint

uninstall:
	rm $(DESTDIR)$(bindir)/zint
