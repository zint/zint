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

static void test_pcx(int index, int debug) {

    struct item {
        int symbology;
        int option_1;
        int option_2;
        char *fgcolour;
        char *bgcolour;
        float scale;
        char *data;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, "", "", 0, "AIM" },
        /*  1*/ { BARCODE_QRCODE, 2, 1, "", "", 0, "1234567890" },
        /*  2*/ { BARCODE_DOTCODE, -1, -1, "", "", 0, "2741" },
        /*  3*/ { BARCODE_MAXICODE, -1, -1, "", "", 0, "1" },
        /*  4*/ { BARCODE_GRIDMATRIX, -1, -1, "", "", 0.75, "Grid Matrix" },
        /*  5*/ { BARCODE_CODABLOCKF, -1, 20, "FFFFFF", "000000", 0, "1234567890123456789012345678901234567890" },
        /*  6*/ { BARCODE_CODE128, -1, -1, "C3C3C3", "", 0, "AIM" },
        /*  7*/ { BARCODE_QRCODE, 2, 1, "", "D2E3F4", 0, "1234567890" },
        /*  8*/ { BARCODE_DOTCODE, -1, -1, "C2C100", "E0E1F2", 0, "2741" },
        /*  9*/ { BARCODE_ULTRA, -1, -1, "", "", 0, "ULTRACODE_123456789!" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char *filename = "out.pcx";

    testStart("test_pcx");

    if (!testUtilHaveIdentify()) {
        testSkip("ImageMagick identify not available");
        return;
    }

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
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

        length = (int) strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, filename);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        ret = testUtilVerifyIdentify(symbol->outfile, debug);
        assert_zero(ret, "i:%d %s identify %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_pcx", test_pcx, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
