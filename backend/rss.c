/* rss.c - GS1 DataBar (formerly Reduced Space Symbology) */

/*
    libzint - the open source barcode library
    Copyright (C) 2008-2020 Robin Stuart <rstuart114@gmail.com>

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

/* The functions "combins" and "getRSSwidths" are copyright BSI and are
   released with permission under the following terms:

   "Copyright subsists in all BSI publications. BSI also holds the copyright, in the
   UK, of the international standardisation bodies. Except as
   permitted under the Copyright, Designs and Patents Act 1988 no extract may be
   reproduced, stored in a retrieval system or transmitted in any form or by any
   means - electronic, photocopying, recording or otherwise - without prior written
   permission from BSI.

   "This does not preclude the free use, in the course of implementing the standard,
   of necessary details such as symbols, and size, type or grade designations. If these
   details are to be used for any other purpose than implementation then the prior
   written permission of BSI must be obtained."

   The date of publication for these functions is 30 November 2006
 */

/* Includes numerous bugfixes thanks to Pablo Orduña @ the PIRAmIDE project */

/* Note: This code reflects the symbol names as used in ISO/IEC 24724:2006. These names
 * were updated in ISO/IEC 24724:2011 as follows:
 *
 * RSS-14 > GS1 DataBar Omnidirectional
 * RSS-14 Truncated > GS1 DataBar Truncated
 * RSS-14 Stacked > GS1 DataBar Stacked
 * RSS-14 Stacked Omnidirectional > GS1 DataBar Stacked Omnidirectional
 * RSS Limited > GS1 DataBar Limited
 * RSS Expanded > GS1 DataBar Expanded Omnidirectional
 * RSS Expanded Stacked > GS1 DataBar Expanded Stacked Omnidirectional
 */

#include <stdio.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"
#include "large.h"
#include "rss.h"
#include "gs1.h"
#include "general_field.h"

/**********************************************************************
 * combins(n,r): returns the number of Combinations of r selected from n:
 *   Combinations = n! / ((n - r)! * r!)
 **********************************************************************/
static int combins(int n, int r) {
    int i, j;
    int maxDenom, minDenom;
    int val;

    if (n - r > r) {
        minDenom = r;
        maxDenom = n - r;
    } else {
        minDenom = n - r;
        maxDenom = r;
    }
    val = 1;
    j = 1;
    for (i = n; i > maxDenom; i--) {
        val *= i;
        if (j <= minDenom) {
            val /= j;
            j++;
        }
    }
    for (; j <= minDenom; j++) {
        val /= j;
    }
    return (val);
}

/**********************************************************************
 * getRSSwidths
 * routine to generate widths for RSS elements for a given value.#
 *
 * Calling arguments:
 * int widths[] = element widths
 * val = required value
 * n = number of modules
 * elements = elements in a set (RSS-14 & Expanded = 4; RSS Limited = 7)
 * maxWidth = maximum module width of an element
 * noNarrow = 0 will skip patterns without a one module wide element
 *
 **********************************************************************/
static void getRSSwidths(int widths[], int val, int n, int elements, int maxWidth, int noNarrow) {
    int bar;
    int elmWidth;
    int mxwElement;
    int subVal, lessVal;
    int narrowMask = 0;
    for (bar = 0; bar < elements - 1; bar++) {
        for (elmWidth = 1, narrowMask |= (1 << bar);
                ;
                elmWidth++, narrowMask &= ~(1 << bar)) {
            /* get all combinations */
            subVal = combins(n - elmWidth - 1, elements - bar - 2);
            /* less combinations with no single-module element */
            if ((!noNarrow) && (!narrowMask) &&
                    (n - elmWidth - (elements - bar - 1) >= elements - bar - 1)) {
                subVal -= combins(n - elmWidth - (elements - bar), elements - bar - 2);
            }
            /* less combinations with elements > maxVal */
            if (elements - bar - 1 > 1) {
                lessVal = 0;
                for (mxwElement = n - elmWidth - (elements - bar - 2);
                        mxwElement > maxWidth;
                        mxwElement--) {
                    lessVal += combins(n - elmWidth - mxwElement - 1, elements - bar - 3);
                }
                subVal -= lessVal * (elements - 1 - bar);
            } else if (n - elmWidth > maxWidth) {
                subVal--;
            }
            val -= subVal;
            if (val < 0) break;
        }
        val += subVal;
        n -= elmWidth;
        widths[bar] = elmWidth;
    }
    widths[bar] = n;
    return;
}

/* Calculate check digit from Annex A */
static int calc_check_digit(unsigned char *src) {
    int i, check_digit;
    int count = 0;

    for (i = 0; i < 13; i++) {
        count += (i & 1) ? ctoi(src[i]) : 3 * ctoi(src[i]);
    }
    check_digit = 10 - (count % 10);
    if (check_digit == 10) {
        check_digit = 0;
    }

    return check_digit;
}

/* Set GTIN-14 human readable text */
static void set_gtin14_hrt(struct zint_symbol *symbol, unsigned char *source, int src_len) {
    int i;
    unsigned char hrt[15];

    ustrcpy(symbol->text, "(01)");
    for (i = 0; i < 12; i++) {
        hrt[i] = '0';
    }
    for (i = 0; i < src_len; i++) {
        hrt[12 - i] = source[src_len - i - 1];
    }

    hrt[13] = itoc(calc_check_digit(hrt));
    hrt[14] = '\0';

    ustrcat(symbol->text, hrt);
}

/* Expand from a width pattern to a bit pattern */
static int rss_expand(struct zint_symbol *symbol, int writer, char *p_latch, int width) {
    int j;
    int latch = *p_latch;

    for (j = 0; j < width; j++) {
        if (latch == '1') {
            set_module(symbol, symbol->rows, writer);
        } else {
            unset_module(symbol, symbol->rows, writer);
        }
        writer++;
    }
    if (latch == '1') {
        *p_latch = '0';
    } else {
        *p_latch = '1';
    }

    return writer;
}

/* Adjust top/bottom separator for finder patterns */
static void rss14_finder_adjust(struct zint_symbol *symbol, int separator_row, int above_below, int finder_start) {
    int i, finder_end;
    int module_row = separator_row + above_below;
    int latch;

    /* Alternation is always left-to-right for Omnidirectional separators (unlike for Expanded) */
    latch = '1';
    for (i = finder_start, finder_end = finder_start + 13; i < finder_end; i++) {
        if (!module_is_set(symbol, module_row, i)) {
            if (latch == '1') {
                set_module(symbol, separator_row, i);
                latch = '0';
            } else {
                unset_module(symbol, separator_row, i);
                latch = '1';
            }
        } else {
            unset_module(symbol, separator_row, i);
            latch = '1';
        }
    }
}

