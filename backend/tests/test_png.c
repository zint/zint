/*
    libzint - the open source barcode library
    Copyright (C) 2020 Robin Stuart <rstuart114@gmail.com>

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

#include "testcommon.h"
#include <sys/stat.h>

extern int png_pixel_plot(struct zint_symbol *symbol, char *pixelbuf);

static void test_pixel_plot(int index, int debug) {

    testStart("");

    if (!testUtilHaveIdentify()) {
        testSkip("ImageMagick identify not available");
        return;
    }

    int ret;
    struct item {
        int width;
        int height;
        unsigned char *pattern;
        int repeat;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 1, 1, "1", 0 },
        /*  1*/ { 2, 1, "11", 0 },
        /*  2*/ { 2, 2, "10", 1 },
        /*  3*/ { 3, 1, "101", 0 },
        /*  4*/ { 3, 2, "101010", 0 },
        /*  5*/ { 3, 3, "101010101", 0 },
        /*  6*/ { 8, 2, "CBMWKRYGGYRKWMBC", 0 },
    };
    int data_size = ARRAY_SIZE(data);

    char *png = "out.png";
    char escaped[1024];
    int escaped_size = 1024;

    char data_buf[8 * 2 + 1];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, png);

        symbol->bitmap_width = data[i].width;
        symbol->bitmap_height = data[i].height;
        symbol->debug |= debug;

        int size = data[i].width * data[i].height;
        assert_nonzero(size < (int) sizeof(data_buf), "i:%d png_pixel_plot size %d < sizeof(data_buf) %d\n", i, size, (int) sizeof(data_buf));

        if (data[i].repeat) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, size);
        } else {
            strcpy(data_buf, data[i].pattern);
        }
        assert_equal(size, (int) strlen(data_buf), "i:%d png_pixel_plot size %d != strlen(data_buf) %d\n", i, size, (int) strlen(data_buf));

        symbol->bitmap = data_buf;

        ret = png_pixel_plot(symbol, data_buf);
        assert_zero(ret, "i:%d png_pixel_plot ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

        ret = testUtilVerifyIdentify(symbol->outfile, debug);
        assert_zero(ret, "i:%d identify %s ret %d != 0\n", i, symbol->outfile, ret);

        assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

        symbol->bitmap = NULL;

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_print(int index, int generate, int debug) {

    testStart("");

    int have_identify = testUtilHaveIdentify();

    int ret;
    struct item {
        int symbology;
        int option_1;
        int option_2;
        unsigned char* data;
        char* expected_file;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, 3, -1, "AAAAAAAAA", "../data/png/codablockf_3rows.png" },
        /*  1*/ { BARCODE_EANX, -1, -1, "9771384524017+12", "../data/png/ean13_2addon_ggs_5.2.2.5.1-2.png" },
        /*  2*/ { BARCODE_EANX, -1, -1, "9780877799306+54321", "../data/png/ean13_5addon_ggs_5.2.2.5.2-2.png" },
        /*  3*/ { BARCODE_UPCA, -1, -1, "012345678905+24", "../data/png/upca_2addon_ggs_5.2.6.6-5.png" },
        /*  4*/ { BARCODE_UPCA, -1, -1, "614141234417+12345", "../data/png/upca_5addon.png" },
        /*  5*/ { BARCODE_UPCE, -1, -1, "1234567+12", "../data/png/upce_2addon.png" },
        /*  6*/ { BARCODE_UPCE, -1, -1, "1234567+12345", "../data/png/upce_5addon.png" },
        /*  7*/ { BARCODE_EANX, -1, -1, "1234567+12", "../data/png/ean8_2addon.png" },
        /*  8*/ { BARCODE_EANX, -1, -1, "1234567+12345", "../data/png/ean8_5addon.png" },
        /*  9*/ { BARCODE_EANX, -1, -1, "12345", "../data/png/ean5.png" },
        /* 10*/ { BARCODE_EANX, -1, -1, "12", "../data/png/ean2.png" },
    };
    int data_size = ARRAY_SIZE(data);

    char* data_dir = "../data/png";
    char* png = "out.png";
    char escaped[1024];
    int escaped_size = 1024;

    if (generate) {
        if (!testUtilExists(data_dir)) {
            ret = mkdir(data_dir, 0755);
            assert_zero(ret, "mkdir(%s) ret %d != 0\n", data_dir, ret);
        }
    }

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, png);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, \"%s\", \"%s\"},\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].option_1, data[i].option_2,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].expected_file);
            ret = rename(symbol->outfile, data[i].expected_file);
            assert_zero(ret, "i:%d rename(%s, %s) ret %d != 0\n", i, symbol->outfile, data[i].expected_file, ret);
            if (have_identify) {
                ret = testUtilVerifyIdentify(data[i].expected_file, debug);
                assert_zero(ret, "i:%d %s identify %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), data[i].expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(data[i].expected_file), "i:%d testUtilExists(%s) == 0\n", i, data[i].expected_file);

            ret = testUtilCmpPngs(symbol->outfile, data[i].expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpPngs(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, data[i].expected_file, ret);
            assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_pixel_plot", test_pixel_plot, 1, 0, 1 },
        { "test_print", test_print, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
