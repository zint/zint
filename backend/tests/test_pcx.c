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

static void test_print(int index, int generate, int debug) {

    struct item {
        int symbology;
        int border_width;
        int output_options;
        int whitespace_width;
        int whitespace_height;
        int option_1;
        int option_2;
        char *fgcolour;
        char *bgcolour;
        float scale;
        char *data;
        char *expected_file;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_GRIDMATRIX, -1, -1, -1, -1, -1, -1, "C3C3C3", "", 0.75, "Grid Matrix", "gridmatrix_fg_0.75.pcx" },
        /*  1*/ { BARCODE_CODABLOCKF, -1, -1, -1, -1, -1, 20, "FFFFFF", "000000", 0, "1234567890123456789012345678901234567890", "codeblockf_reverse.pcx" },
        /*  2*/ { BARCODE_QRCODE, -1, -1, -1, -1, 2, 1, "", "D2E3F4", 0, "1234567890", "qr_bg.pcx" },
        /*  3*/ { BARCODE_ULTRA, 1, BARCODE_BOX, 1, 1, -1, -1, "FF0000", "0000FF", 0, "ULTRACODE_123456789!", "ultra_fg_bg_hvwsp1_box1.pcx" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    const char *data_dir = "/backend/tests/data/pcx";
    char *pcx = "out.pcx";
    char expected_file[4096];
    char escaped[1024];
    int escaped_size = 1024;

    int have_identify = testUtilHaveIdentify();

    testStart("test_pcx");

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
        if (*data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (*data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }
        if (data[i].scale != 0) {
            symbol->scale = data[i].scale;
        }
        symbol->debug |= debug;

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, pcx);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        assert_nonzero(testUtilDataPath(expected_file, sizeof(expected_file), data_dir, data[i].expected_file), "i:%d testUtilDataPath == 0\n", i);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %s, %d, %d, %d, %d, \"%s\", \"%s\", \"%s\", \"%s\"},\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].border_width, testUtilOutputOptionsName(data[i].output_options),
                    data[i].whitespace_width, data[i].whitespace_height,
                    data[i].option_1, data[i].option_2, data[i].fgcolour, data[i].bgcolour,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].expected_file);
            ret = testUtilRename(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d testUtilRename(%s, %s) ret %d != 0 (%d: %s)\n", i, symbol->outfile, expected_file, ret, errno, strerror(errno));
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

INTERNAL int pcx_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);

static void test_outfile(void) {
    int ret;
    struct zint_symbol symbol = {0};
    unsigned char data[] = { "1" };

    testStart("test_outfile");

    symbol.symbology = BARCODE_CODE128;
    symbol.bitmap = data;
    symbol.bitmap_width = symbol.bitmap_height = 1;

    strcpy(symbol.outfile, "nosuch_dir/out.pcx");

    ret = pcx_pixel_plot(&symbol, data);
    assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "pcx_pixel_plot ret %d != ZINT_ERROR_FILE_ACCESS (%d) (%s)\n", ret, ZINT_ERROR_FILE_ACCESS, symbol.errtxt);

    symbol.output_options |= BARCODE_STDOUT;

    ret = pcx_pixel_plot(&symbol, data);
    printf(" - ignore (PCX to stdout)\n"); fflush(stdout);
    assert_zero(ret, "pcx_pixel_plot ret %d != 0 (%s)\n", ret, symbol.errtxt);

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_print", test_print, 1, 1, 1 },
        { "test_outfile", test_outfile, 0, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
