/* common.c - Contains functions needed for a number of barcodes */

/*
    libzint - the open source barcode library
    Copyright (C) 2008 - 2021 Robin Stuart <rstuart114@gmail.com>

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
#ifdef ZINT_TEST
#include <stdio.h>
#endif
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"

/* Converts a character 0-9, A-F to its equivalent integer value */
INTERNAL int ctoi(const char source) {
    if ((source >= '0') && (source <= '9'))
        return (source - '0');
    if ((source >= 'A') && (source <= 'F'))
        return (source - 'A' + 10);
    if ((source >= 'a') && (source <= 'f'))
        return (source - 'a' + 10);
    return -1;
}

/* Converts an integer value to its hexadecimal character */
INTERNAL char itoc(const int source) {
    if ((source >= 0) && (source <= 9)) {
        return ('0' + source);
    } else {
        return ('A' + (source - 10));
    }
}

/* Converts decimal string of length <= 9 to integer value. Returns -1 if not numeric */
INTERNAL int to_int(const unsigned char source[], const int length) {
    int val = 0;
    int i;

    for (i = 0; i < length; i++) {
        if (source[i] < '0' || source[i] > '9') {
            return -1;
        }
        val *= 10;
        val += source[i] - '0';
    }

    return val;
}

/* Converts lower case characters to upper case in a string source[] */
INTERNAL void to_upper(unsigned char source[]) {
    int i, src_len = (int) ustrlen(source);

    for (i = 0; i < src_len; i++) {
        if ((source[i] >= 'a') && (source[i] <= 'z')) {
            source[i] = (source[i] - 'a') + 'A';
        }
    }
}

/* Returns the number of times a character occurs in a string */
INTERNAL int chr_cnt(const unsigned char string[], const int length, const unsigned char c) {
    int count = 0;
    int i;
    for (i = 0; i < length; i++) {
        if (string[i] == c) {
            count++;
        }
    }
    return count;
}

