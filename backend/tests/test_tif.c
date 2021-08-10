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

INTERNAL int tif_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);

// For overview when debugging: ./test_tiff -f pixel_plot -d 5
static void test_pixel_plot(int index, int debug) {

    struct item {
        int width;
        int height;
        char *pattern;
        int repeat;
        int no_identify; // identify fails for some valid TIFFs (eg. RGB with LZW and large rows)
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 1, 1, "1", 0, 0, 0 },
        /*  1*/ { 2, 1, "11", 0, 0, 0 },
        /*  2*/ { 1, 2, "11", 0, 0, 0 },
        /*  3*/ { 2, 2, "10", 1, 0, 0 },
        /*  4*/ { 3, 1, "101", 0, 0, 0 },
        /*  5*/ { 1, 3, "101", 0, 0, 0 },
        /*  6*/ { 4, 1, "1010", 0, 0, 0 },
        /*  7*/ { 1, 4, "1010", 0, 0, 0 },
        /*  8*/ { 5, 1, "10101", 0, 0, 0 },
        /*  9*/ { 1, 5, "10101", 0, 0, 0 },
        /* 10*/ { 3, 2, "101", 1, 0, 0 },
        /* 11*/ { 100, 2, "10", 1, 0, 0 },
        /* 12*/ { 2, 100, "10", 1, 0, 0 },
        /* 13*/ { 3, 3, "101010101", 0, 0, 0 },
        /* 14*/ { 4, 3, "10", 1, 0, 0 },
        /* 15*/ { 3, 4, "10", 1, 0, 0 },
        /* 16*/ { 45, 44, "10", 1, 0, 0 }, // Strip Count 1, Rows Per Strip 44 (45 * 44 * 4 == 7920)
        /* 17*/ { 45, 45, "10", 1, 0, 0 }, // Strip Count 1, Rows Per Strip 45 (45 * 45 * 4 == 8100)
        /* 18*/ { 46, 45, "10", 1, 0, 0 }, // Strip Count 2, Rows Per Strip 44 (46 * 45 * 4 == 8280)
        /* 19*/ { 46, 46, "10", 1, 0, 0 }, // Strip Count 2, Rows Per Strip 44
        /* 20*/ { 2048, 1, "10", 1, 1, 0 }, // Strip Count 1, Rows Per Strip 1 (2048 * 4 == 8192)
        /* 21*/ { 1, 2048, "10", 1, 0, 0 }, // Strip Count 1, Rows Per Strip 2048
        /* 22*/ { 2048, 2, "10", 1, 1, 0 }, // Strip Count 2, Rows Per Strip 1
        /* 23*/ { 2, 2048, "10", 1, 0, 0 }, // Strip Count 2, Rows Per Strip 1024 (2 * 1024 * 4 == 8192)
        /* 24*/ { 2048, 3, "10", 1, 1, 0 }, // Strip Count 3, Rows Per Strip 1
        /* 25*/ { 3, 2048, "10", 1, 0, 0 }, // Strip Count 4, Rows Per Strip 682 ((3 * 682 + 2) * 4 == 8192)
        /* 26*/ { 2049, 4, "10", 1, 1, 0 }, // Strip Count 4, Rows Per Strip 1 (2049 * 1 * 4 == 8196) - large rows in 1 strip, even if > 8192
        /* 27*/ { 4, 2049, "10", 1, 0, 0 }, // Strip Count 5, Rows Per Strip 512 ((4 * 512 + 1) * 4 == 8196)
        /* 28*/ { 4096, 1, "10", 1, 1, 0 }, // Strip Count 1, Rows Per Strip 1
        /* 29*/ { 1, 4096, "10", 1, 0, 0 }, // Strip Count 2, Rows Per Strip 2048
        /* 30*/ { 4096, 2, "10", 1, 1, 0 }, // Strip Count 2, Rows Per Strip 1
        /* 31*/ { 2, 4096, "10", 1, 0, 0 }, // Strip Count 4, Rows Per Strip 1024
        /* 32*/ { 8192, 2, "10", 1, 1, 0 }, // Strip Count 2, Rows Per Strip 1
        /* 33*/ { 2, 8192, "10", 1, 0, 0 }, // Strip Count 8, Rows Per Strip 1024
        /* 34*/ { ZINT_MAX_DATA_LEN, 1, "10", 1, 1, 0 }, // Strip Count 1, Rows Per Strip 1
        /* 35*/ { 1, ZINT_MAX_DATA_LEN, "10", 1, 0, 0 }, // Strip Count 9, Rows Per Strip 2048
        /* 36*/ { ZINT_MAX_DATA_LEN, 2, "10", 1, 1, 0 }, // Strip Count 2, Rows Per Strip 1
        /* 37*/ { 2, ZINT_MAX_DATA_LEN, "10", 1, 0, 0 }, // Strip Count 17, Rows Per Strip 1024
    };
    int data_size = ARRAY_SIZE(data);
    int i, ret;
    struct zint_symbol *symbol;

    char *tif = "out.tif";

    char data_buf[65536];

    int have_tiffinfo = testUtilHaveTiffInfo();
    int have_identify = testUtilHaveIdentify();

    testStart("test_pixel_plot");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    for (i = 0; i < data_size; i++) {
        int size;

        if (index != -1 && i != index) continue;

        strcpy(symbol->outfile, tif);

        symbol->bitmap_width = data[i].width;
        symbol->bitmap_height = data[i].height;
        symbol->symbology = BARCODE_ULTRA; // Use ULTRA with alpha background to force RGB
        strcpy(symbol->bgcolour, "FFFFFFEE");
        symbol->debug |= debug;

        size = data[i].width * data[i].height;
        assert_nonzero(size < (int) sizeof(data_buf), "i:%d tif_pixel_plot size %d >= sizeof(data_buf) %d\n", i, size, (int) sizeof(data_buf));

        if (data[i].repeat) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, size);
        } else {
            strcpy(data_buf, data[i].pattern);
        }
        assert_equal(size, (int) strlen(data_buf), "i:%d tif_pixel_plot size %d != strlen(data_buf) %d\n", i, size, (int) strlen(data_buf));

        symbol->bitmap = (unsigned char *) data_buf;

        ret = tif_pixel_plot(symbol, (unsigned char *) data_buf);
        assert_equal(ret, data[i].ret, "i:%d tif_pixel_plot ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            if (have_tiffinfo) {
                ret = testUtilVerifyTiffInfo(symbol->outfile, debug);
                assert_zero(ret, "i:%d tiffinfo %s ret %d != 0\n", i, symbol->outfile, ret);
            } else if (have_identify && !data[i].no_identify) {
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
    }

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_print(int index, int generate, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int border_width;
        int output_options;
        int whitespace_width;
        int whitespace_height;
        int show_hrt;
        int option_1;
        int option_2;
        int height;
        float scale;
        char *fgcolour;
        char *bgcolour;
        char *data;
        char *composite;
        char *expected_file;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, 1, -1, -1, -1, -1, 0, 0, "112233", "EEDDCC", "A", "", "code128_fgbg.tif", "" },
        /*  1*/ { BARCODE_CODE128, -1, -1, -1, 1, -1, -1, -1, -1, 0, 0, "FFFFFF", "000000", "A", "", "code128_reverse.tif", "" },
        /*  2*/ { BARCODE_CODE128, -1, -1, CMYK_COLOUR, 1, -1, -1, -1, -1, 0, 0, "112233", "CCDDEE", "A", "", "code128_cmyk_fgbg.tif", "" },
        /*  3*/ { BARCODE_CODE128, -1, -1, -1, 1, -1, -1, -1, -1, 0, 0, "C00000", "FEDCBACC", "A", "", "code128_bgalpha.tif", "" },
        /*  4*/ { BARCODE_CODE128, -1, -1, -1, 1, -1, -1, -1, -1, 0, 0, "00000099", "FEDCBA", "A", "", "code128_fgalpha.tif", "" },
        /*  5*/ { BARCODE_CODE128, -1, -1, -1, 1, -1, -1, -1, -1, 0, 0, "00000099", "FEDCBACC", "A", "", "code128_fgbgalpha.tif", "" },
        /*  6*/ { BARCODE_CODE128, -1, -1, CMYK_COLOUR, 1, -1, -1, -1, -1, 0, 0, "C00000", "FEDCBA", "A", "", "code128_cmyk.tif", "" },
        /*  7*/ { BARCODE_CODE128, -1, -1, CMYK_COLOUR, 1, -1, -1, -1, -1, 0, 0, "C0000099", "FEDCBACC", "A", "", "code128_cmyk_fgbgalpha.tif", "" },
        /*  8*/ { BARCODE_ULTRA, -1, -1, -1, 1, -1, -1, -1, -1, 0, 0, "C00000", "FEDCBACC", "1234", "", "ultra_bgalpha.tif", "" },
        /*  9*/ { BARCODE_ULTRA, -1, -1, CMYK_COLOUR, 1, -1, -1, -1, -1, 0, 0, "C00000", "FEDCBACC", "1234", "", "ultra_cmyk_bgalpha.tif", "" },
        /* 10*/ { BARCODE_ULTRA, -1, -1, -1, 1, -1, -1, -1, -1, 0, 0, "000000BB", "FEDCBA", "1234", "", "ultra_fgalpha.tif", "" },
        /* 11*/ { BARCODE_ULTRA, -1, -1, -1, 1, -1, -1, -1, -1, 0, 0, "000000BB", "FEDCBACC", "1234", "", "ultra_fgbgalpha.tif", "" },
        /* 12*/ { BARCODE_ULTRA, -1, -1, -1, 1, -1, -1, -1, -1, 0, 0, "000000BB", "", "1234", "", "ultra_fgalpha_nobg.tif", "" },
        /* 13*/ { BARCODE_ULTRA, -1, -1, -1, 1, -1, -1, -1, -1, 0, 0, "", "FEDCBACC", "1234", "", "ultra_bgalpha_nofg.tif", "" },
        /* 14*/ { BARCODE_ULTRA, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0.5f, "", "", "1", "", "ultra_odd.tif", "" },
        /* 15*/ { BARCODE_ULTRA, -1, -1, CMYK_COLOUR, 1, -1, -1, -1, -1, 0, 0, "", "", "1234", "", "ultra_cmyk.tif", "" },
        /* 16*/ { BARCODE_ULTRA, -1, 1, BARCODE_BOX, 1, 1, -1, -1, -1, 0, 0, "FF0000", "0000FF", "1234", "", "ultra_fgbg_hvwsp1_box1.tif", "" },
        /* 17*/ { BARCODE_HANXIN, UNICODE_MODE, -1, -1, -1, -1, -1, 4, 84, 0, 2, "", "", "1", "", "hanxin_v84_l4_scale2.tif", "" },
        /* 18*/ { BARCODE_AZTEC, -1, -1, -1, -1, -1, -1, -1, 32, 0, 0, "4BE055", "", "1", "", "aztec_v32_fg.tif", "" },
        /* 19*/ { BARCODE_DAFT, -1, -1, -1, -1, -1, -1, -1, -1, 8, 0.5f, "", "", "F", "", "daft_height8_scale0.5.tif", "" },
        /* 20*/ { BARCODE_DAFT, -1, -1, -1, -1, -1, -1, -1, -1, 1, 0.5f, "", "", "DAFT", "", "daft_height1_scale0.5.tif", "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    const char *data_dir = "/backend/tests/data/tif";
    const char *tif = "out.tif";
    char expected_file[1024];
    char escaped[1024];
    int escaped_size = 1024;
    char *text;

    int have_tiffinfo = testUtilHaveTiffInfo();
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
        int text_length;

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }
        if (data[i].height) {
            symbol->height = data[i].height;
        }
        if (data[i].scale) {
            symbol->scale = data[i].scale;
        }
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }
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
        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        text_length = (int) strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, text_length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, tif);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        assert_nonzero(testUtilDataPath(expected_file, sizeof(expected_file), data_dir, data[i].expected_file), "i:%d testUtilDataPath == 0\n", i);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, %s, %d, %d, %d, %d, %d, %d, %.5g, \"%s\",\"%s\",  \"%s\", \"%s\", \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].border_width, testUtilOutputOptionsName(data[i].output_options),
                    data[i].whitespace_width, data[i].whitespace_height, data[i].show_hrt, data[i].option_1, data[i].option_2,
                    data[i].height, data[i].scale, data[i].fgcolour, data[i].bgcolour,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].composite, data[i].expected_file, data[i].comment);
            ret = testUtilRename(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d testUtilRename(%s, %s) ret %d != 0\n", i, symbol->outfile, expected_file, ret);
            if (have_tiffinfo) {
                ret = testUtilVerifyTiffInfo(expected_file, debug);
                assert_zero(ret, "i:%d %s tiffinfo %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
            } else if (have_identify) {
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

    strcpy(symbol.outfile, "nosuch_dir/out.tif");

    ret = tif_pixel_plot(&symbol, data);
    assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "tif_pixel_plot ret %d != ZINT_ERROR_FILE_ACCESS (%d) (%s)\n", ret, ZINT_ERROR_FILE_ACCESS, symbol.errtxt);

    symbol.output_options |= BARCODE_STDOUT;

    ret = tif_pixel_plot(&symbol, data);
    printf(" - ignore (TIF to stdout)\n"); fflush(stdout);
    assert_zero(ret, "tif_pixel_plot ret %d != 0 (%s)\n", ret, symbol.errtxt);

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
