/* dmatrix.c Handles Data Matrix ECC 200 symbols */

/*
    libzint - the open source barcode library
    Copyright (C) 2009 - 2021 Robin Stuart <rstuart114@gmail.com>

    developed from and including some functions from:
        IEC16022 bar code generation
        Adrian Kennard, Andrews & Arnold Ltd
        with help from Cliff Hones on the RS coding

        (c) 2004 Adrian Kennard, Andrews & Arnold Ltd
        (c) 2006 Stefan Schmidt <stefan@datenfreihafen.org>

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
#include <assert.h>
#include <math.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"
#include "reedsol.h"
#include "dmatrix.h"

/* Annex M placement algorithm low level */
static void ecc200placementbit(int *array, const int NR, const int NC, int r, int c, const int p, const char b) {
    if (r < 0) {
        r += NR;
        c += 4 - ((NR + 4) % 8);
    }
    if (c < 0) {
        c += NC;
        r += 4 - ((NC + 4) % 8);
    }
    // Necessary for 26x32,26x40,26x48,36x120,36x144,72x120,72x144
    if (r >= NR) {
#ifdef DEBUG
        fprintf(stderr, "r >= NR:%i,%i at r=%i->", p, b, r);
#endif
        r -= NR;
#ifdef DEBUG
        fprintf(stderr, "%i,c=%i\n", r, c);
#endif
    }
#ifdef DEBUG
    if (0 != array[r * NC + c]) {
        int a = array[r * NC + c];
        fprintf(stderr, "Double:%i,%i->%i,%i at r=%i,c=%i\n", a >> 3, a & 7, p, b, r, c);
        return;
    }
#endif
    // Check index limits
    assert(r < NR);
    assert(c < NC);
    // Check double-assignment
    assert(0 == array[r * NC + c]);
    array[r * NC + c] = (p << 3) + b;
}

static void ecc200placementblock(int *array, const int NR, const int NC, const int r,
        const int c, const int p) {
    ecc200placementbit(array, NR, NC, r - 2, c - 2, p, 7);
    ecc200placementbit(array, NR, NC, r - 2, c - 1, p, 6);
    ecc200placementbit(array, NR, NC, r - 1, c - 2, p, 5);
    ecc200placementbit(array, NR, NC, r - 1, c - 1, p, 4);
    ecc200placementbit(array, NR, NC, r - 1, c - 0, p, 3);
    ecc200placementbit(array, NR, NC, r - 0, c - 2, p, 2);
    ecc200placementbit(array, NR, NC, r - 0, c - 1, p, 1);
    ecc200placementbit(array, NR, NC, r - 0, c - 0, p, 0);
}

static void ecc200placementcornerA(int *array, const int NR, const int NC, const int p) {
    ecc200placementbit(array, NR, NC, NR - 1, 0, p, 7);
    ecc200placementbit(array, NR, NC, NR - 1, 1, p, 6);
    ecc200placementbit(array, NR, NC, NR - 1, 2, p, 5);
    ecc200placementbit(array, NR, NC, 0, NC - 2, p, 4);
    ecc200placementbit(array, NR, NC, 0, NC - 1, p, 3);
    ecc200placementbit(array, NR, NC, 1, NC - 1, p, 2);
    ecc200placementbit(array, NR, NC, 2, NC - 1, p, 1);
    ecc200placementbit(array, NR, NC, 3, NC - 1, p, 0);
}

static void ecc200placementcornerB(int *array, const int NR, const int NC, const int p) {
    ecc200placementbit(array, NR, NC, NR - 3, 0, p, 7);
    ecc200placementbit(array, NR, NC, NR - 2, 0, p, 6);
    ecc200placementbit(array, NR, NC, NR - 1, 0, p, 5);
    ecc200placementbit(array, NR, NC, 0, NC - 4, p, 4);
    ecc200placementbit(array, NR, NC, 0, NC - 3, p, 3);
    ecc200placementbit(array, NR, NC, 0, NC - 2, p, 2);
    ecc200placementbit(array, NR, NC, 0, NC - 1, p, 1);
    ecc200placementbit(array, NR, NC, 1, NC - 1, p, 0);
}

static void ecc200placementcornerC(int *array, const int NR, const int NC, const int p) {
    ecc200placementbit(array, NR, NC, NR - 3, 0, p, 7);
    ecc200placementbit(array, NR, NC, NR - 2, 0, p, 6);
    ecc200placementbit(array, NR, NC, NR - 1, 0, p, 5);
    ecc200placementbit(array, NR, NC, 0, NC - 2, p, 4);
    ecc200placementbit(array, NR, NC, 0, NC - 1, p, 3);
    ecc200placementbit(array, NR, NC, 1, NC - 1, p, 2);
    ecc200placementbit(array, NR, NC, 2, NC - 1, p, 1);
    ecc200placementbit(array, NR, NC, 3, NC - 1, p, 0);
}

static void ecc200placementcornerD(int *array, const int NR, const int NC, const int p) {
    ecc200placementbit(array, NR, NC, NR - 1, 0, p, 7);
    ecc200placementbit(array, NR, NC, NR - 1, NC - 1, p, 6);
    ecc200placementbit(array, NR, NC, 0, NC - 3, p, 5);
    ecc200placementbit(array, NR, NC, 0, NC - 2, p, 4);
    ecc200placementbit(array, NR, NC, 0, NC - 1, p, 3);
    ecc200placementbit(array, NR, NC, 1, NC - 3, p, 2);
    ecc200placementbit(array, NR, NC, 1, NC - 2, p, 1);
    ecc200placementbit(array, NR, NC, 1, NC - 1, p, 0);
}

