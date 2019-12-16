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
/*
 * Intelligent Mail barcode Encoder Test Case Reference Set (csv file)
 * Copyright (C) 2009 U.S. Postal Service
 */

#include "testcommon.h"

#define TEST_IMAIL_CSV_MAX 500

static void test_csv(void)
{
    testStart("");

    FILE* fd = fopen("../data/imail/usps/uspsIMbEncoderTestCases.csv", "r");
    assert_nonnull(fd, "open ../data/imail/usps/uspsIMbEncoderTestCases.csv");

    char buffer[1024];
    char id[10];
    char tracking_code[50];
    char routing_code[50];
    char data[102];
    char expected_daft[70];
    char return_code[10];
    char actual_daft[70];

    int ret;
    int i;
    int lc = 0;
    while (fgets(buffer, sizeof(buffer), fd) != NULL) {

        lc++;
        #ifdef TEST_IMAIL_CSV_MAX
        if (lc > TEST_IMAIL_CSV_MAX) {
            break;
        }
        #endif

        id[0] = tracking_code[0] = routing_code[0] = expected_daft[0] = return_code[0] = '\0';

        char* b = testUtilReadCSVField(buffer, id, sizeof(id));
        assert_nonnull(b, "lc:%d id b == NULL", lc);
        assert_equal(*b, ',', "lc:%d id *b %c != ','", lc, *b);

        b = testUtilReadCSVField(++b, tracking_code, sizeof(tracking_code));
        assert_nonnull(b, "lc:%d tracking_code b == NULL", lc);
        assert_equal(*b, ',', "lc:%d tracking_code *b %c != ','", lc, *b);

        b = testUtilReadCSVField(++b, routing_code, sizeof(routing_code));
        assert_nonnull(b, "lc:%d routing_code b == NULL", lc);
        assert_equal(*b, ',', "lc:%d routing_code *b %c != ','", lc, *b);

        b = testUtilReadCSVField(++b, expected_daft, sizeof(expected_daft));
        assert_nonnull(b, "lc:%d expected_daft b == NULL", lc);
        assert_equal(*b, ',', "lc:%d expected_daft *b %c != ','", lc, *b);

        b = testUtilReadCSVField(++b, return_code, sizeof(return_code));
        assert_nonnull(b, "lc:%d return_code b == NULL", lc);
        assert_equal(*b, ',', "lc:%d return_code *b %c != ','", lc, *b);

        strcpy(data, tracking_code);
        strcat(data, "-");
        strcat(data, routing_code);

        assert_nonzero(strlen(data), "lc:%d strlen(data) == 0", lc);

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_ONECODE;

        ret = ZBarcode_Encode(symbol, data, strlen(data));

        if (strcmp(return_code, "00") == 0) {

            assert_zero(ret, "lc:%d ZBarcode_Encode ret %d != 0\n", lc, ret);

            assert_equal(symbol->rows, 3, "rows %d != 3", symbol->rows);

            ret = testUtilDAFTConvert(symbol, actual_daft, sizeof(actual_daft));
            assert_nonzero(ret, "lc:%d testUtilDAFTConvert == 0", lc);
            assert_zero(strcmp(actual_daft, expected_daft), "lc:%d\n  actual %s\nexpected %s\n", lc, actual_daft, expected_daft);
        } else {
            assert_nonzero(ret, "lc:%d ZBarcode_Encode ret %d == 0\n", lc, ret);
        }

        ZBarcode_Delete(symbol);
    }

    fclose(fd);

    testFinish();
}

int main()
{
    test_csv();

    testReport();

    return 0;
}
