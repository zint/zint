/*
    libzint - the open source barcode library
    Copyright (C) 2019-2025 Robin Stuart <rstuart114@gmail.com>

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
/* SPDX-License-Identifier: BSD-3-Clause */

#include "testcommon.h"

static void test_upce_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *data;
        int ret;
        const char *expected_errtxt;
        const char *hrt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_UPCE, "12345", 0, "", "00123457" }, /* equivalent: 00123400005, Check digit: 7 */
        /*  1*/ { BARCODE_UPCE, "0001234000057", 0, "", "00123457" }, /* GTIN-13 format */
        /*  2*/ { BARCODE_UPCE_CHK, "12345", ZINT_ERROR_INVALID_CHECK, "Error 274: Invalid check digit '5', expecting '4'", "" },
        /*  3*/ { BARCODE_UPCE_CHK, "12344", 0, "", "00012344" }, /* equivalent: 00012000003, Check digit: 4 */
        /*  4*/ { BARCODE_UPCE, "123456", 0, "", "01234565" }, /* equivalent: 01234500006, Check digit: 5 */
        /*  5*/ { BARCODE_UPCE_CHK, "123456", ZINT_ERROR_INVALID_CHECK, "Error 274: Invalid check digit '6', expecting '7'", "" },
        /*  6*/ { BARCODE_UPCE_CHK, "123457", 0, "", "00123457" }, /* equivalent: 00123400005, Check digit: 7 */
        /*  7*/ { BARCODE_UPCE, "1234567", 0, "", "12345670" }, /* equivalent: 12345600007, Check digit: 0 */
        /*  8*/ { BARCODE_UPCE_CHK, "1234567", ZINT_ERROR_INVALID_CHECK, "Error 274: Invalid check digit '7', expecting '5'", "" },
        /*  9*/ { BARCODE_UPCE_CHK, "1234565", 0, "", "01234565" }, /* equivalent: 01234500006, Check digit: 5 */
        /* 10*/ { BARCODE_UPCE, "12345678", ZINT_ERROR_INVALID_CHECK, "Error 274: Invalid check digit '8', expecting '0'", "" },
        /* 11*/ { BARCODE_UPCE, "12345670", 0, "", "12345670" }, /* equivalent: 12345600007, Check digit: 0 */
        /* 12*/ { BARCODE_UPCE_CHK, "12345678", ZINT_ERROR_INVALID_CHECK, "Error 274: Invalid check digit '8', expecting '0'", "" },
        /* 13*/ { BARCODE_UPCE_CHK, "12345670", 0, "", "12345670" }, /* equivalent: 12345600007, Check digit: 0 */
        /* 14*/ { BARCODE_UPCE, "123456789", ZINT_ERROR_TOO_LONG, "Error 290: Input length 9 too long (maximum 8)", "" },
        /* 15*/ { BARCODE_UPCE_CHK, "123456789", ZINT_ERROR_TOO_LONG, "Error 290: Input length 9 too long (maximum 8)", "" },
        /* 16*/ { BARCODE_UPCE, "123456A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 7 in input (digits and \"+\" or space only)", "" },
        /* 17*/ { BARCODE_UPCE, "1234567A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 8 in input (digits and \"+\" or space only)", "" },
        /* 18*/ { BARCODE_UPCE, "12345678A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 9 in input (digits and \"+\" or space only)", "" },
        /* 19*/ { BARCODE_UPCE_CHK, "123456A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 7 in input (digits and \"+\" or space only)", "" },
        /* 20*/ { BARCODE_UPCE_CHK, "1234567A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 8 in input (digits and \"+\" or space only)", "" },
        /* 21*/ { BARCODE_UPCE_CHK, "12345678A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 9 in input (digits and \"+\" or space only)", "" },
        /* 22*/ { BARCODE_UPCE, "1234567+1", 0, "", "12345670+01" },
        /* 23*/ { BARCODE_UPCE, "1234567 1", 0, "", "12345670+01" },
        /* 24*/ { BARCODE_UPCE_CHK, "1234567+1", ZINT_ERROR_INVALID_CHECK, "Error 274: Invalid check digit '7', expecting '5'", "" },
        /* 25*/ { BARCODE_UPCE, "1234567++1", ZINT_ERROR_INVALID_DATA, "Error 293: Invalid add-on data (one \"+\" or space only)", "" },
        /* 26*/ { BARCODE_UPCE, "1234567 +1", ZINT_ERROR_INVALID_DATA, "Error 293: Invalid add-on data (one \"+\" or space only)", "" },
        /* 27*/ { BARCODE_UPCE, "12345670+1", 0, "", "12345670+01" },
        /* 28*/ { BARCODE_UPCE, "12345670 1", 0, "", "12345670+01" },
        /* 29*/ { BARCODE_UPCE_CHK, "12345670+1", 0, "", "12345670+01" },
        /* 30*/ { BARCODE_UPCE_CHK, "12345670 1", 0, "", "12345670+01" },
        /* 31*/ { BARCODE_UPCE, "12345670+12", 0, "", "12345670+12" },
        /* 32*/ { BARCODE_UPCE, "12345670 12", 0, "", "12345670+12" },
        /* 33*/ { BARCODE_UPCE, "012345600007012", 0, "", "12345670+12" }, /* GTIN-13 format with 2-digit add-on */
        /* 34*/ { BARCODE_UPCE, "12345670+123", 0, "", "12345670+00123" },
        /* 35*/ { BARCODE_UPCE, "12345670 123", 0, "", "12345670+00123" },
        /* 36*/ { BARCODE_UPCE, "12345670+12345", 0, "", "12345670+12345" },
        /* 37*/ { BARCODE_UPCE, "12345670 12345", 0, "", "12345670+12345" },
        /* 38*/ { BARCODE_UPCE, "012345600007012345", 0, "", "12345670+12345" }, /* GTIN-13 format with 5-digit add-on */
        /* 39*/ { BARCODE_UPCE_CHK, "12345670+12345", 0, "", "12345670+12345" },
        /* 40*/ { BARCODE_UPCE_CHK, "12345670 12345", 0, "", "12345670+12345" },
        /* 41*/ { BARCODE_UPCE, "1234567+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 42*/ { BARCODE_UPCE_CHK, "12345670+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 43*/ { BARCODE_UPCE, "2345678", ZINT_WARN_INVALID_OPTION, "Warning 851: Ignoring first digit which is not '0' or '1'", "03456781" }, /* 2 ignored, equivalent: 03456700008, Check digit: 1 */
        /* 44*/ { BARCODE_UPCE_CHK, "23456781", ZINT_WARN_INVALID_OPTION, "Warning 851: Ignoring first digit which is not '0' or '1'", "03456781" }, /* 2 ignored, equivalent: 03456700008, Check digit: 1 */
        /* 45*/ { BARCODE_UPCE, "123455", 0, "", "01234558" }, /* equivalent: 01234500005, Check digit: 8 (BS 797 Rule 3 (a)) */
        /* 46*/ { BARCODE_UPCE, "0012345000058", 0, "", "01234558" }, /* GTIN-13 format */
        /* 47*/ { BARCODE_UPCE_CHK, "1234558", 0, "", "01234558" }, /* equivalent: 01234500005, Check digit: 8 (BS 797 Rule 3 (a)) */
        /* 48*/ { BARCODE_UPCE, "456784", 0, "", "04567840" }, /* equivalent: 04567000008, Check digit: 0 (BS 797 Rule 3 (b)) */
        /* 49*/ { BARCODE_UPCE, "0045670000080", 0, "", "04567840" }, /* GTIN-13 format */
        /* 50*/ { BARCODE_UPCE_CHK, "4567840", 0, "", "04567840" }, /* equivalent: 04567000008, Check digit: 0 (BS 797 Rule 3 (b)) */
        /* 51*/ { BARCODE_UPCE, "345670", 0, "", "03456703" }, /* equivalent: 03400000567, Check digit: 3 (BS 797 Rule 3 (c)) */
        /* 52*/ { BARCODE_UPCE, "0034000005673", 0, "", "03456703" }, /* GTIN-13 format */
        /* 53*/ { BARCODE_UPCE_CHK, "3456703", 0, "", "03456703" }, /* equivalent: 03400000567, Check digit: 3 (BS 797 Rule 3 (c)) */
        /* 54*/ { BARCODE_UPCE, "984753", 0, "", "09847531" }, /* equivalent: 09840000075, Check digit: 1 (BS 797 Rule 3 (d)) */
        /* 55*/ { BARCODE_UPCE, "0098400000751", 0, "", "09847531" }, /* GTIN-13 format */
        /* 56*/ { BARCODE_UPCE_CHK, "9847531", 0, "", "09847531" }, /* equivalent: 09840000075, Check digit: 1 (BS 797 Rule 3 (d)) */
        /* 57*/ { BARCODE_UPCE, "120453", ZINT_ERROR_INVALID_DATA, "Error 271: For this UPC-E zero suppression, 3rd character cannot be \"0\", \"1\" or \"2\" (120453)", "" }, /* If last digit (emode) 3, 3rd can't be 0, 1 or 2 (BS 787 Table 5 NOTE 1) */
        /* 58*/ { BARCODE_UPCE, "121453", ZINT_ERROR_INVALID_DATA, "Error 271: For this UPC-E zero suppression, 3rd character cannot be \"0\", \"1\" or \"2\" (121453)", "" }, /* If last digit (emode) 3, 3rd can't be 0, 1 or 2 (BS 787 Table 5 NOTE 1) */
        /* 59*/ { BARCODE_UPCE, "122453", ZINT_ERROR_INVALID_DATA, "Error 271: For this UPC-E zero suppression, 3rd character cannot be \"0\", \"1\" or \"2\" (122453)", "" }, /* If last digit (emode) 3, 3rd can't be 0, 1 or 2 (BS 787 Table 5 NOTE 1) */
        /* 60*/ { BARCODE_UPCE, "123453", 0, "", "01234531" },
        /* 61*/ { BARCODE_UPCE, "123054", ZINT_ERROR_INVALID_DATA, "Error 272: For this UPC-E zero suppression, 4th character cannot be \"0\" (123054)", "" }, /* If last digit (emode) 4, 4th can't be 0 (BS 787 Table 5 NOTE 2) */
        /* 62*/ { BARCODE_UPCE, "123154", 0, "", "01231542" },
        /* 63*/ { BARCODE_UPCE, "123405", ZINT_ERROR_INVALID_DATA, "Error 273: For this UPC-E zero suppression, 5th character cannot be \"0\" (123405)", "" }, /* If last digit (emode) 5, 5th can't be 0 (BS 787 Table 5 NOTE 3) */
        /* 64*/ { BARCODE_UPCE, "123455", 0, "", "01234558" },
        /* 65*/ { BARCODE_UPCE, "123406", ZINT_ERROR_INVALID_DATA, "Error 273: For this UPC-E zero suppression, 5th character cannot be \"0\" (123406)", "" }, /* If last digit (emode) 6, 5th can't be 0 (BS 787 Table 5 NOTE 3) */
        /* 66*/ { BARCODE_UPCE, "123456", 0, "", "01234565" },
        /* 67*/ { BARCODE_UPCE, "123407", ZINT_ERROR_INVALID_DATA, "Error 273: For this UPC-E zero suppression, 5th character cannot be \"0\" (123407)", "" }, /* If last digit (emode) 7, 5th can't be 0 (BS 787 Table 5 NOTE 3) */
        /* 68*/ { BARCODE_UPCE, "123457", 0, "", "01234572" },
        /* 69*/ { BARCODE_UPCE, "123408", ZINT_ERROR_INVALID_DATA, "Error 273: For this UPC-E zero suppression, 5th character cannot be \"0\" (123408)", "" }, /* If last digit (emode) 8, 5th can't be 0 (BS 787 Table 5 NOTE 3) */
        /* 70*/ { BARCODE_UPCE, "123458", 0, "", "01234589" },
        /* 71*/ { BARCODE_UPCE, "123409", ZINT_ERROR_INVALID_DATA, "Error 273: For this UPC-E zero suppression, 5th character cannot be \"0\" (123409)", "" }, /* If last digit (emode) 9, 5th can't be 0 (BS 787 Table 5 NOTE 3) */
        /* 72*/ { BARCODE_UPCE, "123459", 0, "", "01234596" },
        /* 73*/ { BARCODE_UPCE, "000000", 0, "", "00000000" },
        /* 74*/ { BARCODE_UPCE, "000001", 0, "", "00000019" },
        /* 75*/ { BARCODE_UPCE, "000002", 0, "", "00000028" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[4096] = {0}; /* Suppress clang -fsanitize=memory false positive */
    char cmp_msg[1024] = {0}; /* Suppress clang -fsanitize=memory false positive */

    /* Only do BWIPP/ZXing-C++ tests if asked, too slow otherwise */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript();
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder();

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n",
                    i, symbol->errtxt, data[i].expected_errtxt);

        if (ret < ZINT_ERROR) {
            assert_zero(strcmp(ZCCP(symbol->text), data[i].hrt), "i:%d strcmp(%s, %s) != 0\n",
                        i, symbol->text, data[i].hrt);
            if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, -1, debug)) {
                char modules_dump[8192 + 1];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                            "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilBwipp(i, symbol, -1, -1, -1, data[i].hrt, (int) strlen(data[i].hrt), NULL, cmp_buf,
                            sizeof(cmp_buf), NULL);
                assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n",
                            i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                               i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
            }
            if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                int cmp_len, ret_len;
                char modules_dump[8192 + 1];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                            "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, 1 /*zxingcpp_cmp*/, cmp_buf,
                            sizeof(cmp_buf), &cmp_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n",
                            i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].hrt, (int) strlen(data[i].hrt),
                            NULL /*primary*/, escaped, &ret_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                            i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len,
                            escaped);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* Note requires ZINT_SANITIZE to be set */