/* Top/bottom separator for DataBar */
static void rss14_separator(struct zint_symbol *symbol, int width, int separator_row, int above_below, int finder_start, int finder2_start, int bottom_finder_value_3) {
    int i, finder_end, finder_value_3_set;
    int module_row = separator_row + above_below;

    for (i = 4, width -= 4; i < width; i++) {
        if (!module_is_set(symbol, module_row, i)) {
            set_module(symbol, separator_row, i);
        }
    }
    if (bottom_finder_value_3) {
        /* ISO/IEC 24724:2011 5.3.2.2 "The single dark module that occurs in the 13 modules over finder value 3 is
         * shifted one module to the right so that it is over the start of the three module-wide finder bar." */
        finder_value_3_set = finder_start + 10;
        for (i = finder_start, finder_end = finder_start + 13; i < finder_end; i++) {
            if (i == finder_value_3_set) {
                set_module(symbol, separator_row, i);
            } else {
                unset_module(symbol, separator_row, i);
            }
        }
    } else {
        if (finder_start) {
            rss14_finder_adjust(symbol, separator_row, above_below, finder_start);
        }
        if (finder2_start) {
            rss14_finder_adjust(symbol, separator_row, above_below, finder2_start);
        }
    }
}

/* GS1 DataBar Omnidirectional/Truncated/Stacked */
INTERNAL int rss14(struct zint_symbol *symbol, unsigned char source[], int src_len) {
    int error_number = 0, i;
    large_int accum;
    uint64_t left_pair, right_pair;
    int data_character[4] = {0}, data_group[4] = {0}, v_odd[4], v_even[4];
    int data_widths[8][4], checksum, c_left, c_right, total_widths[46], writer;
    char latch;
    int separator_row;
    int widths[4];

    separator_row = 0;

    if (src_len > 14) { /* Allow check digit to be specified (will be verified and ignored) */
        strcpy(symbol->errtxt, "380: Input too long");
        return ZINT_ERROR_TOO_LONG;
    }
    error_number = is_sane(NEON, source, src_len);
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "381: Invalid characters in data");
        return error_number;
    }

    if (src_len == 14) { /* Verify check digit */
        if (calc_check_digit(source) != ctoi(source[13])) {
            strcpy(symbol->errtxt, "388: Invalid check digit");
            return ZINT_ERROR_INVALID_CHECK;
        }
        src_len--; /* Ignore */
    }

    /* make some room for a separator row for composite symbols */
    switch (symbol->symbology) {
        case BARCODE_DBAR_OMN_CC:
        case BARCODE_DBAR_STK_CC:
        case BARCODE_DBAR_OMNSTK_CC:
            separator_row = symbol->rows;
            symbol->row_height[separator_row] = 1;
            symbol->rows += 1;
            break;
    }

    large_load_str_u64(&accum, source, src_len);

    if (symbol->option_1 == 2) {
        /* Add symbol linkage flag */
        large_add_u64(&accum, 10000000000000);
    }

    /* Calculate left and right pair values */

    right_pair = large_div_u64(&accum, 4537077);
    left_pair = large_lo(&accum);

    /* Calculate four data characters */

    data_character[0] = left_pair / 1597;
    data_character[1] = left_pair % 1597;

    data_character[2] = right_pair / 1597;
    data_character[3] = right_pair % 1597;

    /* Calculate odd and even subset values */

    if ((data_character[0] >= 0) && (data_character[0] <= 160)) {
        data_group[0] = 0;
    }
    if ((data_character[0] >= 161) && (data_character[0] <= 960)) {
        data_group[0] = 1;
    }
    if ((data_character[0] >= 961) && (data_character[0] <= 2014)) {
        data_group[0] = 2;
    }
    if ((data_character[0] >= 2015) && (data_character[0] <= 2714)) {
        data_group[0] = 3;
    }
    if ((data_character[0] >= 2715) && (data_character[0] <= 2840)) {
        data_group[0] = 4;
    }

    if ((data_character[1] >= 0) && (data_character[1] <= 335)) {
        data_group[1] = 5;
    }
    if ((data_character[1] >= 336) && (data_character[1] <= 1035)) {
        data_group[1] = 6;
    }
    if ((data_character[1] >= 1036) && (data_character[1] <= 1515)) {
        data_group[1] = 7;
    }
    if ((data_character[1] >= 1516) && (data_character[1] <= 1596)) {
        data_group[1] = 8;
    }

    if ((data_character[3] >= 0) && (data_character[3] <= 335)) {
        data_group[3] = 5;
    }
    if ((data_character[3] >= 336) && (data_character[3] <= 1035)) {
        data_group[3] = 6;
    }
    if ((data_character[3] >= 1036) && (data_character[3] <= 1515)) {
        data_group[3] = 7;
    }
    if ((data_character[3] >= 1516) && (data_character[3] <= 1596)) {
        data_group[3] = 8;
    }

    if ((data_character[2] >= 0) && (data_character[2] <= 160)) {
        data_group[2] = 0;
    }
    if ((data_character[2] >= 161) && (data_character[2] <= 960)) {
        data_group[2] = 1;
    }
    if ((data_character[2] >= 961) && (data_character[2] <= 2014)) {
        data_group[2] = 2;
    }
    if ((data_character[2] >= 2015) && (data_character[2] <= 2714)) {
        data_group[2] = 3;
    }
    if ((data_character[2] >= 2715) && (data_character[2] <= 2840)) {
        data_group[2] = 4;
    }

    v_odd[0] = (data_character[0] - g_sum_table[data_group[0]]) / t_table[data_group[0]];
    v_even[0] = (data_character[0] - g_sum_table[data_group[0]]) % t_table[data_group[0]];
    v_odd[1] = (data_character[1] - g_sum_table[data_group[1]]) % t_table[data_group[1]];
    v_even[1] = (data_character[1] - g_sum_table[data_group[1]]) / t_table[data_group[1]];
    v_odd[3] = (data_character[3] - g_sum_table[data_group[3]]) % t_table[data_group[3]];
    v_even[3] = (data_character[3] - g_sum_table[data_group[3]]) / t_table[data_group[3]];
    v_odd[2] = (data_character[2] - g_sum_table[data_group[2]]) / t_table[data_group[2]];
    v_even[2] = (data_character[2] - g_sum_table[data_group[2]]) % t_table[data_group[2]];


    /* Use RSS subset width algorithm */
    for (i = 0; i < 4; i++) {
        if ((i == 0) || (i == 2)) {
            getRSSwidths(widths, v_odd[i], modules_odd[data_group[i]], 4, widest_odd[data_group[i]], 1);
            data_widths[0][i] = widths[0];
            data_widths[2][i] = widths[1];
            data_widths[4][i] = widths[2];
            data_widths[6][i] = widths[3];
            getRSSwidths(widths, v_even[i], modules_even[data_group[i]], 4, widest_even[data_group[i]], 0);
            data_widths[1][i] = widths[0];
            data_widths[3][i] = widths[1];
            data_widths[5][i] = widths[2];
            data_widths[7][i] = widths[3];
        } else {
            getRSSwidths(widths, v_odd[i], modules_odd[data_group[i]], 4, widest_odd[data_group[i]], 0);
            data_widths[0][i] = widths[0];
            data_widths[2][i] = widths[1];
            data_widths[4][i] = widths[2];
            data_widths[6][i] = widths[3];
            getRSSwidths(widths, v_even[i], modules_even[data_group[i]], 4, widest_even[data_group[i]], 1);
            data_widths[1][i] = widths[0];
            data_widths[3][i] = widths[1];
            data_widths[5][i] = widths[2];
            data_widths[7][i] = widths[3];
        }
    }


    checksum = 0;
    /* Calculate the checksum */
    for (i = 0; i < 8; i++) {
        checksum += checksum_weight[i] * data_widths[i][0];
        checksum += checksum_weight[i + 8] * data_widths[i][1];
        checksum += checksum_weight[i + 16] * data_widths[i][2];
        checksum += checksum_weight[i + 24] * data_widths[i][3];
    }
    checksum %= 79;

    /* Calculate the two check characters */
    if (checksum >= 8) {
        checksum++;
    }
    if (checksum >= 72) {
        checksum++;
    }
    c_left = checksum / 9;
    c_right = checksum % 9;

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("c_left: %d,  c_right: %d\n", c_left, c_right);
    }

    /* Put element widths together */
    total_widths[0] = 1;
    total_widths[1] = 1;
    total_widths[44] = 1;
    total_widths[45] = 1;
    for (i = 0; i < 8; i++) {
        total_widths[i + 2] = data_widths[i][0];
        total_widths[i + 15] = data_widths[7 - i][1];
        total_widths[i + 23] = data_widths[i][3];
        total_widths[i + 36] = data_widths[7 - i][2];
    }
    for (i = 0; i < 5; i++) {
        total_widths[i + 10] = finder_pattern[i + (5 * c_left)];
        total_widths[i + 31] = finder_pattern[(4 - i) + (5 * c_right)];
    }

    /* Put this data into the symbol */
    if ((symbol->symbology == BARCODE_DBAR_OMN) || (symbol->symbology == BARCODE_DBAR_OMN_CC)) {
        writer = 0;
        latch = '0';
        for (i = 0; i < 46; i++) {
            writer = rss_expand(symbol, writer, &latch, total_widths[i]);
        }
        if (symbol->width < writer) {
            symbol->width = writer;
        }
        if (symbol->symbology == BARCODE_DBAR_OMN_CC) {
            /* separator pattern for composite symbol */
            rss14_separator(symbol, 96, separator_row, 1 /*above*/, 18, 63, 0 /*bottom_finder_value_3*/);
        }
        symbol->rows = symbol->rows + 1;

        /* Set human readable text */
        set_gtin14_hrt(symbol, source, src_len);

        set_minimum_height(symbol, 14); // Minimum height is 14X for truncated symbol
    }

    if ((symbol->symbology == BARCODE_DBAR_STK) || (symbol->symbology == BARCODE_DBAR_STK_CC)) {
        /* top row */
        writer = 0;
        latch = '0';
        for (i = 0; i < 23; i++) {
            writer = rss_expand(symbol, writer, &latch, total_widths[i]);
        }
        set_module(symbol, symbol->rows, writer);
        unset_module(symbol, symbol->rows, writer + 1);
        symbol->row_height[symbol->rows] = 5;

        /* bottom row */
        symbol->rows = symbol->rows + 2;
        set_module(symbol, symbol->rows, 0);
        unset_module(symbol, symbol->rows, 1);
        writer = 2;
        latch = '1';
        for (i = 23; i < 46; i++) {
            writer = rss_expand(symbol, writer, &latch, total_widths[i]);
        }
        symbol->row_height[symbol->rows] = 7;

        /* separator pattern */
        /* See #183 for this interpretation of ISO/IEC 24724:2011 5.3.2.1 */
        for (i = 1; i < 46; i++) {
            if (module_is_set(symbol, symbol->rows - 2, i) == module_is_set(symbol, symbol->rows, i)) {
                if (!(module_is_set(symbol, symbol->rows - 2, i))) {
                    set_module(symbol, symbol->rows - 1, i);
                }
            } else {
                if (!(module_is_set(symbol, symbol->rows - 1, i - 1))) {
                    set_module(symbol, symbol->rows - 1, i);
                }
            }
        }
        unset_module(symbol, symbol->rows - 1, 1);
        unset_module(symbol, symbol->rows - 1, 2);
        unset_module(symbol, symbol->rows - 1, 3);
        symbol->row_height[symbol->rows - 1] = 1;

        if (symbol->symbology == BARCODE_DBAR_STK_CC) {
            /* separator pattern for composite symbol */
            rss14_separator(symbol, 50, separator_row, 1 /*above*/, 18, 0, 0 /*bottom_finder_value_3*/);
        }
        symbol->rows = symbol->rows + 1;
        if (symbol->width < 50) {
            symbol->width = 50;
        }
    }

    if ((symbol->symbology == BARCODE_DBAR_OMNSTK) || (symbol->symbology == BARCODE_DBAR_OMNSTK_CC)) {
        /* top row */
        writer = 0;
        latch = '0';
        for (i = 0; i < 23; i++) {
            writer = rss_expand(symbol, writer, &latch, total_widths[i]);
        }
        set_module(symbol, symbol->rows, writer);
        unset_module(symbol, symbol->rows, writer + 1);

        /* bottom row */
        symbol->rows = symbol->rows + 4;
        set_module(symbol, symbol->rows, 0);
        unset_module(symbol, symbol->rows, 1);
        writer = 2;
        latch = '1';
        for (i = 23; i < 46; i++) {
            writer = rss_expand(symbol, writer, &latch, total_widths[i]);
        }

        /* middle separator */
        for (i = 5; i < 46; i += 2) {
            set_module(symbol, symbol->rows - 2, i);
        }
        symbol->row_height[symbol->rows - 2] = 1;

        /* top separator */
        rss14_separator(symbol, 50, symbol->rows - 3, -1 /*below*/, 18, 0, 0 /*bottom_finder_value_3*/);
        symbol->row_height[symbol->rows - 3] = 1;

        /* bottom separator */
        /* 17 == 2 (guard) + 15 (inner char); +2 to skip over finder elements 4 & 5 (right to left) */
        rss14_separator(symbol, 50, symbol->rows - 1, 1 /*above*/, 17 + 2, 0, c_right == 3);
        symbol->row_height[symbol->rows - 1] = 1;
        if (symbol->width < 50) {
            symbol->width = 50;
        }

        if (symbol->symbology == BARCODE_DBAR_OMNSTK_CC) {
            /* separator pattern for composite symbol */
            rss14_separator(symbol, 50, separator_row, 1 /*above*/, 18, 0, 0 /*bottom_finder_value_3*/);
        }
        symbol->rows = symbol->rows + 1;

        set_minimum_height(symbol, 33);
    }

    return error_number;
}

