/* code128.c - Handles Code 128 and derivatives */
/*
    libzint - the open source barcode library
    Copyright (C) 2008-2022 Robin Stuart <rstuart114@gmail.com>
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

/* Determine appropriate mode for a given character */
INTERNAL int c128_parunmodd(const unsigned char llyth) {
    int modd;

    if (llyth <= 31) {
        modd = C128_SHIFTA;
    } else if ((llyth >= 48) && (llyth <= 57)) {
        modd = C128_ABORC;
    } else if (llyth <= 95) {
        modd = C128_AORB;
    } else if (llyth <= 127) {
        modd = C128_SHIFTB;
    } else if (llyth <= 159) {
        modd = C128_SHIFTA;
    } else if (llyth <= 223) {
        modd = C128_AORB;
    } else {
        modd = C128_SHIFTB;
    }

    return modd;
}

/**
 * bring together same type blocks
 */
static void c128_grwp(int list[2][C128_MAX], int *indexliste) {

    /* bring together same type blocks */
    if (*(indexliste) > 1) {
        int i = 1;
        while (i < *(indexliste)) {
            if (list[1][i - 1] == list[1][i]) {
                int j;
                /* bring together */
                list[0][i - 1] = list[0][i - 1] + list[0][i];
                j = i + 1;

                /* decrease the list */
                while (j < *(indexliste)) {
                    list[0][j - 1] = list[0][j];
                    list[1][j - 1] = list[1][j];
                    j++;
                }
                *(indexliste) = *(indexliste) - 1;
                i--;
            }
            i++;
        }
    }
}

/**
 * Implements rules from ISO 15417 Annex E
 */
INTERNAL void c128_dxsmooth(int list[2][C128_MAX], int *indexliste) {
    int i, last, next;

    for (i = 0; i < *(indexliste); i++) {
        int current = list[1][i]; /* Either C128_ABORC, C128_AORB, C128_SHIFTA or C128_SHIFTB */
        int length = list[0][i];
        if (i != 0) {
            last = list[1][i - 1];
        } else {
            last = 0;
        }
        if (i != *(indexliste) - 1) {
            next = list[1][i + 1];
        } else {
            next = 0;
        }

        if (i == 0) { /* first block */
            if (current == C128_ABORC) {
                if ((*(indexliste) == 1) && (length == 2)) {
                    /* Rule 1a */
                    list[1][i] = C128_LATCHC;
                    current = C128_LATCHC;
                } else if (length >= 4) {
                    /* Rule 1b */
                    list[1][i] = C128_LATCHC;
                    current = C128_LATCHC;
                } else {
                    current = C128_AORB; /* Determine below */
                }
            }
            if (current == C128_AORB) {
                if (next == C128_SHIFTA) {
                    /* Rule 1c */
                    list[1][i] = C128_LATCHA;
                } else {
                    /* Rule 1d */
                    list[1][i] = C128_LATCHB;
                }
            } else if (current == C128_SHIFTA) {
                /* Rule 1c */
                list[1][i] = C128_LATCHA;
            } else if (current == C128_SHIFTB) { /* Unless C128_LATCHC set above, can only be C128_SHIFTB */
                /* Rule 1d */
                list[1][i] = C128_LATCHB;
            }
        } else {
            if (current == C128_ABORC) {
                if (length >= 4) {
                    /* Rule 3 */
                    list[1][i] = C128_LATCHC;
                    current = C128_LATCHC;
                } else {
                    current = C128_AORB; /* Determine below */
                }
            }
            if (current == C128_AORB) {
                if (last == C128_LATCHA || last == C128_SHIFTB) { /* Maintain state */
                    list[1][i] = C128_LATCHA;
                } else if (last == C128_LATCHB || last == C128_SHIFTA) { /* Maintain state */
                    list[1][i] = C128_LATCHB;
                } else if (next == C128_SHIFTA) {
                    list[1][i] = C128_LATCHA;
                } else {
                    list[1][i] = C128_LATCHB;
                }
            } else if (current == C128_SHIFTA) {
                if (length > 1) {
                    /* Rule 4 */
                    list[1][i] = C128_LATCHA;
                } else if (last == C128_LATCHA || last == C128_SHIFTB) { /* Maintain state */
                    list[1][i] = C128_LATCHA;
                } else if (last == C128_LATCHC) {
                    list[1][i] = C128_LATCHA;
                }
            } else if (current == C128_SHIFTB) { /* Unless C128_LATCHC set above, can only be C128_SHIFTB */
                if (length > 1) {
                    /* Rule 5 */
                    list[1][i] = C128_LATCHB;
                } else if (last == C128_LATCHB || last == C128_SHIFTA) { /* Maintain state */
                    list[1][i] = C128_LATCHB;
                } else if (last == C128_LATCHC) {
                    list[1][i] = C128_LATCHB;
                }
            }
        } /* Rule 2 is implemented elsewhere, Rule 6 is implied */
    }

    c128_grwp(list, indexliste);
}

