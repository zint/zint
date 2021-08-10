/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2021 Robin Stuart <rstuart114@gmail.com>

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

#define ZINT_DEBUG_TEST_PRINT           16
#define ZINT_DEBUG_TEST_LESS_NOISY      32
#define ZINT_DEBUG_TEST_KEEP_OUTFILE    64
#define ZINT_DEBUG_TEST_BWIPP           128
#define ZINT_DEBUG_TEST_PERFORMANCE     256

#ifdef _MSC_VER
#define testutil_popen(command, mode) _popen(command, mode)
#define testutil_pclose(stream) _pclose(stream)
#else
#include <unistd.h>
#define testutil_popen(command, mode) popen(command, mode)
#define testutil_pclose(stream) pclose(stream)
#endif

#include <stdio.h>
#include <errno.h>
#include "../common.h"

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wpedantic"
#  pragma clang diagnostic ignored "-Woverlength-strings"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wpedantic"
#  pragma GCC diagnostic ignored "-Woverlength-strings"
#elif defined(_MSC_VER)
#  pragma warning(disable: 4305) /* truncation from 'double' to 'float' */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

extern int assertionFailed;
extern int assertionNum;
extern const char *assertionFilename;

#if _MSC_VER < 1900 /* MSVC 2015 */
#define testStart(__arg__) (testStartReal("", __arg__))
#else
#define testStart(__arg__) (testStartReal(__func__, __arg__))
#endif
void testStartReal(const char *func, const char *name);
void testFinish(void);
void testSkip(const char *msg);
void testReport();

typedef struct s_testFunction {
    const char *name; void *func; int has_index; int has_generate; int has_debug;
} testFunction;
void testRun(int argc, char *argv[], testFunction funcs[], int funcs_size);

#if _MSC_VER == 1200 /* VC6 */
#include "../ms_stdint.h"
void assert_zero(int exp, const char *fmt, ...);
void assert_nonzero(int exp, const char *fmt, ...);
void assert_null(void *exp, const char *fmt, ...);
void assert_nonnull(void *exp, const char *fmt, ...);
void assert_equal(int e1, int e2, const char *fmt, ...);
void assert_equalu64(uint64_t e1, uint64_t e2, const char *fmt, ...);
void assert_notequal(int e1, int e2, ...);
#else
#define assert_exp(__exp__, ...) \
    { assertionNum++; if (!(__exp__)) { assertionFailed++; printf("%s:%d ", assertionFilename, __LINE__); \
      printf(__VA_ARGS__); testFinish(); return; } }

#define assert_zero(__exp__, ...) assert_exp((__exp__) == 0, __VA_ARGS__)
#define assert_nonzero(__exp__, ...) assert_exp((__exp__) != 0, __VA_ARGS__)
#define assert_null(__ptr__, ...) assert_exp((__ptr__) == NULL, __VA_ARGS__)
#define assert_nonnull(__ptr__, ...) assert_exp((__ptr__) != NULL, __VA_ARGS__)
#define assert_equal(__e1__, __e2__, ...) assert_exp((__e1__) == (__e2__), __VA_ARGS__)
#define assert_equalu64 assert_equal
#define assert_notequal(__e1__, __e2__, ...) assert_exp((__e1__) != (__e2__), __VA_ARGS__)
#endif

INTERNAL void vector_free(struct zint_symbol *symbol); /* Free vector structures */

int testUtilSetSymbol(struct zint_symbol *symbol, int symbology, int input_mode, int eci,
            int option_1, int option_2, int option_3, int output_options, char *data, int length, int debug);

const char *testUtilBarcodeName(int symbology);
const char *testUtilErrorName(int error_number);
const char *testUtilInputModeName(int input_mode);
const char *testUtilOption3Name(int option_3);
const char *testUtilOutputOptionsName(int output_options);

int testUtilDAFTConvert(const struct zint_symbol *symbol, char *buffer, int buffer_size);
int testUtilIsValidUTF8(const unsigned char str[], const int length);
char *testUtilEscape(char *buffer, int length, char *escaped, int escaped_size);
char *testUtilReadCSVField(char *buffer, char *field, int field_size);
void testUtilStrCpyRepeat(char *buffer, char *repeat, int size);

int testUtilSymbolCmp(const struct zint_symbol *a, const struct zint_symbol *b);
struct zint_vector *testUtilVectorCpy(const struct zint_vector *in);
int testUtilVectorCmp(const struct zint_vector *a, const struct zint_vector *b);

int testUtilModulesDump(const struct zint_symbol *symbol, char dump[], int dump_size);
void testUtilModulesPrint(const struct zint_symbol *symbol, const char *prefix, const char *postfix);
void testUtilModulesPrintRow(const struct zint_symbol *symbol, int row, const char *prefix, const char *postfix);
int testUtilModulesCmp(const struct zint_symbol *symbol, const char *expected, int *width, int *row);
int testUtilModulesCmpRow(const struct zint_symbol *symbol, int row, const char *expected, int *width);
char *testUtilUIntArrayDump(unsigned int *array, int size, char *dump, int dump_size);
char *testUtilUCharArrayDump(unsigned char *array, int size, char *dump, int dump_size);

void testUtilBitmapPrint(const struct zint_symbol *symbol, const char *prefix, const char *postfix);
int testUtilBitmapCmp(const struct zint_symbol *symbol, const char *expected, int *row, int *column);

int testUtilDataPath(char *buffer, int buffer_size, const char *subdir, const char *filename);
int testUtilExists(const char *filename);
int testUtilDirExists(const char *dirname);
int testUtilMkDir(const char *dirname);
int testUtilRmDir(const char *dirname);
int testUtilRename(const char *oldpath, const char *newpath);

int testUtilCmpPngs(const char *file1, const char *file2);
int testUtilCmpTxts(const char *txt1, const char *txt2);
int testUtilCmpBins(const char *bin1, const char *bin2);
int testUtilCmpSvgs(const char *svg1, const char *svg2);
int testUtilCmpEpss(const char *eps1, const char *eps2);

int testUtilHaveIdentify();
int testUtilVerifyIdentify(const char *filename, int debug);
int testUtilHaveLibreOffice();
int testUtilVerifyLibreOffice(const char *filename, int debug);
int testUtilHaveGhostscript();
int testUtilVerifyGhostscript(const char *filename, int debug);
int testUtilHaveVnu();
int testUtilVerifyVnu(const char *filename, int debug);
int testUtilHaveTiffInfo();
int testUtilVerifyTiffInfo(const char *filename, int debug);

int testUtilCanBwipp(int index, const struct zint_symbol *symbol, int option_1, int option_2, int option_3,
            int debug);
int testUtilBwipp(int index, const struct zint_symbol *symbol, int option_1, int option_2, int option_3,
            const char *data, int length, const char *primary, char *buffer, int buffer_size);
int testUtilBwippCmp(const struct zint_symbol *symbol, char *msg, char *bwipp_buf, const char *expected);
int testUtilBwippCmpRow(const struct zint_symbol *symbol, int row, char *msg, const char *bwipp_buf,
            const char *expected);

#ifdef __cplusplus
}
#endif

#endif /* TESTCOMMON_H */
