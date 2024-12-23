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
 * Adapted from qrencode/tests/common.c
 * Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
 */
/* Due to above: */
/* SPDX-License-Identifier: LGPL-2.1+ */

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <wchar.h>
#endif
#ifndef ZINT_NO_PNG
#include <png.h>
#include <zlib.h>
#include <setjmp.h>
#endif
#include <assert.h>
#include <limits.h>
#include <sys/stat.h>

#include "testcommon.h"
#include "../eci.h"
#include "../output.h"

static int testTests = 0;
static int testFailed = 0;
static int testSkipped = 0;
static int testDataset = 0;
static int testDatasetNum = 0;
static int testDatasetTot = 0;
int testAssertFailed = 0;
int testAssertNum = 0;
static int testAssertTot = 0;
struct zint_symbol **testAssertPPSymbol = NULL;
const char *testAssertFilename = "";
static const char *testName = NULL;
static const char *testFunc = NULL;

/* Visual C++ 6 doesn't support variadic args to macros, so make do with functions, which have inferior behaviour,
   e.g. don't exit on failure, `assert_equal()` type-specific */
#if (defined(_MSC_VER) && _MSC_VER <= 1200) || defined(ZINT_IS_C89) /* VC6 or C89 */
#include <stdarg.h>
void assert_zero(int exp, const char *fmt, ...) {
    testAssertNum++;
    if (exp != 0) {
        va_list args; testAssertFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
        if (testAssertPPSymbol) { ZBarcode_Delete(*testAssertPPSymbol); testAssertPPSymbol = NULL; };
    }
}
void assert_nonzero(int exp, const char *fmt, ...) {
    testAssertNum++;
    if (exp == 0) {
        va_list args; testAssertFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
        if (testAssertPPSymbol) { ZBarcode_Delete(*testAssertPPSymbol); testAssertPPSymbol = NULL; };
    }
}
void assert_null(const void *exp, const char *fmt, ...) {
    testAssertNum++;
    if (exp != NULL) {
        va_list args; testAssertFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
        if (testAssertPPSymbol) { ZBarcode_Delete(*testAssertPPSymbol); testAssertPPSymbol = NULL; };
    }
}
void assert_nonnull(const void *exp, const char *fmt, ...) {
    testAssertNum++;
    if (exp == NULL) {
        va_list args; testAssertFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
        if (testAssertPPSymbol) { ZBarcode_Delete(*testAssertPPSymbol); testAssertPPSymbol = NULL; };
    }
}
void assert_equal(int e1, int e2, const char *fmt, ...) {
    testAssertNum++;
    if (e1 != e2) {
        va_list args; testAssertFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
        if (testAssertPPSymbol) { ZBarcode_Delete(*testAssertPPSymbol); testAssertPPSymbol = NULL; };
    }
}
void assert_equalu64(uint64_t e1, uint64_t e2, const char *fmt, ...) {
    testAssertNum++;
    if (e1 != e2) {
        va_list args; testAssertFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
        if (testAssertPPSymbol) { ZBarcode_Delete(*testAssertPPSymbol); testAssertPPSymbol = NULL; };
    }
}
void assert_notequal(int e1, int e2, const char *fmt, ...) {
    testAssertNum++;
    if (e1 == e2) {
        va_list args; testAssertFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
        if (testAssertPPSymbol) { ZBarcode_Delete(*testAssertPPSymbol); testAssertPPSymbol = NULL; };
    }
}
#endif

#ifdef _WIN32
#define utf8_to_wide(u, w) \
    { \
        int lenW; /* Includes NUL terminator */ \
        if ((lenW = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, u, -1, NULL, 0)) == 0) return 0; \
        w = (wchar_t *) z_alloca(sizeof(wchar_t) * lenW); \
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, u, -1, w, lenW) == 0) return 0; \
    }
#endif

/* Begin individual test function */
void testStartReal(const char *func, const char *name, struct zint_symbol **pp_symbol) {
    testTests++;
    if (func && *func && name && *name && strcmp(func, name) == 0) {
        testName = "";
    } else {
        testName = name;
    }
    testFunc = func ? func : "";
    testDataset = 0;
    testDatasetNum = 0;
    testAssertFailed = 0;
    testAssertNum = 0;
    testAssertPPSymbol = pp_symbol;
    printf("_____%d: %s: %s...\n", testTests, testFunc, testName ? testName : "");
}

/* End individual test function */
void testFinish(void) {
    testAssertTot += testAssertNum;
    testDatasetTot += testDatasetNum;
    fputs(testAssertFailed ? "*****" : ".....", stdout);
    if (testName && *testName) {
        printf("%d: %s: %s ", testTests, testFunc, testName);
    } else {
        printf("%d: %s: ", testTests, testFunc);
    }
    if (testAssertFailed) {
        printf("FAILED. (%d assertions failed)\n", testAssertFailed);
        testFailed++;
    } else if (testDataset) {
        if (testAssertNum) {
            printf("PASSED. (%d assertions, %d dataset items)\n", testAssertNum, testDatasetNum);
        } else {
            printf("EMPTY. (***No assertions executed***)\n");
        }
    } else {
        if (testAssertNum) {
            printf("PASSED. (%d assertions)\n", testAssertNum);
        } else {
            printf("EMPTY. (***No assertions executed***)\n");
        }
    }
}

/* Skip (and end) individual test function */
void testSkip(const char *msg) {
    testSkipped++;
    testAssertTot += testAssertNum;
    testDatasetTot += testDatasetNum;
    fputs(testAssertFailed ? "*****" : ".....", stdout);
    if (testName && *testName) {
        printf("%d: %s: %s ", testTests, testFunc, testName);
    } else {
        printf("%d: %s: ", testTests, testFunc);
    }
    if (testAssertFailed) {
        printf("FAILED. (%d assertions failed)\n", testAssertFailed);
        testFailed++;
    } else if (testDataset) {
        printf("SKIPPED. %s. (%d assertions, %d dataset items)\n", msg, testAssertNum, testDatasetNum);
    } else {
        printf("SKIPPED. %s. (%d assertions)\n", msg, testAssertNum);
    }
}

/* End test program */
void testReport(void) {
    if (testFailed && testSkipped) {
        printf("Total %d tests, %d skipped, %d **fails**.\n", testTests, testSkipped, testFailed);
        exit(-1);
    }
    if (testFailed) {
        printf("Total %d tests, %d **fails**.\n", testTests, testFailed);
        exit(-1);
    }
    if (testSkipped) {
        printf("Total %d tests, %d skipped.\n", testTests, testSkipped);
    } else if (testTests) {
        if (testAssertTot) {
            if (testDatasetTot) {
                printf("Total %d tests (%d assertions, %d dataset items), all passed.\n",
                        testTests, testAssertTot, testDatasetTot);
            } else {
                printf("Total %d tests (%d assertions), all passed.\n", testTests, testAssertTot);
            }
        } else {
            printf("***No assertions executed in %d tests.***\n", testTests);
        }
    } else {
        fputs("***No tests run.***\n", stdout);
    }
}

/* Verifies that a string `src` (length <= 9) only uses digits. On success returns value in `p_val` */
static int validate_int(const char src[], int *p_val) {
    int val = 0;
    int i;
    const int length = (int) strlen(src);

    if (length > 9) { /* Prevent overflow */
        return 0;
    }
    for (i = 0; i < length; i++) {
        if (src[i] < '0' || src[i] > '9') {
            return 0;
        }
        val *= 10;
        val += src[i] - '0';
    }
    *p_val = val;

    return 1;
}

/* Verifies that a string `src` only uses digits or a hyphen-separated range of digits.
   On success returns value in `p_val` and if present a range end value in `p_val_end` */
static int validate_int_range(const char src[], int *p_val, int *p_val_end) {
    int val = 0;
    int val_end = -1;
    const int length = (int) strlen(src);
    int i, j;

    for (i = 0; i < length; i++) {
        if (src[i] < '0' || src[i] > '9') {
            if (src[i] != '-') {
                return 0;
            }
            val_end = 0;
            for (j = i + 1; j < length; j++) {
                if (src[j] < '0' || src[j] > '9') {
                    return 0;
                }
                if (j - (i + 1) >= 9) { /* Prevent overflow */
                    return 0;
                }
                val_end *= 10;
                val_end += src[j] - '0';
            }
            break;
        }
        if (i >= 9) { /* Prevent overflow */
            return 0;
        }
        val *= 10;
        val += src[i] - '0';
    }
    *p_val = val;
    *p_val_end = val_end;

    return 1;
}

/* Begin test program, parse args */
void testRun(int argc, char *argv[], testFunction funcs[], int funcs_size) {
    int i, ran;
    char *optarg;
    char *func = NULL;
    char func_buf[256 + 5];
    char *func_not = NULL;
    char func_not_buf[256 + 5];
    char *func_match = NULL;
    int exclude_idx = 0;
    testCtx ctx;

    ctx.index = ctx.index_end = -1;
    for (i = 0; i < ZINT_TEST_CTX_EXC_MAX; i++) {
        ctx.exclude[i] = ctx.exclude_end[i] = -1;
    }
    ctx.generate = ctx.debug = 0;

    if (argc) {
        const char *filename;
#ifdef _WIN32
        if ((filename = strrchr(argv[0], '\\')) == NULL) {
            filename = strrchr(argv[0], '/');
        }
#else
        filename = strrchr(argv[0], '/');
#endif
        if (filename) {
            testAssertFilename = filename + 1;
        } else {
            testAssertFilename = argv[0];
        }
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "***testRun: -d debug value missing, ignoring***\n");
            } else {
                int d; /* Allow multiple debug flags, OR-ing */
                optarg = argv[++i];
                if (!validate_int(optarg, &d)) {
                    fprintf(stderr, "***testRun: -d debug value invalid, ignoring***\n");
                } else {
                    ctx.debug |= d;
                }
            }
        } else if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "***testRun: -f func value missing, ignoring***\n");
            } else {
                optarg = argv[++i];
                if (strlen(optarg) < 256) {
                    if (strncmp(optarg, "test_", 5) == 0) {
                        strcpy(func_buf, optarg);
                    } else {
                        strcpy(func_buf, "test_");
                        strcat(func_buf, optarg);
                    }
                    func = func_buf;
                } else {
                    fprintf(stderr, "***testRun: -f func value too long, ignoring***\n");
                    func = NULL;
                }
            }
        } else if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "***testRun: -n func exclude value missing, ignoring***\n");
            } else {
                optarg = argv[++i];
                if (strlen(optarg) < 256) {
                    if (strncmp(optarg, "test_", 5) == 0) {
                        strcpy(func_not_buf, optarg);
                    } else {
                        strcpy(func_not_buf, "test_");
                        strcat(func_not_buf, optarg);
                    }
                    func_not = func_not_buf;
                } else {
                    fprintf(stderr, "***testRun: -p func exclude value too long, ignoring***\n");
                    func_not = NULL;
                }
            }
        } else if (strcmp(argv[i], "-m") == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "***testRun: -m func match value missing, ignoring***\n");
            } else {
                func_match = argv[++i];
            }
        } else if (strcmp(argv[i], "-g") == 0) {
            ctx.generate = 1;
        } else if (strcmp(argv[i], "-i") == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "***testRun: -i index value missing, ignoring***\n");
            } else {
                optarg = argv[++i];
                if (!validate_int_range(optarg, &ctx.index, &ctx.index_end)) {
                    fprintf(stderr, "***testRun: -i index value invalid, ignoring***\n");
                    ctx.index = ctx.index_end = -1;
                }
            }
        } else if (strcmp(argv[i], "-x") == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "***testRun: -x exclude value missing, ignoring***\n");
            } else {
                optarg = argv[++i];
                if (exclude_idx + 1 == ZINT_TEST_CTX_EXC_MAX) {
                    fprintf(stderr, "***testRun: too many -x exclude values, ignoring***\n");
                } else if (!validate_int_range(optarg, &ctx.exclude[exclude_idx], &ctx.exclude_end[exclude_idx])) {
                    fprintf(stderr, "***testRun: -x exclude value invalid, ignoring***\n");
                    ctx.exclude[exclude_idx] = ctx.exclude_end[exclude_idx] = -1;
                } else {
                    exclude_idx++;
                }
            }
        } else {
            fprintf(stderr, "***testRun: unknown arg '%s', ignoring***\n", argv[i]);
        }
    }

    ran = 0;
    for (i = 0; i < funcs_size; i++) {
        if (func && strcmp(func, funcs[i].name) != 0) {
            continue;
        }
        if (func_not && strcmp(func_not, funcs[i].name) == 0) {
            continue;
        }
        if (func_match && strstr(funcs[i].name, func_match) == NULL) {
            continue;
        }
        (*funcs[i].func)(&ctx);
        ran++;
    }

    if (func && !ran) {
        fprintf(stderr, "***testRun: unknown -f func arg '%s'***\n", func);
    }
    if (func_match && !ran) {
        fprintf(stderr, "***testRun: no funcs matched -m arg '%s'***\n", func_match);
    }
}

/* Call in a dataset loop to determine if a datum should be tested according to -i & -x args */
int testContinue(const testCtx *const p_ctx, const int i) {
    int j;
    testDataset = 1;
    if (p_ctx->index != -1) {
        if (p_ctx->index_end != -1) {
            if (i < p_ctx->index || (p_ctx->index_end && i > p_ctx->index_end)) {
                return 1;
            }
        } else if (i != p_ctx->index) {
            return 1;
        }
    }
    for (j = 0; j < ZINT_TEST_CTX_EXC_MAX && p_ctx->exclude[j] != -1; j++) {
        if (p_ctx->exclude_end[j] != -1) {
            if (i >= p_ctx->exclude[j] && (p_ctx->exclude_end[j] == 0 || i <= p_ctx->exclude_end[j])) {
                return 1;
            }
        } else if (i == p_ctx->exclude[j]) {
            return 1;
        }
    }
    if ((p_ctx->debug & ZINT_DEBUG_TEST_PRINT) && !(p_ctx->debug & ZINT_DEBUG_TEST_LESS_NOISY)) {
        printf("i:%d\n", i);
        fflush(stdout); /* For assertion failures */
    }
    testDatasetNum++;
    return 0;
}

/* Helper to set common symbol fields */
int testUtilSetSymbol(struct zint_symbol *symbol, int symbology, int input_mode, int eci, int option_1, int option_2,
            int option_3, int output_options, const char *data, int length, int debug) {
    symbol->symbology = symbology;
    if (input_mode != -1) {
        symbol->input_mode = input_mode;
    }
    if (eci != -1) {
        symbol->eci = eci;
    }
    if (option_1 != -1) {
        symbol->option_1 = option_1;
    }
    if (option_2 != -1) {
        symbol->option_2 = option_2;
    }
    if (option_3 != -1) {
        symbol->option_3 = option_3;
    }
    if (output_options != -1) {
        symbol->output_options = output_options;
    }
    symbol->debug |= debug;
    if (length == -1) {
        length = (int) strlen(data);
    }

    return length;
}

/* Pretty name for symbology */
const char *testUtilBarcodeName(int symbology) {
    static char name[32];

    if (ZBarcode_BarcodeName(symbology, name) == -1) {
        fprintf(stderr, "testUtilBarcodeName: data table out of sync (%d)\n", symbology);
        abort();
    }
    return name;
}

/* Pretty name for error/warning */
const char *testUtilErrorName(int error_number) {
    struct item {
        const char *name;
        int define;
        int val;
    };
    static const struct item data[] = {
        { "0", 0, 0 },
        { "ZINT_WARN_HRT_TRUNCATED", ZINT_WARN_HRT_TRUNCATED, 1 },
        { "ZINT_WARN_INVALID_OPTION", ZINT_WARN_INVALID_OPTION, 2 },
        { "ZINT_WARN_USES_ECI", ZINT_WARN_USES_ECI, 3 },
        { "ZINT_WARN_NONCOMPLIANT", ZINT_WARN_NONCOMPLIANT, 4 },
        { "ZINT_ERROR_TOO_LONG", ZINT_ERROR_TOO_LONG, 5 },
        { "ZINT_ERROR_INVALID_DATA", ZINT_ERROR_INVALID_DATA, 6 },
        { "ZINT_ERROR_INVALID_CHECK", ZINT_ERROR_INVALID_CHECK, 7 },
        { "ZINT_ERROR_INVALID_OPTION", ZINT_ERROR_INVALID_OPTION, 8 },
        { "ZINT_ERROR_ENCODING_PROBLEM", ZINT_ERROR_ENCODING_PROBLEM, 9 },
        { "ZINT_ERROR_FILE_ACCESS", ZINT_ERROR_FILE_ACCESS, 10 },
        { "ZINT_ERROR_MEMORY", ZINT_ERROR_MEMORY, 11 },
        { "ZINT_ERROR_FILE_WRITE", ZINT_ERROR_FILE_WRITE, 12 },
        { "ZINT_ERROR_USES_ECI", ZINT_ERROR_USES_ECI, 13 },
        { "ZINT_ERROR_NONCOMPLIANT", ZINT_ERROR_NONCOMPLIANT, 14 },
        { "ZINT_ERROR_HRT_TRUNCATED", ZINT_ERROR_HRT_TRUNCATED, 15 },
    };
    const int data_size = ARRAY_SIZE(data);

    if (error_number < 0 || error_number >= data_size) {
        return "";
    }
    /* Self-check */
    if (data[error_number].val != error_number
            || (data[error_number].define != -1 && data[error_number].define != error_number)) {
        fprintf(stderr, "testUtilErrorName: data table out of sync (%d)\n", error_number);
        abort();
    }
    return data[error_number].name;
}

/* Pretty name for input mode */
const char *testUtilInputModeName(int input_mode) {
    static char buf[512];

    struct item {
        const char *name;
        int define;
        int val;
    };
    static const struct item data[] = {
        { "ESCAPE_MODE", ESCAPE_MODE, 0x0008 },
        { "GS1PARENS_MODE", GS1PARENS_MODE, 0x0010 },
        { "GS1NOCHECK_MODE", GS1NOCHECK_MODE, 0x0020 },
        { "HEIGHTPERROW_MODE", HEIGHTPERROW_MODE, 0x0040 },
        { "FAST_MODE", FAST_MODE, 0x0080 },
        { "EXTRA_ESCAPE_MODE", EXTRA_ESCAPE_MODE, 0x0100 },
    };
    const int data_size = ARRAY_SIZE(data);
    int set, i;

    if (input_mode < 0) {
        return "-1";
    }
    *buf = '\0';
    if ((input_mode & 0x7) & UNICODE_MODE) {
        strcpy(buf, "UNICODE_MODE");
        set = UNICODE_MODE;
    } else if ((input_mode & 0x7) & GS1_MODE) {
        strcpy(buf, "GS1_MODE");
        set = GS1_MODE;
    } else {
        set = DATA_MODE;
    }
    for (i = 0; i < data_size; i++) {
        if (data[i].define != data[i].val) { /* Self-check */
            fprintf(stderr, "testUtilInputModeName: data table out of sync (%d)\n", i);
            abort();
        }
        if (input_mode & data[i].define) {
            if (*buf) {
                strcat(buf, " | ");
            }
            strcat(buf, data[i].name);
            set |= data[i].define;
        }
    }
    if (set != input_mode) {
        fprintf(stderr, "testUtilInputModeName: unknown input mode %d (%d)\n", input_mode & set, input_mode);
        abort();
    }
    if (set == DATA_MODE && *buf == '\0') {
        strcpy(buf, "DATA_MODE");
    }
    return buf;
}

/* Pretty name for option 3 */
const char *testUtilOption3Name(int symbology, int option_3) {
    static char buffer[64];

    const char *name = NULL;
    const unsigned int high_byte = option_3 == -1 ? 0 : (option_3 >> 8) & 0xFF;

    if (symbology == BARCODE_DATAMATRIX || symbology == BARCODE_HIBC_DM) {
        if ((option_3 & 0x7F) == DM_SQUARE) {
            if ((option_3 & DM_ISO_144) == DM_ISO_144) {
                name = "DM_SQUARE | DM_ISO_144";
            } else {
                name = "DM_SQUARE";
            }
        } else if ((option_3 & 0x7F) == DM_DMRE) {
            if ((option_3 & DM_ISO_144) == DM_ISO_144) {
                name = "DM_DMRE | DM_ISO_144";
            } else {
                name = "DM_DMRE";
            }
        } else if ((option_3 & DM_ISO_144) == DM_ISO_144) {
            name = "DM_ISO_144";
        } else {
            name = (option_3 & 0xFF) ? "-1" : "0";
        }
    } else if (symbology == BARCODE_QRCODE || symbology == BARCODE_HIBC_QR || symbology == BARCODE_MICROQR
                || symbology == BARCODE_RMQR || symbology == BARCODE_GRIDMATRIX || symbology == BARCODE_HANXIN) {
        if ((option_3 & 0xFF) == ZINT_FULL_MULTIBYTE) {
            name = "ZINT_FULL_MULTIBYTE";
        } else {
            name = (option_3 & 0xFF) ? "-1" : "0";
        }
    } else if (symbology == BARCODE_ULTRA) {
        if ((option_3 & 0xFF) == ULTRA_COMPRESSION) {
            name = "ULTRA_COMPRESSION";
        } else {
            name = (option_3 & 0xFF) ? "-1" : "0";
        }
    } else {
        if (option_3 != -1 && (option_3 & 0xFF) != 0) {
            fprintf(stderr, "testUtilOption3Name: unknown value (%d)\n", option_3);
            abort();
        }
        name = (option_3 & 0xFF) ? "-1" : "0";
    }

    if (high_byte) {
        if (option_3 & 0xFF) {
            sprintf(buffer, "%s | (%d << 8)", name, (int) high_byte);
        } else {
            sprintf(buffer, "%d << 8", (int) high_byte);
        }
        return buffer;
    }

    return name;
}

