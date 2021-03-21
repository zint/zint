/*  eci.c - Extended Channel Interpretations

    libzint - the open source barcode library
    Copyright (C) 2009 - 2021 Robin Stuart <rstuart114@gmail.com>

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

#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"
#include "eci.h"
#include "eci_sb.h"
#include "sjis.h"
#include "big5.h"
#include "gb2312.h"
#include "ksx1001.h"

/* ECI 20 Shift JIS */
static int sjis_wctomb(unsigned char *r, const unsigned int wc) {
    int ret;
    unsigned int c;

    ret = sjis_wctomb_zint(&c, wc);
    if (ret == 0) {
        return 0;
    }
    if (ret == 2) {
        r[0] = (unsigned char) (c >> 8);
        r[1] = (unsigned char) (c & 0xff);
    } else {
        *r = (unsigned char) c;
    }
    return ret;
}

/* ECI 27 ASCII (ISO/IEC 646:1991 IRV (US)) */
static int ascii_wctosb(unsigned char *r, const unsigned int wc) {
    if (wc < 0x80) {
        *r = (unsigned char) wc;
        return 1;
    }
    return 0;
}

/* ECI 170 ASCII subset (ISO/IEC 646:1991 Invariant, excludes chars that historically had national variants) */
static int ascii_invariant_wctosb(unsigned char *r, const unsigned int wc) {
    if (wc == 0x7f || (wc <= 'z' && wc != '#' && wc != '$' && wc != '@' && (wc <= 'Z' || wc == '_' || wc >= 'a'))) {
        *r = (unsigned char) wc;
        return 1;
    }
    return 0;
}

/* ECI 28 Big5 Chinese (Taiwan) */
static int big5_wctomb(unsigned char *r, const unsigned int wc) {
    unsigned int c;

    if (wc < 0x80) {
        *r = (unsigned char) wc;
        return 1;
    }
    if (big5_wctomb_zint(&c, wc)) {
        r[0] = (unsigned char) (c >> 8);
        r[1] = (unsigned char) (c & 0xff);
        return 2;
    }
    return 0;
}

/* ECI 29 GB 2312 Chinese (PRC) */
static int gb2312_wctomb(unsigned char *r, const unsigned int wc) {
    unsigned int c;

    if (wc < 0x80) {
        *r = (unsigned char) wc;
        return 1;
    }
    if (gb2312_wctomb_zint(&c, wc)) {
        r[0] = (unsigned char) (c >> 8);
        r[1] = (unsigned char) (c & 0xff);
        return 2;
    }
    return 0;
}

/* ECI 30 EUC-KR (KS X 1001, formerly KS C 5601) Korean
 * See euc_kr_wctomb() in libiconv-1.16/lib/euc_kr.h */
static int euc_kr_wctomb(unsigned char *r, const unsigned int wc) {
    unsigned int c;

    if (wc < 0x80) {
        *r = (unsigned char) wc;
        return 1;
    }
    if (ksx1001_wctomb_zint(&c, wc)) {
        r[0] = (unsigned char) ((c >> 8) + 0x80);
        r[1] = (unsigned char) ((c & 0xff) + 0x80);
        return 2;
    }
    return 0;
}

/* Helper to count the number of chars in a string within a range */
static int chr_range_cnt(const unsigned char string[], const int length, const unsigned char c1,
            const unsigned char c2) {
    int count = 0;
    int i;
    if (c1) {
        for (i = 0; i < length; i++) {
            if (string[i] >= c1 && string[i] <= c2) {
                count++;
            }
        }
    } else {
        for (i = 0; i < length; i++) {
            if (string[i] <= c2) {
                count++;
            }
        }
    }
    return count;
}

/* Is ECI convertible from UTF-8? */
INTERNAL int is_eci_convertible(const int eci) {
    if (eci == 26 || (eci > 30 && eci != 170)) { /* Exclude ECI 170 - ASCII Invariant */
        /* UTF-8 (26) or 8-bit binary data (899) or undefined (> 30 and < 899) or not character set (> 899) */
        return 0;
    }
    return 1;
}