/**
 * Translate Code 128 Set A characters into barcodes.
 * This set handles all control characters NUL to US.
 */
INTERNAL void c128_set_a(const unsigned char source, int values[], int *bar_chars) {

    if (source > 127) {
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
    int weight;

    weight = (10 * ctoi(source_a)) + ctoi(source_b);
    values[(*bar_chars)] = weight;
    (*bar_chars)++;
}

/* Put set data into set[]. If source given (GS1_MODE) then resolves odd C blocks */
INTERNAL void c128_put_in_set(int list[2][C128_MAX], const int indexliste, char set[C128_MAX],
                unsigned char *source) {
    int read = 0;
    int i, j;

    for (i = 0; i < indexliste; i++) {
        for (j = 0; j < list[0][i]; j++) {
            switch (list[1][i]) {
                case C128_SHIFTA: set[read] = 'a';
                    break;
                case C128_LATCHA: set[read] = 'A';
                    break;
                case C128_SHIFTB: set[read] = 'b';
                    break;
                case C128_LATCHB: set[read] = 'B';
                    break;
                case C128_LATCHC: set[read] = 'C';
                    break;
            }
            read++;
        }
    }
    if (source) {
        /* Watch out for odd-length Mode C blocks */
        int c_count = 0;
        for (i = 0; i < read; i++) {
            if (set[i] == 'C') {
                if (source[i] == '[') {
                    if (c_count & 1) {
                        if ((i - c_count) != 0) {
                            set[i - c_count] = 'B';
                        } else {
                            set[i - 1] = 'B';
                        }
                    }
                    c_count = 0;
                } else {
                    c_count++;
                }
            } else {
                if (c_count & 1) {
                    if ((i - c_count) != 0) {
                        set[i - c_count] = 'B';
                    } else {
                        set[i - 1] = 'B';
                    }
                }
                c_count = 0;
            }
        }
        if (c_count & 1) {
            if ((i - c_count) != 0) {
                set[i - c_count] = 'B';
            } else {
                set[i - 1] = 'B';
            }
        }
        for (i = 1; i < read - 1; i++) {
            if ((set[i] == 'C') && ((set[i - 1] == 'B') && (set[i + 1] == 'B'))) {
                set[i] = 'B';
            }
        }
    }
}

/* Treats source as ISO 8859-1 and copies into symbol->text, converting to UTF-8. Returns length of symbol->text */
static int c128_hrt_cpy_iso8859_1(struct zint_symbol *symbol, const unsigned char source[], const int length) {
    int i, j;

    for (i = 0, j = 0; i < length && j < (int) sizeof(symbol->text); i++) {
        if (source[i] < 0x80) {
            symbol->text[j++] = source[i] >= ' ' && source[i] != 0x7F ? source[i] : ' ';
        } else if (source[i] < 0xC0) {
            if (source[i] >= 0xA0) { /* 0x80-0x9F not valid ISO 8859-1 */
                if (j + 2 >= (int) sizeof(symbol->text)) {
                    break;
                }
                symbol->text[j++] = 0xC2;
                symbol->text[j++] = source[i];
            } else {
                symbol->text[j++] = ' ';
            }
        } else {
            if (j + 2 >= (int) sizeof(symbol->text)) {
                break;
            }
            symbol->text[j++] = 0xC3;
            symbol->text[j++] = source[i] - 0x40;
        }
    }
    if (j == sizeof(symbol->text)) {
        j--;
    }
    symbol->text[j] = '\0';

    return j;
}

#ifdef ZINT_TEST /* Wrapper for direct testing */
INTERNAL int c128_hrt_cpy_iso8859_1_test(struct zint_symbol *symbol, const unsigned char source[], const int length) {
	return c128_hrt_cpy_iso8859_1(symbol, source, length);
}
#endif

/* Handle Code 128, 128B and HIBC 128 */
INTERNAL int code128(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i, j, k, values[C128_MAX] = {0}, bar_characters, read, total_sum;
    int error_number, indexchaine, indexliste, f_state;
    int sourcelen;
    int list[2][C128_MAX] = {{0}};
    char set[C128_MAX] = {0}, fset[C128_MAX], mode, last_set, current_set = ' ';
    float glyph_count;
    char dest[1000];
    char *d = dest;

    /* Suppresses clang-analyzer-core.UndefinedBinaryOperatorResult warning on fset which is fully set */
    assert(length > 0);

    error_number = 0;

    sourcelen = length;

    bar_characters = 0;
    f_state = 0;

    if (sourcelen > C128_MAX) {
        /* This only blocks ridiculously long input - the actual length of the
           resulting barcode depends on the type of data, so this is trapped later */
        sprintf(symbol->errtxt, "340: Input too long (%d character maximum)", C128_MAX);
        return ZINT_ERROR_TOO_LONG;
    }

    /* Detect extended ASCII characters */
    for (i = 0; i < sourcelen; i++) {
        fset[i] = source[i] >= 128 ? 'f' : ' ';
    }

    /* Decide when to latch to extended mode - Annex E note 3 */
    j = 0;
    for (i = 0; i < sourcelen; i++) {
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

        if ((j >= 3) && (i == (sourcelen - 1))) {
            for (k = i; k > (i - 3); k--) {
                fset[k] = 'F';
            }
        }
    }

    /* Decide if it is worth reverting to 646 encodation for a few characters as described in 4.3.4.2 (d) */
    for (i = 1; i < sourcelen; i++) {
        if ((fset[i - 1] == 'F') && (fset[i] == ' ')) {
            /* Detected a change from 8859-1 to 646 - count how long for */
            for (j = 0; ((i + j) < sourcelen) && (fset[i + j] == ' '); j++);
            /* Count how many 8859-1 beyond */
            k = 0;
            if (i + j < sourcelen) {
                for (k = 1; ((i + j + k) < sourcelen) && (fset[i + j + k] != ' '); k++);
            }
            if (j < 3 || (j < 5 && k > 2)) {
                /* Change to shifting back rather than latching back */
                /* Inverts the same figures recommended by Annex E note 3 */
                for (k = 0; k < j; k++) {
                    fset[i + k] = 'n';
                }
            }
        }
    }

    /* Decide on mode using same system as PDF417 and rules of ISO 15417 Annex E */
    indexliste = 0;
    indexchaine = 0;

    mode = c128_parunmodd(source[indexchaine]);
    if ((symbol->symbology == BARCODE_CODE128B) && (mode == C128_ABORC)) {
        mode = C128_AORB;
    }

    do {
        list[1][indexliste] = mode;
        while ((list[1][indexliste] == mode) && (indexchaine < sourcelen)) {
            list[0][indexliste]++;
            indexchaine++;
            if (indexchaine == sourcelen) {
                break;
            }
            mode = c128_parunmodd(source[indexchaine]);
            if ((symbol->symbology == BARCODE_CODE128B) && (mode == C128_ABORC)) {
                mode = C128_AORB;
            }
        }
        indexliste++;
    } while (indexchaine < sourcelen);

    c128_dxsmooth(list, &indexliste);

    /* Resolve odd length C128_LATCHC blocks */
    if ((list[1][0] == C128_LATCHC) && (list[0][0] & 1)) {
        /* Rule 2 */
        list[0][1]++;
        list[0][0]--;
        if (indexliste == 1) {
            list[0][1] = 1;
            list[1][1] = C128_LATCHB;
            indexliste = 2;
        }
    }
    if (indexliste > 1) {
        for (i = 1; i < indexliste; i++) {
            if ((list[1][i] == C128_LATCHC) && (list[0][i] & 1)) {
                /* Rule 3b */
                list[0][i - 1]++;
                list[0][i]--;
            }
        }
    }

    /* Put set data into set[]. Giving NULL as source as used to resolve odd C blocks which has been done above */
    c128_put_in_set(list, indexliste, set, NULL /*source*/);

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("Data: %.*s (%d)\n", sourcelen, source, sourcelen);
        printf(" Set: %.*s\n", sourcelen, set);
        printf("FSet: %.*s\n", sourcelen, fset);
    }

    /* Now we can calculate how long the barcode is going to be - and stop it from
       being too long */
    last_set = set[0];
    glyph_count = 0.0f;
    for (i = 0; i < sourcelen; i++) {
        if ((set[i] == 'a') || (set[i] == 'b')) {
            glyph_count = glyph_count + 1.0f;
        }
        if ((fset[i] == 'f') || (fset[i] == 'n')) {
            glyph_count = glyph_count + 1.0f;
        }
        if (((set[i] == 'A') || (set[i] == 'B')) || (set[i] == 'C')) {
            if (set[i] != last_set) {
                last_set = set[i];
                glyph_count = glyph_count + 1.0f;
            }
        }
        if (i == 0) {
            if (fset[i] == 'F') {
                glyph_count = glyph_count + 2.0f;
            }
        } else {
            if ((fset[i] == 'F') && (fset[i - 1] != 'F')) {
                glyph_count = glyph_count + 2.0f;
            }
            if ((fset[i] != 'F') && (fset[i - 1] == 'F')) {
                glyph_count = glyph_count + 2.0f;
            }
        }

        if (set[i] == 'C') {
            glyph_count = glyph_count + 0.5f;
        } else {
            glyph_count = glyph_count + 1.0f;
        }
    }
    if (glyph_count > 60.0f) {
        strcpy(symbol->errtxt, "341: Input too long (60 symbol character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }

    /* So now we know what start character to use - we can get on with it! */
    if (symbol->output_options & READER_INIT) {
        /* Reader Initialisation mode */
        switch (set[0]) {
            case 'A': /* Start A */
                values[bar_characters++] = 103;
                current_set = 'A';
                values[bar_characters++] = 96; /* FNC3 */
                break;
            case 'B': /* Start B */
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
                values[bar_characters++] = 103;
                current_set = 'A';
                break;
            case 'B': /* Start B */
                values[bar_characters++] = 104;
                current_set = 'B';
                break;
            case 'C': /* Start C */
                values[bar_characters++] = 105;
                current_set = 'C';
                break;
        }
    }

    if (fset[0] == 'F') {
        switch (current_set) {
            case 'A':
                values[bar_characters++] = 101;
                values[bar_characters++] = 101;
                break;
            case 'B':
                values[bar_characters++] = 100;
                values[bar_characters++] = 100;
                break;
        }
        f_state = 1;
    }

    /* Encode the data */
    read = 0;
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
                    values[bar_characters++] = 99;
                    current_set = 'C';
                    break;
            }
        }

        if (read != 0) {
            if ((fset[read] == 'F') && (f_state == 0)) {
                /* Latch beginning of extended mode */
                switch (current_set) {
                    case 'A':
                        values[bar_characters++] = 101;
                        values[bar_characters++] = 101;
                        break;
                    case 'B':
                        values[bar_characters++] = 100;
                        values[bar_characters++] = 100;
                        break;
                }
                f_state = 1;
            }
            if ((fset[read] == ' ') && (f_state == 1)) {
                /* Latch end of extended mode */
                switch (current_set) {
                    case 'A':
                        values[bar_characters++] = 101;
                        values[bar_characters++] = 101;
                        break;
                    case 'B':
                        values[bar_characters++] = 100;
                        values[bar_characters++] = 100;
                        break;
                }
                f_state = 0;
            }
        }

        if ((fset[read] == 'f') || (fset[read] == 'n')) {
            /* Shift to or from extended mode */
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

        switch (set[read]) { /* Encode data characters */
            case 'a':
            case 'A': c128_set_a(source[read], values, &bar_characters);
                read++;
                break;
            case 'b':
            case 'B': (void) c128_set_b(source[read], values, &bar_characters);
                read++;
                break;
            case 'C': c128_set_c(source[read], source[read + 1], values, &bar_characters);
                read += 2;
                break;
        }

    } while (read < sourcelen);

    /* Destination setting and check digit calculation */
    memcpy(d, C128Table[values[0]], 6);
    d += 6;
    total_sum = values[0];

    for (i = 1; i < bar_characters; i++, d += 6) {
        memcpy(d, C128Table[values[i]], 6);
        total_sum += values[i] * i; /* Note can't overflow as 106 * 60 * 60 = 381600 */
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
        printf("Codewords:");
        for (i = 0; i < bar_characters; i++) {
            printf(" %d", values[i]);
        }
        printf(" (%d)\n", bar_characters);
        printf("Barspaces: %.*s\n", (int) (d - dest), dest);
    }
#ifdef ZINT_TEST
    if (symbol->debug & ZINT_DEBUG_TEST) {
        debug_test_codeword_dump_int(symbol, values, bar_characters);
    }
#endif

    expand(symbol, dest, d - dest);

    /* ISO/IEC 15417:2007 leaves dimensions/height as application specification */

    c128_hrt_cpy_iso8859_1(symbol, source, length);

    return error_number;
}