/* Annex M placement alorithm main function */
static void ecc200placement(int *array, const int NR, const int NC) {
    int r, c, p;
    // invalidate
    for (r = 0; r < NR; r++)
        for (c = 0; c < NC; c++)
            array[r * NC + c] = 0;
    // start
    p = 1;
    r = 4;
    c = 0;
    do {
        // check corner
        if (r == NR && !c)
            ecc200placementcornerA(array, NR, NC, p++);
        if (r == NR - 2 && !c && NC % 4)
            ecc200placementcornerB(array, NR, NC, p++);
        if (r == NR - 2 && !c && (NC % 8) == 4)
            ecc200placementcornerC(array, NR, NC, p++);
        if (r == NR + 4 && c == 2 && !(NC % 8))
            ecc200placementcornerD(array, NR, NC, p++);
        // up/right
        do {
            if (r < NR && c >= 0 && !array[r * NC + c])
                ecc200placementblock(array, NR, NC, r, c, p++);
            r -= 2;
            c += 2;
        } while (r >= 0 && c < NC);
        r++;
        c += 3;
        // down/left
        do {
            if (r >= 0 && c < NC && !array[r * NC + c])
                ecc200placementblock(array, NR, NC, r, c, p++);
            r += 2;
            c -= 2;
        } while (r < NR && c >= 0);
        r += 3;
        c++;
    } while (r < NR || c < NC);
    // unfilled corner
    if (!array[NR * NC - 1])
        array[NR * NC - 1] = array[NR * NC - NC - 2] = 1;
}

/* calculate and append ecc code, and if necessary interleave */
static void ecc200(unsigned char *binary, const int bytes, const int datablock, const int rsblock, const int skew) {
    int blocks = (bytes + 2) / datablock, b;
    int rsblocks = rsblock * blocks;
    int n;
    rs_t rs;

    rs_init_gf(&rs, 0x12d);
    rs_init_code(&rs, rsblock, 1);
    for (b = 0; b < blocks; b++) {
        unsigned char buf[256], ecc[256];
        int p = 0;
        for (n = b; n < bytes; n += blocks)
            buf[p++] = binary[n];
        rs_encode(&rs, p, buf, ecc);
        p = rsblock - 1; // comes back reversed
        for (n = b; n < rsblocks; n += blocks) {
            if (skew) {
                /* Rotate ecc data to make 144x144 size symbols acceptable */
                /* See http://groups.google.com/group/postscriptbarcode/msg/5ae8fda7757477da */
                if (b < 8) {
                    binary[bytes + n + 2] = ecc[p--];
                } else {
                    binary[bytes + n - 8] = ecc[p--];
                }
            } else {
                binary[bytes + n] = ecc[p--];
            }
        }
    }
}

/* Is basic (non-shifted) C40? */
static int isc40(const unsigned char input) {
    if ((input >= '0' && input <= '9') || (input >= 'A' && input <= 'Z') || input == ' ') {
        return 1;
    }
    return 0;
}

/* Is basic (non-shifted) TEXT? */
static int istext(const unsigned char input) {
    if ((input >= '0' && input <= '9') || (input >= 'a' && input <= 'z') || input == ' ') {
        return 1;
    }
    return 0;
}

/* Is basic (non-shifted) C40/TEXT? */
static int isc40text(const int current_mode, const unsigned char input) {
    return current_mode == DM_C40 ? isc40(input) : istext(input);
}

/* Return true (1) if a character is valid in X12 set */
static int isX12(const unsigned char input) {

    if (isc40(input)) {
        return 1;
    }
    if (input == 13 || input == '*' || input == '>') {
        return 1;
    }

    return 0;
}

static int p_r_6_2_1(const unsigned char inputData[], const int position, const int sourcelen) {
    /* Annex P section (r)(6)(ii)(I)
       "If one of the three X12 terminator/separator characters first
        occurs in the yet to be processed data before a non-X12 character..."
     */
    int i;

    for (i = position; i < sourcelen && isX12(inputData[i]); i++) {
        if (inputData[i] == 13 || inputData[i] == '*' || inputData[i] == '>') {
            return 1;
        }
    }

    return 0;
}

/* Character counts are multiplied by this, so as to be whole integer divisible by 2, 3 and 4 */
#define DM_MULT             12

#define DM_MULT_1_DIV_2     6
#define DM_MULT_2_DIV_3     8
#define DM_MULT_3_DIV_4     9
#define DM_MULT_1           12
#define DM_MULT_5_DIV_4     15
#define DM_MULT_4_DIV_3     16
#define DM_MULT_2           24
#define DM_MULT_9_DIV_4     27
#define DM_MULT_8_DIV_3     32
#define DM_MULT_13_DIV_4    39
#define DM_MULT_10_DIV_3    40
#define DM_MULT_4           48
#define DM_MULT_17_DIV_4    51
#define DM_MULT_13_DIV_3    52

#define DM_MULT_MINUS_1     11
#define DM_MULT_CEIL(n)     ((((n) + DM_MULT_MINUS_1) / DM_MULT) * DM_MULT)

