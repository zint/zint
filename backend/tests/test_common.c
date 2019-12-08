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

static void test_utf8_to_unicode(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int length;
        int disallow_4byte;
        int ret;
        size_t ret_length;
        int expected_vals[20];
        char* comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "", -1, 1, 0, 0, {}, "" },
        /*  1*/ { "\000a\302\200\340\240\200", 7, 1, 0, 4, { 0, 'a', 0x80, 0x800 }, "NUL a C280 E0A080" },
        /*  2*/ { "\357\277\277", -1, 1, 0, 1, { 0xFFFF }, "EFBFBF" },
        /*  3*/ { "\360\220\200\200", -1, 1, ZINT_ERROR_INVALID_DATA, -1, {}, "Four-byte F0908080" },
        /*  4*/ { "a\200b", -1, 1, ZINT_ERROR_INVALID_DATA, -1, {}, "Orphan continuation 0x80" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    int vals[20];
    struct zint_symbol symbol;

    for (int i = 0; i < data_size; i++) {

        int length = data[i].length == -1 ? strlen(data[i].data) : data[i].length;
        size_t ret_length = length;

        ret = utf8_to_unicode(&symbol, data[i].data, vals, &ret_length, data[i].disallow_4byte);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %ld != %ld\n", i, ret_length, data[i].ret_length);
            for (int j = 0; j < ret_length; j++) {
                assert_equal(vals[j], data[i].expected_vals[j], "i:%d vals[%d] %04X != %04X\n", i, j, vals[j], data[i].expected_vals[j]);
            }
        }
    }

    testFinish();
}

int main()
{
    test_utf8_to_unicode();

    testReport();

    return 0;
}
