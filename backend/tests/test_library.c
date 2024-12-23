/*
    libzint - the open source barcode library
    Copyright (C) 2019-2024 Robin Stuart <rstuart114@gmail.com>

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

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include "testcommon.h"

static void test_checks(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_1;
        char *data;
        int length;
        int input_mode;
        int eci;
        float height;
        int whitespace_width;
        int whitespace_height;
        int border_width;
        float scale;
        float dot_size;
        float text_gap;
        float guard_descent;
        int warn_level;
        int ret;

        char *expected;
        int expected_symbology;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, "1234", -1, -1, 3, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 217: Symbology does not support ECI switching", -1 },
        /*  1*/ { BARCODE_CODE128, -1, "1234", -1, -1, 0, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", -1 },
        /*  2*/ { BARCODE_QRCODE, -1, "1234", -1, -1, 3, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", -1 },
        /*  3*/ { BARCODE_QRCODE, -1, "1234", -1, -1, 999999 + 1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 218: ECI code '1000000' out of range (0 to 999999, excluding 1, 2, 14 and 19)", -1 },
        /*  4*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, 0, 0.009, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 227: Scale out of range (0.01 to 200)", -1 },
        /*  5*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, 0, 200.01, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 227: Scale out of range (0.01 to 200)", -1 },
        /*  6*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, 0, -1, 20.1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 221: Dot size out of range (0.01 to 20)", -1 },
        /*  7*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, 0, 0.01, 0.009, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 221: Dot size out of range (0.01 to 20)", -1 },
        /*  8*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, -0.1, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 765: Height out of range (0 to 2000)", -1 },
        /*  9*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 2000.01, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 765: Height out of range (0 to 2000)", -1 },
        /* 10*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, -1, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 766: Whitespace width out of range (0 to 100)", -1 },
        /* 11*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 101, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 766: Whitespace width out of range (0 to 100)", -1 },
        /* 12*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, -1, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 767: Whitespace height out of range (0 to 100)", -1 },
        /* 13*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 101, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 767: Whitespace height out of range (0 to 100)", -1 },
        /* 14*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, -1, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 768: Border width out of range (0 to 100)", -1 },
        /* 15*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, 101, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 768: Border width out of range (0 to 100)", -1 },
        /* 16*/ { BARCODE_CODE128, -1, "1234", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 220: Selected symbology does not support GS1 mode", -1 },
        /* 17*/ { BARCODE_EANX, -1, "123456789012", -1, -1, -1, 0, 0, 0, 101, -1, -1, -5.1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 219: Text gap out of range (-5 to 10)", -1 },
        /* 18*/ { BARCODE_EANX, -1, "123456789012", -1, -1, -1, 0, 0, 0, 101, -1, -1, 10.1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 219: Text gap out of range (-5 to 10)", -1 },
        /* 19*/ { BARCODE_EANX, -1, "123456789012", -1, -1, -1, 0, 0, 0, 101, -1, -1, 0, -0.5, -1, ZINT_ERROR_INVALID_OPTION, "Error 769: Guard bar descent out of range (0 to 50)", -1 },
        /* 20*/ { BARCODE_EANX, -1, "123456789012", -1, -1, -1, 0, 0, 0, 101, -1, -1, 0, 50.1, -1, ZINT_ERROR_INVALID_OPTION, "Error 769: Guard bar descent out of range (0 to 50)", -1 },
        /* 21*/ { BARCODE_GS1_128, -1, "[21]12\0004", 8, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 262: NUL characters not permitted in GS1 mode", -1 },
        /* 22*/ { BARCODE_GS1_128, -1, "[21]12é4", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1", -1 },
        /* 23*/ { BARCODE_GS1_128, -1, "[21]12\0074", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 251: Control characters are not supported by GS1", -1 },
        /* 24*/ { BARCODE_GS1_128, -1, "[21]1234", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", -1 },
        /* 25*/ { 0, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /* 26*/ { 0, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /* 27*/ { 0, -1, "1", -1, -1, 1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 217: Symbology does not support ECI switching", BARCODE_CODE128 }, /* Not supporting beats invalid ECI */
        /* 28*/ { 0, -1, "1", -1, -1, 1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /* 29*/ { 0, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, 0.009, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 221: Dot size out of range (0.01 to 20)", BARCODE_CODE128 },
        /* 30*/ { 0, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, 0.009, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /* 31*/ { 0, -1, "1", -1, -1, 1, 0, 0, 0, 0, -1, 0.009, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 217: Symbology does not support ECI switching", BARCODE_CODE128 }, /* Invalid dot size no longer beats invalid ECI */
        /* 32*/ { 0, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, 0.009, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /* 33*/ { 5, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_C25STANDARD },
        /* 34*/ { 5, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_C25STANDARD },
        /* 35*/ { 10, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_EANX },
        /* 36*/ { 10, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_EANX },
        /* 37*/ { 11, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_EANX },
        /* 38*/ { 11, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_EANX },
        /* 39*/ { 12, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_EANX },
        /* 40*/ { 12, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_EANX },
        /* 41*/ { 15, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_EANX },
        /* 42*/ { 15, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_EANX },
        /* 43*/ { 17, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_UPCA },
        /* 44*/ { 17, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_UPCA },
        /* 45*/ { 19, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_TOO_LONG, "Error 362: Input length 1 too short (minimum 3)", BARCODE_CODABAR },
        /* 46*/ { 19, -1, "A1B", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 207: Codabar 18 not supported", BARCODE_CODABAR },
        /* 47*/ { 19, -1, "A1B", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 207: Codabar 18 not supported", -1 },
        /* 48*/ { 26, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_UPCA },
        /* 49*/ { 26, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_UPCA },
        /* 50*/ { 27, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 208: UPCD1 not supported", 27 },
        /* 51*/ { 33, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI", BARCODE_GS1_128 },
        /* 52*/ { 33, -1, "[10]23", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_GS1_128 },
        /* 53*/ { 33, -1, "[10]23", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_GS1_128 },
        /* 54*/ { 36, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_UPCA },
        /* 55*/ { 36, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_UPCA },
        /* 56*/ { 39, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_UPCE },
        /* 57*/ { 39, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_UPCE },
        /* 58*/ { 41, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_NONCOMPLIANT, "Warning 479: Input length 1 is not standard (5, 9 or 11)", BARCODE_POSTNET },
        /* 59*/ { 41, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_POSTNET },
        /* 60*/ { 41, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_POSTNET },
        /* 61*/ { 42, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_POSTNET },
        /* 62*/ { 42, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_POSTNET },
        /* 63*/ { 43, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_POSTNET },
        /* 64*/ { 43, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_POSTNET },
        /* 65*/ { 44, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_POSTNET },
        /* 66*/ { 44, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_POSTNET },
        /* 67*/ { 45, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_POSTNET },
        /* 68*/ { 45, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_POSTNET },
        /* 69*/ { 46, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_PLESSEY },
        /* 70*/ { 46, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_PLESSEY },
        /* 71*/ { 48, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_NVE18 },
        /* 72*/ { 48, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_NVE18 },
        /* 73*/ { 59, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_CODE128 },
        /* 74*/ { 59, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_CODE128 },
        /* 75*/ { 61, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_CODE128 },
        /* 76*/ { 61, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_CODE128 },
        /* 77*/ { 62, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_CODE93 },
        /* 78*/ { 62, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_CODE93 },
        /* 79*/ { 64, -1, "12345678", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_AUSPOST },
        /* 80*/ { 64, -1, "12345678", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_AUSPOST },
        /* 81*/ { 65, -1, "12345678", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_AUSPOST },
        /* 82*/ { 65, -1, "12345678", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_AUSPOST },
        /* 83*/ { 78, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_DBAR_OMN },
        /* 84*/ { 78, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_DBAR_OMN },
        /* 85*/ { 83, -1, "12345678901", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_PLANET },
        /* 86*/ { 83, -1, "12345678901", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_PLANET },
        /* 87*/ { 88, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI", BARCODE_GS1_128 },
        /* 88*/ { 88, -1, "[10]12", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_GS1_128 },
        /* 89*/ { 88, -1, "[10]12", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_GS1_128 },
        /* 90*/ { 91, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /* 91*/ { 91, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /* 92*/ { 94, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /* 93*/ { 94, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /* 94*/ { 95, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /* 95*/ { 95, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /* 96*/ { 100, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_HIBC_128 },
        /* 97*/ { 100, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_128 },
        /* 98*/ { 101, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_HIBC_39 },
        /* 99*/ { 101, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_39 },
        /*100*/ { 103, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_HIBC_DM },
        /*101*/ { 103, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_DM },
        /*102*/ { 105, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_HIBC_QR },
        /*103*/ { 105, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_QR },
        /*104*/ { 107, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_HIBC_PDF },
        /*105*/ { 107, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_PDF },
        /*106*/ { 109, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_HIBC_MICPDF },
        /*107*/ { 109, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_MICPDF },
        /*108*/ { 111, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", BARCODE_HIBC_BLOCKF },
        /*109*/ { 111, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_BLOCKF },
        /*110*/ { 113, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*111*/ { 113, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*112*/ { 114, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*113*/ { 114, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*114*/ { 117, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*115*/ { 117, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*116*/ { 118, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*117*/ { 118, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*118*/ { 122, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*119*/ { 122, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*120*/ { 123, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*121*/ { 123, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*122*/ { 124, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*123*/ { 124, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*124*/ { 125, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*125*/ { 125, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*126*/ { 126, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*127*/ { 126, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*128*/ { 127, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*129*/ { 127, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*130*/ { 148, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*131*/ { 148, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*132*/ { BARCODE_LAST + 1, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /*133*/ { BARCODE_LAST + 1, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /*134*/ { BARCODE_CODE128, -1, "\200", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 245: Invalid UTF-8 in input", -1 },
        /*135*/ { BARCODE_CODE128, -1, "\\o200", -1, UNICODE_MODE | ESCAPE_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 245: Invalid UTF-8 in input", -1 },
        /*136*/ { BARCODE_MAXICODE, -1, "\200", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 245: Invalid UTF-8 in input", -1 },
        /*137*/ { BARCODE_MAXICODE, -1, "\\o200", -1, UNICODE_MODE | ESCAPE_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 245: Invalid UTF-8 in input", -1 },
        /*138*/ { BARCODE_GS1_128, -1, "[01]12345678901234", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (01) position 14: Bad checksum '4', expected '1'", -1 },
        /*139*/ { BARCODE_GS1_128, -1, "[01]12345678901234", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_NONCOMPLIANT, "Error 261: AI (01) position 14: Bad checksum '4', expected '1'", -1 },
        /*140*/ { BARCODE_QRCODE, -1, "ก", -1, UNICODE_MODE, 13, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", -1 },
        /*141*/ { BARCODE_QRCODE, -1, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_USES_ECI, "Warning 222: Encoded data includes ECI 13", -1 },
        /*142*/ { BARCODE_QRCODE, -1, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_USES_ECI, "Error 222: Encoded data includes ECI 13", -1 },
        /*143*/ { BARCODE_CODEONE, -1, "[01]12345678901231", -1, GS1_MODE, 3, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 512: ECI ignored for GS1 mode", -1 },
        /*144*/ { BARCODE_CODEONE, -1, "[01]12345678901231", -1, GS1_MODE, 3, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 512: ECI ignored for GS1 mode", -1 },
        /*145*/ { BARCODE_CODEONE, -1, "[01]12345678901234", -1, GS1_MODE, 3, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 512: ECI ignored for GS1 mode", -1 }, /* Warning in encoder overrides library warnings */
        /*146*/ { BARCODE_CODEONE, -1, "[01]12345678901234", -1, GS1_MODE, 3, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_NONCOMPLIANT, "Error 261: AI (01) position 14: Bad checksum '4', expected '1'", -1 }, /* But not errors */
        /*147*/ { BARCODE_AZTEC, -1, "ก", -1, UNICODE_MODE, 13, 0, 0, 0, 0, -1, -1, 0, -1, -1, 0, "", -1 },
        /*148*/ { BARCODE_AZTEC, -1, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_USES_ECI, "Warning 222: Encoded data includes ECI 13", -1 },
        /*149*/ { BARCODE_AZTEC, -1, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_USES_ECI, "Error 222: Encoded data includes ECI 13", -1 },
        /*150*/ { BARCODE_AZTEC, 6, "ก", -1, UNICODE_MODE, 13, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 503: Error correction level '6' out of range (1 to 4), ignoring", -1 },
        /*151*/ { BARCODE_AZTEC, 6, "ก", -1, UNICODE_MODE, 13, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 503: Error correction level '6' out of range (1 to 4)", -1 },
        /*152*/ { BARCODE_AZTEC, 6, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, -1, ZINT_WARN_USES_ECI, "Warning 222: Encoded data includes ECI 13", -1 }, /* ECI warning trumps all other warnings */
        /*153*/ { BARCODE_AZTEC, 6, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, 0, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 503: Error correction level '6' out of range (1 to 4)", -1 }, /* But not errors */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_checks", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci, data[i].option_1, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);
        if (data[i].height) {
            symbol->height = data[i].height;
        }
        if (data[i].whitespace_width) {
            symbol->whitespace_width = data[i].whitespace_width;
        }
        if (data[i].whitespace_height) {
            symbol->whitespace_height = data[i].whitespace_height;
        }
        if (data[i].border_width) {
            symbol->border_width = data[i].border_width;
        }
        if (data[i].scale != -1) {
            symbol->scale = data[i].scale;
        }
        if (data[i].dot_size != -1) {
            symbol->dot_size = data[i].dot_size;
        }
        if (data[i].text_gap) {
            symbol->text_gap = data[i].text_gap;
        }
        if (data[i].guard_descent != -1) {
            symbol->guard_descent = data[i].guard_descent;
        }
        if (data[i].warn_level != -1) {
            symbol->warn_level = data[i].warn_level;
        }

        ret = ZBarcode_Encode(symbol, TU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret, symbol->errtxt);

        ret = strcmp(symbol->errtxt, data[i].expected);
        assert_zero(ret, "i:%d (%d) strcmp(%s, %s) %d != 0\n", i, data[i].symbology, symbol->errtxt, data[i].expected, ret);

        if (data[i].expected_symbology == -1) {
            assert_equal(symbol->symbology, data[i].symbology, "i:%d symbol->symbology %d != original %d\n", i, symbol->symbology, data[i].symbology);
        } else {
            assert_equal(symbol->symbology, data[i].expected_symbology, "i:%d symbol->symbology %d != expected %d\n", i, symbol->symbology, data[i].expected_symbology);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_checks_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_1;
        struct zint_seg segs[2];
        int seg_count;
        int input_mode;
        int eci;
        int warn_level;
        int ret;

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, { { NULL, 0, 0 }, { NULL, 0, 0 } }, 0, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 205: No input data" },
        /*  1*/ { BARCODE_CODE128, -1, { { NULL, 0, 0 }, { NULL, 0, 0 } }, 257, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 771: Too many input segments (maximum 256)" },
        /*  2*/ { BARCODE_CODE128, -1, { { NULL, 0, 0 }, { NULL, 0, 0 } }, 1, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 772: Input segment 0 source NULL" },
        /*  3*/ { BARCODE_CODE128, -1, { { TU(""), 0, 0 }, { NULL, 0, 0 } }, 1, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 778: No input data" },
        /*  4*/ { BARCODE_CODE128, -1, { { TU("A"), 0, 0 }, { NULL, 0, 0 } }, 2, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 772: Input segment 1 source NULL" },
        /*  5*/ { BARCODE_CODE128, -1, { { TU("A"), 0, 0 }, { TU(""), 0, 0 } }, 2, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 773: Input segment 1 empty" },
        /*  6*/ { BARCODE_CODE128, -1, { { TU("A"), 0, 3 }, { TU("B"), 0, 0 } }, 2, -1, 4, -1, ZINT_ERROR_INVALID_OPTION, "Error 774: Symbol ECI '4' must match segment zero ECI '3'" },
        /*  7*/ { BARCODE_CODE128, -1, { { TU("A"), 0, 3 }, { TU("B"), 0, 4 } }, 2, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 775: Symbology does not support multiple segments" },
        /*  8*/ { BARCODE_CODE128, -1, { { TU("A"), 0, 3 }, { NULL, 0, 0 } }, 1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 217: Symbology does not support ECI switching" },
        /*  9*/ { BARCODE_AZTEC, -1, { { TU("A"), 0, 3 }, { TU("B"), 0, 1 } }, 2, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 218: ECI code '1' out of range (0 to 999999, excluding 1, 2, 14 and 19)" },
        /* 10*/ { BARCODE_AZTEC, -1, { { TU("A"), 0, 3 }, { TU("B"), 0, 4 } }, 2, GS1_MODE, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 776: GS1 mode not supported for multiple segments" },
        /* 11*/ { BARCODE_AZTEC, -1, { { TU("A"), 0, 3 }, { TU("\200"), 0, 4 } }, 2, UNICODE_MODE, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 245: Invalid UTF-8 in input" },
        /* 12*/ { BARCODE_AZTEC, -1, { { TU("A"), 0, 3 }, { TU("B"), 0, 4 } }, 2, -1, -1, -1, 0, "" },
        /* 13*/ { BARCODE_AZTEC, -1, { { TU("A"), 0, 0 }, { TU("B"), 0, 4 } }, 2, -1, 3, -1, 0, "" },
        /* 14*/ { BARCODE_AZTEC, -1, { { TU("A"), ZINT_MAX_DATA_LEN, 3 }, { TU("B"), 1, 4 } }, 2, -1, -1, -1, ZINT_ERROR_TOO_LONG, "Error 243: Input too long" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_checks_segs", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci, data[i].option_1, -1, -1, -1 /*output_options*/, NULL, 0, debug);
        if (data[i].warn_level != -1) {
            symbol->warn_level = data[i].warn_level;
        }

        ret = ZBarcode_Encode_Segs(symbol, data[i].segs, data[i].seg_count);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_Segs(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret, symbol->errtxt);

        ret = strcmp(symbol->errtxt, data[i].expected);
        assert_zero(ret, "i:%d (%d) strcmp(%s, %s) %d != 0\n", i, data[i].symbology, symbol->errtxt, data[i].expected, ret);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input_data(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        char *data;
        int length;
        char *composite;
        int ret;

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, "1234", -1, "", 0, "" },
        /*  1*/ { BARCODE_GS1_128, -1, "[01]12345678901231", -1, "", 0, "" },
        /*  2*/ { BARCODE_GS1_128, -1, "", -1, "", ZINT_ERROR_INVALID_DATA, "Error 778: No input data" },
        /*  3*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", -1, "[10]121212", 0, "" },
        /*  4*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", -1, "", ZINT_ERROR_INVALID_DATA, "Error 779: No composite data (2D component)" },
        /*  5*/ { BARCODE_GS1_128_CC, -1, "", -1, "[10]121212", ZINT_ERROR_INVALID_OPTION, "Error 445: No primary (linear) message" },
        /*  6*/ { BARCODE_DATAMATRIX, -1, "", -1, "", ZINT_ERROR_INVALID_DATA, "Error 228: No input data (segment 0 empty)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char *text;

    testStartSymbol("test_input_data", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (is_composite(data[i].symbology)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, text, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret, symbol->errtxt);

        ret = strcmp(symbol->errtxt, data[i].expected);
        assert_zero(ret, "i:%d (%d) strcmp(%s, %s) %d != 0\n", i, data[i].symbology, symbol->errtxt, data[i].expected, ret);

        ZBarcode_Delete(symbol);
    }

    if (p_ctx->index == -1 && p_ctx->exclude[0] == -1) {
        char data_buf[ZINT_MAX_DATA_LEN + 10];
        int expected_ret = ZINT_ERROR_TOO_LONG;
        char *expected_errtxt[] = { "Error 797: Input too long", "Error 340: Input length 17399 too long (maximum 256)" };

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        /* Caught by `ZBarcode_Encode_Segs()` de-escape loop */
        symbol->input_mode = ESCAPE_MODE;
        testUtilStrCpyRepeat(data_buf, "A", ZINT_MAX_DATA_LEN);
        data_buf[ZINT_MAX_DATA_LEN] = '\\';
        data_buf[ZINT_MAX_DATA_LEN + 1] = 'n';

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, ZINT_MAX_DATA_LEN + 2);
        assert_equal(ret, expected_ret, "ZBarcode_Encode(%d) ret %d != %d (%s)\n", symbol->symbology, ret, expected_ret, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, expected_errtxt[0]), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, expected_errtxt[0]);

        /* Passes de-escape loop, caught by `code128()` */
        symbol->errtxt[0] = '\0';
        data_buf[ZINT_MAX_DATA_LEN - 2] = '\\';
        data_buf[ZINT_MAX_DATA_LEN - 1] = 'n';

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, ZINT_MAX_DATA_LEN);
        assert_equal(ret, expected_ret, "ZBarcode_Encode(%d) ret %d != %d (%s)\n", symbol->symbology, ret, expected_ret, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, expected_errtxt[1]), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, expected_errtxt[1]);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static int test_prev_map_invalid_symbology(int *p_symbology) {
    int symbology = *p_symbology;
    int warn_number = 0;

    if (symbology < 1) {
        warn_number = ZINT_WARN_INVALID_OPTION;
        symbology = BARCODE_CODE128;
    /* symbol->symbologys 1 to 126 are defined by tbarcode */
    } else if (symbology == 5) {
        symbology = BARCODE_C25STANDARD;
    } else if ((symbology >= 10) && (symbology <= 12)) {
        symbology = BARCODE_EANX;
    } else if (symbology == 15) {
        symbology = BARCODE_EANX;
    } else if (symbology == 17) {
        symbology = BARCODE_UPCA;
    } else if (symbology == 19) {
        warn_number = ZINT_WARN_INVALID_OPTION;
        symbology = BARCODE_CODABAR;
    } else if (symbology == 26) { /* UPC-A up to tbarcode 9, ISSN for tbarcode 10+ */
        symbology = BARCODE_UPCA;
    } else if (symbology == 27) { /* UPCD1 up to tbarcode 9, ISSN + 2 digit add-on for tbarcode 10+ */
        return ZINT_ERROR_INVALID_OPTION;
    } else if (symbology == 33) {
        symbology = BARCODE_GS1_128;
    } else if (symbology == 36) {
        symbology = BARCODE_UPCA;
    } else if (symbology == 39) {
        symbology = BARCODE_UPCE;
    } else if ((symbology >= 41) && (symbology <= 45)) {
        symbology = BARCODE_POSTNET;
    } else if (symbology == 46) {
        symbology = BARCODE_PLESSEY;
    } else if (symbology == 48) {
        symbology = BARCODE_NVE18;
    } else if ((symbology == 59) || (symbology == 61)) {
        symbology = BARCODE_CODE128;
    } else if (symbology == 62) {
        symbology = BARCODE_CODE93;
    } else if ((symbology == 64) || (symbology == 65)) {
        symbology = BARCODE_AUSPOST;
    } else if (symbology == 78) {
        symbology = BARCODE_DBAR_OMN;
    } else if (symbology == 83) {
        symbology = BARCODE_PLANET;
    } else if (symbology == 88) {
        symbology = BARCODE_GS1_128;
    } else if (symbology == 91) { /* BC412 up to tbarcode 9, Code 32 for tbarcode 10+ */
        warn_number = ZINT_WARN_INVALID_OPTION;
        symbology = BARCODE_CODE128;
    } else if ((symbology >= 94) && (symbology <= 95)) {
        warn_number = ZINT_WARN_INVALID_OPTION;
        symbology = BARCODE_CODE128;
    } else if (symbology == 100) {
        symbology = BARCODE_HIBC_128;
    } else if (symbology == 101) {
        symbology = BARCODE_HIBC_39;
    } else if (symbology == 103) {
        symbology = BARCODE_HIBC_DM;
    } else if (symbology == 105) {
        symbology = BARCODE_HIBC_QR;
    } else if (symbology == 107) {
        symbology = BARCODE_HIBC_PDF;
    } else if (symbology == 109) {
        symbology = BARCODE_HIBC_MICPDF;
    } else if (symbology == 111) {
        symbology = BARCODE_HIBC_BLOCKF;
    } else if ((symbology == 113) || (symbology == 114)) {
        warn_number = ZINT_WARN_INVALID_OPTION;
        symbology = BARCODE_CODE128;
    } else if ((symbology >= 117) && (symbology <= 127)) {
        if (symbology < 119 || symbology > 121) { /* BARCODE_MAILMARK_2D/4S/UPU_S10 */
            warn_number = ZINT_WARN_INVALID_OPTION;
            symbology = BARCODE_CODE128;
        }
    /* Everything from 128 up is Zint-specific */
    } else if (symbology > BARCODE_LAST) {
        warn_number = ZINT_WARN_INVALID_OPTION;
        symbology = BARCODE_CODE128;
    }
    if (symbology == *p_symbology) { /* Should never happen */
        return ZINT_ERROR_ENCODING_PROBLEM;
    }

    *p_symbology = symbology;

    return warn_number;
}

static void test_symbologies(const testCtx *const p_ctx) {
    int i, ret;
    struct zint_symbol s_symbol = {0};
    struct zint_symbol *symbol = &s_symbol;

    testStart("test_symbologies");

    for (i = -1; i < 148; i++) {
        if (testContinue(p_ctx, i)) continue;

        symbol->symbology = i;
        ret = ZBarcode_Encode(symbol, TU(""), 0);
        assert_notequal(ret, ZINT_ERROR_ENCODING_PROBLEM, "i:%d Encoding problem (%s)\n", i, symbol->errtxt);

        if (!ZBarcode_ValidID(i)) {
            int prev_symbology = i;
            int prev_ret = test_prev_map_invalid_symbology(&prev_symbology);
            if (ret != ZINT_ERROR_INVALID_DATA) {
                assert_equal(prev_ret, ret, "i:%d prev_ret (%d) != ret (%d)\n", i, prev_ret, ret);
            }
            assert_equal(prev_symbology, symbol->symbology, "i:%d prev_symbology (%d) != symbol->symbology (%d)\n",
                            i, prev_symbology, symbol->symbology);
        } else {
            /* No input data */
            assert_equal(ret, ZINT_ERROR_INVALID_DATA, "i:%d ret (%d) != ZINT_ERROR_INVALID_DATA\n", i, ret);
        }
    }

    testFinish();
}

static void test_input_mode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        char *data;
        int input_mode;
        int ret;

        int expected_input_mode;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "1234", DATA_MODE, 0, DATA_MODE, "" },
        /*  1*/ { "1234", DATA_MODE | ESCAPE_MODE, 0, DATA_MODE | ESCAPE_MODE, "" },
        /*  2*/ { "1234", UNICODE_MODE, 0, UNICODE_MODE, "" },
        /*  3*/ { "1234", UNICODE_MODE | ESCAPE_MODE, 0, UNICODE_MODE | ESCAPE_MODE, "" },
        /*  4*/ { "[01]12345678901231", GS1_MODE, 0, GS1_MODE, "" },
        /*  5*/ { "[01]12345678901231", GS1_MODE | ESCAPE_MODE, 0, GS1_MODE | ESCAPE_MODE, "" },
        /*  6*/ { "1234", 4 | ESCAPE_MODE, ZINT_WARN_INVALID_OPTION, DATA_MODE, "Warning 212: Invalid input mode - reset to DATA_MODE" }, /* Unknown mode reset to bare DATA_MODE. Note: now warns */
        /*  7*/ { "1234", -1, 0, DATA_MODE, "" },
        /*  8*/ { "1234", DATA_MODE | 0x10, 0, DATA_MODE | 0x10, "" }, /* Unknown flags kept (but ignored) */
        /*  9*/ { "1234", UNICODE_MODE | 0x10, 0, UNICODE_MODE | 0x10, "" },
        /* 10*/ { "[01]12345678901231", GS1_MODE | 0x20, 0, GS1_MODE | 0x20, "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_input_mode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODE49 /*Supports GS1*/, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(symbol->input_mode, data[i].expected_input_mode, "i:%d symbol->input_mode %d != %d\n", i, symbol->input_mode, data[i].expected_input_mode);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d ZBarcode_Encode strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_escape_char_process(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int eci;
        char *data;
        char *composite;
        int ret;
        int expected_width;
        char *expected;
        int compare_previous;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", "", 0, 26, "01 05 08 09 0A 0B 0C 0D 0E 1C 1E 1F EB 02 5D 81 21 0D 92 2E 3D FD B6 9A 37 2A CD 61 FB 95", 0, "" },
        /*  1*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", "", 0, 101, "(45) 67 62 43 40 44 47 48 29 6A 67 62 0B 49 4A 4B 4C 18 6A 67 62 0C 4D 5B 5D 5E 62 6A 67", 0, "" },
        /*  2*/ { BARCODE_CODE16K, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", "", 0, 70, "(20) 14 64 68 71 72 73 74 75 76 77 91 93 94 101 65 60 103 103 45 61", 0, "" },
        /*  3*/ { BARCODE_DOTCODE, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", "", 0, 28, "65 40 44 47 48 49 4A 4B 4C 4D 5B 5D 5E 6E 41 3C 6A", 0, "" },
        /*  4*/ { BARCODE_GRIDMATRIX, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", "", 0, 30, "30 1A 00 02 01 61 00 48 28 16 0C 06 46 63 51 74 05 38 00", 0, "" },
        /*  5*/ { BARCODE_HANXIN, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", "", 0, 23, "2F 80 10 72 09 28 B3 0D 6F F3 00 20 E8 F4 0A E0 00", 0, "" },
        /*  6*/ { BARCODE_MAXICODE, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", "", 0, 30, "(144) 04 3E 3E 00 04 07 08 09 0A 0B 03 3D 2C 24 19 1E 23 1B 18 0E 0C 0D 1E 3F 1D 1E 3C 31", 0, "" },
        /*  7*/ { BARCODE_PDF417, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", "", 0, 120, "(24) 16 901 0 23 655 318 98 18 461 639 893 122 129 92 900 900 872 438 359 646 522 773 831", 0, "" },
        /*  8*/ { BARCODE_ULTRA, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", "", 0, 20, "(15) 257 0 4 7 8 9 10 11 12 13 27 29 30 129 92", 0, "" },
        /*  9*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\d129\\o201\\\\", "", 0, 18, "(32) 01 05 08 09 0A 0B 0C 0D 0E 1C 1E 1F E7 32 45 DB 70 5D E3 16 7B 2B 44 60 E1 55 F7 08", 0, "" },
        /* 10*/ { BARCODE_HANXIN, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\d129\\o201\\\\", "", 0, 23, "2F 80 10 72 09 28 B3 0D 6F F3 00 30 E8 F4 0C 0C 0A E0 00 00 00", 0, "" },
        /* 11*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\c", "", ZINT_ERROR_INVALID_DATA, 0, "Error 234: Unrecognised escape character '\\c' in input", 0, "" },
        /* 12*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\", "", ZINT_ERROR_INVALID_DATA, 0, "Error 236: Incomplete escape character in input", 0, "" },
        /* 13*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\x", "", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete '\\x' escape sequence in input", 0, "" },
        /* 14*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\x1", "", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete '\\x' escape sequence in input", 0, "" },
        /* 15*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\x1g", "", ZINT_ERROR_INVALID_DATA, 0, "Error 238: Invalid character in escape sequence '\\x1g' in input (hexadecimal only)", 0, "" },
        /* 16*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\d", "", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete '\\d' escape sequence in input", 0, "" },
        /* 17*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\d1", "", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete '\\d' escape sequence in input", 0, "" },
        /* 18*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\d12", "", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete '\\d' escape sequence in input", 0, "" },
        /* 19*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\d12a", "", ZINT_ERROR_INVALID_DATA, 0, "Error 238: Invalid character in escape sequence '\\d12a' in input (decimal only)", 0, "" },
        /* 20*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\d256", "", ZINT_ERROR_INVALID_DATA, 0, "Error 237: Value of escape sequence '\\d256' in input out of range (000 to 255)", 0, "" },
        /* 21*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\o", "", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete '\\o' escape sequence in input", 0, "" },
        /* 22*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\o1", "", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete '\\o' escape sequence in input", 0, "" },
        /* 23*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\o12", "", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete '\\o' escape sequence in input", 0, "" },
        /* 24*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\o128", "", ZINT_ERROR_INVALID_DATA, 0, "Error 238: Invalid character in escape sequence '\\o128' in input (octal only)", 0, "" },
        /* 25*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\o400", "", ZINT_ERROR_INVALID_DATA, 0, "Error 237: Value of escape sequence '\\o400' in input out of range (000 to 377)", 0, "" },
        /* 26*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\xA01\\xFF", "", 0, 12, "EB 21 32 EB 80 D8 49 44 DC 7D 9E 3B", 0, "" },
        /* 27*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\d1601\\d255", "", 0, 12, "EB 21 32 EB 80 D8 49 44 DC 7D 9E 3B", 1, "" },
        /* 28*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\o2401\\o377", "", 0, 12, "EB 21 32 EB 80 D8 49 44 DC 7D 9E 3B", 1, "" },
        /* 29*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\u00A01\\u00FF", "", 0, 12, "EB 21 32 EB 80 D8 49 44 DC 7D 9E 3B", 1, "" },
        /* 30*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\U0000A01\\U0000FF", "", 0, 12, "EB 21 32 EB 80 D8 49 44 DC 7D 9E 3B", 1, "" },
        /* 31*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\xc3\\xbF", "", 0, 12, "EB 44 EB 40 81 30 87 17 C5 68 5C 91", 0, "" },
        /* 32*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\d195\\d191", "", 0, 12, "EB 44 EB 40 81 30 87 17 C5 68 5C 91", 1, "" },
        /* 33*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\o303\\o277", "", 0, 12, "EB 44 EB 40 81 30 87 17 C5 68 5C 91", 1, "" },
        /* 34*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\u00fF", "", 0, 12, "EB 44 EB 40 81 30 87 17 C5 68 5C 91", 1, "" },
        /* 35*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\U0000fF", "", 0, 12, "EB 44 EB 40 81 30 87 17 C5 68 5C 91", 1, "" },
        /* 36*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\xc3\\xbF", "", 0, 10, "EB 80 81 47 1E 45 FC 93", 0, "" },
        /* 37*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\d195\\d191", "", 0, 10, "EB 80 81 47 1E 45 FC 93", 1, "" },
        /* 38*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\o303\\o277", "", 0, 10, "EB 80 81 47 1E 45 FC 93", 1, "" },
        /* 39*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\u00fF", "", 0, 10, "EB 80 81 47 1E 45 FC 93", 1, "" },
        /* 40*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\U0000fF", "", 0, 10, "EB 80 81 47 1E 45 FC 93", 1, "" },
        /* 41*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\u", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\u' escape sequence in input", 0, "" },
        /* 42*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\uF", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\u' escape sequence in input", 0, "" },
        /* 43*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\u0F", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\u' escape sequence in input", 0, "" },
        /* 44*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\uFG", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\u' escape sequence in input", 0, "" },
        /* 45*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\u00F", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\u' escape sequence in input", 0, "" },
        /* 46*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\u00FG", "", ZINT_ERROR_INVALID_DATA, 0, "Error 211: Invalid character for '\\u' escape sequence in input (hexadecimal only)", 0, "" },
        /* 47*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\ufffe", "", ZINT_ERROR_INVALID_DATA, 0, "Error 246: Value of escape sequence '\\ufffe' in input out of range", 0, "Reversed BOM" },
        /* 48*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\ud800", "", ZINT_ERROR_INVALID_DATA, 0, "Error 246: Value of escape sequence '\\ud800' in input out of range", 0, "Surrogate" },
        /* 49*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\udfff", "", ZINT_ERROR_INVALID_DATA, 0, "Error 246: Value of escape sequence '\\udfff' in input out of range", 0, "Surrogate" },
        /* 50*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\uffff", "", 0, 12, "E7 2C B0 16 AB A1 1F 85 EB 50 A1 4C", 0, "" },
        /* 51*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 17, "\\xE2\\x82\\xAC", "", 0, 12, "F1 12 EB 25 81 4A 0A 8C 31 AC E3 2E", 0, "Zint manual 4.10 Ex1" },
        /* 52*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 17, "\\u20AC", "", 0, 12, "F1 12 EB 25 81 4A 0A 8C 31 AC E3 2E", 1, "" },
        /* 53*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 17, "\\U0020AC", "", 0, 12, "F1 12 EB 25 81 4A 0A 8C 31 AC E3 2E", 1, "" },
        /* 54*/ { BARCODE_DATAMATRIX, DATA_MODE, 17, "\\xA4", "", 0, 12, "F1 12 EB 25 81 4A 0A 8C 31 AC E3 2E", 1, "" },
        /* 55*/ { BARCODE_DATAMATRIX, DATA_MODE, 28, "\\xB1\\x60", "", 0, 12, "F1 1D EB 32 61 D9 1C 0C C2 46 C3 B2", 0, "Zint manual 4.10 Ex2" },
        /* 56*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 28, "\\u5E38", "", 0, 12, "F1 1D EB 32 61 D9 1C 0C C2 46 C3 B2", 1, "" },
        /* 57*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 28, "\\U005E38", "", 0, 12, "F1 1D EB 32 61 D9 1C 0C C2 46 C3 B2", 1, "" },
        /* 58*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\u007F", "", 0, 10, "80 81 46 73 64 88 6A 84", 0, "" },
        /* 59*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\U00007F", "", 0, 10, "80 81 46 73 64 88 6A 84", 0, "" },
        /* 60*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\U", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\U' escape sequence in input", 0, "" },
        /* 61*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\UF", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\U' escape sequence in input", 0, "" },
        /* 62*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\U0F", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\U' escape sequence in input", 0, "" },
        /* 63*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\UFG", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\U' escape sequence in input", 0, "" },
        /* 64*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\U00F", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\U' escape sequence in input", 0, "" },
        /* 65*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\U00FG", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\U' escape sequence in input", 0, "" },
        /* 66*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\Ufffe", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\U' escape sequence in input", 0, "Reversed BOM" },
        /* 67*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\Ud800", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\U' escape sequence in input", 0, "Surrogate" },
        /* 68*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\Udfff", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\U' escape sequence in input", 0, "Surrogate" },
        /* 69*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\U000F", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\U' escape sequence in input", 0, "" },
        /* 70*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\U0000F", "", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete '\\U' escape sequence in input", 0, "" },
        /* 71*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\U110000", "", ZINT_ERROR_INVALID_DATA, 0, "Error 246: Value of escape sequence '\\U110000' in input out of range", 0, "" },
        /* 72*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 25, "\\U10FFFF", "", 0, 14, "F1 1A E7 57 C7 81 F7 AC 09 06 28 51 F3 00 E1 8C 2A 1C", 0, "" },
        /* 73*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 26, "\\U10FFFF", "", 0, 14, "F1 1B E7 57 E0 11 D7 6C 4F 45 E2 B3 FF F1 72 AB 54 9F", 0, "" },
        /* 74*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 32, "\\U10FFFF", "", 0, 14, "F1 21 EB 64 33 EB 1B 36 1D F7 B1 6D 8C A6 34 64 19 3A", 0, "" },
        /* 75*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 33, "\\U10FFFF", "", 0, 14, "F1 22 E7 57 EB 5D 17 8C C1 B0 B6 B2 53 78 E4 7D 61 CB", 0, "" },
        /* 76*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 34, "\\U10FFFF", "", 0, 14, "F1 23 01 11 EB 80 EB 80 90 33 51 1B FA AE 78 F7 05 44", 0, "" },
        /* 77*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 35, "\\U10FFFF", "", 0, 14, "F1 24 EB 80 EB 80 11 01 17 BA C6 05 9F 4C EA E5 18 31", 0, "" },
        /* 78*/ { BARCODE_GS1_128_CC, GS1_MODE, -1, "[20]10", "[10]A", 0, 99, "(7) 105 102 20 10 100 59 106", 0, "" },
        /* 79*/ { BARCODE_GS1_128_CC, GS1_MODE, -1, "[2\\x30]1\\d048", "[\\x310]\\x41", 0, 99, "(7) 105 102 20 10 100 59 106", 1, "" },
        /* 80*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\^A1", "", ZINT_ERROR_INVALID_DATA, 0, "Error 798: Escape '\\^' only valid for Code 128 in extra escape mode", 0, "" },
        /* 81*/ { BARCODE_CODE128, DATA_MODE | EXTRA_ESCAPE_MODE, -1, "\\^A1", "", 0, 46, "(4) 103 17 17 106", 0, "" },
        /* 82*/ { BARCODE_CODE128, EXTRA_ESCAPE_MODE, -1, "\\^", "", 0, 57, "(5) 104 60 62 82 106", 0, "Partial special escape '\\^' at end allowed" },
        /* 83*/ { BARCODE_CODE128, EXTRA_ESCAPE_MODE, -1, "\\^D1", "", 0, 79, "(7) 104 60 62 36 17 52 106", 0, "Unknown special escapes passed straight thu" },
        /* 84*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\w", "", ZINT_ERROR_INVALID_DATA, 0, "Error 234: Unrecognised escape character '\\w' in input", 0, "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char escaped_composite[1024];
    struct zint_symbol previous_symbol;
    char *input_filename = "test_escape.txt";

    char *text;

    testStartSymbol("test_escape_char_process", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (is_composite(data[i].symbology)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode | ESCAPE_MODE, data[i].eci, -1 /*option_1*/, -1, -1, -1 /*output_options*/, text, -1, debug);

        ret = ZBarcode_Encode(symbol, TU(text), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, \"%s\", \"%s\", %s, %d, \"%s\", %d, \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].eci,
                    testUtilEscape(data[i].data, length, escaped, ARRAY_SIZE(escaped)),
                    testUtilEscape(data[i].composite, (int) strlen(data[i].composite), escaped_composite, ARRAY_SIZE(escaped_composite)),
                    testUtilErrorName(data[i].ret), symbol->width, symbol->errtxt, data[i].compare_previous, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
                if (p_ctx->index == -1 && data[i].compare_previous) {
                    ret = testUtilSymbolCmp(symbol, &previous_symbol);
                    assert_zero(ret, "i:%d testUtilSymbolCmp ret %d != 0\n", i, ret);
                }
            }
            memcpy(&previous_symbol, symbol, sizeof(previous_symbol));

            if (ret < ZINT_ERROR && !data[i].composite[0]) {
                /* Test from input file */
                FILE *fp;
                struct zint_symbol *symbol2;

                fp = testUtilOpen(input_filename, "wb");
                assert_nonnull(fp, "i:%d testUtilOpen(%s) failed\n", i, input_filename);
                assert_notequal(fputs(data[i].data, fp), EOF, "i%d fputs(%s) failed == EOF (%d)\n", i, data[i].data, ferror(fp));
                assert_zero(fclose(fp), "i%d fclose() failed\n", i);

                symbol2 = ZBarcode_Create();
                assert_nonnull(symbol, "Symbol2 not created\n");

                symbol2->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

                (void) testUtilSetSymbol(symbol2, data[i].symbology, data[i].input_mode | ESCAPE_MODE, data[i].eci, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

                ret = ZBarcode_Encode_File(symbol2, input_filename);
                assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_File ret %d != %d (%s)\n", i, ret, data[i].ret, symbol2->errtxt);
                assert_zero(strcmp(symbol2->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol2->errtxt, data[i].expected);

                ret = testUtilSymbolCmp(symbol2, symbol);
                assert_zero(ret, "i:%d testUtilSymbolCmp symbol2 ret %d != 0\n", i, ret);

                assert_zero(testUtilRemove(input_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, input_filename, errno, strerror(errno));

                ZBarcode_Delete(symbol2);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

INTERNAL int escape_char_process_test(struct zint_symbol *symbol, unsigned char *input_string, int *p_length,
                unsigned char *escaped_string);

static void test_escape_char_process_test(const testCtx *const p_ctx) {

    struct item {
        int symbology;
        int input_mode;
        char *data;
        int ret;
        char *expected;
        int expected_len;
    };
    static const struct item data[] = {
        /*  0*/ { 0, 0, "BLANK", 0, "BLANK", 5 },
        /*  1*/ { 0, 0, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\\\o201\\d255", 0, "\000\004\a\b\t\n\v\f\r\033\035\036\201\\\201\377", 16 },
        /*  2*/ { 0, 0, "\\U010283", 0, "\360\220\212\203", 4 },
        /*  3*/ { 0, 0, "\\u007F\\u0080\\u011E\\u13C9\\U010283", 0, "\177\302\200\304\236\341\217\211\360\220\212\203", 12 },
        /*  4*/ { BARCODE_CODE128, EXTRA_ESCAPE_MODE, "\\^A\\^^\\^B", 0, "\\^A\\^^\\^B", 9 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;


    testStart("test_escape_char_process_test");

    for (i = 0; i < data_size; i++) {

        struct zint_symbol s_symbol = {0};
        struct zint_symbol *symbol = &s_symbol;
        int escaped_len;
        char escaped[1024];

        if (testContinue(p_ctx, i)) continue;

        symbol->symbology = data[i].symbology;
        symbol->input_mode = data[i].input_mode;
        length = (int) strlen(data[i].data);

        escaped_len = length;
        ret = escape_char_process_test(symbol, (unsigned char *) data[i].data, &escaped_len, NULL);
        assert_equal(ret, data[i].ret, "i:%d escape_char_process_test(NULL) ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(escaped_len, data[i].expected_len, "i:%d NULL escaped_len %d != %d\n", i, escaped_len, data[i].expected_len);

        memset(escaped, 0xDD, sizeof(escaped));

        escaped_len = length;
        ret = escape_char_process_test(symbol, (unsigned char *) data[i].data, &escaped_len, (unsigned char *) escaped);
        assert_equal(ret, data[i].ret, "i:%d escape_char_process_test(escaped) ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(escaped_len, data[i].expected_len, "i:%d escaped escaped_len %d != %d\n", i, escaped_len, data[i].expected_len);

        assert_zero(memcmp(escaped, data[i].expected, escaped_len), "i:%d memcmp() != 0\n", i);
        assert_zero(escaped[escaped_len], "i:%d escaped[%d] not NUL-terminated (0x%X)\n", i, escaped_len, escaped[escaped_len]);
    }

    testFinish();
}

static void test_cap(const testCtx *const p_ctx) {

    struct item {
        int symbology;
        unsigned cap_flag;
        unsigned int expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /* 0*/ { BARCODE_CODE128, ZINT_CAP_HRT, ZINT_CAP_HRT },
        /* 1*/ { BARCODE_CODE128, ZINT_CAP_HRT | ZINT_CAP_STACKABLE | ZINT_CAP_GS1, ZINT_CAP_HRT | ZINT_CAP_STACKABLE },
        /* 2*/ { BARCODE_PDF417, ZINT_CAP_HRT | ZINT_CAP_ECI | ZINT_CAP_GS1 | ZINT_CAP_READER_INIT | ZINT_CAP_FULL_MULTIBYTE, ZINT_CAP_ECI | ZINT_CAP_READER_INIT },
        /* 3*/ { BARCODE_QRCODE, ZINT_CAP_HRT | ZINT_CAP_ECI | ZINT_CAP_GS1 | ZINT_CAP_DOTTY | ZINT_CAP_READER_INIT | ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_MASK | ZINT_CAP_STRUCTAPP | ZINT_CAP_COMPLIANT_HEIGHT, ZINT_CAP_ECI | ZINT_CAP_GS1 | ZINT_CAP_DOTTY | ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_MASK | ZINT_CAP_STRUCTAPP },
        /* 4*/ { BARCODE_EANX_CC, ZINT_CAP_HRT | ZINT_CAP_COMPOSITE | ZINT_CAP_EXTENDABLE | ZINT_CAP_ECI | ZINT_CAP_GS1 | ZINT_CAP_QUIET_ZONES | ZINT_CAP_COMPLIANT_HEIGHT, ZINT_CAP_HRT | ZINT_CAP_COMPOSITE | ZINT_CAP_EXTENDABLE | ZINT_CAP_GS1 | ZINT_CAP_QUIET_ZONES | ZINT_CAP_COMPLIANT_HEIGHT },
        /* 5*/ { BARCODE_HANXIN, ZINT_CAP_DOTTY | ZINT_CAP_QUIET_ZONES | ZINT_CAP_FIXED_RATIO | ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_MASK, ZINT_CAP_DOTTY | ZINT_CAP_FIXED_RATIO | ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_MASK },
        /* 6*/ { BARCODE_CODE11, ZINT_CAP_DOTTY | ZINT_CAP_FIXED_RATIO | ZINT_CAP_FIXED_RATIO | ZINT_CAP_READER_INIT | ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_COMPLIANT_HEIGHT, 0 },
        /* 7*/ { BARCODE_POSTNET, ZINT_CAP_HRT | ZINT_CAP_STACKABLE | ZINT_CAP_EXTENDABLE | ZINT_CAP_COMPOSITE | ZINT_CAP_ECI | ZINT_CAP_GS1 | ZINT_CAP_DOTTY | ZINT_CAP_FIXED_RATIO | ZINT_CAP_READER_INIT | ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_MASK | ZINT_CAP_STRUCTAPP, 0 },
        /* 8*/ { 0, 0, 0 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i;
    unsigned int uret;

    testStart("test_cap");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        uret = ZBarcode_Cap(data[i].symbology, data[i].cap_flag);
        assert_equal(uret, data[i].expected, "i:%d ZBarcode_Cap(%s, 0x%X) 0x%X != 0x%X\n", i, testUtilBarcodeName(data[i].symbology), data[i].cap_flag, uret, data[i].expected);
    }

    testFinish();
}

static void test_cap_compliant_height(const testCtx *const p_ctx) {
    int symbol_id;
    unsigned int uret;

    testStart("test_cap_compliant_height");

    for (symbol_id = 1; symbol_id <= BARCODE_LAST; symbol_id++) {
        if (!ZBarcode_ValidID(symbol_id)) continue;
        if (testContinue(p_ctx, symbol_id)) continue;

        uret = ZBarcode_Cap(symbol_id, ZINT_CAP_COMPLIANT_HEIGHT);

        switch (symbol_id) {
            /*case BARCODE_CODE11: TODO: Find doc */
            case BARCODE_C25INTER:
            case BARCODE_CODE39:
            case BARCODE_EXCODE39:
            case BARCODE_EANX:
            case BARCODE_EANX_CHK:
            case BARCODE_GS1_128:
            case BARCODE_CODABAR:
            /*case BARCODE_DPLEIT: TODO: Find doc */
            /*case BARCODE_DPIDENT: TODO: Find doc */
            case BARCODE_CODE16K:
            case BARCODE_CODE49:
            case BARCODE_CODE93:
            /*case BARCODE_FLAT: TODO: Find doc */
            case BARCODE_DBAR_OMN:
            case BARCODE_DBAR_LTD:
            case BARCODE_DBAR_EXP:
            case BARCODE_TELEPEN:
            case BARCODE_UPCA:
            case BARCODE_UPCA_CHK:
            case BARCODE_UPCE:
            case BARCODE_UPCE_CHK:
            case BARCODE_POSTNET:
            /*case BARCODE_MSI_PLESSEY: TODO: Find doc */
            case BARCODE_FIM:
            case BARCODE_LOGMARS:
            case BARCODE_PHARMA:
            case BARCODE_PZN:
            case BARCODE_PHARMA_TWO:
            case BARCODE_CEPNET:
            case BARCODE_AUSPOST:
            case BARCODE_AUSREPLY:
            case BARCODE_AUSROUTE:
            case BARCODE_AUSREDIRECT:
            case BARCODE_ISBNX:
            case BARCODE_RM4SCC:
            case BARCODE_EAN14:
            /*case BARCODE_VIN: Spec unlikely */
            case BARCODE_CODABLOCKF:
            case BARCODE_NVE18:
            case BARCODE_JAPANPOST:
            /*case BARCODE_KOREAPOST: TODO: Find doc */
            case BARCODE_DBAR_STK:
            case BARCODE_DBAR_OMNSTK:
            case BARCODE_DBAR_EXPSTK:
            case BARCODE_PLANET:
            case BARCODE_USPS_IMAIL:
            /*case BARCODE_PLESSEY: TODO: Find doc */
            case BARCODE_TELEPEN_NUM:
            case BARCODE_ITF14:
            case BARCODE_KIX:
            case BARCODE_DPD:
            case BARCODE_HIBC_39:
            case BARCODE_HIBC_BLOCKF:
            case BARCODE_UPU_S10:
            case BARCODE_MAILMARK_4S:
            case BARCODE_CODE32:
            case BARCODE_EANX_CC:
            case BARCODE_GS1_128_CC:
            case BARCODE_DBAR_OMN_CC:
            case BARCODE_DBAR_LTD_CC:
            case BARCODE_DBAR_EXP_CC:
            case BARCODE_UPCA_CC:
            case BARCODE_UPCE_CC:
            case BARCODE_DBAR_STK_CC:
            case BARCODE_DBAR_OMNSTK_CC:
            case BARCODE_DBAR_EXPSTK_CC:
            case BARCODE_CHANNEL:
            case BARCODE_BC412:
            case BARCODE_DXFILMEDGE:
                /* Make sure ZINT_CAP_COMPLIANT_HEIGHT set for those that have it */
                assert_equal(uret, ZINT_CAP_COMPLIANT_HEIGHT, "symbol_id %d (%s) uret 0x%X != ZINT_CAP_COMPLIANT_HEIGHT\n", symbol_id, testUtilBarcodeName(symbol_id), uret);
                break;
            default:
                /* And not set for those that don't */
                assert_zero(uret, "symbol_id %d (%s) uret 0x%X non-zero\n", symbol_id, testUtilBarcodeName(symbol_id), uret);
                break;
        }
    }

    testFinish();
}

static void test_encode_file_empty(const testCtx *const p_ctx) {
    int ret;
    struct zint_symbol *symbol = NULL;
    char filename[] = "in.bin";
    FILE *fstream;

    (void)p_ctx;

    testStartSymbol("test_encode_file_empty", &symbol);

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    (void) testUtilRemove(filename); /* In case junk hanging around */

    fstream = testUtilOpen(filename, "w+");
    assert_nonnull(fstream, "testUtilOpen(%s) failed (%d)\n", filename, errno);
    ret = fclose(fstream);
    assert_zero(ret, "fclose(%s) %d != 0\n", filename, ret);

    ret = ZBarcode_Encode_File(symbol, filename);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File empty ret %d != ZINT_ERROR_INVALID_DATA (%s)\n", ret, symbol->errtxt);

    ret = testUtilRemove(filename);
    assert_zero(ret, "testUtilRemove(%s) != 0 (%d: %s)\n", filename, errno, strerror(errno));

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_encode_file_too_large(const testCtx *const p_ctx) {
    char filename[] = "in.bin";
    FILE *fstream;
    int ret;
    struct zint_symbol *symbol = NULL;
    char buf[ZINT_MAX_DATA_LEN + 1] = {0};

    (void)p_ctx;

    testStartSymbol("test_encode_file_too_large", &symbol);

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    (void) testUtilRemove(filename); /* In case junk hanging around */

    fstream = testUtilOpen(filename, "w+");
    assert_nonnull(fstream, "testUtilOpen(%s) failed (%d)\n", filename, errno);
    ret = (int) fwrite(buf, 1, sizeof(buf), fstream);
    assert_equal(ret, sizeof(buf), "fwrite return value: %d != %d\n", ret, (int)sizeof(buf));
    ret = fclose(fstream);
    assert_zero(ret, "fclose(%s) %d != 0\n", filename, ret);

    ret = ZBarcode_Encode_File(symbol, filename);
    assert_equal(ret, ZINT_ERROR_TOO_LONG, "ZBarcode_Encode_File too large ret %d != ZINT_ERROR_TOO_LONG (%s)\n", ret, symbol->errtxt);

    ret = testUtilRemove(filename);
    assert_zero(ret, "testUtilRemove(%s) != 0 (%d: %s)\n", filename, errno, strerror(errno));

    ZBarcode_Delete(symbol);

    testFinish();
}

/* #181 Nico Gunkel OSS-Fuzz */
static void test_encode_file_unreadable(const testCtx *const p_ctx) {
#ifndef _WIN32
    int ret;
    char filename[] = "in.bin";

    char buf[ZINT_MAX_DATA_LEN + 1] = {0};
    int fd;
#endif
    struct zint_symbol *symbol = NULL;

    (void)p_ctx;

    testStartSymbol("test_encode_file_unreadable", &symbol);

#ifdef _WIN32
    testSkip("Test not implemented on Windows");
#else
    if (getuid() == 0) {
        testSkip("Test not available as root");
        return;
    }

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    /* Unreadable file */
    fd = creat(filename, S_IWUSR);
    assert_notequal(fd, -1, "Unreadable input file (%s) not created == -1 (%d: %s)\n", filename, errno, strerror(errno));
    ret = write(fd, buf, 1);
    assert_equal(ret, 1, "Unreadable write ret %d != 1\n", ret);
    ret = close(fd);
    assert_zero(ret, "Unreadable close(%s) != 0(%d: %s)\n", filename, errno, strerror(errno));

    ret = ZBarcode_Encode_File(symbol, filename);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File unreadable ret %d != ZINT_ERROR_INVALID_DATA (%s)\n", ret, symbol->errtxt);

    ret = testUtilRemove(filename);
    assert_zero(ret, "testUtilRemove(%s) != 0 (%d: %s)\n", filename, errno, strerror(errno));

    ZBarcode_Delete(symbol);

    testFinish();
#endif /* _WIN32 */
}

/* #181 Nico Gunkel OSS-Fuzz (buffer not freed on fread() error) Note: unable to reproduce fread() error using this method */
static void test_encode_file_directory(const testCtx *const p_ctx) {
    int ret;
    struct zint_symbol *symbol = NULL;
    char dirname[] = "in_dir";

    (void)p_ctx;

    testStartSymbol("test_encode_file_directory", &symbol);

#if defined(__NetBSD__) || defined(_AIX)
    /* Reading a directory works on NetBSD, and get `code128()` ZINT_ERROR_TOO_LONG instead */
    (void)ret; (void)symbol; (void)dirname;
    testSkip("Test not implemented on NetBSD or AIX");
#else
    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    (void) testUtilRmDir(dirname); /* In case junk hanging around */
    ret = testUtilMkDir(dirname);
    assert_zero(ret, "testUtilMkDir(%s) %d != 0 (%d: %s)\n", dirname, ret, errno, strerror(errno));

    ret = ZBarcode_Encode_File(symbol, dirname);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "ret %d != ZINT_ERROR_INVALID_DATA (%s)\n", ret, symbol->errtxt);

    ret = testUtilRmDir(dirname);
    assert_zero(ret, "testUtilRmDir(%s) %d != 0 (%d: %s)\n", dirname, ret, errno, strerror(errno));

    ZBarcode_Delete(symbol);

    testFinish();
#endif /* __NetBSD__ */
}

static void test_encode_file(const testCtx *const p_ctx) {
    int ret;
    struct zint_symbol *symbol = NULL;
    char *data = "1";
    char *filename = "test_encode_file_in.txt";
    char *outfile = "test_encode_file_out.gif";
    FILE *fp;

    (void)p_ctx;

    testStartSymbol("test_encode_file", &symbol);

    (void) testUtilRemove(filename); /* In case junk hanging around */
    (void) testUtilRemove(outfile); /* In case junk hanging around */

    fp = testUtilOpen(filename, "w+");
    assert_nonnull(fp, "testUtilOpen(%s) failed (%d)\n", filename, errno);
    assert_notequal(fputs(data, fp), EOF, "fputs(%s) failed == EOF (%d)\n", data, ferror(fp));
    ret = fclose(fp);
    assert_zero(ret, "fclose(%s) %d != 0\n", filename, ret);

    {
        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, outfile);
        ret = ZBarcode_Encode_File_and_Print(symbol, filename, 0);
        assert_zero(ret, "ret %d != 0 (%s)\n", ret, symbol->errtxt);

        ret = testUtilRemove(outfile);
        assert_zero(ret, "testUtilRemove(%s) != 0 (%d: %s)\n", outfile, errno, strerror(errno));

        ZBarcode_Delete(symbol);
    }

    {
        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, outfile);
        ret = ZBarcode_Encode_File_and_Buffer(symbol, filename, 0);
        assert_zero(ret, "ret %d != 0 (%s)\n", ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "symbol->bitmap NULL (%s)\n", symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    {
        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, outfile);
        ret = ZBarcode_Encode_File_and_Buffer_Vector(symbol, filename, 0);
        assert_zero(ret, "ret %d != 0 (%s)\n", ret, symbol->errtxt);
        assert_nonnull(symbol->vector, "symbol->vector NULL (%s)\n", symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    ret = testUtilRemove(filename);
    assert_zero(ret, "testUtilRemove(%s) != 0 (%d: %s)\n", filename, errno, strerror(errno));

    testFinish();
}

static void test_encode_print_outfile_directory(const testCtx *const p_ctx) {
    int ret;
    struct zint_symbol *symbol = NULL;
    char dirname[] = "outdir.txt";
    char expected[] = "Error 201: Could not open output file";

    (void)p_ctx;

    testStartSymbol("test_encode_print_outfile_directory", &symbol);

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    (void) testUtilRmDir(dirname); /* In case junk hanging around */
    ret = testUtilMkDir(dirname);
    assert_zero(ret, "testUtilMkDir(%s) %d != 0 (%d: %s)\n", dirname, ret, errno, strerror(errno));

    strcpy(symbol->outfile, dirname);
    ret = ZBarcode_Encode_and_Print(symbol, TU("1"), 0, 0);
    assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "ret %d != ZINT_ERROR_FILE_ACCESS (%s)\n", ret, symbol->errtxt);
    assert_zero(strcmp(symbol->errtxt, expected), "strcmp(%s, %s) != 0\n", symbol->errtxt, expected);

    ret = testUtilRmDir(dirname);
    assert_zero(ret, "testUtilRmDir(%s) %d != 0 (%d: %s)\n", dirname, ret, errno, strerror(errno));

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_bad_args(const testCtx *const p_ctx) {
    int ret;
    unsigned int uret;
    struct zint_symbol *symbol = NULL;
    char *data = "1";
    char *filename = "1.png";
    char *empty = "";
    struct zint_seg seg = { TU("1"), -1, 4 };
    struct zint_seg seg_empty = { TU(""), -1, 4 };
    struct zint_seg seg_too_long = { TU("1"), ZINT_MAX_DATA_LEN + 1, 4 };
    const char *expected[] = {
        "Error 772: Input segment 0 source NULL",
        "Error 200: Input segments NULL",
        "Error 239: Filename NULL",
        "Error 778: No input data",
        "Error 229: Unable to read input file (", /* Excluding OS-dependent `errno` stuff */
        "Error 771: Too many input segments (maximum 256)",
        "Error 205: No input data",
        "Error 777: Input too long",
    };

    (void)p_ctx;

    testStartSymbol("test_bad_args", &symbol);

    /* These just return, no error */
    ZBarcode_Clear(NULL);
    ZBarcode_Reset(NULL);
    ZBarcode_Delete(NULL);

    ret = ZBarcode_Version();
    assert_nonzero(ret >= 20901, "ZBarcode_Version() %d <= 20901\n", ret);

    assert_zero(ZBarcode_ValidID(0), "ZBarcode_ValidID(0) non-zero\n");
    assert_zero(ZBarcode_ValidID(10), "ZBarcode_ValidID(10) non-zero\n"); /* Note 10 remapped to BARCODE_EANX in ZBarcode_Encode() for tbarcode compat but not counted as valid */

    uret = ZBarcode_Cap(0, ~0);
    assert_zero(uret, "ZBarcode_Cap(0, ~0) uret 0x%X != 0\n", uret);
    uret = ZBarcode_Cap(10, ~0);
    assert_zero(uret, "ZBarcode_Cap(10, ~0) uret 0x%X != 0\n", uret);

    /* NULL symbol */
    assert_equal(ZBarcode_Encode(NULL, TU(data), 1), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode(NULL, data, 1) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_Segs(NULL, &seg, 1), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs(NULL, &seg, 1) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Print(NULL, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Print(NULL, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Buffer(NULL, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Buffer(NULL, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Buffer_Vector(NULL, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Buffer_Vector(NULL, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_and_Print(NULL, TU(data), 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Print(NULL, data, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_Segs_and_Print(NULL, &seg, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Seg_and_Print(NULL, &seg, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_and_Buffer(NULL, TU(data), 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Buffer(NULL, data, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_Segs_and_Buffer(NULL, &seg, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs_and_Buffer(NULL, &seg, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_and_Buffer_Vector(NULL, TU(data), 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Buffer_Vector(NULL, data, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_Segs_and_Buffer_Vector(NULL, &seg, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs_and_Buffer_Vector(NULL, &seg, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_File(NULL, filename), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File(NULL, filename) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_File_and_Print(NULL, filename, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Print(NULL, filename, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_File_and_Buffer(NULL, filename, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Buffer(NULL, filename, 0) != ZINT_ERROR_INVALID_DATA\n");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    /* NULL data/segs/filename */
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode(symbol, NULL, 1), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode(symbol, NULL, 1) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[0], symbol->errtxt), "ZBarcode_Encode(symbol, NULL, 1) strcmp(%s, %s) != 0\n", expected[0], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_Segs(symbol, NULL, 1), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs(symbol, NULL, 1) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[1], symbol->errtxt), "ZBarcode_Encode_Segs(symbol, NULL, 1) strcmp(%s, %s) != 0\n", expected[0], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_and_Print(symbol, NULL, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Print(symbol, NULL, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[0], symbol->errtxt), "ZBarcode_Encode_and_Print(symbol, NULL, 1, 0) strcmp(%s, %s) != 0\n", expected[0], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_and_Buffer(symbol, NULL, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Buffer(symbol, NULL, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[0], symbol->errtxt), "ZBarcode_Encode_and_Buffer(symbol, NULL, 1, 0) strcmp(%s, %s) != 0\n", expected[0], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_Segs_and_Buffer(symbol, NULL, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs_and_Buffer(symbol, NULL, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[1], symbol->errtxt), "ZBarcode_Encode_Segs_and_Buffer(symbol, NULL, 1, 0) strcmp(%s, %s) != 0\n", expected[0], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_and_Buffer_Vector(symbol, NULL, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Buffer_Vector(symbol, NULL, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[0], symbol->errtxt), "ZBarcode_Encode_and_Buffer_Vector(symbol, NULL, 1, 0) strcmp(%s, %s) != 0\n", expected[0], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_Segs_and_Buffer_Vector(symbol, NULL, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs_and_Buffer_Vector(symbol, NULL, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[1], symbol->errtxt), "ZBarcode_Encode_Segs_and_Buffer_Vector(symbol, NULL, 1, 0) strcmp(%s, %s) != 0\n", expected[0], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File(symbol, NULL), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File(symbol, NULL) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[2], symbol->errtxt), "ZBarcode_Encode_File(symbol, NULL) strcmp(%s, %s) != 0\n", expected[2], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File_and_Print(symbol, NULL, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Print(symbol, NULL, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[2], symbol->errtxt), "ZBarcode_Encode_File_and_Print(symbol, NULL, 0) strcmp(%s, %s) != 0\n", expected[2], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File_and_Buffer(symbol, NULL, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Buffer(symbol, NULL, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[2], symbol->errtxt), "ZBarcode_Encode_File_and_Buffer(symbol, NULL, 0) strcmp(%s, %s) != 0\n", expected[2], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File_and_Buffer_Vector(symbol, NULL, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Buffer_Vector(symbol, NULL, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[2], symbol->errtxt), "ZBarcode_Encode_File_and_Buffer_Vector(symbol, NULL, 0) strcmp(%s, %s) != 0\n", expected[2], symbol->errtxt);

    /* Empty data/segs/filename */
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode(symbol, TU(empty), 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode(symbol, empty, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[3], symbol->errtxt), "ZBarcode_Encode(symbol, TU(empty), 0) strcmp(%s, %s) != 0\n", expected[3], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_Segs(symbol, &seg_empty, 1), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs(symbol, &seg_empty, 1) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[3], symbol->errtxt), "ZBarcode_Encode_Segs(symbol, &seg_empty, 1) strcmp(%s, %s) != 0\n", expected[3], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_and_Print(symbol, TU(empty), 0, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Print(symbol, empty, 0, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[3], symbol->errtxt), "ZBarcode_Encode_and_Print(symbol, TU(empty), 0, 0) strcmp(%s, %s) != 0\n", expected[3], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_and_Buffer(symbol, TU(empty), 0, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Buffer(symbol, empty, 0, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[3], symbol->errtxt), "ZBarcode_Encode_and_Buffer(symbol, TU(empty), 0, 0) strcmp(%s, %s) != 0\n", expected[3], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_Segs_and_Buffer(symbol, &seg_empty, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs_and_Buffer(symbol, &seg_empty, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[3], symbol->errtxt), "ZBarcode_Encode_Segs_and_Buffer(symbol, &seg_empty, 1, 0) strcmp(%s, %s) != 0\n", expected[3], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_and_Buffer_Vector(symbol, TU(empty), 0, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Buffer_Vector(symbol, empty, 0, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[3], symbol->errtxt), "ZBarcode_Encode_and_Buffer_Vector(symbol, TU(empty), 0, 0) strcmp(%s, %s) != 0\n", expected[3], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_Segs_and_Buffer_Vector(symbol, &seg_empty, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs_and_Buffer_Vector(symbol, &seg_empty, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[3], symbol->errtxt), "ZBarcode_Encode_Segs_and_Buffer_Vector(symbol, &seg_empty, 1, 0) strcmp(%s, %s) != 0\n", expected[3], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File(symbol, empty), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File(symbol, empty) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strncmp(expected[4], symbol->errtxt, strlen(expected[4])), "ZBarcode_Encode_File(symbol, empty) strncmp(%s, %s, %d) != 0\n", expected[4], symbol->errtxt, (int) strlen(expected[4]));
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File_and_Print(symbol, empty, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Print(symbol, empty, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strncmp(expected[4], symbol->errtxt, strlen(expected[4])), "ZBarcode_Encode_File(symbol, empty) strncmp(%s, %s, %d) != 0\n", expected[4], symbol->errtxt, (int) strlen(expected[4]));
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File_and_Buffer(symbol, empty, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Buffer(symbol, empty, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strncmp(expected[4], symbol->errtxt, strlen(expected[4])), "ZBarcode_Encode_File(symbol, empty) strncmp(%s, %s, %d) != 0\n", expected[4], symbol->errtxt, (int) strlen(expected[4]));
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File_and_Buffer_Vector(symbol, empty, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Buffer_Vector(symbol, empty, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strncmp(expected[4], symbol->errtxt, strlen(expected[4])), "ZBarcode_Encode_File(symbol, empty) strncmp(%s, %s, %d) != 0\n", expected[4], symbol->errtxt, (int) strlen(expected[4]));

    /* Bad seg_count */
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_Segs(symbol, &seg_empty, ZINT_MAX_SEG_COUNT + 1), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs(symbol, &seg_empty, ZINT_MAX_SEG_COUNT + 1) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[5], symbol->errtxt), "ZBarcode_Encode_Segs(symbol, &seg_empty, ZINT_MAX_SEG_COUNT + 1) strcmp(%s, %s) != 0\n", expected[5], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_Segs(symbol, &seg_empty, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs(symbol, &seg_empty, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[6], symbol->errtxt), "ZBarcode_Encode_Segs(symbol, &seg_empty, 0) strcmp(%s, %s) != 0\n", expected[6], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_Segs(symbol, &seg_empty, -1), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_Segs(symbol, &seg_empty, -1) != ZINT_ERROR_INVALID_DATA\n");
    assert_zero(strcmp(expected[6], symbol->errtxt), "ZBarcode_Encode_Segs(symbol, &seg_empty, -1) strcmp(%s, %s) != 0\n", expected[6], symbol->errtxt);

    /* Data/seg too big */
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode(symbol, TU(empty), ZINT_MAX_DATA_LEN + 1), ZINT_ERROR_TOO_LONG, "ZBarcode_Encode(symbol, empty, ZINT_MAX_DATA_LEN + 1) != ZINT_ERROR_TOO_LONG\n");
    assert_zero(strcmp(expected[7], symbol->errtxt), "ZBarcode_Encode(symbol, TU(empty), ZINT_MAX_DATA_LEN + 1) strcmp(%s, %s) != 0\n", expected[7], symbol->errtxt);
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_Segs(symbol, &seg_too_long, 1), ZINT_ERROR_TOO_LONG, "ZBarcode_Encode_Segs(symbol, &seg_too_long, 1) != ZINT_ERROR_TOO_LONG\n");
    assert_zero(strcmp(expected[7], symbol->errtxt), "ZBarcode_Encode_Segs(symbol, &seg_too_long, 1) strcmp(%s, %s) != 0\n", expected[7], symbol->errtxt);

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_valid_id(const testCtx *const p_ctx) {

    int ret;
    const char *name;
    int symbol_id;

    (void)p_ctx;

    testStart("test_valid_id");

    for (symbol_id = -1; symbol_id < 160; symbol_id++) {
        ret = ZBarcode_ValidID(symbol_id);
        name = testUtilBarcodeName(symbol_id);
        assert_nonnull((char *) name, "testUtilBarcodeName(%d) NULL\n", symbol_id);
        if (ret) {
            assert_equal(ret, 1, "ZBarcode_Valid(%d) != 1\n", symbol_id);
            assert_nonzero(*name != '\0', "testUtilBarcodeName(%d) empty when ZBarcode_Valid() true\n", symbol_id);
        } else {
            assert_zero(ret, "ZBarcode_Valid(%d) != 0\n", symbol_id);
            assert_zero(*name, "testUtilBarcodeName(%d) non-empty when ZBarcode_Valid() false\n", symbol_id);
        }
    }

    testFinish();
}

static int test_prev_ZBarcode_BarcodeName(int symbol_id, char name[32]) {
    struct item {
        const char *name;
        int define;
        int val;
    };
    static const struct item data[] = {
        { "", -1, 0 },
        { "BARCODE_CODE11", BARCODE_CODE11, 1 },
        { "BARCODE_C25STANDARD", BARCODE_C25STANDARD, 2 },
        { "BARCODE_C25INTER", BARCODE_C25INTER, 3 },
        { "BARCODE_C25IATA", BARCODE_C25IATA, 4 },
        { "", -1, 5 },
        { "BARCODE_C25LOGIC", BARCODE_C25LOGIC, 6 },
        { "BARCODE_C25IND", BARCODE_C25IND, 7 },
        { "BARCODE_CODE39", BARCODE_CODE39, 8 },
        { "BARCODE_EXCODE39", BARCODE_EXCODE39, 9 },
        { "", -1, 10 },
        { "", -1, 11 },
        { "", -1, 12 },
        { "BARCODE_EANX", BARCODE_EANX, 13 },
        { "BARCODE_EANX_CHK", BARCODE_EANX_CHK, 14 },
        { "", -1, 15 },
        { "BARCODE_GS1_128", BARCODE_GS1_128, 16 },
        { "", -1, 17 },
        { "BARCODE_CODABAR", BARCODE_CODABAR, 18 },
        { "", -1, 19 },
        { "BARCODE_CODE128", BARCODE_CODE128, 20 },
        { "BARCODE_DPLEIT", BARCODE_DPLEIT, 21 },
        { "BARCODE_DPIDENT", BARCODE_DPIDENT, 22 },
        { "BARCODE_CODE16K", BARCODE_CODE16K, 23 },
        { "BARCODE_CODE49", BARCODE_CODE49, 24 },
        { "BARCODE_CODE93", BARCODE_CODE93, 25 },
        { "", -1, 26 },
        { "", -1, 27 },
        { "BARCODE_FLAT", BARCODE_FLAT, 28 },
        { "BARCODE_DBAR_OMN", BARCODE_DBAR_OMN, 29 },
        { "BARCODE_DBAR_LTD", BARCODE_DBAR_LTD, 30 },
        { "BARCODE_DBAR_EXP", BARCODE_DBAR_EXP, 31 },
        { "BARCODE_TELEPEN", BARCODE_TELEPEN, 32 },
        { "", -1, 33 },
        { "BARCODE_UPCA", BARCODE_UPCA, 34 },
        { "BARCODE_UPCA_CHK", BARCODE_UPCA_CHK, 35 },
        { "", -1, 36 },
        { "BARCODE_UPCE", BARCODE_UPCE, 37 },
        { "BARCODE_UPCE_CHK", BARCODE_UPCE_CHK, 38 },
        { "", -1, 39 },
        { "BARCODE_POSTNET", BARCODE_POSTNET, 40 },
        { "", -1, 41 },
        { "", -1, 42 },
        { "", -1, 43 },
        { "", -1, 44 },
        { "", -1, 45 },
        { "", -1, 46 },
        { "BARCODE_MSI_PLESSEY", BARCODE_MSI_PLESSEY, 47 },
        { "", -1, 48 },
        { "BARCODE_FIM", BARCODE_FIM, 49 },
        { "BARCODE_LOGMARS", BARCODE_LOGMARS, 50 },
        { "BARCODE_PHARMA", BARCODE_PHARMA, 51 },
        { "BARCODE_PZN", BARCODE_PZN, 52 },
        { "BARCODE_PHARMA_TWO", BARCODE_PHARMA_TWO, 53 },
        { "BARCODE_CEPNET", BARCODE_CEPNET, 54 },
        { "BARCODE_PDF417", BARCODE_PDF417, 55 },
        { "BARCODE_PDF417COMP", BARCODE_PDF417COMP, 56 },
        { "BARCODE_MAXICODE", BARCODE_MAXICODE, 57 },
        { "BARCODE_QRCODE", BARCODE_QRCODE, 58 },
        { "", -1, 59 },
        { "BARCODE_CODE128AB", BARCODE_CODE128AB, 60 },
        { "", -1, 61 },
        { "", -1, 62 },
        { "BARCODE_AUSPOST", BARCODE_AUSPOST, 63 },
        { "", -1, 64 },
        { "", -1, 65 },
        { "BARCODE_AUSREPLY", BARCODE_AUSREPLY, 66 },
        { "BARCODE_AUSROUTE", BARCODE_AUSROUTE, 67 },
        { "BARCODE_AUSREDIRECT", BARCODE_AUSREDIRECT, 68 },
        { "BARCODE_ISBNX", BARCODE_ISBNX, 69 },
        { "BARCODE_RM4SCC", BARCODE_RM4SCC, 70 },
        { "BARCODE_DATAMATRIX", BARCODE_DATAMATRIX, 71 },
        { "BARCODE_EAN14", BARCODE_EAN14, 72 },
        { "BARCODE_VIN", BARCODE_VIN, 73 },
        { "BARCODE_CODABLOCKF", BARCODE_CODABLOCKF, 74 },
        { "BARCODE_NVE18", BARCODE_NVE18, 75 },
        { "BARCODE_JAPANPOST", BARCODE_JAPANPOST, 76 },
        { "BARCODE_KOREAPOST", BARCODE_KOREAPOST, 77 },
        { "", -1, 78 },
        { "BARCODE_DBAR_STK", BARCODE_DBAR_STK, 79 },
        { "BARCODE_DBAR_OMNSTK", BARCODE_DBAR_OMNSTK, 80 },
        { "BARCODE_DBAR_EXPSTK", BARCODE_DBAR_EXPSTK, 81 },
        { "BARCODE_PLANET", BARCODE_PLANET, 82 },
        { "", -1, 83 },
        { "BARCODE_MICROPDF417", BARCODE_MICROPDF417, 84 },
        { "BARCODE_USPS_IMAIL", BARCODE_USPS_IMAIL, 85 },
        { "BARCODE_PLESSEY", BARCODE_PLESSEY, 86 },
        { "BARCODE_TELEPEN_NUM", BARCODE_TELEPEN_NUM, 87 },
        { "", -1, 88 },
        { "BARCODE_ITF14", BARCODE_ITF14, 89 },
        { "BARCODE_KIX", BARCODE_KIX, 90 },
        { "", -1, 91 },
        { "BARCODE_AZTEC", BARCODE_AZTEC, 92 },
        { "BARCODE_DAFT", BARCODE_DAFT, 93 },
        { "", -1, 94 },
        { "", -1, 95 },
        { "BARCODE_DPD", BARCODE_DPD, 96 },
        { "BARCODE_MICROQR", BARCODE_MICROQR, 97 },
        { "BARCODE_HIBC_128", BARCODE_HIBC_128, 98 },
        { "BARCODE_HIBC_39", BARCODE_HIBC_39, 99 },
        { "", -1, 100 },
        { "", -1, 101 },
        { "BARCODE_HIBC_DM", BARCODE_HIBC_DM, 102 },
        { "", -1, 103 },
        { "BARCODE_HIBC_QR", BARCODE_HIBC_QR, 104 },
        { "", -1, 105 },
        { "BARCODE_HIBC_PDF", BARCODE_HIBC_PDF, 106 },
        { "", -1, 107 },
        { "BARCODE_HIBC_MICPDF", BARCODE_HIBC_MICPDF, 108 },
        { "", -1, 109 },
        { "BARCODE_HIBC_BLOCKF", BARCODE_HIBC_BLOCKF, 110 },
        { "", -1, 111 },
        { "BARCODE_HIBC_AZTEC", BARCODE_HIBC_AZTEC, 112 },
        { "", -1, 113 },
        { "", -1, 114 },
        { "BARCODE_DOTCODE", BARCODE_DOTCODE, 115 },
        { "BARCODE_HANXIN", BARCODE_HANXIN, 116 },
        { "", -1, 117 },
        { "", -1, 118 },
        { "BARCODE_MAILMARK_2D", BARCODE_MAILMARK_2D, 119 },
        { "BARCODE_UPU_S10", BARCODE_UPU_S10, 120 },
        { "BARCODE_MAILMARK_4S", BARCODE_MAILMARK_4S, 121 },
        { "", -1, 122 },
        { "", -1, 123 },
        { "", -1, 124 },
        { "", -1, 125 },
        { "", -1, 126 },
        { "", -1, 127 },
        { "BARCODE_AZRUNE", BARCODE_AZRUNE, 128 },
        { "BARCODE_CODE32", BARCODE_CODE32, 129 },
        { "BARCODE_EANX_CC", BARCODE_EANX_CC, 130 },
        { "BARCODE_GS1_128_CC", BARCODE_GS1_128_CC, 131 },
        { "BARCODE_DBAR_OMN_CC", BARCODE_DBAR_OMN_CC, 132 },
        { "BARCODE_DBAR_LTD_CC", BARCODE_DBAR_LTD_CC, 133 },
        { "BARCODE_DBAR_EXP_CC", BARCODE_DBAR_EXP_CC, 134 },
        { "BARCODE_UPCA_CC", BARCODE_UPCA_CC, 135 },
        { "BARCODE_UPCE_CC", BARCODE_UPCE_CC, 136 },
        { "BARCODE_DBAR_STK_CC", BARCODE_DBAR_STK_CC, 137 },
        { "BARCODE_DBAR_OMNSTK_CC", BARCODE_DBAR_OMNSTK_CC, 138 },
        { "BARCODE_DBAR_EXPSTK_CC", BARCODE_DBAR_EXPSTK_CC, 139 },
        { "BARCODE_CHANNEL", BARCODE_CHANNEL, 140 },
        { "BARCODE_CODEONE", BARCODE_CODEONE, 141 },
        { "BARCODE_GRIDMATRIX", BARCODE_GRIDMATRIX, 142 },
        { "BARCODE_UPNQR", BARCODE_UPNQR, 143 },
        { "BARCODE_ULTRA", BARCODE_ULTRA, 144 },
        { "BARCODE_RMQR", BARCODE_RMQR, 145 },
        { "BARCODE_BC412", BARCODE_BC412, 146 },
        { "BARCODE_DXFILMEDGE", BARCODE_DXFILMEDGE, 147 },
    };

    name[0] = '\0';

    if (!ZBarcode_ValidID(symbol_id)) {
        return 1;
    }
    if (!(symbol_id >= 0 && symbol_id < ARRAY_SIZE(data) && data[symbol_id].name[0])) {
        return -1; /* Shouldn't happen */
    }

    /* Self-check, shouldn't happen */
    if (data[symbol_id].val != symbol_id || (data[symbol_id].define != -1 && data[symbol_id].define != symbol_id)) {
        return -1;
    }

    strcpy(name, data[symbol_id].name);

    return 0;
}

static void test_barcode_name(const testCtx *const p_ctx) {

    int ret;
    char name[32];
    int symbol_id;

    (void)p_ctx;

    testStart("test_barcode_name");

    for (symbol_id = -1; symbol_id < 160; symbol_id++) {
        int prev_ret;
        char prev_name[32];
        ret = ZBarcode_BarcodeName(symbol_id, name);
        if (ZBarcode_ValidID(symbol_id)) {
            assert_equal(ret, 0, "ZBarcode_BarcodeName(%d) != 0\n", symbol_id);
            assert_nonzero(*name != '\0', "ZBarcode_BarcodeName(%d) empty when ZBarcode_Valid() true\n", symbol_id);
        } else {
            assert_equal(ret, 1, "ZBarcode_BarcodeName(%d) != 1\n", symbol_id);
            assert_zero(*name, "ZBarcode_BarcodeName(%d) non-empty when ZBarcode_Valid() false\n", symbol_id);
        }
        prev_ret = test_prev_ZBarcode_BarcodeName(symbol_id, prev_name);
        assert_equal(ret, prev_ret, "ZBarcode_BarcodeName(%d) ret %d != prev_ret %d\n", symbol_id, ret, prev_ret);
        assert_zero(strcmp(name, prev_name), "ZBarcode_BarcodeName(%d) strcmp(%s, %s) != 0\n", symbol_id, name, prev_name);
    }

    testFinish();
}

INTERNAL int error_tag_test(int error_number, struct zint_symbol *symbol, const int err_id, const char *error_string);

static void test_error_tag(const testCtx *const p_ctx) {

    struct item {
        int debug_test;
        int error_number;
        int warn_level;
        char *data;
        int ret;
        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { 0, ZINT_WARN_INVALID_OPTION, -1, "", ZINT_WARN_INVALID_OPTION, "Warning " },
        /*  1*/ { 0, ZINT_WARN_INVALID_OPTION, WARN_FAIL_ALL, "", ZINT_ERROR_INVALID_OPTION, "Error " },
        /*  2*/ { 0, ZINT_WARN_USES_ECI, -1, "", ZINT_WARN_USES_ECI, "Warning " },
        /*  3*/ { 0, ZINT_WARN_USES_ECI, WARN_FAIL_ALL, "", ZINT_ERROR_USES_ECI, "Error " },
        /*  4*/ { 0, ZINT_WARN_NONCOMPLIANT, -1, "", ZINT_WARN_NONCOMPLIANT, "Warning " },
        /*  5*/ { 0, ZINT_WARN_NONCOMPLIANT, WARN_FAIL_ALL, "", ZINT_ERROR_NONCOMPLIANT, "Error " },
        /*  6*/ { 0, ZINT_WARN_HRT_TRUNCATED, -1, "", ZINT_WARN_HRT_TRUNCATED, "Warning " },
        /*  7*/ { 0, ZINT_WARN_HRT_TRUNCATED, WARN_FAIL_ALL, "", ZINT_ERROR_HRT_TRUNCATED, "Error " },
        /*  8*/ { 0, ZINT_ERROR_TOO_LONG, WARN_DEFAULT, "", ZINT_ERROR_TOO_LONG, "Error " },
        /*  9*/ { 0, ZINT_ERROR_TOO_LONG, WARN_FAIL_ALL, "", ZINT_ERROR_TOO_LONG, "Error " },
        /* 10*/ { 0, ZINT_WARN_USES_ECI, -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_WARN_USES_ECI, "Warning 1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901" },
        /* 11*/ { 1, ZINT_WARN_USES_ECI, -1, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012", ZINT_WARN_USES_ECI, "Warning 1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901" },
        /* 12*/ { 0, ZINT_WARN_USES_ECI, WARN_FAIL_ALL, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123", ZINT_ERROR_USES_ECI, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
        /* 13*/ { 1, ZINT_WARN_USES_ECI, WARN_FAIL_ALL, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", ZINT_ERROR_USES_ECI, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
        /* 14*/ { 0, ZINT_ERROR_INVALID_DATA, -1, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123", ZINT_ERROR_INVALID_DATA, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
        /* 15*/ { 1, ZINT_ERROR_INVALID_DATA, -1, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", ZINT_ERROR_INVALID_DATA, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
        /* 16*/ { 1, ZINT_ERROR_INVALID_DATA, WARN_FAIL_ALL, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", ZINT_ERROR_INVALID_DATA, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
        /* 17*/ { 1, ZINT_WARN_USES_ECI, -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", ZINT_WARN_USES_ECI, "Warning 1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901" },
        /* 18*/ { 1, ZINT_WARN_USES_ECI, WARN_FAIL_ALL, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", ZINT_ERROR_USES_ECI, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
        /* 19*/ { 1, ZINT_ERROR_INVALID_DATA, -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", ZINT_ERROR_INVALID_DATA, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, ret;

    testStart("test_error_tag");

    for (i = 0; i < data_size; i++) {
        struct zint_symbol s_symbol = {0};
        struct zint_symbol *symbol = &s_symbol;

        if (testContinue(p_ctx, i)) continue;

        if (data[i].debug_test) symbol->debug |= ZINT_DEBUG_TEST;
        symbol->warn_level = data[i].warn_level;

        ret = error_tag_test(data[i].error_number, symbol, -1, data[i].data);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

        if ((int) strlen(data[i].data) < 100) {
            strcpy(symbol->errtxt, data[i].data);
            ret = error_tag_test(data[i].error_number, symbol, -1, NULL);
            assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }
    }

    testFinish();
}

INTERNAL void strip_bom_test(unsigned char *source, int *input_length);

static void test_strip_bom(const testCtx *const p_ctx) {

    int length, ret;
    char data[] = "\357\273\277A"; /* U+FEFF BOM, with "A" */
    char bom_only[] = "\357\273\277"; /* U+FEFF BOM only */
    char buf[6];

    (void)p_ctx;

    testStart("test_strip_bom");

    strcpy(buf, data);
    length = (int) strlen(buf);
    strip_bom_test(TU(buf), &length);
    assert_equal(length, 1, "length %d != 1\n", length);
    assert_zero(buf[1], "buf[1] %d != 0\n", buf[1]);

    /* BOM not stripped if only data */

    strcpy(buf, bom_only);
    length = (int) strlen(buf);
    strip_bom_test(TU(buf), &length);
    assert_equal(length, 3, "BOM only length %d != 3\n", length);
    ret = strcmp(buf, bom_only);
    assert_zero(ret, "BOM only strcmp ret %d != 0\n", ret);

    testFinish();
}

static void test_zero_outfile(const testCtx *const p_ctx) {

    int ret;
    struct zint_symbol *symbol = NULL;
    char *data = "1234";

    (void)p_ctx;

    testStartSymbol("test_zero_outfile", &symbol);

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    assert_nonzero(symbol->outfile[0], "ZBarcode_Create() outfile zero\n");
    symbol->outfile[0] = '\0';

    ret = ZBarcode_Encode(symbol, TU(data), 0);
    assert_zero(ret, "ZBarcode_Encode(%s) ret %d != 0 (%s)\n", data, ret, symbol->errtxt);
    assert_zero(symbol->outfile[0], "ZBarcode_Encode() outfile non-zero\n");

    ret = ZBarcode_Print(symbol, 0);
    assert_equal(ret, ZINT_ERROR_INVALID_OPTION, "ZBarcode_Print() ret %d != ZINT_ERROR_INVALID_OPTION (%s)\n", ret, symbol->errtxt);
    assert_zero(symbol->outfile[0], "ZBarcode_Print() outfile non-zero\n");

    ret = ZBarcode_Buffer(symbol, 0);
    assert_zero(ret, "ZBarcode_Buffer() ret %d != 0 (%s)\n", ret, symbol->errtxt);
    assert_zero(symbol->outfile[0], "ZBarcode_Buffer() outfile non-zero\n");

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_clear(const testCtx *const p_ctx) {

    int ret;
    struct zint_symbol *symbol = NULL;
    char *data = "1234";

    (void)p_ctx;

    testStartSymbol("test_clear", &symbol);

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    symbol->symbology = BARCODE_MAXICODE;
    symbol->whitespace_width = 5;
    symbol->whitespace_height = 5;
    symbol->border_width = 2;
    symbol->output_options = BARCODE_BOX;
    strcpy(symbol->fgcolour, "000000AA");
    strcpy(symbol->bgcolour, "FFFFFFAA");

    /* Raster */

    ret = ZBarcode_Encode(symbol, TU(data), 0);
    assert_zero(ret, "ZBarcode_Encode() ret %d != 0 (%s)\n", ret, symbol->errtxt);

    assert_nonzero(symbol->rows, "ZBarcode_Encode() rows 0\n");
    assert_nonzero(symbol->width, "ZBarcode_Encode() width 0\n");

    ret = ZBarcode_Buffer(symbol, 0);
    assert_zero(ret, "ZBarcode_Buffer() ret %d != 0 (%s)\n", ret, symbol->errtxt);

    assert_nonnull(symbol->bitmap, "ZBarcode_Buffer() bitmap NULL\n");
    assert_nonnull(symbol->alphamap, "ZBarcode_Buffer() alphamap NULL\n");
    assert_nonzero(symbol->bitmap_width, "ZBarcode_Buffer() bitmap_width 0\n");
    assert_nonzero(symbol->bitmap_height, "ZBarcode_Buffer() bitmap_height 0\n");

    assert_nonzero(symbol->rows, "ZBarcode_Buffer() rows 0\n");
    assert_nonzero(symbol->width, "ZBarcode_Buffer() width 0\n");
    assert_null(symbol->vector, "ZBarcode_Buffer() vector != NULL\n");

    ZBarcode_Clear(symbol);

    assert_null(symbol->bitmap, "ZBarcode_Buffer() bitmap != NULL\n");
    assert_null(symbol->alphamap, "ZBarcode_Buffer() alphamap != NULL\n");
    assert_zero(symbol->bitmap_width, "ZBarcode_Buffer() bitmap_width %d != 0\n", symbol->bitmap_width);
    assert_zero(symbol->bitmap_height, "ZBarcode_Buffer() bitmap_height %d != 0\n", symbol->bitmap_height);

    assert_zero(symbol->rows, "ZBarcode_Buffer() rows %d != 0\n", symbol->rows);
    assert_zero(symbol->width, "ZBarcode_Buffer() width %d != 0\n", symbol->width);
    assert_null(symbol->vector, "ZBarcode_Buffer() vector != NULL\n");

    /* Vector */

    ret = ZBarcode_Encode(symbol, TU(data), 0);
    assert_zero(ret, "ZBarcode_Encode() ret %d != 0 (%s)\n", ret, symbol->errtxt);

    assert_nonzero(symbol->rows, "ZBarcode_Encode() rows 0\n");
    assert_nonzero(symbol->width, "ZBarcode_Encode() width 0\n");

    ret = ZBarcode_Buffer_Vector(symbol, 0);
    assert_zero(ret, "ZBarcode_Buffer_Vector() ret %d != 0 (%s)\n", ret, symbol->errtxt);

    assert_nonnull(symbol->vector, "ZBarcode_Buffer_Vector() vector NULL\n");
    assert_nonnull(symbol->vector->rectangles, "ZBarcode_Buffer_Vector() vector->rectangles NULL\n");
    assert_nonnull(symbol->vector->hexagons, "ZBarcode_Buffer_Vector() vector->hexagons NULL\n");
    assert_null(symbol->vector->strings, "ZBarcode_Buffer_Vector() vector->strings != NULL\n"); /* MAXICODE no text */
    assert_nonnull(symbol->vector->circles, "ZBarcode_Buffer_Vector() vector->circles NULL\n");

    assert_nonzero(symbol->rows, "ZBarcode_Buffer_Vector() rows 0\n");
    assert_nonzero(symbol->width, "ZBarcode_Buffer_Vector() width 0\n");
    assert_null(symbol->bitmap, "ZBarcode_Buffer_Vector() bitmap != NULL\n");
    assert_null(symbol->alphamap, "ZBarcode_Buffer_Vector() alphamap != NULL\n");

    ZBarcode_Clear(symbol);

    assert_null(symbol->vector, "ZBarcode_Buffer_Vector() vector != NULL\n");

    assert_zero(symbol->rows, "ZBarcode_Buffer_Vector() rows %d != 0\n", symbol->rows);
    assert_zero(symbol->width, "ZBarcode_Buffer_Vector() width %d != 0\n", symbol->width);
    assert_null(symbol->bitmap, "ZBarcode_Buffer_Vector() bitmap != NULL\n");
    assert_null(symbol->alphamap, "ZBarcode_Buffer_Vector() alphamap != NULL\n");
    assert_zero(symbol->bitmap_width, "ZBarcode_Buffer_Vector() bitmap_width %d != 0\n", symbol->bitmap_width);
    assert_zero(symbol->bitmap_height, "ZBarcode_Buffer_Vector() bitmap_height %d != 0\n", symbol->bitmap_height);

    ZBarcode_Delete(symbol);

    testFinish();
}

/* Helper to set various `zint_symbol` fields */
static void set_symbol_fields(struct zint_symbol *symbol) {
    symbol->symbology = BARCODE_MAXICODE;
    symbol->height = 2.3f;
    symbol->scale = 1.1f;
    symbol->whitespace_width = 5;
    symbol->whitespace_height = 5;
    symbol->border_width = 2;
    symbol->output_options = BARCODE_BOX;
    strcpy(symbol->fgcolour, "000000AA");
    strcpy(symbol->bgcolour, "FFFFFFAA");
    strcpy(symbol->outfile, "gosh.png");
    strcpy(symbol->primary, "1234567");
    symbol->option_1 = 2;
    symbol->option_2 = 3;
    symbol->option_3 = 4;
    symbol->show_hrt = 0;
    symbol->input_mode = UNICODE_MODE | ESCAPE_MODE;
    symbol->eci = 3;
    symbol->dpmm = 24.0f;
    symbol->dot_size = 1.6f;
    symbol->text_gap = 0.5f;
    symbol->guard_descent = 4.5f;
    symbol->structapp.index = 1;
    symbol->structapp.count = 2;
    /*strcpy(symbol->structapp.id, "ID1");*/ /* Not available for MAXICODE */
    symbol->warn_level = WARN_FAIL_ALL;
}

static void test_reset(const testCtx *const p_ctx) {

    int ret;
    struct zint_symbol *symbol;
    struct zint_symbol *symbol_def;
    char *data = "1234";

    (void)p_ctx;

    testStart("test_reset");

    symbol_def = ZBarcode_Create();
    assert_nonnull(symbol_def, "Default symbol not created\n");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    set_symbol_fields(symbol);

    /* Raster */

    ret = ZBarcode_Encode(symbol, TU(data), 0);
    assert_zero(ret, "ZBarcode_Encode() ret %d != 0 (%s)\n", ret, symbol->errtxt);

    assert_nonzero(symbol->rows, "ZBarcode_Encode() rows 0\n");
    assert_nonzero(symbol->width, "ZBarcode_Encode() width 0\n");

    ret = ZBarcode_Buffer(symbol, 0);
    assert_zero(ret, "ZBarcode_Buffer() ret %d != 0 (%s)\n", ret, symbol->errtxt);

    assert_nonnull(symbol->bitmap, "ZBarcode_Buffer() bitmap NULL\n");
    assert_nonnull(symbol->alphamap, "ZBarcode_Buffer() alphamap NULL\n");
    assert_nonzero(symbol->bitmap_width, "ZBarcode_Buffer() bitmap_width 0\n");
    assert_nonzero(symbol->bitmap_height, "ZBarcode_Buffer() bitmap_height 0\n");

    assert_nonzero(symbol->rows, "ZBarcode_Buffer() rows 0\n");
    assert_nonzero(symbol->width, "ZBarcode_Buffer() width 0\n");
    assert_null(symbol->vector, "ZBarcode_Buffer() vector != NULL\n");

    ZBarcode_Reset(symbol);

    assert_nonzero(symbol->symbology == symbol_def->symbology, "ZBarcodeBuffer symbology != symbol_def\n");
    assert_nonzero(symbol->height == symbol_def->height, "ZBarcodeBuffer height != symbol_def\n");
    assert_nonzero(symbol->scale == symbol_def->scale, "ZBarcodeBuffer scale != symbol_def\n");
    assert_nonzero(symbol->whitespace_width == symbol_def->whitespace_width, "ZBarcodeBuffer whitespace_width != symbol_def\n");
    assert_nonzero(symbol->whitespace_height == symbol_def->whitespace_height, "ZBarcodeBuffer whitespace_height != symbol_def\n");
    assert_nonzero(symbol->border_width == symbol_def->border_width, "ZBarcodeBuffer border_width != symbol_def\n");
    assert_nonzero(symbol->output_options == symbol_def->output_options, "ZBarcodeBuffer output_options != symbol_def\n");
    assert_zero(strcmp(symbol->fgcolour, symbol_def->fgcolour), "ZBarcodeBuffer fgcolour != symbol_def\n");
    assert_zero(strcmp(symbol->bgcolour, symbol_def->bgcolour), "ZBarcodeBuffer bgcolour != symbol_def\n");
    assert_zero(strcmp(symbol->fgcolor, symbol_def->fgcolor), "ZBarcodeBuffer fgcolor != symbol_def\n");
    assert_zero(strcmp(symbol->bgcolor, symbol_def->bgcolor), "ZBarcodeBuffer bgcolor != symbol_def\n");
    assert_zero(strcmp(symbol->outfile, symbol_def->outfile), "ZBarcodeBuffer outfile != symbol_def\n");
    assert_zero(strcmp(symbol->primary, symbol_def->primary), "ZBarcodeBuffer primary != symbol_def\n");
    assert_nonzero(symbol->option_1 == symbol_def->option_1, "ZBarcodeBuffer option_1 != symbol_def\n");
    assert_nonzero(symbol->option_2 == symbol_def->option_2, "ZBarcodeBuffer option_2 != symbol_def\n");
    assert_nonzero(symbol->option_3 == symbol_def->option_3, "ZBarcodeBuffer option_3 != symbol_def\n");
    assert_nonzero(symbol->show_hrt == symbol_def->show_hrt, "ZBarcodeBuffer show_hrt != symbol_def\n");
    assert_nonzero(symbol->input_mode == symbol_def->input_mode, "ZBarcodeBuffer input_mode != symbol_def\n");
    assert_nonzero(symbol->eci == symbol_def->eci, "ZBarcodeBuffer eci != symbol_def\n");
    assert_nonzero(symbol->dpmm == symbol_def->dpmm, "ZBarcodeBuffer dpmm != symbol_def\n");
    assert_nonzero(symbol->dot_size == symbol_def->dot_size, "ZBarcodeBuffer dot_size != symbol_def\n");
    assert_nonzero(symbol->text_gap == symbol_def->text_gap, "ZBarcodeBuffer text_gap != symbol_def\n");
    assert_nonzero(symbol->guard_descent == symbol_def->guard_descent, "ZBarcodeBuffer guard_descent != symbol_def\n");
    assert_nonzero(symbol->structapp.index == symbol_def->structapp.index, "ZBarcodeBuffer structapp.index != symbol_def\n");
    assert_nonzero(symbol->structapp.count == symbol_def->structapp.count, "ZBarcodeBuffer structapp.count != symbol_def\n");
    assert_nonzero(symbol->warn_level == symbol_def->warn_level, "ZBarcodeBuffer warn_level != symbol_def\n");

    /* Vector */

    set_symbol_fields(symbol);

    ret = ZBarcode_Encode(symbol, TU(data), 0);
    assert_zero(ret, "ZBarcode_Encode() ret %d != 0 (%s)\n", ret, symbol->errtxt);

    assert_nonzero(symbol->rows, "ZBarcode_Encode() rows 0\n");
    assert_nonzero(symbol->width, "ZBarcode_Encode() width 0\n");

    ret = ZBarcode_Buffer_Vector(symbol, 0);
    assert_zero(ret, "ZBarcode_Buffer_Vector() ret %d != 0 (%s)\n", ret, symbol->errtxt);

    assert_nonnull(symbol->vector, "ZBarcode_Buffer_Vector() vector NULL\n");
    assert_nonnull(symbol->vector->rectangles, "ZBarcode_Buffer_Vector() vector->rectangles NULL\n");
    assert_nonnull(symbol->vector->hexagons, "ZBarcode_Buffer_Vector() vector->hexagons NULL\n");
    assert_null(symbol->vector->strings, "ZBarcode_Buffer_Vector() vector->strings != NULL\n"); /* MAXICODE no text */
    assert_nonnull(symbol->vector->circles, "ZBarcode_Buffer_Vector() vector->circles NULL\n");

    assert_nonzero(symbol->rows, "ZBarcode_Buffer_Vector() rows 0\n");
    assert_nonzero(symbol->width, "ZBarcode_Buffer_Vector() width 0\n");
    assert_null(symbol->bitmap, "ZBarcode_Buffer_Vector() bitmap != NULL\n");
    assert_null(symbol->alphamap, "ZBarcode_Buffer_Vector() alphamap != NULL\n");

    ZBarcode_Reset(symbol);

    assert_nonzero(symbol->symbology == symbol_def->symbology, "ZBarcodeBuffer symbology != symbol_def\n");
    assert_nonzero(symbol->height == symbol_def->height, "ZBarcodeBuffer height != symbol_def\n");
    assert_nonzero(symbol->scale == symbol_def->scale, "ZBarcodeBuffer scale != symbol_def\n");
    assert_nonzero(symbol->whitespace_width == symbol_def->whitespace_width, "ZBarcodeBuffer whitespace_width != symbol_def\n");
    assert_nonzero(symbol->whitespace_height == symbol_def->whitespace_height, "ZBarcodeBuffer whitespace_height != symbol_def\n");
    assert_nonzero(symbol->border_width == symbol_def->border_width, "ZBarcodeBuffer border_width != symbol_def\n");
    assert_nonzero(symbol->output_options == symbol_def->output_options, "ZBarcodeBuffer output_options != symbol_def\n");
    assert_zero(strcmp(symbol->fgcolour, symbol_def->fgcolour), "ZBarcodeBuffer fgcolour != symbol_def\n");
    assert_zero(strcmp(symbol->bgcolour, symbol_def->bgcolour), "ZBarcodeBuffer bgcolour != symbol_def\n");
    assert_zero(strcmp(symbol->fgcolor, symbol_def->fgcolor), "ZBarcodeBuffer fgcolor != symbol_def\n");
    assert_zero(strcmp(symbol->bgcolor, symbol_def->bgcolor), "ZBarcodeBuffer bgcolor != symbol_def\n");
    assert_zero(strcmp(symbol->outfile, symbol_def->outfile), "ZBarcodeBuffer outfile != symbol_def\n");
    assert_zero(strcmp(symbol->primary, symbol_def->primary), "ZBarcodeBuffer primary != symbol_def\n");
    assert_nonzero(symbol->option_1 == symbol_def->option_1, "ZBarcodeBuffer option_1 != symbol_def\n");
    assert_nonzero(symbol->option_2 == symbol_def->option_2, "ZBarcodeBuffer option_2 != symbol_def\n");
    assert_nonzero(symbol->option_3 == symbol_def->option_3, "ZBarcodeBuffer option_3 != symbol_def\n");
    assert_nonzero(symbol->show_hrt == symbol_def->show_hrt, "ZBarcodeBuffer show_hrt != symbol_def\n");
    assert_nonzero(symbol->input_mode == symbol_def->input_mode, "ZBarcodeBuffer input_mode != symbol_def\n");
    assert_nonzero(symbol->eci == symbol_def->eci, "ZBarcodeBuffer eci != symbol_def\n");
    assert_nonzero(symbol->dpmm == symbol_def->dpmm, "ZBarcodeBuffer dpmm != symbol_def\n");
    assert_nonzero(symbol->dot_size == symbol_def->dot_size, "ZBarcodeBuffer dot_size != symbol_def\n");
    assert_nonzero(symbol->text_gap == symbol_def->text_gap, "ZBarcodeBuffer text_gap != symbol_def\n");
    assert_nonzero(symbol->guard_descent == symbol_def->guard_descent, "ZBarcodeBuffer guard_descent != symbol_def\n");
    assert_nonzero(symbol->structapp.index == symbol_def->structapp.index, "ZBarcodeBuffer structapp.index != symbol_def\n");
    assert_nonzero(symbol->structapp.count == symbol_def->structapp.count, "ZBarcodeBuffer structapp.count != symbol_def\n");
    assert_nonzero(symbol->warn_level == symbol_def->warn_level, "ZBarcodeBuffer warn_level != symbol_def\n");

    ZBarcode_Delete(symbol);
    ZBarcode_Delete(symbol_def);

    testFinish();
}

static void test_scale_from_xdimdp(const testCtx *const p_ctx) {

    struct item {
        int symbology;
        float x_dim;
        float dpmm;
        int dpi;
        char *filetype;
        float expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_EANX, 0.33f, 2, 50, "gif", 0.5f },
        /*  1*/ { BARCODE_EANX, 0.33f, 2, 50, "emf", 0.33000001f },
        /*  2*/ { BARCODE_EANX, 0.33f, 2, 50, "svg", 0.33000001f },
        /*  3*/ { BARCODE_EANX, 0.33f, 3, 76, "gif", 0.5f },
        /*  4*/ { BARCODE_EANX, 0.33f, 3, 76, "svg", 0.495f },
        /*  5*/ { BARCODE_EANX, 0.33f, 4, 100, "gif", 0.5f },
        /*  6*/ { BARCODE_EANX, 0.33f, 4, 100, "svg", 0.66000003f },
        /*  7*/ { BARCODE_EANX, 0.33f, 6, 150, "gif", 1 },
        /*  8*/ { BARCODE_EANX, 0.33f, 6, 150, "svg", 0.99f },
        /*  9*/ { BARCODE_EANX, 0.33f, 8, 200, "gif", 1.5f },
        /* 10*/ { BARCODE_EANX, 0.33f, 8, 200, "svg", 1.32f },
        /* 11*/ { BARCODE_EANX, 0.33f, 12, 300, "gif", 2 },
        /* 12*/ { BARCODE_EANX, 0.33f, 12, 300, "svg", 1.98f },
        /* 13*/ { BARCODE_EANX, 0.33f, 16, 400, "gif", 2.5f }, /* NOTE: scale previously documented as 3.0f */
        /* 14*/ { BARCODE_EANX, 0.33f, 16, 400, "svg", 2.64f },
        /* 15*/ { BARCODE_EANX, 0.33f, 24, 600, "gif", 4 },
        /* 16*/ { BARCODE_EANX, 0.33f, 24, 600, "svg", 3.96f },
        /* 17*/ { BARCODE_EANX, 0.33f, 47, 1200, "gif", 8 },
        /* 18*/ { BARCODE_EANX, 0.33f, 47, 1200, "emf", 7.755f },
        /* 19*/ { BARCODE_EANX, 0.33f, 47, 1200, "svg", 7.755f },
        /* 20*/ { BARCODE_EANX, 0.33f, 94, 2400, "gif", 15.5f }, /* NOTE dpmm previously documented as 95 */
        /* 21*/ { BARCODE_EANX, 0.33f, 94, 2400, "svg", 15.51f },
        /* 22*/ { BARCODE_EANX, 0.33f, 189, 4800, "gif", 31 },
        /* 23*/ { BARCODE_EANX, 0.33f, 189, 4800, "svg", 31.185001f },
        /* 24*/ { BARCODE_EANX, 0.33f, 378, 9600, "gif", 62.5f },
        /* 25*/ { BARCODE_EANX, 0.33f, 378, 9600, "svg", 62.370003f },
        /* 26*/ { BARCODE_MAXICODE, 0.88f, 4, 100, "gif", 0.352f },
        /* 27*/ { BARCODE_MAXICODE, 0.88f, 4, 100, "emf", 0.1f }, /* NOTE scale rounded up to min 0.1 so doesn't round trip */
        /* 28*/ { BARCODE_MAXICODE, 0.88f, 4, 100, "svg", 1.76f },
        /* 29*/ { BARCODE_MAXICODE, 0.88f, 6, 150, "gif", 0.528f }, /* NOTE scale previously documented as 0.5f */
        /* 30*/ { BARCODE_MAXICODE, 0.88f, 6, 150, "emf", 0.132f },
        /* 31*/ { BARCODE_MAXICODE, 0.88f, 6, 150, "svg", 2.6399999 },
        /* 32*/ { BARCODE_MAXICODE, 0.88f, 8, 200, "gif", 0.704f }, /* NOTE scale previously documented as 0.7f */
        /* 33*/ { BARCODE_MAXICODE, 0.88f, 8, 200, "emf", 0.176f },
        /* 34*/ { BARCODE_MAXICODE, 0.88f, 8, 200, "svg", 3.52f },
        /* 35*/ { BARCODE_MAXICODE, 0.88f, 12, 300, "gif", 1.056f }, /* NOTE scale previously documented as 1.0f */
        /* 36*/ { BARCODE_MAXICODE, 0.88f, 12, 300, "emf", 0.264f },
        /* 37*/ { BARCODE_MAXICODE, 0.88f, 12, 300, "svg", 5.2799997f },
        /* 38*/ { BARCODE_MAXICODE, 0.88f, 16, 400, "gif", 1.408f }, /* NOTE scale previously documented as 1.4f */
        /* 39*/ { BARCODE_MAXICODE, 0.88f, 16, 400, "emf", 0.352f },
        /* 40*/ { BARCODE_MAXICODE, 0.88f, 16, 400, "gif", 1.408f },
        /* 41*/ { BARCODE_MAXICODE, 0.88f, 24, 600, "gif", 2.112f }, /* NOTE scale previously documented as 2.1f */
        /* 42*/ { BARCODE_MAXICODE, 0.88f, 24, 600, "emf", 0.528f },
        /* 43*/ { BARCODE_MAXICODE, 0.88f, 24, 600, "svg", 10.559999f },
        /* 44*/ { BARCODE_MAXICODE, 0.88f, 47, 1200, "gif", 4.136f }, /* NOTE scale previously documented as 4.1f */
        /* 45*/ { BARCODE_MAXICODE, 0.88f, 47, 1200, "emf", 1.034f },
        /* 46*/ { BARCODE_MAXICODE, 0.88f, 47, 1200, "svg", 20.68f },
        /* 47*/ { BARCODE_MAXICODE, 0.88f, 94, 2400, "gif", 8.272f }, /* NOTE dpmm previously documented as 95, scale as 8.2f */
        /* 48*/ { BARCODE_MAXICODE, 0.88f, 94, 2400, "emf", 2.0680001f },
        /* 49*/ { BARCODE_MAXICODE, 0.88f, 94, 2400, "svg", 41.360001f },
        /* 50*/ { BARCODE_MAXICODE, 0.88f, 189, 4800, "gif", 16.632f }, /* NOTE scale previously documented as 16.4f */
        /* 51*/ { BARCODE_MAXICODE, 0.88f, 189, 4800, "emf", 4.158f },
        /* 52*/ { BARCODE_MAXICODE, 0.88f, 189, 4800, "svg", 83.159996f },
        /* 53*/ { BARCODE_MAXICODE, 0.88f, 378, 9600, "gif", 33.264f },
        /* 54*/ { BARCODE_MAXICODE, 0.88f, 378, 9600, "emf", 8.316f },
        /* 55*/ { BARCODE_MAXICODE, 0.88f, 378, 9600, "svg", 166.31999f },
        /* 56*/ { BARCODE_PDF417, 0.27f, 2, 50, "gif", 0.5f },
        /* 57*/ { BARCODE_PDF417, 0.27f, 2, 50, "svg", 0.27000001f },
        /* 58*/ { BARCODE_PDF417, 0.27f, 6, 150, "gif", 1 },
        /* 59*/ { BARCODE_PDF417, 0.27f, 6, 150, "svg", 0.81000006f },
        /* 60*/ { BARCODE_PDF417, 0.27f, 12, 300, "gif", 1.5 },
        /* 61*/ { BARCODE_PDF417, 0.27f, 12, 300, "svg", 1.6200001f },
        /* 62*/ { BARCODE_PDF417, 0.27f, 24, 600, "gif", 3 },
        /* 63*/ { BARCODE_PDF417, 0.27f, 24, 600, "emf", 3.2400002f },
        /* 64*/ { BARCODE_PDF417, 0.27f, 24, 600, "svg", 3.2400002f },
        /* 65*/ { BARCODE_PHARMA_TWO, 1, 2, 50, "gif", 1 },
        /* 66*/ { BARCODE_PHARMA_TWO, 1, 2, 50, "svg", 1 },
        /* 67*/ { BARCODE_PHARMA_TWO, 1, 6, 150, "gif", 3 },
        /* 68*/ { BARCODE_PHARMA_TWO, 1, 6, 150, "svg", 3 },
        /* 69*/ { BARCODE_PHARMA_TWO, 1, 8, 200, "gif", 4 },
        /* 70*/ { BARCODE_PHARMA_TWO, 1, 8, 200, "svg", 4 },
        /* 71*/ { BARCODE_PHARMA_TWO, 1, 189, 4800, "gif", 94.5f },
        /* 72*/ { BARCODE_PHARMA_TWO, 1, 189, 4800, "svg", 94.5f },
        /* 73*/ { BARCODE_PHARMA_TWO, 1, 378, 9600, "gif", 189 },
        /* 74*/ { BARCODE_PHARMA_TWO, 1, 378, 9600, "svg", 189 },
        /* 75*/ { BARCODE_PHARMA_TWO, 1, 401, 10200, "gif", 200 }, /* NOTE scale capped to 200 so doesn't round trip */
        /* 76*/ { BARCODE_PHARMA_TWO, 1, 401, 10200, "svg", 200 },
        /* 77*/ { BARCODE_CODE128, 0.5, 12, 300, "gif", 3 },
        /* 78*/ { BARCODE_CODE128, 0, 12, -1, "gif", 0 }, /* x_dim zero */
        /* 79*/ { BARCODE_CODE128, 200.1f, 12, -1, "gif", 0 }, /* x_dim > 200 */
        /* 80*/ { BARCODE_CODE128, 0.5f, -0.1f, -1, "gif", 0 }, /* dpmm neg */
        /* 81*/ { BARCODE_CODE128, 0.5f, 1000.1, -1, "gif", 0 }, /* dpmm > 1000 */
        /* 82*/ { BARCODE_CODE128, 0.5f, 300, -1, "abcd", 0 }, /* filetype unknown */
        /* 83*/ { BARCODE_QRCODE, 10, 31, 800, "gif", 155 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i;
    float ret;
    float x_dim_from_scale;
    float dpmm_from_dpi;

    testStart("test_scale_from_xdimdp");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        ret = ZBarcode_Scale_From_XdimDp(data[i].symbology, data[i].x_dim, data[i].dpmm, data[i].filetype);
        assert_equal(ret, data[i].expected, "i:%d ZBarcode_Scale_From_XdimDp(%s, %g, %g, %s) %.8g != %.8g\n",
            i, testUtilBarcodeName(data[i].symbology), data[i].dpmm, data[i].x_dim, data[i].filetype, ret, data[i].expected);

        if (ret) {
            dpmm_from_dpi = stripf(roundf(data[i].dpi / 25.4f));
            ret = ZBarcode_Scale_From_XdimDp(data[i].symbology, data[i].x_dim, dpmm_from_dpi, data[i].filetype);
            assert_equal(ret, data[i].expected, "i:%d ZBarcode_Scale_From_XdimDp(%s, %g (dpi %d), %g, %s) %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), dpmm_from_dpi, data[i].dpi, data[i].x_dim, data[i].filetype, ret, data[i].expected);

            if (data[i].expected > 0.1f && data[i].expected < 200.0f /* Can't round trip scales <= 0.1 or >= 200.0 */
                    && (data[i].symbology == BARCODE_MAXICODE || strcmp(data[i].filetype, "gif") != 0)) { /* Non-MAXICODE raster rounds to half-increments */
                x_dim_from_scale = ZBarcode_XdimDp_From_Scale(data[i].symbology, ret, data[i].dpmm, data[i].filetype);
                x_dim_from_scale = stripf(stripf(roundf(x_dim_from_scale * 100.0f)) / 100.0f);
                assert_equal(x_dim_from_scale, data[i].x_dim, "i:%d ZBarcode_XdimDp_From_Scale(%s, %g, %g, %s) %.8g != x_dim %.8g\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, data[i].x_dim, data[i].filetype, x_dim_from_scale, data[i].x_dim);
            }
        }
    }

    testFinish();
}

static void test_xdimdp_from_scale(const testCtx *const p_ctx) {

    struct item {
        int symbology;
        float scale;
        float dpmm; /* Note testing "normal" case that want X-dim, not dpmm */
        int dpi;
        char *filetype;
        float expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_EANX, 1, 6, 150, "gif", 0.33333334f },
        /*  1*/ { BARCODE_EANX, 1.32f, 8, 200, "gif", 0.33000001f },
        /*  2*/ { BARCODE_EANX, 1.5f, 8, 200, "gif", 0.375f },
        /*  3*/ { BARCODE_EANX, 1.98f, 12, 300, "gif", 0.33f },
        /*  4*/ { BARCODE_EANX, 2, 12, 300, "gif", 0.33333334f },
        /*  5*/ { BARCODE_EANX, 2, 12, 300, "svg", 0.33333334f },
        /*  6*/ { BARCODE_EANX, 2.64f, 16, 400, "gif", 0.33f },
        /*  7*/ { BARCODE_EANX, 2.5f, 16, 400, "gif", 0.3125f },
        /*  8*/ { BARCODE_EANX, 3.96f, 24, 600, "gif", 0.33f },
        /*  9*/ { BARCODE_EANX, 3.96f, 24, 600, "svg", 0.33f },
        /* 10*/ { BARCODE_EANX, 4, 24, 600, "gif", 0.33333334f },
        /* 11*/ { BARCODE_EANX, 7.755f, 47, 1200, "gif", 0.33f },
        /* 12*/ { BARCODE_EANX, 8, 47, 1200, "gif", 0.34042552f },
        /* 13*/ { BARCODE_EANX, 15.51f, 94, 2400, "gif", 0.33f },
        /* 14*/ { BARCODE_EANX, 15.5f, 94, 2400, "gif", 0.32978722f },
        /* 15*/ { BARCODE_EANX, 31.185001f, 189, 4800, "gif", 0.33f },
        /* 16*/ { BARCODE_EANX, 31, 189, 4800, "gif", 0.32804233f },
        /* 17*/ { BARCODE_MAXICODE, 1, 12, 300, "gif", 0.83333331f },
        /* 18*/ { BARCODE_MAXICODE, 0.264f, 12, 300, "emf", 0.87999994f },
        /* 19*/ { BARCODE_MAXICODE, 5.2799997f, 12, 300, "svg", 0.87999994f },
        /* 20*/ { BARCODE_MAXICODE, 2, 24, 600, "gif", 0.83333331f },
        /* 21*/ { BARCODE_MAXICODE, 0.528f, 24, 600, "emf", 0.87999994f },
        /* 22*/ { BARCODE_MAXICODE, 10.559999f, 24, 600, "svg", 0.87999994f },
        /* 23*/ { BARCODE_CODE128, 0, 12, -1, "gif", 0 }, /* scale zero */
        /* 24*/ { BARCODE_CODE128, 200.01f, 12, -1, "gif", 0 }, /* scale > 200 */
        /* 25*/ { BARCODE_CODE128, 0.5f, 0, -1, "gif", 0 }, /* xdim_mm_or_dpmm zero */
        /* 26*/ { BARCODE_CODE128, 0.5f, 1000.1f, -1, "gif", 0 }, /* xdim_mm_or_dpmm > 1000 */
    };
    const int data_size = ARRAY_SIZE(data);
    int i;
    float ret;
    float dpmm_from_dpi;

    testStart("test_xdimdp_from_scale");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        ret = ZBarcode_XdimDp_From_Scale(data[i].symbology, data[i].scale, data[i].dpmm, data[i].filetype);
        assert_equal(ret, data[i].expected, "i:%d ZBarcode_XdimDp_From_Scale(%s, %g, %g, %s) %.8g != %.8g\n",
            i, testUtilBarcodeName(data[i].symbology), data[i].dpmm, data[i].scale, data[i].filetype, ret, data[i].expected);

        if (ret) {
            dpmm_from_dpi = stripf(roundf(data[i].dpi / 25.4f));
            ret = ZBarcode_XdimDp_From_Scale(data[i].symbology, data[i].scale, dpmm_from_dpi, data[i].filetype);
            assert_equal(ret, data[i].expected, "i:%d ZBarcode_XdimDp_From_Scale(%s, %g (dpi %d), %g, %s) %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), dpmm_from_dpi, data[i].dpi, data[i].scale, data[i].filetype, ret, data[i].expected);
        }
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_checks", test_checks },
        { "test_checks_segs", test_checks_segs },
        { "test_input_data", test_input_data },
        { "test_symbologies", test_symbologies },
        { "test_input_mode", test_input_mode },
        { "test_escape_char_process", test_escape_char_process },
        { "test_escape_char_process_test", test_escape_char_process_test },
        { "test_cap", test_cap },
        { "test_cap_compliant_height", test_cap_compliant_height },
        { "test_encode_file_empty", test_encode_file_empty },
        { "test_encode_file_too_large", test_encode_file_too_large },
        { "test_encode_file_unreadable", test_encode_file_unreadable },
        { "test_encode_file_directory", test_encode_file_directory },
        { "test_encode_file", test_encode_file },
        { "test_encode_print_outfile_directory", test_encode_print_outfile_directory },
        { "test_bad_args", test_bad_args },
        { "test_valid_id", test_valid_id },
        { "test_barcode_name", test_barcode_name },
        { "test_error_tag", test_error_tag },
        { "test_strip_bom", test_strip_bom },
        { "test_zero_outfile", test_zero_outfile },
        { "test_clear", test_clear },
        { "test_reset", test_reset },
        { "test_scale_from_xdimdp", test_scale_from_xdimdp },
        { "test_xdimdp_from_scale", test_xdimdp_from_scale },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
