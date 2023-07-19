/*
    libzint - the open source barcode library
    Copyright (C) 2021-2023 Robin Stuart <rstuart114@gmail.com>

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
/* SPDX-License-Identifier: BSD-3-Clause */

#include "testcommon.h"
#include "../output.h"
#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

static void test_check_colour_options(const testCtx *const p_ctx) {
    struct item {
        char *fgcolour;
        char *bgcolour;
        int ret;
        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "FFFFFF", "000000", 0, "" },
        /*  1*/ { "ffffff", "ffffff", 0, "" },
        /*  2*/ { "77777777", "33333333", 0, "" },
        /*  3*/ { "FFFFF", "000000", ZINT_ERROR_INVALID_OPTION, "880: Malformed foreground RGB colour (6 or 8 characters only)" },
        /*  4*/ { "FFFFFFF", "000000", ZINT_ERROR_INVALID_OPTION, "880: Malformed foreground RGB colour (6 or 8 characters only)" },
        /*  5*/ { "FFFFFG", "000000", ZINT_ERROR_INVALID_OPTION, "881: Malformed foreground RGB colour 'FFFFFG' (hexadecimal only)" },
        /*  6*/ { "FFFFFF", "000000000", ZINT_ERROR_INVALID_OPTION, "880: Malformed background RGB colour (6 or 8 characters only)" },
        /*  7*/ { "FFFFFF", "0000000Z", ZINT_ERROR_INVALID_OPTION, "881: Malformed background RGB colour '0000000Z' (hexadecimal only)" },
        /*  8*/ { "100,100,100,100", "0,1,2,3", 0, "" },
        /*  9*/ { "100,,100,100", ",1,2,", 0, "" },
        /* 10*/ { "100,100,100", "0,1,2,3", ZINT_ERROR_INVALID_OPTION, "882: Malformed foreground CMYK colour (4 decimal numbers, comma-separated)" },
        /* 11*/ { "100,100,99,1001", "0,1,2,3", ZINT_ERROR_INVALID_OPTION, "883: Malformed foreground CMYK colour (3 digit maximum per number)" },
        /* 12*/ { "101,100,100,100", "0,1,2,3", ZINT_ERROR_INVALID_OPTION, "884: Malformed foreground CMYK colour C (decimal 0-100 only)" },
        /* 13*/ { "100,101,100,100", "0,1,2,3", ZINT_ERROR_INVALID_OPTION, "885: Malformed foreground CMYK colour M (decimal 0-100 only)" },
        /* 14*/ { "100,100,101,100", "0,1,2,3", ZINT_ERROR_INVALID_OPTION, "886: Malformed foreground CMYK colour Y (decimal 0-100 only)" },
        /* 15*/ { "100,100,100,101", "0,1,2,3", ZINT_ERROR_INVALID_OPTION, "887: Malformed foreground CMYK colour K (decimal 0-100 only)" },
        /* 16*/ { "100,100,100,100", "0,1,", ZINT_ERROR_INVALID_OPTION, "882: Malformed background CMYK colour (4 decimal numbers, comma-separated)" },
        /* 17*/ { "100,100,100,100", "0,0123,3,4", ZINT_ERROR_INVALID_OPTION, "883: Malformed background CMYK colour (3 digit maximum per number)" },
        /* 18*/ { "100,100,100,100", "0,1,2,101", ZINT_ERROR_INVALID_OPTION, "887: Malformed background CMYK colour K (decimal 0-100 only)" },
        /* 19*/ { "100,100,100,100", "0,1,2,3,", ZINT_ERROR_INVALID_OPTION, "882: Malformed background CMYK colour (4 decimal numbers, comma-separated)" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, ret;
    struct zint_symbol symbol;

    testStart("test_check_colour_options");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(symbol.fgcolour, data[i].fgcolour);
        strcpy(symbol.bgcolour, data[i].bgcolour);
        symbol.errtxt[0] = '\0';

        ret = out_check_colour_options(&symbol);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol.errtxt);
        assert_zero(strcmp(symbol.errtxt, data[i].expected), "i:%d symbol.errtxt (%s) != expected (%s)\n", i, symbol.errtxt, data[i].expected);
    }

    testFinish();
}