/* Pretty name for output options */
const char *testUtilOutputOptionsName(int output_options) {
    static char buf[512];

    struct item {
        const char *name;
        int define;
        int val;
    };
    static const struct item data[] = {
        { "BARCODE_BIND_TOP", BARCODE_BIND_TOP, 1 },
        { "BARCODE_BIND", BARCODE_BIND, 2 },
        { "BARCODE_BOX", BARCODE_BOX, 4 },
        { "BARCODE_STDOUT", BARCODE_STDOUT, 8 },
        { "READER_INIT", READER_INIT, 16 },
        { "SMALL_TEXT", SMALL_TEXT, 32 },
        { "BOLD_TEXT", BOLD_TEXT, 64 },
        { "CMYK_COLOUR", CMYK_COLOUR, 128 },
        { "BARCODE_DOTTY_MODE", BARCODE_DOTTY_MODE, 256 },
        { "GS1_GS_SEPARATOR", GS1_GS_SEPARATOR, 512 },
        { "OUT_BUFFER_INTERMEDIATE", OUT_BUFFER_INTERMEDIATE, 1024 },
        { "BARCODE_QUIET_ZONES", BARCODE_QUIET_ZONES, 2048 },
        { "BARCODE_NO_QUIET_ZONES", BARCODE_NO_QUIET_ZONES, 4096 },
        { "COMPLIANT_HEIGHT", COMPLIANT_HEIGHT, 0x2000 },
        { "EANUPC_GUARD_WHITESPACE", EANUPC_GUARD_WHITESPACE, 0x4000 },
        { "EMBED_VECTOR_FONT", EMBED_VECTOR_FONT, 0x8000 },
    };
    static int const data_size = ARRAY_SIZE(data);
    int set = 0;
    int i;

    if (output_options == -1) {
        return "-1";
    }
    if (output_options == 0) {
        return "0";
    }
    buf[0] = '\0';
    for (i = 0; i < data_size; i++) {
        if (data[i].define != data[i].val) { /* Self-check */
            fprintf(stderr, "testUtilOutputOptionsName: data table out of sync (%d)\n", i);
            abort();
        }
        if (output_options & data[i].define) {
            if (set) {
                strcat(buf, " | ");
            }
            strcat(buf, data[i].name);
            set |= data[i].define;
        }
    }
    if (set != output_options) {
        fprintf(stderr, "testUtilOutputOptionsName: unknown output option(s) %d (%d, 0x%X)\n",
                output_options & set, output_options, output_options);
        abort();
    }
    return buf;
}

/* Convert modules spanning 3 rows to DAFT equivalents */
int testUtilDAFTConvert(const struct zint_symbol *symbol, char *buffer, const int buffer_size) {
    int i;
    char *b = buffer;
    *b = '\0';
    for (i = 0; i < symbol->width && b < buffer + buffer_size; i += 2) {
        if (module_is_set(symbol, 0, i) && module_is_set(symbol, 2, i)) {
            *b++ = 'F';
        } else if (module_is_set(symbol, 0, i)) {
            *b++ = 'A';
        } else if (module_is_set(symbol, 2, i)) {
            *b++ = 'D';
        } else {
            *b++ = 'T';
        }
    }
    if (b == buffer + buffer_size) {
        return 0;
    }
    *b = '\0';
    return 1;
}

/* Is string valid UTF-8? */
int testUtilIsValidUTF8(const unsigned char str[], const int length) {
    int i;
    unsigned int codepoint, state = 0;

    for (i = 0; i < length; i++) {
        if (decode_utf8(&state, &codepoint, str[i]) == 12) {
            return 0;
        }
    }

    return state == 0;
}

/* Escape data for printing on generate test. Has a number of issues, e.g. need to use octal escapes */
char *testUtilEscape(const char *buffer, const int length, char *escaped, const int escaped_size) {
    int i;
    unsigned char *b = (unsigned char *) buffer;
    unsigned char *be = b + length;
    int non_utf8 = !testUtilIsValidUTF8(b, length);
    int chunk = -1;

    for (i = 0; b < be && i < escaped_size; b++) {
         /* For VC6-compatibility need to split literal strings into <= 2K chunks */
         if (i > 2040 && i / 2040 != chunk && (*b & 0xC0) != 0x80) { /* Avoid UTF-8 continuations */
            chunk = i / 2040;
            if (i + 3 < escaped_size) {
                escaped[i] = '"';
                escaped[i + 1] = ' ';
                escaped[i + 2] = '"';
            }
            i += 3;
         }
         if (non_utf8 || *b < ' ' || *b == '\177') {
            if (i + 4 < escaped_size) {
                sprintf(escaped + i, "\\%.3o", *b);
            }
            i += 4;
        } else if (*b == '\\' || *b == '"') {
            if (i + 2 < escaped_size) {
                escaped[i] = '\\';
                escaped[i + 1] = *b;
            }
            i += 2;
        } else if (b + 1 < be && *b == 0xC2 && *(b + 1) < 0xA0) {
            if (i + 8 < escaped_size) {
                sprintf(escaped + i, "\\%.3o\\%.3o", *b, *(b + 1));
            }
            i += 8;
            b++;
        } else {
            escaped[i++] = *b;
        }
    }
    if (i >= escaped_size) {
        return NULL;
    }
    escaped[i] = '\0';
    return escaped;
}

/* Helper to read a CSV field */
const char *testUtilReadCSVField(const char *buffer, char *field, const int field_size) {
    int i;
    const char *b = buffer;
    for (i = 0; i < field_size && *b && *b != ',' && *b != '\n' && *b != '\r'; i++) {
        field[i] = *b++;
    }
    if (i == field_size) {
        return NULL;
    }
    field[i] = '\0';
    return b;
}

/* Helper to fill a buffer (for "large" tests) - single-byte filler only */
void testUtilStrCpyRepeat(char *buffer, const char *repeat, const int size) {
    int i;
    int len = (int) strlen(repeat);
    int max = size - len;
    if (len == 0) {
        fprintf(stderr, "testUtilStrCpyRepeat: only use non-empty, non-NUL single-byte data for repeat pattern\n");
        abort();
    }
    for (i = 0; i < max; i += len) {
        memcpy(buffer + i, repeat, len);
    }
    memcpy(buffer + i, repeat, size - i);
    buffer[size] = '\0';
}

/* Compare some "important" symbol fields for equality */
int testUtilSymbolCmp(const struct zint_symbol *a, const struct zint_symbol *b) {
    int i, j;
    if (a->symbology != b->symbology) {
        return 1;
    }
    if (a->rows != b->rows) {
        return 2;
    }
    if (a->width != b->width) {
        return 3;
    }
    if (a->symbology == BARCODE_ULTRA) {
        for (i = 0; i < a->rows; i++) {
            for (j = 0; j < a->width; j++) {
                if (module_colour_is_set(a, i, j) != module_colour_is_set(b, i, j)) {
                    return 4;
                }
            }
        }
    } else {
        for (i = 0; i < a->rows; i++) {
            for (j = 0; j < a->width; j++) {
                if (module_is_set(a, i, j) != module_is_set(b, i, j)) {
                    return 4;
                }
            }
        }
    }
    if (a->height != b->height) {
        return 5;
    }
    if (a->whitespace_width != b->whitespace_width) {
        return 6;
    }
    if (a->whitespace_height != b->whitespace_height) {
        return 7;
    }
    if (a->border_width != b->border_width) {
        return 8;
    }
    if (a->output_options != b->output_options) {
        return 9;
    }
    if (a->scale != b->scale) {
        return 10;
    }

    return 0;
}

/* Copy a full vector structure (for later comparison) */
struct zint_vector *testUtilVectorCpy(const struct zint_vector *in) {
    struct zint_vector_rect *rect;
    struct zint_vector_string *string;
    struct zint_vector_circle *circle;
    struct zint_vector_hexagon *hexagon;

    struct zint_vector_rect **outrect;
    struct zint_vector_string **outstring;
    struct zint_vector_circle **outcircle;
    struct zint_vector_hexagon **outhexagon;

    struct zint_vector *out = malloc(sizeof(struct zint_vector));
    assert(out != NULL);
    out->width = in->width;
    out->height = in->height;
    out->rectangles = NULL;
    out->strings = NULL;
    out->circles = NULL;
    out->hexagons = NULL;

    /* Copy rectangles */
    rect = in->rectangles;
    outrect = &(out->rectangles);
    while (rect) {
        *outrect = malloc(sizeof(struct zint_vector_rect));
        assert(*outrect != NULL);
        memcpy(*outrect, rect, sizeof(struct zint_vector_rect));
        outrect = &((*outrect)->next);
        rect = rect->next;
    }
    *outrect = NULL;

    /* Copy Strings */
    string = in->strings;
    outstring = &(out->strings);
    while (string) {
        *outstring = malloc(sizeof(struct zint_vector_string));
        assert(*outstring != NULL);
        memcpy(*outstring, string, sizeof(struct zint_vector_string));
        (*outstring)->text = malloc(ustrlen(string->text) + 1);
        assert((*outstring)->text != NULL);
        ustrcpy((*outstring)->text, string->text);
        outstring = &((*outstring)->next);
        string = string->next;
    }
    *outstring = NULL;

    /* Copy Circles */
    circle = in->circles;
    outcircle = &(out->circles);
    while (circle) {
        *outcircle = malloc(sizeof(struct zint_vector_circle));
        assert(*outcircle != NULL);
        memcpy(*outcircle, circle, sizeof(struct zint_vector_circle));
        outcircle = &((*outcircle)->next);
        circle = circle->next;
    }
    *outcircle = NULL;

    /* Copy Hexagons */
    hexagon = in->hexagons;
    outhexagon = &(out->hexagons);
    while (hexagon) {
        *outhexagon = malloc(sizeof(struct zint_vector_hexagon));
        assert(*outhexagon != NULL);
        memcpy(*outhexagon, hexagon, sizeof(struct zint_vector_hexagon));
        outhexagon = &((*outhexagon)->next);
        hexagon = hexagon->next;
    }
    *outhexagon = NULL;

    return out;
}

/* Compare 2 full vector structures */
int testUtilVectorCmp(const struct zint_vector *a, const struct zint_vector *b) {
    struct zint_vector_rect *arect;
    struct zint_vector_string *astring;
    struct zint_vector_circle *acircle;
    struct zint_vector_hexagon *ahexagon;

    struct zint_vector_rect *brect;
    struct zint_vector_string *bstring;
    struct zint_vector_circle *bcircle;
    struct zint_vector_hexagon *bhexagon;

    if (a->width != b->width) {
        return 1;
    }
    if (a->height != b->height) {
        return 2;
    }

    /* Compare rectangles */
    arect = a->rectangles;
    brect = b->rectangles;
    while (arect) {
        if (!brect) {
            return 11;
        }
        if (arect->x != brect->x) {
            return 12;
        }
        if (arect->y != brect->y) {
            return 13;
        }
        if (arect->height != brect->height) {
            return 14;
        }
        if (arect->width != brect->width) {
            return 15;
        }
        if (arect->colour != brect->colour) {
            return 16;
        }
        arect = arect->next;
        brect = brect->next;
    }
    if (brect) {
        return 10;
    }

    /* Compare strings */
    astring = a->strings;
    bstring = b->strings;
    while (astring) {
        if (!bstring) {
            return 21;
        }
        if (astring->x != bstring->x) {
            return 22;
        }
        if (astring->y != bstring->y) {
            return 23;
        }
        if (astring->fsize != bstring->fsize) {
            return 24;
        }
        if (astring->width != bstring->width) {
            return 25;
        }
        if (astring->length != bstring->length) {
            return 26;
        }
        if (ustrlen(astring->text) != ustrlen(bstring->text)) {
            return 27;
        }
        if (strcmp((const char *) astring->text, (const char *) bstring->text) != 0) {
            return 28;
        }
        astring = astring->next;
        bstring = bstring->next;
    }
    if (bstring) {
        return 20;
    }

    /* Compare circles */
    acircle = a->circles;
    bcircle = b->circles;
    while (acircle) {
        if (!bcircle) {
            return 31;
        }
        if (acircle->x != bcircle->x) {
            return 32;
        }
        if (acircle->y != bcircle->y) {
            return 33;
        }
        if (acircle->diameter != bcircle->diameter) {
            return 34;
        }
        if (acircle->colour != bcircle->colour) {
            return 35;
        }
        acircle = acircle->next;
        bcircle = bcircle->next;
    }
    if (bcircle) {
        return 30;
    }

    /* Compare hexagons */
    ahexagon = a->hexagons;
    bhexagon = b->hexagons;
    while (ahexagon) {
        if (!bhexagon) {
            return 41;
        }
        if (ahexagon->x != bhexagon->x) {
            return 42;
        }
        if (ahexagon->y != bhexagon->y) {
            return 43;
        }
        if (ahexagon->diameter != bhexagon->diameter) {
            return 44;
        }
        ahexagon = ahexagon->next;
        bhexagon = bhexagon->next;
    }
    if (bhexagon) {
        return 40;
    }

    return 0;
}

/* Dump modules into buffer as '0'/'1' (or colours '0', '1', '2' etc if Ultra) */
int testUtilModulesDump(const struct zint_symbol *symbol, char dump[], int dump_size) {
    int r, w;
    char *d = dump;
    char *de = dump + dump_size;

    for (r = 0; r < symbol->rows && d < de; r++) {
        if (symbol->symbology == BARCODE_ULTRA) {
            for (w = 0; w < symbol->width && d < de; w++) {
                *d++ = module_colour_is_set(symbol, r, w) + '0';
            }
        } else {
            for (w = 0; w < symbol->width && d < de; w++) {
                *d++ = module_is_set(symbol, r, w) + '0';
            }
        }
    }
    if (d == de) {
        return -1;
    }
    *d = '\0';
    return d - dump;
}

/* Print out module dump (for generate tests) */
void testUtilModulesPrint(const struct zint_symbol *symbol, const char *prefix, const char *postfix) {
    int r;
    for (r = 0; r < symbol->rows; r++) {
        testUtilModulesPrintRow(symbol, r, prefix, postfix);
    }
}

/* Print out a single row of a module dump (for generate tests where rows all the same, to avoid large dumps of
   duplicate data) */
void testUtilModulesPrintRow(const struct zint_symbol *symbol, int row, const char *prefix, const char *postfix) {
    int w;
    if (*prefix) {
        fputs(prefix, stdout);
    }
    putchar('"');
    if (symbol->symbology == BARCODE_ULTRA) {
        for (w = 0; w < symbol->width; w++) {
            putchar(module_colour_is_set(symbol, row, w) + '0');
        }
    } else {
        for (w = 0; w < symbol->width; w++) {
            putchar(module_is_set(symbol, row, w) + '0');
        }
    }
    putchar('"');
    if (*postfix) {
        fputs(postfix, stdout);
    }
}

/* Whether 2 module dumps are the same */
int testUtilModulesCmp(const struct zint_symbol *symbol, const char *expected, int *width, int *row) {
    const char *e = expected;
    const char *ep = expected + strlen(expected);
    int r, w = 0;
    if (symbol->symbology == BARCODE_ULTRA) {
        for (r = 0; r < symbol->rows && e < ep; r++) {
            for (w = 0; w < symbol->width && e < ep; w++) {
                if (module_colour_is_set(symbol, r, w) + '0' != *e) {
                    *row = r;
                    *width = w;
                    return 1 /*fail*/;
                }
                e++;
            }
        }
    } else {
        for (r = 0; r < symbol->rows && e < ep; r++) {
            for (w = 0; w < symbol->width && e < ep; w++) {
                if (module_is_set(symbol, r, w) + '0' != *e) {
                    *row = r;
                    *width = w;
                    return 1 /*fail*/;
                }
                e++;
            }
        }
    }
    *row = r;
    *width = w;
    return e != ep || r != symbol->rows || w != symbol->width ? 1 /*fail*/ : 0 /*success*/;
}

/* Whether 2 module row dumps are the same */
int testUtilModulesCmpRow(const struct zint_symbol *symbol, int row, const char *expected, int *width) {
    const char *e = expected;
    const char *ep = expected + strlen(expected);
    int w;
    if (symbol->symbology == BARCODE_ULTRA) {
        for (w = 0; w < symbol->width && e < ep; w++) {
            if (module_colour_is_set(symbol, row, w) + '0' != *e) {
                *width = w;
                return 1 /*fail*/;
            }
            e++;
        }
    } else {
        for (w = 0; w < symbol->width && e < ep; w++) {
            if (module_is_set(symbol, row, w) + '0' != *e) {
                *width = w;
                return 1 /*fail*/;
            }
            e++;
        }
    }
    *width = w;
    return e != ep || w != symbol->width ? 1 /*fail*/ : 0 /*success*/;
}

/* Dump an unsigned int array as hex */
char *testUtilUIntArrayDump(unsigned int *array, int size, char *dump, int dump_size) {
    int i, cnt_len = 0;

    for (i = 0; i < size; i++) {
        cnt_len += sprintf(dump + cnt_len, "%X ", array[i]);
        if (cnt_len + 17 >= dump_size) {
            break;
        }
    }
    dump[cnt_len ? cnt_len - 1 : 0] = '\0';
    return dump;
}

/* Dump an unsigned char array as hex */
char *testUtilUCharArrayDump(unsigned char *array, int size, char *dump, int dump_size) {
    int i, cnt_len = 0;

    for (i = 0; i < size; i++) {
        cnt_len += sprintf(dump + cnt_len, "%X ", array[i]);
        if (cnt_len + 3 >= dump_size) {
            break;
        }
    }
    dump[cnt_len ? cnt_len - 1 : 0] = '\0';
    return dump;
}

/* Dump a bitmap to stdout, for generate tests. Also useful for debugging */
void testUtilBitmapPrint(const struct zint_symbol *symbol, const char *prefix, const char *postfix) {
    static const char colour[] = { '0', 'C', 'M', 'B', 'Y', 'G', 'R', '1' };
    int row, column, i, j;

    if (!prefix) {
        fputs("     ", stdout);
        for (column = 0; column < symbol->bitmap_width; column += 10) printf("%-3d       ", column);
        fputs("\n     ", stdout);
        for (column = 0; column < symbol->bitmap_width; column++) printf("%d", column % 10);
        putchar('\n');
    }

    for (row = 0; row < symbol->bitmap_height; row++) {
        if (!prefix) {
            printf("%3d: ", row);
        } else {
            if (*prefix) {
                fputs(prefix, stdout);
            }
            putchar('"');
        }
        for (column = 0; column < symbol->bitmap_width; column++) {
            if (symbol->output_options & OUT_BUFFER_INTERMEDIATE) {
                putchar(symbol->bitmap[(row * symbol->bitmap_width) + column]);
            } else {
                i = ((row * symbol->bitmap_width) + column) * 3;
                if ((symbol->bitmap[i] == 0 || symbol->bitmap[i] == 0xff)
                        && (symbol->bitmap[i + 1] == 0 || symbol->bitmap[i + 1] == 0xff)
                        && (symbol->bitmap[i + 2] == 0 || symbol->bitmap[i + 2] == 0xff)) {
                    j = !symbol->bitmap[i] + !symbol->bitmap[i + 1] * 2 + !symbol->bitmap[i + 2] * 4;
                    putchar(colour[j]);
                } else {
                    printf("%02X%02X%02X", symbol->bitmap[i], symbol->bitmap[i + 1], symbol->bitmap[i + 2]);
                }
            }
        }
        if (!postfix) {
            putchar('\n');
        } else {
            putchar('"');
            if (*postfix) {
                fputs(postfix, stdout);
            }
        }
    }

    if (!postfix) {
        fputs("     ", stdout);
        for (column = 0; column < symbol->bitmap_width; column++) printf("%d", column % 10);
        fputs("\n     ", stdout);
        for (column = 0; column < symbol->bitmap_width; column += 10) printf("%-3d       ", column);
        putchar('\n');
    }
    fflush(stdout);
}

/* Compare a bitmap to a dump */
int testUtilBitmapCmp(const struct zint_symbol *symbol, const char *expected, int *row, int *column) {
    static const char colour[] = { '0', 'C', 'M', 'B', 'Y', 'G', 'R', '1' };
    int r, c = -1, i, j;
    const char *e = expected;
    const char *ep = expected + strlen(expected);
    char buf[7];

    for (r = 0; r < symbol->bitmap_height; r++) {
        for (c = 0; c < symbol->bitmap_width; c++) {
            if (symbol->output_options & OUT_BUFFER_INTERMEDIATE) {
                if (*e != symbol->bitmap[(r * symbol->bitmap_width) + c]) {
                    *row = r;
                    *column = c;
                    return 1 /*fail*/;
                }
                e++;
            } else {
                i = ((r * symbol->bitmap_width) + c) * 3;
                if ((symbol->bitmap[i] == 0 || symbol->bitmap[i] == 0xff)
                        && (symbol->bitmap[i + 1] == 0 || symbol->bitmap[i + 1] == 0xff)
                        && (symbol->bitmap[i + 2] == 0 || symbol->bitmap[i + 2] == 0xff)) {
                    j = !symbol->bitmap[i] + !symbol->bitmap[i + 1] * 2 + !symbol->bitmap[i + 2] * 4;
                    if (*e != colour[j]) {
                        *row = r;
                        *column = c;
                        return 1 /*fail*/;
                    }
                    e++;
                } else {
                    sprintf(buf, "%02X%02X%02X", symbol->bitmap[i], symbol->bitmap[i + 1], symbol->bitmap[i + 2]);
                    if (strncmp(buf, e, 6) != 0) {
                        *row = r;
                        *column = c;
                        return 1 /*fail*/;
                    }
                    e += 6;
                }
            }
        }
    }

    *row = r;
    *column = c;
    return e != ep || r != symbol->bitmap_height || c != symbol->bitmap_width ? 1 /*fail*/ : 0 /*success*/;
}

/* Dump vectors to stdout, for debugging */
void testUtilVectorPrint(const struct zint_symbol *symbol) {
    struct zint_vector_rect *rect;
    struct zint_vector_hexagon *hex;
    struct zint_vector_circle *circ;
    struct zint_vector_string *str;

    if (symbol->vector == NULL) {
        fputs("symbol->vector NULL\n", stdout);
    } else {
        for (rect = symbol->vector->rectangles; rect; rect = rect->next) {
            printf("rect(x %.9g, y %.9g, width %.9g, height %.9g, colour %d)\n", rect->x, rect->y, rect->width,
                    rect->height, rect->colour);
        }
        for (hex = symbol->vector->hexagons; hex; hex = hex->next) {
            printf("hex(x %.9g, y %.9g, diameter %.9g, rotation %d)\n", hex->x, hex->y, hex->diameter, hex->rotation);
        }
        for (circ = symbol->vector->circles; circ; circ = circ->next) {
            printf("circ(x %.9g, y %.9g, diameter %.9g, width %.9g, colour %d)\n", circ->x, circ->y, circ->diameter,
                    circ->width, circ->colour);
        }
        for (str = symbol->vector->strings; str; str = str->next) {
            printf("str(x %.9g, y %.9g, fsize %.9g, width %.9g, length %d, rotation %d, halign %d, \"%s\")\n", str->x,
                    str->y, str->fsize, str->width, str->length, str->rotation, str->halign, str->text);
        }
    }
    fflush(stdout);
}

