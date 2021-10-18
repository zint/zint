/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2021 Robin Stuart <rstuart114@gmail.com>

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
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

static void test_checks(int index, int debug) {

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
        float guard_descent;
        int warn_level;
        int ret;

        char *expected;
        int expected_symbology;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, "1234", -1, -1, 3, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 217: Symbology does not support ECI switching", -1 },
        /*  1*/ { BARCODE_CODE128, -1, "1234", -1, -1, 0, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", -1 },
        /*  2*/ { BARCODE_QRCODE, -1, "1234", -1, -1, 3, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", -1 },
        /*  3*/ { BARCODE_QRCODE, -1, "1234", -1, -1, 999999 + 1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 218: Invalid ECI mode", -1 },
        /*  4*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, 0, 0.009, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 227: Scale out of range (0.01 to 100)", -1 },
        /*  5*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, 0, 100.01, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 227: Scale out of range (0.01 to 100)", -1 },
        /*  6*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, 0, -1, 20.1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 221: Dot size out of range (0.01 to 20)", -1 },
        /*  7*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, 0, 0.01, 0.009, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 221: Dot size out of range (0.01 to 20)", -1 },
        /*  8*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, -0.1, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 765: Height out of range (0 to 2000)", -1 },
        /*  9*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 2000.01, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 765: Height out of range (0 to 2000)", -1 },
        /* 10*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, -1, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 766: Whitespace width out of range (0 to 100)", -1 },
        /* 11*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 101, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 766: Whitespace width out of range (0 to 100)", -1 },
        /* 12*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, -1, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 767: Whitespace height out of range (0 to 100)", -1 },
        /* 13*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 101, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 767: Whitespace height out of range (0 to 100)", -1 },
        /* 14*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, -1, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 768: Border width out of range (0 to 100)", -1 },
        /* 15*/ { BARCODE_CODE128, -1, "1234", -1, -1, -1, 0, 0, 0, 101, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 768: Border width out of range (0 to 100)", -1 },
        /* 16*/ { BARCODE_CODE128, -1, "1234", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 220: Selected symbology does not support GS1 mode", -1 },
        /* 17*/ { BARCODE_EANX, -1, "123456789012", -1, -1, -1, 0, 0, 0, 101, -1, -1, -0.5, -1, ZINT_ERROR_INVALID_OPTION, "Error 769: Guard bar descent out of range (0 to 50)", -1 },
        /* 18*/ { BARCODE_EANX, -1, "123456789012", -1, -1, -1, 0, 0, 0, 101, -1, -1, 50.1, -1, ZINT_ERROR_INVALID_OPTION, "Error 769: Guard bar descent out of range (0 to 50)", -1 },
        /* 19*/ { BARCODE_GS1_128, -1, "[21]12\0004", 8, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 262: NUL characters not permitted in GS1 mode", -1 },
        /* 20*/ { BARCODE_GS1_128, -1, "[21]12é4", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1", -1 },
        /* 21*/ { BARCODE_GS1_128, -1, "[21]12\0074", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 251: Control characters are not supported by GS1", -1 },
        /* 22*/ { BARCODE_GS1_128, -1, "[21]1234", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", -1 },
        /* 23*/ { 0, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range", BARCODE_CODE128 },
        /* 24*/ { 0, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /* 25*/ { 0, -1, "1", -1, -1, 1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 217: Symbology does not support ECI switching", BARCODE_CODE128 }, // Not supporting beats invalid ECI
        /* 26*/ { 0, -1, "1", -1, -1, 1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /* 27*/ { 0, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, 0.009, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 221: Dot size out of range (0.01 to 20)", BARCODE_CODE128 },
        /* 28*/ { 0, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, 0.009, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /* 29*/ { 0, -1, "1", -1, -1, 1, 0, 0, 0, 0, -1, 0.009, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 217: Symbology does not support ECI switching", BARCODE_CODE128 }, // Invalid dot size no longer beats invalid ECI
        /* 30*/ { 0, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, 0.009, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range", -1 },
        /* 31*/ { 5, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_C25STANDARD },
        /* 32*/ { 5, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_C25STANDARD },
        /* 33*/ { 10, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_EANX },
        /* 34*/ { 10, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_EANX },
        /* 35*/ { 11, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_EANX },
        /* 36*/ { 11, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_EANX },
        /* 37*/ { 12, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_EANX },
        /* 38*/ { 12, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_EANX },
        /* 39*/ { 15, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_EANX },
        /* 40*/ { 15, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_EANX },
        /* 41*/ { 17, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_UPCA },
        /* 42*/ { 17, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_UPCA },
        /* 43*/ { 19, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_TOO_LONG, "Error 362: Input too short (3 character minimum)", BARCODE_CODABAR },
        /* 44*/ { 19, -1, "A1B", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 207: Codabar 18 not supported", BARCODE_CODABAR },
        /* 45*/ { 19, -1, "A1B", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 207: Codabar 18 not supported", -1 },
        /* 46*/ { 26, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_UPCA },
        /* 47*/ { 26, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_UPCA },
        /* 48*/ { 27, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 208: UPCD1 not supported", 27 },
        /* 49*/ { 33, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI", BARCODE_GS1_128 },
        /* 50*/ { 33, -1, "[10]23", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_GS1_128 },
        /* 51*/ { 33, -1, "[10]23", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_GS1_128 },
        /* 52*/ { 36, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_UPCA },
        /* 53*/ { 36, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_UPCA },
        /* 54*/ { 39, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_UPCE },
        /* 55*/ { 39, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_UPCE },
        /* 56*/ { 41, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_NONCOMPLIANT, "Warning 479: Input length is not standard (5, 9 or 11 characters)", BARCODE_POSTNET },
        /* 57*/ { 41, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_POSTNET },
        /* 58*/ { 41, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_POSTNET },
        /* 59*/ { 42, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_POSTNET },
        /* 60*/ { 42, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_POSTNET },
        /* 61*/ { 43, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_POSTNET },
        /* 62*/ { 43, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_POSTNET },
        /* 63*/ { 44, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_POSTNET },
        /* 64*/ { 44, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_POSTNET },
        /* 65*/ { 45, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_POSTNET },
        /* 66*/ { 45, -1, "12345", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_POSTNET },
        /* 67*/ { 46, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_PLESSEY },
        /* 68*/ { 46, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_PLESSEY },
        /* 69*/ { 48, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_NVE18 },
        /* 70*/ { 48, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_NVE18 },
        /* 71*/ { 54, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 210: General Parcel Code not supported", BARCODE_CODE128 },
        /* 72*/ { 54, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 210: General Parcel Code not supported", -1 },
        /* 73*/ { 59, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_CODE128 },
        /* 74*/ { 59, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_CODE128 },
        /* 75*/ { 61, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_CODE128 },
        /* 76*/ { 61, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_CODE128 },
        /* 77*/ { 62, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_CODE93 },
        /* 78*/ { 62, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_CODE93 },
        /* 79*/ { 64, -1, "12345678", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_AUSPOST },
        /* 80*/ { 64, -1, "12345678", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_AUSPOST },
        /* 81*/ { 65, -1, "12345678", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_AUSPOST },
        /* 82*/ { 65, -1, "12345678", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_AUSPOST },
        /* 83*/ { 78, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_DBAR_OMN },
        /* 84*/ { 78, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_DBAR_OMN },
        /* 85*/ { 83, -1, "12345678901", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_PLANET },
        /* 86*/ { 83, -1, "12345678901", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_PLANET },
        /* 87*/ { 88, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI", BARCODE_GS1_128 },
        /* 88*/ { 88, -1, "[10]12", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_GS1_128 },
        /* 89*/ { 88, -1, "[10]12", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_GS1_128 },
        /* 90*/ { 91, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 212: Symbology out of range", BARCODE_CODE128 },
        /* 91*/ { 91, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 212: Symbology out of range", -1 },
        /* 92*/ { 94, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 213: Symbology out of range", BARCODE_CODE128 },
        /* 93*/ { 94, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 213: Symbology out of range", -1 },
        /* 94*/ { 95, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 213: Symbology out of range", BARCODE_CODE128 },
        /* 95*/ { 95, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 213: Symbology out of range", -1 },
        /* 96*/ { 100, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_HIBC_128 },
        /* 97*/ { 100, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_128 },
        /* 98*/ { 101, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_HIBC_39 },
        /* 99*/ { 101, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_39 },
        /*100*/ { 103, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_HIBC_DM },
        /*101*/ { 103, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_DM },
        /*102*/ { 105, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_HIBC_QR },
        /*103*/ { 105, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_QR },
        /*104*/ { 107, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_HIBC_PDF },
        /*105*/ { 107, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_PDF },
        /*106*/ { 109, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_HIBC_MICPDF },
        /*107*/ { 109, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_MICPDF },
        /*108*/ { 111, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", BARCODE_HIBC_BLOCKF },
        /*109*/ { 111, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, 0, "", BARCODE_HIBC_BLOCKF },
        /*110*/ { 113, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 214: Symbology out of range", BARCODE_CODE128 },
        /*111*/ { 113, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 214: Symbology out of range", -1 },
        /*112*/ { 114, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 214: Symbology out of range", BARCODE_CODE128 },
        /*113*/ { 114, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 214: Symbology out of range", -1 },
        /*114*/ { 117, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 215: Symbology out of range", BARCODE_CODE128 },
        /*115*/ { 117, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 215: Symbology out of range", -1 },
        /*116*/ { 118, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 215: Symbology out of range", BARCODE_CODE128 },
        /*117*/ { 118, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 215: Symbology out of range", -1 },
        /*118*/ { 119, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 215: Symbology out of range", BARCODE_CODE128 },
        /*119*/ { 119, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 215: Symbology out of range", -1 },
        /*120*/ { 120, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 215: Symbology out of range", BARCODE_CODE128 },
        /*121*/ { 120, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 215: Symbology out of range", -1 },
        /*122*/ { 122, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 215: Symbology out of range", BARCODE_CODE128 },
        /*123*/ { 122, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 215: Symbology out of range", -1 },
        /*124*/ { 123, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 215: Symbology out of range", BARCODE_CODE128 },
        /*125*/ { 123, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 215: Symbology out of range", -1 },
        /*126*/ { 124, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 215: Symbology out of range", BARCODE_CODE128 },
        /*127*/ { 124, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 215: Symbology out of range", -1 },
        /*128*/ { 125, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 215: Symbology out of range", BARCODE_CODE128 },
        /*129*/ { 125, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 215: Symbology out of range", -1 },
        /*130*/ { 126, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 215: Symbology out of range", BARCODE_CODE128 },
        /*131*/ { 126, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 215: Symbology out of range", -1 },
        /*132*/ { 127, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 215: Symbology out of range", BARCODE_CODE128 },
        /*133*/ { 127, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 215: Symbology out of range", -1 },
        /*134*/ { 146, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 216: Symbology out of range", BARCODE_CODE128 },
        /*135*/ { 146, -1, "1", -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 216: Symbology out of range", -1 },
        /*136*/ { BARCODE_CODE128, -1, "\200", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 245: Invalid UTF-8 in input data", -1 },
        /*137*/ { BARCODE_GS1_128, -1, "[01]12345678901234", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (01) position 14: Bad checksum '4', expected '1'", -1 },
        /*138*/ { BARCODE_GS1_128, -1, "[01]12345678901234", -1, GS1_MODE, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_NONCOMPLIANT, "Error 261: AI (01) position 14: Bad checksum '4', expected '1'", -1 },
        /*139*/ { BARCODE_QRCODE, -1, "ก", -1, UNICODE_MODE, 13, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", -1 },
        /*140*/ { BARCODE_QRCODE, -1, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_USES_ECI, "Warning 222: Encoded data includes ECI 13", -1 },
        /*141*/ { BARCODE_QRCODE, -1, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_USES_ECI, "Error 222: Encoded data includes ECI 13", -1 },
        /*142*/ { BARCODE_CODEONE, -1, "[01]12345678901231", -1, GS1_MODE, 3, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 512: ECI ignored for GS1 mode", -1 },
        /*143*/ { BARCODE_CODEONE, -1, "[01]12345678901231", -1, GS1_MODE, 3, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 512: ECI ignored for GS1 mode", -1 },
        /*144*/ { BARCODE_CODEONE, -1, "[01]12345678901234", -1, GS1_MODE, 3, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 512: ECI ignored for GS1 mode", -1 }, // Warning in encoder overrides library warnings
        /*145*/ { BARCODE_CODEONE, -1, "[01]12345678901234", -1, GS1_MODE, 3, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_NONCOMPLIANT, "Error 261: AI (01) position 14: Bad checksum '4', expected '1'", -1 }, // But not errors
        /*146*/ { BARCODE_AZTEC, -1, "ก", -1, UNICODE_MODE, 13, 0, 0, 0, 0, -1, -1, -1, -1, 0, "", -1 },
        /*147*/ { BARCODE_AZTEC, -1, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_USES_ECI, "Warning 222: Encoded data includes ECI 13", -1 },
        /*148*/ { BARCODE_AZTEC, -1, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_USES_ECI, "Error 222: Encoded data includes ECI 13", -1 },
        /*149*/ { BARCODE_AZTEC, 6, "ก", -1, UNICODE_MODE, 13, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 503: Invalid error correction level - using default instead", -1 },
        /*150*/ { BARCODE_AZTEC, 6, "ก", -1, UNICODE_MODE, 13, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 503: Invalid error correction level - using default instead", -1 },
        /*151*/ { BARCODE_AZTEC, 6, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, -1, -1, ZINT_WARN_USES_ECI, "Warning 222: Encoded data includes ECI 13", -1 }, // ECI warning trumps all other warnings
        /*152*/ { BARCODE_AZTEC, 6, "ก", -1, UNICODE_MODE, -1, 0, 0, 0, 0, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 503: Invalid error correction level - using default instead", -1 }, // But not errors
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_checks");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

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
        if (data[i].guard_descent != -1) {
            symbol->guard_descent = data[i].guard_descent;
        }
        if (data[i].warn_level != -1) {
            symbol->warn_level = data[i].warn_level;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
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

static void test_symbologies(void) {
    int i, ret;
    struct zint_symbol symbol = {0};

    testStart("test_symbologies");

    for (i = -1; i < 148; i++) {
        symbol.symbology = i;
        ret = ZBarcode_Encode(&symbol, (unsigned char *) "1", 0);
        assert_notequal(ret, ZINT_ERROR_ENCODING_PROBLEM, "i:%d Encoding problem (%s)\n", i, symbol.errtxt);
    }

    testFinish();
}

static void test_input_mode(int index, int debug) {

    struct item {
        char *data;
        int input_mode;
        int ret;

        int expected_input_mode;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "1234", DATA_MODE, 0, DATA_MODE },
        /*  1*/ { "1234", DATA_MODE | ESCAPE_MODE, 0, DATA_MODE | ESCAPE_MODE },
        /*  2*/ { "1234", UNICODE_MODE, 0, UNICODE_MODE },
        /*  3*/ { "1234", UNICODE_MODE | ESCAPE_MODE, 0, UNICODE_MODE | ESCAPE_MODE },
        /*  4*/ { "[01]12345678901231", GS1_MODE, 0, GS1_MODE },
        /*  5*/ { "[01]12345678901231", GS1_MODE | ESCAPE_MODE, 0, GS1_MODE | ESCAPE_MODE },
        /*  6*/ { "1234", 4 | ESCAPE_MODE, 0, DATA_MODE }, // Unknown mode reset to bare DATA_MODE
        /*  7*/ { "1234", -1, 0, DATA_MODE },
        /*  8*/ { "1234", DATA_MODE | 0x10, 0, DATA_MODE | 0x10 }, // Unknown flags kept (but ignored)
        /*  9*/ { "1234", UNICODE_MODE | 0x10, 0, UNICODE_MODE | 0x10 },
        /* 10*/ { "[01]12345678901231", GS1_MODE | 0x20, 0, GS1_MODE | 0x20 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_input_mode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODE49 /*Supports GS1*/, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(symbol->input_mode, data[i].expected_input_mode, "i:%d symbol->input_mode %d != %d\n", i, symbol->input_mode, data[i].expected_input_mode);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_escape_char_process(int index, int generate, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int eci;
        char *data;
        int ret;
        int expected_width;
        char *expected;
        int compare_previous;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", 0, 26, "01 05 08 09 0A 0B 0C 0D E7 D8 7B 1F B6 4D 45 B6 45 7C EF DD 8C 4C 8D 1E D0 55 AD FE A8 52", 0, "" },
        /*  1*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", 0, 101, "(45) 67 62 43 40 44 47 48 29 6A 67 62 0B 49 4A 4B 4C 18 6A 67 62 0C 4D 5B 5D 5E 62 6A 67", 0, "" },
        /*  2*/ { BARCODE_CODE16K, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", 0, 70, "(20) 14 64 68 71 72 73 74 75 76 77 91 93 94 101 65 60 103 103 45 61", 0, "" },
        /*  3*/ { BARCODE_DOTCODE, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", 0, 28, "65 40 44 47 48 49 4A 4B 4C 4D 5B 5D 5E 6E 41 3C", 0, "" },
        /*  4*/ { BARCODE_GRIDMATRIX, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", 0, 30, "30 1A 00 02 01 61 00 48 28 16 0C 06 46 63 51 74 05 38 00", 0, "" },
        /*  5*/ { BARCODE_HANXIN, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", 0, 23, "2F 80 10 72 09 28 B3 0D 6F F3 00 20 E8 F4 0A E0 00", 0, "" },
        /*  6*/ { BARCODE_MAXICODE, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", 0, 30, "(144) 04 3E 3E 00 04 07 08 09 0A 0B 03 3D 2C 24 19 1E 23 1B 18 0E 0C 0D 1E 21 3C 1E 3C 31", 0, "" },
        /*  7*/ { BARCODE_PDF417, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", 0, 120, "(24) 16 901 0 23 655 318 98 18 461 639 893 122 129 92 900 900 872 438 359 646 522 773 831", 0, "" },
        /*  8*/ { BARCODE_ULTRA, DATA_MODE, -1, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", 0, 20, "(15) 257 0 4 7 8 9 10 11 12 13 27 29 30 129 92", 0, "" },
        /*  9*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\c", ZINT_ERROR_INVALID_DATA, 0, "Error 234: Unrecognised escape character in input data", 0, "" },
        /* 10*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\", ZINT_ERROR_INVALID_DATA, 0, "Error 236: Incomplete escape character in input data", 0, "" },
        /* 11*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\x", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete escape character in input data", 0, "" },
        /* 12*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\x1", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete escape character in input data", 0, "" },
        /* 13*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\x1g", ZINT_ERROR_INVALID_DATA, 0, "Error 233: Corrupt escape character in input data", 0, "" },
        /* 14*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\xA01\\xFF", 0, 12, "EB 21 32 EB 80 D8 49 44 DC 7D 9E 3B", 0, "" },
        /* 15*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\u00A01\\u00FF", 0, 12, "EB 21 32 EB 80 D8 49 44 DC 7D 9E 3B", 1, "" },
        /* 16*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\xc3\\xbF", 0, 12, "EB 44 EB 40 81 30 87 17 C5 68 5C 91", 0, "" },
        /* 17*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\u00fF", 0, 12, "EB 44 EB 40 81 30 87 17 C5 68 5C 91", 1, "" },
        /* 18*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\xc3\\xbF", 0, 10, "EB 80 81 47 1E 45 FC 93", 0, "" },
        /* 19*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, "\\u00fF", 0, 10, "EB 80 81 47 1E 45 FC 93", 1, "" },
        /* 20*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\u", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete Unicode escape character in input data", 0, "" },
        /* 21*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\uF", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete Unicode escape character in input data", 0, "" },
        /* 22*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\u0F", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete Unicode escape character in input data", 0, "" },
        /* 23*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\uFG", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete Unicode escape character in input data", 0, "" },
        /* 24*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\u00F", ZINT_ERROR_INVALID_DATA, 0, "Error 209: Incomplete Unicode escape character in input data", 0, "" },
        /* 25*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\u00FG", ZINT_ERROR_INVALID_DATA, 0, "Error 211: Corrupt Unicode escape character in input data", 0, "" },
        /* 26*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\ufffe", ZINT_ERROR_INVALID_DATA, 0, "Error 246: Invalid Unicode BMP escape character in input data", 0, "Reversed BOM" },
        /* 27*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\ud800", ZINT_ERROR_INVALID_DATA, 0, "Error 246: Invalid Unicode BMP escape character in input data", 0, "Surrogate" },
        /* 28*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, "\\udfff", ZINT_ERROR_INVALID_DATA, 0, "Error 246: Invalid Unicode BMP escape character in input data", 0, "Surrogate" },
        /* 29*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 17, "\\xE2\\x82\\xAC", 0, 12, "F1 12 EB 25 81 4A 0A 8C 31 AC E3 2E", 0, "Zint manual 4.10 Ex1" },
        /* 30*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 17, "\\u20AC", 0, 12, "F1 12 EB 25 81 4A 0A 8C 31 AC E3 2E", 1, "" },
        /* 31*/ { BARCODE_DATAMATRIX, DATA_MODE, 17, "\\xA4", 0, 12, "F1 12 EB 25 81 4A 0A 8C 31 AC E3 2E", 1, "" },
        /* 32*/ { BARCODE_DATAMATRIX, DATA_MODE, 28, "\\xB1\\x60", 0, 12, "F1 1D EB 32 61 D9 1C 0C C2 46 C3 B2", 0, "Zint manual 4.10 Ex2" },
        /* 33*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 28, "\\u5E38", 0, 12, "F1 1D EB 32 61 D9 1C 0C C2 46 C3 B2", 1, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];
    struct zint_symbol previous_symbol;
    char *input_filename = "test_escape.txt";

    testStart("test_escape_char_process");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode | ESCAPE_MODE, data[i].eci, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, \"%s\", %s, %d, \"%s\", %d, \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].eci,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->width, symbol->errtxt, data[i].compare_previous, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
                if (index == -1 && data[i].compare_previous) {
                    ret = testUtilSymbolCmp(symbol, &previous_symbol);
                    assert_zero(ret, "i:%d testUtilSymbolCmp ret %d != 0\n", i, ret);
                }
            }
            memcpy(&previous_symbol, symbol, sizeof(previous_symbol));

            if (ret < 5) {
                // Test from input file
                FILE *fp;
                struct zint_symbol *symbol2;

                fp = fopen(input_filename, "wb");
                assert_nonnull(fp, "i:%d fopen(%s) failed\n", i, input_filename);
                assert_notequal(fputs(data[i].data, fp), EOF, "i%d fputs(%s) failed == EOF (%d)\n", i, data[i].data, ferror(fp));
                assert_zero(fclose(fp), "i%d fclose() failed\n", i);

                symbol2 = ZBarcode_Create();
                assert_nonnull(symbol, "Symbol2 not created\n");

                symbol2->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

                (void) testUtilSetSymbol(symbol2, data[i].symbology, data[i].input_mode | ESCAPE_MODE, data[i].eci, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

                ret = ZBarcode_Encode_File(symbol2, input_filename);
                assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_File ret %d != %d (%s)\n", i, ret, data[i].ret, symbol2->errtxt);
                assert_zero(strcmp(symbol2->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol2->errtxt, data[i].expected);

                ret = testUtilSymbolCmp(symbol2, symbol);
                assert_zero(ret, "i:%d testUtilSymbolCmp symbol2 ret %d != 0\n", i, ret);

                assert_zero(remove(input_filename), "i:%d remove(%s) != 0 (%d: %s)\n", i, input_filename, errno, strerror(errno));

                ZBarcode_Delete(symbol2);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_cap(int index) {

    struct item {
        int symbology;
        unsigned cap_flag;
        unsigned int expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
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
    int data_size = ARRAY_SIZE(data);
    int i;
    unsigned int ret;

    testStart("test_cap");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        ret = ZBarcode_Cap(data[i].symbology, data[i].cap_flag);
        assert_equal(ret, data[i].expected, "i:%d ZBarcode_Cap(%s, 0x%X) 0x%X != 0x%X\n", i, testUtilBarcodeName(data[i].symbology), data[i].cap_flag, ret, data[i].expected);
    }

    testFinish();
}

static void test_cap_compliant_height() {
    int symbol_id;
    int ret;

    testStart("test_cap_compliant_height");

    for (symbol_id = 1; symbol_id <= BARCODE_RMQR; symbol_id++) {
        if (!ZBarcode_ValidID(symbol_id)) continue;

        ret = ZBarcode_Cap(symbol_id, ZINT_CAP_COMPLIANT_HEIGHT);

        switch (symbol_id) {
            //case BARCODE_CODE11: /* TODO: Find doc */
            case BARCODE_C25INTER:
            case BARCODE_CODE39:
            case BARCODE_EXCODE39:
            case BARCODE_EANX:
            case BARCODE_EANX_CHK:
            case BARCODE_GS1_128:
            case BARCODE_CODABAR:
            //case BARCODE_DPLEIT: /* TODO: Find doc */
            //case BARCODE_DPIDENT: /* TODO: Find doc */
            case BARCODE_CODE16K:
            case BARCODE_CODE49:
            case BARCODE_CODE93:
            //case BARCODE_FLAT: /* TODO: Find doc */
            case BARCODE_DBAR_OMN:
            case BARCODE_DBAR_LTD:
            case BARCODE_DBAR_EXP:
            case BARCODE_TELEPEN:
            case BARCODE_UPCA:
            case BARCODE_UPCA_CHK:
            case BARCODE_UPCE:
            case BARCODE_UPCE_CHK:
            case BARCODE_POSTNET:
            //case BARCODE_MSI_PLESSEY: /* TODO: Find doc */
            case BARCODE_FIM:
            case BARCODE_LOGMARS:
            case BARCODE_PHARMA:
            case BARCODE_PZN:
            case BARCODE_PHARMA_TWO:
            case BARCODE_AUSPOST:
            case BARCODE_AUSREPLY:
            case BARCODE_AUSROUTE:
            case BARCODE_AUSREDIRECT:
            case BARCODE_ISBNX:
            case BARCODE_RM4SCC:
            case BARCODE_EAN14:
            //case BARCODE_VIN: /* Spec unlikely */
            case BARCODE_CODABLOCKF:
            case BARCODE_NVE18:
            case BARCODE_JAPANPOST:
            //case BARCODE_KOREAPOST: /* TODO: Find doc */
            case BARCODE_DBAR_STK:
            case BARCODE_DBAR_OMNSTK:
            case BARCODE_DBAR_EXPSTK:
            case BARCODE_PLANET:
            case BARCODE_USPS_IMAIL:
            //case BARCODE_PLESSEY: /* TODO: Find doc */
            case BARCODE_TELEPEN_NUM:
            case BARCODE_ITF14:
            case BARCODE_KIX:
            case BARCODE_DPD:
            case BARCODE_HIBC_39:
            case BARCODE_HIBC_BLOCKF:
            case BARCODE_MAILMARK:
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
                assert_equal(ret, ZINT_CAP_COMPLIANT_HEIGHT, "symbol_id %d (%s) ret 0x%X != ZINT_CAP_COMPLIANT_HEIGHT\n", symbol_id, testUtilBarcodeName(symbol_id), ret);
                break;
            default:
                assert_zero(ret, "symbol_id %d (%s) ret 0x%X non-zero\n", symbol_id, testUtilBarcodeName(symbol_id), ret);
                break;
        }
    }

    testFinish();
}

static void test_encode_file_empty(void) {
    int ret;
    struct zint_symbol *symbol;
    char filename[] = "in.bin";
    FILE *fstream;

    testStart("test_encode_file_empty");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    (void) remove(filename); // In case junk hanging around

    fstream = fopen(filename, "w+");
    assert_nonnull(fstream, "fopen(%s) failed (%d)\n", filename, ferror(fstream));
    ret = fclose(fstream);
    assert_zero(ret, "fclose(%s) %d != 0\n", filename, ret);

    ret = ZBarcode_Encode_File(symbol, filename);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File empty ret %d != ZINT_ERROR_INVALID_DATA (%s)\n", ret, symbol->errtxt);

    ret = remove(filename);
    assert_zero(ret, "remove(%s) != 0 (%d: %s)\n", filename, errno, strerror(errno));

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_encode_file_too_large(void) {
    char filename[] = "in.bin";
    FILE *fstream;
    int ret;
    struct zint_symbol *symbol;
    char buf[ZINT_MAX_DATA_LEN + 1] = {0};

    testStart("test_encode_file_too_large");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    (void) remove(filename); // In case junk hanging around

    fstream = fopen(filename, "w+");
    assert_nonnull(fstream, "fopen(%s) failed (%d)\n", filename, ferror(fstream));
    ret = (int) fwrite(buf, 1, sizeof(buf), fstream);
    assert_equal(ret, sizeof(buf), "fwrite return value: %d != %d\n", ret, (int)sizeof(buf));
    ret = fclose(fstream);
    assert_zero(ret, "fclose(%s) %d != 0\n", filename, ret);

    ret = ZBarcode_Encode_File(symbol, filename);
    assert_equal(ret, ZINT_ERROR_TOO_LONG, "ZBarcode_Encode_File too large ret %d != ZINT_ERROR_TOO_LONG (%s)\n", ret, symbol->errtxt);

    ret = remove(filename);
    assert_zero(ret, "remove(%s) != 0 (%d: %s)\n", filename, errno, strerror(errno));

    ZBarcode_Delete(symbol);

    testFinish();
}

// #181 Nico Gunkel OSS-Fuzz
static void test_encode_file_unreadable(void) {
#ifndef _WIN32
    int ret;
    struct zint_symbol *symbol;
    char filename[] = "in.bin";

    char buf[ZINT_MAX_DATA_LEN + 1] = {0};
    int fd;
#endif

    testStart("test_encode_file_unreadable");

#ifdef _WIN32
    testSkip("Test not implemented on Windows");
#else

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    // Unreadable file
    fd = creat(filename, S_IWUSR);
    assert_notequal(fd, -1, "Unreadable input file (%s) not created == -1 (%d: %s)\n", filename, errno, strerror(errno));
    ret = write(fd, buf, 1);
    assert_equal(ret, 1, "Unreadable write ret %d != 1\n", ret);
    ret = close(fd);
    assert_zero(ret, "Unreadable close(%s) != 0(%d: %s)\n", filename, errno, strerror(errno));

    ret = ZBarcode_Encode_File(symbol, filename);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File unreadable ret %d != ZINT_ERROR_INVALID_DATA (%s)\n", ret, symbol->errtxt);

    ret = remove(filename);
    assert_zero(ret, "remove(%s) != 0 (%d: %s)\n", filename, errno, strerror(errno));

    ZBarcode_Delete(symbol);

    testFinish();
#endif /* _WIN32 */
}

// #181 Nico Gunkel OSS-Fuzz (buffer not freed on fread() error) Note: unable to reproduce fread() error using this method
static void test_encode_file_directory(void) {
    int ret;
    struct zint_symbol *symbol;
    char dirname[] = "in_dir";

    testStart("test_encode_file_directory");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    (void) testUtilRmDir(dirname); // In case junk hanging around
    ret = testUtilMkDir(dirname);
    assert_zero(ret, "testUtilMkDir(%s) %d != 0 (%d: %s)\n", dirname, ret, errno, strerror(errno));

    ret = ZBarcode_Encode_File(symbol, dirname);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "ret %d != ZINT_ERROR_INVALID_DATA (%s)\n", ret, symbol->errtxt);

    ret = testUtilRmDir(dirname);
    assert_zero(ret, "testUtilRmDir(%s) %d != 0 (%d: %s)\n", dirname, ret, errno, strerror(errno));

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_bad_args(void) {
    int ret;
    struct zint_symbol *symbol;
    char *data = "1";
    char *filename = "1.png";
    char *empty = "";

    testStart("test_bad_args");

    // These just return, no error
    ZBarcode_Clear(NULL);
    ZBarcode_Delete(NULL);

    ret = ZBarcode_Version();
    assert_nonzero(ret >= 20901, "ZBarcode_Version() %d <= 20901\n", ret);

    assert_zero(ZBarcode_ValidID(0), "ZBarcode_ValidID(0) non-zero\n");
    assert_zero(ZBarcode_ValidID(10), "ZBarcode_ValidID(10) non-zero\n"); // Note 10 remapped to BARCODE_EANX in ZBarcode_Encode() for tbarcode compat but not counted as valid

    ret = ZBarcode_Cap(0, ~0);
    assert_zero(ret, "ZBarcode_Cap(0, ~0) ret 0x%X != 0\n", ret);
    ret = ZBarcode_Cap(10, ~0);
    assert_zero(ret, "ZBarcode_Cap(10, ~0) ret 0x%X != 0\n", ret);

    // NULL symbol
    assert_equal(ZBarcode_Encode(NULL, (unsigned char *) data, 1), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode(NULL, data, 1) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Print(NULL, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Print(NULL, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Buffer(NULL, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Buffer(NULL, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Buffer_Vector(NULL, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Buffer_Vector(NULL, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_and_Print(NULL, (unsigned char *) data, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Print(NULL, data, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_and_Buffer(NULL, (unsigned char *) data, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Buffer(NULL, data, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_File(NULL, filename), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File(NULL, filename) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_File_and_Print(NULL, filename, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Print(NULL, filename, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_equal(ZBarcode_Encode_File_and_Buffer(NULL, filename, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Buffer(NULL, filename, 0) != ZINT_ERROR_INVALID_DATA\n");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    // NULL data/filename
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode(symbol, NULL, 1), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode(symbol, NULL, 1) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode(symbol, NULL, 1) no errtxt\n");
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_and_Print(symbol, NULL, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Print(symbol, NULL, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode_and_Print(symbol, NULL, 1, 0) no errtxt\n");
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_and_Buffer(symbol, NULL, 1, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Buffer(symbol, NULL, 1, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode_and_Buffer(symbol, NULL, 1, 0) no errtxt\n");
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File(symbol, NULL), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File(symbol, NULL) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode_File(symbol, NULL) no errtxt\n");
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File_and_Print(symbol, NULL, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Print(symbol, NULL, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode_File_and_Print(symbol, NULL, 0) no errtxt\n");
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File_and_Buffer(symbol, NULL, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Buffer(symbol, NULL, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode_File_and_Buffer(symbol, NULL, 0) no errtxt\n");

    // Empty data/filename
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode(symbol, (unsigned char *) empty, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode(symbol, empty, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode(symbol, empty, 0) no errtxt\n");
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_and_Print(symbol, (unsigned char *) empty, 0, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Print(symbol, empty, 0, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode_and_Print(symbol, empty, 0, 0) no errtxt\n");
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_and_Buffer(symbol, (unsigned char *) empty, 0, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_and_Buffer(symbol, empty, 0, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode_and_Buffer(symbol, empty, 0, 0) no errtxt\n");
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File(symbol, empty), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File(symbol, empty) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode_File(symbol, empty) no errtxt\n");
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File_and_Print(symbol, empty, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Print(symbol, empty, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode_File_and_Print(symbol, empty, 0) no errtxt\n");
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode_File_and_Buffer(symbol, empty, 0), ZINT_ERROR_INVALID_DATA, "ZBarcode_Encode_File_and_Buffer(symbol, empty, 0) != ZINT_ERROR_INVALID_DATA\n");
    assert_nonzero(strlen(symbol->errtxt), "ZBarcode_Encode_File_and_Buffer(symbol, empty, 0) no errtxt\n");

    // Data too big
    symbol->errtxt[0] = '\0';
    assert_equal(ZBarcode_Encode(symbol, (unsigned char *) empty, ZINT_MAX_DATA_LEN + 1), ZINT_ERROR_TOO_LONG, "ZBarcode_Encode(symbol, empty, ZINT_MAX_DATA_LEN + 1) != ZINT_ERROR_TOO_LONG\n");
    assert_nonzero((int) strlen(symbol->errtxt), "ZBarcode_Encode(symbol, empty, ZINT_MAX_DATA_LEN + 1) no errtxt\n");

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_valid_id(void) {

    int ret;
    const char *name;
    int symbol_id;

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

STATIC_UNLESS_ZINT_TEST int error_tag(struct zint_symbol *symbol, int error_number, const char *error_string);

static void test_error_tag(int index) {

    struct item {
        int error_number;
        int warn_level;
        char *data;
        int ret;
        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { ZINT_WARN_INVALID_OPTION, -1, "", ZINT_WARN_INVALID_OPTION, "Warning " },
        /*  1*/ { ZINT_WARN_INVALID_OPTION, WARN_FAIL_ALL, "", ZINT_ERROR_INVALID_OPTION, "Error " },
        /*  2*/ { ZINT_WARN_USES_ECI, -1, "", ZINT_WARN_USES_ECI, "Warning " },
        /*  3*/ { ZINT_WARN_USES_ECI, WARN_FAIL_ALL, "", ZINT_ERROR_USES_ECI, "Error " },
        /*  4*/ { ZINT_WARN_NONCOMPLIANT, -1, "", ZINT_WARN_NONCOMPLIANT, "Warning " },
        /*  5*/ { ZINT_WARN_NONCOMPLIANT, WARN_FAIL_ALL, "", ZINT_ERROR_NONCOMPLIANT, "Error " },
        /*  6*/ { ZINT_ERROR_TOO_LONG, WARN_DEFAULT, "", ZINT_ERROR_TOO_LONG, "Error " },
        /*  7*/ { ZINT_ERROR_TOO_LONG, WARN_FAIL_ALL, "", ZINT_ERROR_TOO_LONG, "Error " },
        /*  8*/ { ZINT_WARN_USES_ECI, -1, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", ZINT_WARN_USES_ECI, "Warning 1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901" },
        /*  9*/ { ZINT_WARN_USES_ECI, WARN_FAIL_ALL, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", ZINT_ERROR_USES_ECI, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
        /* 10*/ { ZINT_ERROR_INVALID_DATA, -1, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", ZINT_ERROR_INVALID_DATA, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
        /* 11*/ { ZINT_ERROR_INVALID_DATA, WARN_FAIL_ALL, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", ZINT_ERROR_INVALID_DATA, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
        /* 12*/ { ZINT_WARN_USES_ECI, -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", ZINT_WARN_USES_ECI, "Warning 1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901" },
        /* 13*/ { ZINT_WARN_USES_ECI, WARN_FAIL_ALL, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", ZINT_ERROR_USES_ECI, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
        /* 14*/ { ZINT_ERROR_INVALID_DATA, -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", ZINT_ERROR_INVALID_DATA, "Error 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, ret;

    testStart("test_error_tag");

    for (i = 0; i < data_size; i++) {
        struct zint_symbol symbol = {0};

        if (index != -1 && i != index) continue;

        symbol.warn_level = data[i].warn_level;

        ret = error_tag(&symbol, data[i].error_number, data[i].data);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_zero(strcmp(symbol.errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol.errtxt, data[i].expected);

        if ((int) strlen(data[i].data) < 100) {
            strcpy(symbol.errtxt, data[i].data);
            ret = error_tag(&symbol, data[i].error_number, NULL);
            assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
            assert_zero(strcmp(symbol.errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol.errtxt, data[i].expected);
        }
    }

    testFinish();
}

STATIC_UNLESS_ZINT_TEST void strip_bom(unsigned char *source, int *input_length);

static void test_strip_bom(void) {

    int length, ret;
    char data[] = "\357\273\277A"; // U+FEFF BOM, with "A"
    char bom_only[] = "\357\273\277"; // U+FEFF BOM only
    char buf[6];

    testStart("test_strip_bom");

    strcpy(buf, data);
    length = (int) strlen(buf);
    strip_bom((unsigned char *) buf, &length);
    assert_equal(length, 1, "length %d != 1\n", length);
    assert_zero(buf[1], "buf[1] %d != 0\n", buf[1]);

    // BOM not stripped if only data

    strcpy(buf, bom_only);
    length = (int) strlen(buf);
    strip_bom((unsigned char *) buf, &length);
    assert_equal(length, 3, "BOM only length %d != 3\n", length);
    ret = strcmp(buf, bom_only);
    assert_zero(ret, "BOM only strcmp ret %d != 0\n", ret);

    testFinish();
}

static void test_zero_outfile(void) {

    int ret;
    struct zint_symbol *symbol;
    char *data = "1234";

    testStart("test_zero_outfile");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    assert_nonzero(symbol->outfile[0], "ZBarcode_Create() outfile zero\n");
    symbol->outfile[0] = '\0';

    ret = ZBarcode_Encode(symbol, (unsigned char *) data, 0);
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

static void test_clear(void) {

    int ret;
    struct zint_symbol *symbol;
    char *data = "1234";

    testStart("test_clear");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    symbol->symbology = BARCODE_MAXICODE;
    symbol->whitespace_width = 5;
    symbol->whitespace_height = 5;
    symbol->border_width = 2;
    symbol->output_options = BARCODE_BOX;
    strcpy(symbol->fgcolour, "000000AA");
    strcpy(symbol->bgcolour, "FFFFFFAA");

    // Raster

    ret = ZBarcode_Encode(symbol, (unsigned char *) data, 0);
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
    assert_zero(symbol->bitmap_byte_length, "ZBarcode_Buffer() bitmap_byte_length %d != 0\n", (int) symbol->bitmap_byte_length);
    assert_null(symbol->vector, "ZBarcode_Buffer() vector != NULL\n");

    ZBarcode_Clear(symbol);

    assert_null(symbol->bitmap, "ZBarcode_Buffer() bitmap != NULL\n");
    assert_null(symbol->alphamap, "ZBarcode_Buffer() alphamap != NULL\n");
    assert_zero(symbol->bitmap_width, "ZBarcode_Buffer() bitmap_width %d != 0\n", symbol->bitmap_width);
    assert_zero(symbol->bitmap_height, "ZBarcode_Buffer() bitmap_height %d != 0\n", symbol->bitmap_height);

    assert_zero(symbol->rows, "ZBarcode_Buffer() rows %d != 0\n", symbol->rows);
    assert_zero(symbol->width, "ZBarcode_Buffer() width %d != 0\n", symbol->width);
    assert_zero(symbol->bitmap_byte_length, "ZBarcode_Buffer() bitmap_byte_length %d != 0\n", (int) symbol->bitmap_byte_length);
    assert_null(symbol->vector, "ZBarcode_Buffer() vector != NULL\n");

    // Vector

    ret = ZBarcode_Encode(symbol, (unsigned char *) data, 0);
    assert_zero(ret, "ZBarcode_Encode() ret %d != 0 (%s)\n", ret, symbol->errtxt);

    assert_nonzero(symbol->rows, "ZBarcode_Encode() rows 0\n");
    assert_nonzero(symbol->width, "ZBarcode_Encode() width 0\n");

    ret = ZBarcode_Buffer_Vector(symbol, 0);
    assert_zero(ret, "ZBarcode_Buffer_Vector() ret %d != 0 (%s)\n", ret, symbol->errtxt);

    assert_nonnull(symbol->vector, "ZBarcode_Buffer_Vector() vector NULL\n");
    assert_nonnull(symbol->vector->rectangles, "ZBarcode_Buffer_Vector() vector->rectangles NULL\n");
    assert_nonnull(symbol->vector->hexagons, "ZBarcode_Buffer_Vector() vector->hexagons NULL\n");
    assert_null(symbol->vector->strings, "ZBarcode_Buffer_Vector() vector->strings != NULL\n"); // MAXICODE no text
    assert_nonnull(symbol->vector->circles, "ZBarcode_Buffer_Vector() vector->circles NULL\n");

    assert_nonzero(symbol->rows, "ZBarcode_Buffer_Vector() rows 0\n");
    assert_nonzero(symbol->width, "ZBarcode_Buffer_Vector() width 0\n");
    assert_null(symbol->bitmap, "ZBarcode_Buffer_Vector() bitmap != NULL\n");
    assert_null(symbol->alphamap, "ZBarcode_Buffer_Vector() alphamap != NULL\n");
    assert_zero(symbol->bitmap_byte_length, "ZBarcode_Buffer_Vector() bitmap_byte_length %d != 0\n", (int) symbol->bitmap_byte_length);

    ZBarcode_Clear(symbol);

    assert_null(symbol->vector, "ZBarcode_Buffer_Vector() vector != NULL\n");

    assert_zero(symbol->rows, "ZBarcode_Buffer_Vector() rows %d != 0\n", symbol->rows);
    assert_zero(symbol->width, "ZBarcode_Buffer_Vector() width %d != 0\n", symbol->width);
    assert_zero(symbol->bitmap_byte_length, "ZBarcode_Buffer_Vector() bitmap_byte_length %d != 0\n", (int) symbol->bitmap_byte_length);
    assert_null(symbol->bitmap, "ZBarcode_Buffer_Vector() bitmap != NULL\n");
    assert_null(symbol->alphamap, "ZBarcode_Buffer_Vector() alphamap != NULL\n");
    assert_zero(symbol->bitmap_width, "ZBarcode_Buffer_Vector() bitmap_width %d != 0\n", symbol->bitmap_width);
    assert_zero(symbol->bitmap_height, "ZBarcode_Buffer_Vector() bitmap_height %d != 0\n", symbol->bitmap_height);

    ZBarcode_Delete(symbol);

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_checks", test_checks, 1, 0, 1 },
        { "test_symbologies", test_symbologies, 0, 0, 0 },
        { "test_input_mode", test_input_mode, 1, 0, 1 },
        { "test_escape_char_process", test_escape_char_process, 1, 1, 1 },
        { "test_cap", test_cap, 1, 0, 0 },
        { "test_cap_compliant_height", test_cap_compliant_height, 0, 0, 0 },
        { "test_encode_file_empty", test_encode_file_empty, 0, 0, 0 },
        { "test_encode_file_too_large", test_encode_file_too_large, 0, 0, 0 },
        { "test_encode_file_unreadable", test_encode_file_unreadable, 0, 0, 0 },
        { "test_encode_file_directory", test_encode_file_directory, 0, 0, 0 },
        { "test_bad_args", test_bad_args, 0, 0, 0 },
        { "test_valid_id", test_valid_id, 0, 0, 0 },
        { "test_error_tag", test_error_tag, 1, 0, 0 },
        { "test_strip_bom", test_strip_bom, 0, 0, 0 },
        { "test_zero_outfile", test_zero_outfile, 0, 0, 0 },
        { "test_clear", test_clear, 0, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