/* Calculate length required to convert UTF-8 to (double-byte) encoding */
INTERNAL int get_eci_length(const int eci, const unsigned char source[], int length) {
    if (eci == 20) { /* Shift JIS */
        /* Only ASCII backslash (reverse solidus) exceeds UTF-8 length */
        length += chr_cnt(source, length, '\\');

    } else if (eci == 25) { /* UCS-2BE */
        /* All ASCII chars take 2 bytes */
        length += chr_range_cnt(source, length, 0, 0x7F);

    } else if (eci == 29) { /* GB 2312 (and GB 18030 if Han Xin) */
        /* Not needed for GB 2312 but allow for GB 18030 4 byters */
        length *= 2;
    }

    /* Big5 and EUC-KR fit in UTF-8 length */

    return length;
}

/* Convert UTF-8 Unicode to other character encodings */
INTERNAL int utf8_to_eci(const int eci, const unsigned char source[], unsigned char dest[], int *p_length) {

    typedef int (*eci_func_t)(unsigned char *r, const unsigned int wc);
    static const eci_func_t eci_funcs[31] = {
                     NULL,              NULL,              NULL,              NULL,  iso8859_2_wctosb,
         iso8859_3_wctosb,  iso8859_4_wctosb,  iso8859_5_wctosb,  iso8859_6_wctosb,  iso8859_7_wctosb,
         iso8859_8_wctosb,  iso8859_9_wctosb, iso8859_10_wctosb, iso8859_11_wctosb,              NULL,
        iso8859_13_wctosb, iso8859_14_wctosb, iso8859_15_wctosb, iso8859_16_wctosb,              NULL,
              sjis_wctomb,     cp1250_wctosb,     cp1251_wctosb,     cp1252_wctosb,     cp1256_wctosb,
            ucs2be_wctomb,              NULL,      ascii_wctosb,       big5_wctomb,     gb2312_wctomb,
            euc_kr_wctomb,
    };
    eci_func_t eci_func;
    unsigned int codepoint, state;
    int in_posn;
    int out_posn;
    int length = *p_length;

    in_posn = 0;
    out_posn = 0;

    /* Special case ISO/IEC 8859-1 */
    if (eci == 0 || eci == 3) { /* Default ECI 0 to ISO/IEC 8859-1 */
        state = 0;
        while (in_posn < length) {
            do {
                decode_utf8(&state, &codepoint, source[in_posn++]);
            } while (in_posn < length && state != 0 && state != 12);
            if (state != 0) {
                return ZINT_ERROR_INVALID_DATA;
            }
            if (codepoint >= 0x80 && (codepoint < 0x00a0 || codepoint >= 0x0100)) {
                return ZINT_ERROR_INVALID_DATA;
            }
            dest[out_posn++] = (unsigned char) codepoint;
        }
        dest[out_posn] = '\0';
        *p_length = out_posn;
        return 0;
    }

    if (eci == 170) { /* ASCII Invariant (archaic subset) */
        eci_func = ascii_invariant_wctosb;
    } else {
        eci_func = eci_funcs[eci];
        if (eci_func == NULL) {
            return ZINT_ERROR_INVALID_DATA;
        }
    }

    state = 0;
    while (in_posn < length) {
        int incr;
        do {
            decode_utf8(&state, &codepoint, source[in_posn++]);
        } while (in_posn < length && state != 0 && state != 12);
        if (state != 0) {
            return ZINT_ERROR_INVALID_DATA;
        }
        incr = (*eci_func)(dest + out_posn, codepoint);
        if (incr == 0) {
            return ZINT_ERROR_INVALID_DATA;
        }
        out_posn += incr;
    }
    dest[out_posn] = '\0';
    *p_length = out_posn;

    return 0;
}

/* Find the lowest single-byte ECI mode which will encode a given set of Unicode text */
INTERNAL int get_best_eci(const unsigned char source[], int length) {
    int eci = 3;

    /* Note: attempting single-byte conversions only, so get_eci_length() unnecessary */
#ifndef _MSC_VER
    unsigned char local_source[length + 1];
#else
    unsigned char *local_source = (unsigned char *) _alloca(length + 1);
#endif

    do {
        if (eci == 14) { /* Reserved */
            eci = 15;
        } else if (eci == 19) { /* Reserved */
            eci = 21; /* Skip 20 Shift JIS */
        }
        if (utf8_to_eci(eci, source, local_source, &length) == 0) {
            return eci;
        }
        eci++;
    } while (eci < 25);

    if (!is_valid_utf8(source, length)) {
        return 0;
    }

    return 26; // If all of these fail, use Unicode!
}
