/*
    libzint - the open source barcode library
    Copyright (C) 2008-2019 Robin Stuart <rstuart114@gmail.com>

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

static int is_row_column_black(struct zint_symbol *symbol, int row, int column) {
    int i = (row * symbol->bitmap_width + column) * 3;
    return symbol->bitmap[i] == 0 && symbol->bitmap[i + 1] == 0 && symbol->bitmap[i + 2] == 0; // Black
}

static void test_chk_extendable(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int show_hrt;
        unsigned char *data;
        int ret;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_addon_text_row;
        int expected_addon_text_col;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { BARCODE_EANX_CHK, -1, "1234567890128+12", 0, 50, 1, 124, 288, 118, 5, 224 },
        /* 1*/ { BARCODE_UPCA_CHK, -1, "12345678905+12345", 0, 50, 1, 151, 342, 118, 5, 258 },
        /* 2*/ { BARCODE_UPCE_CHK, -1, "12345670+12", 0, 50, 1, 80, 200, 118, 5, 147 },
        /* 3*/ { BARCODE_UPCE_CHK, 0, "12345670+12", 0, 50, 1, 80, 200, 118, 5, 147 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode_and_Buffer(symbol, data[i].data, length, 0);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        int addon_text_bits_set = 0;
        int row = data[i].expected_addon_text_row;
        for (int column = data[i].expected_addon_text_col; column < data[i].expected_addon_text_col + 48; column++) {
            if (is_row_column_black(symbol, row, column)) {
                addon_text_bits_set = 1;
                break;
            }
        }
        if (symbol->show_hrt) {
            assert_nonzero(addon_text_bits_set, "i:%d (%d) addon_text_bits_set zero\n", i, data[i].symbology);
        } else {
            assert_zero(addon_text_bits_set, "i:%d (%d) addon_text_bits_set non-zero\n", i, data[i].symbology);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_row_separator(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int option_3;
        unsigned char *data;
        int ret;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_separator_row;
        int expected_separator_col;
        int expected_separator_height;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, -1, "A", 0, 20, 2, 101, 210, 48, 22, 28, 4 }, // Col 28 TODO: investigate extra 2 compared to vector 26
        /*  1*/ { BARCODE_CODABLOCKF, 0, "A", 0, 20, 2, 101, 210, 48, 22, 28, 4 }, // Same as default
        /*  2*/ { BARCODE_CODABLOCKF, 1, "A", 0, 20, 2, 101, 210, 48, 23, 28, 2 },
        /*  3*/ { BARCODE_CODABLOCKF, 2, "A", 0, 20, 2, 101, 210, 48, 22, 28, 4 }, // Same as default
        /*  4*/ { BARCODE_CODABLOCKF, 3, "A", 0, 20, 2, 101, 210, 48, 21, 28, 6 },
        /*  5*/ { BARCODE_CODABLOCKF, 4, "A", 0, 20, 2, 101, 210, 48, 20, 28, 8 },
        /*  6*/ { BARCODE_CODABLOCKF, 5, "A", 0, 20, 2, 101, 210, 48, 22, 28, 4 }, // > 4 ignored, same as default
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode_and_Buffer(symbol, data[i].data, length, 0);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        int j, separator_bits_set;

        for (j = data[i].expected_separator_row; j < data[i].expected_separator_row + data[i].expected_separator_height; j++) {
            separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
            assert_nonzero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) zero\n", i, data[i].symbology, j, data[i].expected_separator_col);
        }

        j = data[i].expected_separator_row - 1;
        separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
        assert_zero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) before non-zero\n", i, data[i].symbology, j, data[i].expected_separator_col);

        j = data[i].expected_separator_row + data[i].expected_separator_height;
        separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
        assert_zero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) after non-zero\n", i, data[i].symbology, j, data[i].expected_separator_col);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_chk_extendable", test_chk_extendable, 1, 0, 1 },
        { "test_row_separator", test_row_separator, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
