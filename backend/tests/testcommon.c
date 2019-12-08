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
/* vim: set ts=4 sw=4 et : */
/*
 * Adapted from qrencode/tests/common.c
 * Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
 */

#include <stdio.h>
#include <string.h>
#include "testcommon.h"
#ifndef NO_PNG
#include <png.h>
#include <zlib.h>
#include <setjmp.h>
#endif

extern int module_is_set(const struct zint_symbol *symbol, const int y_coord, const int x_coord);

static int tests = 0;
static int failed = 0;
int assertionFailed = 0;
int assertionNum = 0;
static const char *testName = NULL;
static const char *testFunc = NULL;

void testStartReal(const char *func, const char *name)
{
    tests++;
    testName = name;
    testFunc = func;
    assertionFailed = 0;
    assertionNum = 0;
    printf("_____%d: %s: %s...\n", tests, func, name);
}

void testEnd(int result)
{
    if (testName[0]) {
        printf(".....%d: %s: %s ", tests, testFunc, testName);
    } else {
        printf(".....%d: %s: ", tests, testFunc);
    }
    if (result) {
        puts("FAILED.");
        failed++;
    } else {
        puts("PASSED.");
    }
}

void testFinish(void)
{
    if (testName[0]) {
        printf(".....%d: %s: %s ", tests, testFunc, testName);
    } else {
        printf(".....%d: %s: ", tests, testFunc);
    }
    if (assertionFailed) {
        printf("FAILED. (%d assertions failed.)\n", assertionFailed);
        failed++;
    } else {
        printf("PASSED. (%d assertions passed.)\n", assertionNum);
    }
}

void testReport()
{
    if ( failed ) {
        printf("Total %d tests, %d fails.\n", tests, failed);
        exit(-1);
    } else {
        printf("Total %d tests, all passed.\n", tests);
    }
}

