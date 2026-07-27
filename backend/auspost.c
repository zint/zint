/* auspost.c - Handles Australia Post 4-State Barcode */
/*
    libzint - the open source barcode library
    Copyright (C) 2008-2026 Robin Stuart <rstuart114@gmail.com>

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
#include "reedsol.h"

static const char AusGDSET[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz #";
#define AUS_GDSET_F (IS_NUM_F | IS_UPR_F | IS_LWR_F | IS_SPC_F | IS_HSH_F)

/* The contents of encoding tables and data pattern `dest[]` conform to the following standard:
   0 = Tracker, Ascender and Descender
   1 = Tracker and Ascender
   2 = Tracker and Descender
   3 = Tracker only */

/* N Encoding Table (numeric) */
static const char AusNTable[10][2] = {
    { 0,0 }, { 0,1 }, { 0,2 }, { 1,0 }, { 1,1 }, { 1,2 }, { 2,0 }, { 2,1 }, { 2,2 }, { 3,0 }
};

/* C Encoding Table (GDSET) */
static const char AusCTable[64][3] = {
    { 2,2,2 }, { 3,0,0 }, { 3,0,1 }, { 3,0,2 }, { 3,1,0 }, { 3,1,1 }, { 3,1,2 }, { 3,2,0 },
    { 3,2,1 }, { 3,2,2 }, { 0,0,0 }, { 0,0,1 }, { 0,0,2 }, { 0,1,0 }, { 0,1,1 }, { 0,1,2 },
    { 0,2,0 }, { 0,2,1 }, { 0,2,2 }, { 1,0,0 }, { 1,0,1 }, { 1,0,2 }, { 1,1,0 }, { 1,1,1 },
    { 1,1,2 }, { 1,2,0 }, { 1,2,1 }, { 1,2,2 }, { 2,0,0 }, { 2,0,1 }, { 2,0,2 }, { 2,1,0 },
    { 2,1,1 }, { 2,1,2 }, { 2,2,0 }, { 2,2,1 }, { 0,2,3 }, { 0,3,0 }, { 0,3,1 }, { 0,3,2 },
    { 0,3,3 }, { 1,0,3 }, { 1,1,3 }, { 1,2,3 }, { 1,3,0 }, { 1,3,1 }, { 1,3,2 }, { 1,3,3 },
    { 2,0,3 }, { 2,1,3 }, { 2,2,3 }, { 2,3,0 }, { 2,3,1 }, { 2,3,2 }, { 2,3,3 }, { 3,0,3 },
    { 3,1,3 }, { 3,2,3 }, { 3,3,0 }, { 3,3,1 }, { 3,3,2 }, { 3,3,3 }, { 0,0,3 }, { 0,1,3 }
};

/* Bar to Decimal Conversion Table (Reed-Solomon) */
static const char AusBarTable[64][3] = {
    { 0,0,0 }, { 0,0,1 }, { 0,0,2 }, { 0,0,3 }, { 0,1,0 }, { 0,1,1 }, { 0,1,2 }, { 0,1,3 },
    { 0,2,0 }, { 0,2,1 }, { 0,2,2 }, { 0,2,3 }, { 0,3,0 }, { 0,3,1 }, { 0,3,2 }, { 0,3,3 },
    { 1,0,0 }, { 1,0,1 }, { 1,0,2 }, { 1,0,3 }, { 1,1,0 }, { 1,1,1 }, { 1,1,2 }, { 1,1,3 },
    { 1,2,0 }, { 1,2,1 }, { 1,2,2 }, { 1,2,3 }, { 1,3,0 }, { 1,3,1 }, { 1,3,2 }, { 1,3,3 },
    { 2,0,0 }, { 2,0,1 }, { 2,0,2 }, { 2,0,3 }, { 2,1,0 }, { 2,1,1 }, { 2,1,2 }, { 2,1,3 },
    { 2,2,0 }, { 2,2,1 }, { 2,2,2 }, { 2,2,3 }, { 2,3,0 }, { 2,3,1 }, { 2,3,2 }, { 2,3,3 },
    { 3,0,0 }, { 3,0,1 }, { 3,0,2 }, { 3,0,3 }, { 3,1,0 }, { 3,1,1 }, { 3,1,2 }, { 3,1,3 },
    { 3,2,0 }, { 3,2,1 }, { 3,2,2 }, { 3,2,3 }, { 3,3,0 }, { 3,3,1 }, { 3,3,2 }, { 3,3,3 }
};

