/* 2of5_based.c - Handles Code 2 of 5 Interleaved derivatives ITF-14, DP Leitcode and DP Identcode */
/*
    libzint - the open source barcode library
    Copyright (C) 2008-2025 Robin Stuart <rstuart114@gmail.com>

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

/* Was in "2of5.c" */

#include <stdio.h>
#include "common.h"
#include "gs1.h"

INTERNAL int c25_inter_common(struct zint_symbol *symbol, unsigned char source[], int length,
                const int checkdigit_option, const int dont_set_height);

/* Interleaved 2-of-5 (ITF-14) */
INTERNAL int itf14(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i, error_number, zeroes;
    unsigned char localstr[16] = {0};

    if (length > 13) {
        return errtxtf(ZINT_ERROR_TOO_LONG, symbol, 311, "Input length %d too long (maximum 13)", length);
    }

    if ((i = not_sane(NEON_F, source, length))) {
        return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 312,
                        "Invalid character at position %d in input (digits only)", i);
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
    error_number = c25_inter_common(symbol, localstr, 14, 0 /*checkdigit_option*/, 1 /*dont_set_height*/);
    ustrcpy(symbol->text, localstr);

    if (error_number < ZINT_ERROR) {
        if (!(symbol->output_options & (BARCODE_BOX | BARCODE_BIND | BARCODE_BIND_TOP))) {
            /* If no option has been selected then uses default box option */
            symbol->output_options |= BARCODE_BOX;
            if (symbol->border_width == 0) { /* Allow override if non-zero */
                /* GS1 General Specifications 21.0.1 Sections 5.3.2.4 & 5.3.6 (4.83 / 1.016 ~ 4.75) */
                symbol->border_width = 5; /* Note change from previous value 8 */
            }
        }

        if (symbol->output_options & COMPLIANT_HEIGHT) {
            /* GS1 General Specifications 21.0.1 5.12.3.2 table 2, including footnote (**): (note bind/box additional
               to symbol->height), same as GS1-128: "in case of further space constraints"
               height 5.8mm / 1.016mm (X max) ~ 5.7; default 31.75mm / 0.495mm ~ 64.14 */
            const float min_height = 5.70866156f; /* 5.8 / 1.016 */
            const float default_height = 64.1414108f; /* 31.75 / 0.495 */
            error_number = set_height(symbol, min_height, default_height, 0.0f, 0 /*no_errtxt*/);
        } else {
            (void) set_height(symbol, 0.0f, 50.0f, 0.0f, 1 /*no_errtxt*/);
        }
    }

    return error_number;
}

/* Deutsche Post check digit */
static char c25_dp_check_digit(const unsigned int count) {
    return itoc((10 - (count % 10)) % 10);
}

/* Deutsche Post Leitcode */
/* Documentation (of a very incomplete and non-technical type):
https://www.deutschepost.de/content/dam/dpag/images/D_d/dialogpost-schwer/dp-dialogpost-schwer-broschuere-072021.pdf
*/
INTERNAL int dpleit(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i, j, error_number;
    unsigned int count;
    int factor;
    unsigned char localstr[16] = {0};
    int zeroes;

    count = 0;
    if (length > 13) {
        return errtxtf(ZINT_ERROR_TOO_LONG, symbol, 313, "Input length %d too long (maximum 13)", length);
    }
    if ((i = not_sane(NEON_F, source, length))) {
        return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 314,
                        "Invalid character at position %d in input (digits only)", i);
    }

    zeroes = 13 - length;
    for (i = 0; i < zeroes; i++)
        localstr[i] = '0';
    ustrcpy(localstr + zeroes, source);

    factor = 4;
    for (i = 12; i >= 0; i--) {
        count += factor * ctoi(localstr[i]);
        factor ^= 0x0D; /* Toggles 4 and 9 */
    }
    localstr[13] = c25_dp_check_digit(count);
    localstr[14] = '\0';
    error_number = c25_inter_common(symbol, localstr, 14, 0 /*checkdigit_option*/, 1 /*dont_set_height*/);

    /* HRT formatting as per DIALOGPOST SCHWER brochure but TEC-IT differs as do examples at
       https://www.philaseiten.de/cgi-bin/index.pl?ST=8615&CP=0&F=1#M147 */
    for (i = 0, j = 0; i <= 14; i++) {
        symbol->text[j++] = localstr[i];
        if (i == 4 || i == 7 || i == 10) {
            symbol->text[j++] = '.';
        }
    }

    /* TODO: Find documentation on BARCODE_DPLEIT dimensions/height */
    /* Based on eyeballing DIALOGPOST SCHWER, using 72X as default */
    (void) set_height(symbol, 0.0f, 72.0f, 0.0f, 1 /*no_errtxt*/);

    return error_number;
}

/* Deutsche Post Identcode */
/* See dpleit() for (sort of) documentation reference */
INTERNAL int dpident(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i, j, error_number, zeroes;
    unsigned int count;
    int factor;
    unsigned char localstr[16] = {0};

    count = 0;
    if (length > 11) {
        return errtxtf(ZINT_ERROR_TOO_LONG, symbol, 315, "Input length %d too long (maximum 11)", length);
    }
    if ((i = not_sane(NEON_F, source, length))) {
        return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 316,
                        "Invalid character at position %d in input (digits only)", i);
    }

    zeroes = 11 - length;
    for (i = 0; i < zeroes; i++)
        localstr[i] = '0';
    ustrcpy(localstr + zeroes, source);

    factor = 4;
    for (i = 10; i >= 0; i--) {
        count += factor * ctoi(localstr[i]);
        factor ^= 0x0D; /* Toggles 4 and 9 */
    }
    localstr[11] = c25_dp_check_digit(count);
    localstr[12] = '\0';
    error_number = c25_inter_common(symbol, localstr, 12, 0 /*checkdigit_option*/, 1 /*dont_set_height*/);

    /* HRT formatting as per DIALOGPOST SCHWER brochure but TEC-IT differs as do other examples (see above) */
    for (i = 0, j = 0; i <= 12; i++) {
        symbol->text[j++] = localstr[i];
        if (i == 1 || i == 4 || i == 7) {
            symbol->text[j++] = '.';
        } else if (i == 3 || i == 10) {
            symbol->text[j++] = ' ';
        }
    }

    /* TODO: Find documentation on BARCODE_DPIDENT dimensions/height */
    /* Based on eyeballing DIALOGPOST SCHWER, using 72X as default */
    (void) set_height(symbol, 0.0f, 72.0f, 0.0f, 1 /*no_errtxt*/);

    return error_number;
}

/* vim: set ts=4 sw=4 et : */
