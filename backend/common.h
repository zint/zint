/* common.h - Header for all common functions in common.c */

/*
    libzint - the open source barcode library
    Copyright (C) 2009 Robin Stuart <robin@zint.org.uk>
*/

/* Used in some logic */
#ifndef __COMMON_H
#define __COMMON_H

#ifndef FALSE
#define FALSE		0
#endif

#ifndef TRUE
#define TRUE		1
#endif

/* The most commonly used set */
#define NEON	"0123456789"

#include "zint.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int ustrlen(const unsigned char source[]);
extern void ustrcpy(unsigned char target[], const unsigned char source[]);
extern void concat(char dest[], const char source[]);
extern void uconcat(unsigned char dest[], const unsigned char source[]);
extern int ctoi(char source);
extern char itoc(int source);
extern void to_upper(unsigned char source[]);
extern int is_sane(char test_string[], unsigned char source[], int length);
extern void lookup(char set_string[],const char *table[], char data, char dest[]);
extern int posn(char set_string[], char data);
extern void expand(struct zint_symbol *symbol, char data[]);
extern int is_stackable(int symbology);
extern int is_extendable(int symbology);
extern int roundup(float input);
extern int module_is_set(struct zint_symbol *symbol, int y_coord, int x_coord);
extern void set_module(struct zint_symbol *symbol, int y_coord, int x_coord);
extern void unset_module(struct zint_symbol *symbol, int y_coord, int x_coord);
extern int istwodigits(unsigned char source[], int position);
extern float froundup(float input);
extern int parunmodd(unsigned char llyth);
extern int latin1_process(struct zint_symbol *symbol, unsigned char source[], unsigned char preprocessed[], int *length);
extern int utf8toutf16(struct zint_symbol *symbol, unsigned char source[], int vals[], int *length);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __COMMON_H */
