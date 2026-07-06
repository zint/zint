/*  gridmtx.c - Grid Matrix */
/*
    libzint - the open source barcode library
    Copyright (C) 2009-2026 Robin Stuart <rstuart114@gmail.com>

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

/* This file implements Grid Matrix as specified in
   AIM Global Document Number AIMD014 Rev. 1.63 Revised 9 Dec 2008 */

#include <assert.h>
#include <stdio.h>
#include "common.h"
#include "reedsol.h"
#include "gridmtx.h"
#include "eci.h"

/* `gm_define_modes()` stuff */

/* Bits multiplied by this for costs, so as to be whole integer divisible by 2 and 3 */
#define GM_MULT 6

/* Non-digit numeral set, excluding EOL (carriage return/linefeed) */
static const char gm_numeral_nondigits[] = " +-.,";

/* Whether in numeral or not. If in numeral, *p_numeral_end is set to position after numeral,
 * and *p_numeral_cost is set to per-numeral cost */
static int gm_in_numeral(const unsigned int ddata[], const int length, const int in_posn,
            unsigned int *p_numeral_end, unsigned int *p_numeral_cost) {
    int i, digit_cnt, nondigit, nondigit_posn;

    if (in_posn < (int) *p_numeral_end) {
        return 1;
    }

    /* Attempt to calculate the average 'cost' of using numeral mode in number of bits (times GM_MULT) */
    /* Also ensures that numeral mode is not selected when it cannot be used: for example in
       a string which has "2.2.0" (cannot have more than one non-numeric character for each
       block of three numeric characters) */
    for (i = in_posn, digit_cnt = 0, nondigit = 0, nondigit_posn = 0; i < length && i < in_posn + 4 && digit_cnt < 3;
            i++) {
        if (z_isdigit(ddata[i])) {
            digit_cnt++;
        } else if (z_posn(gm_numeral_nondigits, (const char) ddata[i]) != -1) {
            if (nondigit) {
                *p_numeral_end = 0;
                return 0;
            }
            nondigit = 1;
            nondigit_posn = i;
        } else if (i + 1 < length && ddata[i] == 13 && ddata[i + 1] == 10) {
            if (nondigit) {
                *p_numeral_end = 0;
                return 0;
            }
            i++;
            nondigit = 2;
            nondigit_posn = i;
        } else {
            break;
        }
    }
    if (digit_cnt == 0) { /* Must have at least one digit */
        *p_numeral_end = 0;
        return 0;
    }
    if (nondigit && nondigit_posn == i - 1) { /* Non-digit can't be at end */
        nondigit = 0;
    }
    *p_numeral_end = in_posn + digit_cnt + nondigit;
    /* Calculate per-numeral cost where 120 == (10 + 10) * GM_MULT, 60 == 10 * GM_MULT */
    if (digit_cnt == 3) {
        *p_numeral_cost = nondigit == 2 ? 24 /* (120 / 5) */ : nondigit == 1 ? 30 /* (120 / 4) */ : 20 /* (60 / 3) */;
    } else if (digit_cnt == 2) {
        *p_numeral_cost = nondigit == 2 ? 30 /* (120 / 4) */ : nondigit == 1 ? 40 /* (120 / 3) */ : 30 /* (60 / 2) */;
    } else {
        *p_numeral_cost = nondigit == 2 ? 40 /* (120 / 3) */ : nondigit == 1 ? 60 /* (120 / 2) */ : 60 /* (60 / 1) */;
    }
    return 1;
}

/* Encoding modes */
#define GM_CHINESE  1
#define GM_NUMERAL  2
#define GM_LOWER    3
#define GM_UPPER    4
#define GM_MIXED    5
#define GM_BYTE     6
#define GM_EOD      7 /* Pseudo-mode, used to put end of data */

/* Indexes into mode_types array */
#define GM_H   0 /* Chinese (Hanzi) */
#define GM_N   1 /* Numeral */
#define GM_L   2 /* Lower case */
#define GM_U   3 /* Upper case */
#define GM_M   4 /* Mixed */
#define GM_B   5 /* Byte */

#define GM_NUM_MODES 6

static const char gm_debug_modes[6][5] = { "HAN ", "NUM ", "LWR ", "UPR ", "MXD ", "BYT " };

