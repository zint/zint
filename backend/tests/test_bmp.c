/*
    libzint - the open source barcode library
    Copyright (C) 2020 - 2021 Robin Stuart <rstuart114@gmail.com>

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

INTERNAL int bmp_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);

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
        char *pattern;
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
        /*  6*/ { 4, 1, "1001", 0 },
        /*  7*/ { 4, 3, "1001", 1 },
        /*  8*/ { 5, 1, "10101", 0 },
        /*  9*/ { 8, 2, "CBMWKRYGGYRKWMBC", 0 },
    };
    int data_size = ARRAY_SIZE(data);

    char *bmp = "out.bmp";

    char data_buf[8 * 2 + 1];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, bmp);

        symbol->bitmap_width = data[i].width;
        symbol->bitmap_height = data[i].height;
        symbol->debug |= debug;

        int size = data[i].width * data[i].height;
        assert_nonzero(size < (int) sizeof(data_buf), "i:%d bmp_pixel_plot size %d < sizeof(data_buf) %d\n", i, size, (int) sizeof(data_buf));

        if (data[i].repeat) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, size);
        } else {
            strcpy(data_buf, data[i].pattern);
        }
        assert_equal(size, (int) strlen(data_buf), "i:%d bmp_pixel_plot size %d != strlen(data_buf) %d\n", i, size, (int) strlen(data_buf));

        symbol->bitmap = (unsigned char *) data_buf;

        ret = bmp_pixel_plot(symbol, (unsigned char *) data_buf);
        assert_zero(ret, "i:%d bmp_pixel_plot ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

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
        int whitespace_width;
        int whitespace_height;
        int option_1;
        int option_2;
        char *fgcolour;
        char *bgcolour;
        char *data;
        char *expected_file;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, 5, -1, -1, -1, "147AD0", "FC9630", "123", "pdf417_fg_bg.bmp" },
        /*  1*/ { BARCODE_ULTRA, 5, -1, -1, -1, "147AD0", "FC9630", "123", "ultracode_fg_bg.bmp" },
        /*  2*/ { BARCODE_PDF417COMP, 2, 2, -1, -1, "", "", "123", "pdf417comp_hvwsp2.bmp" },
    };
    int data_size = ARRAY_SIZE(data);

    const char *data_dir = "/backend/tests/data/bmp";
    const char *bmp = "out.bmp";
    char expected_file[4096];
    char escaped[1024];
    int escaped_size = 1024;

    if (generate) {
        if (!testUtilExists(data_dir)) {
            ret = testutil_mkdir(data_dir, 0755);
            assert_zero(ret, "testutil_mkdir(%s) ret %d != 0 (%d: %s)\n", data_dir, ret, errno, strerror(errno));
        }
    }

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].whitespace_width != -1) {
            symbol->whitespace_width = data[i].whitespace_width;
        }
        if (data[i].whitespace_height != -1) {
            symbol->whitespace_height = data[i].whitespace_height;
        }
        if (*data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (*data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, bmp);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        assert_nonzero(testUtilDataPath(expected_file, sizeof(expected_file), data_dir, data[i].expected_file), "i:%d testUtilDataPath == 0\n", i);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, %d, %d, \"%s\", \"%s\", \"%s\", \"%s\"},\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].whitespace_width, data[i].whitespace_height,
                    data[i].option_1, data[i].option_2, data[i].fgcolour, data[i].bgcolour,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].expected_file);
            ret = rename(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d rename(%s, %s) ret %d != 0\n", i, symbol->outfile, expected_file, ret);
            if (have_identify) {
                ret = testUtilVerifyIdentify(expected_file, debug);
                assert_zero(ret, "i:%d %s identify %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(expected_file), "i:%d testUtilExists(%s) == 0\n", i, expected_file);

            ret = testUtilCmpBins(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpBins(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
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