static void test_colour_get_rgb(const testCtx *const p_ctx) {
    struct item {
        char *colour;
        int ret;
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
        char *expected_cmyk;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "FFFFFF", 0, 0xFF, 0xFF, 0xFF, 0xFF, "0,0,0,0" },
        /*  1*/ { "000000", 0, 0x00, 0x00, 0x00, 0xFF, "0,0,0,100" },
        /*  2*/ { "FEDCBA", 0, 0xFE, 0xDC, 0xBA, 0xFF, "0,13,27,0" },
        /*  3*/ { "EEDD9900", 1, 0xEE, 0xDD, 0x99, 0x00, "0,7,36,7" },
        /*  4*/ { "98765432", 1, 0x98, 0x76, 0x54, 0x32, "0,22,45,40" },
        /*  5*/ { "147AD0", 0, 0x14, 0x7A, 0xD0, 0xFF, "90,41,0,18" },
        /*  6*/ { "FC9630", 0, 0xFC, 0x96, 0x30, 0xFF, "0,40,81,1" },
        /*  7*/ { "112233", 0, 0x11, 0x22, 0x33, 0xFF, "67,33,0,80" },
        /*  8*/ { "CCDDEE", 0, 0xCC, 0xDD, 0xEE, 0xFF, "14,7,0,7" },
        /*  9*/ { "0,0,0,0", 0, 0xFF, 0xFF, 0xFF, 0xFF, "0,0,0,0" },
        /* 10*/ { "80,30,60,0", 0, 0x33, 0xB3, 0x66, 0xFF, "72,0,43,30" },
        /* 11*/ { "50,50,50,50", 0, 0x40, 0x40, 0x40, 0xFF, "0,0,0,75" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, ret;

    testStart("test_colour_get_rgb");

    for (i = 0; i < data_size; i++) {
        /* Suppress clang-16 run-time exception MemorySanitizer: use-of-uninitialized-value (fixed in clang-17) */
        unsigned char red = 0, green = 0, blue = 0, alpha = 0, rgb_alpha = 0;
        int cyan, magenta, yellow, black;
        int have_alpha;
        char rgb[9];
        char cmyk[16];

        if (testContinue(p_ctx, i)) continue;

        ret = out_colour_get_rgb(data[i].colour, &red, &green, &blue, &alpha);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_equal(red, data[i].red, "i:%d red 0x%02X (%d) != 0x%02X (%d) (green 0x%02X, blue 0x%02X)\n", i, red, red, data[i].red, data[i].red, green, blue);
        assert_equal(green, data[i].green, "i:%d green %d (0x%02X) != %d (0x%02X)\n", i, green, green, data[i].green, data[i].green);
        assert_equal(blue, data[i].blue, "i:%d blue %d (0x%02X) != %d (0x%02X)\n", i, blue, blue, data[i].blue, data[i].blue);
        assert_equal(alpha, data[i].alpha, "i:%d alpha %d (0x%02X) != %d (0x%02X)\n", i, alpha, alpha, data[i].alpha, data[i].alpha);

        have_alpha = ret == 1;
        if (have_alpha) {
            sprintf(rgb, "%02X%02X%02X%02X", red, green, blue, alpha);
        } else {
            sprintf(rgb, "%02X%02X%02X", red, green, blue);
        }
        ret = out_colour_get_cmyk(rgb, &cyan, &magenta, &yellow, &black, &rgb_alpha);
        assert_equal(ret, 1 + have_alpha, "i:%d out_colour_get_cmyk(%s) ret %d != %d\n", i, rgb, ret, 1 + have_alpha);
        assert_equal(rgb_alpha, alpha, "i:%d rgb_alpha %d (0x%02X) != %d (0x%02X)\n", i, rgb_alpha, rgb_alpha, alpha, alpha);

        sprintf(cmyk, "%d,%d,%d,%d", cyan, magenta, yellow, black);
        assert_zero(strcmp(cmyk, data[i].expected_cmyk), "i:%d strcmp(%s, %s) != 0\n", i, cmyk, data[i].expected_cmyk);
    }

    testFinish();
}

static void test_colour_get_cmyk(const testCtx *const p_ctx) {
    struct item {
        char *colour;
        int ret;
        int cyan;
        int magenta;
        int yellow;
        int black;
        unsigned char alpha;
        char *expected_rgb;
        int ret_rgb;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "80,30,60,0", 0, 80, 30, 60, 0, 0xFF, "33B366FF", 0 },
        /*  1*/ { "50,50,50,50", 0, 50, 50, 50, 50, 0xFF, "404040FF", 0 },
        /*  2*/ { "0,0,0,100", 0, 0, 0, 0, 100, 0xFF, "000000FF", 0 },
        /*  3*/ { "71,0,40,44", 0, 71, 0, 40, 44, 0xFF, "298F56FF", 0 },
        /*  4*/ { "123456", 1, 79, 40, 0, 44, 0xFF, "123456FF", 0 },
        /*  5*/ { "12345678", 2, 79, 40, 0, 44, 0x78, "12345678", 1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, ret;

    testStart("test_colour_get_cmyk");

    for (i = 0; i < data_size; i++) {
        int cyan, magenta, yellow, black;
        unsigned char red, green, blue, alpha, rgb_alpha;
        char rgb[9];

        if (testContinue(p_ctx, i)) continue;

        ret = out_colour_get_cmyk(data[i].colour, &cyan, &magenta, &yellow, &black, &alpha);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_equal(cyan, data[i].cyan, "i:%d cyan %d != %d (magenta %d, yellow %d, black %d)\n", i, cyan, data[i].cyan, magenta, yellow, black);
        assert_equal(magenta, data[i].magenta, "i:%d magenta %d != %d\n", i, magenta, data[i].magenta);
        assert_equal(yellow, data[i].yellow, "i:%d yellow %d != %d\n", i, yellow, data[i].yellow);
        assert_equal(alpha, data[i].alpha, "i:%d alpha %d != %d\n", i, alpha, data[i].alpha);

        ret = out_colour_get_rgb(data[i].colour, &red, &green, &blue, &rgb_alpha);
        assert_equal(ret, data[i].ret_rgb, "i:%d out_colour_get_rgb(%s) ret %d != %d\n", i, rgb, ret, data[i].ret_rgb);
        assert_equal(rgb_alpha, alpha, "i:%d rgb_alpha %d != %d\n", i, rgb_alpha, alpha);

        sprintf(rgb, "%02X%02X%02X%02X", red, green, blue, rgb_alpha);
        assert_zero(strcmp(rgb, data[i].expected_rgb), "i:%d strcmp(%s, %s) != 0\n", i, rgb, data[i].expected_rgb);
    }

    testFinish();
}

INTERNAL int out_quiet_zones_test(const struct zint_symbol *symbol, const int hide_text, const int comp_xoffset,
                            float *left, float *right, float *top, float *bottom);

static void test_quiet_zones(const testCtx *const p_ctx) {
    int i, ret;
    struct zint_symbol symbol = {0};
    int hide_text = 0;
    int comp_xoffset = 0;
    float left, right, top, bottom;

    testStart("test_quiet_zones");

    for (i = 1; i <= BARCODE_LAST; i++) {
        if (!ZBarcode_ValidID(i)) continue;
        if (testContinue(p_ctx, i)) continue;

        symbol.symbology = i;
        symbol.output_options = BARCODE_QUIET_ZONES;
        ret = out_quiet_zones_test(&symbol, hide_text, comp_xoffset, &left, &right, &top, &bottom);
        if (i != BARCODE_FLAT && i != BARCODE_BC412) { /* Only two which aren't marked as done */
            assert_nonzero(ret, "i:%d %s not done\n", i, testUtilBarcodeName(i));
        }
    }

    testFinish();
}

#ifdef _WIN32
#define TEST_OUT_SEP        '\\'
#define TEST_OUT_SEP_STR    "\\"
#else
#define TEST_OUT_SEP        '/'
#define TEST_OUT_SEP_STR    "/"
#endif

static void test_fopen(const testCtx *const p_ctx) {
    struct item {
        char dir[32];
        char subdir[32];
        char *filename;
        int succeed;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "", "", "out.png", 1 },
        /*  1*/ { "out_test", "", "out.png", 1 },
        /*  2*/ { "out_test_with_subdir", "out_test_subdir", "out.png", 1 },
        /*  3*/ { "out_test_with_subdir", "out_test_subdir", "out.bmp", 1 },
        /*  4*/ { "out_test_with_subdir", "out_test_subdir", "out.emf", 1 },
        /*  5*/ { "out_test_with_subdir", "out_test_subdir", "out.eps", 1 },
        /*  6*/ { "out_test_with_subdir", "out_test_subdir", "out.gif", 1 },
        /*  7*/ { "out_test_with_subdir", "out_test_subdir", "out.pcx", 1 },
        /*  8*/ { "out_test_with_subdir", "out_test_subdir", "out.svg", 1 },
        /*  9*/ { "out_test_with_subdir", "out_test_subdir", "out.tif", 1 },
        /* 10*/ { "out_test/", "", "out.png", 1 },
        /* 11*/ { "out_test//", "", "out.png", 1 },
        /* 12*/ { "out_test/", "/out_test_subdir/", "out.png", 1 },
        /* 13*/ { "out_test\\", "\\out_test_subdir\\", "out.png", 1 },
        /* 14*/ { "", "", "outé.png", 1 },
        /* 15*/ { "outé_test", "", "outé.png", 1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, len;

    FILE *ret;
    char cwdbuf[1024];
    char outfile[1024 + 256];
    char dirname[1024 + 256];
    char subdirname[1024 + 256];
    int dir_exists, subdir_exists;

    testStart("test_fopen");

    assert_nonnull(getcwd(cwdbuf, sizeof(cwdbuf)), "getcwd NULL (%d, %s)\n", errno, strerror(errno));

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(outfile, cwdbuf);
        len = (int) strlen(outfile);
        if (len && outfile[len - 1] != TEST_OUT_SEP) { outfile[len++] = TEST_OUT_SEP; outfile[len] = '\0'; }
        dir_exists = subdir_exists = 0;

        if (data[i].dir[0]) {
            strcat(outfile, data[i].dir);
            strcat(outfile, TEST_OUT_SEP_STR);
            strcpy(dirname, outfile);
            dir_exists = testUtilDirExists(dirname);
            if (data[i].subdir[0]) {
                strcat(outfile, data[i].subdir);
                strcat(outfile, TEST_OUT_SEP_STR);
                strcpy(subdirname, outfile);
                subdir_exists = testUtilDirExists(subdirname);
            }
        }
        strcat(outfile, data[i].filename);

        ret = out_fopen(outfile, "w");
        if (data[i].succeed) {
            assert_nonnull(ret, "i:%d out_fopen(%s) == NULL (%d: %s)\n", i, outfile, errno, strerror(errno));
            assert_zero(fclose(ret), "i:%d fclose(%s) != 0 (%d: %s)\n", i, outfile, errno, strerror(errno));
            assert_nonzero(testUtilExists(outfile), "i:%d testUtilExists(%s) != 0 (%d: %s)\n", i, outfile, errno, strerror(errno));
            if (data[i].dir[0]) {
                assert_nonzero(testUtilDirExists(dirname), "i:%d testUtilDirExists(%s) != 0 (%d: %s)\n", i, dirname, errno, strerror(errno));
            }
            assert_zero(testUtilRemove(outfile), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, outfile, errno, strerror(errno));
            if (data[i].dir[0]) {
                if (data[i].subdir[0] && !subdir_exists) {
                    assert_zero(testUtilRmDir(subdirname), "i:%d rmdir(%s) != 0 (%d: %s)\n", i, subdirname, errno, strerror(errno));
                }
                if (!dir_exists && strcmp(dirname, "/") != 0 && strcmp(dirname, "\\") != 0) {
                    assert_zero(testUtilRmDir(dirname), "i:%d rmdir(%s) != 0 (%d: %s)\n", i, dirname, errno, strerror(errno));
                }
            }
        } else {
            assert_null(ret, "i:%d out_fopen(%s) == NULL (%d: %s)\n", i, outfile, errno, strerror(errno));
            /* TODO: may have left junk around */
        }
    }

    testFinish();
}

static void test_out_putsf(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *prefix;
        int dp;
        float arg;
        const char *locale;
        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "", 2, 1234.123, "", "1234.12" },
        /*  1*/ { "", 3, 1234.123, "", "1234.123" },
        /*  2*/ { "prefix ", 4, 1234.123, "", "prefix 1234.123" },
        /*  3*/ { "", 2, -1234.126, "", "-1234.13" },
        /*  4*/ { "", 2, 1234.1, "", "1234.1" },
        /*  5*/ { "", 3, 1234.1, "", "1234.1" },
        /*  6*/ { "", 4, 1234.1, "", "1234.1" },
        /*  7*/ { "", 2, 1234.0, "", "1234" },
        /*  8*/ { "", 2, -1234.0, "", "-1234" },
        /*  9*/ { "", 3, 1234.1234, "de_DE.UTF-8", "1234.123" },
        /* 10*/ { "", 4, -1234.1234, "de_DE.UTF-8", "-1234.1234" },
        /* 11*/ { "prefix ", 4, -1234.1234, "de_DE.UTF-8", "prefix -1234.1234" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    FILE *fp;
    char buf[512] = {0}; /* Suppress clang-16/17 run-time exception MemorySanitizer: use-of-uninitialized-value */

    testStart("test_out_putsf");

#ifdef _WIN32
    (void)i; (void)fp; (void)buf;
    testSkip("Test not implemented on Windows");
#else

    for (i = 0; i < data_size; i++) {
        const char *locale = NULL;

        if (testContinue(p_ctx, i)) continue;

        buf[0] = '\0';
        fp = fmemopen(buf, sizeof(buf), "w");
        assert_nonnull(fp, "%d: fmemopen fail (%d, %s)\n", i, errno, strerror(errno));

        if (data[i].locale && data[i].locale[0]) {
            locale = setlocale(LC_ALL, data[i].locale);
            if (!locale) { /* May not be available - warn unless quiet mode */
                if (!(debug & ZINT_DEBUG_TEST_LESS_NOISY)) {
                    printf("%d: Warning: locale \"%s\" not available\n", i, data[i].locale);
                }
            }
        }

        out_putsf(data[i].prefix, data[i].dp, data[i].arg, fp);

        assert_zero(fclose(fp), "%d: fclose fail (%d, %s)\n", i, errno, strerror(errno));

        if (locale) {
            assert_nonnull(setlocale(LC_ALL, locale), "%d: setlocale(%s) restore fail (%d, %s)\n",
                i, locale, errno, strerror(errno));
        }

        assert_zero(strcmp(buf, data[i].expected), "%d: strcmp(%s, %s) != 0\n", i, buf, data[i].expected);
    }

    testFinish();
#endif /* _WIN32 */
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_check_colour_options", test_check_colour_options },
        { "test_colour_get_rgb", test_colour_get_rgb },
        { "test_colour_get_cmyk", test_colour_get_cmyk },
        { "test_quiet_zones", test_quiet_zones },
        { "test_fopen", test_fopen },
        { "test_out_putsf", test_out_putsf },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