/* Calculate optimized encoding modes. Adapted from Project Nayuki */
/* Copyright (c) Project Nayuki. (MIT License) See qr.c for detailed notice */
/* SPDX-License-Identifier: MIT */
static void gm_define_modes(char *modes, const unsigned int ddata[], const int length, const int debug_print) {
    /* Must be in same order as GM_H etc */
    static const char mode_types[] = { GM_CHINESE, GM_NUMERAL, GM_LOWER, GM_UPPER, GM_MIXED, GM_BYTE, '\0' };

    /* Initial mode costs */
    static const unsigned int head_costs[GM_NUM_MODES] = {
    /*  H            N (+pad prefix)    L            U            M            B (+byte count) */
        4 * GM_MULT, (4 + 2) * GM_MULT, 4 * GM_MULT, 4 * GM_MULT, 4 * GM_MULT, (4 + 9) * GM_MULT
    };

    /* Cost of switching modes from k to j - see AIMD014 Rev. 1.63 Table 9 – Type conversion codes */
    static const unsigned char switch_costs[GM_NUM_MODES][GM_NUM_MODES] = {
        /*      H             N                   L             U             M             B  */
        /*H*/ {            0, (13 + 2) * GM_MULT, 13 * GM_MULT, 13 * GM_MULT, 13 * GM_MULT, (13 + 9) * GM_MULT },
        /*N*/ { 10 * GM_MULT,                  0, 10 * GM_MULT, 10 * GM_MULT, 10 * GM_MULT, (10 + 9) * GM_MULT },
        /*L*/ {  5 * GM_MULT,  (5 + 2) * GM_MULT,            0,  5 * GM_MULT,  7 * GM_MULT,  (7 + 9) * GM_MULT },
        /*U*/ {  5 * GM_MULT,  (5 + 2) * GM_MULT,  5 * GM_MULT,            0,  7 * GM_MULT,  (7 + 9) * GM_MULT },
        /*M*/ { 10 * GM_MULT, (10 + 2) * GM_MULT, 10 * GM_MULT, 10 * GM_MULT,            0, (10 + 9) * GM_MULT },
        /*B*/ {  4 * GM_MULT,  (4 + 2) * GM_MULT,  4 * GM_MULT,  4 * GM_MULT,  4 * GM_MULT,                  0 },
    };

    /* Final end-of-data cost - see AIMD014 Rev. 1.63 Table 9 – Type conversion codes */
    static const unsigned char eod_costs[GM_NUM_MODES] = {
    /*  H             N             L            U            M             B  */
        13 * GM_MULT, 10 * GM_MULT, 5 * GM_MULT, 5 * GM_MULT, 10 * GM_MULT, 4 * GM_MULT
    };

    unsigned int numeral_end = 0, numeral_cost = 0, byte_count = 0; /* State */
    int double_byte, space, digit, lower, upper, control, double_digit, eol;

    int i, j, k;
    unsigned int min_cost;
    char cur_mode;
    unsigned int prev_costs[GM_NUM_MODES];
    unsigned int cur_costs[GM_NUM_MODES];
    char (*char_modes)[GM_NUM_MODES] = (char (*)[GM_NUM_MODES]) z_alloca(GM_NUM_MODES * length);

    /* char_modes[i][j] represents the mode to encode the code point at index i such that the final segment
       ends in mode_types[j] and the total number of bits is minimized over all possible choices */
    memset(char_modes, 0, length * GM_NUM_MODES);

    /* At the beginning of each iteration of the loop below, prev_costs[j] is the minimum number of 1/6 (1/GM_MULT)
     * bits needed to encode the entire string prefix of length i, and end in mode_types[j] */
    memcpy(prev_costs, head_costs, GM_NUM_MODES * sizeof(unsigned int));

    /* Calculate costs using dynamic programming */
    for (i = 0; i < length; i++) {
        memset(cur_costs, 0, GM_NUM_MODES * sizeof(unsigned int));

        space = digit = lower = upper = control = double_digit = eol = 0;

        if (!(double_byte = ddata[i] > 0xFF) && !(space = ddata[i] == ' ')) {
            if (!(digit = z_isdigit(ddata[i]))) {
                if (!(lower = z_islower(ddata[i])) && !(upper = z_isupper(ddata[i]))) {
                    control = ddata[i] < 0x7F; /* Exclude DEL */
                    eol = i + 1 < length && ddata[i] == 13 && ddata[i + 1] == 10;
                }
            } else if (i + 1 < length) {
                double_digit = z_isdigit(ddata[i + 1]);
            }
        }

        /* Hanzi mode can encode anything */
        cur_costs[GM_H] = prev_costs[GM_H] + (double_digit || eol ? 39 : 78); /* (6.5 : 13) * GM_MULT */
        char_modes[i][GM_H] = GM_CHINESE;

        /* Byte mode can encode anything */
        if (byte_count == 512 || (double_byte && byte_count == 511)) {
            cur_costs[GM_B] = head_costs[GM_B];
            if (double_byte && byte_count == 511) {
                cur_costs[GM_B] += 48; /* 8 * GM_MULT */
                double_byte = 0; /* Splitting double-byte so mark as single */
            }
            byte_count = 0;
        }
        cur_costs[GM_B] += prev_costs[GM_B] + (double_byte ? 96 : 48); /* (16 : 8) * GM_MULT */
        char_modes[i][GM_B] = GM_BYTE;
        byte_count += double_byte ? 2 : 1;

        if (gm_in_numeral(ddata, length, i, &numeral_end, &numeral_cost)) {
            cur_costs[GM_N] = prev_costs[GM_N] + numeral_cost;
            char_modes[i][GM_N] = GM_NUMERAL;
        }

        if (control) {
            cur_costs[GM_L] = prev_costs[GM_L] + 78; /* (7 + 6) * GM_MULT */
            char_modes[i][GM_L] = GM_LOWER;
            cur_costs[GM_U] = prev_costs[GM_U] + 78; /* (7 + 6) * GM_MULT */
            char_modes[i][GM_U] = GM_UPPER;
            cur_costs[GM_M] = prev_costs[GM_M] + 96; /* (10 + 6) * GM_MULT */
            char_modes[i][GM_M] = GM_MIXED;
        } else {
            if (lower || space) {
                cur_costs[GM_L] = prev_costs[GM_L] + 30; /* 5 * GM_MULT */
                char_modes[i][GM_L] = GM_LOWER;
            }
            if (upper || space) {
                cur_costs[GM_U] = prev_costs[GM_U] + 30; /* 5 * GM_MULT */
                char_modes[i][GM_U] = GM_UPPER;
            }
            if (digit || lower || upper || space) {
                cur_costs[GM_M] = prev_costs[GM_M] + 36; /* 6 * GM_MULT */
                char_modes[i][GM_M] = GM_MIXED;
            }
        }

        if (i + 1 == length) { /* Add end of data costs if last character */
            for (j = 0; j < GM_NUM_MODES; j++) {
                if (char_modes[i][j]) {
                    cur_costs[j] += eod_costs[j];
                }
            }
        }

        /* Start new segment at the end to switch modes */
        for (j = 0; j < GM_NUM_MODES; j++) { /* To mode */
            for (k = 0; k < GM_NUM_MODES; k++) { /* From mode */
                if (j != k && char_modes[i][k]) {
                    const unsigned int new_cost = cur_costs[k] + switch_costs[k][j];
                    if (!char_modes[i][j] || new_cost < cur_costs[j]) {
                        cur_costs[j] = new_cost;
                        char_modes[i][j] = mode_types[k];
                    }
                }
            }
        }

        memcpy(prev_costs, cur_costs, GM_NUM_MODES * sizeof(unsigned int));
    }

    /* Find optimal ending mode */
    min_cost = prev_costs[0];
    cur_mode = mode_types[0];
    for (i = 1; i < GM_NUM_MODES; i++) {
        if (prev_costs[i] < min_cost) {
            min_cost = prev_costs[i];
            cur_mode = mode_types[i];
        }
    }

    /* Get optimal mode for each code point by tracing backwards */
    for (i = length - 1; i >= 0; i--) {
        j = z_posn(mode_types, cur_mode);
        cur_mode = char_modes[i][j];
        modes[i] = cur_mode;
    }

    if (debug_print) {
        fputs("  Modes: ", stdout);
        for (i = 0; i < length; i++) fputc(gm_debug_modes[modes[i] - 1][0], stdout);
        fputc('\n', stdout);
    }
}

