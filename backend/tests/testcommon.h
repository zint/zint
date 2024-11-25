/*
    libzint - the open source barcode library
    Copyright (C) 2019-2024 Robin Stuart <rstuart114@gmail.com>

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
/*
 * Adapted from qrencode/tests/common.h
 * Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
 */
/* Due to above: */
/* SPDX-License-Identifier: LGPL-2.1+ */

#ifndef Z_TESTCOMMON_H
#define Z_TESTCOMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define ZINT_DEBUG_TEST_PRINT           16
#define ZINT_DEBUG_TEST_LESS_NOISY      32
#define ZINT_DEBUG_TEST_KEEP_OUTFILE    64
#define ZINT_DEBUG_TEST_BWIPP           128
#define ZINT_DEBUG_TEST_PERFORMANCE     256
#define ZINT_DEBUG_TEST_ZXINGCPP        512

#include <errno.h>
#include <stdio.h>
#include "../common.h"

#ifdef _MSC_VER
#define testutil_popen(command, mode) _popen(command, mode)
#define testutil_pclose(stream) _pclose(stream)
#else
#include <unistd.h>
extern FILE *popen(const char *command, const char *type);
extern int pclose(FILE *stream);
#define testutil_popen(command, mode) popen(command, mode)
#define testutil_pclose(stream) pclose(stream)
#endif

#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wpedantic"
#  pragma GCC diagnostic ignored "-Woverlength-strings"
#elif defined(_MSC_VER)
#  pragma warning(disable: 4305) /* truncation from 'double' to 'float' */
#  pragma warning(disable: 4702) /* unreachable code */
#endif

extern int testAssertFailed;
extern int testAssertNum;
extern struct zint_symbol **testAssertPPSymbol;
extern const char *testAssertFilename;

#if defined(_MSC_VER) && _MSC_VER < 1900 /* MSVC 2015 */
#define testStart(name) (testStartReal("", name, NULL))
#define testStartSymbol(name, pp_symbol) (testStartReal("", name, pp_symbol))
#else
#define testStart(name) (testStartReal(__func__, name, NULL))
#define testStartSymbol(name, pp_symbol) (testStartReal(__func__, name, pp_symbol))
#endif
void testStartReal(const char *func, const char *name, struct zint_symbol **pp_symbol);
void testFinish(void);
void testSkip(const char *msg);
void testReport(void);

#define ZINT_TEST_CTX_EXC_MAX   32
typedef struct s_testCtx {
    int index;
    int index_end;
    int exclude[ZINT_TEST_CTX_EXC_MAX];
    int exclude_end[ZINT_TEST_CTX_EXC_MAX];
    int generate;
    int debug;
} testCtx;
typedef void (*testFunc_t)(const testCtx *const p_ctx);
typedef struct s_testFunction {
    const char *name; testFunc_t func;
} testFunction;
void testRun(int argc, char *argv[], testFunction funcs[], int funcs_size);
int testContinue(const testCtx *const p_ctx, const int i);

#if (defined(_MSC_VER) &&_MSC_VER <= 1200) || defined(ZINT_IS_C89) /* VC6 or C89 */
void assert_zero(int exp, const char *fmt, ...);
void assert_nonzero(int exp, const char *fmt, ...);
void assert_null(const void *exp, const char *fmt, ...);
void assert_nonnull(const void *exp, const char *fmt, ...);
void assert_equal(int e1, int e2, const char *fmt, ...);
void assert_equalu64(uint64_t e1, uint64_t e2, const char *fmt, ...);
void assert_notequal(int e1, int e2, const char *fmt, ...);
#else
#define assert_exp(exp, ...) \
    { testAssertNum++; if (!(exp)) { testAssertFailed++; printf("%s:%d ", testAssertFilename, __LINE__); \
      printf(__VA_ARGS__); testFinish(); \
      if (testAssertPPSymbol) { ZBarcode_Delete(*testAssertPPSymbol); testAssertPPSymbol = NULL; } return; } }

#define assert_zero(exp, ...) assert_exp((exp) == 0, __VA_ARGS__)
#define assert_nonzero(exp, ...) assert_exp((exp) != 0, __VA_ARGS__)
#define assert_null(ptr, ...) assert_exp((ptr) == NULL, __VA_ARGS__)
#define assert_nonnull(ptr, ...) assert_exp((ptr) != NULL, __VA_ARGS__)
#define assert_equal(e1, e2, ...) assert_exp((e1) == (e2), __VA_ARGS__)
#define assert_equalu64 assert_equal
#define assert_notequal(e1, e2, ...) assert_exp((e1) != (e2), __VA_ARGS__)
#endif

#define TU(p) ((unsigned char *) (p))

