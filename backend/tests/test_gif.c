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

INTERNAL int gif_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);

static void test_pixel_plot(int index, int debug) {

    struct item {
        int width;
        int height;
        char *pattern;
        int repeat;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 1, 1, "1", 0, 0 },
        /*  1*/ { 2, 1, "11", 0, 0 },
        /*  2*/ { 3, 1, "101", 0, 0 },
        /*  3*/ { 4, 1, "1010", 0, 0 },
        /*  4*/ { 5, 1, "10101", 0, 0 },
        /*  5*/ { 3, 2, "101010", 0, 0 },
        /*  6*/ { 3, 3, "101010101", 0, 0 },
        /*  7*/ { 8, 2, "CBMWKRYGGYRKWMBC", 0, 0 },
        /*  8*/ { 20, 30, "WWCWBWMWRWYWGWKCCWCMCRCYCGCKBWBCBBMBRBYBGBKMWMCMBMMRMYMGMKRWRCRBRMRRYRGRKYWYCYBYMYRYYGYKGWGCGBGMGRGYGGKKWKCKBKMKRKYKGKK", 1, 0 }, // Single LZW block, size 255
        /*  9*/ { 19, 32, "WWCWBWMWRWYWGWKCCWCMCRCYCGCKBWBCBBMBRBYBGBKMWMCMBMMRMYMGMKRWRCRBRMRRYRGRKYWYCYBYMYRYYGYKGWGCGBGMGRGYGGKKWK", 1, 0 }, // Two LZW blocks, last size 1
        /* 10*/ { 1, 1, "D", 0, ZINT_ERROR_INVALID_DATA },
    };
    int data_size = ARRAY_SIZE(data);
    int i, ret;
    struct zint_symbol *symbol;

    char *gif = "out.gif";

    char data_buf[19 * 32 + 1]; // 19 * 32 == 608

    int have_identify = testUtilHaveIdentify();

    testStart("test_pixel_plot");

    for (i = 0; i < data_size; i++) {
        int size;

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, gif);

        symbol->bitmap_width = data[i].width;
        symbol->bitmap_height = data[i].height;
        symbol->debug |= debug;

        size = data[i].width * data[i].height;
        assert_nonzero(size < (int) sizeof(data_buf), "i:%d gif_pixel_plot size %d < sizeof(data_buf) %d\n", i, size, (int) sizeof(data_buf));

        if (data[i].repeat) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, size);
        } else {
            strcpy(data_buf, data[i].pattern);
        }
        assert_equal(size, (int) strlen(data_buf), "i:%d gif_pixel_plot size %d != strlen(data_buf) %d\n", i, size, (int) strlen(data_buf));

        symbol->bitmap = (unsigned char *) data_buf;

        ret = gif_pixel_plot(symbol, (unsigned char *) data_buf);
        assert_equal(ret, data[i].ret, "i:%d gif_pixel_plot ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            if (have_identify) {
                ret = testUtilVerifyIdentify(symbol->outfile, debug);
                assert_zero(ret, "i:%d identify %s ret %d != 0\n", i, symbol->outfile, ret);
            }
            if (!(debug & ZINT_DEBUG_TEST_KEEP_OUTFILE)) {
                assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);
            }
        } else {
            if (!(debug & ZINT_DEBUG_TEST_KEEP_OUTFILE)) {
                (void) remove(symbol->outfile);
            }
        }

        symbol->bitmap = NULL;

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_print(int index, int generate, int debug) {

    struct item {
        int symbology;
        int border_width;
        int output_options;
        int whitespace_width;
        int whitespace_height;
        int option_1;
        int option_2;
        float scale;
        float dot_size;
        char *fgcolour;
        char *bgcolour;
        char *data;
        char *expected_file;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "12", "dotcode_1.0.gif" },
        /*  1*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 0, 0.1, "", "", "12", "dotcode_1.0_ds0.1.gif" },
        /*  2*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 0, 1.1, "", "", "12", "dotcode_1.0_ds1.1.gif" },
        /*  3*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 1.5, 0, "", "", "12", "dotcode_1.5.gif" },
        /*  4*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 1.5, 0.4, "", "", "12", "dotcode_1.5_ds0.4.gif" },
        /*  5*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 1.5, 1.1, "", "", "12", "dotcode_1.5_ds1.1.gif" },
        /*  6*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 1.5, 2.1, "", "", "12", "dotcode_1.5_ds2.1.gif" },
        /*  7*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 2, 0, "", "", "12", "dotcode_2.0.gif" },
        /*  8*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 2, 0.9, "", "", "12", "dotcode_2.0_ds0.9.gif" },
        /*  9*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 2, 1.1, "", "", "12", "dotcode_2.0_ds1.1.gif" },
        /* 10*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 3, 0, "", "", "12", "dotcode_3.0.gif" },
        /* 11*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 3, 0.4, "", "", "12", "dotcode_3.0_ds0.4.gif" },
        /* 12*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 3, 1.1, "", "", "12", "dotcode_3.0_ds1.1.gif" },
        /* 13*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 3.5, 0, "", "", "12", "dotcode_3.5.gif" },
        /* 14*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 3.5, 0.4, "", "", "12", "dotcode_3.5_ds0.4.gif" },
        /* 15*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 3.5, 1.1, "", "", "12", "dotcode_3.5_ds1.1.gif" },
        /* 16*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 5, 0, "", "", "12", "dotcode_5.0.gif" },
        /* 17*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 5, 0.2, "", "", "12", "dotcode_5.0_ds0.2.gif" },
        /* 18*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 5, 1.1, "", "", "12", "dotcode_5.0_ds1.1.gif" },
        /* 19*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 5, 1.7, "", "", "12", "dotcode_5.0_ds1.7.gif" },
        /* 20*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 0, 0, "2674C344", "FDFFC2CC", "12", "dotcode_bgfgalpha.gif" },
        /* 21*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, 0, 0, "00000000", "FFFFFF00", "12", "dotcode_bgfgtrans.gif" },
        /* 22*/ { BARCODE_ULTRA, 1, BARCODE_BOX, 1, 1, -1, -1, 0, 0, "0000FF", "FF0000", "12", "ultra_fgbg_hvwsp1_box1.gif" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    const char *data_dir = "/backend/tests/data/gif";
    const char *gif = "out.gif";
    char expected_file[4096];
    char escaped[1024];
    int escaped_size = 1024;

    int have_identify = testUtilHaveIdentify();

    testStart("test_print");

    if (generate) {
        char data_dir_path[1024];
        assert_nonzero(testUtilDataPath(data_dir_path, sizeof(data_dir_path), data_dir, NULL), "testUtilDataPath(%s) == 0\n", data_dir);
        if (!testUtilDirExists(data_dir_path)) {
            ret = testUtilMkDir(data_dir_path);
            assert_zero(ret, "testUtilMkDir(%s) ret %d != 0 (%d: %s)\n", data_dir_path, ret, errno, strerror(errno));
        }
    }

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }
        if (data[i].whitespace_width != -1) {
            symbol->whitespace_width = data[i].whitespace_width;
        }
        if (data[i].whitespace_height != -1) {
            symbol->whitespace_height = data[i].whitespace_height;
        }
        if (data[i].scale) {
            symbol->scale = data[i].scale;
        }
        if (data[i].dot_size) {
            symbol->dot_size = data[i].dot_size;
        }
        if (*data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (*data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, gif);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        assert_nonzero(testUtilDataPath(expected_file, sizeof(expected_file), data_dir, data[i].expected_file), "i:%d testUtilDataPath == 0\n", i);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %s, %d, %d, %d, %d, %.5g, %.5g, \"%s\", \"%s\", \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].border_width, testUtilOutputOptionsName(data[i].output_options),
                    data[i].whitespace_width, data[i].whitespace_height,
                    data[i].option_1, data[i].option_2, data[i].scale, data[i].dot_size, data[i].fgcolour, data[i].bgcolour,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].expected_file);
            ret = testUtilRename(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d testUtilRename(%s, %s) ret %d != 0\n", i, symbol->outfile, expected_file, ret);
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

static void test_outfile(void) {
    int ret;
    struct zint_symbol symbol = {0};
    unsigned char data[] = { "1" };

    testStart("test_outfile");

    symbol.symbology = BARCODE_CODE128;
    symbol.bitmap = data;
    symbol.bitmap_width = symbol.bitmap_height = 1;

    strcpy(symbol.outfile, "nosuch_dir/out.gif");

    ret = gif_pixel_plot(&symbol, data);
    assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "gif_pixel_plot ret %d != ZINT_ERROR_FILE_ACCESS (%d) (%s)\n", ret, ZINT_ERROR_FILE_ACCESS, symbol.errtxt);

    symbol.output_options |= BARCODE_STDOUT;

    ret = gif_pixel_plot(&symbol, data);
    printf(" - ignore (GIF to stdout)\n"); fflush(stdout);
    assert_zero(ret, "gif_pixel_plot ret %d != 0 (%s)\n", ret, symbol.errtxt);

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_pixel_plot", test_pixel_plot, 1, 0, 1 },
        { "test_print", test_print, 1, 1, 1 },
        { "test_outfile", test_outfile, 0, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
