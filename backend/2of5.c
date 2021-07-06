/* 2of5.c - Handles Code 2 of 5 barcodes */

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

#include <stdio.h>
#include "common.h"
#include "gs1.h"

static const char *C25MatrixTable[10] = {
    "113311", "311131", "131131", "331111", "113131",
    "313111", "133111", "111331", "311311", "131311"
};

static const char *C25MatrixStartStop[2] = { "411111", "41111" };

static const char *C25IndustTable[10] = {
    "1111313111", "3111111131", "1131111131", "3131111111", "1111311131",
    "3111311111", "1131311111", "1111113131", "3111113111", "1131113111"
};

static const char *C25IndustStartStop[2] = { "313111", "31113" };

static const char *C25IataLogicStartStop[2] = { "1111", "311" };

static const char *C25InterTable[10] = {
    "11331", "31113", "13113", "33111", "11313",
    "31311", "13311", "11133", "31131", "13131"
};

static char check_digit(const unsigned int count) {
    return itoc((10 - (count % 10)) % 10);
}

/* Common to Standard (Matrix), Industrial, IATA, and Data Logic */
static int c25_common(struct zint_symbol *symbol, const unsigned char source[], int length, const int max,
            const char *table[10], const char *start_stop[2], const int error_base) {

    int i, error_number;
    char dest[512]; /* Largest destination 6 + (80 + 1) * 6 + 5 + 1 = 498 */
    unsigned char temp[80 + 1 + 1]; /* Largest maximum 80 */
    int have_checkdigit = symbol->option_2 == 1 || symbol->option_2 == 2;

    if (length > max) {
        sprintf(symbol->errtxt, "%d: Input too long (%d character maximum)", error_base, max);
        return ZINT_ERROR_TOO_LONG;
    }
    error_number = is_sane(NEON, source, length);
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        sprintf(symbol->errtxt, "%d: Invalid character in data (digits only)", error_base + 1);
        return error_number;
    }

    ustrcpy(temp, source);

    if (have_checkdigit) {
        /* Add standard GS1 check digit */
        temp[length] = gs1_check_digit(source, length);
        temp[++length] = '\0';
    }

    /* start character */
    strcpy(dest, start_stop[0]);

    for (i = 0; i < length; i++) {
        lookup(NEON, table, temp[i], dest);
    }

    /* Stop character */
    strcat(dest, start_stop[1]);

    expand(symbol, dest);

    ustrcpy(symbol->text, temp);
    if (symbol->option_2 == 2) {
        /* Remove check digit from HRT */
        symbol->text[length - 1] = '\0';
    }

    return error_number;
}

/* Code 2 of 5 Standard (Code 2 of 5 Matrix) */
INTERNAL int matrix_two_of_five(struct zint_symbol *symbol, unsigned char source[], int length) {
    return c25_common(symbol, source, length, 80, C25MatrixTable, C25MatrixStartStop, 301);
}

/* Code 2 of 5 Industrial */
INTERNAL int industrial_two_of_five(struct zint_symbol *symbol, unsigned char source[], int length) {
    return c25_common(symbol, source, length, 45, C25IndustTable, C25IndustStartStop, 303);
}

/* Code 2 of 5 IATA */
INTERNAL int iata_two_of_five(struct zint_symbol *symbol, unsigned char source[], int length) {
    return c25_common(symbol, source, length, 45, C25IndustTable, C25IataLogicStartStop, 305);
}

/* Code 2 of 5 Data Logic */
INTERNAL int logic_two_of_five(struct zint_symbol *symbol, unsigned char source[], int length) {
    return c25_common(symbol, source, length, 80, C25MatrixTable, C25IataLogicStartStop, 307);
}

/* Common to Interleaved, ITF-14, DP Leitcode, DP Identcode */
static int c25inter_common(struct zint_symbol *symbol, unsigned char source[], int length,
            const int dont_set_height) {
    int i, j, error_number;
    char bars[7], spaces[7], mixed[14], dest[512]; /* 4 + (90 + 2) * 5 + 3 + 1 = 468 */
    unsigned char temp[90 + 2 + 1];
    int have_checkdigit = symbol->option_2 == 1 || symbol->option_2 == 2;
    float height;

    if (length > 90) {
        strcpy(symbol->errtxt, "309: Input too long (90 character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }
    error_number = is_sane(NEON, source, length);
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "310: Invalid character in data (digits only)");
        return error_number;
    }

    temp[0] = '\0';
    /* Input must be an even number of characters for Interlaced 2 of 5 to work:
       if an odd number of characters has been entered and no check digit or an even number and have check digit
       then add a leading zero */
    if (((length & 1) && !have_checkdigit) || (!(length & 1) && have_checkdigit)) {
        ustrcpy(temp, "0");
        length++;
    }
    ustrncat(temp, source, length);

    if (have_checkdigit) {
        /* Add standard GS1 check digit */
        temp[length] = gs1_check_digit(temp, length);
        temp[++length] = '\0';
    }

    /* start character */
    strcpy(dest, "1111");

    for (i = 0; i < length; i += 2) {
        int k = 0;
        /* look up the bars and the spaces and put them in two strings */
        bars[0] = '\0';
        lookup(NEON, C25InterTable, temp[i], bars);
        spaces[0] = '\0';
        lookup(NEON, C25InterTable, temp[i + 1], spaces);

        /* then merge (interlace) the strings together */
        for (j = 0; j <= 4; j++) {
            mixed[k] = bars[j];
            k++;
            mixed[k] = spaces[j];
            k++;
        }
        mixed[k] = '\0';
        strcat(dest, mixed);
    }

    /* Stop character */
    strcat(dest, "311");

    expand(symbol, dest);

    ustrcpy(symbol->text, temp);
    if (symbol->option_2 == 2) {
        /* Remove check digit from HRT */
        symbol->text[length - 1] = '\0';
    }

    if (!dont_set_height) {
#ifdef COMPLIANT_HEIGHTS
        /* ISO/IEC 16390:2007 Section 4.4 min height 5mm or 15% of symbol width whichever greater where
           (P = character pairs, N = wide/narrow ratio = 3) width = (P(4N + 6) + N + 6)X = (length / 2) * 18 + 9 */
        height = (float) ((18.0 * (length / 2) + 9.0) * 0.15);
        if (height < (float) (5.0 / 0.33)) { /* Taking X = 0.330mm from Annex D.3.1 (application specification) */
            height = (float) (5.0 / 0.33);
        }
        /* Using 50 as default as none recommended */
        error_number = set_height(symbol, height, height > 50.0f ? height : 50.0f, 0.0f, 0 /*no_errtxt*/);
#else
        height = 50.0f;
        (void) set_height(symbol, 0.0f, height, 0.0f, 1 /*no_errtxt*/);
#endif
    }

    return error_number;
}