/* Determine the location of test data relative to where the test is being run */
int testUtilDataPath(char *buffer, int buffer_size, const char *subdir, const char *filename) {
    int subdir_len = subdir ? (int) strlen(subdir) : 0;
    int filename_len = filename ? (int) strlen(filename) : 0;
    char *s, *s2;
    int len;
    char *cmake_src_dir;
#ifdef _WIN32
    int i;
#endif

    if ((cmake_src_dir = getenv("CMAKE_CURRENT_SOURCE_DIR")) != NULL) {
        len = (int) strlen(cmake_src_dir);
        if (len <= 0 || len >= buffer_size) {
            fprintf(stderr, "testUtilDataPath: warning CMAKE_CURRENT_SOURCE_DIR len %d, ignoring\n", len);
            cmake_src_dir = NULL;
        } else {
            strcpy(buffer, cmake_src_dir);
        }
    }

    if (cmake_src_dir == NULL) {
        if (getcwd(buffer, buffer_size) == NULL) {
            fprintf(stderr, "testUtilDataPath: getcwd NULL buffer_size %d\n", buffer_size);
            return 0;
        }
        len = (int) strlen(buffer);

        if (len <= 0) {
            fprintf(stderr, "testUtilDataPath: strlen <= 0\n");
            return 0;
        }
    }
#ifdef _WIN32
    for (i = 0; i < len; i++) {
        if (buffer[i] == '\\') {
            buffer[i] = '/';
        }
    }
#endif
    if (buffer[len - 1] == '/') {
        buffer[len--] = '\0';
    }
    if (len == 0) {
        fprintf(stderr, "testUtilDataPath: len == 0\n");
        return 0;
    }

    if ((s = strstr(buffer, "/tests")) != NULL) {
        while ((s2 = strstr(s + 1, "/tests")) != NULL) { /* Find rightmost */
            s = s2;
        }
        *s = '\0';
        len = s - buffer;
    }
    if ((s = strstr(buffer, "/backend")) != NULL) {
        while ((s2 = strstr(s + 1, "/backend")) != NULL) { /* Find rightmost */
            s = s2;
        }
        *s = '\0';
        len = s - buffer;
    } else if ((s = strstr(buffer, "/frontend")) != NULL) {
        while ((s2 = strstr(s + 1, "/frontend")) != NULL) { /* Find rightmost */
            s = s2;
        }
        *s = '\0';
        len = s - buffer;
    }
    if (cmake_src_dir == NULL && (s = strrchr(buffer, '/')) != NULL) { /* Remove "build" dir */
        *s = '\0';
        len = s - buffer;
    }

    if (subdir_len) {
        if (*subdir != '/' && buffer[len - 1] != '/') {
            if (len + 1 >= buffer_size) {
                fprintf(stderr, "testUtilDataPath: subdir len (%d) + 1 >= buffer_size (%d)\n", len, buffer_size);
                return 0;
            }
            buffer[len++] = '/';
            buffer[len] = '\0';
        }
        if (len + subdir_len >= buffer_size) {
            fprintf(stderr, "testUtilDataPath: len (%d) + subdir_len (%d) >= buffer_size (%d)\n",
                    len, subdir_len, buffer_size);
            return 0;
        }
        strcpy(buffer + len, subdir);
        len += subdir_len;
    }

    if (filename_len) {
        if (*filename != '/' && buffer[len - 1] != '/') {
            if (len + 1 >= buffer_size) {
                fprintf(stderr, "testUtilDataPath: filename len (%d) + 1 >= buffer_size (%d)\n", len, buffer_size);
                return 0;
            }
            buffer[len++] = '/';
            buffer[len] = '\0';
        }
        if (len + filename_len >= buffer_size) {
            fprintf(stderr, "testUtilDataPath: len (%d) + filename_len (%d) >= buffer_size (%d)\n",
                    len, filename_len, buffer_size);
            return 0;
        }
        strcpy(buffer + len, filename);
    }

    return 1;
}

/* Open a file (Windows compatibility) */
FILE *testUtilOpen(const char *filename, const char *mode) {
#ifdef _WIN32
    FILE *fp = out_win_fopen(filename, mode);
#else
    FILE *fp = fopen(filename, mode);
#endif
    return fp;
}

/* Does file exist? */
int testUtilExists(const char *filename) {
    FILE *fp = testUtilOpen(filename, "r");
    if (fp == NULL) {
        return 0;
    }
    fclose(fp);
    return 1;
}

/* Remove a file (Windows compatibility). Returns 0 if successful, non-zero if not */
int testUtilRemove(const char *filename) {
#ifdef _WIN32
    wchar_t *filenameW;
    utf8_to_wide(filename, filenameW);
    return DeleteFileW(filenameW) == 0; /* Non-zero on success */
#else
    return remove(filename);
#endif
}

/* Does directory exist? (Windows compatibility) */
int testUtilDirExists(const char *dirname) {
#ifdef _WIN32
    DWORD dwAttrib;
    wchar_t *dirnameW;
    utf8_to_wide(dirname, dirnameW);
    dwAttrib = GetFileAttributesW(dirnameW);
    return dwAttrib != (DWORD) -1 && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#else
    return testUtilExists(dirname);
#endif
}

/* Make a directory (Windows compatibility). Returns 0 if successful, non-zero if not */
int testUtilMkDir(const char *dirname) {
#ifdef _WIN32
    wchar_t *dirnameW;
    utf8_to_wide(dirname, dirnameW);
    return CreateDirectoryW(dirnameW, NULL) == 0;
#else
    return mkdir(dirname, S_IRWXU);
#endif
}

/* Remove a directory (Windows compatibility). Returns 0 if successful, non-zero if not */
int testUtilRmDir(const char *dirname) {
#ifdef _WIN32
    wchar_t *dirnameW;
    utf8_to_wide(dirname, dirnameW);
    return RemoveDirectoryW(dirnameW) == 0;
#else
    return rmdir(dirname);
#endif
}

/* Rename a file (Windows compatibility) */
int testUtilRename(const char *oldpath, const char *newpath) {
#ifdef _WIN32
    wchar_t *oldpathW, *newpathW;
    int ret = testUtilRemove(newpath);
    if (ret != 0) return ret;
    utf8_to_wide(oldpath, oldpathW);
    utf8_to_wide(newpath, newpathW);
    return _wrename(oldpathW, newpathW);
#else
    return rename(oldpath, newpath);
#endif
}

/* Create read-only file */
int testUtilCreateROFile(const char *filename) {
#ifdef _WIN32
    wchar_t *filenameW;
#endif
    FILE *fp = testUtilOpen(filename, "w+");
    if (fp == NULL) {
        return 0;
    }
    if (fclose(fp) != 0) {
        return 0;
    }
#ifdef _WIN32
    utf8_to_wide(filename, filenameW);
    if (SetFileAttributesW(filenameW, GetFileAttributesW(filenameW) | FILE_ATTRIBUTE_READONLY) == 0) {
        return 0;
    }
#else
    if (chmod(filename, S_IRUSR | S_IRGRP | S_IROTH) != 0) {
        return 0;
    }
#endif
    return 1;
}

/* Remove read-only file (Windows compatibility) */
int testUtilRmROFile(const char *filename) {
#ifdef _WIN32
    wchar_t *filenameW;
    utf8_to_wide(filename, filenameW);
    if (SetFileAttributesW(filenameW, GetFileAttributesW(filenameW) & ~FILE_ATTRIBUTE_READONLY) == 0) {
        return -1;
    }
#endif
    return testUtilRemove(filename);
}

/* Read file into buffer */
int testUtilReadFile(const char *filename, unsigned char *buffer, int buffer_size, int *p_size) {
    long fileLen;
    size_t n;
    size_t nRead = 0;
    FILE *fp = testUtilOpen(filename, "rb");
    if (!fp) {
        return 1;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        (void) fclose(fp);
        return 2;
    }
    fileLen = ftell(fp);
    if (fileLen <= 0 || fileLen == LONG_MAX) {
        (void) fclose(fp);
        return 3;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        (void) fclose(fp);
        return 4;
    }
    if (fileLen > (long) buffer_size) {
        (void) fclose(fp);
        return 5;
    }
    do {
        n = fread(buffer + nRead, 1, fileLen - nRead, fp);
        if (ferror(fp)) {
            (void) fclose(fp);
            return 6;
        }
        nRead += n;
    } while (!feof(fp) && (0 < n) && ((long) nRead < fileLen));

    if (fclose(fp) != 0) {
        return 7;
    }

    *p_size = (int) nRead;

    return 0;
}

/* Write file from buffer */
int testUtilWriteFile(const char *filename, const unsigned char *buffer, const int buffer_size, const char *mode) {
    FILE *fp = testUtilOpen(filename, mode);
    if (!fp) {
        return 1;
    }
    if (fwrite(buffer, 1, buffer_size, fp) == 0) {
        (void) fclose(fp);
        return 2;
    }
    if (fclose(fp) != 0) {
        return 3;
    }

    return 0;
}

/* Compare 2 PNG files */
int testUtilCmpPngs(const char *png1, const char *png2) {
    int ret = -1;
#ifdef ZINT_NO_PNG
    (void)png1; (void)png2;
#else
    FILE *fp1;
    FILE *fp2;
    png_structp png_ptr1, png_ptr2;
    png_infop info_ptr1, info_ptr2;
    int width1, height1, width2, height2;
    png_byte color_type1, color_type2;
    png_byte bit_depth1, bit_depth2;
    png_bytep row1, row2;
    size_t rowbytes1, rowbytes2;
    int r;

    fp1 = testUtilOpen(png1, "rb");
    if (!fp1) {
        return 2;
    }
    fp2 = testUtilOpen(png2, "rb");
    if (!fp2) {
        fclose(fp1);
        return 3;
    }

    png_ptr1 = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp) NULL, NULL, NULL);
    if (!png_ptr1) {
        fclose(fp1);
        fclose(fp2);
        return 4;
    }
    info_ptr1 = png_create_info_struct(png_ptr1);
    if (!info_ptr1) {
        png_destroy_read_struct(&png_ptr1, (png_infopp) NULL, (png_infopp) NULL);
        fclose(fp1);
        fclose(fp2);
        return 5;
    }

    png_ptr2 = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp) NULL, NULL, NULL);
    if (!png_ptr2) {
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp) NULL);
        fclose(fp1);
        fclose(fp2);
        return 6;
    }
    info_ptr2 = png_create_info_struct(png_ptr2);
    if (!info_ptr2) {
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp) NULL);
        png_destroy_read_struct(&png_ptr2, (png_infopp) NULL, (png_infopp) NULL);
        fclose(fp1);
        fclose(fp2);
        return 7;
    }

    row1 = row2 = NULL; /* Init here to avoid potential "clobbered" warning */
    if (setjmp(png_jmpbuf(png_ptr1))) {
        if (row1) {
            free(row1);
        }
        if (row2) {
            free(row2);
        }
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp) NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp) NULL);
        fclose(fp1);
        fclose(fp2);
        return 8;
    }
    if (setjmp(png_jmpbuf(png_ptr2))) {
        if (row1) {
            free(row1);
        }
        if (row2) {
            free(row2);
        }
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp) NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp) NULL);
        fclose(fp1);
        fclose(fp2);
        return 9;
    }

    png_init_io(png_ptr1, fp1);
    png_init_io(png_ptr2, fp2);

    png_read_info(png_ptr1, info_ptr1);
    png_read_info(png_ptr2, info_ptr2);

    width1 = png_get_image_width(png_ptr1, info_ptr1);
    height1 = png_get_image_height(png_ptr1, info_ptr1);
    width2 = png_get_image_width(png_ptr2, info_ptr2);
    height2 = png_get_image_height(png_ptr2, info_ptr2);

    if (width1 != width2 || height1 != height2) {
        printf("width1 %d, width2 %d, height1 %d, height2 %d\n", width1, width2, height1, height2);
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp) NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp) NULL);
        fclose(fp1);
        fclose(fp2);
        return 10;
    }

    color_type1 = png_get_color_type(png_ptr1, info_ptr1);
    bit_depth1 = png_get_bit_depth(png_ptr1, info_ptr1);
    if (bit_depth1 == 16) {
#if defined(PNG_LIBPNG_VER) && PNG_LIBPNG_VER >= 10504
        png_set_scale_16(png_ptr1);
#else
        png_set_strip_16(png_ptr1);
#endif
    }
    if (color_type1 == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr1);
    }
    if (color_type1 == PNG_COLOR_TYPE_GRAY && bit_depth1 < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr1);
    }
    if (png_get_valid(png_ptr1, info_ptr1, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr1);
    }
    if (color_type1 == PNG_COLOR_TYPE_RGB || color_type1 == PNG_COLOR_TYPE_GRAY
            || color_type1 == PNG_COLOR_TYPE_PALETTE) {
        png_set_filler(png_ptr1, 0xFF, PNG_FILLER_AFTER);
    }
    if (color_type1 == PNG_COLOR_TYPE_GRAY || color_type1 == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr1);
    }

    color_type2 = png_get_color_type(png_ptr2, info_ptr2);
    bit_depth2 = png_get_bit_depth(png_ptr2, info_ptr2);
    if (bit_depth2 == 16) {
#if defined(PNG_LIBPNG_VER) && PNG_LIBPNG_VER >= 10504
        png_set_scale_16(png_ptr2);
#else
        png_set_strip_16(png_ptr2);
#endif
    }
    if (color_type2 == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr2);
    }
    if (color_type2 == PNG_COLOR_TYPE_GRAY && bit_depth2 < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr2);
    }
    if (png_get_valid(png_ptr2, info_ptr2, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr2);
    }
    if (color_type2 == PNG_COLOR_TYPE_RGB || color_type2 == PNG_COLOR_TYPE_GRAY
            || color_type2 == PNG_COLOR_TYPE_PALETTE) {
        png_set_filler(png_ptr2, 0xFF, PNG_FILLER_AFTER);
    }
    if (color_type2 == PNG_COLOR_TYPE_GRAY || color_type2 == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr2);
    }

    png_read_update_info(png_ptr1, info_ptr1);
    png_read_update_info(png_ptr2, info_ptr2);

    rowbytes1 = png_get_rowbytes(png_ptr1, info_ptr1);
    rowbytes2 = png_get_rowbytes(png_ptr2, info_ptr2);
    if (rowbytes1 != rowbytes2) {
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 11;
    }

    row1 = malloc(rowbytes1);
    if (!row1) {
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 12;
    }
    row2 = malloc(rowbytes2);
    if (!row2) {
        free(row1);
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 13;
    }

    for (r = 0; r < height1; r++) {
        png_read_row(png_ptr1, row1, NULL);
        png_read_row(png_ptr2, row2, NULL);
        if (memcmp(row1, row2, rowbytes1) != 0) {
            break;
        }
    }
    ret = r == height1 ? 0 : 20;

    free(row1);
    free(row2);
    png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
    png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp)NULL);
    fclose(fp1);
    fclose(fp2);
#endif
    return ret;
}

/* Compare 2 TXT files */
int testUtilCmpTxts(const char *txt1, const char *txt2) {
    int ret = -1;
    FILE *fp1;
    FILE *fp2;
    char buf1[1024];
    char buf2[1024];
    size_t len1 = 0, len2 = 0;

    fp1 = testUtilOpen(txt1, "r");
    if (!fp1) {
        return 2;
    }
    fp2 = testUtilOpen(txt2, "r");
    if (!fp2) {
        fclose(fp1);
        return 3;
    }

    while (1) {
        if (fgets(buf1, sizeof(buf1), fp1) == NULL) {
            if (fgets(buf2, sizeof(buf2), fp2) != NULL) {
                ret = 4;
                break;
            }
            break;
        }
        if (fgets(buf2, sizeof(buf2), fp2) == NULL) {
            ret = 5;
            break;
        }
        len1 = strlen(buf1);
        len2 = strlen(buf2);
        if (len1 != len2) {
            ret = 6;
            break;
        }
        if (strcmp(buf1, buf2) != 0) {
            ret = 7;
            break;
        }
    }
    if (ret == -1) {
        ret = feof(fp1) && feof(fp2) ? 0 : 20;
    }
    fclose(fp1);
    fclose(fp2);

    return ret;
}

/* Compare 2 binary files */
int testUtilCmpBins(const char *bin1, const char *bin2) {
    int ret = -1;
    FILE *fp1;
    FILE *fp2;
    char buf1[1024];
    char buf2[1024];
    size_t len1 = 0, len2 = 0;

    fp1 = testUtilOpen(bin1, "rb");
    if (!fp1) {
        return 2;
    }
    fp2 = testUtilOpen(bin2, "rb");
    if (!fp2) {
        fclose(fp1);
        return 3;
    }

    do {
        len1 = fread(buf1, 1, sizeof(buf1), fp1);
        len2 = fread(buf2, 1, sizeof(buf2), fp2);
        if (ferror(fp1) || ferror(fp2)) {
            ret = 4;
            break;
        }
        if (len1 != len2) {
            ret = 6;
            break;
        }
        if (memcmp(buf1, buf2, len1) != 0) {
            ret = 7;
            break;
        }
    } while (!feof(fp1) && !feof(fp2));

    if (ret == -1) {
        ret = feof(fp1) && feof(fp2) ? 0 : 20;
    }
    fclose(fp1);
    fclose(fp2);

    return ret;
}

/* Compare 2 SVG files */
int testUtilCmpSvgs(const char *svg1, const char *svg2) {
    return testUtilCmpTxts(svg1, svg2);
}

/* Compare 2 EPS files */
int testUtilCmpEpss(const char *eps1, const char *eps2) {
    int ret = -1;
    FILE *fp1;
    FILE *fp2;
    char buf1[1024];
    char buf2[1024];
    size_t len1 = 0, len2 = 0;
    static char first_line[] = "%!PS-Adobe-3.0 EPSF-3.0\n";
    static char second_line_start[] = "%%Creator: Zint ";

    fp1 = testUtilOpen(eps1, "r");
    if (!fp1) {
        return 2;
    }
    fp2 = testUtilOpen(eps2, "r");
    if (!fp2) {
        fclose(fp1);
        return 3;
    }

    /* Preprocess the 1st 2 lines to avoid comparing changeable Zint version in 2nd line */
    if (fgets(buf1, sizeof(buf1), fp1) == NULL || strcmp(buf1, first_line) != 0
            || fgets(buf2, sizeof(buf2), fp2) == NULL || strcmp(buf2, first_line) != 0) {
        ret = 10;
    } else if (fgets(buf1, sizeof(buf1), fp1) == NULL
            || strncmp(buf1, second_line_start, sizeof(second_line_start) - 1) != 0
            || fgets(buf2, sizeof(buf2), fp2) == NULL
            || strncmp(buf2, second_line_start, sizeof(second_line_start) - 1) != 0) {
        ret = 11;
    }

    if (ret == -1) {
        while (1) {
            if (fgets(buf1, sizeof(buf1), fp1) == NULL) {
                if (fgets(buf2, sizeof(buf2), fp2) != NULL) {
                    ret = 4;
                    break;
                }
                break;
            }
            if (fgets(buf2, sizeof(buf2), fp2) == NULL) {
                ret = 5;
                break;
            }
            len1 = strlen(buf1);
            len2 = strlen(buf2);
            if (len1 != len2) {
                ret = 6;
                break;
            }
            if (strcmp(buf1, buf2) != 0) {
                ret = 7;
                break;
            }
        }
        if (ret == -1) {
            ret = feof(fp1) && feof(fp2) ? 0 : 20;
        }
    }
    fclose(fp1);
    fclose(fp2);

    return ret;
}

#ifdef _WIN32
#define DEV_NULL "> NUL"
#define DEV_NULL_STDERR "> NUL 2>&1"
#else
#define DEV_NULL "> /dev/null"
#define DEV_NULL_STDERR "> /dev/null 2>&1"
#endif

/* Whether ImageMagick's identify utility available on system */
const char *testUtilHaveIdentify(void) {
    static const char *progs[2] = { "magick identify", "identify" };
    if (system("magick -version " DEV_NULL_STDERR) == 0) {
        return progs[0];
    }
    if (system("identify -version " DEV_NULL_STDERR) == 0) {
        return progs[1];
    }
    return NULL;
}

/* Check raster files */
int testUtilVerifyIdentify(const char *const prog, const char *filename, int debug) {
    char cmd[512 + 128];

    if (strlen(filename) > 512) {
        return -1;
    }
    /* Verbose option does a more thorough check */
    if (debug & ZINT_DEBUG_TEST_PRINT) {
        /* Verbose very noisy though so for quick check just return default output */
        if (debug & ZINT_DEBUG_TEST_LESS_NOISY) {
            sprintf(cmd, "%s %s", prog, filename);
        } else {
            sprintf(cmd, "%s -verbose %s", prog, filename);
        }
    } else {
        sprintf(cmd, "%s -verbose %s " DEV_NULL, prog, filename);
    }

    return system(cmd);
}

/* Whether Libre Office available on system */
int testUtilHaveLibreOffice(void) {
    return system("libreoffice --version " DEV_NULL) == 0;
}

/* Check SVG files, very hacky to evoke. Will fail if Libre package that is not LibreOffice Draw is running */
int testUtilVerifyLibreOffice(const char *filename, int debug) {
    char cmd[512 + 128];
    char svg[512];
    char *slash, *dot;
    char buf[16384];
    char *b = buf, *be = buf + sizeof(buf) - 1;
    FILE *fp;
    int len;

    /* Hack to read SVG produced by LibreOffice and search for 'x="-32767"' which indicates it didn't load barcode
       file */
    if (strlen(filename) > 512) {
        return -1;
    }
    slash = strrchr(filename, '/');
    if (slash) {
        strcpy(svg, slash + 1);
    } else {
        strcpy(svg, filename);
    }
    dot = strrchr(svg, '.');
    if (dot) {
        strcpy(dot, ".svg");
    } else {
        strcat(svg, ".svg");
    }
    if (strcmp(svg, filename) == 0) {
        fprintf(stderr, "testUtilVerifyLibreOffice: input '%s' same as svg '%s'\n", filename, svg);
        return -1;
    }

    sprintf(cmd, "libreoffice --convert-to svg %s " DEV_NULL_STDERR, filename);
    if (debug & ZINT_DEBUG_TEST_PRINT) {
        printf("%s\n", cmd);
    }
    if (system(cmd) != 0) {
        fprintf(stderr, "testUtilVerifyLibreOffice: failed to run '%s'\n", cmd);
        return -1;
    }

    fp = testUtilOpen(svg, "r");
    if (!fp) {
        fprintf(stderr, "testUtilVerifyLibreOffice: failed to open '%s' (%s)\n", svg, cmd);
        return -1;
    }
    while (!feof(fp) && b < be) {
        if (fgets(b, be - b, fp) == NULL) {
            fprintf(stderr, "testUtilVerifyLibreOffice: failed to get line from '%s' (%s)\n", svg, cmd);
            fclose(fp);
            return -1;
        }
        len = (int) strlen(b);
        if (len == 0) {
            break;
        }
        b += len;
    }
    *b = '\0';
    fclose(fp);

    if (strlen(buf) < 1024) {
        fprintf(stderr, "testUtilVerifyLibreOffice: failed to get much input from '%s' (%s)\n", svg, cmd);
        return -1;
    }
    if (strstr(buf, "x=\"-32767\"") != NULL) {
        return -1;
    }
    testUtilRemove(svg);

    return 0;
}

