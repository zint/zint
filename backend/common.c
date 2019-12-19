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
#include "common.h"

/* Local replacement for strlen() with unsigned char strings */
INTERNAL size_t ustrlen(const unsigned char data[]) {
    return strlen((const char*) data);
}

/* Converts a character 0-9 to its equivalent integer value */
INTERNAL int ctoi(const char source) {
    if ((source >= '0') && (source <= '9'))
        return (source - '0');
    if ((source >= 'A') && (source <= 'F'))
        return (source - 'A' + 10);
    if ((source >= 'a') && (source <= 'f'))
        return (source - 'a' + 10);
    return -1;
}

/* Convert an integer value to a string representing its binary equivalent */
INTERNAL void bin_append(const int arg, const int length, char *binary) {
    size_t posn = strlen(binary);

    bin_append_posn(arg, length, binary, posn);

    binary[posn + length] = '\0';
}

/* Convert an integer value to a string representing its binary equivalent at a set position */
INTERNAL void bin_append_posn(const int arg, const int length, char *binary, size_t posn) {
    int i;
    int start;

    start = 0x01 << (length - 1);

    for (i = 0; i < length; i++) {
        binary[posn + i] = '0';
        if (arg & (start >> i)) {
            binary[posn + i] = '1';
        }
    }
}

/* Converts an integer value to its hexadecimal character */
INTERNAL char itoc(const int source) {
    if ((source >= 0) && (source <= 9)) {
        return ('0' + source);
    } else {
        return ('A' + (source - 10));
    }
}

/* Converts lower case characters to upper case in a string source[] */
INTERNAL void to_upper(unsigned char source[]) {
    size_t i, src_len = ustrlen(source);

    for (i = 0; i < src_len; i++) {
        if ((source[i] >= 'a') && (source[i] <= 'z')) {
            source [i] = (source[i] - 'a') + 'A';
        }
    }
}