char* testUtilBarcodeName(int symbology) {
    struct item {
        int define;
        char* name;
        int val;
    };
    struct item data[] = {
        { -1, "", 0 },
        { BARCODE_CODE11, "BARCODE_CODE11", 1 },
        { BARCODE_C25MATRIX, "BARCODE_C25MATRIX", 2 },
        { BARCODE_C25INTER, "BARCODE_C25INTER", 3 },
        { BARCODE_C25IATA, "BARCODE_C25IATA", 4 },
        { -1, "", 5 },
        { BARCODE_C25LOGIC, "BARCODE_C25LOGIC", 6 },
        { BARCODE_C25IND, "BARCODE_C25IND", 7 },
        { BARCODE_CODE39, "BARCODE_CODE39", 8 },
        { BARCODE_EXCODE39, "BARCODE_EXCODE39", 9 },
        { -1, "", 10 },
        { -1, "", 11 },
        { -1, "", 12 },
        { BARCODE_EANX, "BARCODE_EANX", 13 },
        { BARCODE_EANX_CHK, "BARCODE_EANX_CHK", 14 },
        { -1, "", 15 },
        { BARCODE_EAN128, "BARCODE_EAN128", 16 },
        { -1, "", 17 },
        { BARCODE_CODABAR, "BARCODE_CODABAR", 18 },
        { -1, "", 19 },
        { BARCODE_CODE128, "BARCODE_CODE128", 20 },
        { BARCODE_DPLEIT, "BARCODE_DPLEIT", 21 },
        { BARCODE_DPIDENT, "BARCODE_DPIDENT", 22 },
        { BARCODE_CODE16K, "BARCODE_CODE16K", 23 },
        { BARCODE_CODE49, "BARCODE_CODE49", 24 },
        { BARCODE_CODE93, "BARCODE_CODE93", 25 },
        { -1, "", 26 },
        { -1, "", 27 },
        { BARCODE_FLAT, "BARCODE_FLAT", 28 },
        { BARCODE_RSS14, "BARCODE_RSS14", 29 },
        { BARCODE_RSS_LTD, "BARCODE_RSS_LTD", 30 },
        { BARCODE_RSS_EXP, "BARCODE_RSS_EXP", 31 },
        { BARCODE_TELEPEN, "BARCODE_TELEPEN", 32 },
        { -1, "", 33 },
        { BARCODE_UPCA, "BARCODE_UPCA", 34 },
        { BARCODE_UPCA_CHK, "BARCODE_UPCA_CHK", 35 },
        { -1, "", 36 },
        { BARCODE_UPCE, "BARCODE_UPCE", 37 },
        { BARCODE_UPCE_CHK, "BARCODE_UPCE_CHK", 38 },
        { -1, "", 39 },
        { BARCODE_POSTNET, "BARCODE_POSTNET", 40 },
        { -1, "", 41 },
        { -1, "", 42 },
        { -1, "", 43 },
        { -1, "", 44 },
        { -1, "", 45 },
        { -1, "", 46 },
        { BARCODE_MSI_PLESSEY, "BARCODE_MSI_PLESSEY", 47 },
        { -1, "", 48 },
        { BARCODE_FIM, "BARCODE_FIM", 49 },
        { BARCODE_LOGMARS, "BARCODE_LOGMARS", 50 },
        { BARCODE_PHARMA, "BARCODE_PHARMA", 51 },
        { BARCODE_PZN, "BARCODE_PZN", 52 },
        { BARCODE_PHARMA_TWO, "BARCODE_PHARMA_TWO", 53 },
        { -1, "", 54 },
        { BARCODE_PDF417, "BARCODE_PDF417", 55 },
        { BARCODE_PDF417TRUNC, "BARCODE_PDF417TRUNC", 56 },
        { BARCODE_MAXICODE, "BARCODE_MAXICODE", 57 },
        { BARCODE_QRCODE, "BARCODE_QRCODE", 58 },
        { -1, "", 59 },
        { BARCODE_CODE128B, "BARCODE_CODE128B", 60 },
        { -1, "", 61 },
        { -1, "", 62 },
        { BARCODE_AUSPOST, "BARCODE_AUSPOST", 63 },
        { -1, "", 64 },
        { -1, "", 65 },
        { BARCODE_AUSREPLY, "BARCODE_AUSREPLY", 66 },
        { BARCODE_AUSROUTE, "BARCODE_AUSROUTE", 67 },
        { BARCODE_AUSREDIRECT, "BARCODE_AUSREDIRECT", 68 },
        { BARCODE_ISBNX, "BARCODE_ISBNX", 69 },
        { BARCODE_RM4SCC, "BARCODE_RM4SCC", 70 },
        { BARCODE_DATAMATRIX, "BARCODE_DATAMATRIX", 71 },
        { BARCODE_EAN14, "BARCODE_EAN14", 72 },
        { BARCODE_VIN, "BARCODE_VIN", 73 },
        { BARCODE_CODABLOCKF, "BARCODE_CODABLOCKF", 74 },
        { BARCODE_NVE18, "BARCODE_NVE18", 75 },
        { BARCODE_JAPANPOST, "BARCODE_JAPANPOST", 76 },
        { BARCODE_KOREAPOST, "BARCODE_KOREAPOST", 77 },
        { -1, "", 78 },
        { BARCODE_RSS14STACK, "BARCODE_RSS14STACK", 79 },
        { BARCODE_RSS14STACK_OMNI, "BARCODE_RSS14STACK_OMNI", 80 },
        { BARCODE_RSS_EXPSTACK, "BARCODE_RSS_EXPSTACK", 81 },
        { BARCODE_PLANET, "BARCODE_PLANET", 82 },
        { -1, "", 83 },
        { BARCODE_MICROPDF417, "BARCODE_MICROPDF417", 84 },
        { BARCODE_ONECODE, "BARCODE_ONECODE", 85 },
        { BARCODE_PLESSEY, "BARCODE_PLESSEY", 86 },
        { BARCODE_TELEPEN_NUM, "BARCODE_TELEPEN_NUM", 87 },
        { -1, "", 88 },
        { BARCODE_ITF14, "BARCODE_ITF14", 89 },
        { BARCODE_KIX, "BARCODE_KIX", 90 },
        { -1, "", 91 },
        { BARCODE_AZTEC, "BARCODE_AZTEC", 92 },
        { BARCODE_DAFT, "BARCODE_DAFT", 93 },
        { -1, "", 94 },
        { -1, "", 95 },
        { -1, "", 96 },
        { BARCODE_MICROQR, "BARCODE_MICROQR", 97 },
        { BARCODE_HIBC_128, "BARCODE_HIBC_128", 98 },
        { BARCODE_HIBC_39, "BARCODE_HIBC_39", 99 },
        { -1, "", 100 },
        { -1, "", 101 },
        { BARCODE_HIBC_DM, "BARCODE_HIBC_DM", 102 },
        { -1, "", 103 },
        { BARCODE_HIBC_QR, "BARCODE_HIBC_QR", 104 },
        { -1, "", 105 },
        { BARCODE_HIBC_PDF, "BARCODE_HIBC_PDF", 106 },
        { -1, "", 107 },
        { BARCODE_HIBC_MICPDF, "BARCODE_HIBC_MICPDF", 108 },
        { -1, "", 109 },
        { BARCODE_HIBC_BLOCKF, "BARCODE_HIBC_BLOCKF", 110 },
        { -1, "", 111 },
        { BARCODE_HIBC_AZTEC, "BARCODE_HIBC_AZTEC", 112 },
        { -1, "", 113 },
        { -1, "", 114 },
        { BARCODE_DOTCODE, "BARCODE_DOTCODE", 115 },
        { BARCODE_HANXIN, "BARCODE_HANXIN", 116 },
        { -1, "", 117 },
        { -1, "", 118 },
        { -1, "", 119 },
        { -1, "", 120 },
        { BARCODE_MAILMARK, "BARCODE_MAILMARK", 121 },
        { -1, "", 122 },
        { -1, "", 123 },
        { -1, "", 124 },
        { -1, "", 125 },
        { -1, "", 126 },
        { -1, "", 127 },
        { BARCODE_AZRUNE, "BARCODE_AZRUNE", 128 },
        { BARCODE_CODE32, "BARCODE_CODE32", 129 },
        { BARCODE_EANX_CC, "BARCODE_EANX_CC", 130 },
        { BARCODE_EAN128_CC, "BARCODE_EAN128_CC", 131 },
        { BARCODE_RSS14_CC, "BARCODE_RSS14_CC", 132 },
        { BARCODE_RSS_LTD_CC, "BARCODE_RSS_LTD_CC", 133 },
        { BARCODE_RSS_EXP_CC, "BARCODE_RSS_EXP_CC", 134 },
        { BARCODE_UPCA_CC, "BARCODE_UPCA_CC", 135 },
        { BARCODE_UPCE_CC, "BARCODE_UPCE_CC", 136 },
        { BARCODE_RSS14STACK_CC, "BARCODE_RSS14STACK_CC", 137 },
        { BARCODE_RSS14_OMNI_CC, "BARCODE_RSS14_OMNI_CC", 138 },
        { BARCODE_RSS_EXPSTACK_CC, "BARCODE_RSS_EXPSTACK_CC", 139 },
        { BARCODE_CHANNEL, "BARCODE_CHANNEL", 140 },
        { BARCODE_CODEONE, "BARCODE_CODEONE", 141 },
        { BARCODE_GRIDMATRIX, "BARCODE_GRIDMATRIX", 142 },
        { BARCODE_UPNQR, "BARCODE_UPNQR", 143 },
        { BARCODE_ULTRA, "BARCODE_ULTRA", 144 },
        { BARCODE_RMQR, "BARCODE_RMQR", 145 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    if (symbology < 0 || symbology >= data_size) {
        return "";
    }
    if (data[symbology].val != symbology || (data[symbology].define != -1 && data[symbology].define != symbology)) { // Self-check
        fprintf(stderr, "testUtilBarcodeName data table out of sync (%d)\n", symbology);
        abort();
    }
    return data[symbology].name;
}

char* testUtilErrorName(int error_number) {
    struct item {
        int define;
        char* name;
        int val;
    };
    struct item data[] = {
        { 0, "0", 0 },
        { -1, "", 1 },
        { ZINT_WARN_INVALID_OPTION, "ZINT_WARN_INVALID_OPTION", 2 },
        { ZINT_WARN_USES_ECI, "ZINT_WARN_USES_ECI", 3 },
        { -1, "", 4 },
        { ZINT_ERROR_TOO_LONG, "ZINT_ERROR_TOO_LONG", 5 },
        { ZINT_ERROR_INVALID_DATA, "ZINT_ERROR_INVALID_DATA", 6 },
        { ZINT_ERROR_INVALID_CHECK, "ZINT_ERROR_INVALID_CHECK", 7 },
        { ZINT_ERROR_INVALID_OPTION, "ZINT_ERROR_INVALID_OPTION", 8 },
        { ZINT_ERROR_ENCODING_PROBLEM, "ZINT_ERROR_ENCODING_PROBLEM", 9 },
        { ZINT_ERROR_FILE_ACCESS, "ZINT_ERROR_FILE_ACCESS", 10 },
        { ZINT_ERROR_MEMORY, "ZINT_ERROR_MEMORY", 11 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    if (error_number < 0 || error_number >= data_size) {
        return "";
    }
    if (data[error_number].val != error_number || (data[error_number].define != -1 && data[error_number].define != error_number)) { // Self-check
        fprintf(stderr, "testUtilErrorName data table out of sync (%d)\n", error_number);
        abort();
    }
    return data[error_number].name;
}

char* testUtilInputModeName(int input_mode) {
    struct item {
        int define;
        char* name;
        int val;
    };
    struct item data[] = {
        { DATA_MODE, "DATA_MODE", 0 },
        { UNICODE_MODE, "UNICODE_MODE", 1 },
        { GS1_MODE, "GS1_MODE", 2 },
        { -1, "", 3 },
        { -1, "", 4 },
        { -1, "", 5 },
        { -1, "", 6 },
        { -1, "", 7 },
        { DATA_MODE | ESCAPE_MODE, "DATA_MODE | ESCAPE_MODE", 8 },
        { UNICODE_MODE | ESCAPE_MODE, "UNICODE_MODE | ESCAPE_MODE", 9 },
        { GS1_MODE | ESCAPE_MODE, "GS1_MODE | ESCAPE_MODE", 10 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    if (input_mode < 0 || input_mode >= data_size) {
        return input_mode == -1 ? "-1" : "";
    }
    if (data[input_mode].val != input_mode || (data[input_mode].define != -1 && data[input_mode].define != input_mode)) { // Self-check
        fprintf(stderr, "testUtilInputModeName data table out of sync (%d)\n", input_mode);
        abort();
    }
    return data[input_mode].name;
}

int testUtilDAFTConvert(const struct zint_symbol* symbol, char* buffer, int buffer_size)
{
    buffer[0] = '\0';
    char* b = buffer;
    for (int i = 0; i < symbol->width && b < buffer + buffer_size; i += 2) {
        if (module_is_set(symbol, 0, i) && module_is_set(symbol, 2, i)) {
            *b++ = 'F';
        } else if (module_is_set(symbol, 0, i)) {
            *b++ = 'A';
        } else if (module_is_set(symbol, 2, i)) {
            *b++ = 'D';
        } else {
            *b++ = 'T';
        }
    }
    if (b == buffer + buffer_size) {
        return FALSE;
    }
    *b = '\0';
    return TRUE;
}

/* Is string valid UTF-8? */
int testUtilIsValidUTF8(const unsigned char str[], const size_t length) {
    int i;
    unsigned int codepoint, state = 0;

    for (i = 0; i < length; i++) {
        if (decode_utf8(&state, &codepoint, str[i]) == 12) {
            return 0;
        }
    }

    return state == 0;
}

char* testUtilEscape(char* buffer, int length, char* escaped, int escaped_size)
{
    int i;
    unsigned char* b = buffer;
    unsigned char* be = buffer + length;
    int non_utf8 = !testUtilIsValidUTF8(buffer, length);

    for (i = 0; b < be && i < escaped_size; b++) {
        if (non_utf8 || *b < ' ' || *b == '\177') {
            if (i < escaped_size - 4) {
                sprintf(escaped + i, "\\%.3o", *b);
            }
            i += 4;
        } else if (*b == '\\' || *b == '"') {
            if (i < escaped_size - 2) {
                escaped[i] = '\\';
                escaped[i + 1] = *b;
            }
            i += 2;
        } else if (b + 1 < be && *b == 0xC2 && *(b + 1) < 0xA0) {
            if (i < escaped_size - 8) {
                sprintf(escaped + i, "\\%.3o\\%.3o", *b, *(b + 1));
            }
            i += 8;
            b++;
        } else {
            escaped[i++] = *b;
        }
    }
    if (i >= escaped_size) {
        return NULL;
    }
    escaped[i] = '\0';
    return escaped;
}

char* testUtilReadCSVField(char* buffer, char* field, int field_size)
{
    int i;
    char* b = buffer;
    for (i = 0; i < field_size && *b && *b != ',' && *b != '\n' && *b != '\r'; i++) {
        field[i] = *b++;
    }
    if (i == field_size) {
        return NULL;
    }
    field[i] = '\0';
    return b;
}

int testUtilSymbolCmp(const struct zint_symbol* a, const struct zint_symbol* b)
{
    if (a->symbology != b->symbology) {
        return 1;
    }
    if (a->rows != b->rows) {
        return 2;
    }
    if (a->width != b->width) {
        return 3;
    }
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < a->width; j++) {
            if (module_is_set(a, i, j) != module_is_set(b, i, j)) {
                return 4;
            }
        }
    }
    if (a->height != b->height) {
        return 5;
    }
    if (a->whitespace_width != b->whitespace_width) {
        return 6;
    }
    if (a->border_width != b->border_width) {
        return 7;
    }
    if (a->output_options != b->output_options) {
        return 8;
    }
    if (a->scale != b->scale) {
        return 9;
    }

    return 0;
}

struct zint_vector* testUtilVectorCpy(const struct zint_vector* in)
{
    struct zint_vector* out = (struct zint_vector*)malloc(sizeof(struct zint_vector));
    out->width = in->width;
    out->height = in->height;
    out->rectangles = NULL;
    out->strings = NULL;
    out->circles = NULL;
    out->hexagons = NULL;

    struct zint_vector_rect* rect;
    struct zint_vector_string* string;
    struct zint_vector_circle* circle;
    struct zint_vector_hexagon* hexagon;

    struct zint_vector_rect** outrect;
    struct zint_vector_string** outstring;
    struct zint_vector_circle** outcircle;
    struct zint_vector_hexagon** outhexagon;

    // Copy rectangles
    rect = in->rectangles;
    outrect = &(out->rectangles);
    while (rect) {
        *outrect = (struct zint_vector_rect*) malloc(sizeof(struct zint_vector_rect));
        memcpy(*outrect, rect, sizeof(struct zint_vector_rect));
        outrect = &((*outrect)->next);
        rect = rect->next;
    }
    *outrect = NULL;

    // Copy Strings
    string = in->strings;
    outstring = &(out->strings);
    while (string) {
        *outstring = (struct zint_vector_string*) malloc(sizeof(struct zint_vector_string));
        memcpy(*outstring, string, sizeof(struct zint_vector_string));
        (*outstring)->text = (unsigned char*) malloc(sizeof(unsigned char) * (ustrlen(string->text) + 1));
        ustrcpy((*outstring)->text, string->text);
        outstring = &((*outstring)->next);
        string = string->next;
    }
    *outstring = NULL;

    // Copy Circles
    circle = in->circles;
    outcircle = &(out->circles);
    while (circle) {
        *outcircle = (struct zint_vector_circle*) malloc(sizeof(struct zint_vector_circle));
        memcpy(*outcircle, circle, sizeof(struct zint_vector_circle));
        outcircle = &((*outcircle)->next);
        circle = circle->next;
    }
    *outcircle = NULL;

    // Copy Hexagons
    hexagon = in->hexagons;
    outhexagon = &(out->hexagons);
    while (hexagon) {
        *outhexagon = (struct zint_vector_hexagon*) malloc(sizeof(struct zint_vector_hexagon));
        memcpy(*outhexagon, hexagon, sizeof(struct zint_vector_hexagon));
        outhexagon = &((*outhexagon)->next);
        hexagon = hexagon->next;
    }
    *outhexagon = NULL;

    return out;
}

int testUtilVectorCmp(const struct zint_vector* a, const struct zint_vector* b)
{
    struct zint_vector_rect* arect;
    struct zint_vector_string* astring;
    struct zint_vector_circle* acircle;
    struct zint_vector_hexagon* ahexagon;

    struct zint_vector_rect* brect;
    struct zint_vector_string* bstring;
    struct zint_vector_circle* bcircle;
    struct zint_vector_hexagon* bhexagon;

    if (a->width != b->width) {
        return 1;
    }
    if (a->height != b->height) {
        return 2;
    }

    // Compare rectangles
    arect = a->rectangles;
    brect = b->rectangles;
    while (arect) {
        if (!brect) {
            return 11;
        }
        if (arect->x != brect->x) {
            return 12;
        }
        if (arect->y != brect->y) {
            return 13;
        }
        if (arect->height != brect->height) {
            return 14;
        }
        if (arect->width != brect->width) {
            return 15;
        }
        if (arect->colour != brect->colour) {
            return 16;
        }
        arect = arect->next;
        brect = brect->next;
    }
    if (brect) {
        return 10;
    }

    // Compare strings
    astring = a->strings;
    bstring = b->strings;
    while (astring) {
        if (!bstring) {
            return 21;
        }
        if (astring->x != bstring->x) {
            return 22;
        }
        if (astring->y != bstring->y) {
            return 23;
        }
        if (astring->fsize != bstring->fsize) {
            return 24;
        }
        if (astring->width != bstring->width) {
            return 25;
        }
        if (astring->length != bstring->length) {
            return 26;
        }
        if (ustrlen(astring->text) != ustrlen(bstring->text)) {
            return 27;
        }
        if (strcmp((const char*)astring->text, (const char*)bstring->text) != 0) {
            return 28;
        }
        astring = astring->next;
        bstring = bstring->next;
    }
    if (bstring) {
        return 20;
    }

    // Compare circles
    acircle = a->circles;
    bcircle = b->circles;
    while (acircle) {
        if (!bcircle) {
            return 31;
        }
        if (acircle->x != bcircle->x) {
            return 32;
        }
        if (acircle->y != bcircle->y) {
            return 33;
        }
        if (acircle->diameter != bcircle->diameter) {
            return 34;
        }
        if (acircle->colour != bcircle->colour) {
            return 35;
        }
        acircle = acircle->next;
        bcircle = bcircle->next;
    }
    if (bcircle) {
        return 30;
    }

    // Compare hexagons
    ahexagon = a->hexagons;
    bhexagon = b->hexagons;
    while (ahexagon) {
        if (!bhexagon) {
            return 41;
        }
        if (ahexagon->x != bhexagon->x) {
            return 42;
        }
        if (ahexagon->y != bhexagon->y) {
            return 43;
        }
        if (ahexagon->diameter != bhexagon->diameter) {
            return 44;
        }
        ahexagon = ahexagon->next;
        bhexagon = bhexagon->next;
    }
    if (bhexagon) {
        return 40;
    }

    return 0;
}

void testUtilLargeDump(const char* name, const short int reg[])
{
    unsigned words[4];
    words[0] = words[1] = words[2] = words[3] = 0;
    int w = 0;
    for (int i = 0; i < 112; i += 32 ) {
        for (int j = 0; j < 32 && i + j < 112; j++) {
            if (reg[i + j]) {
                words[w] += 1 << j;
            }
        }
        w++;
    }
    printf("%4x 0x%08x%08x%08x %s", words[3], words[2], words[1], words[0], name);
}

void testUtilModulesDump(const struct zint_symbol* symbol, char* prefix, char* postfix)
{
    int r, w;
    for (r = 0; r < symbol->rows; r++) {
        if (*prefix) {
            fputs(prefix, stdout);
        }
        putchar('"');
        for (w = 0; w < symbol->width; w++) {
            putchar(module_is_set(symbol, r, w) ? '1' : '0');
        }
        putchar('"');
        if (*postfix) {
            fputs(postfix, stdout);
        }
    }
}

int testUtilModulesCmp(const struct zint_symbol* symbol, const char* expected, int* row, int* width)
{
    const char* e = expected;
    const char* ep = expected + strlen(expected);
    int r, w = 0;
    for (r = 0; r < symbol->rows && e < ep; r++) {
        for (w = 0; w < symbol->width && e < ep; w++) {
            if (module_is_set(symbol, r, w) != (*e == '1')) {
                *row = r;
                *width = w;
                return 1 /*fail*/;
            }
            e++;
        }
    }
    *row = r;
    *width = w;
    return e != ep || r != symbol->rows || w != symbol->width ? 1 /*fail*/ : 0 /*success*/;
}

int testUtilModulesDumpHex(const struct zint_symbol* symbol, char dump[], int dump_size)
{
    int i, r;
    char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8',
        '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    int space = 0;
    char* d = dump;
    char* de = dump + dump_size;

    for (r = 0; r < symbol->rows && d < de; r++) {
        int byt = 0;
        for (i = 0; i < symbol->width && d < de; i++) {
            if (space == 2) {
                *d++ = ' ';
                space = 0;
            }
            byt = byt << 1;
            if (module_is_set(symbol, r, i)) {
                byt += 1;
            }
            if (d < de && ((i + 1) % 4) == 0) {
                *d++ = hex[byt];
                space++;
                byt = 0;
            }
        }
        if (d < de && (symbol->width % 4) != 0) {
            byt = byt << (4 - (symbol->width % 4));
            *d++ = hex[byt];
            space++;
        }
    }
    if (d == de) {
        return -1;
    }
    *d = '\0';
    return d - dump;
}

int testUtilExists(char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return 0;
    }
    fclose(fp);
    return 1;
}

int testUtilCmpPngs(char* png1, char* png2)
{
    int ret = -1;
#ifndef NO_PNG
    FILE* fp1;
    FILE* fp2;
    png_structp png_ptr1, png_ptr2;
    png_infop info_ptr1, info_ptr2;
    int width1, height1, width2, height2;
    png_byte color_type1, color_type2;
    png_byte bit_depth1, bit_depth2;
    png_bytep row1 = NULL, row2 = NULL;
    size_t rowbytes1, rowbytes2;
    int r;

    fp1 = fopen(png1, "rb");
    if (!fp1) {
        return 2;
    }
    fp2 = fopen(png2, "rb");
    if (!fp2) {
        fclose(fp1);
        return 3;
    }

    png_ptr1 = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
    if (!png_ptr1) {
        fclose(fp1);
        fclose(fp2);
        return 4;
    }
    info_ptr1 = png_create_info_struct(png_ptr1);
    if (!info_ptr1) {
        png_destroy_read_struct(&png_ptr1, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 5;
    }

    png_ptr2 = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
    if (!png_ptr2) {
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 6;
    }
    info_ptr2 = png_create_info_struct(png_ptr2);
    if (!info_ptr2) {
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
        png_destroy_read_struct(&png_ptr2, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 7;
    }

    if (setjmp(png_jmpbuf(png_ptr1))) {
        if (row1) {
            free(row1);
        }
        if (row2) {
            free(row2);
        }
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 8;
    }
    if (setjmp(png_jmpbuf(png_ptr2))) {
        if (row1) {
            free(row1);
        }
        if (row2) {
            free(row2);
        }
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 9;
    }

    png_init_io(png_ptr1, fp1);
    png_init_io(png_ptr2, fp2);

    png_read_info(png_ptr1, info_ptr1);
    png_read_info(png_ptr2, info_ptr2);

    width1 = png_get_image_width(png_ptr1, info_ptr1);
    height1 = png_get_image_height(png_ptr1, info_ptr1);
    width2 = png_get_image_width(png_ptr2, info_ptr2);
    height2 = png_get_image_height(png_ptr2, info_ptr2);

    if (width1 != width2 || height1 != height2) {
        printf("width1 %d, width2 %d, height1 %d, height2 %d\n", width1, width2, height1, height2);
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 10;
    }

    color_type1 = png_get_color_type(png_ptr1, info_ptr1);
    bit_depth1 = png_get_bit_depth(png_ptr1, info_ptr1);
    if (bit_depth1 == 16) {
        png_set_scale_16(png_ptr1);
    }
    if (color_type1 == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr1);
    }
    if (color_type1 == PNG_COLOR_TYPE_GRAY && bit_depth1 < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr1);
    }
    if (png_get_valid(png_ptr1, info_ptr1, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr1);
    }
    if (color_type1 == PNG_COLOR_TYPE_RGB || color_type1 == PNG_COLOR_TYPE_GRAY || color_type1 == PNG_COLOR_TYPE_PALETTE) {
        png_set_filler(png_ptr1, 0xFF, PNG_FILLER_AFTER);
    }
    if (color_type1 == PNG_COLOR_TYPE_GRAY || color_type1 == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr1);
    }

    color_type2 = png_get_color_type(png_ptr2, info_ptr2);
    bit_depth2 = png_get_bit_depth(png_ptr2, info_ptr2);
    if (bit_depth2 == 16) {
        png_set_scale_16(png_ptr2);
    }
    if (color_type2 == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr2);
    }
    if (color_type2 == PNG_COLOR_TYPE_GRAY && bit_depth2 < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr2);
    }
    if (png_get_valid(png_ptr2, info_ptr2, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr2);
    }
    if (color_type2 == PNG_COLOR_TYPE_RGB || color_type2 == PNG_COLOR_TYPE_GRAY || color_type2 == PNG_COLOR_TYPE_PALETTE) {
        png_set_filler(png_ptr2, 0xFF, PNG_FILLER_AFTER);
    }
    if (color_type2 == PNG_COLOR_TYPE_GRAY || color_type2 == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr2);
    }

    png_read_update_info(png_ptr1, info_ptr1);
    png_read_update_info(png_ptr2, info_ptr2);

    rowbytes1 = png_get_rowbytes(png_ptr1, info_ptr1);
    rowbytes2 = png_get_rowbytes(png_ptr2, info_ptr2);
    if (rowbytes1 != rowbytes2) {
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 11;
    }

    row1 = (png_byte*)malloc(rowbytes1);
    if (!row1) {
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 12;
    }
    row2 = (png_byte*)malloc(rowbytes2);
    if (!row2) {
        free(row1);
        png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
        png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp)NULL);
        fclose(fp1);
        fclose(fp2);
        return 13;
    }

    for (r = 0; r < height1; r++) {
        png_read_row(png_ptr1, row1, NULL);
        png_read_row(png_ptr2, row2, NULL);
        if (memcmp(row1, row2, rowbytes1) != 0) {
            break;
        }
    }
    ret = r == height1 ? 0 : 20;

    free(row1);
    free(row2);
    png_destroy_read_struct(&png_ptr1, &info_ptr1, (png_infopp)NULL);
    png_destroy_read_struct(&png_ptr2, &info_ptr2, (png_infopp)NULL);
    fclose(fp1);
    fclose(fp2);