#ifdef _WIN32
#define GS_FILENAME "gswin64c"
#else
#define GS_FILENAME "gs"
#endif

/* Whether Ghostscript available on system */
int testUtilHaveGhostscript(void) {
    return system(GS_FILENAME " -v " DEV_NULL) == 0;
}

/* Check EPS files */
int testUtilVerifyGhostscript(const char *filename, int debug) {
    char cmd[512 + 128];

    if (strlen(filename) > 512) {
        return -1;
    }
    if (debug & ZINT_DEBUG_TEST_PRINT) {
        /* Prints nothing of interest with or without -q unless bad */
        sprintf(cmd, GS_FILENAME " -dNOPAUSE -dBATCH -dNODISPLAY -q %s", filename);
        printf("%s\n", cmd);
    } else {
        sprintf(cmd, GS_FILENAME " -dNOPAUSE -dBATCH -dNODISPLAY -q %s", filename);
    }

    return system(cmd);
}

/* Whether vnu validator available on system. v.Nu https://github.com/validator/validator
   Needs "$INSTALL_DIR/vnu-runtime-image/bin" in PATH */
int testUtilHaveVnu(void) {
    return system("vnu --version " DEV_NULL_STDERR) == 0;
}

/* Check SVG files, very full but very slow */
int testUtilVerifyVnu(const char *filename, int debug) {
    char buf[512 + 128];

    if (strlen(filename) > 512) {
        return -1;
    }
    if (debug & ZINT_DEBUG_TEST_PRINT) {
        sprintf(buf, "vnu --svg --verbose %s", filename);
        printf("%s\n", buf);
    } else {
        sprintf(buf, "vnu --svg %s", filename);
    }

    return system(buf);
}

/* Whether tiffinfo available on system. Requires libtiff 4.2.0 http://www.libtiff.org to be installed */
int testUtilHaveTiffInfo(void) {
    return system("tiffinfo -h " DEV_NULL) == 0;
}

/* Check TIF files */
int testUtilVerifyTiffInfo(const char *filename, int debug) {
    char cmd[512 + 128];

    if (strlen(filename) > 512) {
        return -1;
    }
    if (debug & ZINT_DEBUG_TEST_PRINT) {
        sprintf(cmd, "tiffinfo -D %s", filename);
    } else {
        sprintf(cmd, "tiffinfo -D %s " DEV_NULL_STDERR, filename);
    }

    return system(cmd);
}

/* Map Zint symbology to BWIPP routine */
static const char *testUtilBwippName(int index, const struct zint_symbol *symbol, int option_1, int option_2,
                    int option_3, int debug, int *linear_row_height, int *gs1_cvt) {
    struct item {
        const char *name;
        int define;
        int val;
        int can_option_1;
        int can_option_2;
        int can_option_3;
        int linear_row_height;
        int gs1_cvt;
    };
    static const struct item data[] = {
        { "", -1, 0, 0, 0, 0, 0, 0, },
        { "code11", BARCODE_CODE11, 1, 0, 1, 0, 0, 0, },
        { "matrix2of5", BARCODE_C25STANDARD, 2, 0, 1, 0, 0, 0, },
        { "interleaved2of5", BARCODE_C25INTER, 3, 0, 1, 0, 0, 0, },
        { "iata2of5", BARCODE_C25IATA, 4, 0, 1, 0, 0, 0, },
        { "", -1, 5, 0, 0, 0, 0, 0, },
        { "datalogic2of5", BARCODE_C25LOGIC, 6, 0, 1, 0, 0, 0, },
        { "industrial2of5", BARCODE_C25IND, 7, 0, 1, 0, 0, 0, },
        { "code39", BARCODE_CODE39, 8, 0, 1, 0, 0, 0, },
        { "code39ext", BARCODE_EXCODE39, 9, 0, 1, 0, 0, 0, },
        { "", -1, 10, 0, 0, 0, 0, 0, },
        { "", -1, 11, 0, 0, 0, 0, 0, },
        { "", -1, 12, 0, 0, 0, 0, 0, },
        { "ean13", BARCODE_EANX, 13, 0, 1, 0, 0, 1 /*gs1_cvt*/, },
        { "ean13", BARCODE_EANX_CHK, 14, 0, 1, 0, 0, 1, },
        { "", -1, 15, 0, 0, 0, 0, 0, },
        { "gs1-128", BARCODE_GS1_128, 16, 0, 0, 0, 0, 1 /*gs1_cvt*/, },
        { "", -1, 17, 0, 0, 0, 0, 0, },
        { "rationalizedCodabar", BARCODE_CODABAR, 18, 0, 1, 0, 0, 0, },
        { "", -1, 19, 0, 0, 0, 0, 0, },
        { "code128", BARCODE_CODE128, 20, 0, 0, 0, 0, 0, },
        { "leitcode", BARCODE_DPLEIT, 21, 0, 0, 0, 0, 0, },
        { "identcode", BARCODE_DPIDENT, 22, 0, 0, 0, 0, 0, },
        { "code16k", BARCODE_CODE16K, 23, 1, 0, 0, 8 /*linear_row_height*/, 0, },
        { "code49", BARCODE_CODE49, 24, 1, 0, 0, 8 /*linear_row_height*/, 0, },
        { "code93ext", BARCODE_CODE93, 25, 0, 0, 0, 0, 0, },
        { "", -1, 26, 0, 0, 0, 0, 0, },
        { "", -1, 27, 0, 0, 0, 0, 0, },
        { "flattermarken", BARCODE_FLAT, 28, 0, 0, 0, 0, 0, },
        { "databaromni", BARCODE_DBAR_OMN, 29, 0, 0, 0, 0, 1 /*gs1_cvt*/, },
        { "databarlimited", BARCODE_DBAR_LTD, 30, 0, 0, 0, 0, 1, },
        { "databarexpanded", BARCODE_DBAR_EXP, 31, 0, 1, 0, 1 /*linear_row_height*/, 1, },
        { "telepen", BARCODE_TELEPEN, 32, 0, 0, 0, 0, 0, },
        { "", -1, 33, 0, 0, 0, 0, 0, },
        { "upca", BARCODE_UPCA, 34, 0, 1, 0, 0, 1 /*gs1_cvt*/, },
        { "upca", BARCODE_UPCA_CHK, 35, 0, 1, 0, 0, 1, },
        { "", -1, 36, 0, 0, 0, 0, 0, },
        { "upce", BARCODE_UPCE, 37, 0, 1, 0, 0, 1 /*gs1_cvt*/, },
        { "upce", BARCODE_UPCE_CHK, 38, 0, 1, 0, 0, 1, },
        { "", -1, 39, 0, 0, 0, 0, 0, },
        { "postnet", BARCODE_POSTNET, 40, 0, 0, 0, 0, 0, },
        { "", -1, 41, 0, 0, 0, 0, 0, },
        { "", -1, 42, 0, 0, 0, 0, 0, },
        { "", -1, 43, 0, 0, 0, 0, 0, },
        { "", -1, 44, 0, 0, 0, 0, 0, },
        { "", -1, 45, 0, 0, 0, 0, 0, },
        { "", -1, 46, 0, 0, 0, 0, 0, },
        { "msi", BARCODE_MSI_PLESSEY, 47, 0, 1, 0, 0, 0, },
        { "", -1, 48, 0, 0, 0, 0, 0, },
        { "symbol", BARCODE_FIM, 49, 0, 0, 0, 0, 0, },
        { "code39", BARCODE_LOGMARS, 50, 0, 1, 0, 0, 0, },
        { "pharmacode", BARCODE_PHARMA, 51, 0, 0, 0, 1 /*linear_row_height*/, 0, },
        { "pzn", BARCODE_PZN, 52, 0, 1, 0, 0, 0, },
        { "pharmacode2", BARCODE_PHARMA_TWO, 53, 0, 0, 0, 0, 0, },
        { "", BARCODE_CEPNET, 54, 0, 0, 0, 0, 0, },
        { "pdf417", BARCODE_PDF417, 55, 1, 1, 1, 0, 0, },
        { "pdf417compact", BARCODE_PDF417COMP, 56, 1, 1, 1, 0, 0, },
        { "maxicode", BARCODE_MAXICODE, 57, 1, 1, 0, 0, 0, },
        { "qrcode", BARCODE_QRCODE, 58, 1, 1, 1, 0, 0, },
        { "", -1, 59, 0, 0, 0, 0, 0, },
        { "code128", BARCODE_CODE128AB, 60, 0, 0, 0, 0, 0, },
        { "", -1, 61, 0, 0, 0, 0, 0, },
        { "", -1, 62, 0, 0, 0, 0, 0, },
        { "auspost", BARCODE_AUSPOST, 63, 0, 0, 0, 0, 0, },
        { "", -1, 64, 0, 0, 0, 0, 0, },
        { "", -1, 65, 0, 0, 0, 0, 0, },
        { "", BARCODE_AUSREPLY, 66, 0, 0, 0, 0, 0, },
        { "", BARCODE_AUSROUTE, 67, 0, 0, 0, 0, 0, },
        { "", BARCODE_AUSREDIRECT, 68, 0, 0, 0, 0, 0, },
        { "isbn", BARCODE_ISBNX, 69, 0, 1, 0, 0, 1 /*gs1_cvt*/, },
        { "royalmail", BARCODE_RM4SCC, 70, 0, 0, 0, 0, 0, },
        { "datamatrix", BARCODE_DATAMATRIX, 71, 0, 1, 1, 1, 0, },
        { "ean14", BARCODE_EAN14, 72, 0, 0, 0, 0, 1 /*gs1_cvt*/, },
        { "code39", BARCODE_VIN, 73, 0, 0, 0, 0, 0, },
        { "codablockf", BARCODE_CODABLOCKF, 74, 1, 1, 0, 10 /*linear_row_height*/, 0, },
        { "sscc18", BARCODE_NVE18, 75, 0, 0, 0, 0, 1 /*gs1_cvt*/, },
        { "japanpost", BARCODE_JAPANPOST, 76, 0, 0, 0, 0, 0, },
        { "", BARCODE_KOREAPOST, 77, 0, 0, 0, 0, 0, },
        { "", -1, 78, 0, 0, 0, 0, 0, },
        { "databarstacked", BARCODE_DBAR_STK, 79, 0, 0, 0, 0, 1 /*gs1_cvt*/, },
        { "databarstackedomni", BARCODE_DBAR_OMNSTK, 80, 0, 0, 0, 33 /*linear_row_height*/, 1, },
        { "databarexpandedstacked", BARCODE_DBAR_EXPSTK, 81, 0, 1, 0, 34 /*linear_row_height*/, 1, },
        { "planet", BARCODE_PLANET, 82, 0, 0, 0, 0, 0, },
        { "", -1, 83, 0, 0, 0, 0, 0, },
        { "micropdf417", BARCODE_MICROPDF417, 84, 0, 1, 0, 0, 0, },
        { "onecode", BARCODE_USPS_IMAIL, 85, 0, 0, 0, 0, 0, },
        { "plessey", BARCODE_PLESSEY, 86, 0, 0, 0, 0, 0, },
        { "telepennumeric", BARCODE_TELEPEN_NUM, 87, 0, 0, 0, 0, 0, },
        { "", -1, 88, 0, 0, 0, 0, 0, },
        { "itf14", BARCODE_ITF14, 89, 0, 0, 0, 0, 0, },
        { "kix", BARCODE_KIX, 90, 0, 0, 0, 0, 0, },
        { "", -1, 91, 0, 0, 0, 0, 0, },
        { "azteccode", BARCODE_AZTEC, 92, 1, 1, 0, 0, 0, },
        { "daft", BARCODE_DAFT, 93, 0, 0, 0, 0, 0, },
        { "", -1, 94, 0, 0, 0, 0, 0, },
        { "", -1, 95, 0, 0, 0, 0, 0, },
        { "code128", BARCODE_DPD, 96, 0, 1, 0, 0, 0, },
        { "microqrcode", BARCODE_MICROQR, 97, 1, 1, 1, 0, 0, },
        { "hibccode128", BARCODE_HIBC_128, 98, 0, 0, 0, 0, 0, },
        { "hibccode39", BARCODE_HIBC_39, 99, 0, 0, 0, 0, 0, },
        { "", -1, 100, 0, 0, 0, 0, 0, },
        { "", -1, 101, 0, 0, 0, 0, 0, },
        { "hibcdatamatrix", BARCODE_HIBC_DM, 102, 0, 1, 1, 0, 0, },
        { "", -1, 103, 0, 0, 0, 0, 0, },
        { "hibcqrcode", BARCODE_HIBC_QR, 104, 1, 1, 1, 0, 0, },
        { "", -1, 105, 0, 0, 0, 0, 0, },
        { "hibcpdf417", BARCODE_HIBC_PDF, 106, 1, 1, 1, 0, 0, },
        { "", -1, 107, 0, 0, 0, 0, 0, },
        { "hibcmicropdf417", BARCODE_HIBC_MICPDF, 108, 0, 1, 0, 0, 0, },
        { "", -1, 109, 0, 0, 0, 0, 0, },
        { "hibccodablockf", BARCODE_HIBC_BLOCKF, 110, 1, 1, 0, 10 /*linear_row_height*/, 0, },
        { "", -1, 111, 0, 0, 0, 0, 0, },
        { "hibcazteccode", BARCODE_HIBC_AZTEC, 112, 1, 0, 1, 0, 0, },
        { "", -1, 113, 0, 0, 0, 0, 0, },
        { "", -1, 114, 0, 0, 0, 0, 0, },
        { "dotcode", BARCODE_DOTCODE, 115, 0, 1, 1, 0, 0, },
        { "hanxin", BARCODE_HANXIN, 116, 0, 0, 0, 0, 0, },
        { "", -1, 117, 0, 0, 0, 0, 0, },
        { "", -1, 118, 0, 0, 0, 0, 0, },
        { "mailmark", BARCODE_MAILMARK_2D, 119, 0, 1, 0, 0, 0, },
        { "code128", BARCODE_UPU_S10, 120, 0, 0, 0, 0, 0, },
        { "", BARCODE_MAILMARK_4S, 121, 0, 0, 0, 0, 0, }, /* Note BWIPP mailmark is BARCODE_MAILMARK_2D above */
        { "", -1, 122, 0, 0, 0, 0, 0, },
        { "", -1, 123, 0, 0, 0, 0, 0, },
        { "", -1, 124, 0, 0, 0, 0, 0, },
        { "", -1, 125, 0, 0, 0, 0, 0, },
        { "", -1, 126, 0, 0, 0, 0, 0, },
        { "", BARCODE_DXFILMEDGE, 127, 0, 0, 0, 0, 0, },
        { "aztecrune", BARCODE_AZRUNE, 128, 0, 0, 0, 0, 0, },
        { "code32", BARCODE_CODE32, 129, 0, 0, 0, 0, 0, },
        { "ean13composite", BARCODE_EANX_CC, 130, 1, 1, 0, 72 /*linear_row_height*/, 1 /*gs1_cvt*/, },
        { "gs1-128composite", BARCODE_GS1_128_CC, 131, 1, 0, 0, 36, 1, },
        { "databaromnicomposite", BARCODE_DBAR_OMN_CC, 132, 1, 0, 0, 33, 1, },
        { "databarlimitedcomposite", BARCODE_DBAR_LTD_CC, 133, 1, 0, 0, 10 /*linear_row_height*/, 1, },
        { "databarexpandedcomposite", BARCODE_DBAR_EXP_CC, 134, 1, 1, 0, 34 /*linear_row_height*/, 1, },
        { "upcacomposite", BARCODE_UPCA_CC, 135, 1, 1, 0, 72, 1, },
        { "upcecomposite", BARCODE_UPCE_CC, 136, 1, 1, 0, 72, 1, },
        { "databarstackedcomposite", BARCODE_DBAR_STK_CC, 137, 1, 0, 0, 0, 1, },
        { "databarstackedomnicomposite", BARCODE_DBAR_OMNSTK_CC, 138, 1, 0, 0, 33 /*linear_row_height*/, 1, },
        { "databarexpandedstackedcomposite", BARCODE_DBAR_EXPSTK_CC, 139, 1, 1, 0, 34 /*linear_row_height*/, 1, },
        { "channelcode", BARCODE_CHANNEL, 140, 0, 0, 0, 0, 0, },
        { "codeone", BARCODE_CODEONE, 141, 0, 1, 0, 0, 0, },
        { "", BARCODE_GRIDMATRIX, 142, 0, 0, 0, 0, 0, },
        { "qrcode", BARCODE_UPNQR, 143, 0, 0, 1, 0, 0, },
        { "ultracode", BARCODE_ULTRA, 144, 1, 1, 0, 0, 0, },
        { "rectangularmicroqrcode", BARCODE_RMQR, 145, 1, 1, 1, 0, 0, },
        { "bc412", BARCODE_BC412, 146, 1, 1, 0, 0, 0, },
    };
    const int data_size = ARRAY_SIZE(data);

    const int symbology = symbol->symbology;
    const int gs1 = (symbol->input_mode & 0x07) == GS1_MODE;

    if (symbology < 0 || symbology >= data_size) {
        fprintf(stderr, "testUtilBwippName: unknown symbology (%d)\n", symbology);
        abort();
    }
    /* Self-check */
    if (data[symbology].val != symbology || (data[symbology].define != -1 && data[symbology].define != symbology)) {
        fprintf(stderr, "testUtilBwippName: data table out of sync (%d)\n", symbology);
        abort();
    }
    if (data[symbology].name[0] == '\0') {
        if (debug & ZINT_DEBUG_TEST_PRINT) {
            printf("i:%d %s no BWIPP mapping\n", index, testUtilBarcodeName(symbology));
        }
        return NULL;
    }
    if ((option_1 != -1 && !data[symbology].can_option_1) || (option_2 != -1 && !data[symbology].can_option_2)
             || (option_3 != -1 && !data[symbology].can_option_3)) {
        if (debug & ZINT_DEBUG_TEST_PRINT) {
            printf("i:%d %s not BWIPP compatible, options not supported, option_1 %d, option_2 %d, option_3 %d\n",
                    index, testUtilBarcodeName(symbology), option_1, option_2, option_3);
        }
        return NULL;
    }
    if (symbol->structapp.count && symbology != BARCODE_MAXICODE) {
        if (debug & ZINT_DEBUG_TEST_PRINT) {
            printf("i:%d %s not BWIPP compatible, Structured Append not supported\n",
                    index, testUtilBarcodeName(symbology));
        }
        return NULL;
    }

    if (symbology == BARCODE_CODE11) {
        if (option_2 != 1 && option_2 != 2) { /* 2 check digits (Zint default) not supported */
            if (debug & ZINT_DEBUG_TEST_PRINT) {
                printf("i:%d %s not BWIPP compatible, 2 check digits not supported, option_1 %d, option_2 %d\n",
                        index, testUtilBarcodeName(symbology), option_1, option_2);
            }
            return NULL;
        }
    } else if (symbology == BARCODE_CODABLOCKF || symbology == BARCODE_HIBC_BLOCKF) {
        if (option_1 == 1) { /* Single row i.e. CODE128 not supported */
            if (debug & ZINT_DEBUG_TEST_PRINT) {
                printf("i:%d %s not BWIPP compatible, single row not supported, option_1 %d\n",
                        index, testUtilBarcodeName(symbology), option_1);
            }
            return NULL;
        }
    } else if (symbology == BARCODE_AZTEC) {
        if (option_1 > 0 && option_2 > 0) {
            if (debug & ZINT_DEBUG_TEST_PRINT) {
                printf("i:%d %s not BWIPP compatible, cannot specify both option_1 %d and option_2 %d\n",
                        index, testUtilBarcodeName(symbology), option_1, option_2);
            }
            return NULL;
        }
    } else if (symbology == BARCODE_RMQR) {
        if (option_2 < 1) {
            if (debug & ZINT_DEBUG_TEST_PRINT) {
                printf("i:%d %s not BWIPP compatible, version (option_2) must be specified\n",
                        index, testUtilBarcodeName(symbology));
            }
            return NULL;
        }
        if (option_2 > 32) {
            if (debug & ZINT_DEBUG_TEST_PRINT) {
                printf("i:%d %s not BWIPP compatible, auto width (option_2 > 32) not supported\n",
                        index, testUtilBarcodeName(symbology));
            }
            return NULL;
        }
    } else if (symbology == BARCODE_MAILMARK_2D) {
        if (option_2 < 1) {
            if (debug & ZINT_DEBUG_TEST_PRINT) {
                printf("i:%d %s not BWIPP compatible, version (option_2) must be specified\n",
                        index, testUtilBarcodeName(symbology));
            }
            return NULL;
        }
    }

    if (linear_row_height) {
        *linear_row_height = data[symbology].linear_row_height;
    }
    if (gs1_cvt) {
        *gs1_cvt = data[symbology].gs1_cvt;
    }
    if (gs1) {
        if (symbology == BARCODE_DATAMATRIX) {
            if (gs1_cvt) {
                *gs1_cvt = 1;
            }
            return "gs1datamatrix";
        } else if (symbology == BARCODE_AZTEC || symbology == BARCODE_CODE16K || symbology == BARCODE_ULTRA
                    || symbology == BARCODE_CODE49) {
            if (debug & ZINT_DEBUG_TEST_PRINT) {
                printf("i:%d %s not BWIPP compatible, GS1_MODE not supported\n",
                        index, testUtilBarcodeName(symbology));
            }
            return NULL;
        } else if (symbology == BARCODE_QRCODE) {
            if (gs1_cvt) {
                *gs1_cvt = 1;
            }
            return "gs1qrcode";
        } else if (symbology == BARCODE_DOTCODE) {
            if (gs1_cvt) {
                *gs1_cvt = 1;
            }
            return "gs1dotcode";
        }
    }

    return data[symbology].name;
}

/* Whether can use BWIPP to check a symbology with given options */
int testUtilCanBwipp(int index, const struct zint_symbol *symbol, int option_1, int option_2, int option_3,
            int debug) {
    return testUtilBwippName(index, symbol, option_1, option_2, option_3, debug, NULL, NULL) != NULL;
}

/* Convert Zint GS1 and add-on format to BWIPP's */
static void testUtilBwippCvtGS1Data(char *bwipp_data, const int upcean, const int parens_mode, int *addon_posn) {
    char *b;
    int pipe = 0;

    *addon_posn = 0;
    for (b = bwipp_data; *b; b++) {
        if (upcean && *b == '|') {
            pipe = 1;
        }
        if (!parens_mode && *b == '[') {
            *b = '(';
        } else if (!parens_mode && *b == ']') {
            *b = ')';
        } else if (*b == '+' && upcean && !pipe) {
            *b = ' ';
            *addon_posn = b - bwipp_data;
        }
    }
}

