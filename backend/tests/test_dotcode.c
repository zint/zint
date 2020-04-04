/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2020 Robin Stuart <rstuart114@gmail.com>

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

#define TEST_ENCODE_GENERATE_EXPECTED 1

// TODO: Verify against AIM standard
static void test_encode(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        int option_2;
        unsigned char* data;
        int ret;

        int expected_rows;
        int expected_width;
        char* comment;
        char* expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, "2741", 0, 10, 13, "Verified manually againt bwipp (tec-it differs)",
                    "1010001010101"
                    "0001000000000"
                    "1000100010101"
                    "0100000101000"
                    "0000101000100"
                    "0100010000010"
                    "1000101010001"
                    "0101010001000"
                    "1000100010101"
                    "0101000100010"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_DOTCODE;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }
        symbol->debug = ZINT_DEBUG_PRINT;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_ENCODE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, %d, \"%s\", %s, %d, %d, \"%s\",\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].option_2, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
        testUtilModulesDump(symbol, "                    ", "\n");
        printf("               },\n");
        #else
        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

            if (ret == 0) {
                int width, row;
                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);
            }
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// #181 Christian Hartlage / Nico Gunkel OSS-Fuzz
static void test_fuzz(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int length;
        int input_mode;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "(\207'", -1, DATA_MODE, 0 }, // 0x28,0x87,0x27 Note: should but doesn't trigger sanitize error if no length check, for some reason; UPDATE: use up-to-date gcc (9)!
        /*  1*/ {
                    "\133\061\106\133\061\106\070\161\116\133\116\116\067\040\116\016\000\116\125\111\125\125\316\125\125\116\116\116\116\117\116\125"
                    "\111\125\103\316\125\125\116\116\116\116\117\000\000\116\136\116\116\001\116\316\076\116\116\057\136\116\116\134\000\000\116\116"
                    "\116\230\116\116\116\116\125\125\125\257\257\257\000\001\116\130\212\212\212\212\212\212\212\377\377\210\212\212\177\000\212\212"
                    "\212\212\212\212\175\212\212\212\212\212\212\116\117\001\116\116\112\116\116\116\116\176\136\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\005\377\377\005\125\125\125\325\001\116\116\116\266\116\020\000\200\000\116\116\177\000\000\377"
                    "\377\257\257\257\125\112\117\116\001\000\000\044\241\001\116\116\116\136\116\116\116\056\116\125\111\125\125\316\125\125\116\116"
                    "\116\116\057\000\000\116\136\116\116\001\116\116\076\342\116\057\136\116\116\134\000\000\116\116\116\241\116\116\116\116\125\125"
                    "\125\257\257\257\000\001\116\130\212\212\212\212\212\212\212\212\172\212\071\071\071\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\071\071\110\071\071\051\071\065\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\071\071\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330"
                    "\330\330\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\065\071\071\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\071\071\072\071\071\277\071\071\077\071\071\071\071\071\071\071\071\154\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\011\071\071\071\071\071\071\071\071\071\071\071\071\071\071\105\105\105\105\105\105\105\105\105\105\105\105\105\071"
                    "\071\071\071\071\071", // Original OSS-Fuzz triggering data for index out of bounds (encoding of HT/FS/GS/RS when shifting to code set B)
                    421, UNICODE_MODE, ZINT_WARN_USES_ECI },
        /*  2*/ { "\233:", -1, UNICODE_MODE, ZINT_WARN_USES_ECI }, // Original OSS-Fuzz triggering data for codeword_array buffer overflow, L777
        /*  3*/ { "\241\034", -1, UNICODE_MODE, ZINT_WARN_USES_ECI }, // As above L793
        /*  4*/ { "\270\036", -1, UNICODE_MODE, ZINT_WARN_USES_ECI }, // As above L799
        /*  5*/ { "\237\032", -1, UNICODE_MODE, ZINT_WARN_USES_ECI }, // As above L904
        /*  6*/ { "\237", -1, UNICODE_MODE, ZINT_WARN_USES_ECI }, // As above L1090
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_DOTCODE;
        int length = data[i].length;
        if (length == -1) {
            length = strlen(data[i].data);
        }
        if (data[i].input_mode != -1) {
            symbol->input_mode = data[i].input_mode;
        }

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_encode();
    test_fuzz();

    testReport();

    return 0;
}
