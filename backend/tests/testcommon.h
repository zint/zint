/*
    libzint - the open source barcode library
    Copyright (C) 2008-2019 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 */
/* vim: set ts=4 sw=4 et : */
/*
 * Adapted from qrencode/tests/common.h
 * Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
 */

#ifndef TESTCOMMON_H
#define TESTCOMMON_H

#include <stdio.h>
#include <string.h>
#include "../common.h"

extern int assertionFailed;
extern int assertionNum;

#define testStart(__arg__) (testStartReal(__func__, __arg__))
#define testEndExp(__arg__) (testEnd(!(__arg__)))
void testStartReal(const char *func, const char *name);
void testEnd(int result);
void testFinish(void);
void testReport();

#define assert_exp(__exp__, ...) \
{assertionNum++;if(!(__exp__)) {assertionFailed++; printf(__VA_ARGS__);testFinish();return;}}

#define assert_zero(__exp__, ...) assert_exp((__exp__) == 0, __VA_ARGS__)
#define assert_nonzero(__exp__, ...) assert_exp((__exp__) != 0, __VA_ARGS__)
#define assert_null(__ptr__, ...) assert_exp((__ptr__) == NULL, __VA_ARGS__)
#define assert_nonnull(__ptr__, ...) assert_exp((__ptr__) != NULL, __VA_ARGS__)
#define assert_equal(__e1__, __e2__, ...) assert_exp((__e1__) == (__e2__), __VA_ARGS__)
#define assert_notequal(__e1__, __e2__, ...) assert_exp((__e1__) != (__e2__), __VA_ARGS__)
#define assert_fail(...) assert_exp(0, __VA_ARGS__)
#define assert_nothing(__exp__, ...) {printf(__VA_ARGS__); __exp__;}

extern void vector_free(struct zint_symbol *symbol); /* Free vector structures */

char* testUtilBarcodeName(int symbology);
char* testUtilErrorName(int error_number);
char* testUtilInputModeName(int input_mode);
int testUtilDAFTConvert(const struct zint_symbol* symbol, char* buffer, int buffer_size);
char* testUtilEscape(char* buffer, int length, char* escaped, int escaped_size);
char* testUtilReadCSVField(char* buffer, char* field, int field_size);
int testUtilSymbolCmp(const struct zint_symbol* a, const struct zint_symbol* b);
struct zint_vector* testUtilVectorCpy(const struct zint_vector* in);
int testUtilVectorCmp(const struct zint_vector* a, const struct zint_vector* b);
void testUtilLargeDump(const char* name, const short reg[]);
void testUtilModulesDump(const struct zint_symbol* symbol, char* prefix, char* postfix);
int testUtilModulesCmp(const struct zint_symbol* symbol, const char* expected, int* row, int* width);
int testUtilModulesDumpHex(const struct zint_symbol* symbol, char dump[], int dump_size);
int testUtilExists(char* filename);
int testUtilCmpPngs(char* file1, char* file2);
int testUtilCmpSvgs(char* svg1, char* svg2);
int testUtilCmpEpss(char* eps1, char* eps2);
int testUtilCmpEmfs(char* emf1, char* emf2);
int testUtilCmpGifs(char* gif1, char* gif2);
int testUtilCmpBmps(char* bmp1, char* bmp2);
int testUtilCmpPcxs(char* pcx1, char* pcx2);

#endif /* TESTCOMMON_H */