/* Verifies that a string only uses valid characters */
INTERNAL int is_sane(const char test_string[], const unsigned char source[], const size_t length) {
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
INTERNAL void lookup(const char set_string[], const char *table[], const char data, char dest[]) {
    size_t i, n = strlen(set_string);

    for (i = 0; i < n; i++) {
        if (data == set_string[i]) {
            strcat(dest, table[i]);
        }
    }
}

/* Returns the position of data in set_string */
INTERNAL int posn(const char set_string[], const char data) {
    int i, n = (int)strlen(set_string);

    for (i = 0; i < n; i++) {
        if (data == set_string[i]) {
            return i;
        }
    }
    return -1;
}

/* Returns the number of times a character occurs in a string */
INTERNAL int ustrchr_cnt(const unsigned char string[], const size_t length, const unsigned char c) {
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
INTERNAL int module_is_set(const struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    return (symbol->encoded_data[y_coord][x_coord / 7] >> (x_coord % 7)) & 1;
}

/* Set a module to dark/black */
INTERNAL void set_module(struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    symbol->encoded_data[y_coord][x_coord / 7] |= 1 << (x_coord % 7);
}

/* Set (or unset) a module to white */
INTERNAL void unset_module(struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    symbol->encoded_data[y_coord][x_coord / 7] &= ~(1 << (x_coord % 7));
}

/* Expands from a width pattern to a bit pattern */
INTERNAL void expand(struct zint_symbol *symbol, const char data[]) {

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
INTERNAL int is_stackable(const int symbology) {
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
INTERNAL int is_extendable(const int symbology) {
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
INTERNAL int is_composite(int symbology) {
    return symbology >= BARCODE_EANX_CC && symbology <= BARCODE_RSS_EXPSTACK_CC;
}

INTERNAL int istwodigits(const unsigned char source[], const size_t position) {
    if ((source[position] >= '0') && (source[position] <= '9')) {
        if ((source[position + 1] >= '0') && (source[position + 1] <= '9')) {
            return 1;
        }
    }

    return 0;
}

/* State machine to decode UTF-8 to Unicode codepoints (state 0 means done, state 12 means error) */
INTERNAL unsigned int decode_utf8(unsigned int* state, unsigned int* codep, const unsigned char byte) {
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

/* Convert UTF-8 to Unicode. If `disallow_4byte` unset, allow all values (UTF-32).
 * If `disallow_4byte` set, only allow codepoints <= U+FFFF (ie four-byte sequences not allowed) (UTF-16, no surrogates) */
INTERNAL int utf8_to_unicode(struct zint_symbol *symbol, const unsigned char source[], unsigned int vals[], size_t *length, int disallow_4byte) {
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

/* Enforce minimum permissable height of rows */
INTERNAL void set_minimum_height(struct zint_symbol *symbol, const int min_height) {
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

/* Calculate optimized encoding modes. Adapted from Project Nayuki */
INTERNAL void pn_define_mode(char* mode, const unsigned int data[], const size_t length, const int debug,
        unsigned int state[], const char mode_types[], const int num_modes, pn_head_costs head_costs, pn_switch_cost switch_cost, pn_eod_cost eod_cost, pn_cur_cost cur_cost) {
    /*
     * Copyright (c) Project Nayuki. (MIT License)
     * https://www.nayuki.io/page/qr-code-generator-library
     *
     * Permission is hereby granted, free of charge, to any person obtaining a copy of
     * this software and associated documentation files (the "Software"), to deal in
     * the Software without restriction, including without limitation the rights to
     * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
     * the Software, and to permit persons to whom the Software is furnished to do so,
     * subject to the following conditions:
     * - The above copyright notice and this permission notice shall be included in
     *   all copies or substantial portions of the Software.
     */
    int i, j, k, cm_i;
    unsigned int min_cost;
    char cur_mode;
#ifndef _MSC_VER
    unsigned int prev_costs[num_modes];
    char char_modes[length * num_modes];
    unsigned int cur_costs[num_modes];
#else
    unsigned int* prev_costs;
    char* char_modes;
    unsigned int* cur_costs;
    prev_costs = (unsigned int*) _alloca(num_modes * sizeof(unsigned int));
    char_modes = (char*) _alloca(length * num_modes);
    cur_costs = (unsigned int*) _alloca(num_modes * sizeof(unsigned int));
#endif

    /* char_modes[i * num_modes + j] represents the mode to encode the code point at index i such that the final
     * segment ends in mode_types[j] and the total number of bits is minimized over all possible choices */
    memset(char_modes, 0, length * num_modes);

    /* At the beginning of each iteration of the loop below, prev_costs[j] is the minimum number of 1/6 (1/XX_MULT)
     * bits needed to encode the entire string prefix of length i, and end in mode_types[j] */
    memcpy(prev_costs, (*head_costs)(state), num_modes * sizeof(unsigned int));

    /* Calculate costs using dynamic programming */
    for (i = 0, cm_i = 0; i < length; i++, cm_i += num_modes) {
        memset(cur_costs, 0, num_modes * sizeof(unsigned int));

        (*cur_cost)(state, data, length, i, char_modes, prev_costs, cur_costs);

        if (eod_cost && i == length - 1) { /* Add end of data costs if last character */
            for (j = 0; j < num_modes; j++) {
                if (char_modes[cm_i + j]) {
                    cur_costs[j] += (*eod_cost)(state, j);
                }
            }
        }

        /* Start new segment at the end to switch modes */
        for (j = 0; j < num_modes; j++) { /* To mode */
            for (k = 0; k < num_modes; k++) { /* From mode */
                if (j != k && char_modes[cm_i + k]) {
                    unsigned int new_cost = cur_costs[k] + (*switch_cost)(state, k, j);
                    if (!char_modes[cm_i + j] || new_cost < cur_costs[j]) {
                        cur_costs[j] = new_cost;
                        char_modes[cm_i + j] = mode_types[k];
                    }
                }
            }
        }

        memcpy(prev_costs, cur_costs, num_modes * sizeof(unsigned int));
    }

    /* Find optimal ending mode */
    min_cost = prev_costs[0];
    cur_mode = mode_types[0];
    for (i = 1; i < num_modes; i++) {
        if (prev_costs[i] < min_cost) {
            min_cost = prev_costs[i];
            cur_mode = mode_types[i];
        }
    }

    /* Get optimal mode for each code point by tracing backwards */
    for (i = length - 1, cm_i = i * num_modes; i >= 0; i--, cm_i -= num_modes) {
        j = strchr(mode_types, cur_mode) - mode_types;
        cur_mode = char_modes[cm_i + j];
        mode[i] = cur_mode;
    }

    if (debug & ZINT_DEBUG_PRINT) {
        printf("  Mode: %.*s\n", (int)length, mode);
    }
}

#ifdef ZINT_TEST
/* Dumps hex-formatted codewords in symbol->errtxt (for use in testing) */
void debug_test_codeword_dump(struct zint_symbol *symbol, unsigned char* codewords, int length) {
    int i, max = length, cnt_len = 0;
    if (length > 30) { /* 30*3 < errtxt 92 (100 - "Warning ") chars */
        sprintf(symbol->errtxt, "(%d) ", length); /* Place the number of codewords at the front */
        cnt_len = strlen(symbol->errtxt);
        max = 30 - (cnt_len + 2) / 3;
    }
    for (i = 0; i < max; i++) {
        sprintf(symbol->errtxt + cnt_len + i * 3, "%02X ", codewords[i]);
    }
    symbol->errtxt[strlen(symbol->errtxt) - 1] = '\0'; /* Zap last space */
}
#endif