/* Add the length indicator for byte encoded blocks */
static void gm_add_byte_count(char binary[], const int byte_count_posn, const int byte_count) {
    /* AIMD014 6.3.7: "Let L be the number of bytes of input data to be encoded in the 8-bit binary data set.
     * First output (L-1) as a 9-bit binary prefix to record the number of bytes..." */
    z_bin_append_posn(byte_count - 1, 9, binary, byte_count_posn);
}

/* Add a control character to the data stream */
static int gm_add_shift_char(char binary[], int bp, const int shifty, const int debug_print) {
    int glyph;

    /* See Table 7 - Encoding of control characters */
    if (shifty < ' ') {
        glyph = shifty;
    } else if (shifty < '0') {
        glyph = shifty - 1;
    } else if (shifty < 'A') {
        glyph = shifty - 11;
    } else if (shifty < 'a') {
        glyph = shifty - 46;
    } else {
        glyph = shifty - 63;
    }

    if (debug_print) {
        printf("SHIFT [%d] ", glyph);
    }

    bp = z_bin_append_posn(glyph, 6, binary, bp);

    return bp;
}

static int gm_encode(const unsigned int ddata[], const int length, char binary[], const int eci, int *p_bp,
            const int debug_print) {
    /* Create a binary stream representation of the input data.
       6 sets are defined - Chinese characters, Numerals, Lower case letters, Upper case letters,
       Mixed numerals, letters & control characters, and 8-bit binary data */
    int sp = 0;
    int current_mode = 0;
    int last_mode;
    unsigned int glyph = 0;
    int numeral_cnt = 0;
    int numeral_pad_posn = 0;
    int byte_count_posn = 0;
    int byte_count = 0;
    int bp = *p_bp;
    char *modes = (char *) z_alloca(length);

    if (eci != 0) {
        /* ECI assignment according to Table 8 */
        bp = z_bin_append_posn(12, 4, binary, bp); /* ECI */
        if (eci <= 1023) {
            bp = z_bin_append_posn(eci, 11, binary, bp);
        } else if (eci <= 32767) {
            bp = z_bin_append_posn(2, 2, binary, bp);
            bp = z_bin_append_posn(eci, 15, binary, bp);
        } else {
            bp = z_bin_append_posn(3, 2, binary, bp);
            bp = z_bin_append_posn(eci, 20, binary, bp);
        }
    }

    gm_define_modes(modes, ddata, length, debug_print);

    do {
        const int next_mode = modes[sp];
        int numbuf[3], punt = 0, nondigit_posn;
        int digit;

        if (next_mode != current_mode) {
            if (current_mode == GM_BYTE) {
                /* Add byte block length indicator */
                gm_add_byte_count(binary, byte_count_posn, byte_count);
                byte_count = 0;
            } else if (current_mode == GM_NUMERAL && numeral_cnt) {
                /* Set numeric block padding value */
                (void) z_bin_append_posn(3 - numeral_cnt, 2, binary, numeral_pad_posn);
            }
            bp = z_bin_append_posn(gm_mode_switch[current_mode][next_mode - 1],
                                    gm_mode_len[current_mode][next_mode - 1], binary, bp);
            if (debug_print) {
                fputs(gm_debug_modes[next_mode - 1], stdout);
            }
            if (bp > 9191) {
                return ZINT_ERROR_TOO_LONG;
            }
        }
        last_mode = current_mode;
        current_mode = next_mode;

        switch (current_mode) {
            case GM_CHINESE:
                if (ddata[sp] > 0xFF) {
                    /* GB2312 character */
                    const unsigned char c1 = (ddata[sp] >> 8) & 0xFF;
                    const unsigned char c2 = ddata[sp] & 0xFF;

                    /* GB 2312 always within these ranges */
                    if (c1 >= 0xA1 && c1 <= 0xA9) {
                        glyph = 0x60 * (c1 - 0xA1) + (c2 - 0xA0);
                    } else if (c1 >= 0xB0 && c1 <= 0xF7) {
                        glyph = 0x60 * (c1 - 0xB0 + 9) + (c2 - 0xA0);
                    }
                    /* Note not using the unallocated glyphs 7776 to 8191 mentioned in AIMD014 section 6.3.1.2 */
                } else if (sp + 1 < length && ddata[sp] == 13 && ddata[sp + 1] == 10) {
                    /* End of Line */
                    glyph = 7776;
                    sp++;
                } else if (sp + 1 < length && z_isdigit(ddata[sp]) && z_isdigit(ddata[sp + 1])) {
                    /* Two digits */
                    glyph = 8033 + (10 * (ddata[sp] - '0')) + (ddata[sp + 1] - '0');
                    sp++;
                } else {
                    /* Byte value */
                    glyph = 7777 + ddata[sp];
                }

                if (debug_print) {
                    printf("[%d] ", (int) glyph);
                }

                bp = z_bin_append_posn(glyph, 13, binary, bp);
                sp++;
                break;

            case GM_NUMERAL:
                if (last_mode != current_mode) {
                    /* Reserve a space for numeric digit padding value (2 bits) */
                    numeral_pad_posn = bp;
                    bp = z_bin_append_posn(0, 2, binary, bp);
                }
                numeral_cnt = 0;
                nondigit_posn = -1;

                /* Numeral compression can also include certain combinations of non-digits */

                numbuf[0] = '0';
                numbuf[1] = '0';
                numbuf[2] = '0';
                do {
                    if (z_isdigit(ddata[sp])) {
                        numbuf[numeral_cnt++] = ddata[sp];
                    } else if (z_posn(gm_numeral_nondigits, (const char) ddata[sp]) != -1) {
                        if (nondigit_posn != -1) {
                            break;
                        }
                        punt = ddata[sp];
                        nondigit_posn = numeral_cnt;
                    } else if (sp + 1 < length && ddata[sp] == 13 && ddata[sp + 1] == 10) {
                        /* <end of line> */
                        if (nondigit_posn != -1) {
                            break;
                        }
                        punt = ddata[sp++];
                        nondigit_posn = numeral_cnt;
                    } else {
                        break;
                    }
                    sp++;
                } while (numeral_cnt < 3 && sp < length && modes[sp] == GM_NUMERAL);

                if (nondigit_posn != -1) {
                    if (punt == 13) {
                        glyph = 15;
                    } else {
                        glyph = z_posn(gm_numeral_nondigits, punt) * 3;
                    }
                    glyph += nondigit_posn;
                    glyph += 1000;

                    if (debug_print) {
                        printf("[%d] ", (int) glyph);
                    }

                    bp = z_bin_append_posn(glyph, 10, binary, bp);
                }

                glyph = (100 * (numbuf[0] - '0')) + (10 * (numbuf[1] - '0')) + (numbuf[2] - '0');
                if (debug_print) {
                    printf("[%d] ", (int) glyph);
                }

                bp = z_bin_append_posn(glyph, 10, binary, bp);
                break;

            case GM_BYTE:
                if (last_mode != current_mode) {
                    /* Reserve space for byte block length indicator (9 bits) */
                    byte_count_posn = bp;
                    bp = z_bin_append_posn(0, 9, binary, bp);
                }
                glyph = ddata[sp];
                if (byte_count == 512 || (glyph > 0xFF && byte_count == 511)) {
                    /* Maximum byte block size is 512 bytes. If longer is needed then start a new block */
                    if (glyph > 0xFF && byte_count == 511) { /* Split double-byte */
                        bp = z_bin_append_posn(glyph >> 8, 8, binary, bp);
                        glyph &= 0xFF;
                        byte_count++;
                    }
                    gm_add_byte_count(binary, byte_count_posn, byte_count);
                    bp = z_bin_append_posn(7, 4, binary, bp);
                    byte_count_posn = bp;
                    bp = z_bin_append_posn(0, 9, binary, bp);
                    byte_count = 0;
                }

                if (debug_print) {
                    printf("[%d] ", (int) glyph);
                }
                bp = z_bin_append_posn(glyph, glyph > 0xFF ? 16 : 8, binary, bp);
                sp++;
                byte_count++;
                if (glyph > 0xFF) {
                    byte_count++;
                }
                break;

            case GM_MIXED:
                if (ddata[sp] == ' ' || (digit = z_isdigit(ddata[sp])) || z_isalpha(ddata[sp])) {
                    /* Mixed Mode character */
                    glyph = ddata[sp] == ' ' ? 62
                                             : ddata[sp] - (digit ? '0' : z_isupper(ddata[sp]) ? 'A' - 10 : 'a' - 36);
                    if (debug_print) {
                        printf("[%d] ", (int) glyph);
                    }
                    bp = z_bin_append_posn(glyph, 6, binary, bp);
                } else {
                    /* Shift Mode character */
                    bp = z_bin_append_posn(1014, 10, binary, bp); /* Shift indicator */
                    bp = gm_add_shift_char(binary, bp, ddata[sp], debug_print);
                }

                sp++;
                break;

            case GM_UPPER:
                if (ddata[sp] == ' ' || z_isupper(ddata[sp])) {
                    /* Upper Case character */
                    glyph = ddata[sp] == ' ' ? 26 : ddata[sp] - 'A';
                    if (debug_print) {
                        printf("[%d] ", (int) glyph);
                    }
                    bp = z_bin_append_posn(glyph, 5, binary, bp);
                } else {
                    /* Shift Mode character */
                    bp = z_bin_append_posn(125, 7, binary, bp); /* Shift indicator */
                    bp = gm_add_shift_char(binary, bp, ddata[sp], debug_print);
                }

                sp++;
                break;

            case GM_LOWER:
                if (ddata[sp] == ' ' || z_islower(ddata[sp])) {
                    /* Lower Case character */
                    glyph = ddata[sp] == ' ' ? 26 : ddata[sp] - 'a';
                    if (debug_print) {
                        printf("[%d] ", (int) glyph);
                    }
                    bp = z_bin_append_posn(glyph, 5, binary, bp);
                } else {
                    /* Shift Mode character */
                    bp = z_bin_append_posn(125, 7, binary, bp); /* Shift indicator */
                    bp = gm_add_shift_char(binary, bp, ddata[sp], debug_print);
                }

                sp++;
                break;
        }
        if (bp > 9191) {
            return ZINT_ERROR_TOO_LONG;
        }

    } while (sp < length);

    if (current_mode == GM_BYTE) {
        /* Add byte block length indicator */
        gm_add_byte_count(binary, byte_count_posn, byte_count);
    } else if (current_mode == GM_NUMERAL && numeral_cnt) {
        /* HAN numeric block padding value */
        (void) z_bin_append_posn(3 - numeral_cnt, 2, binary, numeral_pad_posn);
    }

    /* Add "end of data" character */
    assert(current_mode >= GM_CHINESE && current_mode <= GM_BYTE);
    bp = z_bin_append_posn(gm_mode_switch[GM_EOD][current_mode - 1], gm_mode_len[GM_EOD][current_mode - 1],
                            binary, bp);

    if (bp > 9191) {
        return ZINT_ERROR_TOO_LONG;
    }

    *p_bp = bp;

    if (debug_print) {
        printf("\nBinary (%d): %.*s\n", bp, bp, binary);
    }

    return 0;
}