#define z_isxdigit(c) (z_isdigit(c) || ((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))
#define z_isodigit(c) ((c) <= '7' && (c) >= '0')

/* Convert data to Ghostscript format for passing to bwipp_dump.ps */
static char *testUtilBwippEscape(char *bwipp_data, int bwipp_data_size, const char *data, int length,
                int zint_escape_mode, int eci, int *parse, int *parsefnc) {
    const int init_parsefnc = *parsefnc == 1;
    char *b = bwipp_data;
    char *be = b + bwipp_data_size;
    unsigned char *d = (unsigned char *) data;
    unsigned char *de = (unsigned char *) data + length;

    if (eci && !init_parsefnc) {
        sprintf(bwipp_data, "^ECI%06d", eci);
        *parsefnc = 1;
        b = bwipp_data + 10;
    }

    while (b < be && d < de) {
        /* Have to escape double quote otherwise Ghostscript gives "Unterminated quote in @-file" for some reason */
        /* Escape single quote also to avoid having to do proper shell escaping TODO: proper shell escaping */
        if (*d < 0x20 || *d >= 0x7F || (*d == '^' && !init_parsefnc) || *d == '"' || *d == '\''
                || *d == '(' || (*d == '\\' && !zint_escape_mode)) {
            if (b + 4 >= be) {
                fprintf(stderr, "testUtilBwippEscape: double quote bwipp_data buffer full (%d)\n", bwipp_data_size);
                return NULL;
            }
            sprintf(b, "^%03u", *d++);
            b += 4;
            *parse = 1;
        } else if (zint_escape_mode && *d == '\\' && d + 1 < de) {
            int val;
            switch (*++d) {
                case '0': val = 0x00; /* Null */ break;
                case 'E': val = 0x04; /* End of Transmission */ break;
                case 'a': val = 0x07; /* Bell */ break;
                case 'b': val = 0x08; /* Backspace */ break;
                case 't': val = 0x09; /* Horizontal tab */ break;
                case 'n': val = 0x0a; /* Line feed */ break;
                case 'v': val = 0x0b; /* Vertical tab */ break;
                case 'f': val = 0x0c; /* Form feed */ break;
                case 'r': val = 0x0d; /* Carriage return */ break;
                case 'e': val = 0x1b; /* Escape */ break;
                case 'G': val = 0x1d; /* Group Separator */ break;
                case 'R': val = 0x1e; /* Record Separator */ break;
                case 'x':
                    val = d + 2 < de && z_isxdigit(d[1]) && z_isxdigit(d[2]) ? (ctoi(d[1]) << 4) | ctoi(d[2]) : -1;
                    if (val != -1) d+= 2;
                    break;
                case 'd':
                    val = d + 3 < de ? to_int(d + 1, 3) : -1;
                    if (val > 255) val = -1;
                    if (val != -1) d += 3;
                    break;
                case 'o':
                    val = d + 3 < de && z_isodigit(d[1]) && z_isodigit(d[2]) && z_isodigit(d[3])
                            ? (ctoi(d[1]) << 6) | (ctoi(d[2]) << 3) | ctoi(d[3]) : -1;
                    if (val > 255) val = -1;
                    if (val != -1) d += 3;
                    break;
                case '\\': val = '\\'; break;
                /*case 'u': val = 0; TODO: implement break; */
                /*case 'U': val = 0; TODO: implement break; */
                case '^': val = -1; break; /* Code 128 special escapes */
                default: fprintf(stderr, "testUtilBwippEscape: unknown escape %c\n", *d); return NULL; break;
            }
            if (val >= 0) {
                if (b + 4 >= be) {
                    fprintf(stderr, "testUtilBwippEscape: loop bwipp_data buffer full (%d)\n", bwipp_data_size);
                    return NULL;
                }
                sprintf(b, "^%03d", val);
                b += 4;
            } else {
                if (*d == '^' && d + 1 < de && ((*(d + 1) >= '@' && *(d + 1) <= 'C') || *(d + 1) == '1')) {
                    d++;
                    if (*d == '1') {
                        if (b + 5 >= be) {
                            fprintf(stderr, "testUtilBwippEscape: FNC1 bwipp_data buffer full (%d)\n", bwipp_data_size);
                            return NULL;
                        }
                        strcpy(b, "^FNC1");
                        b += 5;
                        *parsefnc = 1;
                    }
                } else {
                    if (b + 8 >= be) {
                        fprintf(stderr, "testUtilBwippEscape: loop bwipp_data buffer full (%d)\n", bwipp_data_size);
                        return NULL;
                    }
                    sprintf(b, "^%03d^%03d", '\\', *d);
                    b += 8;
                    if (*d == '^' && d + 1 < de && *(d + 1) == '^') {
                        d++;
                    }
                }
            }
            d++;
            *parse = 1;
        } else {
            *b++ = *d++;
        }
    }

    if (b == be && d < de) {
        fprintf(stderr, "testUtilBwippEscape: end bwipp_data buffer full (%d)\n", bwipp_data_size);
        return NULL;
    }
    *b = '\0';

    return bwipp_data;
}

/* Convert ISBN to BWIPP format */
static void testUtilISBNHyphenate(char *bwipp_data, int addon_posn) {
    /* Hack in 4 hyphens in fixed format, wrong for many ISBNs */
    char temp[13 + 4 + 1 + 5 + 1];
    int len = (int) strlen(bwipp_data);
    int i, j;

    if (len < 13 || (addon_posn && addon_posn < 13 ) || len >= (int) sizeof(temp)) {
        return;
    }
    for (i = 0, j = 0; i <= len; i++, j++) {
        if (i == 3 || i == 5 || i == 10 || i == 12) {
            temp[j++] = '-';
        }
        temp[j] = bwipp_data[i];
    }
    strcpy(bwipp_data, temp);
}

/* Helper to convert UTF-8 data */
static char *testUtilBwippUtf8Convert(const int index, const int symbology, const int try_sjis, int *p_eci,
                        const unsigned char *data, int *p_data_len, unsigned char *converted) {
    int eci = *p_eci;

    if (eci == 0 && try_sjis
            && (symbology == BARCODE_QRCODE || symbology == BARCODE_MICROQR || symbology == BARCODE_RMQR || symbology == BARCODE_UPNQR)) {
        if (symbology == BARCODE_UPNQR) { /* Note need to add "force binary mode" to BWIPP for this to work */
            if (utf8_to_eci(4, data, converted, p_data_len) != 0) {
                fprintf(stderr, "i:%d testUtilBwippUtf8Convert: failed to convert UTF-8 data for %s, ECI 4\n",
                        index, testUtilBarcodeName(symbology));
                return NULL;
            }
            *p_eci = 4;
        } else {
            if (utf8_to_eci(0, data, converted, p_data_len) != 0) {
                if (utf8_to_eci(20, data, converted, p_data_len) != 0) {
                    fprintf(stderr, "i:%d testUtilBwippUtf8Convert: failed to convert UTF-8 data for %s, ECI 0/20\n",
                            index, testUtilBarcodeName(symbology));
                    return NULL;
                }
                /* NOTE: not setting *p_eci = 20 */
            }
        }
        return (char *) converted;
    }
    if (ZBarcode_Cap(symbology, ZINT_CAP_ECI)) {
        if (utf8_to_eci(eci, data, converted, p_data_len) != 0) {
            if (eci != 0) {
                fprintf(stderr, "i:%d testUtilBwippUtf8Convert: failed to convert UTF-8 data for %s, ECI %d\n",
                        index, testUtilBarcodeName(symbology), eci);
                return NULL;
            }
            *p_eci = eci = get_best_eci(data, *p_data_len);
            if (utf8_to_eci(eci, data, converted, p_data_len) != 0) {
                fprintf(stderr, "i:%d testUtilBwippUtf8Convert: failed to convert UTF-8 data for %s, ECI %d\n",
                        index, testUtilBarcodeName(symbology), eci);
                return NULL;
            }
        }
        return (char *) converted;
    }
    if (eci != 0) {
        fprintf(stderr, "i:%d testUtilBwippUtf8Convert: ECI %d but not supported for %s\n",
                index, eci, testUtilBarcodeName(symbology));
        return NULL;
    }
    if (utf8_to_eci(eci, data, converted, p_data_len) != 0) {
        fprintf(stderr, "i:%d testUtilBwippUtf8Convert: failed to convert UTF-8 data for %s, default ECI %d\n",
                index, testUtilBarcodeName(symbology), eci);
        return NULL;
    }

    return (char *) converted;
}

#define GS_INITIAL_LEN  35 /* Length of cmd up to -q */

