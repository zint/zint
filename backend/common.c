/* common.c - Contains functions needed for a number of barcodes */

/*
    libzint - the open source barcode library
    Copyright (C) 2008-2017 Robin Stuart <rstuart114@gmail.com>

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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

/* Local replacement for strlen() with unsigned char strings */
size_t ustrlen(const unsigned char data[]) {
    return strlen((const char*) data);
}

/* Converts a character 0-9 to its equivalent integer value */
int ctoi(const char source) {
    if ((source >= '0') && (source <= '9'))
        return (source - '0');
    if ((source >= 'A') && (source <= 'F'))
        return (source - 'A' + 10);
    if ((source >= 'a') && (source <= 'f'))
        return (source - 'a' + 10);
    return -1;
}


/* Convert an integer value to a string representing its binary equivalent */
void bin_append(const int arg, const int length, char *binary) {
    int i;
    int start;
    size_t posn = strlen(binary);

    start = 0x01 << (length - 1);

    for (i = 0; i < length; i++) {
        binary[posn + i] = '0';
        if (arg & (start >> i)) {
            binary[posn + i] = '1';
        }
    }
    binary[posn + length] = '\0';

    return;
}

/* Converts an integer value to its hexadecimal character */
char itoc(const int source) {
    if ((source >= 0) && (source <= 9)) {
        return ('0' + source);
    } else {
        return ('A' + (source - 10));
    }
}
/* Converts lower case characters to upper case in a string source[] */
void to_upper(unsigned char source[]) {
    size_t i, src_len = ustrlen(source);

    for (i = 0; i < src_len; i++) {
        if ((source[i] >= 'a') && (source[i] <= 'z')) {
            source [i] = (source[i] - 'a') + 'A';
        }
    }
}

/* Verifies that a string only uses valid characters */
int is_sane(const char test_string[], const unsigned char source[], const size_t length) {
    unsigned int j;
    size_t i, lt = strlen(test_string);

    for (i = 0; i < length; i++) {
        unsigned int latch = FALSE;
        for (j = 0; j < lt; j++) {
            if (source[i] == test_string[j]) {
                latch = TRUE;
                break;
            }
        }
        if (!(latch)) {
            return ZINT_ERROR_INVALID_DATA;
        }
    }

    return 0;
}

/* Replaces huge switch statements for looking up in tables */
void lookup(const char set_string[], const char *table[], const char data, char dest[]) {
    size_t i, n = strlen(set_string);

    for (i = 0; i < n; i++) {
        if (data == set_string[i]) {
            strcat(dest, table[i]);
        }
    }
}

/* Returns the position of data in set_string */
int posn(const char set_string[], const char data) {
    int i, n = (int)strlen(set_string);

    for (i = 0; i < n; i++) {
        if (data == set_string[i]) {
            return i;
        }
    }
    return -1;
}

/* Returns the number of times a character occurs in a string */
int ustrchr_cnt(const unsigned char string[], const size_t length, const unsigned char c) {
    int count = 0;
    int i;
    for (i = 0; i < length; i++) {
        if (string[i] == c) {
            count++;
        }
    }
    return count;
}

/* Return true (1) if a module is dark/black, otherwise false (0) */
int module_is_set(const struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    return (symbol->encoded_data[y_coord][x_coord / 7] >> (x_coord % 7)) & 1;
}

/* Set a module to dark/black */
void set_module(struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    symbol->encoded_data[y_coord][x_coord / 7] |= 1 << (x_coord % 7);
}

/* Set (or unset) a module to white */
void unset_module(struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    symbol->encoded_data[y_coord][x_coord / 7] &= ~(1 << (x_coord % 7));
}

/* Expands from a width pattern to a bit pattern */
void expand(struct zint_symbol *symbol, const char data[]) {

    size_t reader, n = strlen(data);
    int writer, i;
    char latch;

    writer = 0;
    latch = '1';

    for (reader = 0; reader < n; reader++) {
        for (i = 0; i < ctoi(data[reader]); i++) {
            if (latch == '1') {
                set_module(symbol, symbol->rows, writer);
            }
            writer++;
        }

        latch = (latch == '1' ? '0' : '1');
    }

    if (symbol->symbology != BARCODE_PHARMA) {
        if (writer > symbol->width) {
            symbol->width = writer;
        }
    } else {
        /* Pharmacode One ends with a space - adjust for this */
        if (writer > symbol->width + 2) {
            symbol->width = writer - 2;
        }
    }
    symbol->rows = symbol->rows + 1;
}