static int gm_encode_segs(const unsigned int ddata[], const struct zint_seg segs[], const int seg_count,
            char binary[], const int reader, const struct zint_structapp *p_structapp, int *p_bin_len,
            const int debug_print) {
    int i;
    const unsigned int *dd = ddata;
    int bp = 0;
    int padding;

    if (reader && (!p_structapp || p_structapp->index == 1)) { /* Appears only in 1st symbol if Structured Append */
        bp = z_bin_append_posn(10, 4, binary, bp); /* FNC3 - Reader Initialisation */
    }

    if (p_structapp) {
        bp = z_bin_append_posn(9, 4, binary, bp); /* FNC2 - Structured Append */
        bp = z_bin_append_posn(z_to_int(ZCUCP( p_structapp->id), (int) strlen(p_structapp->id)), 8,
                binary, bp); /* File signature */
        bp = z_bin_append_posn(p_structapp->count - 1, 4, binary, bp);
        bp = z_bin_append_posn(p_structapp->index - 1, 4, binary, bp);
    }

    for (i = 0; i < seg_count; i++) {
        int error_number = gm_encode(dd, segs[i].length, binary, segs[i].eci, &bp, debug_print);
        if (error_number != 0) {
            return error_number;
        }
        dd += segs[i].length;
    }

    /* Add padding bits if required */
    padding = 7 - (bp % 7);
    if (padding % 7) {
        bp = z_bin_append_posn(0, padding, binary, bp);
    }
    /* Note bit-padding can't tip `bp` over max 9191 (1313 * 7) */

    if (debug_print) {
        printf("\nBinary (%d): %.*s\n", bp, bp, binary);
    }

    *p_bin_len = bp;

    return 0;
}