/* Create bwipp_dump.ps command and run */
int testUtilBwipp(int index, const struct zint_symbol *symbol, int option_1, int option_2, int option_3,
            const char *data, int length, const char *primary, char *buffer, int buffer_size, int *p_parsefnc) {
    static const char cmd_fmt[] = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%s'"
                                    " backend/tests/tools/bwipp_dump.ps";
    static const char cmd_opts_fmt[] = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%s' -so='%s'"
                                        " backend/tests/tools/bwipp_dump.ps";
    /* If data > 2K and < ~4K */
    static const char cmd_fmt2[] = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%.2040s' -sd2='%s'"
                                        " backend/tests/tools/bwipp_dump.ps";
    static const char cmd_opts_fmt2[] = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%.2040s' -sd2='%s' -so='%s'"
                                        " backend/tests/tools/bwipp_dump.ps";
    /* If data > ~4K and < ~6K */
    static const char cmd_fmt3[] = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%.2040s' -sd2='%.2040s' -sd3='%s'"
                                        " backend/tests/tools/bwipp_dump.ps";
    static const char cmd_opts_fmt3[] = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%.2040s' -sd2='%.2040s' -sd3='%s'"
                                        " -so='%s' backend/tests/tools/bwipp_dump.ps";
    /* If data > ~6K and < ~8K */
    static const char cmd_fmt4[] = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%.2040s' -sd2='%.2040s' -sd3='%.2040s'"
                                        " -sd4='%s' backend/tests/tools/bwipp_dump.ps";
    static const char cmd_opts_fmt4[] = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%.2040s' -sd2='%.2040s'"
                                        " -sd3='%.2040s' -sd4='%s' -so='%s' backend/tests/tools/bwipp_dump.ps";

    const int symbology = symbol->symbology;
    int data_len = length == -1 ? (int) strlen(data) : length;
    int primary_len = primary ? (int) strlen(primary) : 0;
    /* 4 AI prefix + primary + '|' + leading zero + escaped data + fudge */
    int max_data_len = 4 + primary_len + 1 + 1 + data_len * 4 + 64;

    int eci_length = get_eci_length(symbol->eci, (const unsigned char *) data, data_len);
    char *converted = (char *) z_alloca(eci_length + 1);
    char *cmd = (char *) z_alloca(max_data_len + 1024);
    const char *bwipp_barcode = NULL;
    char *bwipp_opts = NULL;
    int bwipp_data_size = max_data_len + 1;
    char *bwipp_data = (char *) z_alloca(bwipp_data_size);
    char bwipp_opts_buf[512];
    int *bwipp_row_height = (int *) z_alloca(sizeof(int) * symbol->rows);
    int linear_row_height;
    int gs1_cvt;
    int user_mask;

    FILE *fp = NULL;
    int cnt;

    char *b = buffer;
    char *be = buffer + buffer_size;
    int r, h;
    int parse = 0, parsefnc = p_parsefnc ? *p_parsefnc : 0;

    const int upcean = (ZBarcode_Cap(symbology, ZINT_CAP_EANUPC) & ZINT_CAP_EANUPC) == ZINT_CAP_EANUPC;
    const int upca = symbology == BARCODE_UPCA || symbology == BARCODE_UPCA_CHK || symbology == BARCODE_UPCA_CC;
    const int parens_mode = symbol->input_mode & GS1PARENS_MODE;
    const char obracket = parens_mode ? '(' : '[';
    const char cbracket = parens_mode ? ')' : ']';
    int addon_posn;
    int eci;
    int i, j, len;

    bwipp_data[0] = bwipp_opts_buf[0] = '\0';

    bwipp_barcode = testUtilBwippName(index, symbol, option_1, option_2, option_3, 0, &linear_row_height, &gs1_cvt);
    if (!bwipp_barcode) {
        fprintf(stderr, "i:%d testUtilBwipp: no mapping for %s, option_1 %d, option_2 %d, option_3 %d\n",
                index, testUtilBarcodeName(symbology), option_1, option_2, option_3);
        return -1;
    }

    for (r = 0; r < symbol->rows; r++) {
        if (symbology == BARCODE_MAXICODE) {
            bwipp_row_height[r] = 1;
        } else {
            bwipp_row_height[r] = symbol->row_height[r] ? symbol->row_height[r] : linear_row_height;
        }
        if ((symbol->debug & ZINT_DEBUG_TEST_PRINT) && !(symbol->debug & ZINT_DEBUG_TEST_LESS_NOISY)) {
            fprintf(stderr, "bwipp_row_height[%d] %d, symbol->row_height[%d] %g\n",
                        r, bwipp_row_height[r], r, symbol->row_height[r]);
        }
    }

    eci = symbol->eci >= 3 && ZBarcode_Cap(symbology, ZINT_CAP_ECI) ? symbol->eci : 0;

    if ((symbol->input_mode & 0x07) == UNICODE_MODE && is_eci_convertible(eci)
            && (data = testUtilBwippUtf8Convert(index, symbology, 1 /*try_sjis*/, &eci, (const unsigned char *) data,
                                &data_len, (unsigned char *) converted)) == NULL) {
        fprintf(stderr, "i:%d testUtilBwipp: failed to convert UTF-8 data for %s\n",
                index, testUtilBarcodeName(symbology));
        return -1;
    }

    if (is_composite(symbology)) {
        if (!primary) {
            fprintf(stderr, "i:%d testUtilBwipp: no primary data given %s\n", index, testUtilBarcodeName(symbology));
            return -1;
        }
        if (*primary != obracket && !upcean) {
            strcat(bwipp_data, "(01)");
        }
        strcat(bwipp_data, primary);
        strcat(bwipp_data, "|");
        strcat(bwipp_data, data);
        testUtilBwippCvtGS1Data(bwipp_data, upcean, parens_mode, &addon_posn);

        /* Always set dontlint for now (until support for exclusive AIs check) */
        sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sdontlint", strlen(bwipp_opts_buf) ? " " : "");
        bwipp_opts = bwipp_opts_buf;

        if (upcean) {
            if (symbology == BARCODE_EANX_CC && (primary_len <= 8 || (addon_posn && addon_posn <= 8))) {
                bwipp_barcode = "ean8composite";
            }
            if (addon_posn) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%saddongap=%d",
                        strlen(bwipp_opts_buf) ? " " : "", option_2 > 0 ? option_2 : upca ? 9 : 7);
            }
            bwipp_row_height[symbol->rows - 1] = 72;
        }

        if (option_1 > 0) {
            sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sccversion=%c",
                    strlen(bwipp_opts_buf) ? " " : "", option_1 == 1 ? 'a' : option_1 == 2 ? 'b' : 'c');
        }
        if (option_2 > 0) {
            sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%ssegments=%d",
                    strlen(bwipp_opts_buf) ? " " : "", option_2 * 2);
        }

        if (symbol->input_mode & GS1NOCHECK_MODE) {
            /* sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sdontlint", strlen(bwipp_opts_buf) ? " " : ""); */
        }
    } else {
        if (gs1_cvt) {
            if (*data != obracket && !upcean) {
                strcat(bwipp_data, symbology == BARCODE_NVE18 ? "(00)" : "(01)");
            }
            strcat(bwipp_data, data);
            testUtilBwippCvtGS1Data(bwipp_data, upcean, parens_mode, &addon_posn);

            /* Always set dontlint for now (until support for exclusive AIs check) */
            sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sdontlint", strlen(bwipp_opts_buf) ? " " : "");
            bwipp_opts = bwipp_opts_buf;

            if (upcean) {
                if ((symbology == BARCODE_EANX || symbology == BARCODE_EANX_CHK)
                        && (data_len <= 8 || (addon_posn && addon_posn <= 8))) {
                    bwipp_barcode = data_len <= 3 ? "ean2" : data_len <= 5 ? "ean5" : "ean8";
                }
                if (symbology == BARCODE_ISBNX) {
                    testUtilISBNHyphenate(bwipp_data, addon_posn);
                }
                if (addon_posn) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%saddongap=%d",
                            strlen(bwipp_opts_buf) ? " " : "", option_2 > 0 ? option_2 : upca ? 9 : 7);
                }
            }

            if (option_2 > 0) {
                if (symbology == BARCODE_DBAR_EXP || symbology == BARCODE_DBAR_EXPSTK) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%ssegments=%d",
                            strlen(bwipp_opts_buf) ? " " : "", option_2 * 2);
                }
            }

            if (symbol->input_mode & GS1NOCHECK_MODE) {
                /* sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sdontlint", strlen(bwipp_opts_buf) ? " " : ""); */
            }
        } else {
            const int is_extra_escaped = (symbol->input_mode & EXTRA_ESCAPE_MODE)
                                            && symbol->symbology == BARCODE_CODE128;
            const int is_escaped = (symbol->input_mode & ESCAPE_MODE) || is_extra_escaped;
            if (testUtilBwippEscape(bwipp_data, bwipp_data_size, data, data_len, is_escaped, eci, &parse, &parsefnc)
                    == NULL) {
                return -1;
            }
            if (parse) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sparse", strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
            }
            if (parsefnc) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sparsefnc",
                        strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
            }

            if (symbology == BARCODE_C25STANDARD || symbology == BARCODE_C25INTER || symbology == BARCODE_C25IATA
                    || symbology == BARCODE_C25LOGIC || symbology == BARCODE_C25IND) {
                if (option_2 == 1 || option_2 == 2) { /* Add check digit without or with HRT suppression */
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sincludecheck",
                            strlen(bwipp_opts_buf) ? " " : "");
                    bwipp_opts = bwipp_opts_buf;
                }
            } else if (symbology == BARCODE_CODE93) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sincludecheck",
                        strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
            } else if (symbology == BARCODE_PZN) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%spzn%c",
                        strlen(bwipp_opts_buf) ? " " : "", option_2 == 1 ? '7' : '8');
                bwipp_opts = bwipp_opts_buf;
            } else if (symbology == BARCODE_TELEPEN_NUM) {
                if (data_len & 1) { /* Add leading zero */
                    memmove(bwipp_data + 1, bwipp_data, strlen(bwipp_data) + 1);
                    *bwipp_data = '0';
                }
            } else if (symbology == BARCODE_CODABLOCKF || symbology == BARCODE_HIBC_BLOCKF) {
                if (option_1 > 0) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%srows=%d",
                            strlen(bwipp_opts_buf) ? " " : "", option_1);
                }
                if (option_2 > 0) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%scolumns=%d",
                            strlen(bwipp_opts_buf) ? " " : "", option_2 - 5);
                } else {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%scolumns=%d",
                            strlen(bwipp_opts_buf) ? " " : "", (symbol->width - 57) / 11);
                }
                bwipp_opts = bwipp_opts_buf;
            } else if (symbology == BARCODE_CODE11 || symbology == BARCODE_CODE39 || symbology == BARCODE_EXCODE39
                        || symbology == BARCODE_LOGMARS || symbology == BARCODE_CODABAR) {
                if (option_2 > 0) {
                    if (option_2 == 1) {
                        sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sincludecheck",
                                strlen(bwipp_opts_buf) ? " " : "");
                    }
                    bwipp_opts = bwipp_opts_buf; /* Set always as option_2 == 2 is bwipp default */
                }
            } else if (symbology == BARCODE_PLESSEY) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sincludecheck", strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
            } else if (symbology == BARCODE_MSI_PLESSEY) {
                if (option_2 > 0) {
                    const char *checktype = NULL;
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sincludecheck",
                            strlen(bwipp_opts_buf) ? " " : "");

                    if (option_2 >= 11 && option_2 <= 16) {
                        option_2 -= 10; /* Remove no-check indicator */
                    }
                    if (option_2 == 2) {
                        checktype = "mod1010";
                    } else if (option_2 == 3) {
                        checktype = "mod11 badmod11";
                    } else if (option_2 == 4) {
                        checktype = "mod1110 badmod11";
                    } else if (option_2 == 5) {
                        checktype = "ncrmod11 badmod11";
                    } else if (option_2 == 6) {
                        checktype = "ncrmod1110 badmod11";
                    }
                    if (checktype) {
                        sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%schecktype=%s",
                                strlen(bwipp_opts_buf) ? " " : "", checktype);
                    }
                    bwipp_opts = bwipp_opts_buf;
                }
            } else if (symbology == BARCODE_PDF417 || symbology == BARCODE_PDF417COMP || symbology == BARCODE_HIBC_PDF
                        || symbology == BARCODE_MICROPDF417 || symbology == BARCODE_HIBC_MICPDF) {
                const int row_height = symbology == BARCODE_MICROPDF417 || symbology == BARCODE_HIBC_MICPDF ? 2 : 3;
                for (r = 0; r < symbol->rows; r++) bwipp_row_height[r] = row_height;
                if (option_1 >= 0) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%seclevel=%d fixedeclevel",
                            strlen(bwipp_opts_buf) ? " " : "", option_1);
                    bwipp_opts = bwipp_opts_buf;
                }
                if (option_2 > 0) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%scolumns=%d",
                            strlen(bwipp_opts_buf) ? " " : "", option_2);
                    bwipp_opts = bwipp_opts_buf;
                }
                if (option_3 > 0) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%srows=%d",
                            strlen(bwipp_opts_buf) ? " " : "", option_3);
                    bwipp_opts = bwipp_opts_buf;
                }
            } else if (symbology == BARCODE_POSTNET || symbology == BARCODE_PLANET || symbology == BARCODE_RM4SCC
                        || symbology == BARCODE_JAPANPOST || symbology == BARCODE_KIX || symbology == BARCODE_DAFT
                        || symbology == BARCODE_USPS_IMAIL || symbology == BARCODE_AUSPOST
                        || symbology == BARCODE_PHARMA_TWO) {
                for (r = 0; r < symbol->rows; r++) bwipp_row_height[r] = 1; /* Zap */
                if (symbology == BARCODE_RM4SCC || symbology == BARCODE_KIX || symbology == BARCODE_JAPANPOST || symbology == BARCODE_DAFT) {
                    to_upper((unsigned char *) bwipp_data, (int) strlen(bwipp_data));
                } else if (symbology == BARCODE_USPS_IMAIL) {
                    char *dash = strchr(bwipp_data, '-');
                    if (dash) {
                        memmove(dash, dash + 1, strlen(dash));
                    }
                } else if (symbology == BARCODE_AUSPOST) {
                    const char *prefix;
                    if (data_len == 8) {
                        prefix = "11";
                    } else if (data_len == 13 || data_len == 16) {
                        prefix = "59";
                        if (data_len == 16) {
                            sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%scustinfoenc=numeric",
                                    strlen(bwipp_opts_buf) ? " " : "");
                            bwipp_opts = bwipp_opts_buf;
                        }
                    } else {
                        prefix = "62";
                        if (data_len == 23) {
                            sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%scustinfoenc=numeric",
                                    strlen(bwipp_opts_buf) ? " " : "");
                            bwipp_opts = bwipp_opts_buf;
                        }
                    }
                    memmove(bwipp_data + 2, bwipp_data, data_len + 1);
                    memmove(bwipp_data, prefix, 2);
                }
            } else if (symbology == BARCODE_CODE128AB) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%ssuppressc", strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
            } else if (symbology == BARCODE_DPD) {
                if (data_len == 27 && option_2 != 1) {
                    memmove(bwipp_data + 1, bwipp_data, data_len + 1);
                    bwipp_data[0] = '%';
                }
                bwipp_opts = bwipp_opts_buf;
            } else if (symbology == BARCODE_FIM) {
                strcpy(bwipp_data, "fima");
                bwipp_data[3] = z_isupper(data[0]) ? data[0] - 'A' + 'a' : data[0];
            } else if (symbology == BARCODE_CODE16K || symbology == BARCODE_CODE49) {
                if (option_1 >= 2) {
                    if ((symbology == BARCODE_CODE16K && option_1 <= 16)
                            || (symbology == BARCODE_CODE49 && option_1 <= 8 && option_1 >= symbol->rows)) {
                        sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%srows=%d",
                            strlen(bwipp_opts_buf) ? " " : "", option_1);
                    }
                    bwipp_opts = bwipp_opts_buf;
                }
            } else if (symbology == BARCODE_AZTEC || symbology == BARCODE_HIBC_AZTEC) {
                int compact = 0, full = 0;
                if (option_1 >= 1 && option_1 <= 4) {
                    int eclevel;
                    if (option_1 == 1) {
                        eclevel = 10;
                    } else if (option_1 == 2) {
                        eclevel = 23;
                    } else if (option_1 == 3) {
                        eclevel = 36;
                    } else {
                        eclevel = 50;
                    }
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%seclevel=%d",
                            strlen(bwipp_opts_buf) ? " " : "", eclevel);
                    bwipp_opts = bwipp_opts_buf;
                }
                if (option_2 >= 1) {
                    int layers;
                    if (option_2 <= 4) {
                        compact = 1;
                        layers = option_2;
                    } else {
                        layers = option_2 - 4;
                        full = layers <= 4;
                    }
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%slayers=%d",
                            strlen(bwipp_opts_buf) ? " " : "", layers);
                    bwipp_opts = bwipp_opts_buf;
                }
                if (symbol->output_options & READER_INIT) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sreaderinit",
                            strlen(bwipp_opts_buf) ? " " : "");
                    bwipp_opts = bwipp_opts_buf;
                }
                if (symbology == BARCODE_HIBC_AZTEC) {
                    compact = 1;
                }
                if (compact || full) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sformat=%s",
                            strlen(bwipp_opts_buf) ? " " : "", compact ? "compact" : "full");
                    bwipp_opts = bwipp_opts_buf;
                }
            } else if (symbology == BARCODE_CODEONE) {
                if ((symbol->input_mode & 0x07) == GS1_MODE) { /* Hack pseudo-GS1 support */
                    int last_ai, ai_latch = 0;
                    /* Reduce square brackets (include NUL) */
                    for (i = 0, j = 0, len = (int) strlen(bwipp_data); i <= len; i++) {
                        if (bwipp_data[i] == obracket) {
                            if (ai_latch == 0) {
                                bwipp_data[j++] = '\x1D';
                            }
                            last_ai = to_int((unsigned char *) (bwipp_data + i + 1), 2);
                            if ((last_ai >= 0 && last_ai <= 4) || (last_ai >= 11 && last_ai <= 20) || last_ai == 23
                                    || (last_ai >= 31 && last_ai <= 36) || last_ai == 41) {
                                ai_latch = 1;
                            }
                        } else if (bwipp_data[i] != cbracket) {
                            bwipp_data[j++] = bwipp_data[i];
                        }
                    }
                    /* Replace square brackets with ^FNC1 */
                    for (len = (int) strlen(bwipp_data), i = len - 1; i >= 0; i--) {
                        if (bwipp_data[i] == '\x1D') {
                            memmove(bwipp_data + i + 5, bwipp_data + i + 1, len - i);
                            memcpy(bwipp_data + i, "^FNC1", 5);
                            len += 4;
                        }
                    }
                    if (!parsefnc) { /* If not already done for ECI */
                        sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sparsefnc",
                                strlen(bwipp_opts_buf) ? " " : "");
                        bwipp_opts = bwipp_opts_buf;
                    }
                }
                if (option_2 >= 1 && option_2 <= 10) {
                    static const char *codeone_versions[] = { "A", "B", "C", "D", "E", "F", "G", "H" };
                    const char *codeone_version;
                    if (option_2 == 9) {
                        codeone_version = length <= 6 ? "S-10" : length <= 12 ? "S-20" : "S-30";
                    } else if (option_2 == 10) {
                        /* TODO: Properly allow for different T sizes */
                        codeone_version = length <= 22 ? "T-16" : length <= 34 ? "T-32" : "T-48";
                    } else {
                        codeone_version = codeone_versions[option_2 - 1];
                    }
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sversion=%s",
                            strlen(bwipp_opts_buf) ? " " : "", codeone_version);
                    bwipp_opts = bwipp_opts_buf;
                }
            } else if (symbology == BARCODE_MAXICODE) {
                int have_eci = memcmp(bwipp_data, "^ECI", 4) == 0;
                int have_scm = memcmp(bwipp_data + have_eci * 10, "[)>^03001^029", 13) == 0
                                    && z_isdigit(bwipp_data[13 + have_eci * 10])
                                    && z_isdigit(bwipp_data[14 + have_eci * 10]);
                int mode = option_1;
                char prefix_buf[30];
                int prefix_len = 0;
                if (mode <= 0) {
                    if (primary_len == 0) {
                        mode = 4;
                    } else {
                        mode = 2;
                        for (i = 0; i < primary_len - 6; i++) {
                            if (!z_isdigit(symbol->primary[i]) && (symbol->primary[i] != ' ')) {
                                mode = 3;
                                break;
                            }
                        }
                    }
                }
                if (mode > 0) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%smode=%d",
                            strlen(bwipp_opts_buf) ? " " : "", mode);
                    bwipp_opts = bwipp_opts_buf;
                }
                if (option_2 > 0) {
                    char scm_vv_buf[40];
                    sprintf(scm_vv_buf, "[)>^03001^029%02d", option_2 - 1); /* [)>\R01\Gvv */
                    memmove(bwipp_data + 15, bwipp_data, strlen(bwipp_data) + 1);
                    memcpy(bwipp_data, scm_vv_buf, 15);
                    have_scm = 1;
                    have_eci = 0;
                    if (!parse) {
                        sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sparse",
                                strlen(bwipp_opts_buf) ? " " : "");
                        bwipp_opts = bwipp_opts_buf;
                        parse = 1;
                    }
                }
                if (primary_len >= 6) { /* Keep gcc happy */
                    int postcode_len = primary_len - 6;
                    char postcode[10];
                    if (postcode_len >= 10) postcode_len = 9;
                    memcpy(postcode, primary, postcode_len);
                    postcode[postcode_len] = '\0';
                    if (mode == 2) {
                        for (i = 0; i < postcode_len; i++) {
                            if (postcode[i] == ' ') {
                                postcode[i] = '\0';
                            }
                        }
                    } else {
                        postcode[6] = '\0';
                        for (i = postcode_len; i < 6; i++) {
                            postcode[i] = ' ';
                        }
                    }
                    sprintf(prefix_buf, "%s^029%.3s^029%.3s^029",
                            postcode, primary + primary_len - 6, primary + primary_len - 3);
                    prefix_len = (int) strlen(prefix_buf);
                }
                if (prefix_len || have_scm) {
                    char eci_buf[10];
                    int offset = 15 * have_scm;
                    if (have_eci) {
                        memcpy(eci_buf, bwipp_data, 10);
                        memmove(bwipp_data, bwipp_data + 10, strlen(bwipp_data) - 10 + 1);
                    }
                    memmove(bwipp_data + offset + prefix_len, bwipp_data + offset, strlen(bwipp_data) - offset + 1);
                    memcpy(bwipp_data + offset, prefix_buf, prefix_len);
                    if (have_eci) {
                        memmove(bwipp_data + offset + prefix_len + 10, bwipp_data + offset + prefix_len,
                                strlen(bwipp_data) - (offset + prefix_len) + 1);
                        memcpy(bwipp_data + offset + prefix_len, eci_buf, 10);
                    }
                }
                if (!parse) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sparse",
                            strlen(bwipp_opts_buf) ? " " : "");
                    bwipp_opts = bwipp_opts_buf;
                    parse = 1;
                }
                if (symbol->structapp.count) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%ssam=%c%c",
                            strlen(bwipp_opts_buf) ? " " : "", itoc(symbol->structapp.index),
                            itoc(symbol->structapp.count));
                    bwipp_opts = bwipp_opts_buf;
                }
            } else if (symbology == BARCODE_BC412) {
                to_upper((unsigned char *) bwipp_data, (int) strlen(bwipp_data));
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%ssemi", strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
            } else if (symbology == BARCODE_MAILMARK_2D) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%stype=%d",
                        strlen(bwipp_opts_buf) ? " " : "", option_2 - 1);
                bwipp_opts = bwipp_opts_buf;
            }
        }

        if (symbology == BARCODE_CODE128 || symbology == BARCODE_CODE128AB || symbology == BARCODE_HIBC_128
                || symbology == BARCODE_GS1_128 || symbology == BARCODE_GS1_128_CC || symbology == BARCODE_NVE18
                || symbology == BARCODE_EAN14 || symbology == BARCODE_UPU_S10 || symbology == BARCODE_MAXICODE) {
            sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%snewencoder", strlen(bwipp_opts_buf) ? " " : "");
            bwipp_opts = bwipp_opts_buf;
        }

        if (symbology == BARCODE_DATAMATRIX || symbology == BARCODE_HIBC_DM) {
            int added_dmre = 0;
            #include "../dmatrix.h"
            (void)dm_matrixrsblock; (void)dm_matrixdatablock; (void)dm_matrixbytes;
            (void)dm_matrixFW; (void)dm_matrixFH;
            (void)dm_isDMRE; (void)dm_text_value; (void)dm_text_shift; (void)dm_c40_value; (void)dm_c40_shift;

            if (symbol->output_options & GS1_GS_SEPARATOR) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sgssep", strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
            }
            if (option_2 >= 1 && option_2 <= ARRAY_SIZE(dm_intsymbol)) {
                int idx = dm_intsymbol[option_2 - 1];
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%srows=%d columns=%d",
                        strlen(bwipp_opts_buf) ? " " : "", dm_matrixH[idx], dm_matrixW[idx]);
                bwipp_opts = bwipp_opts_buf;
                if (option_2 >= 31) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sdmre", strlen(bwipp_opts_buf) ? " " : "");
                    added_dmre = 1;
                }
            }
            if ((option_3 & 0x7F) != DM_SQUARE && symbol->width != symbol->height) {
                if ((option_3 & 0x7F) == DM_DMRE && !added_dmre) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sdmre", strlen(bwipp_opts_buf) ? " " : "");
                    /*added_dmre = 1; */
                }
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sformat=rectangle",
                        strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
            }
            if (option_3 != -1) {
                bwipp_opts = bwipp_opts_buf;
            }
        } else if (symbology == BARCODE_DOTCODE) {
            if (option_2 > 0) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%scolumns=%d",
                        strlen(bwipp_opts_buf) ? " " : "", symbol->option_2);
                bwipp_opts = bwipp_opts_buf;
            }
            if (option_3 != -1) {
                user_mask = (option_3 >> 8) & 0x0F; /* User mask is pattern + 1, so >= 1 and <= 8 */
                if (user_mask >= 1 && user_mask <= 8) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%smask=%d",
                            strlen(bwipp_opts_buf) ? " " : "", (user_mask - 1) % 4);
                    bwipp_opts = bwipp_opts_buf;
                }
            }
            /* Hack to place ECI after Macro header */
            if (eci && length > 5 && memcmp("[)>\036", data, 4) == 0) {
                char macro_eci_buf[13];
                if (length > 6 && data[6] == 29 /*GS*/ && ((data[4] == '0' && data[5] == '5')
                        || (data[4] == '0' && data[5] == '6') || (data[4] == '1' && data[5] == '2'))) {
                    memcpy(macro_eci_buf, bwipp_data + 10, 13); /* Macro */
                    memcpy(bwipp_data + 13, bwipp_data, 10); /* ECI */
                    memcpy(bwipp_data, macro_eci_buf, 13);
                } else if (z_isdigit(data[4]) && z_isdigit(data[5])) {
                    memcpy(macro_eci_buf, bwipp_data, 10); /* ECI */
                    memcpy(bwipp_data, bwipp_data + 10, 9); /* Macro */
                    memcpy(bwipp_data + 9, macro_eci_buf, 10);
                }
            }
        } else if (symbology == BARCODE_QRCODE || symbology == BARCODE_HIBC_QR || symbology == BARCODE_MICROQR
                || symbology == BARCODE_RMQR) {
            if (option_1 >= 1 && option_1 <= 4) {
                static const char eccs[4] = { 'L', 'M', 'Q', 'H' };
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%seclevel=%c fixedeclevel",
                        strlen(bwipp_opts_buf) ? " " : "", eccs[option_1 - 1]);
                bwipp_opts = bwipp_opts_buf;
            }
            if (symbology == BARCODE_RMQR) {
                if (option_2 >= 1 && option_2 <= 32) {
                    static const char *vers[] = {
                        "R7x43", "R7x59", "R7x77", "R7x99", "R7x139",
                        "R9x43", "R9x59", "R9x77", "R9x99", "R9x139",
                        "R11x27", "R11x43", "R11x59", "R11x77", "R11x99", "R11x139",
                        "R13x27", "R13x43", "R13x59", "R13x77", "R13x99", "R13x139",
                        "R15x43", "R15x59", "R15x77", "R15x99", "R15x139",
                        "R17x43", "R17x59", "R17x77", "R17x99", "R17x139",
                    };
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sversion=%s",
                            strlen(bwipp_opts_buf) ? " " : "", vers[option_2 - 1]);
                    bwipp_opts = bwipp_opts_buf;
                }
            } else {
                if (option_2 >= 1 && option_2 <= 40) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sversion=%s%d",
                            strlen(bwipp_opts_buf) ? " " : "", symbology == BARCODE_MICROQR ? "M" : "", option_2);
                    bwipp_opts = bwipp_opts_buf;
                }
                if (option_3 != -1) {
                    int mask = (symbol->option_3 >> 8) & 0x0F;
                    if (mask >= 1 && ((symbology != BARCODE_MICROQR && mask <= 8)
                            || (symbology == BARCODE_MICROQR && mask <= 4))) {
                        sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%smask=%d",
                                strlen(bwipp_opts_buf) ? " " : "", ((symbol->option_3 >> 8) & 0x0F));
                        bwipp_opts = bwipp_opts_buf;
                    }
                }
            }
        } else if (symbology == BARCODE_ULTRA) {
            const int rev = option_2 == 2 ? 2 : 1;
            if (option_1 >= 1 && option_1 <= 6) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%seclevel=EC%d",
                        strlen(bwipp_opts_buf) ? " " : "", option_1 - 1);
            }
            sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%srev=%d", strlen(bwipp_opts_buf) ? " " : "", rev);
            bwipp_opts = bwipp_opts_buf;
        }
    }

    if ((option_1 != -1 || option_2 != -1 || option_3 != -1) && !bwipp_opts) {
        fprintf(stderr,
                "i:%d testUtilBwipp: no BWIPP options set option_1 %d, option_2 %d, option_3 %d for symbology %s\n",
                index, option_1, option_2, option_3, testUtilBarcodeName(symbology));
        return -1;
    }

    if (bwipp_opts && strlen(bwipp_opts)) {
        if (strlen(bwipp_data) >= 2040) { /* Ghostscript's `arg_str_max` 2048 less "-sd?=" + quotes */
            if (strlen(bwipp_data) >= 2040 * 2) {
                if (strlen(bwipp_data) >= 2040 * 3) {
                    sprintf(cmd, cmd_opts_fmt4, bwipp_barcode, bwipp_data, bwipp_data + 2040, bwipp_data + 2040 * 2,
                            bwipp_data + 2040 * 3, bwipp_opts);
                } else {
                    sprintf(cmd, cmd_opts_fmt3, bwipp_barcode, bwipp_data, bwipp_data + 2040, bwipp_data + 2040 * 2,
                            bwipp_opts);
                }
            } else {
                sprintf(cmd, cmd_opts_fmt2, bwipp_barcode, bwipp_data, bwipp_data + 2040, bwipp_opts);
            }
        } else {
            sprintf(cmd, cmd_opts_fmt, bwipp_barcode, bwipp_data, bwipp_opts);
        }
    } else {
        if (strlen(bwipp_data) >= 2040) {
            if (strlen(bwipp_data) >= 2040 * 2) {
                if (strlen(bwipp_data) >= 2040 * 3) {
                    sprintf(cmd, cmd_fmt4, bwipp_barcode, bwipp_data, bwipp_data + 2040, bwipp_data + 2040 * 2,
                            bwipp_data + 2040 * 3);
                } else {
                    sprintf(cmd, cmd_fmt3, bwipp_barcode, bwipp_data, bwipp_data + 2040, bwipp_data + 2040 * 2);
                }
            } else {
                sprintf(cmd, cmd_fmt2, bwipp_barcode, bwipp_data, bwipp_data + 2040);
            }
        } else {
            sprintf(cmd, cmd_fmt, bwipp_barcode, bwipp_data);
        }
    }

    /* Hack in various adjustments */
    if (symbology == BARCODE_DBAR_OMN || symbology == BARCODE_DBAR_LTD || symbology == BARCODE_DBAR_EXP) {
        /* Begin with space */
        char adj[] = " -sbs";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj) - 1, cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj) - 1);
    }
    if (symbology == BARCODE_CODE11 || symbology == BARCODE_CODE39 || symbology == BARCODE_EXCODE39
            || symbology == BARCODE_CODABAR || symbology == BARCODE_PHARMA || symbology == BARCODE_PZN
            || symbology == BARCODE_CODE32 || symbology == BARCODE_VIN) {
        /* Ratio 3 width bar/space -> 2 width */
        char adj[] = " -sr=0.6";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj) - 1, cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj) - 1);
    }
    if (symbology == BARCODE_C25INTER || symbology == BARCODE_DPLEIT || symbology == BARCODE_DPIDENT
            || symbology == BARCODE_ITF14) {
        /* Ratio 2 width bar/space -> 3 width */
        char adj[] = " -sr=1.3";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj) - 1, cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj) - 1);
    }
    if (symbology == BARCODE_FIM) {
        /* Ratio 2.25 width bar/space -> 1 width */
        char adj[] = " -sr=0.444";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj) - 1, cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj) - 1);
    }
    if (symbology == BARCODE_PLESSEY) {
        /* Ceiling ratio 3/4/5 width bar/space -> 2 width then round ratio 2 width bar/space -> 3 width */
        char adj[] = " -sc=0.4 -sr=1.3";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj) - 1, cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj) - 1);
    }
    if (symbology == BARCODE_CODE11 || symbology == BARCODE_CODE39 || symbology == BARCODE_EXCODE39
            || symbology == BARCODE_HIBC_39 || symbology == BARCODE_LOGMARS || symbology == BARCODE_PHARMA
            || symbology == BARCODE_PZN || symbology == BARCODE_CODE32 || symbology == BARCODE_VIN
            || symbology == BARCODE_C25INTER || symbology == BARCODE_DPLEIT || symbology == BARCODE_DPIDENT
            || symbology == BARCODE_ITF14 || symbology == BARCODE_PHARMA_TWO) {
        /* End sbs loop on bar */
        char adj[] = " -selb";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj) - 1, cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj) - 1);
    }
    if (symbology == BARCODE_C25STANDARD) {
        /* Zint uses 4X start/stop wides while BWIPP uses 3X - convert */
        char adj[] = " -sp='i 0 eq i limit 4 sub eq or sbs i get 3 eq and { (1111) print true } { false } ifelse'";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj) - 1, cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj) - 1);
    }
    if (symbology == BARCODE_POSTNET || symbology == BARCODE_PLANET || symbology == BARCODE_RM4SCC
            || symbology == BARCODE_JAPANPOST || symbology == BARCODE_KIX || symbology == BARCODE_DAFT
            || symbology == BARCODE_USPS_IMAIL || symbology == BARCODE_AUSPOST || symbology == BARCODE_PHARMA_TWO) {
        /* Emulate rows with BWIPP heights. */
        char adj[] = " -shs";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj) - 1, cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj) - 1);
    }
    if (symbology == BARCODE_CODE16K || symbology == BARCODE_CODE49) {
        char adj[] = " -sxs=10 -sxe=1"; /* Strip first 10 and last zero */
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj) - 1, cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj) - 1);
    }

    if (symbol->debug & ZINT_DEBUG_TEST_PRINT) {
        printf("i:%d testUtilBwipp: cmd %s\n", index, cmd);
    }

    fp = testutil_popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "i:%d testUtilBwipp: failed to run '%s'\n", index, cmd);
        return -1;
    }

    if (symbol->rows == 0) { /* For testing BWIPP against ZXingC++ (`ZBarcode_Encode()` not called) */
        cnt = (int) fread(b, 1, buffer_size - 1, fp); /* Just read the whole output */
        b += cnt;
    } else {
        for (r = 0; r < symbol->rows; r++) {
            if (b + symbol->width > be) {
                fprintf(stderr, "i:%d testUtilBwipp: row %d, width %d, row width iteration overrun (%s)\n",
                        index, r, symbol->width, cmd);
                testutil_pclose(fp);
                return -1;
            }
            cnt = (int) fread(b, 1, symbol->width, fp);
            if (cnt != symbol->width) {
                fprintf(stderr, "i:%d testUtilBwipp: failed to read row %d of %d, symbol->width %d bytes, cnt %d (%s)\n",
                        index, r + 1, symbol->rows, symbol->width, cnt, cmd);
                testutil_pclose(fp);
                return -1;
            }
            b += cnt;
            for (h = bwipp_row_height[r]; h > 1; h--) { /* Ignore row copies if any */
                cnt = (int) fread(b, 1, symbol->width, fp);
                if (cnt != symbol->width) {
                    fprintf(stderr,
                            "i:%d testUtilBwipp: failed to read/ignore symbol->width %d bytes, cnt %d, h %d"
                            ", bwipp_row_height[%d] %d, symbol->row_height[%d] %g (%s)\n",
                            index, symbol->width, cnt, h, r, bwipp_row_height[r], r, symbol->row_height[r], cmd);
                    testutil_pclose(fp);
                    return -1;
                }
                if (h * 2 == bwipp_row_height[r]) { /* Hack to use middle row (avoids add-on text offsets) */
                    memcpy(b - cnt, b, cnt);
                }
            }
        }
    }
    *b = '\0';

    if (fgetc(fp) != EOF) {
        fprintf(stderr, "i:%d testUtilBwipp: failed to read full stream (%s)\n", index, cmd);
        testutil_pclose(fp);
        return -1;
    }

    testutil_pclose(fp);

    return 0;
}

