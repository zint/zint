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
#include <sys/stat.h>

static void test_print(int index, int generate, int debug) {

    testStart("");

    int have_inkscape = testUtilHaveInkscape();

    int ret;
    struct item {
        int symbology;
        int option_1;
        int option_2;
        unsigned char *data;
        char *expected_file;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, "<>\"&'", "../data/svg/code128_amperands.svg" },
        /*  1*/ { BARCODE_CODABLOCKF, 3, -1, "AAAAAAAAA", "../data/svg/codablockf_3rows.svg"},
        /*  2*/ { BARCODE_EANX, -1, -1, "9771384524017+12", "../data/svg/ean13_2addon_ggs_5.2.2.5.1-2.svg" },
        /*  3*/ { BARCODE_EANX, -1, -1, "9780877799306+54321", "../data/svg/ean13_5addon_ggs_5.2.2.5.2-2.svg" },
        /*  4*/ { BARCODE_UPCA, -1, -1, "012345678905+24", "../data/svg/upca_2addon_ggs_5.2.6.6-5.svg" },
        /*  5*/ { BARCODE_UPCA, -1, -1, "614141234417+12345", "../data/svg/upca_5addon.svg" },
        /*  6*/ { BARCODE_UPCE, -1, -1, "1234567+12", "../data/svg/upce_2addon.svg" },
        /*  7*/ { BARCODE_UPCE, -1, -1, "1234567+12345", "../data/svg/upce_5addon.svg" },
        /*  8*/ { BARCODE_EANX, -1, -1, "1234567+12", "../data/svg/ean8_2addon.svg" },
        /*  9*/ { BARCODE_EANX, -1, -1, "1234567+12345", "../data/svg/ean8_5addon.svg" },
        /* 10*/ { BARCODE_EANX, -1, -1, "12345", "../data/svg/ean5.svg" },
        /* 11*/ { BARCODE_EANX, -1, -1, "12", "../data/svg/ean2.svg" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char *data_dir = "../data/svg";
    char *svg = "out.svg";
    char escaped[1024];
    int escaped_size = 1024;

    if (generate) {
        if (!testUtilExists(data_dir)) {
            ret = mkdir(data_dir, 0755);
            assert_zero(ret, "mkdir(%s) ret %d != 0\n", data_dir, ret);
        }
    }

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, svg);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, \"%s\", \"%s\"},\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].option_1, data[i].option_2,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].expected_file);
            ret = rename(symbol->outfile, data[i].expected_file);
            assert_zero(ret, "i:%d rename(%s, %s) ret %d != 0\n", i, symbol->outfile, data[i].expected_file, ret);
            if (have_inkscape) {
                ret = testUtilVerifyInkscape(data[i].expected_file, debug);
                assert_zero(ret, "i:%d %s inkscape %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), data[i].expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(data[i].expected_file), "i:%d testUtilExists(%s) == 0\n", i, data[i].expected_file);

            ret = testUtilCmpSvgs(symbol->outfile, data[i].expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpSvgs(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, data[i].expected_file, ret);
            assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_print", test_print, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
