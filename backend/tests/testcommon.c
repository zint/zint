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
 * Adapted from qrencode/tests/common.c
 * Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
 */

#include "testcommon.h"

#ifdef _MSC_VER
#include <malloc.h>
#define testutil_alloca(nmemb) _alloca(nmemb)
#else
#define testutil_alloca(nmemb) alloca(nmemb)
#endif

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

#include "../eci.h"
#ifndef NO_PNG
#include <png.h>
#include <zlib.h>
#include <setjmp.h>
#endif
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <sys/stat.h>

static int tests = 0;
static int failed = 0;
static int skipped = 0;
int assertionFailed = 0;
int assertionNum = 0;
const char *assertionFilename = "";
static const char *testName = NULL;
static const char *testFunc = NULL;

/* Visual C++ 6 doesn't support variadic args to macros, so make do with functions, which have inferior behaviour,
   e.g. don't exit on failure, `assert_equal()` type-specific */
#if _MSC_VER == 1200 /* VC6 */
#include <stdarg.h>
void assert_zero(int exp, const char *fmt, ...) {
    assertionNum++;
    if (exp != 0) {
        va_list args; assertionFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
    }
}
void assert_nonzero(int exp, const char *fmt, ...) {
    assertionNum++;
    if (exp == 0) {
        va_list args; assertionFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
    }
}
void assert_null(void *exp, const char *fmt, ...) {
    assertionNum++;
    if (exp != NULL) {
        va_list args; assertionFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
    }
}
void assert_nonnull(void *exp, const char *fmt, ...) {
    assertionNum++;
    if (exp == NULL) {
        va_list args; assertionFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
    }
}
void assert_equal(int e1, int e2, const char *fmt, ...) {
    assertionNum++;
    if (e1 != e2) {
        va_list args; assertionFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
    }
}
void assert_equalu64(uint64_t e1, uint64_t e2, const char *fmt, ...) {
    assertionNum++;
    if (e1 != e2) {
        va_list args; assertionFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
    }
}
void assert_notequal(int e1, int e2, const char *fmt, ...) {
    assertionNum++;
    if (e1 == e2) {
        va_list args; assertionFailed++; va_start(args, fmt); vprintf(fmt, args); va_end(args); testFinish();
    }
}
#endif

/* Begin individual test function */
void testStartReal(const char *func, const char *name) {
    tests++;
    if (func && *func && name && *name && strcmp(func, name) == 0) {
        testName = "";
    } else {
        testName = name;
    }
    testFunc = func ? func : "";
    assertionFailed = 0;
    assertionNum = 0;
    printf("_____%d: %s: %s...\n", tests, testFunc, testName ? testName : "");
}

/* End individual test function */
void testFinish(void) {
    if (testName && *testName) {
        printf(".....%d: %s: %s ", tests, testFunc, testName);
    } else {
        printf(".....%d: %s: ", tests, testFunc);
    }
    if (assertionFailed) {
        printf("FAILED. (%d assertions failed.)\n", assertionFailed);
        failed++;
    } else {
        printf("PASSED. (%d assertions passed.)\n", assertionNum);
    }
}

/* Skip (and end) individual test function */
void testSkip(const char *msg) {
    skipped++;
    if (testName && *testName) {
        printf(".....%d: %s: %s ", tests, testFunc, testName);
    } else {
        printf(".....%d: %s: ", tests, testFunc);
    }
    if (assertionFailed) {
        printf("FAILED. (%d assertions failed.)\n", assertionFailed);
        failed++;
    } else {
        printf("SKIPPED. %s. (%d assertions passed.)\n", msg, assertionNum);
    }
}

/* End test program */
void testReport() {
    if (failed && skipped) {
        printf("Total %d tests, %d skipped, %d fails.\n", tests, skipped, failed);
        exit(-1);
    }
    if (failed) {
        printf("Total %d tests, %d fails.\n", tests, failed);
        exit(-1);
    }
    if (skipped) {
        printf("Total %d tests, %d skipped.\n", tests, skipped);
    } else if (tests) {
        printf("Total %d tests, all passed.\n", tests);
    } else {
        printf("Total %d tests.\n", tests);
    }
}

/* Begin test program, parse args */
void testRun(int argc, char *argv[], testFunction funcs[], int funcs_size) {
    int i, opt, ran;
    long long_opt;
    char *optarg_endptr = NULL;
    int debug = 0;
    char *func = NULL;
    char func_buf[256 + 5];
    int index = -1;
    int generate = 0;

    typedef void (*func_void)(void);
    typedef void (*func_debug)(int debug);
    typedef void (*func_index)(int index);
    typedef void (*func_index_debug)(int index, int debug);
    typedef void (*func_generate)(int generate);
    typedef void (*func_generate_debug)(int generate, int debug);
    typedef void (*func_index_generate)(int index, int generate);
    typedef void (*func_index_generate_debug)(int index, int generate, int debug);

    if (argc) {
        char *filename = strrchr(argv[0], '/');
#ifdef _WIN32
        if (filename == NULL) {
            filename = strrchr(argv[0], '\\');
        }
#endif
        if (filename) {
            assertionFilename = filename + 1;
        } else {
            assertionFilename = argv[0];
        }
    }

    while ((opt = getopt(argc, argv, "d:f:gi:")) != -1) {
        switch (opt) {
            case 'd':
                errno = 0;
                long_opt = strtol(optarg, &optarg_endptr, 10);
                if (errno || optarg_endptr == optarg || long_opt < 0 || long_opt > INT_MAX) {
                    fprintf(stderr, "testRun: -d debug value invalid\n");
                    debug = 0;
                } else {
                    debug = long_opt;
                }
                break;
            case 'f':
                if (strlen(optarg) < 256) {
                    if (strncmp(optarg, "test_", 5) == 0) {
                        strcpy(func_buf, optarg);
                    } else {
                        strcpy(func_buf, "test_");
                        strcat(func_buf, optarg);
                    }
                    func = func_buf;
                } else {
                    fprintf(stderr, "testRun: -f func value too long\n");
                    func = NULL;
                }
                break;
            case 'g':
                generate = 1;
                break;
            case 'i':
                errno = 0;
                long_opt = strtol(optarg, &optarg_endptr, 10);
                if (errno || optarg_endptr == optarg || long_opt < 0 || long_opt > INT_MAX) {
                    fprintf(stderr, "testRun: -i index value invalid\n");
                    index = -1;
                } else {
                    index = long_opt;
                }
                break;
        }
    }

    ran = 0;
    for (i = 0; i < funcs_size; i++) {
        if (func && strcmp(func, funcs[i].name) != 0) {
            continue;
        }
        if (funcs[i].has_index && funcs[i].has_generate && funcs[i].has_debug) {
            (*(func_index_generate_debug)funcs[i].func)(index, generate, debug);
        } else if (funcs[i].has_index && funcs[i].has_generate) {
            if (debug) fprintf(stderr, "testRun %s: -d ignored\n", funcs[i].name);
            (*(func_index_generate)funcs[i].func)(index, generate);
        } else if (funcs[i].has_index && funcs[i].has_debug) {
            if (generate) fprintf(stderr, "testRun %s: -g ignored\n", funcs[i].name);
            (*(func_index_debug)funcs[i].func)(index, debug);
        } else if (funcs[i].has_index) {
            if (generate) fprintf(stderr, "testRun %s: -g ignored\n", funcs[i].name);
            if (debug) fprintf(stderr, "testRun %s: -d ignored\n", funcs[i].name);
            (*(func_index)funcs[i].func)(index);
        } else if (funcs[i].has_generate && funcs[i].has_debug) {
            if (index != -1) fprintf(stderr, "testRun %s: -i index ignored\n", funcs[i].name);
            (*(func_generate_debug)funcs[i].func)(generate, debug);
        } else if (funcs[i].has_generate) {
            if (index != -1) fprintf(stderr, "testRun %s: -i index ignored\n", funcs[i].name);
            if (debug) fprintf(stderr, "testRun %s: -d ignored\n", funcs[i].name);
            (*(func_generate)funcs[i].func)(generate);
        } else if (funcs[i].has_debug) {
            if (index != -1) fprintf(stderr, "testRun %s: -i index ignored\n", funcs[i].name);
            if (generate) fprintf(stderr, "testRun %s -g ignored\n", funcs[i].name);
            (*(func_debug)funcs[i].func)(debug);
        } else {
            if (index != -1) fprintf(stderr, "testRun %s: -i index ignored\n", funcs[i].name);
            if (generate) fprintf(stderr, "testRun %s -g ignored\n", funcs[i].name);
            if (debug) fprintf(stderr, "testRun %s: -d ignored\n", funcs[i].name);
            (*(func_void)funcs[i].func)();
        }
        ran++;
    }

    if (func && !ran) {
        fprintf(stderr, "testRun: unknown -f func arg '%s'\n", func);
    }
}