/* Verifies that a string only uses valid characters */
INTERNAL int is_sane(const char test_string[], const unsigned char source[], const int length) {
    int i, j, lt = (int) strlen(test_string);

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
INTERNAL void lookup(const char set_string[], const char *table[], const char data, char dest[]) {
    int i, n = (int) strlen(set_string);

    for (i = 0; i < n; i++) {
        if (data == set_string[i]) {
            strcat(dest, table[i]);
            break;
        }
    }
}

/* Convert an integer value to a string representing its binary equivalent */
INTERNAL void bin_append(const int arg, const int length, char *binary) {
    int bin_posn = (int) strlen(binary);

    bin_append_posn(arg, length, binary, bin_posn);

    binary[bin_posn + length] = '\0';
}

/* Convert an integer value to a string representing its binary equivalent at a set position */
INTERNAL int bin_append_posn(const int arg, const int length, char *binary, const int bin_posn) {
    int i;
    int start;

    start = 0x01 << (length - 1);

    for (i = 0; i < length; i++) {
        if (arg & (start >> i)) {
            binary[bin_posn + i] = '1';
        } else {
            binary[bin_posn + i] = '0';
        }
    }
    return bin_posn + length;
}

/* Returns the position of data in set_string */
INTERNAL int posn(const char set_string[], const char data) {
    int i, n = (int) strlen(set_string);

    for (i = 0; i < n; i++) {
        if (data == set_string[i]) {
            return i;
        }
    }
    return -1;
}

#ifndef COMMON_INLINE
/* Return true (1) if a module is dark/black, otherwise false (0) */
INTERNAL int module_is_set(const struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    return (symbol->encoded_data[y_coord][x_coord >> 3] >> (x_coord & 0x07)) & 1;
}

/* Set a module to dark/black */
INTERNAL void set_module(struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    symbol->encoded_data[y_coord][x_coord >> 3] |= 1 << (x_coord & 0x07);
}

/* Return true (1-8) if a module is colour, otherwise false (0) */
INTERNAL int module_colour_is_set(const struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    return symbol->encoded_data[y_coord][x_coord];
}

/* Set a module to a colour */
INTERNAL void set_module_colour(struct zint_symbol *symbol, const int y_coord, const int x_coord, const int colour) {
    symbol->encoded_data[y_coord][x_coord] = colour;
}
#endif

/* Set a dark/black module to white (i.e. unset) */
INTERNAL void unset_module(struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    symbol->encoded_data[y_coord][x_coord >> 3] &= ~(1 << (x_coord & 0x07));
}

/* Expands from a width pattern to a bit pattern */
INTERNAL void expand(struct zint_symbol *symbol, const char data[]) {

    int reader, n = (int) strlen(data);
    int writer, i;
    int latch, num;

    writer = 0;
    latch = 1;

    for (reader = 0; reader < n; reader++) {
        num = ctoi(data[reader]);
        for (i = 0; i < num; i++) {
            if (latch) {
                set_module(symbol, symbol->rows, writer);
            }
            writer++;
        }

        latch = !latch;
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
INTERNAL int is_stackable(const int symbology) {
    if (symbology < BARCODE_PHARMA_TWO && symbology != BARCODE_POSTNET) {
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
        case BARCODE_HIBC_BLOCKF:
            return 1;
            break;
    }

    return 0;
}

/* Indicates which symbols can have addon (EAN-2 and EAN-5) */
INTERNAL int is_extendable(const int symbology) {

    switch (symbology) {
        case BARCODE_EANX:
        case BARCODE_EANX_CHK:
        case BARCODE_UPCA:
        case BARCODE_UPCA_CHK:
        case BARCODE_UPCE:
        case BARCODE_UPCE_CHK:
        case BARCODE_ISBNX:
        case BARCODE_EANX_CC:
        case BARCODE_UPCA_CC:
        case BARCODE_UPCE_CC:
            return 1;
            break;
    }

    return 0;
}

/* Indicates which symbols can have composite 2D component data */
INTERNAL int is_composite(const int symbology) {
    return symbology >= BARCODE_EANX_CC && symbology <= BARCODE_DBAR_EXPSTK_CC;
}

/* Whether next two characters are digits */
INTERNAL int istwodigits(const unsigned char source[], const int length, const int position) {
    if ((position + 1 < length) && (source[position] >= '0') && (source[position] <= '9')
            && (source[position + 1] >= '0') && (source[position + 1] <= '9')) {
        return 1;
    }

    return 0;
}

/* State machine to decode UTF-8 to Unicode codepoints (state 0 means done, state 12 means error) */
INTERNAL unsigned int decode_utf8(unsigned int *state, unsigned int *codep, const unsigned char byte) {
    /*
        Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>

        Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
        documentation files (the "Software"), to deal in the Software without restriction, including without
        limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
        Software, and to permit persons to whom the Software is furnished to do so, subject to the following
        conditions:

        The above copyright notice and this permission notice shall be included in all copies or substantial portions
        of the Software.

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

/* Is string valid UTF-8? */
INTERNAL int is_valid_utf8(const unsigned char source[], const int length) {
    int i;
    unsigned int codepoint, state = 0;

    for (i = 0; i < length; i++) {
        if (decode_utf8(&state, &codepoint, source[i]) == 12) {
            return 0;
        }
    }

    return state == 0;
}

/* Convert UTF-8 to Unicode. If `disallow_4byte` unset, allow all values (UTF-32). If `disallow_4byte` set,
 * only allow codepoints <= U+FFFF (ie four-byte sequences not allowed) (UTF-16, no surrogates) */
INTERNAL int utf8_to_unicode(struct zint_symbol *symbol, const unsigned char source[], unsigned int vals[],
            int *length, const int disallow_4byte) {
    int bpos;
    int jpos;
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
        if (disallow_4byte && codepoint > 0xffff) {
            strcpy(symbol->errtxt, "242: Unicode sequences of more than 3 bytes not supported");
            return ZINT_ERROR_INVALID_DATA;
        }

        vals[jpos] = codepoint;
        jpos++;
    }

    *length = jpos;

    return 0;
}

/* Set symbol height, returning a warning if not within minimum and/or maximum if given.
   `default_height` does not include height of fixed-height rows (i.e. separators/composite data) */
INTERNAL int set_height(struct zint_symbol *symbol, const float min_row_height, const float default_height,
            const float max_height, const int no_errtxt) {
    int error_number = 0;
    float fixed_height = 0.0f;
    int zero_count = 0;
    float row_height;
    int i;
    int rows = symbol->rows ? symbol->rows : 1; /* Sometimes called before expand() */

    for (i = 0; i < rows; i++) {
        if (symbol->row_height[i]) {
            fixed_height += symbol->row_height[i];
        } else {
            zero_count++;
        }
    }

    if (zero_count) {
        if (symbol->height) {
            row_height = (symbol->height - fixed_height) / zero_count;
        } else if (default_height) {
            row_height = default_height / zero_count;
        } else {
            row_height = min_row_height;
        }
        if (row_height < 0.5f) { /* Absolute minimum */
            row_height = 0.5f;
        }
        if (min_row_height && row_height < min_row_height) {
            error_number = ZINT_WARN_NONCOMPLIANT;
            if (!no_errtxt) {
                strcpy(symbol->errtxt, "247: Height not compliant with standards");
            }
        }
        symbol->height = row_height * zero_count + fixed_height;
    } else {
        symbol->height = fixed_height; /* Ignore any given height */
    }
    if (max_height && symbol->height > max_height) {
        error_number = ZINT_WARN_NONCOMPLIANT;
        if (!no_errtxt) {
            strcpy(symbol->errtxt, "248: Height not compliant with standards");
        }
    }

    return error_number;
}

/* Returns red component if any of ultra colour indexing "0CBMRYGKW" */
INTERNAL int colour_to_red(const int colour) {
    int return_val = 0;

    switch (colour) {
        case 8: // White
        case 3: // Magenta
        case 4: // Red
        case 5: // Yellow
            return_val = 255;
            break;
    }

    return return_val;
}

/* Returns green component if any of ultra colour indexing "0CBMRYGKW" */
INTERNAL int colour_to_green(const int colour) {
    int return_val = 0;

    switch (colour) {
        case 8: // White
        case 1: // Cyan
        case 5: // Yellow
        case 6: // Green
            return_val = 255;
            break;
    }

    return return_val;
}

/* Returns blue component if any of ultra colour indexing "0CBMRYGKW" */
INTERNAL int colour_to_blue(const int colour) {
    int return_val = 0;

    switch (colour) {
        case 8: // White
        case 1: // Cyan
        case 2: // Blue
        case 3: // Magenta
            return_val = 255;
            break;
    }

    return return_val;
}

#ifdef ZINT_TEST
/* Dumps hex-formatted codewords in symbol->errtxt (for use in testing) */
void debug_test_codeword_dump(struct zint_symbol *symbol, const unsigned char *codewords, const int length) {
    int i, max = length, cnt_len = 0;
    if (length > 30) { /* 30*3 < errtxt 92 (100 - "Warning ") chars */
        sprintf(symbol->errtxt, "(%d) ", length); /* Place the number of codewords at the front */
        cnt_len = (int) strlen(symbol->errtxt);
        max = 30 - (cnt_len + 2) / 3;
    }
    for (i = 0; i < max; i++) {
        sprintf(symbol->errtxt + cnt_len + i * 3, "%02X ", codewords[i]);
    }
    symbol->errtxt[strlen(symbol->errtxt) - 1] = '\0'; /* Zap last space */
}

/* Dumps decimal-formatted codewords in symbol->errtxt (for use in testing) */
void debug_test_codeword_dump_int(struct zint_symbol *symbol, const int *codewords, const int length) {
    int i, max = 0, cnt_len, errtxt_len;
    char temp[20];
    errtxt_len = sprintf(symbol->errtxt, "(%d) ", length); /* Place the number of codewords at the front */
    for (i = 0, cnt_len = errtxt_len; i < length; i++) {
        cnt_len += sprintf(temp, "%d ", codewords[i]);
        if (cnt_len > 92) {
            break;
        }
        max++;
    }
    for (i = 0; i < max; i++) {
        errtxt_len += sprintf(symbol->errtxt + errtxt_len, "%d ", codewords[i]);
    }
    symbol->errtxt[strlen(symbol->errtxt) - 1] = '\0'; /* Zap last space */
}
#endif