/* 'look ahead test' from Annex P */
static int look_ahead_test(const unsigned char inputData[], const int sourcelen, const int position,
            const int current_mode, const int gs1, const int debug) {
    int ascii_count, c40_count, text_count, x12_count, edf_count, b256_count;
    int ascii_rnded, c40_rnded, text_rnded, x12_rnded, edf_rnded, b256_rnded;
    int cnt_1;
    int sp;

    /* step (j) */
    if (current_mode == DM_ASCII) {
        ascii_count = 0;
        c40_count = DM_MULT_1;
        text_count = DM_MULT_1;
        x12_count = DM_MULT_1;
        edf_count = DM_MULT_1;
        b256_count = DM_MULT_5_DIV_4; // 1.25
    } else {
        ascii_count = DM_MULT_1;
        c40_count = DM_MULT_2;
        text_count = DM_MULT_2;
        x12_count = DM_MULT_2;
        edf_count = DM_MULT_2;
        b256_count = DM_MULT_9_DIV_4; // 2.25
    }

    switch (current_mode) {
        case DM_C40: c40_count = 0;
            break;
        case DM_TEXT: text_count = 0;
            break;
        case DM_X12: x12_count = 0;
            break;
        case DM_EDIFACT: edf_count = 0;
            break;
        case DM_BASE256: b256_count = 0;
            break;
    }

    for (sp = position; sp < sourcelen; sp++) {
        unsigned char c = inputData[sp];
        int is_extended = c & 0x80;

        /* ascii ... step (l) */
        if ((c >= '0') && (c <= '9')) {
            ascii_count += DM_MULT_1_DIV_2; // (l)(1)
        } else {
            if (is_extended) {
                ascii_count = DM_MULT_CEIL(ascii_count) + DM_MULT_2; // (l)(2)
            } else {
                ascii_count = DM_MULT_CEIL(ascii_count) + DM_MULT_1; // (l)(3)
            }
        }

        /* c40 ... step (m) */
        if (isc40(c)) {
            c40_count += DM_MULT_2_DIV_3; // (m)(1)
        } else {
            if (is_extended) {
                c40_count += DM_MULT_8_DIV_3; // (m)(2)
            } else {
                c40_count += DM_MULT_4_DIV_3; // (m)(3)
            }
        }

        /* text ... step (n) */
        if (istext(c)) {
            text_count += DM_MULT_2_DIV_3; // (n)(1)
        } else {
            if (is_extended) {
                text_count += DM_MULT_8_DIV_3; // (n)(2)
            } else {
                text_count += DM_MULT_4_DIV_3; // (n)(3)
            }
        }

        /* x12 ... step (o) */
        if (isX12(c)) {
            x12_count += DM_MULT_2_DIV_3; // (o)(1)
        } else {
            if (is_extended) {
                x12_count += DM_MULT_13_DIV_3; // (o)(2)
            } else {
                x12_count += DM_MULT_10_DIV_3; // (o)(3)
            }
        }

        /* edifact ... step (p) */
        if ((c >= ' ') && (c <= '^')) {
            edf_count += DM_MULT_3_DIV_4; // (p)(1)
        } else {
            if (is_extended) {
                edf_count += DM_MULT_17_DIV_4; // (p)(2)
            } else {
                edf_count += DM_MULT_13_DIV_4; // (p)(3)
            }
        }

        /* base 256 ... step (q) */
        if ((gs1 == 1) && (c == '[')) {
            /* FNC1 separator */
            b256_count += DM_MULT_4; // (q)(1)
        } else {
            b256_count += DM_MULT_1; // (q)(2)
        }

        if (sp >= position + 4) {
            /* At least 5 data characters processed ... step (r) */
            /* NOTE: different than spec, where it's at least 4. Following previous behaviour here (and BWIPP) */

            if (debug) {
                printf("\n(%d, %d, %d): ascii_count %d, b256_count %d, edf_count %d, text_count %d"
                        ", x12_count %d, c40_count %d ",
                        current_mode, position, sp, ascii_count, b256_count, edf_count, text_count,
                        x12_count, c40_count);
            }

            cnt_1 = ascii_count + DM_MULT_1;
            if (cnt_1 <= b256_count && cnt_1 <= edf_count && cnt_1 <= text_count && cnt_1 <= x12_count
                    && cnt_1 <= c40_count) {
                return DM_ASCII; /* step (r)(1) */
            }
            cnt_1 = b256_count + DM_MULT_1;
            if (cnt_1 <= ascii_count || (cnt_1 < edf_count && cnt_1 < text_count && cnt_1 < x12_count
                    && cnt_1 < c40_count)) {
                return DM_BASE256; /* step (r)(2) */
            }
            cnt_1 = edf_count + DM_MULT_1;
            if (cnt_1 < ascii_count && cnt_1 < b256_count && cnt_1 < text_count && cnt_1 < x12_count
                    && cnt_1 < c40_count) {
                return DM_EDIFACT; /* step (r)(3) */
            }
            cnt_1 = text_count + DM_MULT_1;
            if (cnt_1 < ascii_count && cnt_1 < b256_count && cnt_1 < edf_count && cnt_1 < x12_count
                    && cnt_1 < c40_count) {
                return DM_TEXT; /* step (r)(4) */
            }
            cnt_1 = x12_count + DM_MULT_1;
            if (cnt_1 < ascii_count && cnt_1 < b256_count && cnt_1 < edf_count && cnt_1 < text_count
                    && cnt_1 < c40_count) {
                return DM_X12; /* step (r)(5) */
            }
            cnt_1 = c40_count + DM_MULT_1;
            if (cnt_1 < ascii_count && cnt_1 < b256_count && cnt_1 < edf_count && cnt_1 < text_count) {
                if (c40_count < x12_count) {
                    return DM_C40; /* step (r)(6)(i) */
                }
                if (c40_count == x12_count) {
                    if (p_r_6_2_1(inputData, sp, sourcelen) == 1) {
                        return DM_X12; /* step (r)(6)(ii)(I) */
                    }
                    return DM_C40; /* step (r)(6)(ii)(II) */
                }
            }
        }
    }

    /* At the end of data ... step (k) */
    /* step (k)(1) */
    ascii_rnded = DM_MULT_CEIL(ascii_count);
    b256_rnded = DM_MULT_CEIL(b256_count);
    edf_rnded = DM_MULT_CEIL(edf_count);
    text_rnded = DM_MULT_CEIL(text_count);
    x12_rnded = DM_MULT_CEIL(x12_count);
    c40_rnded = DM_MULT_CEIL(c40_count);
    if (debug) {
        printf("\nEOD(%d, %d): ascii_rnded %d, b256_rnded %d, edf_rnded %d, text_rnded %d, x12_rnded %d (%d)"
                ", c40_rnded %d (%d) ",
                current_mode, position, ascii_rnded, b256_rnded, edf_rnded, text_rnded, x12_rnded, x12_count,
                c40_rnded, c40_count);
    }

    if (ascii_rnded <= b256_rnded && ascii_rnded <= edf_rnded && ascii_rnded <= text_rnded && ascii_rnded <= x12_rnded
            && ascii_rnded <= c40_rnded) {
        return DM_ASCII; /* step (k)(2) */
    }
    if (b256_rnded < ascii_rnded && b256_rnded < edf_rnded && b256_rnded < text_rnded && b256_rnded < x12_rnded
            && b256_rnded < c40_rnded) {
        return DM_BASE256; /* step (k)(3) */
    }
    if (edf_rnded < ascii_rnded && edf_rnded < b256_rnded && edf_rnded < text_rnded && edf_rnded < x12_rnded
            && edf_rnded < c40_rnded) {
        return DM_EDIFACT; /* step (k)(4) */
    }
    if (text_rnded < ascii_rnded && text_rnded < b256_rnded && text_rnded < edf_rnded && text_rnded < x12_rnded
            && text_rnded < c40_rnded) {
        return DM_TEXT; /* step (k)(5) */
    }
    if (x12_rnded < ascii_rnded && x12_rnded < b256_rnded && x12_rnded < edf_rnded && x12_rnded < text_rnded
            && x12_rnded < c40_rnded) {
        return DM_X12; /* step (k)(6) */
    }
    /* Note the algorithm is particularly sub-optimal here, returning C40 even if X12/EDIFACT (much) better, due to
       the < comparisons of rounded X12/EDIFACT values to each other above - comparisons would need to be <= or
       unrounded (cf. very similar Code One algorithm). Not changed to maintain compatibility with spec and BWIPP */
    return DM_C40; /* step (k)(7) */
}