/* Append multiple segments together and then call `testUtilBwipp()` */
int testUtilBwippSegs(int index, struct zint_symbol *symbol, int option_1, int option_2, int option_3,
            const struct zint_seg segs[], const int seg_count, const char *primary, char *buffer, int buffer_size) {
    const int symbology = symbol->symbology;
    const int unicode_mode = (symbol->input_mode & 0x7) == UNICODE_MODE;
    const int symbol_eci = symbol->eci;
    struct zint_seg *local_segs = (struct zint_seg *) z_alloca(sizeof(struct zint_seg) * seg_count);
    int total_len = 0;
    char *data, *d;
    int parsefnc = 1;
    int ret;
    int i;

    assert(ZBarcode_Cap(symbology, ZINT_CAP_ECI));

    for (i = 0; i < seg_count; i++) {
        local_segs[i] = segs[i];
        if (local_segs[i].length == -1) {
            local_segs[i].length = (int) ustrlen(local_segs[i].source);
        }
        if (unicode_mode) {
            total_len += get_eci_length(local_segs[i].eci, local_segs[i].source, local_segs[i].length);
        } else {
            total_len += local_segs[i].length;
        }
    }
    total_len += 10 * seg_count;
    d = data = (char *) z_alloca(total_len + 1);

    for (i = 0; i < seg_count; i++) {
        if (unicode_mode && is_eci_convertible(local_segs[i].eci)) {
            char *converted = testUtilBwippUtf8Convert(index, symbology, 0 /*try_sjis*/, &local_segs[i].eci,
                                local_segs[i].source, &local_segs[i].length, (unsigned char *) d);
            if (converted == NULL) {
                return -1;
            }
            if (converted == d) {
                /* Ensure default ECI set if follows non-default ECI */
                if (i != 0 && local_segs[i].eci == 0 && local_segs[i - 1].eci > 3) {
                    local_segs[i].eci = 3;
                }
                if (local_segs[i].eci) { /* Note this will fail if have DotCode macro */
                    char eci_str[10 + 1];
                    sprintf(eci_str, "^ECI%06d", local_segs[i].eci);
                    memmove(d + 10, d, local_segs[i].length);
                    memcpy(d, eci_str, 10);
                    d += 10;
                }
                d += local_segs[i].length;
            } else {
                /* Ensure default ECI set if follows non-default ECI */
                if (i != 0 && local_segs[i].eci == 0 && local_segs[i - 1].eci > 3) {
                    local_segs[i].eci = 3;
                }
                if (local_segs[i].eci) {
                    d += sprintf(d, "^ECI%06d", local_segs[i].eci);
                }
                memcpy(d, local_segs[i].source, local_segs[i].length);
                d += local_segs[i].length;
            }
        } else {
            /* Ensure default ECI set if follows non-default ECI */
            if (i != 0 && local_segs[i].eci == 0 && local_segs[i - 1].eci > 3) {
                local_segs[i].eci = 3;
            }
            if (local_segs[i].eci) {
                d += sprintf(d, "^ECI%06d", local_segs[i].eci);
            }
            memcpy(d, local_segs[i].source, local_segs[i].length);
            d += local_segs[i].length;
        }
    }
    total_len = d - data;

    if (unicode_mode) {
        symbol->input_mode = DATA_MODE;
    }
    symbol->eci = 0;

    ret = testUtilBwipp(index, symbol, option_1, option_2, option_3, data, total_len, primary, buffer, buffer_size, &parsefnc);

    if (unicode_mode) {
        symbol->input_mode = UNICODE_MODE;
    }
    symbol->eci = symbol_eci;

    return ret;
}

/* Compare bwipp_dump.ps output to test suite module dump */
int testUtilBwippCmp(const struct zint_symbol *symbol, char *msg, char *cmp_buf, const char *expected) {
    int cmp_len = (int) strlen(cmp_buf);
    int expected_len = (int) strlen(expected);
    int ret_memcmp;
    int i;

    (void)symbol;

    if (cmp_len != expected_len) {
        sprintf(msg, "cmp_len %d != expected_len %d", cmp_len, expected_len);
        return 2;
    }

    if (symbol->symbology == BARCODE_ULTRA) {
        static const char map[] = { '8', '1', '2', '3', '4', '5', '6', '7', '8', '7' };
        for (i = 0; i < cmp_len; i++) {
            if (z_isdigit(cmp_buf[i])) {
                cmp_buf[i] = map[cmp_buf[i] - '0'];
            }
        }
    }
    ret_memcmp = memcmp(cmp_buf, expected, expected_len);
    if (ret_memcmp != 0) {
        for (i = 0; i < expected_len; i++) {
            if (cmp_buf[i] != expected[i]) {
                break;
            }
        }
        sprintf(msg, "memcmp %d != 0, at %d, len %d", ret_memcmp, i, expected_len);
        return ret_memcmp;
    }

    return 0;
}

/* Compare bwipp_dump.ps output to single row module dump (see testUtilModulesPrintRow) */
int testUtilBwippCmpRow(const struct zint_symbol *symbol, int row, char *msg, const char *cmp_buf,
            const char *expected) {
    int cmp_len = (int) strlen(cmp_buf);
    int expected_len = (int) strlen(expected);
    int ret_memcmp;
    int i, j;

    (void)symbol;

    if (cmp_len != expected_len * symbol->rows) {
        sprintf(msg, "cmp_len %d != expected_len %d * symbol->rows %d", cmp_len, expected_len, symbol->rows);
        return 2;
    }

    ret_memcmp = memcmp(cmp_buf + expected_len * row, expected, expected_len);
    if (ret_memcmp != 0) {
        for (i = 0, j = expected_len * row; i < expected_len; i++, j++) {
            if (cmp_buf[j] != expected[i]) {
                break;
            }
        }
        sprintf(msg, "memcmp %d != 0, at %d (%d), len %d", ret_memcmp, i, j, expected_len);
        return ret_memcmp;
    }

    return 0;
}

/* Whether ZXing-C++ Decoder available on system */
/* Requires the "diagnostics2" branch from https://github.com/gitlost/zxing-cpp built with BUILD_EXAMPLE_DECODER
   and "zxingcppdecoder" placed in PATH, e.g.:
     git clone --branch diagnostics2 https://github.com/gitlost/zxing-cpp zxing-cpp-diagnostics2
     cd zxing-cpp-diagnostics2
     mkdir build; cd build
     cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_EXAMPLE_DECODER=ON ..
     make && sudo make install
 */
int testUtilHaveZXingCPPDecoder(void) {
    return system("zxingcppdecoder " DEV_NULL_STDERR) == 0;
}

/* Helper to test whether have non-ASCII */
static int testUtilHasNonASCII(const char *source, const int length) {
    int i;
    for (i = 0; i < length; i++) {
        if (source[i] & 0x80) {
            return 1;
        }
    }
    return 0;
}

/* Map Zint symbology to ZXing-C++ format name */
static const char *testUtilZXingCPPName(int index, const struct zint_symbol *symbol, const char *source,
            const int length, const int debug) {
    struct item {
        const char *name;
        int define;
        int val;
    };
    static const struct item data[] = {
        { "", -1, 0, },
        { "", BARCODE_CODE11, 1, },
        { "", BARCODE_C25STANDARD, 2, },
        { "ITF", BARCODE_C25INTER, 3, },
        { "", BARCODE_C25IATA, 4, },
        { "", -1, 5, },
        { "", BARCODE_C25LOGIC, 6, },
        { "", BARCODE_C25IND, 7, },
        { "Code39", BARCODE_CODE39, 8, },
        { "Code39", BARCODE_EXCODE39, 9, }, /* TODO: Code39 with specially encoded chars */
        { "", -1, 10, },
        { "", -1, 11, },
        { "", -1, 12, },
        { "EAN-13", BARCODE_EANX, 13, },
        { "EAN-13", BARCODE_EANX_CHK, 14, },
        { "", -1, 15, },
        { "Code128", BARCODE_GS1_128, 16, },
        { "", -1, 17, },
        { "Codabar", BARCODE_CODABAR, 18, },
        { "", -1, 19, },
        { "Code128", BARCODE_CODE128, 20, },
        { "ITF", BARCODE_DPLEIT, 21, },
        { "ITF", BARCODE_DPIDENT, 22, },
        { "Code16K", BARCODE_CODE16K, 23, },
        { "", BARCODE_CODE49, 24, },
        { "Code93", BARCODE_CODE93, 25, },
        { "", -1, 26, },
        { "", -1, 27, },
        { "", BARCODE_FLAT, 28, },
        { "DataBar", BARCODE_DBAR_OMN, 29, },
        { "DataBarLimited", BARCODE_DBAR_LTD, 30, },
        { "DataBarExpanded", BARCODE_DBAR_EXP, 31, },
        { "", BARCODE_TELEPEN, 32, },
        { "", -1, 33, },
        { "UPC-A", BARCODE_UPCA, 34, },
        { "UPC-A", BARCODE_UPCA_CHK, 35, },
        { "", -1, 36, },
        { "UPC-E", BARCODE_UPCE, 37, },
        { "UPC-E", BARCODE_UPCE_CHK, 38, },
        { "", -1, 39, },
        { "", BARCODE_POSTNET, 40, },
        { "", -1, 41, },
        { "", -1, 42, },
        { "", -1, 43, },
        { "", -1, 44, },
        { "", -1, 45, },
        { "", -1, 46, },
        { "", BARCODE_MSI_PLESSEY, 47, },
        { "", -1, 48, },
        { "", BARCODE_FIM, 49, },
        { "Code39", BARCODE_LOGMARS, 50, },
        { "", BARCODE_PHARMA, 51, },
        { "Code39", BARCODE_PZN, 52, },
        { "", BARCODE_PHARMA_TWO, 53, },
        { "", -1, 54, },
        { "PDF417", BARCODE_PDF417, 55, },
        { "PDF417", BARCODE_PDF417COMP, 56, },
        { "MaxiCode", BARCODE_MAXICODE, 57, },
        { "QRCode", BARCODE_QRCODE, 58, },
        { "", -1, 59, },
        { "Code128", BARCODE_CODE128AB, 60, },
        { "", -1, 61, },
        { "", -1, 62, },
        { "", BARCODE_AUSPOST, 63, },
        { "", -1, 64, },
        { "", -1, 65, },
        { "", BARCODE_AUSREPLY, 66, },
        { "", BARCODE_AUSROUTE, 67, },
        { "", BARCODE_AUSREDIRECT, 68, },
        { "EAN-13", BARCODE_ISBNX, 69, },
        { "", BARCODE_RM4SCC, 70, },
        { "DataMatrix", BARCODE_DATAMATRIX, 71, },
        { "Code128", BARCODE_EAN14, 72, },
        { "Code39", BARCODE_VIN, 73, },
        { "CodablockF", BARCODE_CODABLOCKF, 74, },
        { "Code128", BARCODE_NVE18, 75, },
        { "", BARCODE_JAPANPOST, 76, },
        { "", BARCODE_KOREAPOST, 77, },
        { "", -1, 78, },
        { "DataBar", BARCODE_DBAR_STK, 79, },
        { "DataBar", BARCODE_DBAR_OMNSTK, 80, },
        { "DataBarExpanded", BARCODE_DBAR_EXPSTK, 81, },
        { "", BARCODE_PLANET, 82, },
        { "", -1, 83, },
        { "MicroPDF417", BARCODE_MICROPDF417, 84, },
        { "", BARCODE_USPS_IMAIL, 85, },
        { "", BARCODE_PLESSEY, 86, },
        { "", BARCODE_TELEPEN_NUM, 87, },
        { "", -1, 88, },
        { "ITF", BARCODE_ITF14, 89, },
        { "", BARCODE_KIX, 90, },
        { "", -1, 91, },
        { "Aztec", BARCODE_AZTEC, 92, },
        { "", BARCODE_DAFT, 93, },
        { "", -1, 94, },
        { "", -1, 95, },
        { "Code128", BARCODE_DPD, 96, },
        { "MicroQRCode", BARCODE_MICROQR, 97, },
        { "Code128", BARCODE_HIBC_128, 98, },
        { "Code39", BARCODE_HIBC_39, 99, },
        { "", -1, 100, },
        { "", -1, 101, },
        { "DataMatrix", BARCODE_HIBC_DM, 102, },
        { "", -1, 103, },
        { "QRCode", BARCODE_HIBC_QR, 104, },
        { "", -1, 105, },
        { "PDF417", BARCODE_HIBC_PDF, 106, },
        { "", -1, 107, },
        { "MicroPDF417", BARCODE_HIBC_MICPDF, 108, },
        { "", -1, 109, },
        { "CodablockF", BARCODE_HIBC_BLOCKF, 110, },
        { "", -1, 111, },
        { "Aztec", BARCODE_HIBC_AZTEC, 112, },
        { "", -1, 113, },
        { "", -1, 114, },
        { "DotCode", BARCODE_DOTCODE, 115, },
        { "HanXin", BARCODE_HANXIN, 116, },
        { "", -1, 117, },
        { "", -1, 118, },
        { "DataMatrix", BARCODE_MAILMARK_2D, 119, },
        { "Code128", BARCODE_UPU_S10, 120, },
        { "", BARCODE_MAILMARK_4S, 121, },
        { "", -1, 122, },
        { "", -1, 123, },
        { "", -1, 124, },
        { "", -1, 125, },
        { "", -1, 126, },
        { "", -1, 127, },
        { "", BARCODE_AZRUNE, 128, },
        { "", BARCODE_CODE32, 129, }, /* Code39 based */
        { "", BARCODE_EANX_CC, 130, },
        { "", BARCODE_GS1_128_CC, 131, },
        { "", BARCODE_DBAR_OMN_CC, 132, },
        { "", BARCODE_DBAR_LTD_CC, 133, },
        { "", BARCODE_DBAR_EXP_CC, 134, },
        { "", BARCODE_UPCA_CC, 135, },
        { "", BARCODE_UPCE_CC, 136, },
        { "", BARCODE_DBAR_STK_CC, 137, },
        { "", BARCODE_DBAR_OMNSTK_CC, 138, },
        { "", BARCODE_DBAR_EXPSTK_CC, 139, },
        { "", BARCODE_CHANNEL, 140, },
        { "", BARCODE_CODEONE, 141, },
        { "", BARCODE_GRIDMATRIX, 142, },
        { "QRCode", BARCODE_UPNQR, 143, },
        { "", BARCODE_ULTRA, 144, },
        { "RMQRCode", BARCODE_RMQR, 145, },
        { "", BARCODE_BC412, 146, },
        { "DXFilmEdge", BARCODE_DXFILMEDGE, 147, },
    };
    const int data_size = ARRAY_SIZE(data);

    const int symbology = symbol->symbology;

    if (symbology < 0 || symbology >= data_size) {
        fprintf(stderr, "testUtilZXingCPPName: unknown symbology (%d)\n", symbology);
        abort();
    }
    /* Self-check */
    if (data[symbology].val != symbology || (data[symbology].define != -1 && data[symbology].define != symbology)) {
        fprintf(stderr, "testUtilZXingCPPName: data table out of sync (%d)\n", symbology);
        abort();
    }
    if (data[symbology].name[0] == '\0') {
        if (debug & ZINT_DEBUG_TEST_PRINT) {
            printf("i:%d %s no ZXint-C++ mapping\n", index, testUtilBarcodeName(symbology));
        }
        return NULL;
    }

    if (symbology == BARCODE_QRCODE || symbology == BARCODE_HIBC_QR || symbology == BARCODE_MICROQR
            || symbology == BARCODE_RMQR) {
        const int full_multibyte = (symbol->option_3 & 0xFF) == ZINT_FULL_MULTIBYTE;
        if (full_multibyte && testUtilHasNonASCII(source, length)) { /* TODO: Support in ZXing-C++ */
            printf("i:%d %s not ZXing-C++ compatible, ZINT_FULL_MULTIBYTE not supported (with non-ASCII data)\n",
                    index, testUtilBarcodeName(symbology));
            return NULL;
        }
    } else if (symbology == BARCODE_CODABLOCKF || symbology == BARCODE_HIBC_BLOCKF) {
        if (symbol->rows == 1) { /* Single row i.e. CODE128 not supported */
            if (debug & ZINT_DEBUG_TEST_PRINT) {
                printf("i:%d %s not ZXing-C++ compatible, single row not supported\n",
                        index, testUtilBarcodeName(symbology));
            }
            return NULL;
        }
    } else if ((ZBarcode_Cap(symbology, ZINT_CAP_EANUPC) & ZINT_CAP_EANUPC) == ZINT_CAP_EANUPC) {
        if (symbology == BARCODE_EANX || symbology == BARCODE_EANX_CHK) {
            if (length < 9) {
                if (length < 6) {
                    printf("i:%d %s not ZXing-C++ compatible, EAN-5/EAN-2 not supported\n",
                            index, testUtilBarcodeName(symbology));
                    return NULL;
                }
                return "EAN-8";
            }
            if (strchr(source, '+') != NULL && length < 15) {
                return "EAN-8";
            }
        }
    }

    return data[symbology].name;
}

/* Whether can use ZXing-C++ to check a symbology with given options */
int testUtilCanZXingCPP(int index, const struct zint_symbol *symbol, const char *source, const int length,
            const int debug) {
    return testUtilZXingCPPName(index, symbol, source, length, debug) != NULL;
}

/* Run "zxingcppdecoder", returning result in `buffer` */
int testUtilZXingCPP(int index, struct zint_symbol *symbol, const char *source, const int length, char *bits,
            char *buffer, const int buffer_size, int *p_cmp_len) {
    static const char cmd_fmt[] = "zxingcppdecoder -textonly -format %s -width %d -bits '%s'";
    static const char opts_cmd_fmt[] = "zxingcppdecoder -textonly -format %s -opts '%s' -width %d -bits '%s'";
    static const char cs_cmd_fmt[] = "zxingcppdecoder -textonly -format %s -charset %s -width %d -bits '%s'";

    const int bits_len = (int) strlen(bits);
    const int width = symbol->width;
    const int symbology = symbol->symbology;
    char *cmd = (char *) z_alloca(bits_len + 1024);
    const char *zxingcpp_barcode = NULL;
    const int data_mode = (symbol->input_mode & 0x07) == DATA_MODE;
    int set_charset = 0;
    const char *opts = NULL;

    FILE *fp = NULL;
    int cnt;

    buffer[0] = '\0';

    zxingcpp_barcode = testUtilZXingCPPName(index, symbol, source, length, 0 /*debug*/);
    if (!zxingcpp_barcode) {
        fprintf(stderr, "i:%d testUtilZXingCPP: no mapping for %s\n", index, testUtilBarcodeName(symbology));
        return -1;
    }

    if (symbology == BARCODE_EXCODE39) {
        if (symbol->option_2 == 1) {
            opts = "tryCode39ExtendedMode,validateCode39CheckSum";
        } else {
            opts = "tryCode39ExtendedMode";
        }
    } else if ((symbology == BARCODE_CODE39 || symbology == BARCODE_LOGMARS) && symbol->option_2 == 1) {
        opts = "validateCode39CheckSum";
    }

    if ((symbol->input_mode & 0x07) == UNICODE_MODE && symbol->eci == 0
            && (symbology == BARCODE_QRCODE || symbology == BARCODE_MICROQR || symbology == BARCODE_HANXIN)) {
        int converted_len = length;
        unsigned char *converted_buf = (unsigned char *) z_alloca(converted_len + 1);
        if (symbology == BARCODE_HANXIN) {
            set_charset = utf8_to_eci(0, (const unsigned char *) source, converted_buf, &converted_len) != 0;
        } else {
            set_charset = utf8_to_eci(0, (const unsigned char *) source, converted_buf, &converted_len) == 0;
        }
    }
    if (set_charset) {
        const char *charset;
        if (symbology == BARCODE_HANXIN) {
            charset = "GB18030";
        } else {
            charset = "ISO8859_1";
        }
        sprintf(cmd, cs_cmd_fmt, zxingcpp_barcode, charset, width, bits);
    } else if (opts) {
        sprintf(cmd, opts_cmd_fmt, zxingcpp_barcode, opts, width, bits);
    } else {
        sprintf(cmd, cmd_fmt, zxingcpp_barcode, width, bits);
    }

    if (symbol->debug & ZINT_DEBUG_TEST_PRINT) {
        printf("i:%d testUtilZXingCPP: cmd %s\n", index, cmd);
    }

    fp = testutil_popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "i:%d testUtilZXingCPP: failed to run '%s'\n", index, cmd);
        return -1;
    }

    cnt = (int) fread(buffer, 1, buffer_size, fp);
    if (cnt == buffer_size) {
        fprintf(stderr, "i:%d testUtilZXingCPP: buffer too small, %d bytes, cnt %d (%s)\n",
                index, buffer_size, cnt, cmd);
        testutil_pclose(fp);
        return -1;
    }

    if (fgetc(fp) != EOF) {
        fprintf(stderr, "i:%d testUtilZXingCPP: failed to read full stream (%s)\n", index, cmd);
        testutil_pclose(fp);
        return -1;
    }

    testutil_pclose(fp);

    if ((data_mode && is_eci_convertible(symbol->eci)) || symbol->eci >= 899) {
        const int eci = symbol->eci >= 899 ? 3 : symbol->eci;
        int error_number;
        const int eci_length = get_eci_length(eci, (const unsigned char *) buffer, cnt);
        unsigned char *preprocessed = (unsigned char *) z_alloca(eci_length + 1);

        if (eci_length >= buffer_size) {
            fprintf(stderr, "i:%d testUtilZXingCPP: buffer too small, %d bytes, eci_length %d (%s)\n",
                    index, buffer_size, eci_length, cmd);
            return -1;
        }
        error_number = utf8_to_eci(eci, (const unsigned char *) buffer, preprocessed, &cnt);
        if (error_number == 0) {
            memcpy(buffer, preprocessed, cnt);
        } else {
            if (eci != 0 && symbol->eci < 899) {
                fprintf(stderr, "i:%d testUtilZXingCPP: utf8_to_eci(%d) == %d (%s)\n", index, eci, error_number, cmd);
                return -1;
            } else {
                int i;
                unsigned int *vals = (unsigned int *) z_alloca(sizeof(int) * (cnt + 1));
                error_number = utf8_to_unicode(symbol, (const unsigned char *) buffer, vals, &cnt, 1);
                if (error_number != 0) {
                    fprintf(stderr, "i:%d testUtilZXingCPP: utf8_to_unicode == %d (%s)\n", index, error_number, cmd);
                    return -1;
                }
                for (i = 0; i < cnt; i++) {
                    buffer[i] = (char) vals[i];
                }
                buffer[cnt] = '\0';
            }
        }
    }

    *p_cmp_len = cnt;

    return 0;
}