/* GS1 DataBar Limited */
INTERNAL int rsslimited(struct zint_symbol *symbol, unsigned char source[], int src_len) {
    int error_number = 0, i;
    large_int accum;
    uint64_t left_character, right_character;
    int left_group, right_group, left_odd, left_even, right_odd, right_even;
    int left_widths[14], right_widths[14];
    int checksum, check_elements[14], total_widths[46], writer;
    char latch;
    int separator_row;
    int widths[7];

    separator_row = 0;

    if (src_len > 14) { /* Allow check digit to be specified (will be verified and ignored) */
        strcpy(symbol->errtxt, "382: Input too long");
        return ZINT_ERROR_TOO_LONG;
    }
    error_number = is_sane(NEON, source, src_len);
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "383: Invalid characters in data");
        return error_number;
    }

    if (src_len == 14) { /* Verify check digit */
        if (calc_check_digit(source) != ctoi(source[13])) {
            strcpy(symbol->errtxt, "389: Invalid check digit");
            return ZINT_ERROR_INVALID_CHECK;
        }
        src_len--; /* Ignore */
    }

    if (src_len == 13) {
        if ((source[0] != '0') && (source[0] != '1')) {
            strcpy(symbol->errtxt, "384: Input out of range");
            return ZINT_ERROR_INVALID_DATA;
        }
    }

    /* make some room for a separator row for composite symbols */
    if (symbol->symbology == BARCODE_DBAR_LTD_CC) {
        separator_row = symbol->rows;
        symbol->row_height[separator_row] = 1;
        symbol->rows += 1;
    }

    large_load_str_u64(&accum, source, src_len);

    if (symbol->option_1 == 2) {
        /* Add symbol linkage flag */
        large_add_u64(&accum, 2015133531096);
    }

    /* Calculate left and right pair values */

    right_character = large_div_u64(&accum, 2013571);
    left_character = large_lo(&accum);

    if (left_character >= 1996939) {
        left_group = 6;
        left_character -= 1996939;
    } else if (left_character >= 1979845) {
        left_group = 5;
        left_character -= 1979845;
    } else if (left_character >= 1491021) {
        left_group = 4;
        left_character -= 1491021;
    } else if (left_character >= 1000776) {
        left_group = 3;
        left_character -= 1000776;
    } else if (left_character >= 820064) {
        left_group = 2;
        left_character -= 820064;
    } else if (left_character >= 183064) {
        left_group = 1;
        left_character -= 183064;
    } else {
        left_group = 0;
    }

    if (right_character >= 1996939) {
        right_group = 6;
        right_character -= 1996939;
    } else if (right_character >= 1979845) {
        right_group = 5;
        right_character -= 1979845;
    } else if (right_character >= 1491021) {
        right_group = 4;
        right_character -= 1491021;
    } else if (right_character >= 1000776) {
        right_group = 3;
        right_character -= 1000776;
    } else if (right_character >= 820064) {
        right_group = 2;
        right_character -= 820064;
    } else if (right_character >= 183064) {
        right_group = 1;
        right_character -= 183064;
    } else {
        right_group = 0;
    }

    left_odd = left_character / t_even_ltd[left_group];
    left_even = left_character % t_even_ltd[left_group];
    right_odd = right_character / t_even_ltd[right_group];
    right_even = right_character % t_even_ltd[right_group];

    getRSSwidths(widths, left_odd, modules_odd_ltd[left_group], 7, widest_odd_ltd[left_group], 1);
    for (i = 0; i <= 6; i++) {
        left_widths[i * 2] = widths[i];
    }
    getRSSwidths(widths, left_even, modules_even_ltd[left_group], 7, widest_even_ltd[left_group], 0);
    for (i = 0; i <= 6; i++) {
        left_widths[i * 2 + 1] = widths[i];
    }
    getRSSwidths(widths, right_odd, modules_odd_ltd[right_group], 7, widest_odd_ltd[right_group], 1);
    for (i = 0; i <= 6; i++) {
        right_widths[i * 2] = widths[i];
    }
    getRSSwidths(widths, right_even, modules_even_ltd[right_group], 7, widest_even_ltd[right_group], 0);
    for (i = 0; i <= 6; i++) {
        right_widths[i * 2 + 1] = widths[i];
    }

    checksum = 0;
    /* Calculate the checksum */
    for (i = 0; i < 14; i++) {
        checksum += checksum_weight_ltd[i] * left_widths[i];
        checksum += checksum_weight_ltd[i + 14] * right_widths[i];
    }
    checksum %= 89;

    for (i = 0; i < 14; i++) {
        check_elements[i] = finder_pattern_ltd[i + (checksum * 14)];
    }

    total_widths[0] = 1;
    total_widths[1] = 1;
    total_widths[44] = 1;
    total_widths[45] = 1;
    for (i = 0; i < 14; i++) {
        total_widths[i + 2] = left_widths[i];
        total_widths[i + 16] = check_elements[i];
        total_widths[i + 30] = right_widths[i];
    }

    writer = 0;
    latch = '0';
    for (i = 0; i < 46; i++) {
        writer = rss_expand(symbol, writer, &latch, total_widths[i]);
    }
    if (symbol->width < writer) {
        symbol->width = writer;
    }
    symbol->rows = symbol->rows + 1;

    /* add separator pattern if composite symbol */
    if (symbol->symbology == BARCODE_DBAR_LTD_CC) {
        for (i = 4; i < 70; i++) {
            if (!(module_is_set(symbol, separator_row + 1, i))) {
                set_module(symbol, separator_row, i);
            }
        }
    }

    /* Set human readable text */
    set_gtin14_hrt(symbol, source, src_len);

    set_minimum_height(symbol, 10);

    return error_number;
}

