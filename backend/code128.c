/* code128.c - Handles Code 128 and derivatives */
/*
    libzint - the open source barcode library
    Copyright (C) 2008-2024 Robin Stuart <rstuart114@gmail.com>
    Bugfixes thanks to Christian Sakowski and BogDan Vatra

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

#include <assert.h>
#include <stdio.h>
#include "common.h"
#include "code128.h"
#include "gs1.h"

#define C128_SYMBOL_MAX 99

static const char KRSET[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
#define KRSET_F (IS_NUM_F | IS_UPR_F)

/* Code 128 tables checked against ISO/IEC 15417:2007 */

INTERNAL_DATA const char C128Table[107][6] = { /* Used by CODABLOCKF and CODE16K also */
    /* Code 128 character encodation - Table 1 (with final CODE16K-only character in place of Stop character) */
    {'2','1','2','2','2','2'}, {'2','2','2','1','2','2'}, {'2','2','2','2','2','1'}, {'1','2','1','2','2','3'},
    {'1','2','1','3','2','2'}, {'1','3','1','2','2','2'}, {'1','2','2','2','1','3'}, {'1','2','2','3','1','2'},
    {'1','3','2','2','1','2'}, {'2','2','1','2','1','3'}, {'2','2','1','3','1','2'}, {'2','3','1','2','1','2'},
    {'1','1','2','2','3','2'}, {'1','2','2','1','3','2'}, {'1','2','2','2','3','1'}, {'1','1','3','2','2','2'},
    {'1','2','3','1','2','2'}, {'1','2','3','2','2','1'}, {'2','2','3','2','1','1'}, {'2','2','1','1','3','2'},
    {'2','2','1','2','3','1'}, {'2','1','3','2','1','2'}, {'2','2','3','1','1','2'}, {'3','1','2','1','3','1'},
    {'3','1','1','2','2','2'}, {'3','2','1','1','2','2'}, {'3','2','1','2','2','1'}, {'3','1','2','2','1','2'},
    {'3','2','2','1','1','2'}, {'3','2','2','2','1','1'}, {'2','1','2','1','2','3'}, {'2','1','2','3','2','1'},
    {'2','3','2','1','2','1'}, {'1','1','1','3','2','3'}, {'1','3','1','1','2','3'}, {'1','3','1','3','2','1'},
    {'1','1','2','3','1','3'}, {'1','3','2','1','1','3'}, {'1','3','2','3','1','1'}, {'2','1','1','3','1','3'},
    {'2','3','1','1','1','3'}, {'2','3','1','3','1','1'}, {'1','1','2','1','3','3'}, {'1','1','2','3','3','1'},
    {'1','3','2','1','3','1'}, {'1','1','3','1','2','3'}, {'1','1','3','3','2','1'}, {'1','3','3','1','2','1'},
    {'3','1','3','1','2','1'}, {'2','1','1','3','3','1'}, {'2','3','1','1','3','1'}, {'2','1','3','1','1','3'},
    {'2','1','3','3','1','1'}, {'2','1','3','1','3','1'}, {'3','1','1','1','2','3'}, {'3','1','1','3','2','1'},
    {'3','3','1','1','2','1'}, {'3','1','2','1','1','3'}, {'3','1','2','3','1','1'}, {'3','3','2','1','1','1'},
    {'3','1','4','1','1','1'}, {'2','2','1','4','1','1'}, {'4','3','1','1','1','1'}, {'1','1','1','2','2','4'},
    {'1','1','1','4','2','2'}, {'1','2','1','1','2','4'}, {'1','2','1','4','2','1'}, {'1','4','1','1','2','2'},
    {'1','4','1','2','2','1'}, {'1','1','2','2','1','4'}, {'1','1','2','4','1','2'}, {'1','2','2','1','1','4'},
    {'1','2','2','4','1','1'}, {'1','4','2','1','1','2'}, {'1','4','2','2','1','1'}, {'2','4','1','2','1','1'},
    {'2','2','1','1','1','4'}, {'4','1','3','1','1','1'}, {'2','4','1','1','1','2'}, {'1','3','4','1','1','1'},
    {'1','1','1','2','4','2'}, {'1','2','1','1','4','2'}, {'1','2','1','2','4','1'}, {'1','1','4','2','1','2'},
    {'1','2','4','1','1','2'}, {'1','2','4','2','1','1'}, {'4','1','1','2','1','2'}, {'4','2','1','1','1','2'},
    {'4','2','1','2','1','1'}, {'2','1','2','1','4','1'}, {'2','1','4','1','2','1'}, {'4','1','2','1','2','1'},
    {'1','1','1','1','4','3'}, {'1','1','1','3','4','1'}, {'1','3','1','1','4','1'}, {'1','1','4','1','1','3'},
    {'1','1','4','3','1','1'}, {'4','1','1','1','1','3'}, {'4','1','1','3','1','1'}, {'1','1','3','1','4','1'},
    {'1','1','4','1','3','1'}, {'3','1','1','1','4','1'}, {'4','1','1','1','3','1'}, {'2','1','1','4','1','2'},
    {'2','1','1','2','1','4'}, {'2','1','1','2','3','2'}, {/* Only used by CODE16K */ '2','1','1','1','3','3'}
};

/* Whether `ch` can be encoded directly in code set `charset` (no shifts) (for `c128_define_mode()` below) */
static int c128_can_aorb(const unsigned char ch, const int charset, const int check_fnc1) {

    if (ch <= 31) {
        return charset == 1 || (check_fnc1 && ch == '\x1D');
    }
    if (ch <= 95) {
        return 1;
    }
    if (ch <= 127) {
        return charset == 2;
    }
    if (ch <= 159) {
        return charset == 1;
    }
    if (ch <= 223) {
        return 1;
    }
    return charset == 2;
}