static void gm_add_ecc(const char binary[], const int num_data_cws, const int layers, const int ecc_level,
                unsigned char cws[]) {
    int tot_data_cws, i, j, wp;
    int n1, b1, n2, b2, e1, b3, e2;
    int num_blocks;
    unsigned char data[1313] = {0};
    rs_t rs;

    /* Convert from binary stream to 7-bit codewords */
    for (i = 0; i < num_data_cws; i++) {
        for (j = 0; j < 7; j++) {
            data[i] |= (binary[i * 7 + j] == '1') << (6 - j);
        }
    }

    tot_data_cws = gm_data_cws[layers - 1][ecc_level - 1];

    /* Add padding codewords */
    for (i = num_data_cws + 1; i < tot_data_cws; i++) {
        if (i & 1) {
            data[i] = 0x7E;
        }
    }

    /* Get block sizes */
    n1 = gm_n1[layers - 1];
    b1 = gm_b1[layers - 1];
    n2 = n1 - 1;
    b2 = gm_b2[layers - 1];
    e1 = gm_e1b3e2[layers - 1][ecc_level - 1][0];
    b3 = gm_e1b3e2[layers - 1][ecc_level - 1][1];
    e2 = gm_e1b3e2[layers - 1][ecc_level - 1][2];

    num_blocks = b1 + b2;

    zint_rs_init_gf(&rs, 0x89);

    /* Split the data into blocks */
    wp = 0;
    for (i = 0; i < num_blocks; i++) {
        unsigned char block[126];
        const int block_size = i < b1 ? n1 : n2;
        const int ecc_size = i < b3 ? e1 : e2;
        const int data_size = block_size - ecc_size;

        memcpy(block, data + wp, data_size);
        wp += data_size;

        /* Calculate and add the ECC data for this block */
        zint_rs_init_code(&rs, ecc_size, 1);
        zint_rs_encode(&rs, data_size, block, block + data_size);

        for (j = 0; j < block_size; j++) {
            cws[num_blocks * j + i] = block[j];
        }
    }
}