/* Helper to set common symbol fields */
int testUtilSetSymbol(struct zint_symbol *symbol, int symbology, int input_mode, int eci, int option_1, int option_2,
            int option_3, int output_options, char *data, int length, int debug) {
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
    struct item {
        const char *name;
        int define;
        int val;
    };
    static const struct item data[] = {
        { "", -1, 0 },
        { "BARCODE_CODE11", BARCODE_CODE11, 1 },
        { "BARCODE_C25STANDARD", BARCODE_C25STANDARD, 2 },
        { "BARCODE_C25INTER", BARCODE_C25INTER, 3 },
        { "BARCODE_C25IATA", BARCODE_C25IATA, 4 },
        { "", -1, 5 },
        { "BARCODE_C25LOGIC", BARCODE_C25LOGIC, 6 },
        { "BARCODE_C25IND", BARCODE_C25IND, 7 },
        { "BARCODE_CODE39", BARCODE_CODE39, 8 },
        { "BARCODE_EXCODE39", BARCODE_EXCODE39, 9 },
        { "", -1, 10 },
        { "", -1, 11 },
        { "", -1, 12 },
        { "BARCODE_EANX", BARCODE_EANX, 13 },
        { "BARCODE_EANX_CHK", BARCODE_EANX_CHK, 14 },
        { "", -1, 15 },
        { "BARCODE_GS1_128", BARCODE_GS1_128, 16 },
        { "", -1, 17 },
        { "BARCODE_CODABAR", BARCODE_CODABAR, 18 },
        { "", -1, 19 },
        { "BARCODE_CODE128", BARCODE_CODE128, 20 },
        { "BARCODE_DPLEIT", BARCODE_DPLEIT, 21 },
        { "BARCODE_DPIDENT", BARCODE_DPIDENT, 22 },
        { "BARCODE_CODE16K", BARCODE_CODE16K, 23 },
        { "BARCODE_CODE49", BARCODE_CODE49, 24 },
        { "BARCODE_CODE93", BARCODE_CODE93, 25 },
        { "", -1, 26 },
        { "", -1, 27 },
        { "BARCODE_FLAT", BARCODE_FLAT, 28 },
        { "BARCODE_DBAR_OMN", BARCODE_DBAR_OMN, 29 },
        { "BARCODE_DBAR_LTD", BARCODE_DBAR_LTD, 30 },
        { "BARCODE_DBAR_EXP", BARCODE_DBAR_EXP, 31 },
        { "BARCODE_TELEPEN", BARCODE_TELEPEN, 32 },
        { "", -1, 33 },
        { "BARCODE_UPCA", BARCODE_UPCA, 34 },
        { "BARCODE_UPCA_CHK", BARCODE_UPCA_CHK, 35 },
        { "", -1, 36 },
        { "BARCODE_UPCE", BARCODE_UPCE, 37 },
        { "BARCODE_UPCE_CHK", BARCODE_UPCE_CHK, 38 },
        { "", -1, 39 },
        { "BARCODE_POSTNET", BARCODE_POSTNET, 40 },
        { "", -1, 41 },
        { "", -1, 42 },
        { "", -1, 43 },
        { "", -1, 44 },
        { "", -1, 45 },
        { "", -1, 46 },
        { "BARCODE_MSI_PLESSEY", BARCODE_MSI_PLESSEY, 47 },
        { "", -1, 48 },
        { "BARCODE_FIM", BARCODE_FIM, 49 },
        { "BARCODE_LOGMARS", BARCODE_LOGMARS, 50 },
        { "BARCODE_PHARMA", BARCODE_PHARMA, 51 },
        { "BARCODE_PZN", BARCODE_PZN, 52 },
        { "BARCODE_PHARMA_TWO", BARCODE_PHARMA_TWO, 53 },
        { "", -1, 54 },
        { "BARCODE_PDF417", BARCODE_PDF417, 55 },
        { "BARCODE_PDF417COMP", BARCODE_PDF417COMP, 56 },
        { "BARCODE_MAXICODE", BARCODE_MAXICODE, 57 },
        { "BARCODE_QRCODE", BARCODE_QRCODE, 58 },
        { "", -1, 59 },
        { "BARCODE_CODE128B", BARCODE_CODE128B, 60 },
        { "", -1, 61 },
        { "", -1, 62 },
        { "BARCODE_AUSPOST", BARCODE_AUSPOST, 63 },
        { "", -1, 64 },
        { "", -1, 65 },
        { "BARCODE_AUSREPLY", BARCODE_AUSREPLY, 66 },
        { "BARCODE_AUSROUTE", BARCODE_AUSROUTE, 67 },
        { "BARCODE_AUSREDIRECT", BARCODE_AUSREDIRECT, 68 },
        { "BARCODE_ISBNX", BARCODE_ISBNX, 69 },
        { "BARCODE_RM4SCC", BARCODE_RM4SCC, 70 },
        { "BARCODE_DATAMATRIX", BARCODE_DATAMATRIX, 71 },
        { "BARCODE_EAN14", BARCODE_EAN14, 72 },
        { "BARCODE_VIN", BARCODE_VIN, 73 },
        { "BARCODE_CODABLOCKF", BARCODE_CODABLOCKF, 74 },
        { "BARCODE_NVE18", BARCODE_NVE18, 75 },
        { "BARCODE_JAPANPOST", BARCODE_JAPANPOST, 76 },
        { "BARCODE_KOREAPOST", BARCODE_KOREAPOST, 77 },
        { "", -1, 78 },
        { "BARCODE_DBAR_STK", BARCODE_DBAR_STK, 79 },
        { "BARCODE_DBAR_OMNSTK", BARCODE_DBAR_OMNSTK, 80 },
        { "BARCODE_DBAR_EXPSTK", BARCODE_DBAR_EXPSTK, 81 },
        { "BARCODE_PLANET", BARCODE_PLANET, 82 },
        { "", -1, 83 },
        { "BARCODE_MICROPDF417", BARCODE_MICROPDF417, 84 },
        { "BARCODE_USPS_IMAIL", BARCODE_USPS_IMAIL, 85 },
        { "BARCODE_PLESSEY", BARCODE_PLESSEY, 86 },
        { "BARCODE_TELEPEN_NUM", BARCODE_TELEPEN_NUM, 87 },
        { "", -1, 88 },
        { "BARCODE_ITF14", BARCODE_ITF14, 89 },
        { "BARCODE_KIX", BARCODE_KIX, 90 },
        { "", -1, 91 },
        { "BARCODE_AZTEC", BARCODE_AZTEC, 92 },
        { "BARCODE_DAFT", BARCODE_DAFT, 93 },
        { "", -1, 94 },
        { "", -1, 95 },
        { "BARCODE_DPD", BARCODE_DPD, 96 },
        { "BARCODE_MICROQR", BARCODE_MICROQR, 97 },
        { "BARCODE_HIBC_128", BARCODE_HIBC_128, 98 },
        { "BARCODE_HIBC_39", BARCODE_HIBC_39, 99 },
        { "", -1, 100 },
        { "", -1, 101 },
        { "BARCODE_HIBC_DM", BARCODE_HIBC_DM, 102 },
        { "", -1, 103 },
        { "BARCODE_HIBC_QR", BARCODE_HIBC_QR, 104 },
        { "", -1, 105 },
        { "BARCODE_HIBC_PDF", BARCODE_HIBC_PDF, 106 },
        { "", -1, 107 },
        { "BARCODE_HIBC_MICPDF", BARCODE_HIBC_MICPDF, 108 },
        { "", -1, 109 },
        { "BARCODE_HIBC_BLOCKF", BARCODE_HIBC_BLOCKF, 110 },
        { "", -1, 111 },
        { "BARCODE_HIBC_AZTEC", BARCODE_HIBC_AZTEC, 112 },
        { "", -1, 113 },
        { "", -1, 114 },
        { "BARCODE_DOTCODE", BARCODE_DOTCODE, 115 },
        { "BARCODE_HANXIN", BARCODE_HANXIN, 116 },
        { "", -1, 117 },
        { "", -1, 118 },
        { "", -1, 119 },
        { "", -1, 120 },
        { "BARCODE_MAILMARK", BARCODE_MAILMARK, 121 },
        { "", -1, 122 },
        { "", -1, 123 },
        { "", -1, 124 },
        { "", -1, 125 },
        { "", -1, 126 },
        { "", -1, 127 },
        { "BARCODE_AZRUNE", BARCODE_AZRUNE, 128 },
        { "BARCODE_CODE32", BARCODE_CODE32, 129 },
        { "BARCODE_EANX_CC", BARCODE_EANX_CC, 130 },
        { "BARCODE_GS1_128_CC", BARCODE_GS1_128_CC, 131 },
        { "BARCODE_DBAR_OMN_CC", BARCODE_DBAR_OMN_CC, 132 },
        { "BARCODE_DBAR_LTD_CC", BARCODE_DBAR_LTD_CC, 133 },
        { "BARCODE_DBAR_EXP_CC", BARCODE_DBAR_EXP_CC, 134 },
        { "BARCODE_UPCA_CC", BARCODE_UPCA_CC, 135 },
        { "BARCODE_UPCE_CC", BARCODE_UPCE_CC, 136 },
        { "BARCODE_DBAR_STK_CC", BARCODE_DBAR_STK_CC, 137 },
        { "BARCODE_DBAR_OMNSTK_CC", BARCODE_DBAR_OMNSTK_CC, 138 },
        { "BARCODE_DBAR_EXPSTK_CC", BARCODE_DBAR_EXPSTK_CC, 139 },
        { "BARCODE_CHANNEL", BARCODE_CHANNEL, 140 },
        { "BARCODE_CODEONE", BARCODE_CODEONE, 141 },
        { "BARCODE_GRIDMATRIX", BARCODE_GRIDMATRIX, 142 },
        { "BARCODE_UPNQR", BARCODE_UPNQR, 143 },
        { "BARCODE_ULTRA", BARCODE_ULTRA, 144 },
        { "BARCODE_RMQR", BARCODE_RMQR, 145 },
    };
    static const int data_size = ARRAY_SIZE(data);

    if (symbology < 0 || symbology >= data_size) {
        return "";
    }
    // Self-check
    if (data[symbology].val != symbology || (data[symbology].define != -1 && data[symbology].define != symbology)) {
        fprintf(stderr, "testUtilBarcodeName: data table out of sync (%d)\n", symbology);
        abort();
    }
    return data[symbology].name;
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
        { "", -1, 1 },
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
    };
    static const int data_size = ARRAY_SIZE(data);

    if (error_number < 0 || error_number >= data_size) {
        return "";
    }
    // Self-check
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
        { "ESCAPE_MODE", ESCAPE_MODE, 8 },
        { "GS1PARENS_MODE", GS1PARENS_MODE, 16 },
        { "GS1NOCHECK_MODE", GS1NOCHECK_MODE, 32 },
    };
    static const int data_size = ARRAY_SIZE(data);
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
        if (data[i].define != data[i].val) { // Self-check
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
const char *testUtilOption3Name(int option_3) {
    static char buffer[64];

    const char *name = NULL;
    unsigned int high_byte = option_3 == -1 ? 0 : (option_3 >> 8) & 0xFF;

    switch (option_3 & 0xFF) {
        case DM_SQUARE:
            name = "DM_SQUARE";
            break;
        case DM_DMRE:
            name = "DM_DMRE";
            break;
        case ZINT_FULL_MULTIBYTE:
            name = "ZINT_FULL_MULTIBYTE";
            break;
        case ULTRA_COMPRESSION:
            name = "ULTRA_COMPRESSION";
            break;
        default:
            if (option_3 != -1 && (option_3 & 0xFF) != 0) {
                fprintf(stderr, "testUtilOption3Name: unknown value (%d)\n", option_3);
                abort();
            }
            name = (option_3 & 0xFF) ? "-1" : "0";
            break;
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
        { "BARCODE_NO_ASCII", BARCODE_NO_ASCII, 1 },
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
        if (data[i].define != data[i].val) { // Self-check
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
        fprintf(stderr, "testUtilOutputOptionsName: unknown output option(s) %d (%d)\n",
                output_options & set, output_options);
        abort();
    }
    return buf;
}

/* Convert modules spanning 3 rows to DAFT equivalents */
int testUtilDAFTConvert(const struct zint_symbol *symbol, char *buffer, int buffer_size) {
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
        return FALSE;
    }
    *b = '\0';
    return TRUE;
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
char *testUtilEscape(char *buffer, int length, char *escaped, int escaped_size) {
    int i;
    unsigned char *b = (unsigned char *) buffer;
    unsigned char *be = b + length;
    int non_utf8 = !testUtilIsValidUTF8(b, length);
    int chunk = -1;

    for (i = 0; b < be && i < escaped_size; b++) {
         // For VC6-compatibility need to split literal strings into <= 2K chunks
         if (i > 2040 && i / 2040 != chunk) {
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
char *testUtilReadCSVField(char *buffer, char *field, int field_size) {
    int i;
    char *b = buffer;
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
void testUtilStrCpyRepeat(char *buffer, char *repeat, int size) {
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

    // Copy rectangles
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

    // Copy Strings
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

    // Copy Circles
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

    // Copy Hexagons
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

    // Compare rectangles
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

    // Compare strings
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

    // Compare circles
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

    // Compare hexagons
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
        while ((s2 = strstr(s + 1, "/tests")) != NULL) { // Find rightmost
            s = s2;
        }
        *s = '\0';
        len = s - buffer;
    }
    if ((s = strstr(buffer, "/backend")) != NULL) {
        while ((s2 = strstr(s + 1, "/backend")) != NULL) { // Find rightmost
            s = s2;
        }
        *s = '\0';
        len = s - buffer;
    } else if ((s = strstr(buffer, "/frontend")) != NULL) {
        while ((s2 = strstr(s + 1, "/frontend")) != NULL) { // Find rightmost
            s = s2;
        }
        *s = '\0';
        len = s - buffer;
    }
    if (cmake_src_dir == NULL && (s = strrchr(buffer, '/')) != NULL) { // Remove "build" dir
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

/* Does file exist? */
int testUtilExists(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return 0;
    }
    fclose(fp);
    return 1;
}

/* Does directory exist? (Windows compatibility) */
int testUtilDirExists(const char *dirname) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributes(dirname);
    return dwAttrib != (DWORD) -1 && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#else
    return testUtilExists(dirname);
#endif
}

/* Make a directory (Windows compatibility). Returns 0 if successful, non-zero if not */
int testUtilMkDir(const char *dirname) {
#ifdef _WIN32
    return CreateDirectory(dirname, NULL) == 0;
#else
    return mkdir(dirname, S_IRWXU);
#endif
}

/* Remove a directory (Windows compatibility). Returns 0 if successful, non-zero if not */
int testUtilRmDir(const char *dirname) {
#ifdef _WIN32
    return RemoveDirectory(dirname) == 0;
#else
    return rmdir(dirname);
#endif
}

/* Rename a file (Windows compatibility). */
int testUtilRename(const char *oldpath, const char *newpath) {
#ifdef _MSVC
    int ret = remove(newpath);
    if (ret != 0) return ret;
#endif
    return rename(oldpath, newpath);
}

/* Compare 2 PNG files */
int testUtilCmpPngs(const char *png1, const char *png2) {
    int ret = -1;
#ifdef NO_PNG
    (void)png1; (void)png2;
#else
    FILE *fp1;
    FILE *fp2;
    png_structp png_ptr1, png_ptr2;
    png_infop info_ptr1, info_ptr2;
    int width1, height1, width2, height2;
    png_byte color_type1, color_type2;
    png_byte bit_depth1, bit_depth2;
    png_bytep row1 = NULL, row2 = NULL;
    size_t rowbytes1, rowbytes2;
    int r;

    fp1 = fopen(png1, "rb");
    if (!fp1) {
        return 2;
    }
    fp2 = fopen(png2, "rb");
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
        png_set_scale_16(png_ptr1);
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
        png_set_scale_16(png_ptr2);
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

    fp1 = fopen(txt1, "r");
    if (!fp1) {
        return 2;
    }
    fp2 = fopen(txt2, "r");
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

    fp1 = fopen(bin1, "rb");
    if (!fp1) {
        return 2;
    }
    fp2 = fopen(bin2, "rb");
    if (!fp2) {
        fclose(fp1);
        return 3;
    }

    do {
        len1 = fread(buf1, 1, sizeof(buf1), fp1);
        len2 = fread(buf2, 1, sizeof(buf2), fp2);
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
    char first_line[] = "%!PS-Adobe-3.0 EPSF-3.0\n";
    char second_line_start[] = "%%Creator: Zint ";

    fp1 = fopen(eps1, "r");
    if (!fp1) {
        return 2;
    }
    fp2 = fopen(eps2, "r");
    if (!fp2) {
        fclose(fp1);
        return 3;
    }

    // Preprocess the 1st 2 lines to avoid comparing changeable Zint version in 2nd line
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
int testUtilHaveIdentify() {
    return system("magick -version " DEV_NULL) == 0;
}

/* Check raster files */
int testUtilVerifyIdentify(const char *filename, int debug) {
    char cmd[512 + 128];

    if (strlen(filename) > 512) {
        return -1;
    }
    // Verbose option does a more thorough check
    if (debug & ZINT_DEBUG_TEST_PRINT) {
        // Verbose very noisy though so for quick check just return default output
        if (debug & ZINT_DEBUG_TEST_LESS_NOISY) {
            sprintf(cmd, "magick identify %s", filename);
        } else {
            sprintf(cmd, "magick identify -verbose %s", filename);
        }
    } else {
        sprintf(cmd, "magick identify -verbose %s " DEV_NULL, filename);
    }

    return system(cmd);
}

/* Whether Libre Office available on system */
int testUtilHaveLibreOffice() {
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

    fp = fopen(svg, "r");
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
    remove(svg);

    return 0;
}

#ifdef _WIN32
#define GS_FILENAME "gswin64c"
#else
#define GS_FILENAME "gs"
#endif

/* Whether Ghostscript available on system */
int testUtilHaveGhostscript() {
    return system(GS_FILENAME " -v " DEV_NULL) == 0;
}

/* Check EPS files */
int testUtilVerifyGhostscript(const char *filename, int debug) {
    char cmd[512 + 128];

    if (strlen(filename) > 512) {
        return -1;
    }
    if (debug & ZINT_DEBUG_TEST_PRINT) {
        // Prints nothing of interest with or without -q unless bad
        sprintf(cmd, GS_FILENAME " -dNOPAUSE -dBATCH -dNODISPLAY -q %s", filename);
        printf("%s\n", cmd);
    } else {
        sprintf(cmd, GS_FILENAME " -dNOPAUSE -dBATCH -dNODISPLAY -q %s", filename);
    }

    return system(cmd);
}

/* Whether vnu validator available on system. v.Nu https://github.com/validator/validator
   Needs "$INSTALL_DIR/vnu-runtime-image/bin" in PATH */
int testUtilHaveVnu() {
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
int testUtilHaveTiffInfo() {
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
        { "code16k", BARCODE_CODE16K, 23, 0, 0, 0, 8 /*linear_row_height*/, 0, },
        { "code49", BARCODE_CODE49, 24, 0, 0, 0, 8 /*linear_row_height*/, 0, },
        { "code93", BARCODE_CODE93, 25, 0, 0, 0, 0, 0, },
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
        { "pzn", BARCODE_PZN, 52, 0, 0, 0, 0, 0, },
        { "pharmacode2", BARCODE_PHARMA_TWO, 53, 0, 0, 0, 0, 0, },
        { "", -1, 54, 0, 0, 0, 0, 0, },
        { "pdf417", BARCODE_PDF417, 55, 1, 1, 0, 0, 0, },
        { "pdf417compact", BARCODE_PDF417COMP, 56, 1, 1, 0, 0, 0, },
        { "maxicode", BARCODE_MAXICODE, 57, 1, 1, 0, 0, 0, },
        { "qrcode", BARCODE_QRCODE, 58, 0, 0, 0, 0, 0, },
        { "", -1, 59, 0, 0, 0, 0, 0, },
        { "", BARCODE_CODE128B, 60, 0, 0, 0, 0, 0, },
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
        { "", BARCODE_DPD, 96, 0, 0, 0, 0, 0, },
        { "microqrcode", BARCODE_MICROQR, 97, 1, 1, 1, 0, 0, },
        { "hibccode128", BARCODE_HIBC_128, 98, 0, 0, 0, 0, 0, },
        { "hibccode39", BARCODE_HIBC_39, 99, 0, 0, 0, 0, 0, },
        { "", -1, 100, 0, 0, 0, 0, 0, },
        { "", -1, 101, 0, 0, 0, 0, 0, },
        { "hibcdatamatrix", BARCODE_HIBC_DM, 102, 0, 1, 1, 0, 0, },
        { "", -1, 103, 0, 0, 0, 0, 0, },
        { "hibcqrcode", BARCODE_HIBC_QR, 104, 0, 0, 0, 0, 0, },
        { "", -1, 105, 0, 0, 0, 0, 0, },
        { "hibcpdf417", BARCODE_HIBC_PDF, 106, 1, 1, 0, 0, 0, },
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
        { "", -1, 119, 0, 0, 0, 0, 0, },
        { "", -1, 120, 0, 0, 0, 0, 0, },
        { "", BARCODE_MAILMARK, 121, 0, 0, 0, 0, 0, }, /* Note BWIPP mailmark is Data Matrix variant */
        { "", -1, 122, 0, 0, 0, 0, 0, },
        { "", -1, 123, 0, 0, 0, 0, 0, },
        { "", -1, 124, 0, 0, 0, 0, 0, },
        { "", -1, 125, 0, 0, 0, 0, 0, },
        { "", -1, 126, 0, 0, 0, 0, 0, },
        { "", -1, 127, 0, 0, 0, 0, 0, },
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
        { "", BARCODE_UPNQR, 143, 0, 0, 0, 0, 0, },
        { "ultracode", BARCODE_ULTRA, 144, 1, 0, 0, 0, 0, },
        { "rectangularmicroqrcode", BARCODE_RMQR, 145, 1, 1, 0, 0, 0, },
    };
    static const int data_size = ARRAY_SIZE(data);

    int symbology = symbol->symbology;
    int gs1 = (symbol->input_mode & 0x07) == GS1_MODE;

    if (symbology < 0 || symbology >= data_size) {
        fprintf(stderr, "testUtilBwippName: unknown symbology (%d)\n", symbology);
        abort();
    }
    // Self-check
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
        } else if (symbology == BARCODE_AZTEC || symbology == BARCODE_ULTRA) {
            if (debug & ZINT_DEBUG_TEST_PRINT) {
                printf("i:%d %s not BWIPP compatible, GS1_MODE not supported\n",
                        index, testUtilBarcodeName(symbology));
            }
            return NULL;
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
static void testUtilBwippCvtGS1Data(char *bwipp_data, int upcean, int *addon_posn) {
    char *b;
    int pipe = 0;

    *addon_posn = 0;
    for (b = bwipp_data; *b; b++) {
        if (upcean && *b == '|') {
            pipe = 1;
        }
        if (*b == '[') {
            *b = '(';
        } else if (*b == ']') {
            *b = ')';
        } else if (*b == '+' && upcean && !pipe) {
            *b = ' ';
            *addon_posn = b - bwipp_data;
        }
    }
}

/* Convert data to Ghostscript format for passing to bwipp_dump.ps */
static char *testUtilBwippEscape(char *bwipp_data, int bwipp_data_size, const char *data, int length,
                int zint_escape_mode, int eci, int *parse, int *parsefnc) {
    char *b = bwipp_data;
    char *be = b + bwipp_data_size;
    unsigned char *d = (unsigned char *) data;
    unsigned char *de = (unsigned char *) data + length;

    *parse = *parsefnc = 0;

    if (eci) {
        sprintf(bwipp_data, "^ECI%06d", eci);
        *parsefnc = 1;
        b = bwipp_data + 10;
    }

    while (b < be && d < de) {
        /* Have to escape double quote otherwise Ghostscript gives "Unterminated quote in @-file" for some reason */
        /* Escape single quote also to avoid having to do proper shell escaping TODO: proper shell escaping */
        if (*d < 0x20 || *d >= 0x7F || *d == '^' || *d == '"' || *d == '\'') {
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
                //case 'x': val = 0; /* TODO: implement */ break;
                case '\\': val = '\\'; break;
                //case 'u': val = 0; /* TODO: implement */ break;
                default: fprintf(stderr, "testUtilBwippEscape: unknown escape %c\n", *d); return NULL; break;
            }
            if (b + 4 >= be) {
                fprintf(stderr, "testUtilBwippEscape: loop bwipp_data buffer full (%d)\n", bwipp_data_size);
                return NULL;
            }
            sprintf(b, "^%03d", val);
            b += 4;
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

#define GS_INITIAL_LEN  35 /* Length of cmd up to -q */

/* Create bwipp_dump.ps command and run */
int testUtilBwipp(int index, const struct zint_symbol *symbol, int option_1, int option_2, int option_3,
            const char *data, int length, const char *primary, char *buffer, int buffer_size) {
    const char *cmd_fmt = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%s' backend/tests/tools/bwipp_dump.ps";
    const char *cmd_opts_fmt = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%s' -so='%s'"
                                " backend/tests/tools/bwipp_dump.ps";
    // If data > 2K
    const char *cmd_fmt2 = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%.2043s' -sd2='%s'"
                                " backend/tests/tools/bwipp_dump.ps";
    const char *cmd_opts_fmt2 = "gs -dNOPAUSE -dBATCH -dNODISPLAY -q -sb=%s -sd='%.2043s' -sd2='%s' -so='%s'"
                                " backend/tests/tools/bwipp_dump.ps";

    int symbology = symbol->symbology;
    int data_len = length == -1 ? (int) strlen(data) : length;
    int primary_len = primary ? (int) strlen(primary) : 0;
    /* 4 AI prefix + primary + '|' + leading zero + escaped data + fudge */
    int max_data_len = 4 + primary_len + 1 + 1 + data_len * 4 + 64;

    int eci_length = get_eci_length(symbol->eci, (const unsigned char *) data, data_len);
    char *converted = (char *) testutil_alloca(eci_length + 1);
    char *cmd = (char *) testutil_alloca(max_data_len + 1024);
    const char *bwipp_barcode = NULL;
    char *bwipp_opts = NULL;
    int bwipp_data_size = max_data_len + 1;
    char *bwipp_data = (char *) testutil_alloca(bwipp_data_size);
    char bwipp_opts_buf[512];
    int *bwipp_row_height = (int *) testutil_alloca(sizeof(int) * symbol->rows);
    int linear_row_height;
    int gs1_cvt;
    int user_mask;

    FILE *fp = NULL;
    int cnt;

    char *b = buffer;
    char *be = buffer + buffer_size;
    int r, h;
    int parse, parsefnc;

    int upcean = is_extendable(symbology);
    int upca = symbology == BARCODE_UPCA || symbology == BARCODE_UPCA_CHK || symbology == BARCODE_UPCA_CC;
    char obracket = symbol->input_mode & GS1PARENS_MODE ? '(' : '[';
    char cbracket = symbol->input_mode & GS1PARENS_MODE ? ')' : ']';
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

    if ((symbol->input_mode & 0x07) == UNICODE_MODE && ZBarcode_Cap(symbology, ZINT_CAP_ECI)
            && is_eci_convertible(symbol->eci)) {
        if (utf8_to_eci(symbol->eci, (const unsigned char *) data, (unsigned char *) converted, &data_len) == 0) {
            eci = symbol->eci;
        } else {
            if (symbol->eci != 0) {
                eci = get_best_eci((const unsigned char *) data, data_len);
                if (utf8_to_eci(eci, (const unsigned char *) data, (unsigned char *) converted, &data_len) != 0) {
                    fprintf(stderr, "i:%d testUtilBwipp: failed to convert Unicode data for %s\n",
                            index, testUtilBarcodeName(symbology));
                    return -1;
                }
            } else {
                fprintf(stderr, "i:%d testUtilBwipp: failed to convert Unicode data for %s\n",
                        index, testUtilBarcodeName(symbology));
                return -1;
            }
        }
        data = converted;
    } else {
        eci = symbol->eci >= 3 && ZBarcode_Cap(symbology, ZINT_CAP_ECI) ? symbol->eci : 0;
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
        testUtilBwippCvtGS1Data(bwipp_data, upcean, &addon_posn);

        if (upcean) {
            if (symbology == BARCODE_EANX_CC && (primary_len <= 8 || (addon_posn && addon_posn <= 8))) {
                bwipp_barcode = "ean8composite";
            }
            if (addon_posn) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%saddongap=%d",
                        strlen(bwipp_opts_buf) ? " " : "", option_2 > 0 ? option_2 : upca ? 9 : 7);
                bwipp_opts = bwipp_opts_buf;
            }
            bwipp_row_height[symbol->rows - 1] = 72;
        }

        if (option_1 > 0) {
            sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sccversion=%c",
                    strlen(bwipp_opts_buf) ? " " : "", option_1 == 1 ? 'a' : option_1 == 2 ? 'b' : 'c');
            bwipp_opts = bwipp_opts_buf;
        }
        if (option_2 > 0) {
            sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%ssegments=%d",
                    strlen(bwipp_opts_buf) ? " " : "", option_2 * 2);
            bwipp_opts = bwipp_opts_buf;
        }

        if (symbol->input_mode & GS1NOCHECK_MODE) {
            sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sdontlint", strlen(bwipp_opts_buf) ? " " : "");
            bwipp_opts = bwipp_opts_buf;
        }
    } else {
        if (gs1_cvt) {
            if (*data != obracket && !upcean) {
                strcat(bwipp_data, symbology == BARCODE_NVE18 ? "(00)" : "(01)");
            }
            strcat(bwipp_data, data);
            testUtilBwippCvtGS1Data(bwipp_data, upcean, &addon_posn);

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
                    bwipp_opts = bwipp_opts_buf;
                }
            }

            if (option_2 > 0) {
                if (symbology == BARCODE_DBAR_EXP || symbology == BARCODE_DBAR_EXPSTK) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%ssegments=%d",
                            strlen(bwipp_opts_buf) ? " " : "", option_2 * 2);
                    bwipp_opts = bwipp_opts_buf;
                }
            }

            if (symbol->input_mode & GS1NOCHECK_MODE) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sdontlint", strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
            }
        } else {
            if (testUtilBwippEscape(bwipp_data, bwipp_data_size, data, data_len, symbol->input_mode & ESCAPE_MODE,
                    eci, &parse, &parsefnc) == NULL) {
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
                if (option_2 == 1 || option_2 == 2) { // Add check digit without or with HRT suppression
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sincludecheck",
                            strlen(bwipp_opts_buf) ? " " : "");
                    bwipp_opts = bwipp_opts_buf;
                }
            } else if (symbology == BARCODE_CODE93) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sincludecheck",
                        strlen(bwipp_opts_buf) ? " " : "");
                if (parse) {
                    bwipp_barcode = "code93ext";
                }
                bwipp_opts = bwipp_opts_buf;
            } else if (symbology == BARCODE_PZN) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%spzn8", strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
            } else if (symbology == BARCODE_TELEPEN_NUM) {
                if (data_len & 1) { // Add leading zero
                    memmove(bwipp_data + 1, bwipp_data, strlen(bwipp_data) + 1);
                    *bwipp_data = '0';
                }
            } else if (symbology == BARCODE_CODABLOCKF || symbology == BARCODE_HIBC_BLOCKF) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%ssepheight=0", strlen(bwipp_opts_buf) ? " " : "");
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
                for (r = 0; r < symbol->rows; r++) bwipp_row_height[r] = 1; /* Change from 3 */
                if (option_1 >= 0) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%seclevel=%d",
                            strlen(bwipp_opts_buf) ? " " : "", option_1);
                    bwipp_opts = bwipp_opts_buf;
                }
                if (option_2 > 0) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%scolumns=%d",
                            strlen(bwipp_opts_buf) ? " " : "", option_2);
                    bwipp_opts = bwipp_opts_buf;
                }
            } else if (symbology == BARCODE_POSTNET || symbology == BARCODE_PLANET || symbology == BARCODE_RM4SCC
                        || symbology == BARCODE_JAPANPOST || symbology == BARCODE_KIX || symbology == BARCODE_DAFT
                        || symbology == BARCODE_USPS_IMAIL || symbology == BARCODE_AUSPOST
                        || symbology == BARCODE_PHARMA_TWO) {
                for (r = 0; r < symbol->rows; r++) bwipp_row_height[r] = 1; /* Zap */
                if (symbology == BARCODE_KIX) {
                    to_upper((unsigned char *) bwipp_data);
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
            } else if (symbology == BARCODE_FIM) {
                strcpy(bwipp_data, "fima");
                bwipp_data[3] = data[0] - 'A' + 'a';
            } else if (symbology == BARCODE_CODE16K || symbology == BARCODE_CODE49) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%ssepheight=0", strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
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
                                bwipp_data[j++] = '[';
                            }
                            last_ai = atoi(bwipp_data + i + 1);
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
                        if (bwipp_data[i] == '[') {
                            memmove(bwipp_data + i + 5, bwipp_data + i + 1, len - i);
                            memcpy(bwipp_data + i, "^FNC1", 5);
                            len += 4;
                        }
                    }
                    if (symbol->eci == 0) { /* If not already done for ECI */
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
                        // TODO: Properly allow for different T sizes
                        codeone_version = length <= 22 ? "T-16" : length <= 34 ? "T-32" : "T-48";
                    } else {
                        codeone_version = codeone_versions[option_2 - 1];
                    }
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sversion=%s",
                            strlen(bwipp_opts_buf) ? " " : "", codeone_version);
                    bwipp_opts = bwipp_opts_buf;
                }
            } else if (symbology == BARCODE_MAXICODE) {
                int have_scm = memcmp(bwipp_data, "[)>^03001^02996", 15) == 0;
                int mode = option_1;
                if (mode <= 0) {
                    if (primary_len == 0) {
                        mode = 4;
                    } else {
                        for (i = 0; i < primary_len - 6; i++) {
                            if (((symbol->primary[i] < '0') || (symbol->primary[i] > '9'))
                                    && (symbol->primary[i] != ' ')) {
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
                if (primary_len) {
                    char prefix_buf[30];
                    int prefix_len;
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
                    if (have_scm) {
                        memmove(bwipp_data + 15 + prefix_len, bwipp_data, strlen(bwipp_data) - 15 + 1);
                        memcpy(bwipp_data + 15, prefix_buf, prefix_len);
                    } else {
                        memmove(bwipp_data + prefix_len, bwipp_data, strlen(bwipp_data) + 1);
                        memcpy(bwipp_data, prefix_buf, prefix_len);
                    }
                    if (!parse) {
                        sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sparse",
                                strlen(bwipp_opts_buf) ? " " : "");
                        bwipp_opts = bwipp_opts_buf;
                        parse = 1;
                    }
                }
                if (option_2 > 0) {
                    char scm_vv_buf[32];
                    sprintf(scm_vv_buf, "[)>^03001^029%02d", option_2); /* [)>\R01\Gvv */
                    memmove(bwipp_data + 15, bwipp_data, strlen(bwipp_data) + 1);
                    memcpy(bwipp_data, scm_vv_buf, 15);
                    if (!parse) {
                        sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sparse",
                                strlen(bwipp_opts_buf) ? " " : "");
                        bwipp_opts = bwipp_opts_buf;
                        parse = 1;
                    }
                }
            }
        }

        if (symbology == BARCODE_DATAMATRIX || symbology == BARCODE_HIBC_DM) {
            int added_dmre = 0;
            #include "../dmatrix.h"
            (void)matrixrsblock; (void)matrixdatablock; (void)matrixbytes; (void)matrixFW; (void)matrixFH;
            (void)isDMRE; (void)text_value; (void)text_shift; (void)c40_value; (void)c40_shift;

            if (symbol->output_options & GS1_GS_SEPARATOR) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sgssep", strlen(bwipp_opts_buf) ? " " : "");
                bwipp_opts = bwipp_opts_buf;
            }
            if (option_2 >= 1 && option_2 <= ARRAY_SIZE(intsymbol)) {
                int idx = intsymbol[option_2 - 1];
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%srows=%d columns=%d",
                        strlen(bwipp_opts_buf) ? " " : "", matrixH[idx], matrixW[idx]);
                bwipp_opts = bwipp_opts_buf;
                if (option_2 >= 31) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sdmre", strlen(bwipp_opts_buf) ? " " : "");
                    added_dmre = 1;
                }
            }
            if (option_3 != DM_SQUARE && symbol->width != symbol->height) {
                if (option_3 == DM_DMRE && !added_dmre) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sdmre", strlen(bwipp_opts_buf) ? " " : "");
                    //added_dmre = 1;
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
        } else if (symbology == BARCODE_MICROQR || symbology == BARCODE_RMQR) {
            if (option_1 >= 1 && option_1 <= 4) {
                static const char eccs[4] = { 'L', 'M', 'Q', 'H' };
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%seclevel=%c",
                        strlen(bwipp_opts_buf) ? " " : "", eccs[option_1 - 1]);
                bwipp_opts = bwipp_opts_buf;
            }
            if (symbology == BARCODE_MICROQR) {
                if (option_2 >= 1 && option_2 <= 4) {
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sversion=M%d",
                            strlen(bwipp_opts_buf) ? " " : "", option_2);
                    bwipp_opts = bwipp_opts_buf;
                }
                if (option_3 != -1) {
                    int mask = (symbol->option_3 >> 8) & 0x0F;
                    if (mask >= 1 && mask <= 4) {
                        sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%smask=%d",
                                strlen(bwipp_opts_buf) ? " " : "", ((symbol->option_3 >> 8) & 0x0F));
                        bwipp_opts = bwipp_opts_buf;
                    }
                }
            } else if (symbology == BARCODE_RMQR) {
                if (option_2 >= 1 && option_2 <= 32) {
                    static const char *vers[] = {
                        "R7x43", "R7x59", "R7x77", "R7x99", "R7x139", "R9x43", "R9x59", "R9x77", "R9x99", "R9x139",
                        "R11x27", "R11x43", "R11x59", "R11x77", "R11x99", "R11x139", "R13x27", "R13x43", "R13x59", "R13x77",
                        "R13x99", "R13x139", "R15x43", "R15x59", "R15x77", "R15x99", "R15x139", "R17x43", "R17x59", "R17x77",
                        "R17x99", "R17x139",
                    };
                    sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%sversion=%s",
                            strlen(bwipp_opts_buf) ? " " : "", vers[option_2 - 1]);
                    bwipp_opts = bwipp_opts_buf;
                }
            }
        } else if (symbology == BARCODE_ULTRA) {
            if (option_1 >= 1 && option_1 <= 6) {
                sprintf(bwipp_opts_buf + strlen(bwipp_opts_buf), "%seclevel=EC%d",
                        strlen(bwipp_opts_buf) ? " " : "", option_1 - 1);
                bwipp_opts = bwipp_opts_buf;
            }
        }
    }

    if ((option_1 != -1 || option_2 != -1 || option_3 != -1) && !bwipp_opts) {
        fprintf(stderr,
                "i:%d testUtilBwipp: no BWIPP options set option_1 %d, option_2 %d, option_3 %d for symbology %s\n",
                index, option_1, option_2, option_3, testUtilBarcodeName(symbology));
        return -1;
    }

    if (bwipp_opts && strlen(bwipp_opts)) {
        if (strlen(bwipp_data) >= 2043) { /* Ghostscript's `arg_str_max` 2048 less "-sd=" */
            sprintf(cmd, cmd_opts_fmt2, bwipp_barcode, bwipp_data, bwipp_data + 2043, bwipp_opts);
        } else {
            sprintf(cmd, cmd_opts_fmt, bwipp_barcode, bwipp_data, bwipp_opts);
        }
    } else {
        if (strlen(bwipp_data) >= 2043) {
            sprintf(cmd, cmd_fmt2, bwipp_barcode, bwipp_data, bwipp_data + 2043);
        } else {
            sprintf(cmd, cmd_fmt, bwipp_barcode, bwipp_data);
        }
    }

    /* Hack in various adjustments */
    if (symbology == BARCODE_DBAR_OMN || symbology == BARCODE_DBAR_LTD || symbology == BARCODE_DBAR_EXP) {
        /* Begin with space */
        char adj[5] = " -sbs";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj), cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj));
    }
    if (symbology == BARCODE_CODE11 || symbology == BARCODE_CODE39 || symbology == BARCODE_EXCODE39
            || symbology == BARCODE_CODABAR || symbology == BARCODE_PHARMA || symbology == BARCODE_PZN
            || symbology == BARCODE_CODE32 || symbology == BARCODE_VIN) {
        /* Ratio 3 width bar/space -> 2 width */
        char adj[8] = " -sr=0.6";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj), cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj));
    }
    if (symbology == BARCODE_C25INTER || symbology == BARCODE_DPLEIT || symbology == BARCODE_DPIDENT
            || symbology == BARCODE_ITF14) {
        /* Ratio 2 width bar/space -> 3 width */
        char adj[8] = " -sr=1.3";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj), cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj));
    }
    if (symbology == BARCODE_FIM) {
        /* Ratio 2 width bar/space -> 1 width */
        char adj[8] = " -sr=0.5";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj), cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj));
    }
    if (symbology == BARCODE_PLESSEY) {
        /* Ceiling ratio 3/4/5 width bar/space -> 2 width then round ratio 2 width bar/space -> 3 width */
        char adj[16] = " -sc=0.4 -sr=1.3";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj), cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj));
    }
    if (symbology == BARCODE_CODE11 || symbology == BARCODE_CODE39 || symbology == BARCODE_EXCODE39
            || symbology == BARCODE_HIBC_39 || symbology == BARCODE_LOGMARS || symbology == BARCODE_PHARMA
            || symbology == BARCODE_PZN || symbology == BARCODE_CODE32 || symbology == BARCODE_VIN
            || symbology == BARCODE_C25INTER || symbology == BARCODE_DPLEIT || symbology == BARCODE_DPIDENT
            || symbology == BARCODE_ITF14 || symbology == BARCODE_PHARMA_TWO) {
        /* End sbs loop on bar */
        char adj[6] = " -selb";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj), cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj));
    }
    if (symbology == BARCODE_C25STANDARD) {
        /* Zint uses 4X start/stop wides while BWIPP uses 3X - convert */
        char adj[91] = " -sp='i 0 eq i limit 4 sub eq or sbs i get 3 eq and { (1111) print true } { false } ifelse'";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj), cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj));
    }
    if (symbology == BARCODE_POSTNET || symbology == BARCODE_PLANET || symbology == BARCODE_RM4SCC
            || symbology == BARCODE_JAPANPOST || symbology == BARCODE_KIX || symbology == BARCODE_DAFT
            || symbology == BARCODE_USPS_IMAIL || symbology == BARCODE_AUSPOST || symbology == BARCODE_PHARMA_TWO) {
        /* Emulate rows with BWIPP heights. */
        char adj[5] = " -shs";
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj), cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj));
    }
    if (symbology == BARCODE_CODE16K || symbology == BARCODE_CODE49) {
        char adj[15] = " -sxs=10 -sxe=1"; /* Strip first 10 and last zero */
        memmove(cmd + GS_INITIAL_LEN + sizeof(adj), cmd + GS_INITIAL_LEN, strlen(cmd) + 1 - GS_INITIAL_LEN);
        memcpy(cmd + GS_INITIAL_LEN, adj, sizeof(adj));
    }

    if (symbol->debug & ZINT_DEBUG_TEST_PRINT) {
        printf("i:%d testUtilBwipp: cmd %s\n", index, cmd);
    }

    fp = testutil_popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "i:%d testUtilBwipp: failed to run '%s'\n", index, cmd);
        return -1;
    }

    for (r = 0; r < symbol->rows; r++) {
        if (b + symbol->width > be) {
            fprintf(stderr, "i:%d testUtilBwipp: row %d, width %d, row width iteration overrun (%s)\n",
                    index, r, symbol->width, cmd);
            testutil_pclose(fp);
            return -1;
        }
        cnt = (int) fread(b, 1, symbol->width, fp);
        if (cnt != symbol->width) {
            fprintf(stderr, "i:%d testUtilBwipp: failed to read symbol->width %d bytes, cnt %d (%s)\n",
                    index, symbol->width, cnt, cmd);
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

/* Compare bwipp_dump.ps output to test suite module dump */
int testUtilBwippCmp(const struct zint_symbol *symbol, char *msg, char *bwipp_buf, const char *expected) {
    int bwipp_len = (int) strlen(bwipp_buf);
    int expected_len = (int) strlen(expected);
    int ret_memcmp;
    int i;

    (void)symbol;

    if (bwipp_len != expected_len) {
        sprintf(msg, "bwipp_len %d != expected_len %d", bwipp_len, expected_len);
        return 2;
    }

    if (symbol->symbology == BARCODE_ULTRA) {
        static const char map[] = { '8', '1', '2', '3', '4', '5', '6', '7', '8', '7' };
        for (i = 0; i < bwipp_len; i++) {
            if (bwipp_buf[i] >= '0' && bwipp_buf[i] <= '9') {
                bwipp_buf[i] = map[bwipp_buf[i] - '0'];
            }
        }
    }
    ret_memcmp = memcmp(bwipp_buf, expected, expected_len);
    if (ret_memcmp != 0) {
        for (i = 0; i < expected_len; i++) {
            if (bwipp_buf[i] != expected[i]) {
                break;
            }
        }
        sprintf(msg, "bwipp memcmp %d != 0, at %d, len %d", ret_memcmp, i, expected_len);
        return ret_memcmp;
    }

    return 0;
}

/* Compare bwipp_dump.ps output to single row module dump (see testUtilModulesPrintRow) */
int testUtilBwippCmpRow(const struct zint_symbol *symbol, int row, char *msg, const char *bwipp_buf,
            const char *expected) {
    int bwipp_len = (int) strlen(bwipp_buf);
    int expected_len = (int) strlen(expected);
    int ret_memcmp;
    int i, j;

    (void)symbol;

    if (bwipp_len != expected_len * symbol->rows) {
        sprintf(msg, "bwipp_len %d != expected_len %d * symbol->rows %d", bwipp_len, expected_len, symbol->rows);
        return 2;
    }

    ret_memcmp = memcmp(bwipp_buf + expected_len * row, expected, expected_len);
    if (ret_memcmp != 0) {
        for (i = 0, j = expected_len * row; i < expected_len; i++, j++) {
            if (bwipp_buf[j] != expected[i]) {
                break;
            }
        }
        sprintf(msg, "bwipp memcmp %d != 0, at %d (%d), len %d", ret_memcmp, i, j, expected_len);
        return ret_memcmp;
    }

    return 0;
}