/* Adds Reed-Solomon error correction to auspost */
static char *aus_rs_error(const char dest[], char *d) {
    const int length = (int) (d - dest);
    int reader, triple_writer;
    unsigned char triple[17]; /* Max bars 67 - 12 (ECC) - 4 (start/stop) = 51 / 3 */
    unsigned char result[5];
    rs_t rs;

    for (reader = 2, triple_writer = 0; reader < length; reader += 3, triple_writer++) {
        triple[triple_writer] = (dest[reader] << 4) | (dest[reader + 1] << 2) | dest[reader + 2];
    }

    zint_rs_init_gf(&rs, 0x43);
    zint_rs_init_code(&rs, 4, 1);
    zint_rs_encode(&rs, triple_writer, triple, result);

    for (reader = 0; reader < 4; reader++, d += 3) {
        memcpy(d, AusBarTable[result[reader]], 3);
    }

    return d;
}

/* In "postal.c" */
INTERNAL int zint_daft_set_height(struct zint_symbol *symbol, const float min_height, const float max_height);

/* Handles Australia Posts's 4 State Codes */
INTERNAL int zint_auspost(struct zint_symbol *symbol, unsigned char source[], int length) {
    /* Standard Customer Barcode, Customer Barcode 2 or Customer Barcode 3 system determined automatically
       (i.e. the Format Control Code (FCC) doesn't need to be specified by the user) dependent
       on the length of the input string */
    static const unsigned char fccs[7][2] = {
    /*  Null         Standard     Barcode 2    Barcode 3    Reply        Route        Redirect */
        { '0','0' }, { '1','1' }, { '5','9' }, { '6','2' }, { '4','5' }, { '8','7' }, { '9','2' }
    };
    static const char start_stop[2] = { 1,3 }; /* Full,tracker */

    int i;
    int error_number;
    int writer;
    int loopey, reader;
    int h;

    char dest[67]; /* Max bars (Customer Barcode 3) */
    char *d = dest;
    int fcc_idx; /* Index into `fccs[]` */
    unsigned char src_buf[8]; /* For zero-padded DPID */
    int not_all_digits = 0;
    const int content_segs = symbol->output_options & BARCODE_CONTENT_SEGS;

    /* Suppress clang-tidy-21 clang-analyzer-security.ArrayBound */
    assert(symbol->symbology == BARCODE_AUSPOST || symbol->symbology == BARCODE_AUSREPLY
            || symbol->symbology == BARCODE_AUSROUTE || symbol->symbology == BARCODE_AUSREDIRECT);

    if (symbol->symbology == BARCODE_AUSPOST) {
        if (length > 23) {
            return z_errtxtf(ZINT_ERROR_TOO_LONG, symbol, 401,
                            "Input length %d too long (maximum 23)", length);
        }
        not_all_digits = z_not_sane(NEON_F, source, length);
        if (length <= 8) {
            if (not_all_digits) {
                return z_errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 405,
                            "Invalid character at position %d in DPID (digits only for Standard Customer Barcode)",
                            not_all_digits);
            }
            if (z_zero_fill(source, length, src_buf, 8) > 0) {
                source = src_buf;
                length = 8;
            }
            fcc_idx = 1; /* FCC 11 Standard Customer */
        } else {
            if (not_all_digits) {
                if ((i = z_not_sane(NEON_F, source, 8))) {
                    return z_errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 402,
                                    "Invalid character at position %d in DPID (digits only)", i);
                }
                if ((i = z_not_sane(AUS_GDSET_F, source + 8, length - 8))) {
                    return z_errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 404,
                                    "Invalid character at position %d in input (alphanumerics, space and \"#\" only)",
                                    i + 8);
                }
            }
            if (length > 18) {
                if (not_all_digits) {
                    return ZEXT z_errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 407,
                                    "Invalid character at position %1$d in input (digits only for Customer Barcode 3"
                                    " length %2$d)", not_all_digits, length);
                }
                fcc_idx = 3; /* FCC 62 Customer Barcode 3 all-digits */
            } else if (length > 16 || (length > 13 && not_all_digits)) {
                fcc_idx = 3; /* FCC 62 Customer Barcode 3 */
            } else {
                fcc_idx = 2; /* FCC 59 Customer Barcode 2 */
            }
        }

        /* Check if DPID all zeros (Null) */
        if (z_chr_cnt(source, 8, '0') == 8) {
            fcc_idx = 0; /* FCC 00 Null */
        }
    } else {
        if (length > 8) {
            return z_errtxtf(ZINT_ERROR_TOO_LONG, symbol, 403, "Input length %d too long (maximum 8)", length);
        }

        if ((i = z_not_sane(NEON_F, source, length))) {
            return z_errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 406,
                            "Invalid character at position %d in DPID (digits only)", i);
        }
        /* Add leading zeros as required */
        if (z_zero_fill(source, length, src_buf, 8) > 0) {
            source = src_buf;
            length = 8;
        }

        /* Format control code (FCC) */
        fcc_idx = symbol->symbology - BARCODE_AUSREPLY + 4; /* 4 (FCC 45), 5 (FCC 87) or 6 (FCC 92) */
    }

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("AUSPOST FCC: %.2s\n", fccs[fcc_idx]);
    }

    /* Start character */
    memcpy(d, start_stop, 2);
    d += 2;

    /* Encode the FCC */
    for (reader = 0; reader < 2; reader++, d += 2) {
        memcpy(d, AusNTable[fccs[fcc_idx][reader] - '0'], 2);
    }

    /* Delivery Point Identifier (DPID) */
    for (reader = 0; reader < 8; reader++, d += 2) {
        memcpy(d, AusNTable[source[reader] - '0'], 2);
    }

    /* Customer Information */
    if (length > 8) {
        if (not_all_digits) {
            for (reader = 8; reader < length; reader++, d += 3) {
                memcpy(d, AusCTable[z_posn(AusGDSET, source[reader])], 3);
            }
        } else {
            for (reader = 8; reader < length; reader++, d += 2) {
                memcpy(d, AusNTable[source[reader] - '0'], 2);
            }
        }
    }

    /* Filler bar(s) */
    h = (int) (d - dest);
    assert(h < 53);
    for (i = h; i != 23 && i != 38 && i != 53; i++) {
        *d++ = 3; /* Tracker */
    }
    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("Filler: %d\n", i - h);
    }

    /* Reed Solomon error correction */
    d = aus_rs_error(dest, d);

    /* Stop character */
    memcpy(d, start_stop, 2);
    d += 2;

    /* Turn the symbol into a bar pattern ready for plotting */
    h = (int) (d - dest);
    for (loopey = 0, writer = 0; loopey < h; loopey++, writer += 2) {
        if (dest[loopey] == 1 || dest[loopey] == 0) {
            z_set_module(symbol, 0, writer);
        }
        z_set_module(symbol, 1, writer);
        if (dest[loopey] == 2 || dest[loopey] == 0) {
            z_set_module(symbol, 2, writer);
        }
    }
    symbol->rows = 3; /* Not stackable */
    symbol->width = writer - 1;

    if (symbol->output_options & COMPLIANT_HEIGHT) {
        /* Australia Post Customer Barcoding Technical Specifications (Revised Aug 2012) Dimensions, placement and
           printing p.12
           (https://auspost.com.au/content/dam/auspost_corp/media/documents/
            customer-barcode-technical-specifications-aug2012.pdf)
           X 0.5mm (average of 0.4mm - 0.6mm), min height 4.2mm / 0.6mm (X max) = 7, max 5.6mm / 0.4mm (X min) = 14
           Tracker 1.3mm (average of 1mm - 1.6mm)
           Ascender/Descender 3.15mm (average of 2.6mm - 3.7mm) less T = 1.85mm
         */
        symbol->row_height[0] = 3.7f; /* 1.85f / 0.5f */
        symbol->row_height[1] = 2.6f; /* 1.3f / 0.5f */
        /* Note using max X for minimum and min X for maximum */
        error_number = zint_daft_set_height(symbol, 7.0f, 14.0f);
    } else {
        symbol->row_height[0] = 3.0f;
        symbol->row_height[1] = 2.0f;
        error_number = zint_daft_set_height(symbol, 0.0f, 0.0f);
    }

    if ((symbol->show_hrt & 0x7) >= ZINT_HRT_LINEAR_ALL) {
        z_hrt_cpy_cat_nochk(symbol, fccs[fcc_idx], 2, '\xFF' /*separator (none)*/, source, length);
    }

    if (content_segs && z_ct_cpy_cat(symbol, fccs[fcc_idx], 2, '\xFF' /*separator (none)*/, source, length)) {
        return ZINT_ERROR_MEMORY; /* `z_ct_cpy_cat()` only fails with OOM */
    }

    return error_number;
}

/* vim: set ts=4 sw=4 et : */
