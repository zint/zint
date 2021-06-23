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
#include <sys/stat.h>

#define TEST_PRINT_OVERWRITE_EXPECTED   "bmp,emf,eps,gif,pcx,png,svg,tif,txt"

static void test_print(int index, int generate, int debug) {

    struct item {
        int symbology;
        int option_1;
        int option_2;
        float scale;
        char *data;
        char *expected_file;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, "AIM", "code128_aim" },
        /*  1*/ { BARCODE_QRCODE, 2, 1, -1, "1234567890", "qr_v1_m" },
        /*  2*/ { BARCODE_DOTCODE, -1, -1, 5, "2741", "dotcode_aim_fig7" },
        /*  3*/ { BARCODE_ULTRA, -1, -1, -1, "A", "ultracode_a" },
        /*  4*/ { BARCODE_MAXICODE, -1, -1, -1, "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL...", "maxicode_fig_2" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;
    int j;

    char *exts[] = { "bmp", "emf", "eps", "gif", "pcx", "png", "svg", "tif", "txt" };
    int exts_size = ARRAY_SIZE(exts);

    char data_dir[1024];
    char data_subdir[1024];
    char expected_file[1024];

    char escaped[1024];
    int escaped_size = 1024;

    int have_identify = 0;
    int have_libreoffice = 0;
    int have_ghostscript = 0;
    int have_vnu = 0;
    int have_tiffinfo = 0;
    if (generate) {
        have_identify = testUtilHaveIdentify();
        have_libreoffice = testUtilHaveLibreOffice();
        have_ghostscript = testUtilHaveGhostscript();
        have_vnu = testUtilHaveVnu();
        have_tiffinfo = testUtilHaveTiffInfo();
    }

    testStart("test_print");

    assert_nonzero(testUtilDataPath(data_dir, sizeof(data_dir), "/backend/tests/data", NULL), "testUtilDataPath == 0\n");

    if (generate) {
        if (!testUtilDirExists(data_dir)) {
            ret = testUtilMkDir(data_dir);
            assert_zero(ret, "testUtilMkDir(%s) ret %d != 0 (%d: %s)\n", data_dir, ret, errno, strerror(errno));
        }
        assert_nonzero(sizeof(data_dir) > strlen(data_dir) + 6, "sizeof(data_dir) %d <= strlen (%d) + 6\n", (int) sizeof(data_dir), (int) strlen(data_dir));
        strcat(data_dir, "/print");
        if (!testUtilDirExists(data_dir)) {
            ret = testUtilMkDir(data_dir);
            assert_zero(ret, "testUtilMkDir(%s) ret %d != 0 (%d: %s)\n", data_dir, ret, errno, strerror(errno));
        }
    } else {
        assert_nonzero(sizeof(data_dir) > strlen(data_dir) + 6, "sizeof(data_dir) %d <= strlen (%d) + 6\n", (int) sizeof(data_dir), (int) strlen(data_dir));
        strcat(data_dir, "/print");
    }

    for (j = 0; j < exts_size; j++) {
#ifdef NO_PNG
        if (strcmp(exts[j], "png") == 0) continue;
#endif
        assert_nonzero(sizeof(data_subdir) > strlen(data_dir) + 1 + strlen(exts[j]),
            "sizeof(data_subdir) (%d) <= strlen(data_dir) (%d) + 1 + strlen(%s) (%d)\n",
            (int) sizeof(data_subdir), (int) strlen(data_dir), exts[j], (int) strlen(exts[j]));
        strcpy(data_subdir, data_dir);
        strcat(data_subdir, "/");
        strcat(data_subdir, exts[j]);

        if (generate) {
            if (!testUtilDirExists(data_subdir)) {
                ret = testUtilMkDir(data_subdir);
                assert_zero(ret, "testUtilMkDir(%s) ret %d != 0 (%d: %s)\n", data_subdir, ret, errno, strerror(errno));
            }
        }

        for (i = 0; i < data_size; i++) {

            if (index != -1 && i != index) continue;

            symbol = ZBarcode_Create();
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);
            if (data[i].scale != -1) {
                symbol->scale = data[i].scale;
            }

            ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
            assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

            strcpy(symbol->outfile, "out.");
            strcat(symbol->outfile, exts[j]);

            assert_nonzero(sizeof(expected_file) > strlen(data_subdir) + 1 + strlen(data[i].expected_file) + 1 + strlen(exts[j]),
                "i:%d sizeof(expected_file) (%d) > strlen(data_subdir) (%d) + 1 + strlen(%s) (%d) + 1 + strlen(%s) (%d),\n",
                i, (int) sizeof(expected_file), (int) strlen(data_subdir), data[i].expected_file, (int) strlen(data[i].expected_file), exts[j], (int) strlen(exts[j]));
            strcpy(expected_file, data_subdir);
            strcat(expected_file, "/");
            strcat(expected_file, data[i].expected_file);
            strcat(expected_file, ".");
            strcat(expected_file, exts[j]);

            ret = ZBarcode_Print(symbol, 0);
            assert_zero(ret, "i:%d j:%d %s %s ZBarcode_Print %s ret %d != 0\n", i, j, exts[j], testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

            if (generate) {
                if (j == 0) {
                    printf("        /*%3d*/ { %s, %d, %d, %.8g, \"%s\", \"%s\" },\n",
                            i, testUtilBarcodeName(data[i].symbology), data[i].option_1, data[i].option_2, data[i].scale,
                            testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].expected_file);
                }
                if (strstr(TEST_PRINT_OVERWRITE_EXPECTED, exts[j])) {
                    ret = testUtilRename(symbol->outfile, expected_file);
                    assert_zero(ret, "i:%d testUtilRename(%s, %s) ret %d != 0\n", i, symbol->outfile, expected_file, ret);
                    if (strcmp(exts[j], "eps") == 0) {
                        if (have_ghostscript) {
                            ret = testUtilVerifyGhostscript(expected_file, debug);
                            assert_zero(ret, "i:%d %s ghostscript %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
                        }
                    } else if (strcmp(exts[j], "svg") == 0 || strcmp(exts[j], "emf") == 0) {
                        if (have_libreoffice) {
                            ret = testUtilVerifyLibreOffice(expected_file, debug); // Slow
                            assert_zero(ret, "i:%d %s libreoffice %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
                        }
                        if (have_vnu && strcmp(exts[j], "svg") == 0) {
                            ret = testUtilVerifyVnu(expected_file, debug); // Very slow
                            assert_zero(ret, "i:%d %s vnu %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
                        }
                    } else if (strcmp(exts[j], "tif") == 0 && have_tiffinfo) { // Much faster (and better) than identify
                        ret = testUtilVerifyTiffInfo(expected_file, debug);
                        assert_zero(ret, "i:%d %s tiffinfo %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
                    } else if (strcmp(exts[j], "txt") != 0) { // I.e. rasters
                        if (have_identify) {
                            ret = testUtilVerifyIdentify(expected_file, debug);
                            assert_zero(ret, "i:%d %s identify %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
                        }
                    }
                }
            } else {
                assert_nonzero(testUtilExists(symbol->outfile), "i:%d j:%d %s testUtilExists(%s) == 0\n", i, j, exts[j], symbol->outfile);

                if (strcmp(exts[j], "eps") == 0) {
                    ret = testUtilCmpEpss(symbol->outfile, expected_file);
                    assert_zero(ret, "i:%d %s testUtilCmpEpss(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
                } else if (strcmp(exts[j], "png") == 0) {
                    ret = testUtilCmpPngs(symbol->outfile, expected_file);
                    assert_zero(ret, "i:%d %s testUtilCmpPngs(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
                    ret = testUtilCmpBins(symbol->outfile, expected_file);
                    assert_zero(ret, "i:%d %s testUtilCmpBins(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
                } else if (strcmp(exts[j], "svg") == 0) {
                    ret = testUtilCmpSvgs(symbol->outfile, expected_file);
                    assert_zero(ret, "i:%d %s testUtilCmpSvgs(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
                } else if (strcmp(exts[j], "txt") == 0) {
                    ret = testUtilCmpTxts(symbol->outfile, expected_file);
                    assert_zero(ret, "i:%d %s testUtilCmpTxts(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
                } else {
                    ret = testUtilCmpBins(symbol->outfile, expected_file);
                    assert_zero(ret, "i:%d %s testUtilCmpBins(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
                }

                if (index == -1) assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);
            }

            ZBarcode_Delete(symbol);
        }
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
