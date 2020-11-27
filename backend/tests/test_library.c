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
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static void test_checks(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        char *data;
        int length;
        int input_mode;
        int eci;
        float dot_size;
        int warn_level;
        int ret;

        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "1234", -1, -1, 3, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 217: Symbology does not support ECI switching" },
        /*  1*/ { BARCODE_CODE128, "1234", -1, -1, 0, -1, -1, 0, "" },
        /*  2*/ { BARCODE_QRCODE, "1234", -1, -1, 3, -1, -1, 0, "" },
        /*  3*/ { BARCODE_QRCODE, "1234", -1, -1, 999999 + 1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 218: Invalid ECI mode" },
        /*  4*/ { BARCODE_CODE128, "1234", -1, -1, -1, 20.1, -1, ZINT_ERROR_INVALID_OPTION, "Error 221: Invalid dot size" },
        /*  5*/ { BARCODE_CODE128, "1234", -1, GS1_MODE, -1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 220: Selected symbology does not support GS1 mode" },
        /*  6*/ { BARCODE_GS1_128, "[21]12\0004", 8, GS1_MODE, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 262: NUL characters not permitted in GS1 mode" },
        /*  7*/ { BARCODE_GS1_128, "[21]12é4", -1, GS1_MODE, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1" },
        /*  8*/ { BARCODE_GS1_128, "[21]12\0074", -1, GS1_MODE, -1, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 251: Control characters are not supported by GS1" },
        /*  9*/ { BARCODE_GS1_128, "[21]1234", -1, GS1_MODE, -1, -1, -1, 0, "" },
        /* 10*/ { 0, "1", -1, -1, -1, -1, -1, ZINT_WARN_INVALID_OPTION, "Warning 206: Symbology out of range" },
        /* 11*/ { 0, "1", -1, -1, -1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range" },
        /* 12*/ { 0, "1", -1, -1, 1, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 217: Symbology does not support ECI switching" }, // Not supporting beats invalid ECI
        /* 13*/ { 0, "1", -1, -1, 1, -1, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range" },
        /* 14*/ { 0, "1", -1, -1, -1, 0.009, -1, ZINT_ERROR_INVALID_OPTION, "Error 221: Invalid dot size" },
        /* 15*/ { 0, "1", -1, -1, -1, 0.009, WARN_FAIL_ALL, ZINT_ERROR_INVALID_OPTION, "Error 206: Symbology out of range" },
        /* 16*/ { 0, "1", -1, -1, 1, 0.009, -1, ZINT_ERROR_INVALID_OPTION, "Error 221: Invalid dot size" }, // Invalid dot size beats invalid ECI
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].input_mode != -1) {
            symbol->input_mode = data[i].input_mode;
        }
        if (data[i].eci != -1) {
            symbol->eci = data[i].eci;
        }
        if (data[i].dot_size != -1) {
            symbol->dot_size = data[i].dot_size;
        }
        if (data[i].warn_level != -1) {
            symbol->warn_level = data[i].warn_level;
        }
        symbol->debug |= debug;

        int length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret, symbol->errtxt);

        ret = strcmp(symbol->errtxt, data[i].expected);
        assert_zero(ret, "i:%d (%d) strcmp(%s, %s) %d != 0\n", i, data[i].symbology, symbol->errtxt, data[i].expected, ret);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input_mode(int index, int debug) {

    testStart("");

    int ret;
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
        /*  4*/ { "[01]12345678901234", GS1_MODE, 0, GS1_MODE },
        /*  5*/ { "[01]12345678901234", GS1_MODE | ESCAPE_MODE, 0, GS1_MODE | ESCAPE_MODE },
        /*  6*/ { "1234", 4 | ESCAPE_MODE, 0, DATA_MODE }, // Unknown mode reset to bare DATA_MODE
        /*  7*/ { "1234", -1, 0, DATA_MODE },
        /*  8*/ { "1234", DATA_MODE | 0x10, 0, DATA_MODE | 0x10 }, // Unknown flags kept (but ignored)
        /*  9*/ { "1234", UNICODE_MODE | 0x10, 0, UNICODE_MODE | 0x10 },
        /* 10*/ { "[01]12345678901234", GS1_MODE | 0x20, 0, GS1_MODE | 0x20 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_CODE49; // Supports GS1
        symbol->input_mode = data[i].input_mode;
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(symbol->input_mode, data[i].expected_input_mode, "i:%d symbol->input_mode %d != %d\n", i, symbol->input_mode, data[i].expected_input_mode);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_escape_char_process(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int input_mode;
        char *data;
        int ret;
        int expected_width;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { DATA_MODE, "\\0\\E\\a\\b\\t\\n\\v\\f\\r\\e\\G\\R\\x81\\\\", 0, 200, "(18) 103 64 68 71 72 73 74 75 76 77 91 93 94 101 65 60 44 106", "" },
        /*  1*/ { DATA_MODE, "\\c", ZINT_ERROR_INVALID_DATA, 0, "Error 234: Unrecognised escape character in input data", "" },
        /*  2*/ { DATA_MODE, "\\", ZINT_ERROR_INVALID_DATA, 0, "Error 236: Incomplete escape character in input data", "" },
        /*  3*/ { DATA_MODE, "\\x", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete escape character in input data", "" },
        /*  4*/ { DATA_MODE, "\\x1", ZINT_ERROR_INVALID_DATA, 0, "Error 232: Incomplete escape character in input data", "" },
        /*  5*/ { DATA_MODE, "\\x1g", ZINT_ERROR_INVALID_DATA, 0, "Error 233: Corrupt escape character in input data", "" },
        /*  6*/ { UNICODE_MODE, "\\xA01\\xFF", 0, 90, "(8) 104 100 0 17 100 95 100 106", "" },
        /*  7*/ { UNICODE_MODE, "\\u00A01\\u00FF", 0, 90, "(8) 104 100 0 17 100 95 100 106", "" },
        /*  8*/ { DATA_MODE, "\\xc3\\xbF", 0, 79, "(7) 104 100 35 100 31 80 106", "" },
        /*  9*/ { DATA_MODE, "\\u00fF", 0, 79, "(7) 104 100 35 100 31 80 106", "" },
        /* 10*/ { DATA_MODE, "\\u", ZINT_ERROR_INVALID_DATA, 0, "Error 235: Incomplete unicode escape character in input data", "" },
        /* 11*/ { DATA_MODE, "\\uF", ZINT_ERROR_INVALID_DATA, 0, "Error 235: Incomplete unicode escape character in input data", "" },
        /* 12*/ { DATA_MODE, "\\u0F", ZINT_ERROR_INVALID_DATA, 0, "Error 235: Incomplete unicode escape character in input data", "" },
        /* 13*/ { DATA_MODE, "\\uFG", ZINT_ERROR_INVALID_DATA, 0, "Error 235: Incomplete unicode escape character in input data", "" },
        /* 14*/ { DATA_MODE, "\\u00F", ZINT_ERROR_INVALID_DATA, 0, "Error 235: Incomplete unicode escape character in input data", "" },
        /* 15*/ { DATA_MODE, "\\u00FG", ZINT_ERROR_INVALID_DATA, 0, "Error 236: Corrupt unicode escape character in input data", "" },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = testUtilSetSymbol(symbol, BARCODE_CODE128, data[i].input_mode | ESCAPE_MODE, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < 5) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
                assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_cap(int index) {

    testStart("");

    unsigned int ret;
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
        /* 3*/ { BARCODE_QRCODE, ZINT_CAP_HRT | ZINT_CAP_ECI | ZINT_CAP_GS1 | ZINT_CAP_DOTTY | ZINT_CAP_READER_INIT | ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_MASK, ZINT_CAP_ECI | ZINT_CAP_GS1 | ZINT_CAP_DOTTY | ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_MASK },
        /* 4*/ { BARCODE_EANX_CC, ZINT_CAP_HRT | ZINT_CAP_COMPOSITE | ZINT_CAP_EXTENDABLE | ZINT_CAP_ECI | ZINT_CAP_GS1, ZINT_CAP_HRT | ZINT_CAP_COMPOSITE | ZINT_CAP_EXTENDABLE | ZINT_CAP_GS1 },
        /* 5*/ { BARCODE_HANXIN, ZINT_CAP_DOTTY | ZINT_CAP_FIXED_RATIO | ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_MASK, ZINT_CAP_DOTTY | ZINT_CAP_FIXED_RATIO | ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_MASK },
        /* 6*/ { BARCODE_CODE11, ZINT_CAP_DOTTY | ZINT_CAP_FIXED_RATIO | ZINT_CAP_READER_INIT | ZINT_CAP_FULL_MULTIBYTE, 0 },
        /* 7*/ { BARCODE_POSTNET, ZINT_CAP_HRT | ZINT_CAP_STACKABLE | ZINT_CAP_EXTENDABLE | ZINT_CAP_COMPOSITE | ZINT_CAP_ECI | ZINT_CAP_GS1 | ZINT_CAP_DOTTY | ZINT_CAP_FIXED_RATIO | ZINT_CAP_READER_INIT | ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_MASK, 0 },
        /* 8*/ { 0, 0, 0 },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        ret = ZBarcode_Cap(data[i].symbology, data[i].cap_flag);
        assert_equal(ret, data[i].expected, "i:%d ZBarcode_Cap(%s, 0x%X) 0x%X != 0x%X\n", i, testUtilBarcodeName(data[i].symbology), data[i].cap_flag, ret, data[i].expected);
    }

    testFinish();
}

// #181 Nico Gunkel OSS-Fuzz
static void test_encode_file_zero_length(void) {

    testStart("");

    int ret;
    char filename[] = "in.bin";
    int fd;

    struct zint_symbol *symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    (void)remove(filename); // In case junk hanging around
    fd = creat(filename, S_IRUSR);
    assert_nonzero(fd, "Input file not created\n");
    assert_zero(close(fd), "close(%s) != 0\n", filename);

    ret = ZBarcode_Encode_File(symbol, filename);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "ret %d != ZINT_ERROR_INVALID_DATA\n", ret);

    assert_zero(remove(filename), "remove(%s) != 0\n", filename);

    ZBarcode_Delete(symbol);

    testFinish();
}

// #181 Nico Gunkel OSS-Fuzz (buffer not freed on fread() error) Note: unable to reproduce fread() error using this method
static void test_encode_file_directory(void) {

    testStart("");

    int ret;
    char dirname[] = "in_dir";

    struct zint_symbol *symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    (void)rmdir(dirname); // In case junk hanging around
    assert_zero(mkdir(dirname, 0700), "mkdir(%s, 0700) != 0\n", dirname);

    ret = ZBarcode_Encode_File(symbol, dirname);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "ret %d != ZINT_ERROR_INVALID_DATA (%s)\n", ret, symbol->errtxt);

    assert_zero(rmdir(dirname), "rmdir(%s) != 0\n", dirname);

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_bad_args(void) {

    testStart("");

    int ret;
    char *data = "1";
    char *filename = "1.png";
    char *empty = "";

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

    struct zint_symbol *symbol = ZBarcode_Create();
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

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_valid_id(void) {

    testStart("");

    int ret;
    const char *name;

    for (int symbol_id = -1; symbol_id < 160; symbol_id++) {
        ret = ZBarcode_ValidID(symbol_id);
        name = testUtilBarcodeName(symbol_id);
        assert_nonnull(name, "testUtilBarcodeName(%d) NULL\n", symbol_id);
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

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_checks", test_checks, 1, 0, 1 },
        { "test_input_mode", test_input_mode, 1, 0, 1 },
        { "test_escape_char_process", test_escape_char_process, 1, 1, 1 },
        { "test_cap", test_cap, 1, 0, 0 },
        { "test_encode_file_zero_length", test_encode_file_zero_length, 0, 0, 0 },
        { "test_encode_file_directory", test_encode_file_directory, 0, 0, 0 },
        { "test_bad_args", test_bad_args, 0, 0, 0 },
        { "test_valid_id", test_valid_id, 0, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