INTERNAL int escape_char_process_test(struct zint_symbol *symbol, const unsigned char *input_string, int *length,
                unsigned char *escaped_string);

#include "../gs1.h"

static const char TECHNETIUM[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%"; /* Same as SILVER (CODE39) */

/* Helper to strip leading zeroes (as long as have at least one non-zero digit) */
static const char *testUtilZXingCPPLeadingZeroes(const char *expected) {
    const char *stripped = expected;
    while (*stripped == '0') stripped++;
    return z_isdigit(*stripped) ? stripped : expected;
}

/* Helper to convert DX number from "NNNN"/"NNNNNN" format to "NNN-NN" format */
static int textUtilZXingCPPDX(const char *expected, const int expected_len, const char *cmp_buf, char *out) {
    if (strchr(cmp_buf, '-')) {
        const char *stripped;
        if (strchr(expected, '-') == NULL) {
            if (expected_len == 6) {
                const int dx = to_int((const unsigned char *) expected + 1, expected_len - 2);
                sprintf(out, "%d-%d", dx / 16, dx % 16);
            } else {
                const int dx = to_int((const unsigned char *) expected, expected_len);
                sprintf(out, "%d-%d", dx / 16, dx % 16);
            }
            return 1;
        }
        if ((stripped = testUtilZXingCPPLeadingZeroes(expected)) != expected) {
            memcpy(out, stripped, expected_len - (stripped - expected));
            out[expected_len - (stripped - expected)] = '\0';
            return 1;
        }
    }
    return 0;
}

/* Massage result from "zxingcppdecoder" so as can compare to Zint input */
int testUtilZXingCPPCmp(struct zint_symbol *symbol, char *msg, char *cmp_buf, int cmp_len,
            const char *expected, int expected_len, const char *primary, char *ret_buf, int *p_ret_len) {
    const int symbology = symbol->symbology;

    const int is_gs1_128_dbar_exp = symbology == BARCODE_GS1_128 || symbology == BARCODE_DBAR_EXP
                                || symbology == BARCODE_DBAR_EXPSTK;
    const int gs1 = (symbol->input_mode & 0x07) == GS1_MODE || is_gs1_128_dbar_exp;
    const int is_extra_escaped = (symbol->input_mode & EXTRA_ESCAPE_MODE) && symbol->symbology == BARCODE_CODE128;
    const int is_escaped = (symbol->input_mode & ESCAPE_MODE) || is_extra_escaped;
    const int is_hibc = symbology >= BARCODE_HIBC_128 && symbology <= BARCODE_HIBC_AZTEC;
    const int have_c25checkdigit = symbol->option_2 == 1 || symbol->option_2 == 2;
    const int have_c25inter = (symbology == BARCODE_C25INTER && ((expected_len & 1) || have_c25checkdigit))
                                || symbology == BARCODE_ITF14 || symbology == BARCODE_DPLEIT
                                || symbology == BARCODE_DPIDENT;
    const int is_upcean = (ZBarcode_Cap(symbology, ZINT_CAP_EANUPC) & ZINT_CAP_EANUPC) == ZINT_CAP_EANUPC;
    const int need_dpd_prefix = (symbology == BARCODE_DPD && expected_len == 27 && symbol->option_2 != 1);
    const int is_vin_international = symbology == BARCODE_VIN && (symbol->option_2 & 1);

    char *reduced = gs1 ? (char *) z_alloca(expected_len + 1) : NULL;
    char *escaped = is_escaped ? (char *) z_alloca(expected_len + 1) : NULL;
    char *hibc = is_hibc ? (char *) z_alloca(expected_len + 2 + 1) : NULL;
    char *maxi = symbology == BARCODE_MAXICODE && primary
                    ? (char *) z_alloca(expected_len + strlen(primary) + 4 + 6 + 9 + 1) : NULL;
    char *vin = is_vin_international ? (char *) z_alloca(expected_len + 1 + 1) : NULL;
    char *c25inter = have_c25inter ? (char *) z_alloca(expected_len + 13 + 1 + 1) : NULL;
    char *upcean = is_upcean ? (char *) z_alloca(expected_len + 1 + 1) : NULL;
    char *ean14_nve18 = symbology == BARCODE_EAN14 || symbology == BARCODE_NVE18
                        ? (char *) z_alloca(expected_len + 3 + 19 + 1) : NULL;
    char *dpd = need_dpd_prefix ? (char *) z_alloca(28 + 1) : NULL;
    char *pzn = symbology == BARCODE_PZN ? (char *) z_alloca(expected_len + 1 + 1) : NULL;
    char *dxfe = symbology == BARCODE_DXFILMEDGE ? (char *) z_alloca(expected_len * 2 + 1) : NULL;

    int ret;
    int ret_memcmp;
    int i;

    if (ret_buf) {
        ret_buf[0] = '\0';
    }
    if (p_ret_len) {
        *p_ret_len = 0;
    }

    if (is_escaped) {
        ret = escape_char_process_test(symbol, (unsigned char *) expected, &expected_len,
                                        (unsigned char *) escaped);
        if (ret != 0) {
            sprintf(msg, "escape_char_process %d != 0", ret);
            return 3;
        }
        if (is_extra_escaped) {
            /* Remove any Code 128 special escapes */
            int j = 0;
            int have_manual_ab = 0;
            for (i = 0; i < expected_len; i++) {
                if (escaped[i] == '\\' && i + 2 < expected_len && escaped[i + 1] == '^'
                        && ((escaped[i + 2] >= '@' && escaped[i + 2] <= 'C') || escaped[i + 2] == '1'
                            || escaped[i + 2] == '^')) {
                    if (escaped[i + 2] != '^') {
                        i += 2;
                        if (escaped[i] == 'A' || escaped[i] == 'B') {
                            have_manual_ab = 1; /* Hack to help guess if in Code Set C for AIM exception below */
                        } else if (escaped[i] == '1') {
                            /* FNC1 in 1st position treated as GS1 and in 2nd position AIM, neither transmitted -
                               need to skip AIM (single alphabetic or Code Set C double digit)
                               TODO: guessing about whether in Code Set C for double digit */
                            if (j > 2 || (j == 1 && !(z_isupper(escaped[0]) || z_islower(escaped[0])))
                                    || (j == 2 && !(z_isdigit(escaped[0]) && z_isdigit(escaped[1])
                                                    && !have_manual_ab))) {
                                /* Probably not AIM */
                                escaped[j++] = 29; /* GS */
                            }
                        }
                    } else {
                        escaped[j++] = escaped[i++];
                        escaped[j++] = escaped[i++];
                    }
                } else {
                    escaped[j++] = escaped[i];
                }
            }
            expected_len = j;
            escaped[expected_len] = '\0';
        }
        expected = escaped;
    }
    if (gs1 && symbology != BARCODE_EAN14 && symbology != BARCODE_NVE18) {
        ret = gs1_verify(symbol, (const unsigned char *) expected, expected_len, (unsigned char *) reduced);
        if (ret >= ZINT_ERROR) {
            sprintf(msg, "gs1_verify %d != 0", ret);
            return 4;
        }
        expected_len = (int) strlen(reduced);
        expected = reduced;
        if (primary) {
            /* TODO: */
        }
    } else if (is_hibc) {
        int counter;
        int posns[110];
        hibc[0] = '+';
        memcpy(hibc + 1, expected, expected_len);
        to_upper((unsigned char *) (hibc + 1), expected_len);
        if (not_sane_lookup(TECHNETIUM, sizeof(TECHNETIUM) - 1, (unsigned char *) (hibc + 1), expected_len, posns)) {
            sprintf(msg, "HIBC not_sane_lookup(TECHNETIUM) failed");
            return 5;
        }
        counter = 41;
        for (i = 0; i < expected_len && i < 110; i++) {
            counter += posns[i];
        }
        counter = counter % 43;
        hibc[++expected_len] = TECHNETIUM[counter];
        hibc[++expected_len] = '\0';
        expected = hibc;
    }
    if (symbology == BARCODE_MAXICODE) {
        if (primary && primary[0]) {
            int primary_len = (int) strlen(primary);
            int maxi_len = 0;
            int have_manual_scm = 0;
            if (symbol->option_2 >= 1 && symbol->option_2 <= 100) {
/* Suppress gcc warning null destination pointer [-Wformat-overflow=] false-positive */
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 7
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-overflow="
#endif
                sprintf(maxi, "[)>\03601\035%02d", symbol->option_2 - 1);
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 7
#pragma GCC diagnostic pop
#endif
                maxi_len = (int) strlen(maxi);
            } else if (expected_len >= 9 && strncmp(expected, "[)>\03601\035", 7) == 0
                        && z_isdigit(expected[7]) && z_isdigit(expected[8])) {
                have_manual_scm = 1;
            }
            if (primary[0] > '9') {
                sprintf(maxi + maxi_len, "%-6.*s\035%.*s\035%.*s\035", primary_len - 6, primary,
                        3, primary + primary_len - 6, 3, primary + primary_len - 3);
            } else {
                if (primary_len == 11 && primary[5] == '8' && primary[6] == '4' && primary[7] == '0') {
                    sprintf(maxi + maxi_len, "%.*s0000\035%.*s\035%.*s\035", primary_len - 6, primary,
                            3, primary + primary_len - 6, 3, primary + primary_len - 3);
                } else {
                    sprintf(maxi + maxi_len, "%.*s\035%.*s\035%.*s\035", primary_len - 6, primary,
                            3, primary + primary_len - 6, 3, primary + primary_len - 3);
                }
            }
            maxi_len = (int) strlen(maxi);
            if (have_manual_scm) {
                memmove(maxi + 9, maxi, maxi_len);
                memcpy(maxi, expected, 9);
                memcpy(maxi + maxi_len + 9, expected + 9, expected_len - 9);
            } else {
                memcpy(maxi + maxi_len, expected, expected_len);
            }
            expected = maxi;
            expected_len += maxi_len;
            maxi[expected_len] = '\0';
        }
    } else if (symbology == BARCODE_CODABAR) {
        /* Ignore start A/B/C/D and stop A/B/C/D chars to avoid upper/lowercase issues */
        cmp_buf++;
        cmp_len -= 2;
        expected++;
        expected_len -= 2;
        if (symbol->option_2 == 1 || symbol->option_2 == 2) {
            cmp_len--; /* Too messy to calc the check digit so ignore */
        }
    } else if (is_vin_international) {
        vin[0] = 'I';
        memcpy(vin + 1, expected, expected_len);
        vin[++expected_len] = '\0';
        expected = vin;
    } else if (have_c25inter) {
        if (symbology == BARCODE_C25INTER) {
            if ((expected_len & 1) || have_c25checkdigit) {
                if (((expected_len & 1) && !have_c25checkdigit) || (!(expected_len & 1) && have_c25checkdigit)) {
                    c25inter[0] = '0';
                    memcpy(c25inter + 1, expected, expected_len);
                    expected_len++;
                } else {
                    memcpy(c25inter, expected, expected_len);
                }
                if (have_c25checkdigit) {
                    c25inter[expected_len] = gs1_check_digit((const unsigned char *) c25inter, expected_len);
                    expected_len++;
                }
                c25inter[expected_len] = '\0';
                expected = c25inter;
            }
        } else if (symbology == BARCODE_DPLEIT || symbology == BARCODE_DPIDENT) {
            const int len = symbology == BARCODE_DPLEIT ? 13 : 11;
            int zeroes = len - expected_len;
            unsigned int count = 0;
            int factor = 4;
            for (i = 0; i < zeroes; i++) {
                c25inter[i] = '0';
            }
            memcpy(c25inter + zeroes, expected, expected_len);
            expected_len += zeroes;
            for (i = len - 1; i >= 0; i--) {
                count += factor * ctoi(c25inter[i]);
                factor ^= 0x0D; /* Toggles 4 and 9 */
            }
            c25inter[expected_len] = itoc((10 - (count % 10)) % 10);
            c25inter[++expected_len] = '\0';
            expected = c25inter;
        } else if (symbology == BARCODE_ITF14) {
            int zeroes = 13 - expected_len;
            for (i = 0; i < zeroes; i++) {
                c25inter[i] = '0';
            }
            memcpy(c25inter + zeroes, expected, expected_len);
            expected_len += zeroes;
            c25inter[expected_len] = gs1_check_digit((const unsigned char *) c25inter, 13);
            c25inter[++expected_len] = '\0';
            expected = c25inter;
        }
    } else if (symbology == BARCODE_DBAR_OMN || symbology == BARCODE_DBAR_LTD || symbology == BARCODE_DBAR_OMNSTK
                || symbology == BARCODE_DBAR_STK) {
        if (expected_len == 13) {
            cmp_len--; /* Too messy to calc the check digit so ignore */
        }
        if (symbology == BARCODE_DBAR_LTD) {
            cmp_buf += 2; /* Ignore prefixed "01" */
            cmp_len -= 2;
        }
    } else if (is_upcean) {
        if (symbology == BARCODE_UPCA && (expected_len == 11 || expected_len == 14 || expected_len == 17)) {
            memcpy(upcean, expected, 11);
            upcean[11] = gs1_check_digit((const unsigned char *) upcean, 11);
            if (expected_len == 14) {
                upcean[12] = ' ';
                memcpy(upcean + 13, expected + 12, 2);
            } else if (expected_len == 17) {
                upcean[12] = ' ';
                memcpy(upcean + 13, expected + 12, 5);
            }
            expected_len++;
            upcean[expected_len] = '\0';
            expected = upcean;
        } else if (symbology == BARCODE_UPCA_CHK && (expected_len == 15 || expected_len == 18)) {
            memcpy(upcean, expected, expected_len);
            upcean[12] = ' ';
            expected = upcean;
        } else if (symbology == BARCODE_UPCE && (expected_len == 7 || expected_len == 10 || expected_len == 13)) {
            char equivalent[11];
            memcpy(upcean, expected, 7);
            memcpy(equivalent, upcean, 3);
            memset(equivalent + 3, '0', 8);
            switch (upcean[6]) {
                case '0': case '1': case '2':
                    equivalent[3] = upcean[6];
                    equivalent[8] = upcean[3];
                    equivalent[9] = upcean[4];
                    equivalent[10] = upcean[5];
                    break;
                case '3':
                    equivalent[3] = upcean[3];
                    equivalent[9] = upcean[4];
                    equivalent[10] = upcean[5];
                    break;
                case '4':
                    equivalent[3] = upcean[3];
                    equivalent[4] = upcean[4];
                    equivalent[10] = upcean[5];
                    break;
                case '5': case '6': case '7': case '8': case '9':
                    equivalent[3] = upcean[3];
                    equivalent[4] = upcean[4];
                    equivalent[5] = upcean[5];
                    equivalent[10] = upcean[6];
                    break;
            }
            upcean[7] = gs1_check_digit((const unsigned char *) equivalent, 11);
            if (expected_len == 10) {
                upcean[8] = ' ';
                memcpy(upcean + 9, expected + 8, 2);
            } else if (expected_len == 13) {
                upcean[8] = ' ';
                memcpy(upcean + 9, expected + 8, 5);
            }
            expected_len++;
            upcean[expected_len] = '\0';
            expected = upcean;
        } else if (symbology == BARCODE_UPCE_CHK && (expected_len == 11 || expected_len == 14)) {
            memcpy(upcean, expected, expected_len);
            upcean[8] = ' ';
            expected = upcean;
        } else if (symbology == BARCODE_EANX && (expected_len == 12 || expected_len == 15 || expected_len == 18)) {
            memcpy(upcean, expected, 12);
            upcean[12] = gs1_check_digit((const unsigned char *) upcean, 12);
            if (expected_len == 15) {
                upcean[13] = ' ';
                memcpy(upcean + 14, expected + 13, 2);
            } else if (expected_len == 18) {
                upcean[13] = ' ';
                memcpy(upcean + 14, expected + 13, 5);
            }
            expected_len++;
            upcean[expected_len] = '\0';
            expected = upcean;
        } else if (symbology == BARCODE_EANX && (expected_len == 16 || expected_len == 19)) {
            memcpy(upcean, expected, expected_len);
            upcean[13] = ' ';
            expected = upcean;
        } else if (symbology == BARCODE_EANX && (expected_len == 7
                || (strchr(expected, '+') != NULL && (expected_len == 10 || expected_len == 13)))) {
            memcpy(upcean, expected, 7);
            upcean[7] = gs1_check_digit((const unsigned char *) upcean, 7);
            if (expected_len == 10) {
                upcean[8] = ' ';
                memcpy(upcean + 9, expected + 8, 2);
            } else if (expected_len == 13) {
                upcean[8] = ' ';
                memcpy(upcean + 9, expected + 8, 5);
            }
            expected_len++;
            upcean[expected_len] = '\0';
            expected = upcean;
        } else if ((symbology == BARCODE_EANX_CHK || symbology == BARCODE_ISBNX)
                && (expected_len == 16 || expected_len == 19)) {
            memcpy(upcean, expected, expected_len);
            upcean[13] = ' ';
            expected = upcean;
        } else if (symbology == BARCODE_EANX_CHK && strchr(expected, '+') != NULL
                && (expected_len == 11 || expected_len == 14)) {
            memcpy(upcean, expected, expected_len);
            upcean[8] = ' ';
            expected = upcean;
        }

    } else if (symbology == BARCODE_EAN14 || symbology == BARCODE_NVE18) {
        int len = symbology == BARCODE_NVE18 ? 17 : 13;
        int zeroes = expected_len < len ? len - expected_len: 0;
        ean14_nve18[0] = '0';
        ean14_nve18[1] = symbology == BARCODE_NVE18 ? '0' : '1';
        memset(ean14_nve18 + 2, '0', zeroes);
        memcpy(ean14_nve18 + 2 + zeroes, expected, expected_len);
        ean14_nve18[len + 2] = gs1_check_digit((unsigned char *) (ean14_nve18 + 2), len);
        expected = ean14_nve18;
        expected_len += zeroes + 3;

    } else if (need_dpd_prefix) {
        dpd[0] = '%';
        memcpy(dpd + 1, expected, expected_len);
        expected = dpd;
        expected_len++;

    } else if (symbology == BARCODE_PZN) {
        /* Add hyphen at start */
        pzn[0] = '-';
        memcpy(pzn + 1, expected, expected_len);
        if ((symbol->option_2 == 0 && expected_len != 8) || (symbol->option_2 == 1 && expected_len != 7)) {
            cmp_len--; /* Don't bother with check digit */
        }
        expected = pzn;
        expected_len++;

    } else if (symbology == BARCODE_DXFILMEDGE) {
        const int dx_info_len = posn(expected, '/');
        if (dx_info_len != -1) {
            char frame_info[20];
            assert(strlen(expected + dx_info_len + 1) < sizeof(frame_info));
            strcpy(frame_info, expected + dx_info_len + 1);
            to_upper((unsigned char *) frame_info, (int) strlen(frame_info));
            if (!textUtilZXingCPPDX(expected, dx_info_len, cmp_buf, dxfe)) {
                memcpy(dxfe, expected, dx_info_len);
                dxfe[dx_info_len] = '\0';
            }
            if (strcmp(frame_info, "S") == 0 || strcmp(frame_info, "X") == 0) {
                strcat(dxfe, "/62");
            } else if (strcmp(frame_info, "SA") == 0 || strcmp(frame_info, "XA") == 0) {
                strcat(dxfe, "/62A");
            } else if (strcmp(frame_info, "K") == 0 || strcmp(frame_info, "00") == 0) {
                strcat(dxfe, "/63");
            } else if (strcmp(frame_info, "KA") == 0 || strcmp(frame_info, "00A") == 0) {
                strcat(dxfe, "/63A");
            } else if (strcmp(frame_info, "F") == 0) {
                strcat(dxfe, "/0");
            } else if (strcmp(frame_info, "FA") == 0) {
                strcat(dxfe, "/0A");
            } else {
                const char *stripped;
                if ((stripped = testUtilZXingCPPLeadingZeroes(frame_info)) != frame_info) {
                    strcat(dxfe, "/");
                    strcat(dxfe, stripped);
                } else {
                    strcat(dxfe, expected + dx_info_len);
                }
            }
            expected = dxfe;
            expected_len = (int) strlen(expected);
            to_upper((unsigned char *) expected, expected_len);
        } else {
            if (textUtilZXingCPPDX(expected, expected_len, cmp_buf, dxfe)) {
                expected = dxfe;
                expected_len = (int) strlen(expected);
            }
        }
    }

    if (ret_buf) {
        memcpy(ret_buf, expected, expected_len);
    }
    if (p_ret_len) {
        *p_ret_len = expected_len;
    }

    if (cmp_len != expected_len) {
        sprintf(msg, "cmp_len %d != expected_len %d", cmp_len, expected_len);
        return 2;
    }
    ret_memcmp = memcmp(cmp_buf, expected, expected_len);
    if (ret_memcmp != 0) {
        for (i = 0; i < expected_len; i++) {
            if (cmp_buf[i] != expected[i]) {
                break;
            }
        }
        sprintf(msg, "memcmp %d != 0, at %d, len %d", ret_memcmp, i, expected_len);
        return ret_memcmp;
    }

    return 0;
}

int testUtilZXingCPPCmpSegs(struct zint_symbol *symbol, char *msg, char *cmp_buf, int cmp_len,
            const struct zint_seg segs[], const int seg_count, const char *primary, char *ret_buf, int *p_ret_len) {
    int expected_len = segs_length(segs, seg_count);
    char *expected = (char *) z_alloca(expected_len + 1);
    char *s = expected;
    int i;

    for (i = 0; i < seg_count; i++) {
        int len = segs[i].length == -1 ? (int) ustrlen(segs[i].source) : segs[i].length;
        memcpy(s, segs[i].source, len);
        s += len;
    }
    *s = '\0';

    return testUtilZXingCPPCmp(symbol, msg, cmp_buf, cmp_len, expected, expected_len, primary, ret_buf, p_ret_len);
}

/* vim: set ts=4 sw=4 et : */