static void test_upca_print(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *data;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_UPCA, "01234567890", 0 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        strcpy(symbol->outfile, "out.gif");
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0 (%s)\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret, symbol->errtxt);

        assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_upca_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *data;
        int ret;
        const char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_UPCA, "12345678901", 0, "" },
        /*  1*/ { BARCODE_UPCA, "1234567890", 0, "" },
        /*  2*/ { BARCODE_UPCA, "123456789012", 0, "" }, /* UPC-A accepts CHK */
        /*  3*/ { BARCODE_UPCA, "0123456789012", 0, "" }, /* GTIN-13 format */
        /*  4*/ { BARCODE_UPCA, "123456789011", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '1', expecting '2'" },
        /*  5*/ { BARCODE_UPCA, "1234567890123", ZINT_ERROR_TOO_LONG, "Error 288: Input length 13 too long (maximum 12)" },
        /*  6*/ { BARCODE_UPCA, "123456789012A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 13 in input (digits and \"+\" or space only)" },
        /*  7*/ { BARCODE_UPCA, "12345678901A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 12 in input (digits and \"+\" or space only)" },
        /*  8*/ { BARCODE_UPCA, "12345", 0, "" },
        /*  9*/ { BARCODE_UPCA, "1X345", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 2 in input (digits and \"+\" or space only)" },
        /* 10*/ { BARCODE_UPCA, "01A345", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 3 in input (digits and \"+\" or space only)" },
        /* 11*/ { BARCODE_UPCA, "12345678901+1", 0, "" },
        /* 12*/ { BARCODE_UPCA, "12345678901 1", 0, "" },
        /* 13*/ { BARCODE_UPCA, "12345678901++1", ZINT_ERROR_INVALID_DATA, "Error 293: Invalid add-on data (one \"+\" or space only)" },
        /* 14*/ { BARCODE_UPCA, "1234567890+1+1", ZINT_ERROR_INVALID_DATA, "Error 293: Invalid add-on data (one \"+\" or space only)" },
        /* 15*/ { BARCODE_UPCA, "12345678901  1", ZINT_ERROR_INVALID_DATA, "Error 293: Invalid add-on data (one \"+\" or space only)" },
        /* 16*/ { BARCODE_UPCA, "1234567890 1 1", ZINT_ERROR_INVALID_DATA, "Error 293: Invalid add-on data (one \"+\" or space only)" },
        /* 17*/ { BARCODE_UPCA, "12345678901+ 1", ZINT_ERROR_INVALID_DATA, "Error 293: Invalid add-on data (one \"+\" or space only)" },
        /* 18*/ { BARCODE_UPCA, "1234567890+1 1", ZINT_ERROR_INVALID_DATA, "Error 293: Invalid add-on data (one \"+\" or space only)" },
        /* 19*/ { BARCODE_UPCA, "123456789012+1", 0, "" },
        /* 20*/ { BARCODE_UPCA, "123456789013+1", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '3', expecting '2'" },
        /* 21*/ { BARCODE_UPCA, "12345678901+12", 0, "" },
        /* 22*/ { BARCODE_UPCA, "12345678901 12", 0, "" },
        /* 23*/ { BARCODE_UPCA, "123456789012+12", 0, "" },
        /* 24*/ { BARCODE_UPCA, "123456789014+12", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '4', expecting '2'" },
        /* 25*/ { BARCODE_UPCA, "012345678901212", 0, "" }, /* GTIN-13 format with 2-digit add-on */
        /* 26*/ { BARCODE_UPCA, "12345678901+123", 0, "" },
        /* 27*/ { BARCODE_UPCA, "12345678901 123", 0, "" },
        /* 28*/ { BARCODE_UPCA, "123456789012+123", 0, "" },
        /* 29*/ { BARCODE_UPCA, "123456789015+123", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '5', expecting '2'" },
        /* 30*/ { BARCODE_UPCA, "123456789012+1234", 0, "" },
        /* 31*/ { BARCODE_UPCA, "123456789012 1234", 0, "" },
        /* 32*/ { BARCODE_UPCA, "123456789016+1234", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '6', expecting '2'" },
        /* 33*/ { BARCODE_UPCA, "123456789012+12345", 0, "" },
        /* 34*/ { BARCODE_UPCA, "123456789012 12345", 0, "" },
        /* 35*/ { BARCODE_UPCA, "012345678901212345", 0, "" }, /* GTIN-13 format with 5-digit add-on */
        /* 36*/ { BARCODE_UPCA, "123456789017+12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '7', expecting '2'" },
        /* 37*/ { BARCODE_UPCA, "123456789012+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)" },
        /* 38*/ { BARCODE_UPCA, "123456789017+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)" },
        /* 39*/ { BARCODE_UPCA, "123456789017+12345A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 19 in input (digits and \"+\" or space only)" },
        /* 40*/ { BARCODE_UPCA_CHK, "123456789012", 0, "" },
        /* 41*/ { BARCODE_UPCA_CHK, "123456789011", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '1', expecting '2'" },
        /* 42*/ { BARCODE_UPCA_CHK, "1234567890123", ZINT_ERROR_TOO_LONG, "Error 288: Input length 13 too long (maximum 12)" },
        /* 43*/ { BARCODE_UPCA_CHK, "123456789012A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 13 in input (digits and \"+\" or space only)" },
        /* 44*/ { BARCODE_UPCA_CHK, "12345678901A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 12 in input (digits and \"+\" or space only)" },
        /* 45*/ { BARCODE_UPCA_CHK, "12345678901", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '1', expecting '5'" },
        /* 46*/ { BARCODE_UPCA_CHK, "12345678905", 0, "" },
        /* 47*/ { BARCODE_UPCA_CHK, "1234567890", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '0', expecting '5'" },
        /* 48*/ { BARCODE_UPCA_CHK, "1234567895", 0, "" },
        /* 49*/ { BARCODE_UPCA_CHK, "123456789", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '9', expecting '4'" },
        /* 50*/ { BARCODE_UPCA_CHK, "123456784", 0, "" },
        /* 51*/ { BARCODE_UPCA_CHK, "12345678", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '8', expecting '0'" },
        /* 52*/ { BARCODE_UPCA_CHK, "12345670", 0, "" },
        /* 53*/ { BARCODE_UPCA_CHK, "1234567", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '7', expecting '5'" },
        /* 54*/ { BARCODE_UPCA_CHK, "1234565", 0, "" },
        /* 55*/ { BARCODE_UPCA_CHK, "123456", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '6', expecting '7'" },
        /* 56*/ { BARCODE_UPCA_CHK, "123457", 0, "" },
        /* 57*/ { BARCODE_UPCA_CHK, "12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '5', expecting '8'" },
        /* 58*/ { BARCODE_UPCA_CHK, "12348", 0, "" },
        /* 59*/ { BARCODE_UPCA_CHK, "1234", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '4', expecting '6'" },
        /* 60*/ { BARCODE_UPCA_CHK, "1236", 0, "" },
        /* 61*/ { BARCODE_UPCA_CHK, "123", 0, "" }, /* Happens to be correct check digit */
        /* 62*/ { BARCODE_UPCA_CHK, "124", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '4', expecting '3'" },
        /* 63*/ { BARCODE_UPCA_CHK, "12", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '2', expecting '7'" },
        /* 64*/ { BARCODE_UPCA_CHK, "17", 0, "" },
        /* 65*/ { BARCODE_UPCA_CHK, "1", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '1', expecting '0'" },
        /* 66*/ { BARCODE_UPCA_CHK, "0", 0, "" },
        /* 67*/ { BARCODE_UPCA_CHK, "12345678905+12", 0, "" },
        /* 68*/ { BARCODE_UPCA_CHK, "12345678905+12345", 0, "" },
        /* 69*/ { BARCODE_UPCA_CHK, "12345678905+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)" },
        /* 70*/ { BARCODE_UPCA_CHK, "12345678905+12345A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 18 in input (digits and \"+\" or space only)" },
        /* 71*/ { BARCODE_UPCA_CHK, "12345678905+1234A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 17 in input (digits and \"+\" or space only)" },
        /* 72*/ { BARCODE_UPCA_CHK, "1234567895+12345", 0, "" },
        /* 73*/ { BARCODE_UPCA_CHK, "1234567891+12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '1', expecting '5'" },
        /* 74*/ { BARCODE_UPCA_CHK, "123456784+12345", 0, "" },
        /* 75*/ { BARCODE_UPCA_CHK, "123456782+12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '2', expecting '4'" },
        /* 76*/ { BARCODE_UPCA_CHK, "12345670+12345", 0, "" },
        /* 77*/ { BARCODE_UPCA_CHK, "12345673+12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '3', expecting '0'" },
        /* 78*/ { BARCODE_UPCA_CHK, "1234565+12345", 0, "" },
        /* 79*/ { BARCODE_UPCA_CHK, "1234564+12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '4', expecting '5'" },
        /* 80*/ { BARCODE_UPCA_CHK, "123457+12345", 0, "" },
        /* 81*/ { BARCODE_UPCA_CHK, "123455+12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '5', expecting '7'" },
        /* 82*/ { BARCODE_UPCA_CHK, "12348+12345", 0, "" },
        /* 83*/ { BARCODE_UPCA_CHK, "12346+12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '6', expecting '8'" },
        /* 84*/ { BARCODE_UPCA_CHK, "1236+12345", 0, "" },
        /* 85*/ { BARCODE_UPCA_CHK, "1237+12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '7', expecting '6'" },
        /* 86*/ { BARCODE_UPCA_CHK, "123+12345", 0, "" },
        /* 87*/ { BARCODE_UPCA_CHK, "128+12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '8', expecting '3'" },
        /* 88*/ { BARCODE_UPCA_CHK, "17+12345", 0, "" },
        /* 89*/ { BARCODE_UPCA_CHK, "19+12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '9', expecting '7'" },
        /* 90*/ { BARCODE_UPCA_CHK, "1+12345", ZINT_ERROR_INVALID_CHECK, "Error 270: Invalid check digit '1', expecting '0'" },
        /* 91*/ { BARCODE_UPCA_CHK, "0+12345", 0, "" },
        /* 92*/ { BARCODE_UPCA_CHK, "0 12345", 0, "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n",
                    i, symbol->errtxt, data[i].expected_errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_ean13_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
        int ret;
        int expected_width;
        const char *ret_errtxt;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "123456789012", 0, 95, "", "" },
        /*  1*/ { "12345678901A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 12 in input (digits and \"+\" or space only)", "" },
        /*  2*/ { "12345A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 6 in input (digits and \"+\" or space only)", "" },
        /*  3*/ { "123A56", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 4 in input (digits and \"+\" or space only)", "" },
        /*  4*/ { "12345678901", 0, 95, "", "" },
        /*  5*/ { "1234567890128", 0, 95, "", "EAN13 accepts CHK (treated as such if no leading zeroes required)" },
        /*  6*/ { "1234567890120", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '0', expecting '8'", "" },
        /*  7*/ { "123456789012+1", 0, 122, "", "" },
        /*  8*/ { "123456789012 1", 0, 122, "", "" },
        /*  9*/ { "1234567890128+1", 0, 122, "", "" },
        /* 10*/ { "1234567890128 1", 0, 122, "", "" },
        /* 11*/ { "1234567890121+1", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '1', expecting '8'", "" },
        /* 12*/ { "1234567890121 1", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '1', expecting '8'", "" },
        /* 13*/ { "123456789012+12", 0, 122, "", "" },
        /* 14*/ { "123456789012 12", 0, 122, "", "" },
        /* 15*/ { "1234567890128+12", 0, 122, "", "" },
        /* 16*/ { "123456789012812", 0, 122, "", "GTIN-13 format, with 2-digit add-on" },
        /* 17*/ { "1234567890122+12", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '2', expecting '8'", "" },
        /* 18*/ { "12345678901234+12", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 14 too long (maximum 13)", "" },
        /* 19*/ { "123456789012345+12", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 15 too long (maximum 13)", "" },
        /* 20*/ { "1234567890123456+12", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 16 too long (maximum 13)", "" },
        /* 21*/ { "123456789012+123", 0, 149, "", "" },
        /* 22*/ { "123456789012 123", 0, 149, "", "" },
        /* 23*/ { "1234567890128+123", 0, 149, "", "" },
        /* 24*/ { "1234567890123+123", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '3', expecting '8'", "" },
        /* 25*/ { "12345678901234+123", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 14 too long (maximum 13)", "" },
        /* 26*/ { "123456789012345+123", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 15 too long (maximum 13)", "" },
        /* 27*/ { "123456789012+1234", 0, 149, "", "" },
        /* 28*/ { "123456789012+1234", 0, 149, "", "" },
        /* 29*/ { "1234567890128+1234", 0, 149, "", "" },
        /* 30*/ { "1234567890124+1234", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '4', expecting '8'", "" },
        /* 31*/ { "12345678901234+1234", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 14 too long (maximum 13)", "" },
        /* 32*/ { "123456789012+12345", 0, 149, "", "" },
        /* 33*/ { "123456789012 12345", 0, 149, "", "" },
        /* 34*/ { "1234567890128+12345", 0, 149, "", "" },
        /* 35*/ { "1234567890128 12345", 0, 149, "", "" },
        /* 36*/ { "123456789012812345", 0, 149, "", "GTIN-13 format, with 5-digit add-on" },
        /* 37*/ { "12345678901234+12345", ZINT_ERROR_TOO_LONG, -1, "Error 283: Input length 20 too long (maximum 19)", "" },
        /* 38*/ { "1234567890125+12345", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '5', expecting '8'", "" },
        /* 39*/ { "123456789012+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 40*/ { "1234567890128+123456", ZINT_ERROR_TOO_LONG, -1, "Error 283: Input length 20 too long (maximum 19)", "" },
        /* 41*/ { "1234567890128 123456", ZINT_ERROR_TOO_LONG, -1, "Error 283: Input length 20 too long (maximum 19)", "" },
        /* 42*/ { "12345678901+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 43*/ { "12345678901 123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 44*/ { "12345678901+1234567", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 7 too long (maximum 5)", "" },
        /* 45*/ { "1234567890+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 46*/ { "1234567890+1234567", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 7 too long (maximum 5)", "" },
        /* 47*/ { "123456789+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 48*/ { "123456789+1234567", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 7 too long (maximum 5)", "" },
        /* 49*/ { "12345678+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 50*/ { "1234567+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 51*/ { "123456+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 52*/ { "12345+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 53*/ { "1234+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 54*/ { "123+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 55*/ { "12+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 56*/ { "1+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 57*/ { "1+12345678901234", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 14 too long (maximum 5)", "" },
        /* 58*/ { "1+12345", 0, 149, "", "" },
        /* 59*/ { "1++12345", ZINT_ERROR_INVALID_DATA, -1, "Error 293: Invalid add-on data (one \"+\" or space only)", "" },
        /* 60*/ { "1  12345", ZINT_ERROR_INVALID_DATA, -1, "Error 293: Invalid add-on data (one \"+\" or space only)", "" },
        /* 61*/ { "1+ 12345", ZINT_ERROR_INVALID_DATA, -1, "Error 293: Invalid add-on data (one \"+\" or space only)", "" },
        /* 62*/ { "1+", 0, 95, "", "" },
        /* 63*/ { "1 ", 0, 95, "", "" },
        /* 64*/ { "+1", 0, 122, "", "" },
        /* 65*/ { " 1", 0, 122, "", "" },
        /* 66*/ { "+", 0, 95, "", "" },
        /* 67*/ { " ", 0, 95, "", "" },
        /* 68*/ { "1", 0, 95, "", "" },
        /* 69*/ { "12", 0, 95, "", "" },
        /* 70*/ { "123", 0, 95, "", "" },
        /* 71*/ { "12345678901234", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 14 too long (maximum 13)", "" },
        /* 72*/ { "1234567890123A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 14 in input (digits and \"+\" or space only)", "" },
        /* 73*/ { "123456789012345", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 15 too long (maximum 13)", "" },
        /* 74*/ { "12345678901234A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 15 in input (digits and \"+\" or space only)", "" },
        /* 75*/ { "1234567890123456", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 16 too long (maximum 13)", "" },
        /* 76*/ { "12345678901234567", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 17 too long (maximum 13)", "" },
        /* 77*/ { "123456789012345678", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 18 too long (maximum 13)", "" },
        /* 78*/ { "1234567890123456789", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 19 too long (maximum 13)", "" },
        /* 79*/ { "1234567890128", 0, 95, "", "" },
        /* 80*/ { "1234567890126", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '6', expecting '8'", "" },
        /* 81*/ { "123456789012A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 13 in input (digits and \"+\" or space only)", "" },
        /* 82*/ { "123456789012", 0, 95, "", "" },
        /* 83*/ { "123456789013", 0, 95, "", "" },
        /* 84*/ { "12345678901", 0, 95, "", "" },
        /* 85*/ { "12345678905", 0, 95, "", "" },
        /* 86*/ { "1234567890", 0, 95, "", "" },
        /* 87*/ { "123456789", 0, 95, "", "" },
        /* 88*/ { "12345678", 0, 95, "", "" },
        /* 89*/ { "1234567", 0, 95, "", "" },
        /* 90*/ { "123456", 0, 95, "", "" },
        /* 91*/ { "12345", 0, 95, "", "" },
        /* 92*/ { "1234", 0, 95, "", "" },
        /* 93*/ { "123", 0, 95, "", "" },
        /* 94*/ { "12", 0, 95, "", "" },
        /* 95*/ { "1", 0, 95, "", "" },
        /* 96*/ { "123456789012+1", 0, 122, "", "" },
        /* 97*/ { "1234567890128+1", 0, 122, "", "" },
        /* 98*/ { "1234567890127+1", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '7', expecting '8'", "" },
        /* 99*/ { "123456789012+12", 0, 122, "", "" },
        /*100*/ { "1234567890128+12", 0, 122, "", "" },
        /*101*/ { "1234567890129+12", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '9', expecting '8'", "" },
        /*102*/ { "123456789012+123", 0, 149, "", "" },
        /*103*/ { "1234567890128+123", 0, 149, "", "" },
        /*104*/ { "1234567890120+1234", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '0', expecting '8'", "" },
        /*105*/ { "123456789012+1234", 0, 149, "", "" },
        /*106*/ { "1234567890128+1234", 0, 149, "", "" },
        /*107*/ { "1234567890121+1234", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '1', expecting '8'", "" },
        /*108*/ { "123456789012+12345", 0, 149, "", "" },
        /*109*/ { "1234567890128+12345", 0, 149, "", "" },
        /*110*/ { "1234567890122+12345", ZINT_ERROR_INVALID_CHECK, -1, "Error 275: Invalid check digit '2', expecting '8'", "" },
        /*111*/ { "1234567890122+1234A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 19 in input (digits and \"+\" or space only)", "" },
        /*112*/ { "123456789012+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /*113*/ { "123456789012+12345A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 19 in input (digits and \"+\" or space only)", "" },
        /*114*/ { "1234567890128+123456", ZINT_ERROR_TOO_LONG, -1, "Error 283: Input length 20 too long (maximum 19)", "" },
        /*115*/ { "12345678901+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /*116*/ { "12345678901+1234567", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 7 too long (maximum 5)", "" },
        /*117*/ { "12345678901+12345", 0, 149, "", "" },
        /*118*/ { "1234567890+12345", 0, 149, "", "" },
        /*119*/ { "1234567890+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /*120*/ { "123456789+12345", 0, 149, "", "" },
        /*121*/ { "12345678+12345", 0, 149, "", "" },
        /*122*/ { "12345670+12345", 0, 149, "", "" },
        /*123*/ { "1234567+12345", 0, 149, "", "" },
        /*124*/ { "1234565+12345", 0, 149, "", "" },
        /*125*/ { "123456+12345", 0, 149, "", "" },
        /*126*/ { "123457+12345", 0, 149, "", "" },
        /*127*/ { "12345+12345", 0, 149, "", "" },
        /*128*/ { "12348+12345", 0, 149, "", "" },
        /*129*/ { "1234+12345", 0, 149, "", "" },
        /*130*/ { "1236+12345", 0, 149, "", "" },
        /*131*/ { "123+12345", 0, 149, "", "3 happens to be correct check digit" },
        /*132*/ { "124+12345", 0, 149, "", "" },
        /*133*/ { "12+12345", 0, 149, "", "" },
        /*134*/ { "17+12345", 0, 149, "", "" },
        /*135*/ { "1+12345", 0, 149, "", "" },
        /*136*/ { "0+12345", 0, 149, "", "" },
        /*137*/ { "0+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /*138*/ { "1+12345678901234", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 14 too long (maximum 5)", "" },
        /*139*/ { "0+12345678901234", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 14 too long (maximum 5)", "" },
        /*140*/ { "1+", 0, 95, "", "" },
        /*141*/ { "+1", 0, 122, "", "" },
        /*142*/ { "+", 0, 95, "", "" },
        /*143*/ { "12345678901234", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 14 too long (maximum 13)", "" },
        /*144*/ { "1234567890123A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 14 in input (digits and \"+\" or space only)", "" },
        /*145*/ { "123456789012345", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 15 too long (maximum 13)", "" },
        /*146*/ { "1234567890123456", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 16 too long (maximum 13)", "" },
        /*147*/ { "12345678901234567", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 17 too long (maximum 13)", "" },
        /*148*/ { "123456789012345678", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 18 too long (maximum 13)", "" },
        /*149*/ { "1234567890123456789", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 19 too long (maximum 13)", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char errtxt_escaped[256];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_EAN13, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                i, data[i].data, testUtilErrorName(ret), symbol->width,
                testUtilEscape(symbol->errtxt, (int) strlen(symbol->errtxt), errtxt_escaped, sizeof(errtxt_escaped)),
                data[i].comment);
        } else {
            assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
            assert_zero(strcmp(symbol->errtxt, data[i].ret_errtxt), "i:%d errtxt %s != %s\n",
                        i, symbol->errtxt, data[i].ret_errtxt);
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d width %d != expected_width %d (%s)\n",
                            i, symbol->width, data[i].expected_width, symbol->errtxt);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_ean8_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
        int ret;
        int expected_width;
        const char *ret_errtxt;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "1234567", 0, 67, "", "" },
        /*  1*/ { "123456A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 7 in input (digits and \"+\" or space only)", "" },
        /*  2*/ { "12345A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 6 in input (digits and \"+\" or space only)", "" },
        /*  3*/ { "123A56", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 4 in input (digits and \"+\" or space only)", "" },
        /*  4*/ { "12345670", 0, 67, "", "" },
        /*  5*/ { "0000012345670", 0, 67, "", "GTIN-13 format" },
        /*  6*/ { "12345678", ZINT_ERROR_INVALID_CHECK, -1, "Error 276: Invalid EAN-8 check digit '8', expecting '0'", "" },
        /*  7*/ { "1234567+1", ZINT_WARN_NONCOMPLIANT, 94, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /*  8*/ { "1234567 1", ZINT_WARN_NONCOMPLIANT, 94, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /*  9*/ { "12345670+1", ZINT_WARN_NONCOMPLIANT, 94, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 10*/ { "12345670 1", ZINT_WARN_NONCOMPLIANT, 94, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 11*/ { "12345678+1", ZINT_ERROR_INVALID_CHECK, -1, "Error 276: Invalid EAN-8 check digit '8', expecting '0'", "" },
        /* 12*/ { "1234567+12", ZINT_WARN_NONCOMPLIANT, 94, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 13*/ { "1234567 12", ZINT_WARN_NONCOMPLIANT, 94, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 14*/ { "12345670+12", ZINT_WARN_NONCOMPLIANT, 94, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 15*/ { "12345678+12", ZINT_ERROR_INVALID_CHECK, -1, "Error 276: Invalid EAN-8 check digit '8', expecting '0'", "" },
        /* 16*/ { "000001234567012", ZINT_WARN_NONCOMPLIANT, 94, "Warning 292: EAN-8 with add-on is non-standard", "GTIN-13 format, with 2-digit add-on" },
        /* 17*/ { "123456789+12", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 9 too long (maximum 8)", "" },
        /* 18*/ { "1234567890+12", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 10 too long (maximum 8)", "" },
        /* 19*/ { "12345678901+12", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 11 too long (maximum 8)", "" },
        /* 20*/ { "1234567+123", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 21*/ { "1234567 123", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 22*/ { "12345670+123", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 23*/ { "12345678+123", ZINT_ERROR_INVALID_CHECK, -1, "Error 276: Invalid EAN-8 check digit '8', expecting '0'", "" },
        /* 24*/ { "123456789+123", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 9 too long (maximum 8)", "" },
        /* 25*/ { "1234567890+123", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 10 too long (maximum 8)", "" },
        /* 26*/ { "1234567+1234", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 27*/ { "1234567 1234", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 28*/ { "12345670+1234", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 29*/ { "12345678+1234", ZINT_ERROR_INVALID_CHECK, -1, "Error 276: Invalid EAN-8 check digit '8', expecting '0'", "" },
        /* 30*/ { "123456789+1234", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 9 too long (maximum 8)", "" },
        /* 31*/ { "1234567+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 32*/ { "1234567 12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 33*/ { "12345670+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 34*/ { "000001234567012345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "GTIN-13 format, with 5-digit add-on" },
        /* 35*/ { "123456789+12345", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 9 too long (maximum 8)", "" },
        /* 36*/ { "12345678+12345", ZINT_ERROR_INVALID_CHECK, -1, "Error 276: Invalid EAN-8 check digit '8', expecting '0'", "" },
        /* 37*/ { "1234567+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 38*/ { "1234567 123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 39*/ { "12345670+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 40*/ { "123456+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 41*/ { "12345678901+1234567", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 11 too long (maximum 8)", "" },
        /* 42*/ { "123456789+123456", ZINT_ERROR_TOO_LONG, -1, "Error 298: Input EAN length 9 too long (maximum 8)", "" },
        /* 43*/ { "12345670+1234567", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 7 too long (maximum 5)", "" },
        /* 44*/ { "12345670+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 45*/ { "1234567+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 46*/ { "123456+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 47*/ { "12345+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 48*/ { "1234+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 49*/ { "123+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 50*/ { "12+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 51*/ { "1+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 52*/ { "1+12345678901234", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 14 too long (maximum 5)", "" },
        /* 53*/ { "1+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 54*/ { "1++12345", ZINT_ERROR_INVALID_DATA, -1, "Error 293: Invalid add-on data (one \"+\" or space only)", "" },
        /* 55*/ { "1 +12345", ZINT_ERROR_INVALID_DATA, -1, "Error 293: Invalid add-on data (one \"+\" or space only)", "" },
        /* 56*/ { "1+", 0, 67, "", "" },
        /* 57*/ { "1 ", 0, 67, "", "" },
        /* 58*/ { "+1", ZINT_WARN_NONCOMPLIANT, 94, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 59*/ { " 1", ZINT_WARN_NONCOMPLIANT, 94, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 60*/ { "+", 0, 67, "", "" },
        /* 61*/ { " ", 0, 67, "", "" },
        /* 62*/ { "1", 0, 67, "", "" },
        /* 63*/ { "12", 0, 67, "", "" },
        /* 64*/ { "123", 0, 67, "", "" },
        /* 65*/ { "1234", 0, 67, "", "" },
        /* 66*/ { "12345", 0, 67, "", "" },
        /* 67*/ { "123456", 0, 67, "", "" },
        /* 68*/ { "12345670901234", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 14 too long (maximum 8)", "" },
        /* 69*/ { "1234567090123A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 14 in input (digits and \"+\" or space only)", "" },
        /* 70*/ { "123456709012345", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 15 too long (maximum 8)", "" },
        /* 71*/ { "12345670901234A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 15 in input (digits and \"+\" or space only)", "" },
        /* 72*/ { "1234567090123456", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 16 too long (maximum 8)", "" },
        /* 73*/ { "12345670901234567", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 17 too long (maximum 8)", "" },
        /* 74*/ { "123456709012345678", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 18 too long (maximum 8)", "" },
        /* 75*/ { "1234567090123456789", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 19 too long (maximum 8)", "" },
        /* 76*/ { "1234567090128", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 13 too long (maximum 8)", "" },
        /* 77*/ { "123456709012A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 13 in input (digits and \"+\" or space only)", "" },
        /* 78*/ { "123456709012", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 12 too long (maximum 8)", "" },
        /* 79*/ { "12345670901", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 11 too long (maximum 8)", "" },
        /* 80*/ { "1234567090", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 10 too long (maximum 8)", "" },
        /* 81*/ { "123456709", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 9 too long (maximum 8)", "" },
        /* 82*/ { "12345670+1234A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 14 in input (digits and \"+\" or space only)", "" },
        /* 83*/ { "12345670+12345A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 15 in input (digits and \"+\" or space only)", "" },
        /* 84*/ { "1234567890128+123456", ZINT_ERROR_TOO_LONG, -1, "Error 283: Input length 20 too long (maximum 19)", "" },
        /* 85*/ { "1234565+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 86*/ { "123456+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 87*/ { "123457+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 88*/ { "12345+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 89*/ { "12348+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 90*/ { "1234+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 91*/ { "1236+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 92*/ { "123+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 93*/ { "12+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 94*/ { "17+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 95*/ { "0+12345", ZINT_WARN_NONCOMPLIANT, 121, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 96*/ { "12345678901234", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 14 too long (maximum 8)", "" },
        /* 97*/ { "1234567890123A", ZINT_ERROR_INVALID_DATA, -1, "Error 284: Invalid character at position 14 in input (digits and \"+\" or space only)", "" },
        /* 98*/ { "123456789012345", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 15 too long (maximum 8)", "" },
        /* 99*/ { "1234567890123456", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 16 too long (maximum 8)", "" },
        /*100*/ { "12345678901234567", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 17 too long (maximum 8)", "" },
        /*101*/ { "123456789012345678", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 18 too long (maximum 8)", "" },
        /*102*/ { "1234567890123456789", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 19 too long (maximum 8)", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char errtxt_escaped[256];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_EAN8, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                i, data[i].data, testUtilErrorName(ret), symbol->width,
                testUtilEscape(symbol->errtxt, (int) strlen(symbol->errtxt), errtxt_escaped, sizeof(errtxt_escaped)),
                data[i].comment);
        } else {
            assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
            assert_zero(strcmp(symbol->errtxt, data[i].ret_errtxt), "i:%d errtxt %s != %s\n",
                        i, symbol->errtxt, data[i].ret_errtxt);
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d width %d != expected_width %d (%s)\n",
                            i, symbol->width, data[i].expected_width, symbol->errtxt);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_ean_2addon_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
        int ret;
        int expected_width;
        const char *ret_errtxt;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "12", 0, 20, "", "" },
        /*  1*/ { "1A", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /*  2*/ { "12A", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 3 in input (digits only)", "" },
        /*  3*/ { "123A", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 4 in input (digits only)", "" },
        /*  4*/ { "123", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 3 too long (maximum 2)", "" },
        /*  5*/ { "12345", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 5 too long (maximum 2)", "" },
        /*  6*/ { "1234567890123", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 13 too long (maximum 2)", "" },
        /*  7*/ { "12345678901234", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 14 too long (maximum 2)", "" },
        /*  8*/ { "1+12", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /*  9*/ { "1 12", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /* 10*/ { "1+12345", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /* 11*/ { "1 12345", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /* 12*/ { "12+12", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 3 in input (digits only)", "" },
        /* 13*/ { "12+12345", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 3 in input (digits only)", "" },
        /* 14*/ { "1+", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /* 15*/ { "1 ", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /* 16*/ { "+1", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 1 in input (digits only)", "" },
        /* 17*/ { " 1", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 1 in input (digits only)", "" },
        /* 18*/ { "+", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 1 in input (digits only)", "" },
        /* 19*/ { " ", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 1 in input (digits only)", "" },
        /* 20*/ { "1", 0, 20, "", "" },
        /* 21*/ { "0", 0, 20, "", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char errtxt_escaped[256];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_EAN_2ADDON, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                i, data[i].data, testUtilErrorName(ret), symbol->width,
                testUtilEscape(symbol->errtxt, (int) strlen(symbol->errtxt), errtxt_escaped, sizeof(errtxt_escaped)),
                data[i].comment);
        } else {
            assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
            assert_zero(strcmp(symbol->errtxt, data[i].ret_errtxt), "i:%d errtxt %s != %s\n",
                        i, symbol->errtxt, data[i].ret_errtxt);
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d width %d != expected_width %d (%s)\n",
                            i, symbol->width, data[i].expected_width, symbol->errtxt);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_ean_5addon_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
        int ret;
        int expected_width;
        const char *ret_errtxt;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "12345", 0, 47, "", "" },
        /*  1*/ { "1234A", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 5 in input (digits only)", "" },
        /*  2*/ { "12A", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 3 in input (digits only)", "" },
        /*  3*/ { "123A", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 4 in input (digits only)", "" },
        /*  4*/ { "123456", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 6 too long (maximum 5)", "" },
        /*  5*/ { "1234567", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 7 too long (maximum 5)", "" },
        /*  6*/ { "1234567890123", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 13 too long (maximum 5)", "" },
        /*  7*/ { "12345678901234", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 14 too long (maximum 5)", "" },
        /*  8*/ { "1+12345", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /*  9*/ { "1 12345", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /* 10*/ { "1+12", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /* 11*/ { "1 12", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /* 12*/ { "12+12345", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 3 in input (digits only)", "" },
        /* 13*/ { "12+12", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 3 in input (digits only)", "" },
        /* 14*/ { "123+12345", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 4 in input (digits only)", "" },
        /* 15*/ { "1234+12345", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 5 in input (digits only)", "" },
        /* 16*/ { "12345+12345", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 6 in input (digits only)", "" },
        /* 17*/ { "1+", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /* 18*/ { "1 ", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 2 in input (digits only)", "" },
        /* 19*/ { "+1", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 1 in input (digits only)", "" },
        /* 20*/ { " 1", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 1 in input (digits only)", "" },
        /* 21*/ { "+", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 1 in input (digits only)", "" },
        /* 22*/ { " ", ZINT_ERROR_INVALID_DATA, -1, "Error 852: Invalid character at position 1 in input (digits only)", "" },
        /* 23*/ { "1", 0, 47, "", "" },
        /* 24*/ { "12", 0, 47, "", "" },
        /* 25*/ { "123", 0, 47, "", "" },
        /* 26*/ { "1234", 0, 47, "", "" },
        /* 27*/ { "0", 0, 47, "", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char errtxt_escaped[256];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_EAN_5ADDON, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                i, data[i].data, testUtilErrorName(ret), symbol->width,
                testUtilEscape(symbol->errtxt, (int) strlen(symbol->errtxt), errtxt_escaped, sizeof(errtxt_escaped)),
                data[i].comment);
        } else {
            assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
            assert_zero(strcmp(symbol->errtxt, data[i].ret_errtxt), "i:%d errtxt %s != %s\n",
                        i, symbol->errtxt, data[i].ret_errtxt);
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d width %d != expected_width %d (%s)\n",
                            i, symbol->width, data[i].expected_width, symbol->errtxt);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_eanx_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *data;
        int ret;
        const char *ret_errtxt;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_EANX, "123456789012", 0, "", "" },
        /*  1*/ { BARCODE_EANX, "12345678901A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 12 in input (digits and \"+\" or space only)", "" },
        /*  2*/ { BARCODE_EANX, "12345A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 6 in input (digits and \"+\" or space only)", "" },
        /*  3*/ { BARCODE_EANX, "123A56", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 4 in input (digits and \"+\" or space only)", "" },
        /*  4*/ { BARCODE_EANX, "12345678901", 0, "", "" },
        /*  5*/ { BARCODE_EANX, "1234567890128", 0, "", "EANX accepts CHK (treated as such if no leading zeroes required)" },
        /*  6*/ { BARCODE_EANX, "1234567890120", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '0', expecting '8'", "" },
        /*  7*/ { BARCODE_EANX, "123456789012+1", 0, "", "" },
        /*  8*/ { BARCODE_EANX, "1234567890128+1", 0, "", "" },
        /*  9*/ { BARCODE_EANX, "1234567890121+1", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '1', expecting '8'", "" },
        /* 10*/ { BARCODE_EANX, "123456789012+12", 0, "", "" },
        /* 11*/ { BARCODE_EANX, "1234567890128+12", 0, "", "" },
        /* 12*/ { BARCODE_EANX, "1234567890122+12", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '2', expecting '8'", "" },
        /* 13*/ { BARCODE_EANX, "12345678901234+12", ZINT_ERROR_TOO_LONG, "Error 298: Input EAN length 14 too long (maximum 13)", "" },
        /* 14*/ { BARCODE_EANX, "123456789012345+12", ZINT_ERROR_TOO_LONG, "Error 298: Input EAN length 15 too long (maximum 13)", "" },
        /* 15*/ { BARCODE_EANX, "1234567890123456+12", ZINT_ERROR_TOO_LONG, "Error 298: Input EAN length 16 too long (maximum 13)", "" },
        /* 16*/ { BARCODE_EANX, "123456789012+123", 0, "", "" },
        /* 17*/ { BARCODE_EANX, "1234567890128+123", 0, "", "" },
        /* 18*/ { BARCODE_EANX, "1234567890123+123", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '3', expecting '8'", "" },
        /* 19*/ { BARCODE_EANX, "12345678901234+123", ZINT_ERROR_TOO_LONG, "Error 298: Input EAN length 14 too long (maximum 13)", "" },
        /* 20*/ { BARCODE_EANX, "123456789012345+123", ZINT_ERROR_TOO_LONG, "Error 298: Input EAN length 15 too long (maximum 13)", "" },
        /* 21*/ { BARCODE_EANX, "123456789012+1234", 0, "", "" },
        /* 22*/ { BARCODE_EANX, "1234567890128+1234", 0, "", "" },
        /* 23*/ { BARCODE_EANX, "1234567890124+1234", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '4', expecting '8'", "" },
        /* 24*/ { BARCODE_EANX, "12345678901234+1234", ZINT_ERROR_TOO_LONG, "Error 298: Input EAN length 14 too long (maximum 13)", "" },
        /* 25*/ { BARCODE_EANX, "123456789012+12345", 0, "", "" },
        /* 26*/ { BARCODE_EANX, "1234567890128+12345", 0, "", "" },
        /* 27*/ { BARCODE_EANX, "12345678901234+12345", ZINT_ERROR_TOO_LONG, "Error 283: Input length 20 too long (maximum 19)", "" },
        /* 28*/ { BARCODE_EANX, "1234567890125+12345", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '5', expecting '8'", "" },
        /* 29*/ { BARCODE_EANX, "123456789012+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 30*/ { BARCODE_EANX, "1234567890128+123456", ZINT_ERROR_TOO_LONG, "Error 283: Input length 20 too long (maximum 19)", "" },
        /* 31*/ { BARCODE_EANX, "12345678901+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 32*/ { BARCODE_EANX, "12345678901+1234567", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 7 too long (maximum 5)", "" },
        /* 33*/ { BARCODE_EANX, "1234567890+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 34*/ { BARCODE_EANX, "1234567890+1234567", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 7 too long (maximum 5)", "" },
        /* 35*/ { BARCODE_EANX, "123456789+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 36*/ { BARCODE_EANX, "123456789+1234567", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 7 too long (maximum 5)", "" },
        /* 37*/ { BARCODE_EANX, "12345678+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 38*/ { BARCODE_EANX, "1234567+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "EAN-8" },
        /* 39*/ { BARCODE_EANX, "123456+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 40*/ { BARCODE_EANX, "12345+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 41*/ { BARCODE_EANX, "1234+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 42*/ { BARCODE_EANX, "123+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 43*/ { BARCODE_EANX, "12+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 44*/ { BARCODE_EANX, "1+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 45*/ { BARCODE_EANX, "1+12345678901234", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 14 too long (maximum 5)", "" },
        /* 46*/ { BARCODE_EANX, "1+12345", ZINT_WARN_NONCOMPLIANT, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /* 47*/ { BARCODE_EANX, "1+", 0, "", "EAN-2" },
        /* 48*/ { BARCODE_EANX, "+1", ZINT_WARN_NONCOMPLIANT, "Warning 292: EAN-8 with add-on is non-standard", "EAN-8" },
        /* 49*/ { BARCODE_EANX, "+", 0, "", "EAN-2" },
        /* 50*/ { BARCODE_EANX, "1", 0, "", "EAN-2" },
        /* 51*/ { BARCODE_EANX, "12", 0, "", "EAN-2" },
        /* 52*/ { BARCODE_EANX, "123", 0, "", "EAN-5" },
        /* 53*/ { BARCODE_EANX, "12345678901234", ZINT_ERROR_TOO_LONG, "Error 294: Input length 14 too long (maximum 13)", "" },
        /* 54*/ { BARCODE_EANX, "1234567890123A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 14 in input (digits and \"+\" or space only)", "" },
        /* 55*/ { BARCODE_EANX, "123456789012345", ZINT_ERROR_TOO_LONG, "Error 294: Input length 15 too long (maximum 13)", "" },
        /* 56*/ { BARCODE_EANX, "12345678901234A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 15 in input (digits and \"+\" or space only)", "" },
        /* 57*/ { BARCODE_EANX, "1234567890123456", ZINT_ERROR_TOO_LONG, "Error 294: Input length 16 too long (maximum 13)", "" },
        /* 58*/ { BARCODE_EANX, "12345678901234567", ZINT_ERROR_TOO_LONG, "Error 294: Input length 17 too long (maximum 13)", "" },
        /* 59*/ { BARCODE_EANX, "123456789012345678", ZINT_ERROR_TOO_LONG, "Error 294: Input length 18 too long (maximum 13)", "" },
        /* 60*/ { BARCODE_EANX, "1234567890123456789", ZINT_ERROR_TOO_LONG, "Error 294: Input length 19 too long (maximum 13)", "" },
        /* 61*/ { BARCODE_EANX_CHK, "1234567890128", 0, "", "" },
        /* 62*/ { BARCODE_EANX_CHK, "1234567890126", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '6', expecting '8'", "" },
        /* 63*/ { BARCODE_EANX_CHK, "123456789012A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 13 in input (digits and \"+\" or space only)", "" },
        /* 64*/ { BARCODE_EANX_CHK, "123456789012", 0, "", "Note: this is '0123456789012' with '2' happening to be the correct check digit" },
        /* 65*/ { BARCODE_EANX_CHK, "123456789013", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '3', expecting '2'", "" },
        /* 66*/ { BARCODE_EANX_CHK, "12345678901", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '1', expecting '5'", "" },
        /* 67*/ { BARCODE_EANX_CHK, "12345678905", 0, "", "" },
        /* 68*/ { BARCODE_EANX_CHK, "1234567890", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '0', expecting '5'", "" },
        /* 69*/ { BARCODE_EANX_CHK, "123456789", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '9', expecting '4'", "" },
        /* 70*/ { BARCODE_EANX_CHK, "12345678", ZINT_ERROR_INVALID_CHECK, "Error 276: Invalid EAN-8 check digit '8', expecting '0'", "EAN-8" },
        /* 71*/ { BARCODE_EANX_CHK, "1234567", ZINT_ERROR_INVALID_CHECK, "Error 276: Invalid EAN-8 check digit '7', expecting '5'", "" },
        /* 72*/ { BARCODE_EANX_CHK, "123456", ZINT_ERROR_INVALID_CHECK, "Error 276: Invalid EAN-8 check digit '6', expecting '7'", "" },
        /* 73*/ { BARCODE_EANX_CHK, "12345", 0, "", "EAN-5" },
        /* 74*/ { BARCODE_EANX_CHK, "1234", 0, "", "" },
        /* 75*/ { BARCODE_EANX_CHK, "123", 0, "", "" },
        /* 76*/ { BARCODE_EANX_CHK, "12", 0, "", "EAN-2" },
        /* 77*/ { BARCODE_EANX_CHK, "1", 0, "", "" },
        /* 78*/ { BARCODE_EANX_CHK, "123456789012+1", 0, "", "" },
        /* 79*/ { BARCODE_EANX_CHK, "1234567890128+1", 0, "", "" },
        /* 80*/ { BARCODE_EANX_CHK, "1234567890127+1", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '7', expecting '8'", "" },
        /* 81*/ { BARCODE_EANX_CHK, "123456789012+12", 0, "", "" },
        /* 82*/ { BARCODE_EANX_CHK, "1234567890128+12", 0, "", "" },
        /* 83*/ { BARCODE_EANX_CHK, "1234567890129+12", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '9', expecting '8'", "" },
        /* 84*/ { BARCODE_EANX_CHK, "123456789012+123", 0, "", "" },
        /* 85*/ { BARCODE_EANX_CHK, "1234567890128+123", 0, "", "" },
        /* 86*/ { BARCODE_EANX_CHK, "1234567890120+1234", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '0', expecting '8'", "" },
        /* 87*/ { BARCODE_EANX_CHK, "123456789012+1234", 0, "", "" },
        /* 88*/ { BARCODE_EANX_CHK, "1234567890128+1234", 0, "", "" },
        /* 89*/ { BARCODE_EANX_CHK, "1234567890121+1234", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '1', expecting '8'", "" },
        /* 90*/ { BARCODE_EANX_CHK, "123456789012+12345", 0, "", "" },
        /* 91*/ { BARCODE_EANX_CHK, "1234567890128+12345", 0, "", "" },
        /* 92*/ { BARCODE_EANX_CHK, "1234567890122+12345", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '2', expecting '8'", "" },
        /* 93*/ { BARCODE_EANX_CHK, "1234567890122+1234A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 19 in input (digits and \"+\" or space only)", "" },
        /* 94*/ { BARCODE_EANX_CHK, "123456789012+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 95*/ { BARCODE_EANX_CHK, "123456789012+12345A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 19 in input (digits and \"+\" or space only)", "" },
        /* 96*/ { BARCODE_EANX_CHK, "1234567890128+123456", ZINT_ERROR_TOO_LONG, "Error 283: Input length 20 too long (maximum 19)", "" },
        /* 97*/ { BARCODE_EANX_CHK, "12345678901+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 98*/ { BARCODE_EANX_CHK, "12345678901+1234567", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 7 too long (maximum 5)", "" },
        /* 99*/ { BARCODE_EANX_CHK, "12345678901+12345", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '1', expecting '5'", "" },
        /*100*/ { BARCODE_EANX_CHK, "1234567890+12345", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '0', expecting '5'", "" },
        /*101*/ { BARCODE_EANX_CHK, "1234567890+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /*102*/ { BARCODE_EANX_CHK, "123456789+12345", ZINT_ERROR_INVALID_CHECK, "Error 275: Invalid check digit '9', expecting '4'", "" },
        /*103*/ { BARCODE_EANX_CHK, "12345678+12345", ZINT_ERROR_INVALID_CHECK, "Error 276: Invalid EAN-8 check digit '8', expecting '0'", "EAN-8" },
        /*104*/ { BARCODE_EANX_CHK, "12345670+12345", ZINT_WARN_NONCOMPLIANT, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /*105*/ { BARCODE_EANX_CHK, "1234567+12345", ZINT_ERROR_INVALID_CHECK, "Error 276: Invalid EAN-8 check digit '7', expecting '5'", "" },
        /*106*/ { BARCODE_EANX_CHK, "1234565+12345", ZINT_WARN_NONCOMPLIANT, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /*107*/ { BARCODE_EANX_CHK, "123456+12345", ZINT_ERROR_INVALID_CHECK, "Error 276: Invalid EAN-8 check digit '6', expecting '7'", "" },
        /*108*/ { BARCODE_EANX_CHK, "123457+12345", ZINT_WARN_NONCOMPLIANT, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /*109*/ { BARCODE_EANX_CHK, "12345+12345", ZINT_ERROR_INVALID_CHECK, "Error 276: Invalid EAN-8 check digit '5', expecting '8'", "" },
        /*110*/ { BARCODE_EANX_CHK, "12348+12345", ZINT_WARN_NONCOMPLIANT, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /*111*/ { BARCODE_EANX_CHK, "1234+12345", ZINT_ERROR_INVALID_CHECK, "Error 276: Invalid EAN-8 check digit '4', expecting '6'", "" },
        /*112*/ { BARCODE_EANX_CHK, "1236+12345", ZINT_WARN_NONCOMPLIANT, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /*113*/ { BARCODE_EANX_CHK, "123+12345", ZINT_WARN_NONCOMPLIANT, "Warning 292: EAN-8 with add-on is non-standard", "3 happens to be correct check digit" },
        /*114*/ { BARCODE_EANX_CHK, "124+12345", ZINT_ERROR_INVALID_CHECK, "Error 276: Invalid EAN-8 check digit '4', expecting '3'", "" },
        /*115*/ { BARCODE_EANX_CHK, "12+12345", ZINT_ERROR_INVALID_CHECK, "Error 276: Invalid EAN-8 check digit '2', expecting '7'", "" },
        /*116*/ { BARCODE_EANX_CHK, "17+12345", ZINT_WARN_NONCOMPLIANT, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /*117*/ { BARCODE_EANX_CHK, "1+12345", ZINT_ERROR_INVALID_CHECK, "Error 276: Invalid EAN-8 check digit '1', expecting '0'", "" },
        /*118*/ { BARCODE_EANX_CHK, "0+12345", ZINT_WARN_NONCOMPLIANT, "Warning 292: EAN-8 with add-on is non-standard", "" },
        /*119*/ { BARCODE_EANX_CHK, "0+123456", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /*120*/ { BARCODE_EANX_CHK, "1+12345678901234", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 14 too long (maximum 5)", "" },
        /*121*/ { BARCODE_EANX_CHK, "0+12345678901234", ZINT_ERROR_TOO_LONG, "Error 297: Input add-on length 14 too long (maximum 5)", "" },
        /*122*/ { BARCODE_EANX_CHK, "1+", 0, "", "EAN-2" },
        /*123*/ { BARCODE_EANX_CHK, "+1", ZINT_WARN_NONCOMPLIANT, "Warning 292: EAN-8 with add-on is non-standard", "EAN-8" },
        /*124*/ { BARCODE_EANX_CHK, "+", 0, "", "EAN-2" },
        /*125*/ { BARCODE_EANX_CHK, "12345678901234", ZINT_ERROR_TOO_LONG, "Error 294: Input length 14 too long (maximum 13)", "" },
        /*126*/ { BARCODE_EANX_CHK, "1234567890123A", ZINT_ERROR_INVALID_DATA, "Error 284: Invalid character at position 14 in input (digits and \"+\" or space only)", "" },
        /*127*/ { BARCODE_EANX_CHK, "123456789012345", ZINT_ERROR_TOO_LONG, "Error 294: Input length 15 too long (maximum 13)", "" },
        /*128*/ { BARCODE_EANX_CHK, "1234567890123456", ZINT_ERROR_TOO_LONG, "Error 294: Input length 16 too long (maximum 13)", "" },
        /*129*/ { BARCODE_EANX_CHK, "12345678901234567", ZINT_ERROR_TOO_LONG, "Error 294: Input length 17 too long (maximum 13)", "" },
        /*130*/ { BARCODE_EANX_CHK, "123456789012345678", ZINT_ERROR_TOO_LONG, "Error 294: Input length 18 too long (maximum 13)", "" },
        /*131*/ { BARCODE_EANX_CHK, "1234567890123456789", ZINT_ERROR_TOO_LONG, "Error 294: Input length 19 too long (maximum 13)", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char errtxt_escaped[256];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                i, testUtilBarcodeName(data[i].symbology), data[i].data, testUtilErrorName(ret),
                testUtilEscape(symbol->errtxt, (int) strlen(symbol->errtxt), errtxt_escaped, sizeof(errtxt_escaped)),
                data[i].comment);
        } else {
            assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
            assert_zero(strcmp(symbol->errtxt, data[i].ret_errtxt), "i:%d errtxt %s != %s\n",
                        i, symbol->errtxt, data[i].ret_errtxt);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_isbn_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
        int ret_encode;
        int ret_vector;
        const char *ret_errtxt;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "0", 0, 0, "", "Left zero-padded if < 10 chars" },
        /*  1*/ { "1", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit '1', expecting '0'", "" },
        /*  2*/ { "X", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit 'X', expecting '0'", "" },
        /*  3*/ { "12", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit '2', expecting '9'", "" },
        /*  4*/ { "19", 0, 0, "", "" },
        /*  5*/ { "X9", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /*  6*/ { "123", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit '3', expecting '4'", "" },
        /*  7*/ { "124", 0, 0, "", "" },
        /*  8*/ { "1X4", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /*  9*/ { "1234", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit '4', expecting '6'", "" },
        /* 10*/ { "1236", 0, 0, "", "" },
        /* 11*/ { "12X6", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 12*/ { "12345", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit '5', expecting '3'", "" },
        /* 13*/ { "12343", 0, 0, "", "" },
        /* 14*/ { "123X3", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 15*/ { "123456", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit '6', expecting '5'", "" },
        /* 16*/ { "123455", 0, 0, "", "" },
        /* 17*/ { "1234X5", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 18*/ { "1234567", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit '7', expecting '0'", "" },
        /* 19*/ { "1234560", 0, 0, "", "" },
        /* 20*/ { "12345X0", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 21*/ { "12345678", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit '8', expecting '9'", "" },
        /* 22*/ { "12345679", 0, 0, "", "9 is correct check digit" },
        /* 23*/ { "98765434", 0, 0, "", "4 is correct check digit" },
        /* 24*/ { "123456X9", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 25*/ { "123456789", 0, 0, "", "" },
        /* 26*/ { "340013817", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit '7', expecting '8'", "" },
        /* 27*/ { "340013818", 0, 0, "", "8 is correct check digit" },
        /* 28*/ { "902888455", 0, 0, "", "5 is correct check digit" },
        /* 29*/ { "9028884X5", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 30*/ { "0123456789", 0, 0, "", "" },
        /* 31*/ { "1234567890", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid ISBN check digit '0', expecting 'X'", "" },
        /* 32*/ { "123456789X", 0, 0, "", "X is correct check digit" },
        /* 33*/ { "123456789x", 0, 0, "", "x is correct check digit" },
        /* 34*/ { "8175257660", 0, 0, "", "0 is correct check digit" },
        /* 35*/ { "0590764845", 0, 0, "", "5 is correct check digit" },
        /* 36*/ { "0906495741", 0, 0, "", "1 is correct check digit" },
        /* 37*/ { "0140430016", 0, 0, "", "6 is correct check digit" },
        /* 38*/ { "0571086187", 0, 0, "", "7 is correct check digit" },
        /* 39*/ { "0486600882", 0, 0, "", "2 is correct check digit" },
        /* 40*/ { "04866008X2", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 41*/ { "123456789A", ZINT_ERROR_INVALID_DATA, -1, "Error 285: Invalid character at position 10 in input (digits, \"X\" and \"+\" or space only)", "" },
        /* 42*/ { "12345678901", ZINT_ERROR_TOO_LONG, -1, "Error 278: Input length 11 wrong (9, 10, or 13 characters required)", "" },
        /* 43*/ { "1234567890A", ZINT_ERROR_INVALID_DATA, -1, "Error 285: Invalid character at position 11 in input (digits, \"X\" and \"+\" or space only)", "" },
        /* 44*/ { "123456789012", ZINT_ERROR_TOO_LONG, -1, "Error 278: Input length 12 wrong (9, 10, or 13 characters required)", "" },
        /* 45*/ { "12345678901", ZINT_ERROR_TOO_LONG, -1, "Error 278: Input length 11 wrong (9, 10, or 13 characters required)", "" },
        /* 46*/ { "123456789012", ZINT_ERROR_TOO_LONG, -1, "Error 278: Input length 12 wrong (9, 10, or 13 characters required)", "" },
        /* 47*/ { "1234567890123", ZINT_ERROR_INVALID_DATA, -1, "Error 279: Invalid ISBN (must begin with \"978\" or \"979\")", "" },
        /* 48*/ { "9784567890123", ZINT_ERROR_INVALID_CHECK, -1, "Error 280: Invalid ISBN check digit '3', expecting '0'", "" },
        /* 49*/ { "9784567890120", 0, 0, "", "0 is correct check digit" },
        /* 50*/ { "9783161484100", 0, 0, "", "0 is correct check digit" },
        /* 51*/ { "9781846688225", 0, 0, "", "5 is correct check digit" },
        /* 52*/ { "9781847657954", 0, 0, "", "4 is correct check digit" },
        /* 53*/ { "9781846688188", 0, 0, "", "8 is correct check digit" },
        /* 54*/ { "9781847659293", 0, 0, "", "3 is correct check digit" },
        /* 55*/ { "97845678901201", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 14 too long (maximum 13)", "" },
        /* 56*/ { "978456789012012", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 15 too long (maximum 13)", "" },
        /* 57*/ { "3954994+12", 0, 0, "", "" },
        /* 58*/ { "3954994+1X", ZINT_ERROR_INVALID_DATA, -1, "Error 295: Invalid add-on data (digits only)", "" },
        /* 59*/ { "39549X4+12", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 60*/ { "3954994+12345", 0, 0, "", "" },
        /* 61*/ { "3954994+1234X", ZINT_ERROR_INVALID_DATA, -1, "Error 295: Invalid add-on data (digits only)", "" },
        /* 62*/ { "39549X4+12345", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 63*/ { "3954994+123456", ZINT_ERROR_TOO_LONG, -1, "Error 297: Input add-on length 6 too long (maximum 5)", "" },
        /* 64*/ { "3954994+", 0, 0, "", "" },
        /* 65*/ { "3954X94+", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 66*/ { "61954993+1", 0, 0, "", "" },
        /* 67*/ { "61954993+X", ZINT_ERROR_INVALID_DATA, -1, "Error 295: Invalid add-on data (digits only)", "" },
        /* 68*/ { "619549X3+1", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 69*/ { "61954992+123", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit '2', expecting '3'", "" },
        /* 70*/ { "61954993+123", 0, 0, "", "" },
        /* 71*/ { "61954993+12X", ZINT_ERROR_INVALID_DATA, -1, "Error 295: Invalid add-on data (digits only)", "" },
        /* 72*/ { "619549X3+123", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 73*/ { "361954990+12", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid SBN check digit '0', expecting '9'", "" },
        /* 74*/ { "361954999+12", 0, 0, "", "" },
        /* 75*/ { "361954999+1X", ZINT_ERROR_INVALID_DATA, -1, "Error 295: Invalid add-on data (digits only)", "" },
        /* 76*/ { "3619549X9+12", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 77*/ { "361954999+1234", 0, 0, "", "" },
        /* 78*/ { "361954999+123X", ZINT_ERROR_INVALID_DATA, -1, "Error 295: Invalid add-on data (digits only)", "" },
        /* 79*/ { "3619549X9+1234", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 80*/ { "1999000030+12", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid ISBN check digit '0', expecting 'X'", "" },
        /* 81*/ { "199900003X+12", 0, 0, "", "" },
        /* 82*/ { "199900003x+12", 0, 0, "", "" },
        /* 83*/ { "19990000XX+12", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 84*/ { "199900003X+1X", ZINT_ERROR_INVALID_DATA, -1, "Error 295: Invalid add-on data (digits only)", "" },
        /* 85*/ { "1999000031+12345", ZINT_ERROR_INVALID_CHECK, -1, "Error 281: Invalid ISBN check digit '1', expecting 'X'", "" },
        /* 86*/ { "199900003X+12345", 0, 0, "", "" },
        /* 87*/ { "199900003x+12345", 0, 0, "", "" },
        /* 88*/ { "199900003X+1234X", ZINT_ERROR_INVALID_DATA, -1, "Error 295: Invalid add-on data (digits only)", "" },
        /* 89*/ { "19990000XX+12345", ZINT_ERROR_INVALID_DATA, -1, "Error 296: Invalid character in input, \"X\" allowed in last position only", "" },
        /* 90*/ { "199900003X+1234A", ZINT_ERROR_INVALID_DATA, -1, "Error 285: Invalid character at position 16 in input (digits, \"X\" and \"+\" or space only)", "" },
        /* 91*/ { "9791234567895+12", ZINT_ERROR_INVALID_CHECK, -1, "Error 280: Invalid ISBN check digit '5', expecting '6'", "" },
        /* 92*/ { "9791234567896+12", 0, 0, "", "" },
        /* 93*/ { "9791234567896+1X", ZINT_ERROR_INVALID_DATA, -1, "Error 295: Invalid add-on data (digits only)", "" },
        /* 94*/ { "97912345678X6+12", ZINT_ERROR_INVALID_DATA, -1, "Error 282: Invalid character in input, \"X\" not allowed in ISBN-13", "" },
        /* 95*/ { "9791234567897+12345", ZINT_ERROR_INVALID_CHECK, -1, "Error 280: Invalid ISBN check digit '7', expecting '6'", "" },
        /* 96*/ { "9791234567896+12345", 0, 0, "", "" },
        /* 97*/ { "9791234567896+1234X", ZINT_ERROR_INVALID_DATA, -1, "Error 295: Invalid add-on data (digits only)", "" },
        /* 98*/ { "979123456X896+12345", ZINT_ERROR_INVALID_DATA, -1, "Error 282: Invalid character in input, \"X\" not allowed in ISBN-13", "" },
        /* 99*/ { "9791234567892+", ZINT_ERROR_INVALID_CHECK, -1, "Error 280: Invalid ISBN check digit '2', expecting '6'", "" },
        /*100*/ { "9791234567896+", 0, 0, "", "" },
        /*101*/ { "97912345678X6+", ZINT_ERROR_INVALID_DATA, -1, "Error 282: Invalid character in input, \"X\" not allowed in ISBN-13", "" },
        /*102*/ { "97912345678961+", ZINT_ERROR_TOO_LONG, -1, "Error 294: Input length 14 too long (maximum 13)", "" },
        /*103*/ { "97912345678961+12345", ZINT_ERROR_TOO_LONG, -1, "Error 283: Input length 20 too long (maximum 19)", "" },
        /*104*/ { "9791234567896+123456", ZINT_ERROR_TOO_LONG, -1, "Error 283: Input length 20 too long (maximum 19)", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char errtxt_escaped[256];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_ISBNX, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                i, data[i].data, testUtilErrorName(data[i].ret_encode), data[i].ret_vector,
                testUtilEscape(symbol->errtxt, (int) strlen(symbol->errtxt), errtxt_escaped, sizeof(errtxt_escaped)),
                data[i].comment);
        } else {
            assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                        i, ret, data[i].ret_encode, symbol->errtxt);
            assert_zero(strcmp(symbol->errtxt, data[i].ret_errtxt), "i:%d errtxt %s != %s\n",
                        i, symbol->errtxt, data[i].ret_errtxt);

            if (data[i].ret_vector != -1) {
                ret = ZBarcode_Buffer_Vector(symbol, 0);
                assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d (%s)\n",
                            i, ret, data[i].ret_vector, symbol->errtxt);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_hrt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        const char *data;
        int ret;
        const char *expected;
        const char *expected_content;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_EAN13, -1, "12345678901", 0, "0123456789012", "" },
        /*  1*/ { BARCODE_EANX, -1, "12345678901", 0, "0123456789012", "" },
        /*  2*/ { BARCODE_EAN13, BARCODE_CONTENT_SEGS, "12345678901", 0, "0123456789012", "0123456789012" },
        /*  3*/ { BARCODE_EANX, BARCODE_CONTENT_SEGS, "12345678901", 0, "0123456789012", "0123456789012" },
        /*  4*/ { BARCODE_EAN13, -1, "123456789012", 0, "1234567890128", "" },
        /*  5*/ { BARCODE_EANX, -1, "123456789012", 0, "1234567890128", "" },
        /*  6*/ { BARCODE_EAN13, BARCODE_CONTENT_SEGS, "123456789012", 0, "1234567890128", "1234567890128" },
        /*  7*/ { BARCODE_EANX, BARCODE_CONTENT_SEGS, "123456789012", 0, "1234567890128", "1234567890128" },
        /*  8*/ { BARCODE_EAN13, -1, "1234567890128", 0, "1234567890128", "" }, /* EAN13 accepts CHK (treated as such if no leading zeroes required) */
        /*  9*/ { BARCODE_EANX, -1, "1234567890128", 0, "1234567890128", "" }, /* EANX accepts CHK (treated as such if no leading zeroes required) */
        /* 10*/ { BARCODE_EAN13, -1, "1234567890128", 0, "1234567890128", "1234567890128" },
        /* 11*/ { BARCODE_EANX_CHK, -1, "1234567890128", 0, "1234567890128", "1234567890128" },
        /* 12*/ { BARCODE_EAN13, BARCODE_CONTENT_SEGS, "1234567890128", 0, "1234567890128", "1234567890128" },
        /* 13*/ { BARCODE_EANX_CHK, BARCODE_CONTENT_SEGS, "1234567890128", 0, "1234567890128", "1234567890128" },
        /* 14*/ { BARCODE_EAN13, -1, "0123456789012", 0, "0123456789012", "0123456789012" }, /* '2' happens to be correct check digit for "012345678901" */
        /* 15*/ { BARCODE_EANX_CHK, -1, "123456789012", 0, "0123456789012", "0123456789012" }, /* '2' happens to be correct check digit for "012345678901" */
        /* 16*/ { BARCODE_EAN13, -1, "1234567890128+1", 0, "1234567890128+01", "" },
        /* 17*/ { BARCODE_EANX, -1, "1234567890128+1", 0, "1234567890128+01", "" },
        /* 18*/ { BARCODE_EAN13, BARCODE_CONTENT_SEGS, "1234567890128+1", 0, "1234567890128+01", "123456789012801" },
        /* 19*/ { BARCODE_EANX, BARCODE_CONTENT_SEGS, "1234567890128+1", 0, "1234567890128+01", "123456789012801" },
        /* 20*/ { BARCODE_EAN13, -1, "1234567890128+1", 0, "1234567890128+01", "" },
        /* 21*/ { BARCODE_EANX_CHK, -1, "1234567890128+1", 0, "1234567890128+01", "" },
        /* 22*/ { BARCODE_EAN13, BARCODE_CONTENT_SEGS, "1234567890128+1", 0, "1234567890128+01", "123456789012801" },
        /* 23*/ { BARCODE_EANX_CHK, BARCODE_CONTENT_SEGS, "1234567890128+1", 0, "1234567890128+01", "123456789012801" },
        /* 24*/ { BARCODE_EAN13, -1, "12345678", 0, "0000123456784", "" },
        /* 25*/ { BARCODE_EANX, -1, "12345678", 0, "0000123456784", "" },
        /* 26*/ { BARCODE_EAN13, BARCODE_CONTENT_SEGS, "12345678", 0, "0000123456784", "0000123456784" },
        /* 27*/ { BARCODE_EANX, BARCODE_CONTENT_SEGS, "12345678", 0, "0000123456784", "0000123456784" },
        /* 28*/ { BARCODE_EAN8, -1, "1234567", 0, "12345670", "" },
        /* 29*/ { BARCODE_EANX, -1, "1234567", 0, "12345670", "" }, /* EAN-8 */
        /* 30*/ { BARCODE_EAN8, BARCODE_CONTENT_SEGS, "1234567", 0, "12345670", "12345670" },
        /* 31*/ { BARCODE_EANX, BARCODE_CONTENT_SEGS, "1234567", 0, "12345670", "12345670" },
        /* 32*/ { BARCODE_EAN8, -1, "12345670", 0, "12345670", "" },
        /* 33*/ { BARCODE_EANX_CHK, -1, "12345670", 0, "12345670", "" }, /* EAN-8 */
        /* 34*/ { BARCODE_EAN8, BARCODE_CONTENT_SEGS, "12345670", 0, "12345670", "12345670" },
        /* 35*/ { BARCODE_EANX_CHK, BARCODE_CONTENT_SEGS, "12345670", 0, "12345670", "12345670" },
        /* 36*/ { BARCODE_EAN8, -1, "123456", 0, "01234565", "" },
        /* 37*/ { BARCODE_EANX, -1, "123456", 0, "01234565", "" }, /* EAN-8 */
        /* 38*/ { BARCODE_EAN8, BARCODE_CONTENT_SEGS, "123456", 0, "01234565", "01234565" },
        /* 39*/ { BARCODE_EANX, BARCODE_CONTENT_SEGS, "123456", 0, "01234565", "01234565" },
        /* 40*/ { BARCODE_EAN8, -1, "00123457", 0, "00123457", "" },
        /* 41*/ { BARCODE_EANX_CHK, -1, "123457", 0, "00123457", "" }, /* EAN-8 */
        /* 42*/ { BARCODE_EAN8, BARCODE_CONTENT_SEGS, "00123457", 0, "00123457", "00123457" },
        /* 43*/ { BARCODE_EANX_CHK, BARCODE_CONTENT_SEGS, "123457", 0, "00123457", "00123457" },
        /* 44*/ { BARCODE_EAN_5ADDON, -1, "12345", 0, "12345", "" },
        /* 45*/ { BARCODE_EANX, -1, "12345", 0, "12345", "" }, /* EAN-5 */
        /* 46*/ { BARCODE_EAN_5ADDON, BARCODE_CONTENT_SEGS, "12345", 0, "12345", "12345" },
        /* 47*/ { BARCODE_EANX, BARCODE_CONTENT_SEGS, "12345", 0, "12345", "12345" },
        /* 48*/ { BARCODE_EAN_5ADDON, -1, "123", 0, "00123", "" },
        /* 49*/ { BARCODE_EANX, -1, "123", 0, "00123", "" }, /* EAN-5 */
        /* 50*/ { BARCODE_EAN_5ADDON, BARCODE_CONTENT_SEGS, "123", 0, "00123", "00123" },
        /* 51*/ { BARCODE_EANX, BARCODE_CONTENT_SEGS, "123", 0, "00123", "00123" },
        /* 52*/ { BARCODE_EAN_2ADDON, -1, "12", 0, "12", "" },
        /* 53*/ { BARCODE_EANX, -1, "12", 0, "12", "" }, /* EAN-2 */
        /* 54*/ { BARCODE_EAN_2ADDON, BARCODE_CONTENT_SEGS, "12", 0, "12", "12" },
        /* 55*/ { BARCODE_EANX, BARCODE_CONTENT_SEGS, "12", 0, "12", "12" },
        /* 56*/ { BARCODE_EAN_2ADDON, -1, "1", 0, "01", "" },
        /* 57*/ { BARCODE_EANX, -1, "1", 0, "01", "" }, /* EAN-2 */
        /* 58*/ { BARCODE_EAN_2ADDON, BARCODE_CONTENT_SEGS, "1", 0, "01", "01" },
        /* 59*/ { BARCODE_EANX, BARCODE_CONTENT_SEGS, "1", 0, "01", "01" },
        /* 60*/ { BARCODE_EAN_2ADDON, -1, "0", 0, "00", "" },
        /* 61*/ { BARCODE_EANX, -1, "0", 0, "00", "" }, /* EAN-2 */
        /* 62*/ { BARCODE_EAN_2ADDON, BARCODE_CONTENT_SEGS, "0", 0, "00", "00" },
        /* 63*/ { BARCODE_EANX, BARCODE_CONTENT_SEGS, "0", 0, "00", "00" },
        /* 64*/ { BARCODE_ISBNX, -1, "0", 0, "9780000000002", "" },
        /* 65*/ { BARCODE_ISBNX, BARCODE_CONTENT_SEGS, "0", 0, "9780000000002", "9780000000002" },
        /* 66*/ { BARCODE_ISBNX, -1, "123456789X", 0, "9781234567897", "" },
        /* 67*/ { BARCODE_ISBNX, BARCODE_CONTENT_SEGS, "123456789X", 0, "9781234567897", "9781234567897" },
        /* 68*/ { BARCODE_ISBNX, -1, "9781234567897", 0, "9781234567897", "" },
        /* 69*/ { BARCODE_ISBNX, BARCODE_CONTENT_SEGS, "9781234567897", 0, "9781234567897", "9781234567897" },
        /* 70*/ { BARCODE_ISBNX, -1, "9791234567896+12", 0, "9791234567896+12", "" },
        /* 71*/ { BARCODE_ISBNX, BARCODE_CONTENT_SEGS, "9791234567896+12", 0, "9791234567896+12", "979123456789612" },
        /* 72*/ { BARCODE_UPCA, -1, "12345678901", 0, "123456789012", "" },
        /* 73*/ { BARCODE_UPCA, BARCODE_CONTENT_SEGS, "12345678901", 0, "123456789012", "0123456789012" },
        /* 74*/ { BARCODE_UPCA, -1, "123456789012", 0, "123456789012", "" },
        /* 75*/ { BARCODE_UPCA, BARCODE_CONTENT_SEGS, "123456789012", 0, "123456789012", "0123456789012" },
        /* 76*/ { BARCODE_UPCA_CHK, -1, "123456789012", 0, "123456789012", "" },
        /* 77*/ { BARCODE_UPCA_CHK, BARCODE_CONTENT_SEGS, "123456789012", 0, "123456789012", "0123456789012" },
        /* 78*/ { BARCODE_UPCA, -1, "12345678905+1", 0, "123456789050+01", "" },
        /* 79*/ { BARCODE_UPCA, BARCODE_CONTENT_SEGS, "12345678905+1", 0, "123456789050+01", "012345678905001" },
        /* 80*/ { BARCODE_UPCA_CHK, -1, "123456789050+1", 0, "123456789050+01", "" },
        /* 81*/ { BARCODE_UPCA_CHK, BARCODE_CONTENT_SEGS, "123456789050+1", 0, "123456789050+01", "012345678905001" },
        /* 82*/ { BARCODE_UPCA, -1, "123456789050+123", 0, "123456789050+00123", "" },
        /* 83*/ { BARCODE_UPCA, BARCODE_CONTENT_SEGS, "123456789050+123", 0, "123456789050+00123", "012345678905000123" },
        /* 84*/ { BARCODE_UPCA_CHK, -1, "123456789050+123", 0, "123456789050+00123", "" },
        /* 85*/ { BARCODE_UPCA_CHK, BARCODE_CONTENT_SEGS, "123456789050+123", 0, "123456789050+00123", "012345678905000123" },
        /* 86*/ { BARCODE_UPCE, -1, "12345", 0, "00123457", "" }, /* equivalent: 00123400005, hrt: 00123457, Check digit: 7 */
        /* 87*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "12345", 0, "00123457", "0001234000057" },
        /* 88*/ { BARCODE_UPCE_CHK, -1, "12344", 0, "00012344", "" }, /* equivalent: 00012000003, hrt: 00012344, Check digit: 4 */
        /* 89*/ { BARCODE_UPCE_CHK, BARCODE_CONTENT_SEGS, "12344", 0, "00012344", "0000120000034" },
        /* 90*/ { BARCODE_UPCE, -1, "123456", 0, "01234565", "" }, /* equivalent: 01234500006, hrt: 01234565, Check digit: 5 */
        /* 91*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "123456", 0, "01234565", "0012345000065" },
        /* 92*/ { BARCODE_UPCE_CHK, -1, "123457", 0, "00123457", "" }, /* equivalent: 00123400005, hrt: 00123457, Check digit: 7 */
        /* 93*/ { BARCODE_UPCE_CHK, BARCODE_CONTENT_SEGS, "123457", 0, "00123457", "0001234000057" },
        /* 94*/ { BARCODE_UPCE, -1, "1234567", 0, "12345670", "" }, /* equivalent: 12345600007, hrt: 12345670, Check digit: 0 */
        /* 95*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "1234567", 0, "12345670", "0123456000070" },
        /* 96*/ { BARCODE_UPCE_CHK, -1, "1234565", 0, "01234565", "" }, /* equivalent: 01234500006, hrt: 01234565, Check digit: 5 */
        /* 97*/ { BARCODE_UPCE_CHK, BARCODE_CONTENT_SEGS, "1234565", 0, "01234565", "0012345000065" },
        /* 98*/ { BARCODE_UPCE_CHK, -1, "12345670", 0, "12345670", "" }, /* equivalent: 12345600007, hrt: 12345670, Check digit: 0 */
        /* 99*/ { BARCODE_UPCE_CHK, BARCODE_CONTENT_SEGS, "12345670", 0, "12345670", "0123456000070" },
        /*100*/ { BARCODE_UPCE, -1, "2345678", ZINT_WARN_INVALID_OPTION, "03456781", "" }, /* 2 ignored, equivalent: 03456700008, hrt: 03456781, Check digit: 1 */
        /*101*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "2345678", ZINT_WARN_INVALID_OPTION, "03456781", "0034567000081" },
        /*102*/ { BARCODE_UPCE_CHK, -1, "23456781", ZINT_WARN_INVALID_OPTION, "03456781", "" }, /* 2 ignored, equivalent: 03456700008, hrt: 03456781, Check digit: 1 */
        /*103*/ { BARCODE_UPCE, -1, "123455", 0, "01234558", "" }, /* equivalent: 01234500005, hrt: 01234558, Check digit: 8 (BS 797 Rule 3 (a)) */
        /*104*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "123455", 0, "01234558", "0012345000058" },
        /*105*/ { BARCODE_UPCE_CHK, -1, "1234558", 0, "01234558", "" }, /* equivalent: 01234500005, hrt: 01234558, Check digit: 8 (BS 797 Rule 3 (a)) */
        /*106*/ { BARCODE_UPCE, -1, "456784", 0, "04567840", "" }, /* equivalent: 04567000008, hrt: 04567840, Check digit: 0 (BS 797 Rule 3 (b)) */
        /*107*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "456784", 0, "04567840", "0045670000080" },
        /*108*/ { BARCODE_UPCE_CHK, -1, "4567840", 0, "04567840", "" }, /* equivalent: 04567000008, hrt: 04567840, Check digit: 0 (BS 797 Rule 3 (b)) */
        /*109*/ { BARCODE_UPCE, -1, "345670", 0, "03456703", "" }, /* equivalent: 03400000567, hrt: 03456703, Check digit: 3 (BS 797 Rule 3 (c)) */
        /*110*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "345670", 0, "03456703", "0034000005673" },
        /*111*/ { BARCODE_UPCE_CHK, -1, "3456703", 0, "03456703", "" }, /* equivalent: 03400000567, hrt: 03456703, Check digit: 3 (BS 797 Rule 3 (c)) */
        /*112*/ { BARCODE_UPCE, -1, "984753", 0, "09847531", "" }, /* equivalent: 09840000075, hrt: 09847531, Check digit: 1 (BS 797 Rule 3 (d)) */
        /*113*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "984753", 0, "09847531", "0098400000751" },
        /*114*/ { BARCODE_UPCE_CHK, -1, "9847531", 0, "09847531", "" }, /* equivalent: 09840000075, hrt: 09847531, Check digit: 1 (BS 797 Rule 3 (d)) */
        /*115*/ { BARCODE_UPCE, -1, "123453", 0, "01234531", "" },
        /*116*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "123453", 0, "01234531", "0012300000451" },
        /*117*/ { BARCODE_UPCE, -1, "000000", 0, "00000000", "" },
        /*118*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "000000", 0, "00000000", "0000000000000" },
        /*119*/ { BARCODE_UPCE, -1, "0000000", 0, "00000000", "" },
        /*120*/ { BARCODE_UPCE, -1, "1000000", 0, "10000007", "" },
        /*121*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "1000000", 0, "10000007", "0100000000007" },
        /*122*/ { BARCODE_UPCE, -1, "2000000", ZINT_WARN_INVALID_OPTION, "00000000", "" }, /* First char 2-9 ignored, replaced with 0 */
        /*123*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "2000000", ZINT_WARN_INVALID_OPTION, "00000000", "0000000000000" },
        /*124*/ { BARCODE_UPCE, -1, "3000000", ZINT_WARN_INVALID_OPTION, "00000000", "" },
        /*125*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "3000000", ZINT_WARN_INVALID_OPTION, "00000000", "0000000000000" },
        /*126*/ { BARCODE_UPCE, -1, "8000000", ZINT_WARN_INVALID_OPTION, "00000000", "" },
        /*127*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "8000000", ZINT_WARN_INVALID_OPTION, "00000000", "0000000000000" },
        /*128*/ { BARCODE_UPCE, -1, "9000000", ZINT_WARN_INVALID_OPTION, "00000000", "" },
        /*129*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "9000000", ZINT_WARN_INVALID_OPTION, "00000000", "0000000000000" },
        /*130*/ { BARCODE_UPCE, -1, "1234567+1", 0, "12345670+01", "" },
        /*131*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "1234567+1", 0, "12345670+01", "012345600007001" },
        /*132*/ { BARCODE_UPCE, -1, "12345670+1", 0, "12345670+01", "" },
        /*133*/ { BARCODE_UPCE, BARCODE_CONTENT_SEGS, "12345670+1", 0, "12345670+01", "012345600007001" },
        /*134*/ { BARCODE_UPCE_CHK, -1, "12345670+1", 0, "12345670+01", "" },
        /*135*/ { BARCODE_UPCE_CHK, BARCODE_CONTENT_SEGS, "12345670+1", 0, "12345670+01", "012345600007001" },
        /*136*/ { BARCODE_UPCE_CHK, -1, "1234565+1", 0, "01234565+01", "" },
        /*137*/ { BARCODE_UPCE_CHK, BARCODE_CONTENT_SEGS, "1234565+1", 0, "01234565+01", "001234500006501" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;
    int expected_length, expected_content_length;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, data[i].output_options,
                                    data[i].data, -1, debug);
        expected_length = (int) strlen(data[i].expected);
        expected_content_length = (int) strlen(data[i].expected_content);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        assert_equal(symbol->text_length, expected_length, "i:%d text_length %d != expected_length %d\n",
                    i, symbol->text_length, expected_length);
        assert_zero(strcmp((const char *) symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n",
                    i, symbol->text, data[i].expected);
        if (symbol->output_options & BARCODE_CONTENT_SEGS) {
            assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
            assert_nonnull(symbol->content_segs[0].source, "i:%d content_segs[0].source NULL\n", i);
            assert_equal(symbol->content_segs[0].length, expected_content_length,
                        "i:%d content_segs[0].length %d != expected_content_length %d\n",
                        i, symbol->content_segs[0].length, expected_content_length);
            assert_zero(memcmp(symbol->content_segs[0].source, data[i].expected_content, expected_content_length),
                        "i:%d memcmp(%.*s, %s, %d) != 0\n",
                        i, symbol->content_segs[0].length, symbol->content_segs[0].source, data[i].expected_content,
                        expected_content_length);
        } else {
            assert_null(symbol->content_segs, "i:%d content_segs not NULL\n", i);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

INTERNAL void zint_vector_free(struct zint_symbol *symbol); /* Free vector structures */

static void test_vector_same(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *data;
        int ret_encode;
        int ret_vector;
    };
    static const struct item data[] = {
        /* 0*/ { BARCODE_UPCE, "123456", 0, 0 },
        /* 1*/ { BARCODE_UPCE_CHK, "1234565", 0, 0 }, /* 5 is correct check digit */
        /* 2*/ { BARCODE_ISBNX, "0195049969", 0, 0 }, /* 9 is correct check digit */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        struct zint_vector *vectors[4];
        int vectors_size = ARRAY_SIZE(vectors);
        int j;

        if (testContinue(p_ctx, i)) continue;

        for (j = 0; j < vectors_size; j++) {
            symbol = ZBarcode_Create();
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                                        -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                        data[i].data, -1, debug);

            ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
            assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                        i, ret, data[i].ret_encode, symbol->errtxt);

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d (%s)\n",
                        i, ret, data[i].ret_vector, symbol->errtxt);

            assert_nonnull(symbol->vector, "i:%d symbol->vector NULL\n", i);
            vectors[j] = testUtilVectorCpy(symbol->vector);

            ZBarcode_Delete(symbol);
        }

        for (j = 1; j < vectors_size; j++) {
            ret = testUtilVectorCmp(vectors[j - 1], vectors[j]);
            assert_zero(ret, "i:%d testUtilVectorCmp ret %d != 0\n", i, ret);
        }

        for (j = 0; j < vectors_size; j++) {
            struct zint_symbol symbol_vector = {0}; /* Suppress clang -fsanitize=memory false positive */
            symbol_vector.vector = vectors[j];
            zint_vector_free(&symbol_vector);
        }
    }

    testFinish();
}

/* https://www.isbn-international.org/content/isbn-users-manual ISBN Users' Manual */
/* https://bisg.org/page/BarcodingGuidelines BISG Barcode Guidelines for the US Book Industry (BISG BGUSBI) */
static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_2;
        const char *data;
        int ret;

        int expected_rows;
        int expected_width;
        const char *comment;
        const char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_UPCA, -1, "01234567890", 0, 1, 95, "GGS Figure 5.1-1 UPC-A (also Figure 5.2.2.3-1., 5.2.6.6-2., 6.4.9-1. and BS EN 797:1996 Figure 3)",
                    "10100011010011001001001101111010100011011000101010101000010001001001000111010011100101001110101"
                },
        /*  1*/ { BARCODE_EAN13, -1, "4512345678906", 0, 1, 95, "GGS Figure 5.1-1 EAN-13",
                    "10101100010110011001001101111010011101011100101010101000010001001001000111010011100101010000101"
                },
        /*  2*/ { BARCODE_EANX, -1, "4512345678906", 0, 1, 95, "GGS Figure 5.1-1 EAN-13",
                    "10101100010110011001001101111010011101011100101010101000010001001001000111010011100101010000101"
                },
        /*  3*/ { BARCODE_EAN13, -1, "9501101531000", 0, 1, 95, "GGS Figure 5.2.2.1-2. EAN-13 (also Figure 6.5.2.2-1., 6.5.3.2-1., 6.5.3.2-2., 6.5.6.4-1.)",
                    "10101100010100111011001100110010100111001100101010100111010000101100110111001011100101110010101"
                },
        /*  4*/ { BARCODE_EANX_CHK, -1, "9501101531000", 0, 1, 95, "GGS Figure 5.2.2.1-2. EAN-13 (also Figure 6.5.2.2-1., 6.5.3.2-1., 6.5.3.2-2., 6.5.6.4-1.)",
                    "10101100010100111011001100110010100111001100101010100111010000101100110111001011100101110010101"
                },
        /*  5*/ { BARCODE_EAN8, -1, "9501234", 0, 1, 67, "GGS Figure 5.2.2.2-1. EAN-8",
                    "1010001011011000100011010011001010101101100100001010111001010000101"
                },
        /*  6*/ { BARCODE_EANX, -1, "9501234", 0, 1, 67, "GGS Figure 5.2.2.2-1. EAN-8",
                    "1010001011011000100011010011001010101101100100001010111001010000101"
                },
        /*  7*/ { BARCODE_UPCE, -1, "0012345", 0, 1, 51, "GGS Figure 5.2.2.4-1. UPC-E (also BS EN 797:1996 Figure 4)",
                    "101010011100110010011011011110100111010110001010101"
                },
        /*  8*/ { BARCODE_EAN13, -1, "9771384524017+12", 0, 1, 122, "GGS Figure 5.2.2.5.1-2. EAN-13 barcode with two-digit add-on",
                    "10101110110010001011001101111010001001010001101010100111011011001011100111001011001101000100101000000010110011001010010011"
                },
        /*  9*/ { BARCODE_EANX, -1, "9771384524017+12", 0, 1, 122, "GGS Figure 5.2.2.5.1-2. EAN-13 barcode with two-digit add-on",
                    "10101110110010001011001101111010001001010001101010100111011011001011100111001011001101000100101000000010110011001010010011"
                },
        /* 10*/ { BARCODE_EAN13, -1, "9780877799306+54321", 0, 1, 149, "GGS Figure 5.2.2.5.2-2. EAN-13 barcode with five-digit add-on",
                    "10101110110001001010011101101110010001011101101010100010011101001110100100001011100101010000101000000010110111001010100011010100001010010011010011001"
                },
        /* 11*/ { BARCODE_EANX, -1, "9780877799306+54321", 0, 1, 149, "GGS Figure 5.2.2.5.2-2. EAN-13 barcode with five-digit add-on",
                    "10101110110001001010011101101110010001011101101010100010011101001110100100001011100101010000101000000010110111001010100011010100001010010011010011001"
                },
        /* 12*/ { BARCODE_EAN13, -1, "501234567890", 0, 1, 95, "GGS Figure 5.2.6.6-1. EAN-13 (also BS EN 797:1996 Figure 1)",
                    "10100011010110011001101101111010100011011100101010101000010001001001000111010011100101110010101"
                },
        /* 13*/ { BARCODE_EANX, -1, "501234567890", 0, 1, 95, "GGS Figure 5.2.6.6-1. EAN-13 (also BS EN 797:1996 Figure 1)",
                    "10100011010110011001101101111010100011011100101010101000010001001001000111010011100101110010101"
                },
        /* 14*/ { BARCODE_EAN8, -1, "2012345", 0, 1, 67, "GGS Figure 5.2.6.6-3. EAN-8",
                    "1010010011000110100110010010011010101000010101110010011101100110101"
                },
        /* 15*/ { BARCODE_EANX, -1, "2012345", 0, 1, 67, "GGS Figure 5.2.6.6-3. EAN-8",
                    "1010010011000110100110010010011010101000010101110010011101100110101"
                },
        /* 16*/ { BARCODE_UPCE_CHK, -1, "00783491", 0, 1, 51, "GGS Figure 5.2.6.6-4. UPC-E",
                    "101010011100100010110111010000101000110001011010101"
                },
        /* 17*/ { BARCODE_UPCA, -1, "01234567890+24", 0, 1, 124, "GGS Figure 5.2.6.6-5. UPC-A barcode with two-digit add-on",
                    "1010001101001100100100110111101010001101100010101010100001000100100100011101001110010100111010100000000010110010011010100011"
                },
        /* 18*/ { BARCODE_EAN13, -1, "1234567891019+45678", 0, 1, 149, "GGS Figure 5.2.6.6-6. EAN-13 barcode with five-digit add-on",
                    "10100100110111101001110101100010000101001000101010100100011101001100110111001011001101110100101000000010110011101010110001010101111010010001010110111"
                },
        /* 19*/ { BARCODE_EANX, -1, "1234567891019+45678", 0, 1, 149, "GGS Figure 5.2.6.6-6. EAN-13 barcode with five-digit add-on",
                    "10100100110111101001110101100010000101001000101010100100011101001100110111001011001101110100101000000010110011101010110001010101111010010001010110111"
                },
        /* 20*/ { BARCODE_EAN13, -1, "5024425377399", 0, 1, 95, "GGS Figure 5.10.5.12-1. EAN-13",
                    "10100011010011011001110101000110010011011100101010100001010001001000100100001011101001110100101"
                },
        /* 21*/ { BARCODE_EANX, -1, "5024425377399", 0, 1, 95, "GGS Figure 5.10.5.12-1. EAN-13",
                    "10100011010011011001110101000110010011011100101010100001010001001000100100001011101001110100101"
                },
        /* 22*/ { BARCODE_UPCA_CHK, -1, "614141000302", 0, 1, 95, "GGS Figure 5.10.6.3-1. UPC-A",
                    "10101011110011001010001100110010100011001100101010111001011100101110010100001011100101101100101"
                },
        /* 23*/ { BARCODE_UPCA, -1, "61414123440", 0, 1, 95, "GGS Figure 6.4.17-2. UPC-A Daisy 5pc Set",
                    "10101011110011001010001100110010100011001100101010110110010000101011100101110011100101110010101"
                },
        /* 24*/ { BARCODE_EAN13, -1, "5410038302178", 0, 1, 95, "GGS Figure 6.5.5.3-1. EAN-13",
                    "10101000110110011010011100011010111101000100101010100001011100101101100110011010001001001000101"
                },
        /* 25*/ { BARCODE_EANX, -1, "5410038302178", 0, 1, 95, "GGS Figure 6.5.5.3-1. EAN-13",
                    "10101000110110011010011100011010111101000100101010100001011100101101100110011010001001001000101"
                },
        /* 26*/ { BARCODE_EAN13, -1, "501234567890+12", 0, 1, 122, "BS EN 797:1996 Figure 5 EAN-13 with 2-digit add-on",
                    "10100011010110011001101101111010100011011100101010101000010001001001000111010011100101110010101000000010110011001010010011"
                },
        /* 27*/ { BARCODE_EANX, -1, "501234567890+12", 0, 1, 122, "BS EN 797:1996 Figure 5 EAN-13 with 2-digit add-on",
                    "10100011010110011001101101111010100011011100101010101000010001001001000111010011100101110010101000000010110011001010010011"
                },
        /* 28*/ { BARCODE_EAN13, -1, "501234567890+86104", 0, 1, 149, "BS EN 797:1996 Figure 6 EAN-13 with 5-digit add-on",
                    "10100011010110011001101101111010100011011100101010101000010001001001000111010011100101110010101000000010110001001010101111010011001010001101010011101"
                },
        /* 29*/ { BARCODE_EANX, -1, "501234567890+86104", 0, 1, 149, "BS EN 797:1996 Figure 6 EAN-13 with 5-digit add-on",
                    "10100011010110011001101101111010100011011100101010101000010001001001000111010011100101110010101000000010110001001010101111010011001010001101010011101"
                },
        /* 30*/ { BARCODE_UPCA_CHK, -1, "614141234417+12345", 0, 1, 151, "Example of UPC-A with 5-digit add-on",
                    "1010101111001100101000110011001010001100110010101011011001000010101110010111001100110100010010100000000010110110011010010011010100001010100011010110001"
                },
        /* 31*/ { BARCODE_UPCE, -1, "1234567+12", 0, 1, 78, "Example of UPC-E with 2-digit add-on",
                    "101001001101111010100011011100100001010010001010101000000010110011001010010011"
                },
        /* 32*/ { BARCODE_UPCE_CHK, -1, "12345670+12345", 0, 1, 105, "Example of UPC-E with 5-digit add-on",
                    "101001001101111010100011011100100001010010001010101000000010110110011010010011010100001010100011010110001"
                },
        /* 33*/ { BARCODE_EAN8, -1, "2012345+12", ZINT_WARN_NONCOMPLIANT, 1, 94, "Example of EAN-8 with 2-digit add-on",
                    "1010010011000110100110010010011010101000010101110010011101100110101000000010110011001010010011"
                },
        /* 34*/ { BARCODE_EANX, -1, "2012345+12", ZINT_WARN_NONCOMPLIANT, 1, 94, "Example of EAN-8 with 2-digit add-on",
                    "1010010011000110100110010010011010101000010101110010011101100110101000000010110011001010010011"
                },
        /* 35*/ { BARCODE_EAN8, -1, "1234567+12", ZINT_WARN_NONCOMPLIANT, 1, 94, "Example of EAN-8 with 2-digit add-on",
                    "1010011001001001101111010100011010101001110101000010001001110010101000000010110011001010010011"
                },
        /* 36*/ { BARCODE_EANX, -1, "1234567+12", ZINT_WARN_NONCOMPLIANT, 1, 94, "Example of EAN-8 with 2-digit add-on",
                    "1010011001001001101111010100011010101001110101000010001001110010101000000010110011001010010011"
                },
        /* 37*/ { BARCODE_EAN8, -1, "12345670+12345", ZINT_WARN_NONCOMPLIANT, 1, 121, "Example of EAN-8 with 5-digit add-on",
                    "1010011001001001101111010100011010101001110101000010001001110010101000000010110110011010010011010100001010100011010110001"
                },
        /* 38*/ { BARCODE_EANX_CHK, -1, "12345670+12345", ZINT_WARN_NONCOMPLIANT, 1, 121, "Example of EAN-8 with 5-digit add-on",
                    "1010011001001001101111010100011010101001110101000010001001110010101000000010110110011010010011010100001010100011010110001"
                },
        /* 39*/ { BARCODE_EAN_5ADDON, -1, "54321", 0, 1, 47, "Example of EAN-5",
                    "10110111001010100011010100001010010011010011001"
                },
        /* 40*/ { BARCODE_EANX, -1, "54321", 0, 1, 47, "Example of EAN-5",
                    "10110111001010100011010100001010010011010011001"
                },
        /* 41*/ { BARCODE_EAN_2ADDON, -1, "21", 0, 1, 20, "Example of EAN-2",
                    "10110010011010110011"
                },
        /* 42*/ { BARCODE_EANX, -1, "21", 0, 1, 20, "Example of EAN-2",
                    "10110010011010110011"
                },
        /* 43*/ { BARCODE_EAN8, 9, "76543210+21", ZINT_WARN_NONCOMPLIANT, 1, 96, "Example of EAN-8 barcode with two-digit add-on, addon_gap 9",
                    "101011101101011110110001010001101010100001011011001100110111001010100000000010110010011010110011"
                },
        /* 44*/ { BARCODE_EANX_CHK, 9, "76543210+21", ZINT_WARN_NONCOMPLIANT, 1, 96, "Example of EAN-8 barcode with two-digit add-on, addon_gap 9",
                    "101011101101011110110001010001101010100001011011001100110111001010100000000010110010011010110011"
                },
        /* 45*/ { BARCODE_EAN8, 12, "7654321+54321", ZINT_WARN_NONCOMPLIANT, 1, 126, "Example of EAN-8 barcode with five-digit add-on, addon_gap 12",
                    "101011101101011110110001010001101010100001011011001100110111001010100000000000010110111001010100011010100001010010011010011001"
                },
        /* 46*/ { BARCODE_EANX, 12, "7654321+54321", ZINT_WARN_NONCOMPLIANT, 1, 126, "Example of EAN-8 barcode with five-digit add-on, addon_gap 12",
                    "101011101101011110110001010001101010100001011011001100110111001010100000000000010110111001010100011010100001010010011010011001"
                },
        /* 47*/ { BARCODE_UPCA_CHK, 10, "210987654329+21", 0, 1, 125, "Example of UPC-A with 2-digit add-on, addon_gap 10",
                    "10100100110011001000110100010110110111011101101010101000010011101011100100001011011001110100101000000000010110010011010110011"
                },
        /* 48*/ { BARCODE_UPCA, 12, "21098765432+12121", 0, 1, 154, "Example of UPC-A with 5-digit add-on, addon_gap 12",
                    "1010010011001100100011010001011011011101110110101010100001001110101110010000101101100111010010100000000000010110011001010010011010110011010011011010011001"
                },
        /* 49*/ { BARCODE_UPCE_CHK, 8, "06543217+21", 0, 1, 79, "Example of UPC-E with 2-digit add-on, addon_gap 8",
                    "1010000101011000100111010111101001101100110010101010000000010110010011010110011"
                },
        /* 50*/ { BARCODE_UPCE, 11, "1654321+12121", 0, 1, 109, "Example of UPC-E with 5-digit add-on, addon_gap 11",
                    "1010101111011100101000110111101001101101100110101010000000000010110011001010010011010110011010011011010011001"
                },
        /* 51*/ { BARCODE_ISBNX, -1, "9789295055124", 0, 1, 95, "ISBN Users' Manual 7th Ed. 13.2",
                    "10101110110001001001011100100110010111011000101010111001010011101001110110011011011001011100101"
                },
        /* 52*/ { BARCODE_ISBNX, -1, "9780123456786+50995", 0, 1, 149, "BISG BGUSBI Figure 1: Bookland EAN (with Price Add-On) (and Figures 4, 5, 6)",
                    "10101110110001001010011100110010011011011110101010101110010011101010000100010010010001010000101000000010110110001010100111010001011010001011010111001"
                },
        /* 53*/ { BARCODE_ISBNX, -1, "9781402894626+50495", 0, 1, 149, "BISG BGUSBI Figure 2",
                    "10101110110001001011001101000110100111001001101010100100011101001011100101000011011001010000101000000010110111001010001101010100011010001011010111001"
                },
        /* 54*/ { BARCODE_ISBNX, 10, "9780940016644+50750", 0, 1, 152, "BISG BGUSBI Figure 3, addon_gap 10",
                    "10101110110001001010011100010110011101000110101010111001011001101010000101000010111001011100101000000000010110111001010001101010010001010110001010001101"
                },
        /* 55*/ { BARCODE_ISBNX, -1, "9791234567896+12", 0, 1, 122, "Example of ISBN with 2-digit add-on",
                    "10101110110010111011001100100110100001010001101010100111010100001000100100100011101001010000101000000010110011001010010011"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[4096] = {0}; /* Suppress clang -fsanitize=memory false positive */
    char cmp_msg[1024];

    /* Only do BWIPP/ZXing-C++ tests if asked, too slow otherwise */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript();
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder();

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, data[i].option_2, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                    i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %s, %d, %d, \"%s\",\n",
                                i, testUtilBarcodeName(data[i].symbology), data[i].option_2, data[i].data,
                                testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n",
                            i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n",
                            i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n",
                            i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, -1, debug)) {
                    ret = testUtilBwipp(i, symbol, -1, data[i].option_2, -1, data[i].data, length, NULL, cmp_buf,
                                sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n",
                                i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf,
                                   data[i].expected);
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[8192 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                                "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, 1 /*zxingcpp_cmp*/, cmp_buf,
                                sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n",
                                i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length,
                                NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len,
                                escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* #181 Christian Hartlage OSS-Fuzz */
static void test_fuzz(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *data;
        int length;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_EAN8, "55++15", -1, ZINT_ERROR_INVALID_DATA },
        /*  1*/ { BARCODE_EAN13, "55++15", -1, ZINT_ERROR_INVALID_DATA },
        /*  2*/ { BARCODE_EANX, "55++15", -1, ZINT_ERROR_INVALID_DATA },
        /*  3*/ { BARCODE_EAN8, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /*  4*/ { BARCODE_EAN13, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /*  5*/ { BARCODE_EANX, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /*  6*/ { BARCODE_EANX_CHK, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /*  7*/ { BARCODE_UPCA, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /*  8*/ { BARCODE_UPCA_CHK, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /*  9*/ { BARCODE_UPCE, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 10*/ { BARCODE_UPCE_CHK, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 11*/ { BARCODE_ISBNX, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 12*/ { BARCODE_EAN8, "+12345", -1, ZINT_WARN_NONCOMPLIANT },
        /* 13*/ { BARCODE_EAN13, "+12345", -1, 0 },
        /* 14*/ { BARCODE_EANX, "+12345", -1, ZINT_WARN_NONCOMPLIANT },
        /* 15*/ { BARCODE_EAN8, "+123456", -1, ZINT_ERROR_TOO_LONG },
        /* 16*/ { BARCODE_EAN13, "+123456", -1, ZINT_ERROR_TOO_LONG },
        /* 17*/ { BARCODE_EANX, "+123456", -1, ZINT_ERROR_TOO_LONG },
        /* 18*/ { BARCODE_EAN13, "000002000000200+203", -1, ZINT_ERROR_TOO_LONG }, /* #218 Jan Schrewe CI-Fuzz */
        /* 19*/ { BARCODE_EANX, "000002000000200+203", -1, ZINT_ERROR_TOO_LONG }, /* #218 Jan Schrewe CI-Fuzz */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_upce_input", test_upce_input },
        { "test_upca_print", test_upca_print },
        { "test_upca_input", test_upca_input },
        { "test_ean13_input", test_ean13_input },
        { "test_ean8_input", test_ean8_input },
        { "test_ean_2addon_input", test_ean_2addon_input },
        { "test_ean_5addon_input", test_ean_5addon_input },
        { "test_eanx_input", test_eanx_input },
        { "test_isbn_input", test_isbn_input },
        { "test_hrt", test_hrt },
        { "test_vector_same", test_vector_same },
        { "test_encode", test_encode },
        { "test_fuzz", test_fuzz },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
