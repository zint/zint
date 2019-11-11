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

static void test_chk_extendable(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
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
        /* 0*/ { BARCODE_EANX_CHK, "1234567890128+12", 0, 50, 1, 124, 288, 118, 5, 224 },
        /* 1*/ { BARCODE_UPCA_CHK, "12345678905+12345", 0, 50, 1, 151, 342, 118, 5, 258 },
        /* 2*/ { BARCODE_UPCE_CHK, "12345670+12", 0, 50, 1, 80, 200, 118, 5, 147 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
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
            int j = (row * symbol->bitmap_width + column) * 3;
            if (symbol->bitmap[j] == 0 && symbol->bitmap[j + 1] == 0 && symbol->bitmap[j + 2] == 0) { // Black
                addon_text_bits_set = 1;
                break;
            }
        }
        assert_nonzero(addon_text_bits_set, "i:%d (%d) addon_text_bits_set zero\n", i, data[i].symbology);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_chk_extendable();

    testReport();

    return 0;
}