#endif
    return ret;
}

static int testUtilCmpTxts(char* txt1, char* txt2)
{
    int ret = -1;
    FILE* fp1;
    FILE* fp2;
    char buf1[1024];
    char buf2[1024];
    size_t len1 = 0, len2 = 0;

    fp1 = fopen(txt1, "r");
    if (!fp1) {
        return 2;
    }
    fp2 = fopen(txt2, "r");
    if (!fp2) {
        fclose(fp1);
        return 3;
    }

    while (1) {
        if (fgets(buf1, sizeof(buf1), fp1) == NULL) {
            if (fgets(buf2, sizeof(buf2), fp2) != NULL) {
                ret = 4;
                break;
            }
            break;
        }
        if (fgets(buf2, sizeof(buf2), fp2) == NULL) {
            ret = 5;
            break;
        }
        len1 = strlen(buf1);
        len2 = strlen(buf2);
        if (len1 != len2) {
            ret = 6;
            break;
        }
        if (strcmp(buf1, buf2) != 0) {
            ret = 7;
            break;
        }
    }
    if (ret == -1) {
        ret = feof(fp1) && feof(fp2) ? 0 : 20;
    }
    fclose(fp1);
    fclose(fp2);

    return ret;
}

static int testUtilCmpBins(char* bin1, char* bin2)
{
    int ret = -1;
    FILE* fp1;
    FILE* fp2;
    char buf1[1024];
    char buf2[1024];
    size_t len1 = 0, len2 = 0;

    fp1 = fopen(bin1, "rb");
    if (!fp1) {
        return 2;
    }
    fp2 = fopen(bin2, "rb");
    if (!fp2) {
        fclose(fp1);
        return 3;
    }

    do {
        len1 = fread(buf1, 1, sizeof(buf1), fp1);
        len2 = fread(buf2, 1, sizeof(buf2), fp2);
        if (len1 != len2) {
            ret = 6;
            break;
        }
        if (memcmp(buf1, buf2, len1) != 0) {
            ret = 7;
            break;
        }
    } while (!feof(fp1) && !feof(fp2));

    if (ret == -1) {
        ret = feof(fp1) && feof(fp2) ? 0 : 20;
    }
    fclose(fp1);
    fclose(fp2);

    return ret;
}