INTERNAL void vector_free(struct zint_symbol *symbol); /* Free vector structures */

int testUtilSetSymbol(struct zint_symbol *symbol, int symbology, int input_mode, int eci,
            int option_1, int option_2, int option_3, int output_options, const char *data, int length, int debug);

const char *testUtilBarcodeName(int symbology);
const char *testUtilErrorName(int error_number);
const char *testUtilInputModeName(int input_mode);
const char *testUtilOption3Name(int symbology, int option_3);
const char *testUtilOutputOptionsName(int output_options);

int testUtilDAFTConvert(const struct zint_symbol *symbol, char *buffer, const int buffer_size);
int testUtilIsValidUTF8(const unsigned char str[], const int length);
char *testUtilEscape(const char *buffer, const int length, char *escaped, const int escaped_size);
const char *testUtilReadCSVField(const char *buffer, char *field, const int field_size);
void testUtilStrCpyRepeat(char *buffer, const char *repeat, const int size);

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

void testUtilVectorPrint(const struct zint_symbol *symbol);

int testUtilDataPath(char *buffer, int buffer_size, const char *subdir, const char *filename);
FILE *testUtilOpen(const char *filename, const char *mode);
int testUtilExists(const char *filename);
int testUtilRemove(const char *filename);
int testUtilDirExists(const char *dirname);
int testUtilMkDir(const char *dirname);
int testUtilRmDir(const char *dirname);
int testUtilRename(const char *oldpath, const char *newpath);
int testUtilCreateROFile(const char *filename);
int testUtilRmROFile(const char *filename);
int testUtilReadFile(const char *filename, unsigned char *buffer, int buffer_size, int *p_size);
int testUtilWriteFile(const char *filename, const unsigned char *buffer, const int buffer_size, const char *mode);

int testUtilCmpPngs(const char *file1, const char *file2);
int testUtilCmpTxts(const char *txt1, const char *txt2);
int testUtilCmpBins(const char *bin1, const char *bin2);
int testUtilCmpSvgs(const char *svg1, const char *svg2);
int testUtilCmpEpss(const char *eps1, const char *eps2);

const char *testUtilHaveIdentify(void);
int testUtilVerifyIdentify(const char *const prog, const char *filename, int debug);
int testUtilHaveLibreOffice(void);
int testUtilVerifyLibreOffice(const char *filename, int debug);
int testUtilHaveGhostscript(void);
int testUtilVerifyGhostscript(const char *filename, int debug);
int testUtilHaveVnu(void);
int testUtilVerifyVnu(const char *filename, int debug);
int testUtilHaveTiffInfo(void);
int testUtilVerifyTiffInfo(const char *filename, int debug);

int testUtilCanBwipp(int index, const struct zint_symbol *symbol, int option_1, int option_2, int option_3,
            int debug);
int testUtilBwipp(int index, const struct zint_symbol *symbol, int option_1, int option_2, int option_3,
            const char *data, int length, const char *primary, char *buffer, int buffer_size, int *p_parsefnc);
int testUtilBwippSegs(int index, struct zint_symbol *symbol, int option_1, int option_2, int option_3,
            const struct zint_seg segs[], const int seg_count, const char *primary, char *buffer, int buffer_size);
int testUtilBwippCmp(const struct zint_symbol *symbol, char *msg, char *cmp_buf, const char *expected);
int testUtilBwippCmpRow(const struct zint_symbol *symbol, int row, char *msg, const char *cmp_buf,
            const char *expected);

int testUtilHaveZXingCPPDecoder(void);
int testUtilCanZXingCPP(int index, const struct zint_symbol *symbol, const char *data, const int length,
            const int debug);
int testUtilZXingCPP(int index, struct zint_symbol *symbol, const char *source, const int length, char *bits,
            char *buffer, const int buffer_size, int *p_cmp_len);
int testUtilZXingCPPSegs(int index, struct zint_symbol *symbol, const struct zint_seg segs[], const int seg_count, char *bits,
            char *buffer, const int buffer_size, int *p_cmp_len);
int testUtilZXingCPPCmp(struct zint_symbol *symbol, char *msg, char *cmp_buf, int cmp_len,
            const char *expected, int expected_len, const char *primary, char *ret_buf, int *p_ret_len);
int testUtilZXingCPPCmpSegs(struct zint_symbol *symbol, char *msg, char *cmp_buf, int cmp_len,
            const struct zint_seg segs[], const int seg_count, const char *primary, char *ret_buf, int *p_ret_len);

#ifdef __cplusplus
}
#endif

/* vim: set ts=4 sw=4 et : */
#endif /* Z_TESTCOMMON_H */
