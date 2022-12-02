/*
    libzint - the open source barcode library
    Copyright (C) 2021-2022 Robin Stuart <rstuart114@gmail.com>

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
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

INTERNAL int out_quiet_zones_test(const struct zint_symbol *symbol, const int hide_text,
                            float *left, float *right, float *top, float *bottom);

static void test_quiet_zones(const testCtx *const p_ctx) {
    int i, ret;
    struct zint_symbol symbol = {0};
    int hide_text = 0;
    float left, right, top, bottom;

    testStart("test_quiet_zones");

    for (i = 1; i <= BARCODE_LAST; i++) {
        if (!ZBarcode_ValidID(i)) continue;
        if (testContinue(p_ctx, i)) continue;

        symbol.symbology = i;
        symbol.output_options = BARCODE_QUIET_ZONES;
        ret = out_quiet_zones_test(&symbol, hide_text, &left, &right, &top, &bottom);
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
        len = strlen(outfile);
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
            assert_zero(remove(outfile), "i:%d remove(%s) != 0 (%d: %s)\n", i, outfile, errno, strerror(errno));
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


int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_quiet_zones", test_quiet_zones },
        { "test_fopen", test_fopen },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