/* Copy C40/TEXT/X12 triplets from buffer to target. Returns elements left in buffer (< 3) */
static int ctx_process_buffer_transfer(int process_buffer[8], int process_p, unsigned char target[], int *p_tp,
            int debug) {
    int i, process_e;
    int tp = *p_tp;

    process_e = (process_p / 3) * 3;

    for (i = 0; i < process_e; i += 3) {
        int iv = (1600 * process_buffer[i]) + (40 * process_buffer[i + 1]) + (process_buffer[i + 2]) + 1;
        target[tp++] = (unsigned char) (iv >> 8);
        target[tp++] = (unsigned char) (iv & 0xFF);
        if (debug) {
            printf("[%d %d %d (%d %d)] ", process_buffer[i], process_buffer[i + 1], process_buffer[i + 2],
                target[tp - 2], target[tp - 1]);
        }
    }

    process_p -= process_e;

    if (process_p) {
        memmove(process_buffer, process_buffer + process_e, sizeof(int) * process_p);
    }

    *p_tp = tp;

    return process_p;
}

/* Copy EDIFACT quadruplets from buffer to target. Returns elements left in buffer (< 4) */
static int edi_process_buffer_transfer(int process_buffer[8], int process_p, unsigned char target[], int *p_tp,
            int debug) {
    int i, process_e;
    int tp = *p_tp;

    process_e = (process_p / 4) * 4;

    for (i = 0; i < process_e; i += 4) {
        target[tp++] = (unsigned char) (process_buffer[i] << 2 | (process_buffer[i + 1] & 0x30) >> 4);
        target[tp++] = (unsigned char) ((process_buffer[i + 1] & 0x0f) << 4 | (process_buffer[i + 2] & 0x3c) >> 2);
        target[tp++] = (unsigned char) ((process_buffer[i + 2] & 0x03) << 6 | process_buffer[i + 3]);
        if (debug) {
            printf("[%d %d %d %d (%d %d %d)] ", process_buffer[i], process_buffer[i + 1], process_buffer[i + 2],
                process_buffer[i + 3], target[tp - 3], target[tp - 2], target[tp - 1]);
        }
    }

    process_p -= process_e;

    if (process_p) {
        memmove(process_buffer, process_buffer + process_e, sizeof(int) * process_p);
    }

    *p_tp = tp;

    return process_p;
}

/* Get symbol size, as specified or else smallest containing `minimum` codewords */
static int get_symbolsize(struct zint_symbol *symbol, const int minimum) {
    int i;

    if ((symbol->option_2 >= 1) && (symbol->option_2 <= DMSIZESCOUNT)) {
        return intsymbol[symbol->option_2 - 1];
    }
    for (i = DMSIZESCOUNT - 2; i >= 0; i--) {
        if (minimum > matrixbytes[i]) {
            if (symbol->option_3 == DM_DMRE) {
                return i + 1;
            }
            if (symbol->option_3 == DM_SQUARE) {
                /* Skip rectangular symbols in square only mode */
                while (i + 1 < DMSIZESCOUNT && matrixH[i + 1] != matrixW[i + 1]) {
                    i++;
                }
                return i + 1 < DMSIZESCOUNT ? i + 1 : 0;
            }
            /* Skip DMRE symbols in no dmre mode */
            while (i + 1 < DMSIZESCOUNT && isDMRE[i + 1]) {
                i++;
            }
            return i + 1 < DMSIZESCOUNT ? i + 1 : 0;
        }
    }
    return 0;
}