/* Whether `source[position]` can be encoded in code set C (for `c128_define_mode()` below) */
static int c128_can_c(const unsigned char source[], const int length, const int position, const int check_fnc1) {
    return (position + 1 < length && z_isdigit(source[position]) && z_isdigit(source[position + 1]))
            || (check_fnc1 && source[position] == '\x1D');
}

/* Calculate the cost of encoding from `position` starting in code set `charset` (for `c128_define_mode()` below) */
static int c128_cost(const unsigned char source[], const int length, const int position, const int charset,
            const int ab_only, const char manual_set[C128_MAX], const unsigned char fncs[C128_MAX], int (*costs)[4],
            char (*modes)[4]) {

    /* Check if memoized */
    if (costs[position][charset]) {
        return costs[position][charset];
    } else {
        const int at_end = position + 1 >= length;
        const int check_fnc1 = !fncs || fncs[position];
        const int can_c = c128_can_c(source, length, position, check_fnc1);
        const int manual_c_fail = !can_c && manual_set && manual_set[position] == 3; /* C requested but not doable */
        int min_cost = 999999; /* Max possible cost 2 * 256 */
        int min_latch = 0;
        int tryset;

        /* Try code set C first (preferring C over B over A as seems to better preserve previous encodation) */
        if (!ab_only && can_c && (!manual_set || !manual_set[position] || manual_set[position] == 3)) {
            const int advance = source[position] == '\x1D' ? 1 : 2;
            int cost = 1;
            int latch = 0; /* Continue current `charset` */
            if (charset != 3) {
                cost++;
                latch = 3;
            }
            if (position + advance < length) {
                cost += c128_cost(source, length, position + advance, 3, ab_only, manual_set, fncs, costs, modes);
            }
            if (cost < min_cost) {
                min_cost = cost;
                min_latch = latch;
            }
        }
        /* Then code sets B and A */
        for (tryset = 2; tryset >= 1; tryset--) {
            if (manual_set && manual_set[position] && manual_set[position] != tryset && !manual_c_fail) {
                continue;
            }
            if (c128_can_aorb(source[position], tryset, check_fnc1)) {
                int cost = 1;
                int latch = 0; /* Continue current `charset` */
                if (charset != tryset) {
                    cost++;
                    latch = tryset;
                }
                if (!at_end) {
                    cost += c128_cost(source, length, position + 1, tryset, ab_only, manual_set, fncs, costs, modes);
                }
                if (cost < min_cost) {
                    min_cost = cost;
                    min_latch = latch;
                }
                if (charset != tryset && (charset == 1 || charset == 2)) {
                    cost = 2;
                    latch = 3 + charset; /* Shift A/B */
                    if (!at_end) {
                        cost += c128_cost(source, length, position + 1, charset, ab_only, manual_set, fncs, costs,
                                            modes);
                    }
                    if (cost < min_cost) {
                        min_cost = cost;
                        min_latch = latch;
                    }
                }
            } else if (manual_set && manual_set[position] == tryset) {
                /* Manually set, requires shift */
                int cost = 2;
                int latch = 3 + tryset; /* Shift A/B */
                if (charset != tryset) {
                    cost++;
                }
                if (!at_end) {
                    cost += c128_cost(source, length, position + 1, tryset, ab_only, manual_set, fncs, costs, modes);
                }
                if (cost < min_cost) {
                    min_cost = cost;
                    min_latch = latch;
                }
            }
        }
        assert(min_cost != 999999);

        costs[position][charset] = min_cost;
        modes[position][charset] = min_latch;

        return min_cost;
    }
}

/* Minimal encoding using Divide-And-Conquer with Memoization by Alex Geller - see
   https://github.com/zxing/zxing/commit/94fb277607003c070ffd1413754a782f3f87cbcd
   (note minimal for non-extended characters only, which are dealt with non-optimally by `fset` logic) */
static void c128_define_mode(char set[C128_MAX], const unsigned char source[], const int length,
            const int ab_only, const char manual_set[C128_MAX], const unsigned char fncs[C128_MAX]) {
    int (*costs)[4] = (int (*)[4]) z_alloca(sizeof(int) * 4 * length);
    char (*modes)[4] = (char (*)[4]) z_alloca(4 * length);
    int charset = 0;
    int i;

    memset(costs, 0, sizeof(int) * 4 * length);
    memset(modes, 0, 4 * length);

    c128_cost(source, length, 0 /*position*/, 0 /*charset*/, ab_only, manual_set, fncs, costs, modes);

    for (i = 0; i < length; i++) {
        const int latch = modes[i][charset];
        if (latch >= 1 && latch <= 3) {
            charset = latch;
            set[i] = '@' + latch; /* A, B or C */
        } else if (latch >= 4 && latch <= 5) {
            /* Shift A/B */
            charset = latch - 3;
            set[i] = charset == 1 ? 'b' : 'a';
        } else {
            /* Continue in same `charset` */
            assert(charset);
            set[i] = '@' + charset; /* A, B or C */
        }
        if (charset == 3 && source[i] != '\x1D') {
            assert(i + 1 < length); /* Guaranteed by algorithm */
            set[++i] = 'C';
        }
    }
}

/**
 * Translate Code 128 Set A characters into barcodes.
 * This set handles all control characters NUL to US.
 */
