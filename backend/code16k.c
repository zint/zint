/* code16k.c - Handles Code 16k stacked symbology */
/*
    libzint - the open source barcode library
    Copyright (C) 2008-2023 Robin Stuart <rstuart114@gmail.com>

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

/* Updated to comply with BS EN 12323:2005 */

/* Code 16k can hold up to 77 characters or 154 numbers */

#include <assert.h>
#include <stdio.h>
#include "common.h"
#include "code128.h"

/* Note using C128Table with extra entry at 106 (Triple Shift) for C16KTable */

static const char C16KStartStop[8][4] = {
    /* EN 12323 Table 3 and Table 4 - Start patterns and stop patterns */
    {'3','2','1','1'}, {'2','2','2','1'}, {'2','1','2','2'}, {'1','4','1','1'},
    {'1','1','3','2'}, {'1','2','3','1'}, {'1','1','1','4'}, {'3','1','1','2'}
};

/* EN 12323 Table 5 - Start and stop values defining row numbers */
static const int C16KStartValues[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7
};

static const int C16KStopValues[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 4, 5, 6, 7, 0, 1, 2, 3
};

INTERNAL int code16k(struct zint_symbol *symbol, unsigned char source[], int length) {
    char width_pattern[40]; /* 4 (start) + 1 (guard) + 5*6 (chars) + 4 (stop) + 1 */
    int current_row, rows, looper, first_check, second_check;
    int indexchaine;
    int list[2][C128_MAX] = {{0}};
    char set[C128_MAX] = {0}, fset[C128_MAX] = {0}, mode, current_set;
    int pads_needed, indexliste, i, m, read, mx_reader;
    int extra_pads = 0;
    int values[C128_MAX] = {0};
    int bar_characters;
    int error_number = 0, first_sum, second_sum;
    const int gs1 = (symbol->input_mode & 0x07) == GS1_MODE;
    const int debug_print = symbol->debug & ZINT_DEBUG_PRINT;

    if (length > C128_MAX) {
        strcpy(symbol->errtxt, "420: Input too long");
        return ZINT_ERROR_TOO_LONG;
    }

    if (symbol->option_1 == 1 || symbol->option_1 > 16) {
        strcpy(symbol->errtxt, "424: Minimum number of rows out of range (2 to 16)");
        return ZINT_ERROR_INVALID_OPTION;
    }

    /* Detect extended ASCII characters */
    for (i = 0; i < length; i++) {
        fset[i] = source[i] >= 128 ? 'f' : ' ';
    }
    /* Note to be safe not using extended ASCII latch as not mentioned in BS EN 12323:2005 */

    /* Detect mode A, B and C characters */
    indexliste = 0;
    indexchaine = 0;

    mode = c128_parunmodd(source[indexchaine]);

    do {
        list[1][indexliste] = mode;
        while ((list[1][indexliste] == mode) && (indexchaine < length)) {
            list[0][indexliste]++;
            indexchaine++;
            if (indexchaine == length) {
                break;
            }
            mode = c128_parunmodd(source[indexchaine]);
            if ((gs1) && (source[indexchaine] == '[')) {
                mode = C128_ABORC;
            } /* FNC1 */
        }
        indexliste++;
    } while (indexchaine < length);

    c128_dxsmooth(list, &indexliste, NULL /*manual_set*/);

    /* Put set data into set[], resolving odd C blocks */
    c128_put_in_set(list, indexliste, set, source);

    if (debug_print) {
        printf("Data: %.*s\n", length, source);
        printf(" Set: %.*s\n", length, set);
        printf("FSet: %.*s\n", length, fset);
    }

    /* start with the mode character - Table 2 */
    m = 0;
    switch (set[0]) {
        case 'A': m = 0;
            break;
        case 'B': m = 1;
            break;
        case 'C': m = 2;
            break;
    }

    if (symbol->output_options & READER_INIT) {
        if (gs1) {
            strcpy(symbol->errtxt, "422: Cannot use both GS1 mode and Reader Initialisation");
            return ZINT_ERROR_INVALID_OPTION;
        }
        if (m == 2) {
            m = 5;
        } else if ((set[0] == 'B') && (set[1] == 'C') && fset[0] != 'f') {
            m = 6;
        }
        values[1] = 96; /* FNC3 */
        bar_characters = 2;
    } else {
        if (gs1) {
            /* Integrate FNC1 */
            switch (set[0]) {
                case 'B': m = 3;
                    break;
                case 'C': m = 4;
                    break;
            }
        } else {
            if ((set[0] == 'B') && (set[1] == 'C')) {
                m = fset[0] == 'f' ? 6 : 5;
            } else if ((set[0] == 'B') && (set[1] == 'B') && (set[2] == 'C') && fset[0] != 'f' && fset[1] != 'f') {
                m = 6;
            }
        }
        bar_characters = 1;
    }

    current_set = set[0];
    read = 0;

    /* Encode the data */
    /* TODO: make use of extra (non-CODE128) shifts: 1SB, 2SA/B/C, 3SB/C */
    do {

        if ((read != 0) && (set[read] != current_set)) {
            /* Latch different code set */
            switch (set[read]) {
                case 'A':
                    values[bar_characters++] = 101;
                    current_set = 'A';
                    break;
                case 'B':
                    values[bar_characters++] = 100;
                    current_set = 'B';
                    break;
                case 'C':
                    /* If not Mode C/Shift B and not Mode C/Double Shift B */
                    if (!(read == 1 && m >= 5) && !(read == 2 && m == 6)) {
                        values[bar_characters++] = 99;
                    }
                    current_set = 'C';
                    break;
            }
        }

        if (fset[read] == 'f') {
            /* Shift extended mode */
            switch (current_set) {
                case 'A':
                    values[bar_characters++] = 101; /* FNC 4 */
                    break;
                case 'B':
                    values[bar_characters++] = 100; /* FNC 4 */
                    break;
            }
        }

        if ((set[read] == 'a') || (set[read] == 'b')) {
            /* Insert shift character */
            values[bar_characters++] = 98;
        }

        if (!((gs1) && (source[read] == '['))) {
            switch (set[read]) { /* Encode data characters */
                case 'A':
                case 'a': c128_set_a(source[read], values, &bar_characters);
                    read++;
                    break;
                case 'B':
                case 'b': (void) c128_set_b(source[read], values, &bar_characters);
                    read++;
                    break;
                case 'C': c128_set_c(source[read], source[read + 1], values, &bar_characters);
                    read += 2;
                    break;
            }
        } else {
            values[bar_characters++] = 102;
            read++;
        }

        if (bar_characters > 80 - 2) { /* Max rows 16 * 5 - 2 check chars */
            strcpy(symbol->errtxt, "421: Input too long");
            return ZINT_ERROR_TOO_LONG;
        }
    } while (read < length);

    pads_needed = 5 - ((bar_characters + 2) % 5);
    if (pads_needed == 5) {
        pads_needed = 0;
    }
    if ((bar_characters + pads_needed) < 8) {
        pads_needed += 8 - (bar_characters + pads_needed);
    }

    rows = (bar_characters + pads_needed + 4) / 5;
    if (symbol->option_1 > rows) {
        extra_pads = (symbol->option_1 - rows) * 5;
        rows = symbol->option_1;
    }

    for (i = 0; i < pads_needed + extra_pads; i++) {
        values[bar_characters++] = 103;
    }
    values[0] = (7 * (rows - 2)) + m; /* see 4.3.4.2 */

    /* Calculate check digits */
    first_sum = 0;
    second_sum = 0;
    for (i = 0; i < bar_characters; i++) {
        first_sum += (i + 2) * values[i];
        second_sum += (i + 1) * values[i];
    }
    first_check = first_sum % 107;
    second_sum += first_check * (bar_characters + 1);
    second_check = second_sum % 107;
    values[bar_characters] = first_check;
    values[bar_characters + 1] = second_check;
    bar_characters += 2;

    if (debug_print) {
        printf("Codewords (%d):", bar_characters);
        for (i = 0; i < bar_characters; i++) {
            if (i % 5 == 0) {
                fputc('\n', stdout);
            }
            printf(" %3d", values[i]);
        }
        fputc('\n', stdout);
    }
#ifdef ZINT_TEST
    if (symbol->debug & ZINT_DEBUG_TEST) {
        debug_test_codeword_dump_int(symbol, values, bar_characters); /* Missing row start/stop */
    }
#endif

    assert(rows * 5 == bar_characters);

    for (current_row = 0; current_row < rows; current_row++) {
        int writer;
        int flip_flop;
        int len;
        char *d = width_pattern;

        memcpy(d, C16KStartStop[C16KStartValues[current_row]], 4);
        d += 4;
        *d++ = '1';
        for (i = 0; i < 5; i++, d += 6) {
            memcpy(d, C128Table[values[(current_row * 5) + i]], 6);
        }
        memcpy(d, C16KStartStop[C16KStopValues[current_row]], 4);
        d += 4;

        /* Write the information into the symbol */
        writer = 0;
        flip_flop = 1;
        for (mx_reader = 0, len = d - width_pattern; mx_reader < len; mx_reader++) {
            for (looper = 0; looper < ctoi(width_pattern[mx_reader]); looper++) {
                if (flip_flop == 1) {
                    set_module(symbol, current_row, writer);
                }
                writer++;
            }
            flip_flop = !flip_flop;
        }
    }

    symbol->rows = rows;
    symbol->width = 70;

    if (symbol->output_options & COMPLIANT_HEIGHT) {
        /* BS EN 12323:2005 Section 4.5 (d) minimum 8X; use 10X as default
           Section 4.5 (b) H = X[r(h + g) + g] = rows * row_height + (rows - 1) * separator as borders not included
           in symbol->height (added on) */
        const int separator = symbol->option_3 >= 1 && symbol->option_3 <= 4 ? symbol->option_3 : 1;
        const float min_row_height = stripf((8.0f * rows + separator * (rows - 1)) / rows);
        const float default_height = 10.0f * rows + separator * (rows - 1);
        error_number = set_height(symbol, min_row_height, default_height, 0.0f, 0 /*no_errtxt*/);
    } else {
        (void) set_height(symbol, 0.0f, 10.0f * rows, 0.0f, 1 /*no_errtxt*/);
    }

    symbol->output_options |= BARCODE_BIND;

    if (symbol->border_width == 0) { /* Allow override if non-zero */
        symbol->border_width = 1; /* BS EN 12323:2005 Section 4.3.7 minimum (note change from previous default 2) */
    }

    return error_number;
}

/* vim: set ts=4 sw=4 et : */