static void gm_place_macromodule(char grid[], const int size, const int x, const int y, const unsigned int cw) {
    const int i = (x * 6) + 1;
    const int j = ((y * 6) + 1) * size;
    int r, c;

    if (cw & 0x2000) {
        grid[j + i + 2] = '1';
    }
    if (cw & 0x1000) {
        grid[j + i + 3] = '1';
    }
    for (r = 1; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            if ((cw >> (15 - r * 4 - c)) & 1) {
                grid[j + r * size + i + c] = '1';
            }
        }
    }
}

/* Get the macromodule index, `x` column, `y` row - see https://stackoverflow.com/a/63909183 */
static int gm_macromodule(const int macromodules_per_dim, const int x, const int y) {
    /* Ring 0-26 from outermost to inner */
    const int r = x < y ? x < macromodules_per_dim - y - 1 ? x : macromodules_per_dim - y - 1
                        : y < macromodules_per_dim - x - 1 ? y : macromodules_per_dim - x - 1;
    const int m = (((macromodules_per_dim - 1) >> 1) - r) * 2 + 1; /* Macromodules per dim for ring */
    const int b = m * m - 1; /* Highest index in ring */
    int idx;

    /* Left */
    if (x == r) {
        idx = b - y + r;
    /* Top */
    } else if (y == r) {
        idx = b - (m - 1) * 4 + (x - r);
    /* Right */
    } else if (x == macromodules_per_dim - r - 1) {
        idx = b - (m - 1) * 3 + (y - r);
    /* Bottom */
    } else {
        assert(y == macromodules_per_dim - r - 1);
        idx = b - (m - 1) - (x - r);
    }
    return idx << 1; /* 2 codewords per macromodule */
}

static void gm_place_data_in_grid(const unsigned char cws[], char grid[], const int size, const int layers) {
    int x, y;
    const int macromodules_per_dim = 1 + (layers << 1);

    for (y = 0; y < macromodules_per_dim; y++) {
        for (x = 0; x < macromodules_per_dim; x++) {
            const int macromodule = gm_macromodule(macromodules_per_dim, x, y);
            gm_place_macromodule(grid, size, x, y, ((unsigned int) cws[macromodule + 1]) << 7 | cws[macromodule]);
        }
    }
}

/* Place the layer ID into each macromodule */
static void gm_place_layer_id(char grid[], const int size, const int layers, const int ecc_level) {
    int i, j, layer, start, stop;
    const int macromodules_per_dim = 1 + (layers << 1);
    int *layer_ids = (int *) z_alloca(sizeof(int) * (layers + 1));
    int *ids = (int *) z_alloca(sizeof(int) * (macromodules_per_dim * macromodules_per_dim));

    assert(layers > 0); /* Suppress clang-tidy-23 warning clang-analyzer-core.UndefinedBinaryOperatorResult */

    /* Calculate Layer IDs */
    for (i = 0; i <= layers; i++) {
        if (ecc_level == 1) {
            layer_ids[i] = 3 - (i & 0x03);
        } else {
            layer_ids[i] = (i + 5 - ecc_level) & 0x03;
        }
    }

    /* Calculate which value goes in each macromodule */
    start = macromodules_per_dim >> 1;
    stop = macromodules_per_dim >> 1;
    for (layer = 0; layer <= layers; layer++) {
        for (i = start; i <= stop; i++) {
            ids[(start * macromodules_per_dim) + i] = layer_ids[layer];
            ids[(i * macromodules_per_dim) + start] = layer_ids[layer];
            ids[((macromodules_per_dim - start - 1) * macromodules_per_dim) + i] = layer_ids[layer];
            ids[(i * macromodules_per_dim) + (macromodules_per_dim - start - 1)] = layer_ids[layer];
        }
        start--;
        stop++;
    }

    /* Place the data in the grid */
    for (i = 0; i < macromodules_per_dim; i++) {
        for (j = 0; j < macromodules_per_dim; j++) {
            const int id = ids[(i * macromodules_per_dim) + j];
            if (id & 0x02) {
                grid[(i * 6 + 1) * size + j * 6 + 1] = '1';
            }
            if (id & 0x01) {
                grid[(i * 6 + 1) * size + j * 6 + 1 + 1] = '1';
            }
        }
    }
}