INTERNAL void c128_set_a(const unsigned char source, int values[], int *bar_chars) {

    if (source >= 128) {
        if (source < 160) {
            values[(*bar_chars)] = (source - 128) + 64;
        } else {
            values[(*bar_chars)] = (source - 128) - 32;
        }
    } else {
        if (source < 32) {
            values[(*bar_chars)] = source + 64;
        } else {
            values[(*bar_chars)] = source - 32;
        }
    }
    (*bar_chars)++;
}

/**
 * Translate Code 128 Set B characters into barcodes.
 * This set handles all characters which are not part of long numbers and not
 * control characters.
 */
INTERNAL int c128_set_b(const unsigned char source, int values[], int *bar_chars) {
    if (source >= 128 + 32) {
        values[(*bar_chars)] = source - 32 - 128;
    } else if (source >= 128) { /* Should never happen */
        return 0; /* Not reached */
    } else if (source >= 32) {
        values[(*bar_chars)] = source - 32;
    } else { /* Should never happen */
        return 0; /* Not reached */
    }
    (*bar_chars)++;
    return 1;
}

/* Translate Code 128 Set C characters into barcodes
 * This set handles numbers in a compressed form
 */
INTERNAL void c128_set_c(const unsigned char source_a, const unsigned char source_b, int values[], int *bar_chars) {
    values[(*bar_chars)] = 10 * (source_a - '0') + source_b - '0';
    (*bar_chars)++;
}

/* Helper to write out symbol, calculating check digit */
static void c128_expand(struct zint_symbol *symbol, int values[C128_MAX], int bar_characters) {
    char dest[640]; /* (1 (Start) + 2 (max initial extra) + 99 + 1 (check digit)) * 6 + 7 (Stop) == 625 */
    char *d = dest;
    int total_sum;
    int i;

    /* Destination setting and check digit calculation */
    memcpy(d, C128Table[values[0]], 6);
    d += 6;
    total_sum = values[0];

    for (i = 1; i < bar_characters; i++, d += 6) {
        memcpy(d, C128Table[values[i]], 6);
        total_sum += values[i] * i; /* Note can't overflow as 106 * C128_SYMBOL_MAX * C128_SYMBOL_MAX = 1038906 */
    }
    total_sum %= 103;
    memcpy(d, C128Table[total_sum], 6);
    d += 6;
    values[bar_characters++] = total_sum;

    /* Stop character */
    memcpy(d, "2331112", 7);
    d += 7;
    values[bar_characters++] = 106;

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        fputs("Codewords:", stdout);
        for (i = 0; i < bar_characters; i++) {
            printf(" %d", values[i]);
        }
        printf(" (%d)\n", bar_characters);
        printf("Barspaces: %.*s\n", (int) (d - dest), dest);
        printf("Checksum:  %d\n", total_sum);
    }
#ifdef ZINT_TEST
    if (symbol->debug & ZINT_DEBUG_TEST) {
        debug_test_codeword_dump_int(symbol, values, bar_characters);
    }
#endif

    expand(symbol, dest, d - dest);
}

/* Return codeword estimate (character encodation only) */
static int c128_glyph_count(const unsigned char source[], const int length, const char set[C128_MAX],
            const char fset[C128_MAX]) {
    int glyph_count = 0;
    char current_set = ' ';
    int f_state = 0;
    int i;

    switch (set[0]) {
        case 'A':
        case 'b': /* Manual switching can cause immediate shift */
            current_set = 'A';
            break;
        case 'B':
        case 'a':
            current_set = 'B';
            break;
        case 'C':
            current_set = 'C';
            break;
    }

    for (i = 0; i < length; i++) {
        if (set[i] != current_set) {
            /* Latch different code set */
            switch (set[0]) {
                case 'A':
                case 'b': /* Manual switching can cause immediate shift */
                    if (current_set != 'A') {
                        current_set = 'A';
                        glyph_count++;
                    }
                    break;
                case 'B':
                case 'a':
                    if (current_set != 'B') {
                        current_set = 'B';
                        glyph_count++;
                    }
                    break;
                case 'C':
                    current_set = 'C';
                    glyph_count++;
                    break;
            }
        }
        if (fset) {
            if ((fset[i] == 'F' && f_state == 0) || (fset[i] == ' ' && f_state == 1)) {
                /* Latch beginning/end of extended mode */
                f_state = !f_state;
                glyph_count += 2;
            } else if ((fset[i] == 'f' && f_state == 0) || (fset[i] == 'n' && f_state == 1)) {
                /* Shift to or from extended mode */
                glyph_count++;
            }
        }
        if ((set[i] == 'a') || (set[i] == 'b')) {
            /* Insert shift character */
            glyph_count++;
        }

        /* Actual character */
        glyph_count++;

        if (set[i] == 'C' && source[i] != '\x1D') {
            i++;
        }
    }

    return glyph_count;
}