/* Code 2 of 5 Interleaved ISO/IEC 16390:2007 */
INTERNAL int interleaved_two_of_five(struct zint_symbol *symbol, unsigned char source[], int length) {
    return c25inter_common(symbol, source, length, 0 /*dont_set_height*/);
}

/* Interleaved 2-of-5 (ITF-14) */
INTERNAL int itf14(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i, error_number, warn_number = 0, zeroes;
    unsigned char localstr[16] = {0};

    if (length > 13) {
        strcpy(symbol->errtxt, "311: Input too long (13 character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }

    error_number = is_sane(NEON, source, length);
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "312: Invalid character in data (digits only)");
        return error_number;
    }

    /* Add leading zeros as required */
    zeroes = 13 - length;
    for (i = 0; i < zeroes; i++) {
        localstr[i] = '0';
    }
    ustrcpy(localstr + zeroes, source);

    /* Calculate the check digit - the same method used for EAN-13 */
    localstr[13] = gs1_check_digit(localstr, 13);
    localstr[14] = '\0';
    error_number = c25inter_common(symbol, localstr, 14, 1 /*dont_set_height*/);
    ustrcpy(symbol->text, localstr);

    if (!((symbol->output_options & BARCODE_BOX) || (symbol->output_options & BARCODE_BIND))) {
        // If no option has been selected then uses default box option
        symbol->output_options |= BARCODE_BOX;
        if (symbol->border_width == 0) { /* Allow override if non-zero */
            /* GS1 General Specifications 21.0.1 Sections 5.3.2.4 & 5.3.6 (4.83 / 1.016 ~ 4.75) */
            symbol->border_width = 5; /* Note change from previous value 8 */
        }
    }

    if (error_number < ZINT_ERROR) {
#ifdef COMPLIANT_HEIGHTS
        /* GS1 General Specifications 21.0.1 5.12.3.2 table 2, including footnote (**): (note bind/box additional to
           symbol->height), same as GS1-128: "in case of further space constraints"
           height 5.8mm / 1.016mm (X max) ~ 5.7; default 31.75mm / 0.495mm ~ 64.14 */
        warn_number = set_height(symbol, (float) (5.8 / 1.016), (float) (31.75 / 0.495), 0.0f, 0 /*no_errtxt*/);
#else
        (void) set_height(symbol, 0.0f, 50.0f, 0.0f, 1 /*no_errtxt*/);
#endif
    }

    return error_number ? error_number : warn_number;
}

/* Deutshe Post Leitcode */
INTERNAL int dpleit(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i, error_number;
    unsigned int count;
    unsigned char localstr[16] = {0};
    int zeroes;

    count = 0;
    if (length > 13) {
        strcpy(symbol->errtxt, "313: Input wrong length (13 character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }
    error_number = is_sane(NEON, source, length);
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "314: Invalid character in data (digits only)");
        return error_number;
    }

    zeroes = 13 - length;
    for (i = 0; i < zeroes; i++)
        localstr[i] = '0';
    ustrcpy(localstr + zeroes, source);

    for (i = 12; i >= 0; i--) {
        count += 4 * ctoi(localstr[i]);

        if (i & 1) {
            count += 5 * ctoi(localstr[i]);
        }
    }
    localstr[13] = check_digit(count);
    localstr[14] = '\0';
    error_number = c25inter_common(symbol, localstr, 14, 1 /*dont_set_height*/);
    ustrcpy(symbol->text, localstr);

    // TODO: Find documentation on BARCODE_DPLEIT dimensions/height

    return error_number;
}

/* Deutsche Post Identcode */
INTERNAL int dpident(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i, error_number, zeroes;
    unsigned int count;
    unsigned char localstr[16] = {0};

    count = 0;
    if (length > 11) {
        strcpy(symbol->errtxt, "315: Input wrong length (11 character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }
    error_number = is_sane(NEON, source, length);
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "316: Invalid character in data (digits only)");
        return error_number;
    }

    zeroes = 11 - length;
    for (i = 0; i < zeroes; i++)
        localstr[i] = '0';
    ustrcpy(localstr + zeroes, source);

    for (i = 10; i >= 0; i--) {
        count += 4 * ctoi(localstr[i]);

        if (i & 1) {
            count += 5 * ctoi(localstr[i]);
        }
    }
    localstr[11] = check_digit(count);
    localstr[12] = '\0';
    error_number = c25inter_common(symbol, localstr, 12, 1 /*dont_set_height*/);
    ustrcpy(symbol->text, localstr);

    // TODO: Find documentation on BARCODE_DPIDENT dimensions/height

    return error_number;
}