/* Handle EAN-128 (Now known as GS1-128), and composite version if `cc_mode` set */
INTERNAL int gs1_128_cc(struct zint_symbol *symbol, unsigned char source[], int length, const int cc_mode,
                const int cc_rows) {
    int i, values[C128_MAX] = {0}, bar_characters, read, total_sum;
    int error_number, warn_number = 0, indexchaine, indexliste;
    int list[2][C128_MAX] = {{0}};
    char set[C128_MAX] = {0}, mode, last_set;
    float glyph_count;
    char dest[1000];
    char *d = dest;
    int separator_row, linkage_flag;
    int reduced_length;
    unsigned char *reduced = (unsigned char *) z_alloca(length + 1);

    linkage_flag = 0;

    bar_characters = 0;
    separator_row = 0;

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

    /* Decide on mode using same system as PDF417 and rules of ISO 15417 Annex E */
    indexliste = 0;
    indexchaine = 0;

    mode = c128_parunmodd(reduced[indexchaine]);

    do {
        list[1][indexliste] = mode;
        while ((list[1][indexliste] == mode) && (indexchaine < reduced_length)) {
            list[0][indexliste]++;
            indexchaine++;
            if (indexchaine == reduced_length) {
                break;
            }
            mode = c128_parunmodd(reduced[indexchaine]);
            if (reduced[indexchaine] == '[') {
                mode = C128_ABORC;
            }
        }
        indexliste++;
    } while (indexchaine < reduced_length);

    c128_dxsmooth(list, &indexliste);

    /* Put set data into set[], resolving odd C blocks */
    c128_put_in_set(list, indexliste, set, reduced);

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("Data: %s (%d)\n", reduced, reduced_length);
        printf(" Set: %.*s\n", reduced_length, set);
    }

    /* Now we can calculate how long the barcode is going to be - and stop it from
       being too long */
    last_set = set[0];
    glyph_count = 0.0f;
    for (i = 0; i < reduced_length; i++) {
        if ((set[i] == 'A') || (set[i] == 'B') || (set[i] == 'C')) {
            if (set[i] != last_set) {
                last_set = set[i];
                glyph_count = glyph_count + 1.0f;
            }
        } else if ((set[i] == 'a') || (set[i] == 'b')) {
            glyph_count = glyph_count + 1.0f; /* Not reached */
        }

        if ((set[i] == 'C') && (reduced[i] != '[')) {
            glyph_count = glyph_count + 0.5f;
        } else {
            glyph_count = glyph_count + 1.0f;
        }
    }
    if (glyph_count > 60.0f) {
        strcpy(symbol->errtxt, "344: Input too long (60 symbol character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }

    /* So now we know what start character to use - we can get on with it! */
    switch (set[0]) {
        case 'A': /* Start A */
            values[bar_characters++] = 103; /* Not reached */
            break;
        case 'B': /* Start B */
            values[bar_characters++] = 104;
            break;
        case 'C': /* Start C */
            values[bar_characters++] = 105;
            break;
    }

    values[bar_characters++] = 102;

    /* Encode the data */
    read = 0;
    do {

        if ((read != 0) && (set[read] != set[read - 1])) { /* Latch different code set */
            switch (set[read]) {
                case 'A':
                    values[bar_characters++] = 101; /* Not reached */
                    break;
                case 'B':
                    values[bar_characters++] = 100;
                    break;
                case 'C':
                    values[bar_characters++] = 99;
                    break;
            }
        }

        if ((set[read] == 'a') || (set[read] == 'b')) {
            /* Insert shift character */
            values[bar_characters++] = 98; /* Not reached */
        }

        if (reduced[read] != '[') {
            switch (set[read]) { /* Encode data characters */
                case 'A':
                case 'a':
                    c128_set_a(reduced[read], values, &bar_characters); /* Not reached */
                    read++;
                    break;
                case 'B':
                case 'b':
                    (void) c128_set_b(reduced[read], values, &bar_characters);
                    read++;
                    break;
                case 'C':
                    c128_set_c(reduced[read], reduced[read + 1], values, &bar_characters);
                    read += 2;
                    break;
            }
        } else {
            values[bar_characters++] = 102;
            read++;
        }
    } while (read < reduced_length);

    /* "...note that the linkage flag is an extra code set character between
    the last data character and the Symbol Check Character" (GS1 Specification) */

    /* Linkage flags in GS1-128 are determined by ISO/IEC 24723 section 7.4 */

    switch (cc_mode) {
        case 1:
        case 2:
            /* CC-A or CC-B 2D component */
            switch (set[reduced_length - 1]) {
                case 'A': linkage_flag = 100; /* Not reached */
                    break;
                case 'B': linkage_flag = 99;
                    break;
                case 'C': linkage_flag = 101;
                    break;
            }
            break;
        case 3:
            /* CC-C 2D component */
            switch (set[reduced_length - 1]) {
                case 'A': linkage_flag = 99; /* Not reached */
                    break;
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

    /* Destination setting and check digit calculation */
    memcpy(d, C128Table[values[0]], 6);
    d += 6;
    total_sum = values[0];

    for (i = 1; i < bar_characters; i++, d += 6) {
        memcpy(d, C128Table[values[i]], 6);
        total_sum += values[i] * i; /* Note can't overflow as 106 * 60 * 60 = 381600 */
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
        printf("Codewords:");
        for (i = 0; i < bar_characters; i++) {
            printf(" %d", values[i]);
        }
        printf(" (%d)\n", bar_characters);
        printf("Barspaces: %.*s\n", (int) (d - dest), dest);
    }
#ifdef ZINT_TEST
    if (symbol->debug & ZINT_DEBUG_TEST) {
        debug_test_codeword_dump_int(symbol, values, bar_characters);
    }
#endif

    expand(symbol, dest, d - dest);

    /* Add the separator pattern for composite symbols */
    if (symbol->symbology == BARCODE_GS1_128_CC) {
        for (i = 0; i < symbol->width; i++) {
            if (!(module_is_set(symbol, separator_row + 1, i))) {
                set_module(symbol, separator_row, i);
            }
        }
    }

    if (symbol->output_options & COMPLIANT_HEIGHT) {
        /* GS1 General Specifications 21.0.1 5.12.3.2 table 2, including footnote (**):
           same as ITF-14: "in case of further space constraints" height 5.8mm / 1.016mm (X max) ~ 5.7;
           default 31.75mm / 0.495mm ~ 64.14 */
        const float min_height = stripf(5.8f / 1.016f);
        const float default_height = stripf(31.75f / 0.495f);
        if (symbol->symbology == BARCODE_GS1_128_CC) {
            /* Pass back via temporary linear structure */
            symbol->height = symbol->height ? min_height : default_height;
        } else {
            warn_number = set_height(symbol, min_height, default_height, 0.0f, 0 /*no_errtxt*/);
        }
    } else {
        const float height = 50.0f;
        if (symbol->symbology == BARCODE_GS1_128_CC) {
            symbol->height = height - cc_rows * (cc_mode == 3 ? 3 : 2) - 1.0f;
        } else {
            (void) set_height(symbol, 0.0f, height, 0.0f, 1 /*no_errtxt*/);
        }
    }

    for (i = 0; i < length; i++) {
        if (source[i] == '[') {
            symbol->text[i] = '(';
        } else if (source[i] == ']') {
            symbol->text[i] = ')';
        } else {
            symbol->text[i] = source[i];
        }
    }

    return error_number ? error_number : warn_number;
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
/* Specification at ftp://dpd.at/Datenspezifikationen/EN/gbs_V4.0.2_hauptdokument.pdf
 * or https://docplayer.net/33728877-Dpd-parcel-label-specification.html */
INTERNAL int dpd(struct zint_symbol *symbol, unsigned char source[], int length) {
    int error_number = 0;
    int i, p;
    unsigned char identifier;
    const int mod = 36;
    int cd; /* Check digit */

    if (length != 28) {
        strcpy(symbol->errtxt, "349: DPD input wrong length (28 characters required)");
        return ZINT_ERROR_TOO_LONG;
    }

    identifier = source[0];

    to_upper(source + 1, length - 1);
    if (!is_sane(KRSET_F, source + 1, length - 1)) {
        strcpy(symbol->errtxt, "300: Invalid character in DPD data (alphanumerics only)");
        return ZINT_ERROR_INVALID_DATA;
    }

    if ((identifier < 32) || (identifier > 127)) {
        strcpy(symbol->errtxt, "343: Invalid DPD identifier (first character), ASCII values 32 to 127 only");
        return ZINT_ERROR_INVALID_DATA;
    }

    error_number = code128(symbol, source, length); /* Only returns errors, not warnings */

    if (error_number < ZINT_ERROR) {
        if (symbol->output_options & COMPLIANT_HEIGHT) {
            /* Specification DPD and primetime Parcel Despatch 4.0.2 Section 5.5.1
               25mm / 0.4mm (X max) = 62.5 min, 25mm / 0.375 (X) ~ 66.66 default */
            error_number = set_height(symbol, 62.5f, stripf(25.0f / 0.375f), 0.0f, 0 /*no_errtxt*/);
        } else {
            (void) set_height(symbol, 0.0f, 50.0f, 0.0f, 1 /*no_errtxt*/);
        }

        cd = mod;

        p = 0;
        for (i = 1; i < length; i++) {
            symbol->text[p] = source[i];
            p++;

            cd += posn(KRSET, source[i]);
            if (cd > mod) cd -= mod;
            cd *= 2;
            if (cd >= (mod + 1)) cd -= mod + 1;

            switch (i) {
                case 4:
                case 7:
                case 11:
                case 15:
                case 19:
                case 21:
                case 24:
                case 27:
                    symbol->text[p] = ' ';
                    p++;
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
    }

    return error_number;
}

/* vim: set ts=4 sw=4 et : */
