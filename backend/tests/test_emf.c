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

    int have_libreoffice = 0;
    if (generate) {
        have_libreoffice = testUtilHaveLibreOffice();
    }

    int ret;
    struct item {
        int symbology;
        int input_mode;
        int output_options;
        int whitespace_width;
        int option_1;
        int option_2;
        char *fgcolour;
        char *bgcolour;
        int rotate_angle;
        char *data;
        char *expected_file;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, BOLD_TEXT, -1, -1, -1, "", "", 0, "Égjpqy", "data/emf/code128_egrave_bold.emf", "" },
        /*  1*/ { BARCODE_TELEPEN, -1, -1, -1, -1, -1, "147AD0", "FC9630", 0, "123", "data/emf/telenum_fg_bg.emf", "" },
        /*  2*/ { BARCODE_ULTRA, -1, -1, 5, -1, -1, "147AD0", "FC9630", 0, "123", "data/emf/ultracode_fg_bg.emf", "" },
        /*  3*/ { BARCODE_EANX, -1, -1, -1, -1, -1, "", "", 0, "9780877799306+54321", "data/emf/ean13_5addon_ggs_5.2.2.5.2-2.emf", "" },
        /*  4*/ { BARCODE_EANX, -1, -1, -1, -1, -1, "", "", 0, "210987654321+54321", "data/emf/ean13_5addon_#185.emf", "#185 Byte count, font data, HeaderExtension1/2" },
        /*  5*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, "", "", 0, "012345678905+24", "data/emf/upca_2addon_ggs_5.2.6.6-5.emf", "" },
        /*  6*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, "", "", 0, "0123456+12", "data/emf/upce_2addon.emf", "" },
        /*  7*/ { BARCODE_UPCE, -1, SMALL_TEXT | BOLD_TEXT, -1, -1, -1, "", "", 0, "0123456+12", "data/emf/upce_2addon_small_bold.emf", "" },
        /*  8*/ { BARCODE_ITF14, -1, BOLD_TEXT, -1, -1, -1, "", "", 0, "123", "data/emf/itf14_bold.emf", "" },
        /*  9*/ { BARCODE_CODE39, -1, -1, -1, -1, -1, "", "", 90, "123", "data/emf/code39_rotate_90.emf", "" },
        /* 10*/ { BARCODE_CODE39, -1, -1, -1, -1, -1, "", "", 180, "123", "data/emf/code39_rotate_180.emf", "" },
        /* 11*/ { BARCODE_CODE39, -1, -1, -1, -1, -1, "", "", 270, "123", "data/emf/code39_rotate_270.emf", "" },
        /* 12*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, "E0E0E0", "700070", 0, "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL...", "data/emf/maxicode_#185.emf", "#185 Maxicode scaling" },
    };
    int data_size = ARRAY_SIZE(data);

    char *data_dir = "data/emf";
    char *emf = "out.emf";
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

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].whitespace_width != -1) {
            symbol->whitespace_width = data[i].whitespace_width;
        }
        if (*data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (*data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, emf);
        ret = ZBarcode_Print(symbol, data[i].rotate_angle);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %s, %d, %d, %d, \"%s\", \"%s\", %d, \"%s\", \"%s\" \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode),
                    testUtilOutputOptionsName(data[i].output_options), data[i].whitespace_width,
                    data[i].option_1, data[i].option_2, data[i].fgcolour, data[i].bgcolour, data[i].rotate_angle,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].expected_file, data[i].comment);
            ret = rename(symbol->outfile, data[i].expected_file);
            assert_zero(ret, "i:%d rename(%s, %s) ret %d != 0\n", i, symbol->outfile, data[i].expected_file, ret);
            if (have_libreoffice) {
                // Note this will fail (on Ubuntu anyway) if LibreOffice Base/Calc/Impress/Writer running (i.e. anything but LibreOffice Draw)
                // Doesn't seem to be a way to force Draw invocation through the command line
                ret = testUtilVerifyLibreOffice(data[i].expected_file, debug);
                assert_zero(ret, "i:%d %s libreoffice %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), data[i].expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(data[i].expected_file), "i:%d testUtilExists(%s) == 0\n", i, data[i].expected_file);

            ret = testUtilCmpBins(symbol->outfile, data[i].expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpBins(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, data[i].expected_file, ret);
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