/* Indicates which symbologies can have row binding */
int is_stackable(const int symbology) {
    if (symbology < BARCODE_PDF417) {
        return 1;
    }

    switch (symbology) {
        case BARCODE_CODE128B:
        case BARCODE_ISBNX:
        case BARCODE_EAN14:
        case BARCODE_NVE18:
        case BARCODE_KOREAPOST:
        case BARCODE_PLESSEY:
        case BARCODE_TELEPEN_NUM:
        case BARCODE_ITF14:
        case BARCODE_CODE32:
        case BARCODE_CODABLOCKF:
            return 1;
    }

    return 0;
}

/* Indicates which symbols can have addon (EAN-2 and EAN-5) */
int is_extendable(const int symbology) {
    if (symbology == BARCODE_EANX || symbology == BARCODE_EANX_CHK) {
        return 1;
    }
    if (symbology == BARCODE_UPCA || symbology == BARCODE_UPCA_CHK) {
        return 1;
    }
    if (symbology == BARCODE_UPCE || symbology == BARCODE_UPCE_CHK) {
        return 1;
    }
    if (symbology == BARCODE_ISBNX) {
        return 1;
    }
    if (symbology == BARCODE_UPCA_CC) {
        return 1;
    }
    if (symbology == BARCODE_UPCE_CC) {
        return 1;
    }
    if (symbology == BARCODE_EANX_CC) {
        return 1;
    }

    return 0;
}

/* Indicates which symbols can have composite 2D component data */
int is_composite(int symbology) {
    return symbology >= BARCODE_EANX_CC && symbology <= BARCODE_RSS_EXPSTACK_CC;
}

int istwodigits(const unsigned char source[], const size_t position) {
    if ((source[position] >= '0') && (source[position] <= '9')) {
        if ((source[position + 1] >= '0') && (source[position + 1] <= '9')) {
            return 1;
        }
    }

    return 0;
}

/* State machine to decode UTF-8 to Unicode codepoints (state 0 means done, state 12 means error) */
unsigned int decode_utf8(unsigned int* state, unsigned int* codep, const unsigned char byte) {
    /*
        Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>

        Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
        files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
        modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
        Software is furnished to do so, subject to the following conditions:

        The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

        See https://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
     */

    static const unsigned char utf8d[] = {
        /* The first part of the table maps bytes to character classes that
         * reduce the size of the transition table and create bitmasks. */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
         7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
         8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

        /* The second part is a transition table that maps a combination
         * of a state of the automaton and a character class to a state. */
         0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
        12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
        12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
        12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
        12,36,12,12,12,12,12,12,12,12,12,12,
    };

    unsigned int type = utf8d[byte];

    *codep = *state != 0 ? (byte & 0x3fu) | (*codep << 6) : (0xff >> type) & byte;

    *state = utf8d[256 + *state + type];

    return *state;
}

/* Convert UTF-8 to UTF-16 for codepoints <= U+FFFF (ie four-byte sequences (requiring UTF-16 surrogates) not allowed) */
int utf8toutf16(struct zint_symbol *symbol, const unsigned char source[], int vals[], size_t *length) {
    size_t bpos;
    int    jpos;
    unsigned int codepoint, state = 0;

    bpos = 0;
    jpos = 0;

    while (bpos < *length) {
        do {
            decode_utf8(&state, &codepoint, source[bpos++]);
        } while (bpos < *length && state != 0 && state != 12);

        if (state != 0) {
            strcpy(symbol->errtxt, "240: Corrupt Unicode data");
            return ZINT_ERROR_INVALID_DATA;
        }
        if (codepoint > 0xffff) {
            strcpy(symbol->errtxt, "242: Unicode sequences of more than 3 bytes not supported");
            return ZINT_ERROR_INVALID_DATA;
        }

        vals[jpos] = codepoint;
        jpos++;

    }
    *length = jpos;

    return 0;
}


void set_minimum_height(struct zint_symbol *symbol, const int min_height) {
    /* Enforce minimum permissable height of rows */
    int fixed_height = 0;
    int zero_count = 0;
    int i;

    for (i = 0; i < symbol->rows; i++) {
        fixed_height += symbol->row_height[i];

        if (symbol->row_height[i] == 0) {
            zero_count++;
        }
    }

    if (zero_count > 0) {
        if (((symbol->height - fixed_height) / zero_count) < min_height) {
            for (i = 0; i < symbol->rows; i++) {
                if (symbol->row_height[i] == 0) {
                    symbol->row_height[i] = min_height;
                }
            }
        }
    }
}