INTERNAL int zint_gridmatrix(struct zint_symbol *symbol, struct zint_seg segs[], const int seg_count) {
    int warn_number = 0;
    int size, macromodules_per_dim, error_number;
    int auto_layers, min_layers, layers, rec_ecc_level, min_rec_ecc_level, ecc_level;
    int x, y, i;
    int full_multibyte;
    char binary[9240]; /* 1313 * 7 = 9191 + 46 (max overflow (GM_BYTE) in `gm_encode()`) + 3 */
    int num_data_cws;
    unsigned char cws[1458] = {0};
    int reader = 0;
    const struct zint_structapp *p_structapp = NULL;
    int size_squared;
    int bin_len;
    /* Raw text dealt with by `ZBarcode_Encode_Segs()`, except for `eci` feedback */
    const int content_segs = symbol->output_options & BARCODE_CONTENT_SEGS;
    const int debug_print = symbol->debug & ZINT_DEBUG_PRINT;
    const int eci_length_segs = zint_get_eci_length_segs(segs, seg_count);
    struct zint_seg *local_segs = (struct zint_seg *) z_alloca(sizeof(struct zint_seg) * seg_count);
    unsigned int *ddata = (unsigned int *) z_alloca(sizeof(unsigned int) * eci_length_segs);
    char *grid;

    z_segs_cpy(symbol, segs, seg_count, local_segs); /* Shallow copy (needed to set default ECIs & protect lengths) */

    /* If ZINT_FULL_MULTIBYTE set use Hanzi mode in DATA_MODE or for non-GB 2312 in UNICODE_MODE */
    full_multibyte = (symbol->option_3 & 0xFF) == ZINT_FULL_MULTIBYTE;

    if ((symbol->input_mode & 0x07) == DATA_MODE) {
        zint_gb2312_cpy_segs(symbol, local_segs, seg_count, ddata, full_multibyte);
    } else {
        unsigned int *dd = ddata;
        for (i = 0; i < seg_count; i++) {
            int eci = 0;
            if (local_segs[i].eci != 0 && local_segs[i].eci != 29) { /* Unless default or ECI 29 (GB 2312) */
                /* Try other conversions */
                error_number = zint_gb2312_utf8_to_eci(local_segs[i].eci, local_segs[i].source, &local_segs[i].length,
                                                        dd, full_multibyte);
                if (error_number == 0) {
                    eci = local_segs[i].eci;
                } else {
                    return z_errtxtf(error_number, symbol, 535, "Invalid character in input for ECI '%d'",
                                    local_segs[i].eci);
                }
            }
            if (!eci) {
                /* Try GB 2312 (EUC-CN) */
                error_number = zint_gb2312_utf8(symbol, local_segs[i].source, &local_segs[i].length, dd);
                if (error_number != 0) {
                    return error_number;
                }
                eci = 29;
            }
            if (content_segs && eci) {
                z_ct_set_seg_eci(symbol, i, eci);
            }
            dd += local_segs[i].length;
        }
    }

    if (symbol->output_options & READER_INIT) reader = 1;

    if (symbol->structapp.count) {
        if (symbol->structapp.count < 2 || symbol->structapp.count > 16) {
            return z_errtxtf(ZINT_ERROR_INVALID_OPTION, symbol, 536,
                            "Structured Append count '%d' out of range (2 to 16)", symbol->structapp.count);
        }
        if (symbol->structapp.index < 1 || symbol->structapp.index > symbol->structapp.count) {
            return ZEXT z_errtxtf(ZINT_ERROR_INVALID_OPTION, symbol, 537,
                                    "Structured Append index '%1$d' out of range (1 to count %2$d)",
                                    symbol->structapp.index, symbol->structapp.count);
        }
        if (symbol->structapp.id[0]) {
            int id, id_len;

            for (id_len = 1; id_len < 4 && symbol->structapp.id[id_len]; id_len++);

            if (id_len > 3) { /* 255 (8 bits) */
                return z_errtxtf(ZINT_ERROR_INVALID_OPTION, symbol, 538,
                                "Structured Append ID length %d too long (3 digit maximum)", id_len);
            }

            id = z_to_int(ZCUCP(symbol->structapp.id), id_len);
            if (id == -1) {
                return z_errtxt(ZINT_ERROR_INVALID_OPTION, symbol, 539, "Invalid Structured Append ID (digits only)");
            }
            if (id > 255) {
                return z_errtxtf(ZINT_ERROR_INVALID_OPTION, symbol, 530,
                                "Structured Append ID value '%d' out of range (0 to 255)", id);
            }
        }
        p_structapp = &symbol->structapp;
    }

    if (symbol->eci > 811799) {
        return z_errtxtf(ZINT_ERROR_INVALID_OPTION, symbol, 533, "ECI code '%d' out of range (0 to 811799)",
                        symbol->eci);
    }

    error_number = gm_encode_segs(ddata, local_segs, seg_count, binary, reader, p_structapp, &bin_len, debug_print);
    if (error_number != 0) {
        return z_errtxt(error_number, symbol, 531, "Input too long, requires too many codewords (maximum 1313)");
    }

    /* Determine the size of the symbol */
    assert(bin_len % 7 == 0); /* Binary length always a multiple of 7 */
    num_data_cws = bin_len / 7;

    auto_layers = 13;
    for (i = 12; i > 0; i--) {
        if (gm_recommend_cws[i - 1] >= num_data_cws) {
            auto_layers = i;
        }
    }
    min_layers = 13;
    for (i = 12; i > 0; i--) {
        if (gm_max_cws[i - 1] >= num_data_cws) {
            min_layers = i;
        }
    }
    layers = auto_layers;

    if (symbol->option_2 >= 1 && symbol->option_2 <= 13) {
        if (symbol->option_2 >= min_layers) {
            layers = symbol->option_2;
        } else {
            return ZEXT z_errtxtf(ZINT_ERROR_TOO_LONG, symbol, 534,
                                    "Input too long for Version %1$d, requires %2$d codewords (maximum %3$d)",
                                    symbol->option_2, num_data_cws, gm_max_cws[symbol->option_2 - 1]);
        }
    }

    /* Table 11 - Recommended error correction levels - Recommended ECL */
    rec_ecc_level = 3;
    if (layers == 1) {
        rec_ecc_level = 5;
    } else if (layers == 2 || layers == 3) {
        rec_ecc_level = 4;
    }
    ecc_level = rec_ecc_level;

    /* Table 11 - Recommended error correction levels - Minimum Recommended ECL */
    min_rec_ecc_level = 1;
    if (layers == 1) {
        min_rec_ecc_level = 4;
    } else if (layers == 2) {
        min_rec_ecc_level = 2;
    }

    if (symbol->option_1 >= 1 && symbol->option_1 <= 5) {
        /* Silently ignoring user-specified if less than minimum recommended */
        if (symbol->option_1 >= min_rec_ecc_level) {
            ecc_level = symbol->option_1;
        } else {
            ecc_level = min_rec_ecc_level;
        }
    }
    if (num_data_cws > gm_data_cws[layers - 1][ecc_level - 1]) {
        /* Reduce ECC level */
        const int min_ecc_level = layers > 1 ? 1 : 2; /* ECL 1 in version 1 symbols is invalid */
        do {
            ecc_level--;
        } while (num_data_cws > gm_data_cws[layers - 1][ecc_level - 1] && ecc_level > min_ecc_level);
        assert(num_data_cws <= gm_data_cws[layers - 1][ecc_level - 1]);
    }

    if (debug_print) {
        printf("Layers: %d, ECC level: %d, Data Codewords: %d\n", layers, ecc_level, num_data_cws);
    }

    /* Feedback options */
    symbol->option_1 = ecc_level;
    symbol->option_2 = layers;

    gm_add_ecc(binary, num_data_cws, layers, ecc_level, cws);
#ifdef ZINT_TEST
    if (symbol->debug & ZINT_DEBUG_TEST) z_debug_test_codeword_dump(symbol, cws, num_data_cws);
#endif
    size = 6 + (layers * 12);
    macromodules_per_dim = 1 + (layers * 2);
    size_squared = size * size;

    grid = (char *) z_alloca(size_squared);
    memset(grid, '0', size_squared);

    gm_place_data_in_grid(cws, grid, size, layers);
    gm_place_layer_id(grid, size, layers, ecc_level);

    /* Add macromodule frames */
    for (x = 0; x < macromodules_per_dim; x++) {
        const int x_offset = x * 6;
        int dark = !(x & 1);
        for (y = 0; y < macromodules_per_dim; y++) {
            if (dark) {
                const int y_offset = y * 6 * size;
                for (i = 0; i < 5; i++) {
                    grid[y_offset + x_offset + i] = '1';
                    grid[y_offset + 5 * size + x_offset + i] = '1';
                    grid[y_offset + i * size + x_offset] = '1';
                    grid[y_offset + i * size + x_offset + 5] = '1';
                }
                grid[y_offset + 5 * size + x_offset + 5] = '1';
            }
            dark = !dark;
        }
    }

    /* Copy values to symbol */
    symbol->width = size;
    symbol->rows = size;

    for (x = 0; x < size; x++) {
        for (y = 0; y < size; y++) {
            if (grid[(y * size) + x] == '1') {
                z_set_module(symbol, y, x);
            }
        }
        symbol->row_height[x] = 1;
    }
    symbol->height = size;

    return warn_number;
}

/* vim: set ts=4 sw=4 et : */