/* Handle Code 128, 128B and HIBC 128 */
INTERNAL int code128(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i, j, k, read;
    int error_number;
    int values[C128_MAX] = {0}, bar_characters = 0;
    unsigned char src_buf[C128_MAX + 1];
    unsigned char *src = source;
    char manual_set[C128_MAX] = {0};
    unsigned char fncs[C128_MAX] = {0}; /* Manual FNC1 positions */
    char set[C128_MAX] = {0}, fset[C128_MAX], current_set = ' ';
    int f_state = 0;
    int have_fnc1 = 0; /* Whether have at least 1 manual FNC1 */
    char manual_ch = 0;

    /* Suppresses clang-analyzer-core.UndefinedBinaryOperatorResult warning on fset which is fully set */
    assert(length > 0);

    if (length > C128_MAX) {
        /* This only blocks ridiculously long input - the actual length of the
           resulting barcode depends on the type of data, so this is trapped later */
        sprintf(symbol->errtxt, "340: Input too long (%d character maximum)", C128_MAX);
        return ZINT_ERROR_TOO_LONG;
    }

    /* Detect special Code Set escapes for Code 128 in extra escape mode only */
    if ((symbol->input_mode & EXTRA_ESCAPE_MODE) && symbol->symbology == BARCODE_CODE128) {
        j = 0;
        for (i = 0; i < length; i++) {
            if (source[i] == '\\' && i + 2 < length && source[i + 1] == '^'
                    && ((source[i + 2] >= 'A' && source[i + 2] <= 'C') || source[i + 2] == '1'
                        || source[i + 2] == '^')) {
                if (source[i + 2] == '^') { /* Escape sequence '\^^' */
                    manual_set[j] = manual_ch;
                    src_buf[j++] = source[i++];
                    manual_set[j] = manual_ch;
                    src_buf[j++] = source[i++];
                    /* Drop second '^' */
                } else if (source[i + 2] == '1') { /* FNC1 */
                    i += 2;
                    fncs[j] = have_fnc1 = 1;
                    manual_set[j] = manual_ch;
                    src_buf[j++] = '\x1D'; /* Manual FNC1 dummy */
                } else { /* Manual mode A/B/C */
                    i += 2;
                    manual_ch = source[i] - '@'; /* 1 (A), 2 (B), 3 (C) */
                }
            } else {
                manual_set[j] = manual_ch;
                src_buf[j++] = source[i];
            }
        }
        if (j != length) {
            length = j;
            if (length == 0) {
                strcpy(symbol->errtxt, "842: No input data");
                return ZINT_ERROR_INVALID_DATA;
            }
            src = src_buf;
            src[length] = '\0';
            if (symbol->debug & ZINT_DEBUG_PRINT) {
                fputs("MSet: ", stdout);
                for (i = 0; i < length; i++) printf("%c", manual_set[i] + '@');
                fputc('\n', stdout);
            }
        }
    }

    c128_define_mode(set, src, length, symbol->symbology == BARCODE_CODE128AB /*ab_only*/,
                    manual_ch ? manual_set : NULL, have_fnc1 ? fncs : NULL);

    /* Detect extended ASCII characters */
    for (i = 0; i < length; i++) {
        fset[i] = src[i] >= 128 ? 'f' : ' ';
    }

    /* Decide when to latch to extended mode - Annex E note 3 */
    j = 0;
    for (i = 0; i < length; i++) {
        if (fset[i] == 'f') {
            j++;
        } else {
            j = 0;
        }

        if (j >= 5) {
            for (k = i; k > (i - 5); k--) {
                fset[k] = 'F';
            }
        }
    }
    if (j >= 3) {
        for (k = length - 1; k > length - 1 - j; k--) {
            fset[k] = 'F';
        }
    }

    /* Decide if it is worth reverting to 646 encodation for a few characters as described in 4.3.4.2 (d) */
    for (i = 1; i < length; i++) {
        if ((fset[i - 1] == 'F') && (fset[i] == ' ')) {
            int c = 0;
            /* Detected a change from 8859-1 to 646 - count how long for */
            for (j = 0; ((i + j) < length) && (fset[i + j] == ' '); j++) {
                c += set[i + j] == 'C'; /* Count code set C so can subtract when deciding below */
            }
            /* Count how many 8859-1 beyond */
            k = 0;
            if (i + j < length) {
                for (k = 1; ((i + j + k) < length) && (fset[i + j + k] != ' '); k++);
            }
            if (j - c < 3 || (j - c < 5 && k > 2)) {
                /* Change to shifting back rather than latching back */
                /* Inverts the same figures recommended by Annex E note 3 */
                for (k = 0; k < j; k++) {
                    fset[i + k] = 'n';
                }
            }
        }
    }

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("Data: %.*s (%d)\n", length, src, length);
        printf(" Set: %.*s\n", length, set);
        printf("FSet: %.*s\n", length, fset);
    }

    /* Now we can calculate how long the barcode is going to be - and stop it from
       being too long */
    if (c128_glyph_count(source, length, set, fset) > C128_SYMBOL_MAX) {
        sprintf(symbol->errtxt, "341: Input too long (%d symbol character maximum)", C128_SYMBOL_MAX);
        return ZINT_ERROR_TOO_LONG;
    }

    /* So now we know what start character to use - we can get on with it! */
    if (symbol->output_options & READER_INIT) {
        /* Reader Initialisation mode */
        switch (set[0]) {
            case 'A': /* Start A */
            case 'b': /* Manual switching can cause immediate shift */
                values[bar_characters++] = 103;
                current_set = 'A';
                values[bar_characters++] = 96; /* FNC3 */
                break;
            case 'B': /* Start B */
            case 'a':
                values[bar_characters++] = 104;
                current_set = 'B';
                values[bar_characters++] = 96; /* FNC3 */
                break;
            case 'C': /* Start C */
                values[bar_characters++] = 104; /* Start B */
                values[bar_characters++] = 96; /* FNC3 */
                values[bar_characters++] = 99; /* Code C */
                current_set = 'C';
                break;
        }
    } else {
        /* Normal mode */
        switch (set[0]) {
            case 'A': /* Start A */
            case 'b': /* Manual switching can cause immediate shift */
                values[bar_characters++] = 103;
                current_set = 'A';
                break;
            case 'B': /* Start B */
            case 'a':
                values[bar_characters++] = 104;
                current_set = 'B';
                break;
            case 'C': /* Start C */
                values[bar_characters++] = 105;
                current_set = 'C';
                break;
        }
    }

    /* Encode the data */
    for (read = 0; read < length; read++) {

        if (set[read] != current_set) {
            /* Latch different code set */
            switch (set[read]) {
                case 'A':
                case 'b': /* Manual switching can cause immediate shift */
                    if (current_set != 'A') {
                        values[bar_characters++] = 101;
                        current_set = 'A';
                    }
                    break;
                case 'B':
                case 'a':
                    if (current_set != 'B') {
                        values[bar_characters++] = 100;
                        current_set = 'B';
                    }
                    break;
                case 'C':
                    values[bar_characters++] = 99;
                    current_set = 'C';
                    break;
            }
        }

        if ((fset[read] == 'F' && f_state == 0) || (fset[read] == ' ' && f_state == 1)) {
            /* Latch beginning/end of extended mode */
            switch (current_set) {
                case 'A':
                    values[bar_characters++] = 101;
                    values[bar_characters++] = 101;
                    f_state = !f_state;
                    break;
                case 'B':
                    values[bar_characters++] = 100;
                    values[bar_characters++] = 100;
                    f_state = !f_state;
                    break;
            }
        } else if ((fset[read] == 'f' && f_state == 0) || (fset[read] == 'n' && f_state == 1)) {
            /* Shift to or from extended mode */
            switch (current_set) {
                case 'A':
                    values[bar_characters++] = 101; /* FNC4 */
                    break;
                case 'B':
                    values[bar_characters++] = 100; /* FNC4 */
                    break;
            }
        }

        if ((set[read] == 'a') || (set[read] == 'b')) {
            /* Insert shift character */
            values[bar_characters++] = 98;
        }

        /* Encode data characters */
        if (!fncs[read]) {
            switch (set[read]) {
                case 'A':
                case 'a':
                    c128_set_a(src[read], values, &bar_characters);
                    break;
                case 'B':
                case 'b':
                    (void) c128_set_b(src[read], values, &bar_characters);
                    break;
                case 'C':
                    c128_set_c(src[read], src[read + 1], values, &bar_characters);
                    read++;
                    break;
            }
        } else {
            values[bar_characters++] = 102; /* FNC1 in all modes */
        }

    }

    c128_expand(symbol, values, bar_characters);

    /* ISO/IEC 15417:2007 leaves dimensions/height as application specification */

    /* HRT */
    if (have_fnc1) {
        /* Remove any manual FNC1 dummies ('\x1D') */
        for (i = 0, j = 0; i < length; i++) {
            if (!fncs[i]) {
                src[j++] = src[i];
            }
        }
        length = j;
    }
    error_number = hrt_cpy_iso8859_1(symbol, src, length);

    return error_number;
}