/* Handles all data encodation from section 7.2.5 of ISO/IEC 24724 */
static int rss_binary_string(struct zint_symbol *symbol, char source[], char binary_string[]) {
    int encoding_method, i, j, read_posn, last_digit, debug = (symbol->debug & ZINT_DEBUG_PRINT), mode = NUMERIC;
    int symbol_characters, characters_per_row;
#ifndef _MSC_VER
    char general_field[strlen(source) + 1];
#else
    char* general_field = (char*) _alloca(strlen(source) + 1);
#endif
    int remainder, d1, d2;
    char padstring[40];

    /* Decide whether a compressed data field is required and if so what
    method to use - method 2 = no compressed data field */

    if ((strlen(source) >= 16) && ((source[0] == '0') && (source[1] == '1'))) {
        /* (01) and other AIs */
        encoding_method = 1;
        if (debug) printf("Choosing Method 1\n");
    } else {
        /* any AIs */
        encoding_method = 2;
        if (debug) printf("Choosing Method 2\n");
    }

    if (((strlen(source) >= 20) && (encoding_method == 1)) && ((source[2] == '9') && (source[16] == '3'))) {
        /* Possibly encoding method > 2 */
        if (debug) printf("Checking for other methods\n");

        if ((strlen(source) >= 26) && (source[17] == '1')) {
            /* Methods 3, 7, 9, 11 and 13 */

            if (source[18] == '0') {
                /* (01) and (310x) */
                char weight_str[7];

                for (i = 0; i < 6; i++) {
                    weight_str[i] = source[20 + i];
                }
                weight_str[6] = '\0';

                if (weight_str[0] == '0') { /* Maximum weight = 99999 */

                    if ((source[19] == '3') && (strlen(source) == 26)) {
                        /* (01) and (3103) */
                        float weight; /* In kilos */
                        weight = atof(weight_str) / 1000.0;

                        if (weight <= 32.767) {
                            encoding_method = 3;
                        }
                    }

                    if (strlen(source) == 34) {
                        if ((source[26] == '1') && (source[27] == '1')) {
                            /* (01), (310x) and (11) - metric weight and production date */
                            encoding_method = 7;
                        }

                        if ((source[26] == '1') && (source[27] == '3')) {
                            /* (01), (310x) and (13) - metric weight and packaging date */
                            encoding_method = 9;
                        }

                        if ((source[26] == '1') && (source[27] == '5')) {
                            /* (01), (310x) and (15) - metric weight and "best before" date */
                            encoding_method = 11;
                        }

                        if ((source[26] == '1') && (source[27] == '7')) {
                            /* (01), (310x) and (17) - metric weight and expiration date */
                            encoding_method = 13;
                        }
                    }
                }
            }
            if (debug) printf("Now using method %d\n", encoding_method);
        }

        if ((strlen(source) >= 26) && (source[17] == '2')) {
            /* Methods 4, 8, 10, 12 and 14 */

            if (source[18] == '0') {
                /* (01) and (320x) */
                char weight_str[7];

                for (i = 0; i < 6; i++) {
                    weight_str[i] = source[20 + i];
                }
                weight_str[6] = '\0';

                if (weight_str[0] == '0') { /* Maximum weight = 99999 */

                    if (((source[19] == '2') || (source[19] == '3')) && (strlen(source) == 26)) {
                        /* (01) and (3202)/(3203) */
                        float weight; /* In pounds */

                        if (source[19] == '3') {
                            weight = (float) (atof(weight_str) / 1000.0F);
                            if (weight <= 22.767) {
                                encoding_method = 4;
                            }
                        } else {
                            weight = (float) (atof(weight_str) / 100.0F);
                            if (weight <= 99.99) {
                                encoding_method = 4;
                            }
                        }

                    }

                    if (strlen(source) == 34) {
                        if ((source[26] == '1') && (source[27] == '1')) {
                            /* (01), (320x) and (11) - English weight and production date */
                            encoding_method = 8;
                        }

                        if ((source[26] == '1') && (source[27] == '3')) {
                            /* (01), (320x) and (13) - English weight and packaging date */
                            encoding_method = 10;
                        }

                        if ((source[26] == '1') && (source[27] == '5')) {
                            /* (01), (320x) and (15) - English weight and "best before" date */
                            encoding_method = 12;
                        }

                        if ((source[26] == '1') && (source[27] == '7')) {
                            /* (01), (320x) and (17) - English weight and expiration date */
                            encoding_method = 14;
                        }
                    }
                }
            }
            if (debug) printf("Now using method %d\n", encoding_method);

        }

        if (source[17] == '9') {
            /* Methods 5 and 6 */
            if ((source[18] == '2') && ((source[19] >= '0') && (source[19] <= '3'))) {
                /* (01) and (392x) */
                encoding_method = 5;
            }
            if ((source[18] == '3') && ((source[19] >= '0') && (source[19] <= '3'))) {
                /* (01) and (393x) */
                encoding_method = 6;
            }
            if (debug) printf("Now using method %d\n", encoding_method);
        }
    }

    switch (encoding_method) { /* Encoding method - Table 10 */
        case 1: strcat(binary_string, "1XX");
            read_posn = 16;
            break;
        case 2: strcat(binary_string, "00XX");
            read_posn = 0;
            break;
        case 3: // 0100
        case 4: // 0101
            bin_append(4 + (encoding_method - 3), 4, binary_string);
            read_posn = strlen(source);
            break;
        case 5: strcat(binary_string, "01100XX");
            read_posn = 20;
            break;
        case 6: strcat(binary_string, "01101XX");
            read_posn = 23;
            break;
        default: /* modes 7 to 14 */
            bin_append(56 + (encoding_method - 7), 7, binary_string);
            read_posn = strlen(source);
            break;
    }
    if (debug) printf("Setting binary = %s\n", binary_string);

    /* Variable length symbol bit field is just given a place holder (XX)
    for the time being */

    /* Verify that the data to be placed in the compressed data field is all
    numeric data before carrying out compression */
    for (i = 0; i < read_posn; i++) {
        if ((source[i] < '0') || (source[i] > '9')) {
            if ((source[i] != '[') && (source[i] != ']')) {
                /* Something is wrong */
                strcpy(symbol->errtxt, "385: Invalid characters in input data");
                return ZINT_ERROR_INVALID_DATA;
            }
        }
    }

    /* Now encode the compressed data field */

    if (debug) printf("Proceeding to encode data\n");
    if (encoding_method == 1) {
        /* Encoding method field "1" - general item identification data */
        char group[4];

        group[0] = source[2];
        group[1] = '\0';

        bin_append(atoi(group), 4, binary_string);

        for (i = 1; i < 5; i++) {
            group[0] = source[(i * 3)];
            group[1] = source[(i * 3) + 1];
            group[2] = source[(i * 3) + 2];
            group[3] = '\0';

            bin_append(atoi(group), 10, binary_string);
        }
    }

    if ((encoding_method == 3) || (encoding_method == 4)) {
        /* Encoding method field "0100" - variable weight item
        (0,001 kilogram icrements) */
        /* Encoding method field "0101" - variable weight item (0,01 or
        0,001 pound increment) */
        char group[4];
        char weight_str[7];

        for (i = 1; i < 5; i++) {
            group[0] = source[(i * 3)];
            group[1] = source[(i * 3) + 1];
            group[2] = source[(i * 3) + 2];
            group[3] = '\0';

            bin_append(atoi(group), 10, binary_string);
        }

        for (i = 0; i < 6; i++) {
            weight_str[i] = source[20 + i];
        }
        weight_str[6] = '\0';

        if ((encoding_method == 4) && (source[19] == '3')) {
            bin_append(atoi(weight_str) + 10000, 15, binary_string);
        } else {
            bin_append(atoi(weight_str), 15, binary_string);
        }
    }

    if ((encoding_method == 5) || (encoding_method == 6)) {
        /* Encoding method "01100" - variable measure item and price */
        /* Encoding method "01101" - variable measure item and price with ISO 4217
        Currency Code */

        char group[4];

        for (i = 1; i < 5; i++) {
            group[0] = source[(i * 3)];
            group[1] = source[(i * 3) + 1];
            group[2] = source[(i * 3) + 2];
            group[3] = '\0';

            bin_append(atoi(group), 10, binary_string);
        }

        bin_append(source[19] - '0', 2, binary_string);

        if (encoding_method == 6) {
            char currency_str[5];

            for (i = 0; i < 3; i++) {
                currency_str[i] = source[20 + i];
            }
            currency_str[3] = '\0';

            bin_append(atoi(currency_str), 10, binary_string);
        }
    }

    if ((encoding_method >= 7) && (encoding_method <= 14)) {
        /* Encoding method fields "0111000" through "0111111" - variable
        weight item plus date */
        char group[4];
        int group_val;
        char weight_str[8];

        for (i = 1; i < 5; i++) {
            group[0] = source[(i * 3)];
            group[1] = source[(i * 3) + 1];
            group[2] = source[(i * 3) + 2];
            group[3] = '\0';

            bin_append(atoi(group), 10, binary_string);
        }

        weight_str[0] = source[19];

        for (i = 0; i < 5; i++) {
            weight_str[i + 1] = source[21 + i];
        }
        weight_str[6] = '\0';

        bin_append(atoi(weight_str), 20, binary_string);

        if (strlen(source) == 34) {
            /* Date information is included */
            char date_str[4];
            date_str[0] = source[28];
            date_str[1] = source[29];
            date_str[2] = '\0';
            group_val = atoi(date_str) * 384;

            date_str[0] = source[30];
            date_str[1] = source[31];
            group_val += (atoi(date_str) - 1) * 32;

            date_str[0] = source[32];
            date_str[1] = source[33];
            group_val += atoi(date_str);
        } else {
            group_val = 38400;
        }

        bin_append(group_val, 16, binary_string);
    }

    /* The compressed data field has been processed if appropriate - the
    rest of the data (if any) goes into a general-purpose data compaction field */

    j = 0;
    for (i = read_posn; i < (int) strlen(source); i++) {
        general_field[j] = source[i];
        j++;
    }
    general_field[j] = '\0';
    if (debug) printf("General field data = %s\n", general_field);

    if (!general_field_encode(general_field, &mode, &last_digit, binary_string)) {
        /* Invalid characters in input data */
        strcpy(symbol->errtxt, "386: Invalid characters in input data");
        return ZINT_ERROR_INVALID_DATA;
    }
    if (debug) printf("Resultant binary = %s\n", binary_string);
    if (debug) printf("\tLength: %d\n", (int) strlen(binary_string));

    remainder = 12 - (strlen(binary_string) % 12);
    if (remainder == 12) {
        remainder = 0;
    }
    symbol_characters = ((strlen(binary_string) + remainder) / 12) + 1;

    if ((symbol->symbology == BARCODE_DBAR_EXPSTK) || (symbol->symbology == BARCODE_DBAR_EXPSTK_CC)) {
        characters_per_row = symbol->option_2 * 2;

        if ((characters_per_row < 2) || (characters_per_row > 20)) {
            characters_per_row = 4;
        }

        if ((symbol_characters % characters_per_row) == 1) {
            symbol_characters++;
        }
    }

    if (symbol_characters < 4) {
        symbol_characters = 4;
    }

    remainder = (12 * (symbol_characters - 1)) - strlen(binary_string);

    if (last_digit) {
        /* There is still one more numeric digit to encode */
        if (debug) printf("Adding extra (odd) numeric digit\n");

        if ((remainder >= 4) && (remainder <= 6)) {
            bin_append(ctoi(last_digit) + 1, 4, binary_string);
        } else {
            d1 = ctoi(last_digit);
            d2 = 10;

            bin_append((11 * d1) + d2 + 8, 7, binary_string);
        }

        remainder = 12 - (strlen(binary_string) % 12);
        if (remainder == 12) {
            remainder = 0;
        }
        symbol_characters = ((strlen(binary_string) + remainder) / 12) + 1;

        if ((symbol->symbology == BARCODE_DBAR_EXPSTK) || (symbol->symbology == BARCODE_DBAR_EXPSTK_CC)) {
            characters_per_row = symbol->option_2 * 2;

            if ((characters_per_row < 2) || (characters_per_row > 20)) {
                characters_per_row = 4;
            }

            if ((symbol_characters % characters_per_row) == 1) {
                symbol_characters++;
            }
        }

        if (symbol_characters < 4) {
            symbol_characters = 4;
        }

        remainder = (12 * (symbol_characters - 1)) - strlen(binary_string);

        if (debug) printf("Resultant binary = %s\n", binary_string);
        if (debug) printf("\tLength: %d\n", (int) strlen(binary_string));
    }

    if (strlen(binary_string) > 252) { /* 252 = (21 * 12) */
        strcpy(symbol->errtxt, "387: Input too long");
        return ZINT_ERROR_TOO_LONG;
    }

    /* Now add padding to binary string (7.2.5.5.4) */
    i = remainder;
    if (mode == NUMERIC) {
        strcpy(padstring, "0000");
        i -= 4;
    } else {
        strcpy(padstring, "");
    }
    for (; i > 0; i -= 5) {
        strcat(padstring, "00100");
    }

    padstring[remainder] = '\0';
    strcat(binary_string, padstring);

    /* Patch variable length symbol bit field */
    d1 = symbol_characters & 1;

    if (symbol_characters <= 14) {
        d2 = 0;
    } else {
        d2 = 1;
    }

    if (encoding_method == 1) {
        binary_string[2] = d1 ? '1' : '0';
        binary_string[3] = d2 ? '1' : '0';
    }
    if (encoding_method == 2) {
        binary_string[3] = d1 ? '1' : '0';
        binary_string[4] = d2 ? '1' : '0';
    }
    if ((encoding_method == 5) || (encoding_method == 6)) {
        binary_string[6] = d1 ? '1' : '0';
        binary_string[7] = d2 ? '1' : '0';
    }
    if (debug) printf("Resultant binary = %s\n", binary_string);
    if (debug) printf("\tLength: %d\n", (int) strlen(binary_string));
    return 0;
}

