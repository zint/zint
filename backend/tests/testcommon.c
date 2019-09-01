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
 * Adapted from qrencode/tests/common.c
 * Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
 */

#include <stdio.h>
#include <string.h>
#include "testcommon.h"

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

struct zint_render* testUtilRenderCpy(const struct zint_render* in)
{
    struct zint_render* out = (struct zint_render*)malloc(sizeof(struct zint_render));
    out->width = in->width;
    out->height = in->height;
    out->lines = NULL;
    out->strings = NULL;
    out->rings = NULL;
    out->hexagons = NULL;

    struct zint_render_line* line;
    struct zint_render_string* string;
    struct zint_render_ring* ring;
    struct zint_render_hexagon* hexagon;

    struct zint_render_line** outline;
    struct zint_render_string** outstring;
    struct zint_render_ring** outring;
    struct zint_render_hexagon** outhexagon;

    // Copy lines
    line = in->lines;
    outline = &(out->lines);
    while (line) {
        *outline = (struct zint_render_line*) malloc(sizeof(struct zint_render_line));
        memcpy(*outline, line, sizeof(struct zint_render_line));
        outline = &((*outline)->next);
        line = line->next;
    }
    *outline = NULL;

    // Copy Strings
    string = in->strings;
    outstring = &(out->strings);
    while (string) {
        *outstring = (struct zint_render_string*) malloc(sizeof(struct zint_render_string));
        memcpy(*outstring, string, sizeof(struct zint_render_string));
        (*outstring)->text = (unsigned char*) malloc(sizeof(unsigned char) * (strlen(string->text) + 1));
        strcpy((*outstring)->text, string->text);
        outstring = &((*outstring)->next);
        string = string->next;
    }
    *outstring = NULL;

    // Copy Rings
    ring = in->rings;
    outring = &(out->rings);
    while (ring) {
        *outring = (struct zint_render_ring*) malloc(sizeof(struct zint_render_ring));
        memcpy(*outring, ring, sizeof(struct zint_render_ring));
        outring = &((*outring)->next);
        ring = ring->next;
    }
    *outstring = NULL;

    // Copy Hexagons
    hexagon = in->hexagons;
    outhexagon = &(out->hexagons);
    while (hexagon) {
        *outhexagon = (struct zint_render_hexagon*) malloc(sizeof(struct zint_render_hexagon));
        memcpy(*outhexagon, hexagon, sizeof(struct zint_render_hexagon));
        outhexagon = &((*outhexagon)->next);
        hexagon = hexagon->next;
    }
    *outhexagon = NULL;

    return out;
}

int testUtilRenderCmp(const struct zint_render* a, const struct zint_render* b)
{
    struct zint_render_line* aline;
    struct zint_render_string* astring;
    struct zint_render_ring* aring;
    struct zint_render_hexagon* ahexagon;

    struct zint_render_line* bline;
    struct zint_render_string* bstring;
    struct zint_render_ring* bring;
    struct zint_render_hexagon* bhexagon;

    if (a->width != b->width) {
        return 1;
    }
    if (a->height != b->height) {
        return 2;
    }

    // Compare lines
    aline = a->lines;
    bline = b->lines;
    while (aline) {
        if (!bline) {
            return 11;
        }
        if (aline->x != bline->x) {
            return 12;
        }
        if (aline->y != bline->y) {
            return 13;
        }
        if (aline->length != bline->length) {
            return 14;
        }
        if (aline->width != bline->width) {
            return 15;
        }
        aline = aline->next;
        bline = bline->next;
    }
    if (bline) {
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
        if (strlen(astring->text) != strlen(bstring->text)) {
            return 27;
        }
        if (strcmp(astring->text, bstring->text) != 0) {
            return 28;
        }
        astring = astring->next;
        bstring = bstring->next;
    }
    if (bstring) {
        return 20;
    }

    // Compare rings
    aring = a->rings;
    bring = b->rings;
    while (aring) {
        if (!bring) {
            return 31;
        }
        if (aring->x != bring->x) {
            return 32;
        }
        if (aring->y != bring->y) {
            return 33;
        }
        if (aring->radius != bring->radius) {
            return 34;
        }
        if (aring->line_width != bring->line_width) {
            return 35;
        }
        aring = aring->next;
        bring = bring->next;
    }
    if (bring) {
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
        if (ahexagon->height != bhexagon->height) {
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

void testUtilModulesDump(const struct zint_symbol* symbol)
{
    int r, w;
    for (r = 0; r < symbol->rows; r++) {
        putchar('"');
        for (w = 0; w < symbol->width; w++) {
            putchar(module_is_set(symbol, r, w) ? '1' : '0');
        }
        puts("\"");
    }
    putchar('\n');
}

int testUtilModulesCmp(const struct zint_symbol* symbol, const char* expected, int* row, int* width)
{
    const char* e = expected;
    const char* ep = expected + strlen(expected);
    int r, w;
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