int testUtilCmpSvgs(char* svg1, char* svg2)
{
    return testUtilCmpTxts(svg1, svg2);
}

int testUtilCmpEpss(char* eps1, char* eps2)
{
    int ret = -1;
    FILE* fp1;
    FILE* fp2;
    char buf1[1024];
    char buf2[1024];
    size_t len1 = 0, len2 = 0;
    char first_line[] = "%!PS-Adobe-3.0 EPSF-3.0\n";
    char second_line_start[] = "%%Creator: Zint ";

    fp1 = fopen(eps1, "r");
    if (!fp1) {
        return 2;
    }
    fp2 = fopen(eps2, "r");
    if (!fp2) {
        fclose(fp1);
        return 3;
    }

    // Preprocess the 1st 2 lines to avoid comparing changeable Zint version in 2nd line
    if (fgets(buf1, sizeof(buf1), fp1) == NULL || strcmp(buf1, first_line) != 0
            || fgets(buf2, sizeof(buf2), fp2) == NULL || strcmp(buf2, first_line) != 0) {
        ret = 10;
    } else if (fgets(buf1, sizeof(buf1), fp1) == NULL || strncmp(buf1, second_line_start, sizeof(second_line_start) - 1) != 0
            || fgets(buf2, sizeof(buf2), fp2) == NULL || strncmp(buf2, second_line_start, sizeof(second_line_start) - 1) != 0) {
        ret = 11;
    }

    if (ret == -1) {
        while (1) {
            if (fgets(buf1, sizeof(buf1), fp1) == NULL) {
                if (fgets(buf2, sizeof(buf2), fp2) != NULL) {
                    ret = 4;
                    break;
                }
                break;
            }
            if (fgets(buf2, sizeof(buf2), fp2) == NULL) {
                ret = 5;
                break;
            }
            len1 = strlen(buf1);
            len2 = strlen(buf2);
            if (len1 != len2) {
                ret = 6;
                break;
            }
            if (strcmp(buf1, buf2) != 0) {
                ret = 7;
                break;
            }
        }
        if (ret == -1) {
            ret = feof(fp1) && feof(fp2) ? 0 : 20;
        }
    }
    fclose(fp1);
    fclose(fp2);

    return ret;
}

int testUtilCmpEmfs(char* emf1, char* emf2)
{
    return testUtilCmpBins(emf1, emf2);
}

int testUtilCmpGifs(char* gif1, char* gif2)
{
    return testUtilCmpBins(gif1, gif2);
}

int testUtilCmpBmps(char* bmp1, char* bmp2)
{
    return testUtilCmpBins(bmp1, bmp2);
}

int testUtilCmpPcxs(char* pcx1, char* pcx2)
{
    return testUtilCmpBins(pcx1, pcx2);
}