static void rssexp_separator(struct zint_symbol *symbol, int width, int cols, int separator_row, int above_below, int special_case_row, int left_to_right, int odd_last_row, int *p_v2_latch) {
    int i, i_start, i_end, j, k;
    int module_row = separator_row + above_below;
    int v2_latch = p_v2_latch ? *p_v2_latch : 0;
    int space_latch = 0;

    for (j = 4 + special_case_row, width -= 4; j < width; j++) {
        if (module_is_set(symbol, module_row, j)) {
            unset_module(symbol, separator_row, j);
        } else {
            set_module(symbol, separator_row, j);
        }
    }

    /* finder adjustment */
    for (j = 0; j < cols; j++) {
        k = (49 * j) + 19 + special_case_row; /* 49 == data (17) + finder (15) + data(17) triplet, 19 == 2 (guard) + 17 (initial check/data character) */
        if (left_to_right) {
            i_start = v2_latch ? 2 : 0; /* Last 13 modules of version 2 finder and first 13 modules of version 1 finder */
            i_end = v2_latch ? 15 : 13;
            for (i = i_start; i < i_end; i++) {
                if (module_is_set(symbol, module_row, i + k)) {
                    unset_module(symbol, separator_row, i + k);
                    space_latch = 0;
                } else {
                    if (space_latch) {
                        unset_module(symbol, separator_row, i + k);
                    } else {
                        set_module(symbol, separator_row, i + k);
                    }
                    space_latch = !space_latch;
                }
            }
        } else {
            if (odd_last_row) {
                k -= 17; /* No data char at beginning of row, i.e. ends with finder */
            }
            i_start = v2_latch ? 14 : 12; /* First 13 modules of version 1 finder and last 13 modules of version 2 finder */
            i_end = v2_latch ? 2 : 0;
            for (i = i_start; i >= i_end; i--) {
                if (module_is_set(symbol, module_row, i + k)) {
                    unset_module(symbol, separator_row, i + k);
                    space_latch = 0;
                } else {
                    if (space_latch) {
                        unset_module(symbol, separator_row, i + k);
                    } else {
                        set_module(symbol, separator_row, i + k);
                    }
                    space_latch = !space_latch;
                }
            }
        }
        v2_latch = !v2_latch;
    }

    if (p_v2_latch && above_below == -1) { /* Only set if below */
        *p_v2_latch = v2_latch;
    }
}