/* Handle EAN-128 (Now known as GS1-128), and composite version if `cc_mode` set */
INTERNAL int gs1_128_cc(struct zint_symbol *symbol, unsigned char source[], int length, const int cc_mode,
                const int cc_rows) {
    int i, read;
    int error_number;
    int values[C128_MAX] = {0}, bar_characters = 0;
    char set[C128_MAX] = {0};
    int separator_row = 0, linkage_flag = 0;
    int reduced_length;
    unsigned char *reduced = (unsigned char *) z_alloca(length + 1);

    if (length > C128_MAX) {
        /* This only blocks ridiculously long input - the actual length of the
        resulting barcode depends on the type of data, so this is trapped later */
        sprintf(symbol->errtxt, "342: Input too long (%d character maximum)", C128_MAX);
        return ZINT_ERROR_TOO_LONG;
    }

    /* if part of a composite symbol make room for the separator pattern */
    if (symbol->symbology == BARCODE_GS1_128_CC) {
        separator_row = symbol->rows;
        symbol->row_height[symbol->rows] = 1;
        symbol->rows += 1;
    }

    error_number = gs1_verify(symbol, source, length, reduced);
    if (error_number >= ZINT_ERROR) {
        return error_number;
    }

    reduced_length = (int) ustrlen(reduced);

    c128_define_mode(set, reduced, reduced_length, 0 /*ab_only*/, NULL, NULL /*fncs*/);

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("Data: %s (%d)\n", reduced, reduced_length);
        printf(" Set: %.*s\n", reduced_length, set);
    }

    /* Now we can calculate how long the barcode is going to be - and stop it from
       being too long */
    if (c128_glyph_count(reduced, reduced_length, set, NULL /*fset*/) > C128_SYMBOL_MAX) {
        sprintf(symbol->errtxt, "344: Input too long (%d symbol character maximum)", C128_SYMBOL_MAX);
        return ZINT_ERROR_TOO_LONG;
    }

    /* So now we know what start character to use - we can get on with it! */
    assert(set[0] == 'B' || set[0] == 'C');
    switch (set[0]) {
        case 'B': /* Start B */
            values[bar_characters++] = 104;
            break;
        case 'C': /* Start C */
            values[bar_characters++] = 105;
            break;
    }

    values[bar_characters++] = 102; /* FNC1 */

    /* Encode the data */
    for (read = 0; read < reduced_length; read++) {
        assert(set[read] == 'B' || set[read] == 'C');

        if ((read != 0) && (set[read] != set[read - 1])) {
            /* Latch different code set */
            switch (set[read]) {
                case 'B':
                    values[bar_characters++] = 100;
                    break;
                case 'C':
                    values[bar_characters++] = 99;
                    break;
            }
        }

        if (reduced[read] != '\x1D') {
            switch (set[read]) { /* Encode data characters */
                case 'B':
                    (void) c128_set_b(reduced[read], values, &bar_characters);
                    break;
                case 'C':
                    c128_set_c(reduced[read], reduced[read + 1], values, &bar_characters);
                    read++;
                    break;
            }
        } else {
            values[bar_characters++] = 102; /* FNC1 in all modes */
        }
    }

    /* "...note that the linkage flag is an extra code set character between
    the last data character and the Symbol Check Character" (GS1 Specification) */

    /* Linkage flags in GS1-128 are determined by ISO/IEC 24723 section 7.4 */

    switch (cc_mode) {
        case 1:
        case 2:
            /* CC-A or CC-B 2D component */
            switch (set[reduced_length - 1]) {
                case 'B': linkage_flag = 99;
                    break;
                case 'C': linkage_flag = 101;
                    break;
            }
            break;
        case 3:
            /* CC-C 2D component */
            switch (set[reduced_length - 1]) {
                case 'B': linkage_flag = 101;
                    break;
                case 'C': linkage_flag = 100;
                    break;
            }
            break;
    }

    if (linkage_flag != 0) {
        values[bar_characters++] = linkage_flag;
    }

    c128_expand(symbol, values, bar_characters);

    /* Add the separator pattern for composite symbols */
    if (symbol->symbology == BARCODE_GS1_128_CC) {
        for (i = 0; i < symbol->width; i++) {
            if (!(module_is_set(symbol, separator_row + 1, i))) {
                set_module(symbol, separator_row, i);
            }
        }
    }

    if (reduced_length > 48) { /* GS1 General Specifications 5.4.4.3 */
        if (error_number == 0) { /* Don't overwrite any `gs1_verify()` warning */
            strcpy(symbol->errtxt, "843: GS1-128 input too long (48 character maximum)");
            error_number = ZINT_WARN_NONCOMPLIANT;
        }
    }

    if (symbol->output_options & COMPLIANT_HEIGHT) {
        /* GS1 General Specifications 21.0.1 5.12.3.2 table 2, including footnote (**):
           same as ITF-14: "in case of further space constraints" height 5.8mm / 1.016mm (X max) ~ 5.7;
           default 31.75mm / 0.495mm ~ 64.14 */
        const float min_height = 5.70866156f; /* 5.8 / 1.016 */
        const float default_height = 64.1414108f; /* 31.75 / 0.495 */
        if (symbol->symbology == BARCODE_GS1_128_CC) {
            /* Pass back via temporary linear structure */
            symbol->height = symbol->height ? min_height : default_height;
        } else {
            if (error_number == 0) { /* Don't overwrite any `gs1_verify()` warning */
                error_number = set_height(symbol, min_height, default_height, 0.0f, 0 /*no_errtxt*/);
            } else {
                (void) set_height(symbol, min_height, default_height, 0.0f, 1 /*no_errtxt*/);
            }
        }
    } else {
        const float height = 50.0f;
        if (symbol->symbology == BARCODE_GS1_128_CC) {
            symbol->height = height - cc_rows * (cc_mode == 3 ? 3 : 2) - 1.0f;
        } else {
            (void) set_height(symbol, 0.0f, height, 0.0f, 1 /*no_errtxt*/);
        }
    }

    if (symbol->input_mode & GS1PARENS_MODE) {
        i = length < (int) sizeof(symbol->text) ? length : (int) sizeof(symbol->text);
        memcpy(symbol->text, source, i);
    } else {
        int bracket_level = 0; /* Non-compliant closing square brackets may be in text */
        for (i = 0; i < length && i < (int) sizeof(symbol->text); i++) {
            if (source[i] == '[') {
                symbol->text[i] = '(';
                bracket_level++;
            } else if (source[i] == ']' && bracket_level) {
                symbol->text[i] = ')';
                bracket_level--;
            } else {
                symbol->text[i] = source[i];
            }
        }
    }
    if (i == sizeof(symbol->text)) {
        /* Trumps all other warnings */
        strcpy(symbol->errtxt, "844: Human Readable Text truncated");
        error_number = ZINT_WARN_HRT_TRUNCATED;
        i--;
    }
    symbol->text[i] = '\0';

    return error_number;
}

