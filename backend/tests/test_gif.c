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

INTERNAL int gif_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);

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
        /*  2*/ { 3, 1, "101", 0 },
        /*  3*/ { 4, 1, "1010", 0 },
        /*  4*/ { 5, 1, "10101", 0 },
        /*  5*/ { 3, 2, "101010", 0 },
        /*  6*/ { 3, 3, "101010101", 0 },
        /*  7*/ { 8, 2, "CBMWKRYGGYRKWMBC", 0 },
        /*  8*/ { 20, 30, "WWCWBWMWRWYWGWKCCWCMCRCYCGCKBWBCBBMBRBYBGBKMWMCMBMMRMYMGMKRWRCRBRMRRYRGRKYWYCYBYMYRYYGYKGWGCGBGMGRGYGGKKWKCKBKMKRKYKGKK", 1 }, // Single LZW block, size 255
        /*  9*/ { 19, 32, "WWCWBWMWRWYWGWKCCWCMCRCYCGCKBWBCBBMBRBYBGBKMWMCMBMMRMYMGMKRWRCRBRMRRYRGRKYWYCYBYMYRYYGYKGWGCGBGMGRGYGGKKWK", 1 }, // Two LZW blocks, last size 1
    };
    int data_size = ARRAY_SIZE(data);

    char *gif = "out.gif";

    char data_buf[19 * 32 + 1]; // 19 * 32 == 608

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, gif);

        symbol->bitmap_width = data[i].width;
        symbol->bitmap_height = data[i].height;
        symbol->debug |= debug;

        int size = data[i].width * data[i].height;
        assert_nonzero(size < (int) sizeof(data_buf), "i:%d gif_pixel_plot size %d < sizeof(data_buf) %d\n", i, size, (int) sizeof(data_buf));

        if (data[i].repeat) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, size);
        } else {
            strcpy(data_buf, data[i].pattern);
        }
        assert_equal(size, (int) strlen(data_buf), "i:%d gif_pixel_plot size %d != strlen(data_buf) %d\n", i, size, (int) strlen(data_buf));

        symbol->bitmap = (unsigned char *) data_buf;

        ret = gif_pixel_plot(symbol, (unsigned char *) data_buf);
        assert_zero(ret, "i:%d gif_pixel_plot ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

        ret = testUtilVerifyIdentify(symbol->outfile, debug);
        assert_zero(ret, "i:%d identify %s ret %d != 0\n", i, symbol->outfile, ret);

        assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

        symbol->bitmap = NULL;

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_pixel_plot", test_pixel_plot, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