/* GS1 DataBar Expanded */
INTERNAL int rssexpanded(struct zint_symbol *symbol, unsigned char source[], int src_len) {
    int i, j, k, p, data_chars, vs[21], group[21], v_odd[21], v_even[21];
    char substring[21][14], latch;
    int char_widths[21][8], checksum, check_widths[8], c_group;
    int check_char, c_odd, c_even, elements[235], pattern_width, reader, writer;
    int separator_row;
    unsigned int bin_len = 13 * src_len + 200 + 1; /* Allow for 8 bits + 5-bit latch per char + 200 bits overhead/padding */
    int widths[4];
#ifndef _MSC_VER
    char reduced[src_len + 1], binary_string[bin_len];
#else
    char* reduced = (char*) _alloca(src_len + 1);
    char* binary_string = (char*) _alloca(bin_len);
#endif

    separator_row = 0;

    i = gs1_verify(symbol, source, src_len, reduced);
    if (i != 0) {
        return i;
    }

    if ((symbol->symbology == BARCODE_DBAR_EXP_CC) || (symbol->symbology == BARCODE_DBAR_EXPSTK_CC)) {
        /* make space for a composite separator pattern */
        separator_row = symbol->rows;
        symbol->row_height[separator_row] = 1;
        symbol->rows += 1;
    }

    strcpy(binary_string, "");

    if (symbol->option_1 == 2) { /* The "component linkage" flag */
        strcat(binary_string, "1");
    } else {
        strcat(binary_string, "0");
    }

    i = rss_binary_string(symbol, reduced, binary_string);
    if (i != 0) {
        return i;
    }

    data_chars = strlen(binary_string) / 12;

    for (i = 0; i < data_chars; i++) {
        for (j = 0; j < 12; j++) {
            substring[i][j] = binary_string[(i * 12) + j];
        }
        substring[i][12] = '\0';
    }

    for (i = 0; i < data_chars; i++) {
        vs[i] = 0;
        for (p = 0; p < 12; p++) {
            if (substring[i][p] == '1') {
                vs[i] += (0x800 >> p);
            }
        }
    }

    for (i = 0; i < data_chars; i++) {
        if (vs[i] <= 347) {
            group[i] = 1;
        }
        if ((vs[i] >= 348) && (vs[i] <= 1387)) {
            group[i] = 2;
        }
        if ((vs[i] >= 1388) && (vs[i] <= 2947)) {
            group[i] = 3;
        }
        if ((vs[i] >= 2948) && (vs[i] <= 3987)) {
            group[i] = 4;
        }
        if (vs[i] >= 3988) {
            group[i] = 5;
        }
        v_odd[i] = (vs[i] - g_sum_exp[group[i] - 1]) / t_even_exp[group[i] - 1];
        v_even[i] = (vs[i] - g_sum_exp[group[i] - 1]) % t_even_exp[group[i] - 1];

        getRSSwidths(widths, v_odd[i], modules_odd_exp[group[i] - 1], 4, widest_odd_exp[group[i] - 1], 0);
        char_widths[i][0] = widths[0];
        char_widths[i][2] = widths[1];
        char_widths[i][4] = widths[2];
        char_widths[i][6] = widths[3];
        getRSSwidths(widths, v_even[i], modules_even_exp[group[i] - 1], 4, widest_even_exp[group[i] - 1], 1);
        char_widths[i][1] = widths[0];
        char_widths[i][3] = widths[1];
        char_widths[i][5] = widths[2];
        char_widths[i][7] = widths[3];
    }

    /* 7.2.6 Check character */
    /* The checksum value is equal to the mod 211 residue of the weighted sum of the widths of the
       elements in the data characters. */
    checksum = 0;
    for (i = 0; i < data_chars; i++) {
        int row = weight_rows[(((data_chars - 2) / 2) * 21) + i];
        for (j = 0; j < 8; j++) {
            checksum += (char_widths[i][j] * checksum_weight_exp[(row * 8) + j]);

        }
    }

    check_char = (211 * ((data_chars + 1) - 4)) + (checksum % 211);

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("Data chars: %d, Check char: %d\n", data_chars, check_char);
    }

    if (check_char <= 347) {
        c_group = 1;
    }
    if ((check_char >= 348) && (check_char <= 1387)) {
        c_group = 2;
    }
    if ((check_char >= 1388) && (check_char <= 2947)) {
        c_group = 3;
    }
    if ((check_char >= 2948) && (check_char <= 3987)) {
        c_group = 4;
    }
    if (check_char >= 3988) {
        c_group = 5;
    }

    c_odd = (check_char - g_sum_exp[c_group - 1]) / t_even_exp[c_group - 1];
    c_even = (check_char - g_sum_exp[c_group - 1]) % t_even_exp[c_group - 1];

    getRSSwidths(widths, c_odd, modules_odd_exp[c_group - 1], 4, widest_odd_exp[c_group - 1], 0);
    check_widths[0] = widths[0];
    check_widths[2] = widths[1];
    check_widths[4] = widths[2];
    check_widths[6] = widths[3];
    getRSSwidths(widths, c_even, modules_even_exp[c_group - 1], 4, widest_even_exp[c_group - 1], 1);
    check_widths[1] = widths[0];
    check_widths[3] = widths[1];
    check_widths[5] = widths[2];
    check_widths[7] = widths[3];

    /* Initialise element array */
    pattern_width = ((((data_chars + 1) / 2) + ((data_chars + 1) & 1)) * 5) + ((data_chars + 1) * 8) + 4;
    for (i = 0; i < pattern_width; i++) {
        elements[i] = 0;
    }

    /* Put finder patterns in element array */
    for (i = 0; i < (((data_chars + 1) / 2) + ((data_chars + 1) & 1)); i++) {
        k = ((((((data_chars + 1) - 2) / 2) + ((data_chars + 1) & 1)) - 1) * 11) + i;
        for (j = 0; j < 5; j++) {
            elements[(21 * i) + j + 10] = finder_pattern_exp[((finder_sequence[k] - 1) * 5) + j];
        }
    }

    /* Put check character in element array */
    for (i = 0; i < 8; i++) {
        elements[i + 2] = check_widths[i];
    }

    /* Put forward reading data characters in element array */
    for (i = 1; i < data_chars; i += 2) {
        for (j = 0; j < 8; j++) {
            elements[(((i - 1) / 2) * 21) + 23 + j] = char_widths[i][j];
        }
    }

    /* Put reversed data characters in element array */
    for (i = 0; i < data_chars; i += 2) {
        for (j = 0; j < 8; j++) {
            elements[((i / 2) * 21) + 15 + j] = char_widths[i][7 - j];
        }
    }

    if ((symbol->symbology == BARCODE_DBAR_EXP) || (symbol->symbology == BARCODE_DBAR_EXP_CC)) {
        /* Copy elements into symbol */

        elements[0] = 1; // left guard
        elements[1] = 1;

        elements[pattern_width - 2] = 1; // right guard
        elements[pattern_width - 1] = 1;

        writer = 0;
        latch = '0';
        for (i = 0; i < pattern_width; i++) {
            writer = rss_expand(symbol, writer, &latch, elements[i]);
        }
        if (symbol->width < writer) {
            symbol->width = writer;
        }
        symbol->rows = symbol->rows + 1;

        /* Add human readable text */
        for (i = 0; i <= src_len; i++) {
            if ((source[i] != '[') && (source[i] != ']')) {
                symbol->text[i] = source[i];
            } else {
                if (source[i] == '[') {
                    symbol->text[i] = '(';
                }
                if (source[i] == ']') {
                    symbol->text[i] = ')';
                }
            }
        }

    } else {
        int stack_rows;
        int current_row, current_block, left_to_right;
        int v2_latch = 0;
        /* RSS Expanded Stacked */

        /* Bug corrected: Character missing for message
         * [01]90614141999996[10]1234222222222221
         * Patch by Daniel Frede
         */
        int codeblocks = (data_chars + 1) / 2 + ((data_chars + 1) % 2);

        if ((symbol->option_2 < 1) || (symbol->option_2 > 11)) {
            symbol->option_2 = 2;
        }
        if ((symbol->option_1 == 2) && (symbol->option_2 == 1)) {
            /* "There shall be a minimum of four symbol characters in the
            first row of an RSS Expanded Stacked symbol when it is the linear
            component of an EAN.UCC Composite symbol." */
            symbol->option_2 = 2;
        }

        stack_rows = codeblocks / symbol->option_2;
        if (codeblocks % symbol->option_2 > 0) {
            stack_rows++;
        }

        current_block = 0;
        for (current_row = 1; current_row <= stack_rows; current_row++) {
            int special_case_row = 0;
            int elements_in_sub;
            int sub_elements[235] = {0};
            int num_columns;

            /* Number of columns in current row */
            if (current_row * symbol->option_2 > codeblocks) {
                num_columns = codeblocks - current_block;
            } else {
                num_columns = symbol->option_2;
            }

            /* Row Start */
            sub_elements[0] = 1; // left guard
            sub_elements[1] = 1;
            elements_in_sub = 2;

            /* If last row and is partial and even-numbered, and have even columns (segment pairs), and odd number of finders */
            if ((current_row == stack_rows) && (num_columns != symbol->option_2) &&
                    !(current_row & 1) && !(symbol->option_2 & 1) && (num_columns & 1)) { /* Odd number of finders == odd number of columns */
                /* Special case bottom row */
                special_case_row = 1;
                sub_elements[0] = 2; /* Extra space (latch set below) */
            }

            /* If odd number of columns or current row odd-numbered or special case last row then left-to-right, else right-to-left */
            if ((symbol->option_2 & 1) || (current_row & 1) || special_case_row) {
                left_to_right = 1;
            } else {
                left_to_right = 0;
            }

            if (symbol->debug & ZINT_DEBUG_PRINT) {
                if (current_row == stack_rows) {
                    printf("Last row: number of columns: %d / %d, left to right: %d, special case: %d\n", num_columns, symbol->option_2, left_to_right, special_case_row);
                }
            }

            /* Row Data */
            reader = 0;
            do {
                i = 2 + (current_block * 21);
                for (j = 0; j < 21; j++) {
                    if ((i + j) < pattern_width) {
                        if (left_to_right) {
                            sub_elements[j + (reader * 21) + 2] = elements[i + j];
                        } else {
                            sub_elements[(20 - j) + (num_columns - 1 - reader) * 21 + 2] = elements[i + j];
                        }
                    }
                    elements_in_sub++;
                }
                reader++;
                current_block++;
            } while ((reader < symbol->option_2) && (current_block < codeblocks));

            /* Row Stop */
            sub_elements[elements_in_sub] = 1; // right guard
            sub_elements[elements_in_sub + 1] = 1;
            elements_in_sub += 2;

            latch = (current_row & 1) || special_case_row ? '0' : '1';

            writer = 0;
            for (i = 0; i < elements_in_sub; i++) {
                writer = rss_expand(symbol, writer, &latch, sub_elements[i]);
            }
            if (symbol->width < writer) {
                symbol->width = writer;
            }

            if (current_row != 1) {
                int odd_last_row = (current_row == stack_rows) && (data_chars % 2 == 0);

                /* middle separator pattern (above current row) */
                for (j = 5; j < (49 * symbol->option_2); j += 2) {
                    set_module(symbol, symbol->rows - 2, j);
                }
                symbol->row_height[symbol->rows - 2] = 1;

                /* bottom separator pattern (above current row) */
                rssexp_separator(symbol, writer, reader, symbol->rows - 1, 1 /*above*/, special_case_row, left_to_right, odd_last_row, &v2_latch);
                symbol->row_height[symbol->rows - 1] = 1;
            }

            if (current_row != stack_rows) {
                /* top separator pattern (below current row) */
                rssexp_separator(symbol, writer, reader, symbol->rows + 1, -1 /*below*/, 0 /*special_case_row*/, left_to_right, 0 /*odd_last_row*/, &v2_latch);
                symbol->row_height[symbol->rows + 1] = 1;
            }

            symbol->rows = symbol->rows + 4;
        }
        symbol->rows = symbol->rows - 3;
    }

    if (symbol->symbology == BARCODE_DBAR_EXP_CC || symbol->symbology == BARCODE_DBAR_EXPSTK_CC) {
        /* Composite separator */
        rssexp_separator(symbol, symbol->width, 4, separator_row, 1 /*above*/, 0 /*special_case_row*/, 1 /*left_to_right*/, 0 /*odd_last_row*/, NULL);
    }

    for (i = 0; i < symbol->rows; i++) {
        if (symbol->row_height[i] == 0) {
            symbol->row_height[i] = 34;
        }
    }

    return 0;
}