/* Number of codewords remaining in a particular version (may be negative) */
static int codewords_remaining(struct zint_symbol *symbol, const int tp, const int process_p) {
    int symbolsize = get_symbolsize(symbol, tp + process_p); /* Allow for the remaining data characters */

    return matrixbytes[symbolsize] - tp;
}

/* Number of C40/TEXT elements needed to encode `input` */
static int c40text_cnt(const int current_mode, const int gs1, unsigned char input) {
    int cnt;

    if (gs1 && input == '[') {
        return 2;
    }
    cnt = 1;
    if (input & 0x80) {
        cnt += 2;
        input = input - 128;
    }
    if ((current_mode == DM_C40 && c40_shift[input]) || (current_mode == DM_TEXT && text_shift[input])) {
        cnt += 1;
    }

    return cnt;
}

/* Update Base 256 field length */
static int update_b256_field_length(unsigned char target[], int tp, int b256_start) {
    int b256_count = tp - (b256_start + 1);
    if (b256_count <= 249) {
        target[b256_start] = b256_count;
    } else {
        /* Insert extra codeword */
        memmove(target + b256_start + 2, target + b256_start + 1, b256_count);
        target[b256_start] = (unsigned char) (249 + (b256_count / 250));
        target[b256_start + 1] = (unsigned char) (b256_count % 250);
        tp++;
    }

    return tp;
}

/* Encodes data using ASCII, C40, Text, X12, EDIFACT or Base 256 modes as appropriate
   Supports encoding FNC1 in supporting systems */