/* Handle EAN-128 (Now known as GS1-128) */
INTERNAL int gs1_128(struct zint_symbol *symbol, unsigned char source[], int length) {
    return gs1_128_cc(symbol, source, length, 0 /*cc_mode*/, 0 /*cc_rows*/);
}

/* Add check digit if encoding an NVE18 symbol */
INTERNAL int nve18(struct zint_symbol *symbol, unsigned char source[], int length) {
    int error_number, zeroes;
    unsigned char ean128_equiv[23];

    if (length > 17) {
        strcpy(symbol->errtxt, "345: Input too long (17 character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }

    if (!is_sane(NEON_F, source, length)) {
        strcpy(symbol->errtxt, "346: Invalid character in data (digits only)");
        return ZINT_ERROR_INVALID_DATA;
    }

    zeroes = 17 - length;
    ustrcpy(ean128_equiv, symbol->input_mode & GS1PARENS_MODE ? "(00)" : "[00]");
    memset(ean128_equiv + 4, '0', zeroes);
    ustrcpy(ean128_equiv + 4 + zeroes, source);

    ean128_equiv[21] = gs1_check_digit(ean128_equiv + 4, 17);
    ean128_equiv[22] = '\0';

    error_number = gs1_128(symbol, ean128_equiv, 22);

    return error_number;
}

/* EAN-14 - A version of EAN-128 */
INTERNAL int ean14(struct zint_symbol *symbol, unsigned char source[], int length) {
    int error_number, zeroes;
    unsigned char ean128_equiv[19];

    if (length > 13) {
        strcpy(symbol->errtxt, "347: Input too long (13 character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }

    if (!is_sane(NEON_F, source, length)) {
        strcpy(symbol->errtxt, "348: Invalid character in data (digits only)");
        return ZINT_ERROR_INVALID_DATA;
    }

    zeroes = 13 - length;
    ustrcpy(ean128_equiv, symbol->input_mode & GS1PARENS_MODE ? "(01)" : "[01]");
    memset(ean128_equiv + 4, '0', zeroes);
    ustrcpy(ean128_equiv + 4 + zeroes, source);

    ean128_equiv[17] = gs1_check_digit(ean128_equiv + 4, 13);
    ean128_equiv[18] = '\0';

    error_number = gs1_128(symbol, ean128_equiv, 18);

    return error_number;
}

/* DPD (Deutscher Paketdienst) Code */
/* Specification at https://esolutions.dpd.com/dokumente/DPD_Parcel_Label_Specification_2.4.1_EN.pdf
 * and identification tag info (Barcode ID) at https://esolutions.dpd.com/dokumente/DPD_Routing_Database_1.3_EN.pdf */
INTERNAL int dpd(struct zint_symbol *symbol, unsigned char source[], int length) {
    int error_number = 0;
    int i, p;
    unsigned char ident_tag;
    unsigned char local_source_buf[29];
    unsigned char *local_source;
    const int mod = 36;
    const int relabel = symbol->option_2 == 1; /* A "relabel" has no identification tag */
    int cd; /* Check digit */

    if ((length != 27 && length != 28) || (length == 28 && relabel)) {
        if (relabel) {
            strcpy(symbol->errtxt, "830: DPD relabel input wrong length (27 characters required)");
        } else {
            strcpy(symbol->errtxt, "349: DPD input wrong length (27 or 28 characters required)");
        }
        return ZINT_ERROR_TOO_LONG;
    }

    if (length == 27 && !relabel) {
        local_source_buf[0] = '%';
        ustrcpy(local_source_buf + 1, source);
        local_source = local_source_buf;
        length++;
    } else {
        local_source = source;
    }

    ident_tag = local_source[0];

    to_upper(local_source + !relabel, length - !relabel);
    if (!is_sane(KRSET_F, local_source + !relabel, length - !relabel)) {
        if (local_source == local_source_buf || relabel) {
            strcpy(symbol->errtxt, "300: Invalid character in data (alphanumerics only)");
        } else {
            strcpy(symbol->errtxt, "299: Invalid character in data (alphanumerics only after first character)");
        }
        return ZINT_ERROR_INVALID_DATA;
    }

    if ((ident_tag < 32) || (ident_tag > 127)) {
        strcpy(symbol->errtxt, "343: Invalid DPD identification tag (first character), ASCII values 32 to 127 only");
        return ZINT_ERROR_INVALID_DATA;
    }

    (void) code128(symbol, local_source, length); /* Only error returned is for large text which can't happen */

    if (!(symbol->output_options & (BARCODE_BOX | BARCODE_BIND | BARCODE_BIND_TOP))) {
        /* If no option has been selected then uses default bind top option */
        symbol->output_options |= BARCODE_BIND_TOP; /* Note won't extend over quiet zones for DPD */
        if (symbol->border_width == 0) { /* Allow override if non-zero */
            symbol->border_width = 3; /* From examples, not mentioned in spec */
        }
    }

    if (symbol->output_options & COMPLIANT_HEIGHT) {
        /* DPD Parcel Label Specification Version 2.4.1 (19.01.2021) Section 4.6.1.2
           25mm / 0.4mm (X max) = 62.5 min, 25mm / 0.375 (X) ~ 66.66 default */
        if (relabel) { /* If relabel then half-size */
            const float default_height = 33.3333321f; /* 12.5 / 0.375 */
            error_number = set_height(symbol, 31.25f, default_height, 0.0f, 0 /*no_errtxt*/);
        } else {
            const float default_height = 66.6666641f; /* 25.0 / 0.375 */
            error_number = set_height(symbol, 62.5f, default_height, 0.0f, 0 /*no_errtxt*/);
        }
    } else {
        (void) set_height(symbol, 0.0f, relabel ? 25.0f : 50.0f, 0.0f, 1 /*no_errtxt*/);
    }

    cd = mod;

    p = 0;
    for (i = !relabel; i < length; i++) {
        symbol->text[p] = local_source[i];
        p++;

        cd += posn(KRSET, local_source[i]);
        if (cd > mod) cd -= mod;
        cd *= 2;
        if (cd >= (mod + 1)) cd -= mod + 1;

        switch (i + relabel) {
            case 4:
            case 7:
            case 11:
            case 15:
            case 19:
            case 21:
            case 24:
            case 27:
                symbol->text[p++] = ' ';
                break;
        }
    }

    cd = mod + 1 - cd;
    if (cd == mod) cd = 0;

    if (cd < 10) {
        symbol->text[p] = cd + '0';
    } else {
        symbol->text[p] = (cd - 10) + 'A';
    }
    p++;

    symbol->text[p] = '\0';

    /* Some compliance checks */
    if (!is_sane(NEON_F, local_source + length - 16, 16)) {
        if (!is_sane(NEON_F, local_source + length - 3, 3)) { /* 3-digit Country Code (ISO 3166-1) */
            strcpy(symbol->errtxt, "831: Destination Country Code (last 3 characters) should be numeric");
        } else if (!is_sane(NEON_F, local_source + length - 6, 3)) { /* 3-digit Service Code */
            strcpy(symbol->errtxt, "832: Service Code (characters 6-4 from end) should be numeric");
        } else { /* Last 10 characters of Tracking No. */
            strcpy(symbol->errtxt,
                "833: Last 10 characters of Tracking Number (characters 16-7 from end) should be numeric");
        }
        error_number = ZINT_WARN_NONCOMPLIANT;
    }

    return error_number;
}

/* Universal Postal Union S10 */
/* https://www.upu.int/UPU/media/upu/files/postalSolutions/programmesAndServices/standards/S10-12.pdf */
INTERNAL int upu_s10(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i, j;
    unsigned char local_source[13 + 1];
    unsigned char have_check_digit = '\0';
    int check_digit;
    static const char weights[8] = { 8, 6, 4, 2, 3, 5, 9, 7 };
    int error_number = 0;

    if (length != 12 && length != 13) {
        strcpy(symbol->errtxt, "834: Input must be 12 or 13 characters long");
        return ZINT_ERROR_TOO_LONG;
    }
    if (length == 13) { /* Includes check digit - remove for now */
        have_check_digit = source[10];
        memcpy(local_source, source, 10);
        ustrcpy(local_source + 10, source + 11);
    } else {
        ustrcpy(local_source, source);
    }
    to_upper(local_source, length);

    if (!z_isupper(local_source[0]) || !z_isupper(local_source[1])) {
        strcpy(symbol->errtxt, "835: Invalid character in Service Indictor (first 2 characters) (alphabetic only)");
        return ZINT_ERROR_INVALID_DATA;
    }
    if (!is_sane(NEON_F, local_source + 2, 12 - 4) || (have_check_digit && !z_isdigit(have_check_digit))) {
        sprintf(symbol->errtxt, "836: Invalid character in Serial Number (middle %d characters) (digits only)",
                have_check_digit ? 9 : 8);
        return ZINT_ERROR_INVALID_DATA;
    }
    if (!z_isupper(local_source[10]) || !z_isupper(local_source[11])) {
        strcpy(symbol->errtxt, "837: Invalid character in Country Code (last 2 characters) (alphabetic only)");
        return ZINT_ERROR_INVALID_DATA;
    }

    check_digit = 0;
    for (i = 2; i < 10; i++) { /* Serial Number only */
        check_digit += ctoi(local_source[i]) * weights[i - 2];
    }
    check_digit %= 11;
    check_digit = 11 - check_digit;
    if (check_digit == 10) {
        check_digit = 0;
    } else if (check_digit == 11) {
        check_digit = 5;
    }
    if (have_check_digit && ctoi(have_check_digit) != check_digit) {
        sprintf(symbol->errtxt, "838: Invalid check digit '%c', expecting '%c'", have_check_digit, itoc(check_digit));
        return ZINT_ERROR_INVALID_CHECK;
    }
    /* Add in (back) check digit */
    local_source[12] = local_source[11];
    local_source[11] = local_source[10];
    local_source[10] = itoc(check_digit);
    local_source[13] = '\0';

    /* Do some checks on the Service Indicator (first char only) and Country Code */
    if (strchr("JKSTW", local_source[0]) != NULL) { /* These are reserved & cannot be assigned */
        strcpy(symbol->errtxt, "839: Invalid Service Indicator (first character should not be any of \"JKSTW\")");
        error_number = ZINT_WARN_NONCOMPLIANT;
    } else if (strchr("FHIOXY", local_source[0]) != NULL) { /* These aren't allocated as of spec Oct 2017 */
        strcpy(symbol->errtxt, "840: Non-standard Service Indicator (first 2 characters)");
        error_number = ZINT_WARN_NONCOMPLIANT;
    } else if (!gs1_iso3166_alpha2(local_source + 11)) {
        strcpy(symbol->errtxt, "841: Country code (last two characters) is not ISO 3166-1");
        error_number = ZINT_WARN_NONCOMPLIANT;
    }

    (void) code128(symbol, local_source, 13); /* Only error returned is for large text which can't happen */

    j = 0;
    for (i = 0; i < 13; i++) {
        if (i == 2 || i == 5 || i == 8 || i == 11) {
            symbol->text[j++] = ' ';
        }
        symbol->text[j++] = local_source[i];
    }
    symbol->text[j] = '\0';

    if (symbol->output_options & COMPLIANT_HEIGHT) {
        /* Universal Postal Union S10 Section 8, using max X 0.51mm & minimum height 12.5mm or 15% of width */
        const float min_height_min = 24.5098038f; /* 12.5 / 0.51 */
        float min_height = stripf(symbol->width * 0.15f);
        if (min_height < min_height_min) {
            min_height = min_height_min;
        }
        /* Using 50 as default as none recommended */
        if (error_number == 0) {
            error_number = set_height(symbol, min_height, min_height > 50.0f ? min_height : 50.0f, 0.0f,
                                        0 /*no_errtxt*/);
        } else {
            (void) set_height(symbol, min_height, min_height > 50.0f ? min_height : 50.0f, 0.0f, 1 /*no_errtxt*/);
        }
    } else {
        (void) set_height(symbol, 0.0f, 50.0f, 0.0f, 1 /*no_errtxt*/);
    }

    return error_number;
}

/* vim: set ts=4 sw=4 et : */
