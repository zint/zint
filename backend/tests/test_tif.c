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

extern int tif_pixel_plot(struct zint_symbol *symbol, char *pixelbuf);

// For overview when debugging: ./test_tiff -f pixel_plot -d 5
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
        /*  2*/ { 1, 2, "11", 0 },
        /*  3*/ { 2, 2, "10", 1 },
        /*  4*/ { 3, 1, "101", 0 },
        /*  5*/ { 1, 3, "101", 0 },
        /*  6*/ { 4, 1, "1010", 0 },
        /*  7*/ { 1, 4, "1010", 0 },
        /*  8*/ { 5, 1, "10101", 0 },
        /*  9*/ { 1, 5, "10101", 0 },
        /* 10*/ { 3, 2, "101", 1 },
        /* 11*/ { 100, 2, "10", 1 },
        /* 12*/ { 2, 100, "10", 1 },
        /* 13*/ { 3, 3, "101010101", 0 },
        /* 14*/ { 4, 3, "10", 1 },
        /* 15*/ { 3, 4, "10", 1 },
        /* 16*/ { 52, 51, "10", 1 }, // Strip Count 1, Rows Per Strip 51 (52 * 51 * 3 == 7956)
        /* 17*/ { 52, 52, "10", 1 }, // Strip Count 1, Rows Per Strip 52 (52 * 52 * 3 == 8112)
        /* 18*/ { 53, 52, "10", 1 }, // Strip Count 2, Rows Per Strip 51 (53 * 51 * 3 == 8109)
        /* 19*/ { 53, 53, "10", 1 }, // Strip Count 2, Rows Per Strip 51
        /* 20*/ { 2730, 1, "10", 1 }, // Strip Count 1, Rows Per Strip 1 (2730 * 1 * 3 == 8190)
        /* 21*/ { 1, 2730, "10", 1 }, // Strip Count 1, Rows Per Strip 2730
        /* 22*/ { 2730, 2, "10", 1 }, // Strip Count 2, Rows Per Strip 1
        /* 23*/ { 2, 2730, "10", 1 }, // Strip Count 2, Rows Per Strip 1365 (2 * 1365 * 3 == 8190)
        /* 24*/ { 2730, 3, "10", 1 }, // Strip Count 3, Rows Per Strip 1
        /* 25*/ { 3, 2730, "10", 1 }, // Strip Count 3, Rows Per Strip 910 (3 * 910 * 3 == 8190)
        /* 26*/ { 2731, 4, "10", 1 }, // Strip Count 4, Rows Per Strip 1 (2731 * 1 * 3 == 8193) - large rows in 1 strip, even if > 8192
        /* 27*/ { 4, 2731, "10", 1 }, // Strip Count 5, Rows Per Strip 682 (4 * 682 * 3 == 8184)
    };
    int data_size = ARRAY_SIZE(data);

    char *tif = "out.tif";
    char escaped[1024];
    int escaped_size = 1024;

    char data_buf[2731 * 4 + 1];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, tif);

        symbol->bitmap_width = data[i].width;
        symbol->bitmap_height = data[i].height;
        symbol->debug |= debug;

        int size = data[i].width * data[i].height;
        assert_nonzero(size < (int) sizeof(data_buf), "i:%d tif_pixel_plot size %d < sizeof(data_buf) %d\n", i, size, (int) sizeof(data_buf));

        if (data[i].repeat) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, size);
        } else {
            strcpy(data_buf, data[i].pattern);
        }
        assert_equal(size, (int) strlen(data_buf), "i:%d tif_pixel_plot size %d != strlen(data_buf) %d\n", i, size, (int) strlen(data_buf));

        symbol->bitmap = data_buf;

        ret = tif_pixel_plot(symbol, data_buf);
        assert_zero(ret, "i:%d tif_pixel_plot ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

        ret = testUtilVerifyIdentify(symbol->outfile, debug);
        assert_zero(ret, "i:%d identify %s ret %d != 0\n", i, symbol->outfile, ret);

        if (!(debug & ZINT_DEBUG_TEST_KEEP_OUTFILE)) {
            assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);
        }

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