static int dm200encode(struct zint_symbol *symbol, const unsigned char source[], unsigned char target[],
        int *p_length, int *p_binlen) {

    int sp;
    int tp, i, gs1;
    int current_mode, next_mode;
    int inputlen = *p_length;
    int process_buffer[8]; /* holds remaining data to finalised */
    int process_p = 0; /* number of characters left to finalise */
    int b256_start = 0;
    int symbols_left;
    int debug = symbol->debug & ZINT_DEBUG_PRINT;

    sp = 0;
    tp = 0;

    /* step (a) */
    current_mode = DM_ASCII;
    next_mode = DM_ASCII;

    /* gs1 flag values: 0: no gs1, 1: gs1 with FNC1 serparator, 2: GS separator */
    if ((symbol->input_mode & 0x07) == GS1_MODE) {
        if (symbol->output_options & GS1_GS_SEPARATOR) {
            gs1 = 2;
        } else {
            gs1 = 1;
        }
    } else {
        gs1 = 0;
    }

    if (gs1) {
        target[tp] = 232;
        tp++;
        if (debug) printf("FN1 ");
    } /* FNC1 */

    if (symbol->output_options & READER_INIT) {
        if (gs1) {
            strcpy(symbol->errtxt, "521: Cannot encode in GS1 mode and Reader Initialisation at the same time");
            return ZINT_ERROR_INVALID_OPTION;
        } else {
            target[tp] = 234;
            tp++; /* Reader Programming */
            if (debug) printf("RP ");
        }
    }

    if (symbol->eci > 0) {
        /* Encode ECI numbers according to Table 6 */
        target[tp] = 241; /* ECI Character */
        tp++;
        if (symbol->eci <= 126) {
            target[tp] = (unsigned char) (symbol->eci + 1);
            tp++;
        } else if (symbol->eci <= 16382) {
            target[tp] = (unsigned char) ((symbol->eci - 127) / 254 + 128);
            tp++;
            target[tp] = (unsigned char) ((symbol->eci - 127) % 254 + 1);
            tp++;
        } else {
            target[tp] = (unsigned char) ((symbol->eci - 16383) / 64516 + 192);
            tp++;
            target[tp] = (unsigned char) (((symbol->eci - 16383) / 254) % 254 + 1);
            tp++;
            target[tp] = (unsigned char) ((symbol->eci - 16383) % 254 + 1);
            tp++;
        }
        if (debug) printf("ECI %d ", symbol->eci + 1);
    }

    /* Check for Macro05/Macro06 */
    /* "[)>[RS]05[GS]...[RS][EOT]" -> CW 236 */
    /* "[)>[RS]06[GS]...[RS][EOT]" -> CW 237 */
    if (tp == 0 && sp == 0 && inputlen >= 9
            && source[0] == '[' && source[1] == ')' && source[2] == '>'
            && source[3] == '\x1e' && source[4] == '0'
            && (source[5] == '5' || source[5] == '6')
            && source[6] == '\x1d'
            && source[inputlen - 2] == '\x1e' && source[inputlen - 1] == '\x04') {
        /* Output macro Codeword */
        if (source[5] == '5') {
            target[tp] = 236;
            if (debug) printf("Macro05 ");
        } else {
            target[tp] = 237;
            if (debug) printf("Macro06 ");
        }
        tp++;
        /* Remove macro characters from input string */
        sp = 7;
        inputlen -= 2;
        *p_length -= 2;
    }

    while (sp < inputlen) {

        current_mode = next_mode;

        /* step (b) - ASCII encodation */
        if (current_mode == DM_ASCII) {
            next_mode = DM_ASCII;

            if (istwodigits(source, inputlen, sp)) {
                target[tp] = (unsigned char) ((10 * ctoi(source[sp])) + ctoi(source[sp + 1]) + 130);
                if (debug) printf("N%02d ", target[tp] - 130);
                tp++;
                sp += 2;
            } else {
                next_mode = look_ahead_test(source, inputlen, sp, current_mode, gs1, debug);

                if (next_mode != DM_ASCII) {
                    switch (next_mode) {
                        case DM_C40: target[tp] = 230;
                            tp++;
                            if (debug) printf("C40 ");
                            break;
                        case DM_TEXT: target[tp] = 239;
                            tp++;
                            if (debug) printf("TEX ");
                            break;
                        case DM_X12: target[tp] = 238;
                            tp++;
                            if (debug) printf("X12 ");
                            break;
                        case DM_EDIFACT: target[tp] = 240;
                            tp++;
                            if (debug) printf("EDI ");
                            break;
                        case DM_BASE256: target[tp] = 231;
                            tp++;
                            b256_start = tp;
                            target[tp++] = 0; /* Byte count holder (may be expanded to 2 codewords) */
                            if (debug) printf("BAS ");
                            break;
                    }
                } else {
                    if (source[sp] & 0x80) {
                        target[tp] = 235; /* FNC4 */
                        tp++;
                        target[tp] = (source[sp] - 128) + 1;
                        tp++;
                        if (debug) printf("FN4 A%02X ", target[tp - 1] - 1);
                    } else {
                        if (gs1 && (source[sp] == '[')) {
                            if (gs1 == 2) {
                                target[tp] = 29 + 1; /* GS */
                                if (debug) printf("GS ");
                            } else {
                                target[tp] = 232; /* FNC1 */
                                if (debug) printf("FN1 ");
                            }
                        } else {
                            target[tp] = source[sp] + 1;
                            if (debug) printf("A%02X ", target[tp] - 1);
                        }
                        tp++;
                    }
                    sp++;
                }
            }

        /* step (c)/(d) C40/TEXT encodation */
        } else if (current_mode == DM_C40 || current_mode == DM_TEXT) {

            next_mode = current_mode;
            if (process_p == 0) {
                next_mode = look_ahead_test(source, inputlen, sp, current_mode, gs1, debug);
            }

            if (next_mode != current_mode) {
                target[tp] = 254; /* Unlatch */
                tp++;
                next_mode = DM_ASCII;
                if (debug) printf("ASC ");
            } else {
                int shift_set, value;
                const char *ct_shift, *ct_value;

                if (current_mode == DM_C40) {
                    ct_shift = c40_shift;
                    ct_value = c40_value;
                } else {
                    ct_shift = text_shift;
                    ct_value = text_value;
                }

                if (source[sp] & 0x80) {
                    process_buffer[process_p++] = 1;
                    process_buffer[process_p++] = 30; /* Upper Shift */
                    shift_set = ct_shift[source[sp] - 128];
                    value = ct_value[source[sp] - 128];
                } else {
                    if (gs1 && (source[sp] == '[')) {
                        if (gs1 == 2) {
                            shift_set = ct_shift[29];
                            value = ct_value[29]; /* GS */
                        } else {
                            shift_set = 2;
                            value = 27; /* FNC1 */
                        }
                    } else {
                        shift_set = ct_shift[source[sp]];
                        value = ct_value[source[sp]];
                    }
                }

                if (shift_set != 0) {
                    process_buffer[process_p++] = shift_set - 1;
                }
                process_buffer[process_p++] = value;

                if (process_p >= 3) {
                    process_p = ctx_process_buffer_transfer(process_buffer, process_p, target, &tp, debug);
                }
                sp++;
            }

        /* step (e) X12 encodation */
        } else if (current_mode == DM_X12) {

            next_mode = DM_X12;
            if (process_p == 0) {
                next_mode = look_ahead_test(source, inputlen, sp, current_mode, gs1, debug);
            }

            if (next_mode != DM_X12) {
                target[tp] = 254; /* Unlatch */
                tp++;
                next_mode = DM_ASCII;
                if (debug) printf("ASC ");
            } else {
                static const char x12_nonalphanum_chars[] = "\015*> ";
                int value = 0;

                if ((source[sp] >= '0') && (source[sp] <= '9')) {
                    value = (source[sp] - '0') + 4;
                } else if ((source[sp] >= 'A') && (source[sp] <= 'Z')) {
                    value = (source[sp] - 'A') + 14;
                } else {
                    value = posn(x12_nonalphanum_chars, source[sp]);
                }

                process_buffer[process_p++] = value;

                if (process_p >= 3) {
                    process_p = ctx_process_buffer_transfer(process_buffer, process_p, target, &tp, debug);
                }
                sp++;
            }

        /* step (f) EDIFACT encodation */
        } else if (current_mode == DM_EDIFACT) {

            next_mode = DM_EDIFACT;
            if (process_p == 3) {
                /* Note different then spec Step (f)(1), which suggests checking when 0, but this seems to work
                   better in many cases. */
                next_mode = look_ahead_test(source, inputlen, sp, current_mode, gs1, debug);
            }

            if (next_mode != DM_EDIFACT) {
                process_buffer[process_p++] = 31;
                next_mode = DM_ASCII;
            } else {
                int value = source[sp];

                if (value >= 64) { // '@'
                    value -= 64;
                }

                process_buffer[process_p++] = value;
                sp++;
            }

            if (process_p >= 4) {
                process_p = edi_process_buffer_transfer(process_buffer, process_p, target, &tp, debug);
            }
            if (debug && next_mode == DM_ASCII) printf("ASC ");

        /* step (g) Base 256 encodation */
        } else if (current_mode == DM_BASE256) {
            next_mode = look_ahead_test(source, inputlen, sp, current_mode, gs1, debug);

            if (next_mode == DM_BASE256) {
                target[tp] = source[sp];
                tp++;
                sp++;
                if (debug) printf("B%02X ", target[tp - 1]);
            } else {
                tp = update_b256_field_length(target, tp, b256_start);
                /* B.2.1 255-state randomising algorithm */
                for (i = b256_start; i < tp; i++) {
                    int prn = ((149 * (i + 1)) % 255) + 1;
                    target[i] = (unsigned char) ((target[i] + prn) & 0xFF);
                }
                next_mode = DM_ASCII;
                if (debug) printf("ASC ");
            }
        }

        if (tp > 1558) {
            strcpy(symbol->errtxt, "520: Data too long to fit in symbol");
            return ZINT_ERROR_TOO_LONG;
        }

    } /* while */

    symbols_left = codewords_remaining(symbol, tp, process_p);

    if (debug) printf("\nsymbols_left %d, process_p %d ", symbols_left, process_p);

    if (current_mode == DM_C40 || current_mode == DM_TEXT) {
        /* NOTE: changed to follow spec exactly here, only using Shift 1 padded triplets when 2 symbol chars remain.
           This matches the behaviour of BWIPP but not tec-it, nor figures 4.15.1-1 and 4.15-1-2 in GS1 General
           Specifications 21.0.1.
         */
        if (debug) printf("%s ", current_mode == DM_C40 ? "C40" : "TEX");
        if (process_p == 0) {
            if (symbols_left > 0) {
                target[tp++] = 254; // Unlatch
                if (debug) printf("ASC ");
            }
        } else {
            if (process_p == 2 && symbols_left == 2) {
                /* 5.2.5.2 (b) */
                process_buffer[process_p++] = 0; // Shift 1
                (void) ctx_process_buffer_transfer(process_buffer, process_p, target, &tp, debug);

            } else if (process_p == 1 && symbols_left <= 2 && isc40text(current_mode, source[inputlen - 1])) {
                /* 5.2.5.2 (c)/(d) */
                if (symbols_left > 1) {
                    /* 5.2.5.2 (c) */
                    target[tp++] = 254; // Unlatch and encode remaining data in ascii.
                    if (debug) printf("ASC ");
                }
                target[tp++] = source[inputlen - 1] + 1;
                if (debug) printf("A%02X ", target[tp - 1] - 1);

            } else {
                int cnt, total_cnt = 0;
                /* Backtrack to last complete triplet (same technique as BWIPP) */
                while (sp > 0 && process_p % 3) {
                    sp--;
                    cnt = c40text_cnt(current_mode, gs1, source[sp]);
                    total_cnt += cnt;
                    process_p -= cnt;
                }
                tp -= (total_cnt / 3) * 2;

                target[tp++] = 254; // Unlatch
                if (debug) printf("ASC ");
                for (; sp < inputlen; sp++) {
                    if (istwodigits(source, inputlen, sp)) {
                        target[tp++] = (unsigned char) ((10 * ctoi(source[sp])) + ctoi(source[sp + 1]) + 130);
                        if (debug) printf("N%02d ", target[tp - 1] - 130);
                        sp++;
                    } else if (source[sp] & 0x80) {
                        target[tp++] = 235; /* FNC4 */
                        target[tp++] = (source[sp] - 128) + 1;
                        if (debug) printf("FN4 A%02X ", target[tp - 1] - 1);
                    } else if (gs1 && source[sp] == '[') {
                        if (gs1 == 2) {
                            target[tp] = 29 + 1; /* GS */
                            if (debug) printf("GS ");
                        } else {
                            target[tp] = 232; /* FNC1 */
                            if (debug) printf("FN1 ");
                        }
                    } else {
                        target[tp++] = source[sp] + 1;
                        if (debug) printf("A%02X ", target[tp - 1] - 1);
                    }
                }
            }
        }

    } else if (current_mode == DM_X12) {
        if (debug) printf("X12 ");
        if ((symbols_left == 1) && (process_p == 1)) {
            // Unlatch not required!
            target[tp++] = source[inputlen - 1] + 1;
            if (debug) printf("A%02X ", target[tp - 1] - 1);
        } else {
            if (symbols_left > 0) {
                target[tp++] = (254); // Unlatch.
                if (debug) printf("ASC ");
            }

            if (process_p == 1) {
                target[tp++] = source[inputlen - 1] + 1;
                if (debug) printf("A%02X ", target[tp - 1] - 1);
            } else if (process_p == 2) {
                target[tp++] = source[inputlen - 2] + 1;
                target[tp++] = source[inputlen - 1] + 1;
                if (debug) printf("A%02X A%02X ", target[tp - 2] - 1, target[tp - 1] - 1);
            }
        }

    } else if (current_mode == DM_EDIFACT) {
        if (debug) printf("EDI ");
        if (symbols_left <= 2 && process_p <= symbols_left) { // Unlatch not required!
            if (process_p == 1) {
                target[tp++] = source[inputlen - 1] + 1;
                if (debug) printf("A%02X ", target[tp - 1] - 1);
            } else if (process_p == 2) {
                target[tp++] = source[inputlen - 2] + 1;
                target[tp++] = source[inputlen - 1] + 1;
                if (debug) printf("A%02X A%02X ", target[tp - 2] - 1, target[tp - 1] - 1);
            }
        } else {
            // Append edifact unlatch value (31) and empty buffer
            if (process_p <= 3) {
                process_buffer[process_p++] = 31;
                if (process_p < 4) {
                    memset(process_buffer + process_p, 0, sizeof(int) * (4 - process_p));
                }
            }
            (void) edi_process_buffer_transfer(process_buffer, 4, target, &tp, debug);
        }

    } else if (current_mode == DM_BASE256) {
        if (symbols_left > 0) {
            tp = update_b256_field_length(target, tp, b256_start);
        }
        /* B.2.1 255-state randomising algorithm */
        for (i = b256_start; i < tp; i++) {
            int prn = ((149 * (i + 1)) % 255) + 1;
            target[i] = (unsigned char) ((target[i] + prn) & 0xFF);
        }
    }

    if (debug) {
        printf("\nData (%d): ", tp);
        for (i = 0; i < tp; i++)
            printf("%d ", target[i]);

        printf("\n");
    }

    *p_binlen = tp;

    return 0;
}

/* add pad bits */
static void add_tail(unsigned char target[], int tp, const int tail_length) {
    int i, prn, temp;

    for (i = tail_length; i > 0; i--) {
        if (i == tail_length) {
            target[tp] = 129;
            tp++; /* Pad */
        } else {
            /* B.1.1 253-state randomising algorithm */
            prn = ((149 * (tp + 1)) % 253) + 1;
            temp = 129 + prn;
            if (temp <= 254) {
                target[tp] = (unsigned char) (temp);
                tp++;
            } else {
                target[tp] = (unsigned char) (temp - 254);
                tp++;
            }
        }
    }
}

static int data_matrix_200(struct zint_symbol *symbol, const unsigned char source[], int inputlen) {
    int i, skew = 0;
    unsigned char binary[2200];
    int binlen;
    int symbolsize;
    int taillength, error_number = 0;
    int H, W, FH, FW, datablock, bytes, rsblock;
    int debug = symbol->debug & ZINT_DEBUG_PRINT;

    /* inputlen may be decremented by 2 if macro character is used */
    error_number = dm200encode(symbol, source, binary, &inputlen, &binlen);
    if (error_number != 0) {
        return error_number;
    }

    symbolsize = get_symbolsize(symbol, binlen);

    if (binlen > matrixbytes[symbolsize]) {
        if ((symbol->option_2 >= 1) && (symbol->option_2 <= DMSIZESCOUNT)) {
            // The symbol size was given by --ver (option_2)
            strcpy(symbol->errtxt, "522: Input too long for selected symbol size");
        } else {
            strcpy(symbol->errtxt, "523: Data too long to fit in symbol");
        }
        return ZINT_ERROR_TOO_LONG;
    }

    H = matrixH[symbolsize];
    W = matrixW[symbolsize];
    FH = matrixFH[symbolsize];
    FW = matrixFW[symbolsize];
    bytes = matrixbytes[symbolsize];
    datablock = matrixdatablock[symbolsize];
    rsblock = matrixrsblock[symbolsize];

    taillength = bytes - binlen;

    if (taillength != 0) {
        add_tail(binary, binlen, taillength);
    }
    if (debug) {
        printf("Pads (%d): ", taillength);
        for (i = binlen; i < binlen + taillength; i++) printf("%d ", binary[i]);
        printf("\n");
    }

    // ecc code
    if (symbolsize == INTSYMBOL144) {
        skew = 1;
    }
    ecc200(binary, bytes, datablock, rsblock, skew);
    if (debug) {
        printf("ECC (%d): ", rsblock * (bytes / datablock));
        for (i = bytes; i < bytes + rsblock * (bytes / datablock); i++) printf("%d ", binary[i]);
        printf("\n");
    }

#ifdef ZINT_TEST
    if (symbol->debug & ZINT_DEBUG_TEST) {
        debug_test_codeword_dump(symbol, binary, skew ? 1558 + 620 : bytes + rsblock * (bytes / datablock));
    }
#endif
    { // placement
        int x, y, NC, NR, *places;
        unsigned char *grid;
        NC = W - 2 * (W / FW);
        NR = H - 2 * (H / FH);
        places = (int *) malloc(sizeof(int) * NC * NR);
        ecc200placement(places, NR, NC);
        grid = (unsigned char *) malloc((size_t) W * H);
        memset(grid, 0, W * H);
        for (y = 0; y < H; y += FH) {
            for (x = 0; x < W; x++)
                grid[y * W + x] = 1;
            for (x = 0; x < W; x += 2)
                grid[(y + FH - 1) * W + x] = 1;
        }
        for (x = 0; x < W; x += FW) {
            for (y = 0; y < H; y++)
                grid[y * W + x] = 1;
            for (y = 0; y < H; y += 2)
                grid[y * W + x + FW - 1] = 1;
        }
#ifdef DEBUG
        // Print position matrix as in standard
        for (y = NR - 1; y >= 0; y--) {
            for (x = 0; x < NC; x++) {
                int v;
                if (x != 0)
                    fprintf(stderr, "|");
                v = places[(NR - y - 1) * NC + x];
                fprintf(stderr, "%3d.%2d", (v >> 3), 8 - (v & 7));
            }
            fprintf(stderr, "\n");
        }
#endif
        for (y = 0; y < NR; y++) {
            for (x = 0; x < NC; x++) {
                int v = places[(NR - y - 1) * NC + x];
                if (v == 1 || (v > 7 && (binary[(v >> 3) - 1] & (1 << (v & 7)))))
                    grid[(1 + y + 2 * (y / (FH - 2))) * W + 1 + x + 2 * (x / (FW - 2))] = 1;
            }
        }
        for (y = H - 1; y >= 0; y--) {
            for (x = 0; x < W; x++) {
                if (grid[W * y + x]) {
                    set_module(symbol, (H - y) - 1, x);
                }
            }
            symbol->row_height[(H - y) - 1] = 1;
        }
        free(grid);
        free(places);
    }

    symbol->height = H;
    symbol->rows = H;
    symbol->width = W;

    return error_number;
}

INTERNAL int dmatrix(struct zint_symbol *symbol, unsigned char source[], int length) {
    int error_number;

    if (symbol->option_1 <= 1) {
        /* ECC 200 */
        error_number = data_matrix_200(symbol, source, length);
    } else {
        /* ECC 000 - 140 */
        strcpy(symbol->errtxt, "524: Older Data Matrix standards are no longer supported");
        error_number = ZINT_ERROR_INVALID_OPTION;
    }

    return error_number;
}
