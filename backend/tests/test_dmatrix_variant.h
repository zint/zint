/*
    libzint - the open source barcode library
    Copyright (C) 2021 Robin Stuart <rstuart114@gmail.com>

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

/* Used by test_matrix.c test_minimize() (TODO: remove in the not-too-distant future) */

#ifndef Z_DMATRIX_VARIANT_H
#define Z_DMATRIX_VARIANT_H

#include "../dmatrix.h"

static int dm_isc40(const unsigned char input) {
    if (input <= '9') {
        return input >= '0' || input == ' ';
    }
    return input >= 'A' && input <= 'Z';
}

/* Is basic (non-shifted) TEXT? */
static int dm_istext(const unsigned char input) {
    if (input <= '9') {
        return input >= '0' || input == ' ';
    }
    return input >= 'a' && input <= 'z';
}

/* Is basic (non-shifted) C40/TEXT? */
static int dm_isc40text(const int current_mode, const unsigned char input) {
    return current_mode == DM_C40 ? dm_isc40(input) : dm_istext(input);
}

/* Return true (1) if a character is valid in X12 set */
static int dm_isX12(const unsigned char input) {
    return dm_isc40(input) || input == 13 || input == '*' || input == '>';
}

/* Return true (1) if a character is valid in EDIFACT set */
static int dm_isedifact(const unsigned char input, const int gs1) {
    return (input >= ' ' && input <= '^') && (!gs1 || input != '['); /* Can't encode GS1 FNC1/GS in EDIFACT */
}

static int dm_p_r_6_2_1(const unsigned char source[], const int length, const int sp) {
    /* Annex P section (r)(6)(ii)(I)
       "If one of the three X12 terminator/separator characters first
        occurs in the yet to be processed data before a non-X12 character..."
     */
    int i;

    for (i = sp; i < length && dm_isX12(source[i]); i++) {
        if (source[i] == 13 || source[i] == '*' || source[i] == '>') {
            return 1;
        }
    }

    return 0;
}

/* Count number of TEXT characters around `sp` between `position` and `length`
   - helper to avoid exiting from Base 256 too early if have series of TEXT characters */
static int dm_text_sp_cnt(const unsigned char source[], const int position, const int length, const int sp) {
    int i;
    int cnt = 0;

    /* Count from `sp` forward */
    for (i = sp; i < length && dm_istext(source[i]); i++, cnt++);
    /* Count backwards from `sp` */
    for (i = sp - 1; i >= position && dm_istext(source[i]); i--, cnt++);

    return cnt;
}

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
#define DM_MULT_3           26
#define DM_MULT_13_DIV_4    39
#define DM_MULT_10_DIV_3    40
#define DM_MULT_4           48
#define DM_MULT_17_DIV_4    51
#define DM_MULT_13_DIV_3    52

#define DM_MULT_MINUS_1     11
#define DM_MULT_CEIL(n)     ((((n) + DM_MULT_MINUS_1) / DM_MULT) * DM_MULT)

static int dm_look_ahead_test_variant(const unsigned char source[], const int length, const int position,
            const int current_mode, const int mode_arg, const int gs1, const int debug_print, const int variant) {
    int ascii_count, c40_count, text_count, x12_count, edf_count, b256_count;
    int ascii_rnded, c40_rnded, text_rnded, x12_rnded, edf_rnded, b256_rnded;
    int cnt_1;
    int sp;

    int ascii_init_test, loop_test, edf_eod_test, x12_eod_test;

    /* step (j) */
    if (variant == 1) {
        ascii_init_test = (current_mode == DM_ASCII || current_mode == DM_BASE256);
    } else {
        ascii_init_test = (current_mode == DM_ASCII);
    }
    if (ascii_init_test) {
        ascii_count = 0;
        c40_count = DM_MULT_1;
        text_count = DM_MULT_1;
        x12_count = DM_MULT_1;
        edf_count = DM_MULT_1;
        if (variant == 1) {
            b256_count = DM_MULT_2;
        } else {
            b256_count = DM_MULT_5_DIV_4; // 1.25
        }
    } else {
        ascii_count = DM_MULT_1;
        c40_count = DM_MULT_2;
        text_count = DM_MULT_2;
        x12_count = DM_MULT_2;
        edf_count = DM_MULT_2;
        if (variant == 1) {
            b256_count = DM_MULT_3;
        } else {
            b256_count = DM_MULT_9_DIV_4; // 2.25
        }
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
        case DM_BASE256:
            if (variant == 1) {
                b256_count = mode_arg == 249 ? DM_MULT_1 : 0;
            } else {
                b256_count = 0;
            }
            break;
    }

    for (sp = position; sp < length; sp++) {
        unsigned char c = source[sp];
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
        if (dm_isc40(c)) {
            c40_count += DM_MULT_2_DIV_3; // (m)(1)
        } else {
            if (is_extended) {
                c40_count += DM_MULT_8_DIV_3; // (m)(2)
            } else {
                c40_count += DM_MULT_4_DIV_3; // (m)(3)
            }
        }

        /* text ... step (n) */
        if (dm_istext(c)) {
            text_count += DM_MULT_2_DIV_3; // (n)(1)
        } else {
            if (is_extended) {
                text_count += DM_MULT_8_DIV_3; // (n)(2)
            } else {
                text_count += DM_MULT_4_DIV_3; // (n)(3)
            }
        }

        /* x12 ... step (o) */
        if (dm_isX12(c)) {
            x12_count += DM_MULT_2_DIV_3; // (o)(1)
        } else {
            if (is_extended) {
                x12_count += DM_MULT_13_DIV_3; // (o)(2)
            } else {
                x12_count += DM_MULT_10_DIV_3; // (o)(3)
            }
        }

        /* edifact ... step (p) */
        if (dm_isedifact(c, gs1)) {
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

        if (variant == 1) {
            loop_test = sp >= position + 3;
        } else {
            loop_test = sp >= position + 4;
        }
        if (loop_test) {
            int ascii_loop_test;

            /* At least 5 data characters processed ... step (r) */
            /* NOTE: different than spec, where it's at least 4. Following previous behaviour here (and BWIPP) */

            if (debug_print) {
                printf("\n(m:%d, p:%d, sp:%d, a:%d): ascii_count %d, b256_count %d, edf_count %d, text_count %d"
                        ", x12_count %d, c40_count %d ",
                        current_mode, position, sp, mode_arg, ascii_count, b256_count, edf_count, text_count,
                        x12_count, c40_count);
            }

            cnt_1 = ascii_count + DM_MULT_1;
            if (variant == 1) {
                ascii_loop_test = (cnt_1 < b256_count && cnt_1 <= edf_count && cnt_1 <= text_count && cnt_1 <= x12_count && cnt_1 <= c40_count);
            } else {
                ascii_loop_test = (cnt_1 <= b256_count && cnt_1 <= edf_count && cnt_1 <= text_count && cnt_1 <= x12_count && cnt_1 <= c40_count);
            }
            if (ascii_loop_test) {
                if (debug_print) printf("ASC->");
                return DM_ASCII; /* step (r)(1) */
            }

            cnt_1 = b256_count + DM_MULT_1;
            if (cnt_1 <= ascii_count || (cnt_1 < edf_count && cnt_1 < text_count && cnt_1 < x12_count
                    && cnt_1 < c40_count)) {
                if (debug_print) printf("BAS->");
                return DM_BASE256; /* step (r)(2) */
            }
            cnt_1 = edf_count + DM_MULT_1;
            if (cnt_1 < ascii_count && cnt_1 < b256_count && cnt_1 < text_count && cnt_1 < x12_count
                    && cnt_1 < c40_count) {
                if (debug_print) printf("EDI->");
                return DM_EDIFACT; /* step (r)(3) */
            }
            cnt_1 = text_count + DM_MULT_1;
            if (cnt_1 < ascii_count && cnt_1 < b256_count && cnt_1 < edf_count && cnt_1 < x12_count
                    && cnt_1 < c40_count) {
                if (variant == 1) {
                    if (current_mode == DM_BASE256 && position + 6 < length) {
                        int text_sp_cnt = dm_text_sp_cnt(source, position, length, sp);
                        if (debug_print) printf("text_sp_cnt %d\n", text_sp_cnt);
                        if (text_sp_cnt >= 12) {
                            if (debug_print) printf("TEX->");
                            return DM_TEXT; /* step (r)(4) */
                        }
                    } else {
                        if (debug_print) printf("TEX->");
                        return DM_TEXT; /* step (r)(4) */
                    }
                } else {
                    if (debug_print) printf("TEX->");
                    return DM_TEXT; /* step (r)(4) */
                }
            }
            cnt_1 = x12_count + DM_MULT_1;
            if (cnt_1 < ascii_count && cnt_1 < b256_count && cnt_1 < edf_count && cnt_1 < text_count
                    && cnt_1 < c40_count) {
                if (debug_print) printf("X12->");
                return DM_X12; /* step (r)(5) */
            }
            cnt_1 = c40_count + DM_MULT_1;
            if (cnt_1 < ascii_count && cnt_1 < b256_count && cnt_1 < edf_count && cnt_1 < text_count) {
                if (c40_count < x12_count) {
                    if (debug_print) printf("C40->");
                    return DM_C40; /* step (r)(6)(i) */
                }
                if (c40_count == x12_count) {
                    if (dm_p_r_6_2_1(source, length, sp) == 1) {
                        if (debug_print) printf("X12->");
                        return DM_X12; /* step (r)(6)(ii)(I) */
                    }
                    if (debug_print) printf("C40->");
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
    if (debug_print) {
        printf("\nEOD(m:%d, p:%d, a:%d): ascii_rnded %d, b256_rnded %d, edf_rnded %d, text_rnded %d, x12_rnded %d (%d)"
                ", c40_rnded %d (%d) ",
                current_mode, position, mode_arg, ascii_rnded, b256_rnded, edf_rnded, text_rnded, x12_rnded, x12_count,
                c40_rnded, c40_count);
    }

    if (ascii_rnded <= b256_rnded && ascii_rnded <= edf_rnded && ascii_rnded <= text_rnded && ascii_rnded <= x12_rnded
            && ascii_rnded <= c40_rnded) {
        if (debug_print) printf("ASC->");
        return DM_ASCII; /* step (k)(2) */
    }
    if (b256_rnded < ascii_rnded && b256_rnded < edf_rnded && b256_rnded < text_rnded && b256_rnded < x12_rnded
            && b256_rnded < c40_rnded) {
        if (debug_print) printf("BAS->");
        return DM_BASE256; /* step (k)(3) */
    }
    if (variant == 1) {
        edf_eod_test = (edf_rnded < ascii_rnded && edf_rnded < b256_rnded && edf_rnded < text_rnded && edf_rnded <= x12_rnded && edf_rnded < c40_rnded);
    } else {
        edf_eod_test = (edf_rnded < ascii_rnded && edf_rnded < b256_rnded && edf_rnded < text_rnded && edf_rnded < x12_rnded && edf_rnded < c40_rnded);
    }
    if (edf_eod_test) {
        if (debug_print) printf("EDI->");
        return DM_EDIFACT; /* step (k)(4) */
    }
    if (text_rnded < ascii_rnded && text_rnded < b256_rnded && text_rnded < edf_rnded && text_rnded < x12_rnded
            && text_rnded < c40_rnded) {
        if (debug_print) printf("TEX->");
        return DM_TEXT; /* step (k)(5) */
    }
    if (variant == 1) {
        x12_eod_test = (x12_rnded < ascii_rnded && x12_rnded < b256_rnded && x12_rnded <= edf_rnded && x12_rnded < text_rnded && x12_rnded < c40_rnded);
    } else {
        x12_eod_test = (x12_rnded < ascii_rnded && x12_rnded < b256_rnded && x12_rnded < edf_rnded && x12_rnded < text_rnded && x12_rnded < c40_rnded);
    }
    if (x12_eod_test) {
        if (debug_print) printf("X12->");
        return DM_X12; /* step (k)(6) */
    }
    /* Note the algorithm is particularly sub-optimal here, returning C40 even if X12/EDIFACT (much) better, due to
       the < comparisons of rounded X12/EDIFACT values to each other above - comparisons would need to be <= or
       unrounded (cf. very similar Code One algorithm). Not changed to maintain compatibility with spec and BWIPP */
    if (debug_print) printf("C40->");
    return DM_C40; /* step (k)(7) */
}

/* Copy C40/TEXT/X12 triplets from buffer to target. Returns elements left in buffer (< 3) */
static int dm_ctx_buffer_xfer(int process_buffer[8], int process_p, unsigned char target[], int *p_tp,
            const int debug_print) {
    int i, process_e;
    int tp = *p_tp;

    process_e = (process_p / 3) * 3;

    for (i = 0; i < process_e; i += 3) {
        int iv = (1600 * process_buffer[i]) + (40 * process_buffer[i + 1]) + (process_buffer[i + 2]) + 1;
        target[tp++] = (unsigned char) (iv >> 8);
        target[tp++] = (unsigned char) (iv & 0xFF);
        if (debug_print) {
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
static int dm_edi_buffer_xfer(int process_buffer[8], int process_p, unsigned char target[], int *p_tp,
            const int empty, const int debug_print) {
    int i, process_e;
    int tp = *p_tp;

    process_e = (process_p / 4) * 4;

    for (i = 0; i < process_e; i += 4) {
        target[tp++] = (unsigned char) (process_buffer[i] << 2 | (process_buffer[i + 1] & 0x30) >> 4);
        target[tp++] = (unsigned char) ((process_buffer[i + 1] & 0x0f) << 4 | (process_buffer[i + 2] & 0x3c) >> 2);
        target[tp++] = (unsigned char) ((process_buffer[i + 2] & 0x03) << 6 | process_buffer[i + 3]);
        if (debug_print) {
            printf("[%d %d %d %d (%d %d %d)] ", process_buffer[i], process_buffer[i + 1], process_buffer[i + 2],
                process_buffer[i + 3], target[tp - 3], target[tp - 2], target[tp - 1]);
        }
    }

    process_p -= process_e;

    if (process_p) {
        memmove(process_buffer, process_buffer + process_e, sizeof(int) * process_p);
        if (empty) {
            if (process_p == 3) {
                target[tp++] = (unsigned char) (process_buffer[i] << 2 | (process_buffer[i + 1] & 0x30) >> 4);
                target[tp++] = (unsigned char) ((process_buffer[i + 1] & 0x0f) << 4
                                                | (process_buffer[i + 2] & 0x3c) >> 2);
                target[tp++] = (unsigned char) ((process_buffer[i + 2] & 0x03) << 6);
                if (debug_print) {
                    printf("[%d %d %d (%d %d %d)] ", process_buffer[i], process_buffer[i + 1], process_buffer[i + 2],
                            target[tp - 3], target[tp - 2], target[tp - 1]);
                }
            } else if (process_p == 2) {
                target[tp++] = (unsigned char) (process_buffer[i] << 2 | (process_buffer[i + 1] & 0x30) >> 4);
                target[tp++] = (unsigned char) ((process_buffer[i + 1] & 0x0f) << 4);
                if (debug_print) {
                    printf("[%d %d (%d %d)] ", process_buffer[i], process_buffer[i + 1], target[tp - 2],
                            target[tp - 1]);
                }
            } else {
                target[tp++] = (unsigned char) (process_buffer[i] << 2);
                if (debug_print) printf("[%d (%d)] ", process_buffer[i], target[tp - 1]);
            }
            process_p = 0;
        }
    }

    *p_tp = tp;

    return process_p;
}

STATIC_UNLESS_ZINT_TEST int dm_get_symbolsize(struct zint_symbol *symbol, const int minimum);
STATIC_UNLESS_ZINT_TEST int dm_codewords_remaining(struct zint_symbol *symbol, const int tp, const int process_p);
STATIC_UNLESS_ZINT_TEST int dm_c40text_cnt(const int current_mode, const int gs1, unsigned char input);
STATIC_UNLESS_ZINT_TEST int dm_update_b256_field_length(unsigned char target[], int tp, int b256_start);
STATIC_UNLESS_ZINT_TEST int dm_switch_mode(const int next_mode, unsigned char target[], int tp, int *b256_start,
            const int debug_print);

/* Version of dm200encode() to check variant look ahead parameters */
static int dm200encode_variant(struct zint_symbol *symbol, const unsigned char source[], unsigned char target[],
        int *p_length, int *p_binlen, const int variant) {

    int sp;
    int tp, i, gs1;
    int current_mode, next_mode;
    int not_first = 0;
    int inputlen = *p_length;
    int process_buffer[8]; /* holds remaining data to finalised */
    int process_p = 0; /* number of characters left to finalise */
    int b256_start = 0;
    int symbols_left;
    const int debug_print = symbol->debug & ZINT_DEBUG_PRINT;

    sp = 0;
    tp = 0;

    if (symbol->structapp.count) {
        int id1, id2;

        if (symbol->structapp.count < 2 || symbol->structapp.count > 16) {
            strcpy(symbol->errtxt, "720: Structured Append count out of range (2-16)");
            return ZINT_ERROR_INVALID_OPTION;
        }
        if (symbol->structapp.index < 1 || symbol->structapp.index > symbol->structapp.count) {
            sprintf(symbol->errtxt, "721: Structured Append index out of range (1-%d)", symbol->structapp.count);
            return ZINT_ERROR_INVALID_OPTION;
        }
        if (symbol->structapp.id[0]) {
            int id, id_len, id1_err, id2_err;

            for (id_len = 0; id_len < 32 && symbol->structapp.id[id_len]; id_len++);

            if (id_len > 6) { /* ID1 * 1000 + ID2 */
                strcpy(symbol->errtxt, "722: Structured Append ID too long (6 digit maximum)");
                return ZINT_ERROR_INVALID_OPTION;
            }

            id = to_int((const unsigned char *) symbol->structapp.id, id_len);
            if (id == -1) {
                strcpy(symbol->errtxt, "723: Invalid Structured Append ID (digits only)");
                return ZINT_ERROR_INVALID_OPTION;
            }
            id1 = id / 1000;
            id2 = id % 1000;
            id1_err = id1 < 1 || id1 > 254;
            id2_err = id2 < 1 || id2 > 254;
            if (id1_err || id2_err) {
                if (id1_err && id2_err) {
                    sprintf(symbol->errtxt,
                            "724: Structured Append ID1 '%03d' and ID2 '%03d' out of range (001-254) (ID '%03d%03d')",
                            id1, id2, id1, id2);
                } else if (id1_err) {
                    sprintf(symbol->errtxt,
                            "725: Structured Append ID1 '%03d' out of range (001-254) (ID '%03d%03d')",
                            id1, id1, id2);
                } else {
                    sprintf(symbol->errtxt,
                            "726: Structured Append ID2 '%03d' out of range (001-254) (ID '%03d%03d')",
                            id2, id1, id2);
                }
                return ZINT_ERROR_INVALID_OPTION;
            }
        } else {
            id1 = id2 = 1;
        }

        target[tp++] = 233;
        target[tp++] = (17 - symbol->structapp.count) | ((symbol->structapp.index - 1) << 4);
        target[tp++] = id1;
        target[tp++] = id2;
    }

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
        target[tp++] = 232;
        if (debug_print) printf("FN1 ");
    } /* FNC1 */

    if (symbol->output_options & READER_INIT) {
        if (gs1) {
            strcpy(symbol->errtxt, "521: Cannot encode in GS1 mode and Reader Initialisation at the same time");
            return ZINT_ERROR_INVALID_OPTION;
        }
        if (symbol->structapp.count) {
            strcpy(symbol->errtxt, "727: Cannot have Structured Append and Reader Initialisation at the same time");
            return ZINT_ERROR_INVALID_OPTION;
        }
        target[tp++] = 234; /* Reader Programming */
        if (debug_print) printf("RP ");
    }

    if (symbol->eci > 0) {
        /* Encode ECI numbers according to Table 6 */
        target[tp++] = 241; /* ECI Character */
        if (symbol->eci <= 126) {
            target[tp++] = (unsigned char) (symbol->eci + 1);
        } else if (symbol->eci <= 16382) {
            target[tp++] = (unsigned char) ((symbol->eci - 127) / 254 + 128);
            target[tp++] = (unsigned char) ((symbol->eci - 127) % 254 + 1);
        } else {
            target[tp++] = (unsigned char) ((symbol->eci - 16383) / 64516 + 192);
            target[tp++] = (unsigned char) (((symbol->eci - 16383) / 254) % 254 + 1);
            target[tp++] = (unsigned char) ((symbol->eci - 16383) % 254 + 1);
        }
        if (debug_print) printf("ECI %d ", symbol->eci + 1);
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
            target[tp++] = 236;
            if (debug_print) printf("Macro05 ");
        } else {
            target[tp++] = 237;
            if (debug_print) printf("Macro06 ");
        }
        /* Remove macro characters from input string */
        sp = 7;
        inputlen -= 2;
        *p_length -= 2;
    }

    /* step (a) */
    current_mode = DM_ASCII;
    next_mode = DM_ASCII;

    while (sp < inputlen) {

        current_mode = next_mode;

        /* step (b) - ASCII encodation */
        if (current_mode == DM_ASCII) {
            next_mode = DM_ASCII;

            if (is_twodigits(source, inputlen, sp)) {
                target[tp++] = (unsigned char) ((10 * ctoi(source[sp])) + ctoi(source[sp + 1]) + 130);
                if (debug_print) printf("N%02d ", target[tp - 1] - 130);
                sp += 2;
            } else {
                next_mode = dm_look_ahead_test_variant(source, inputlen, sp, current_mode, 0, gs1, debug_print, variant);

                if (next_mode != DM_ASCII) {
                    tp = dm_switch_mode(next_mode, target, tp, &b256_start, debug_print);
                    not_first = 0;
                } else {
                    if (source[sp] & 0x80) {
                        target[tp++] = 235; /* FNC4 */
                        target[tp++] = (source[sp] - 128) + 1;
                        if (debug_print) printf("FN4 A%02X ", target[tp - 1] - 1);
                    } else {
                        if (gs1 && (source[sp] == '[')) {
                            if (gs1 == 2) {
                                target[tp++] = 29 + 1; /* GS */
                                if (debug_print) printf("GS ");
                            } else {
                                target[tp++] = 232; /* FNC1 */
                                if (debug_print) printf("FN1 ");
                            }
                        } else {
                            target[tp++] = source[sp] + 1;
                            if (debug_print) printf("A%02X ", target[tp - 1] - 1);
                        }
                    }
                    sp++;
                }
            }

        /* step (c)/(d) C40/TEXT encodation */
        } else if (current_mode == DM_C40 || current_mode == DM_TEXT) {

            next_mode = current_mode;
            if (process_p == 0 && not_first) {
                next_mode = dm_look_ahead_test_variant(source, inputlen, sp, current_mode, process_p, gs1, debug_print, variant);
            }

            if (next_mode != current_mode) {
                target[tp++] = 254; /* Unlatch */
                next_mode = DM_ASCII;
                if (debug_print) printf("ASC ");
            } else {
                int shift_set, value;
                const char *ct_shift, *ct_value;

                if (current_mode == DM_C40) {
                    ct_shift = dm_c40_shift;
                    ct_value = dm_c40_value;
                } else {
                    ct_shift = dm_text_shift;
                    ct_value = dm_text_value;
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
                    process_p = dm_ctx_buffer_xfer(process_buffer, process_p, target, &tp, debug_print);
                }
                sp++;
                not_first = 1;
            }

        /* step (e) X12 encodation */
        } else if (current_mode == DM_X12) {

            if (!dm_isX12(source[sp])) {
                next_mode = DM_ASCII;
            } else {
                next_mode = DM_X12;
                if (process_p == 0 && not_first) {
                    next_mode = dm_look_ahead_test_variant(source, inputlen, sp, current_mode, process_p, gs1, debug_print, variant);
                }
            }

            if (next_mode != DM_X12) {
                process_p = 0; /* Throw away buffer if any */
                target[tp++] = 254; /* Unlatch */
                next_mode = DM_ASCII;
                if (debug_print) printf("ASC ");
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
                    process_p = dm_ctx_buffer_xfer(process_buffer, process_p, target, &tp, debug_print);
                }
                sp++;
                not_first = 1;
            }

        /* step (f) EDIFACT encodation */
        } else if (current_mode == DM_EDIFACT) {

            if (!dm_isedifact(source[sp], gs1)) {
                next_mode = DM_ASCII;
            } else {
                next_mode = DM_EDIFACT;
                if (process_p == 3) {
                    /* Note different then spec Step (f)(1), which suggests checking when 0, but this seems to work
                       better in many cases as the switch to ASCII is "free" */
                    next_mode = dm_look_ahead_test_variant(source, inputlen, sp, current_mode, process_p, gs1, debug_print, variant);
                }
            }

            if (next_mode != DM_EDIFACT) {
                process_buffer[process_p++] = 31;
                process_p = dm_edi_buffer_xfer(process_buffer, process_p, target, &tp, 1 /*empty*/, debug_print);
                next_mode = DM_ASCII;
                if (debug_print) printf("ASC ");
            } else {
                int value = source[sp];

                if (value >= 64) { // '@'
                    value -= 64;
                }

                process_buffer[process_p++] = value;
                sp++;
                not_first = 1;

                if (process_p >= 4) {
                    process_p = dm_edi_buffer_xfer(process_buffer, process_p, target, &tp, 0 /*empty*/, debug_print);
                }
            }

        /* step (g) Base 256 encodation */
        } else if (current_mode == DM_BASE256) {

            if (gs1 == 1 && source[sp] == '[') {
                next_mode = DM_ASCII;
            } else {
                next_mode = DM_BASE256;
                if (not_first) {
                    next_mode = dm_look_ahead_test_variant(source, inputlen, sp, current_mode, tp - (b256_start + 1), gs1,
                                    debug_print, variant);
                }
            }

            if (next_mode != DM_BASE256) {
                tp = dm_update_b256_field_length(target, tp, b256_start);
                /* B.2.1 255-state randomising algorithm */
                for (i = b256_start; i < tp; i++) {
                    const int prn = ((149 * (i + 1)) % 255) + 1;
                    target[i] = (unsigned char) ((target[i] + prn) & 0xFF);
                }
                if (variant == 1) {
                    tp = dm_switch_mode(next_mode, target, tp, &b256_start, debug_print);
                    not_first = 0;
                } else {
                    next_mode = DM_ASCII;
                    if (debug_print) printf("ASC ");
                }
            } else {
                if (gs1 == 2 && source[sp] == '[') {
                    target[tp++] = 29; /* GS */
                } else {
                    target[tp++] = source[sp];
                }
                sp++;
                not_first = 1;
                if (debug_print) printf("B%02X ", target[tp - 1]);
            }
        }

        if (tp > 1558) {
            strcpy(symbol->errtxt, "520: Data too long to fit in symbol");
            return ZINT_ERROR_TOO_LONG;
        }

    } /* while */

    symbols_left = dm_codewords_remaining(symbol, tp, process_p);

    if (debug_print) printf("\nsymbols_left %d, process_p %d ", symbols_left, process_p);

    if (current_mode == DM_C40 || current_mode == DM_TEXT) {
        /* NOTE: changed to follow spec exactly here, only using Shift 1 padded triplets when 2 symbol chars remain.
           This matches the behaviour of BWIPP but not tec-it, nor figures 4.15.1-1 and 4.15-1-2 in GS1 General
           Specifications 21.0.1.
         */
        if (debug_print) printf("%s ", current_mode == DM_C40 ? "C40" : "TEX");
        if (process_p == 0) {
            if (symbols_left > 0) {
                target[tp++] = 254; // Unlatch
                if (debug_print) printf("ASC ");
            }
        } else {
            if (process_p == 2 && symbols_left == 2) {
                /* 5.2.5.2 (b) */
                process_buffer[process_p++] = 0; // Shift 1
                (void) dm_ctx_buffer_xfer(process_buffer, process_p, target, &tp, debug_print);

            } else if (process_p == 1 && symbols_left <= 2 && dm_isc40text(current_mode, source[inputlen - 1])) {
                /* 5.2.5.2 (c)/(d) */
                if (symbols_left > 1) {
                    /* 5.2.5.2 (c) */
                    target[tp++] = 254; // Unlatch and encode remaining data in ascii.
                    if (debug_print) printf("ASC ");
                }
                target[tp++] = source[inputlen - 1] + 1;
                if (debug_print) printf("A%02X ", target[tp - 1] - 1);

            } else {
                int cnt, total_cnt = 0;
                /* Backtrack to last complete triplet (same technique as BWIPP) */
                while (sp > 0 && process_p % 3) {
                    sp--;
                    cnt = dm_c40text_cnt(current_mode, gs1, source[sp]);
                    total_cnt += cnt;
                    process_p -= cnt;
                }
                tp -= (total_cnt / 3) * 2;

                target[tp++] = 254; // Unlatch
                if (debug_print) printf("ASC ");
                for (; sp < inputlen; sp++) {
                    if (is_twodigits(source, inputlen, sp)) {
                        target[tp++] = (unsigned char) ((10 * ctoi(source[sp])) + ctoi(source[sp + 1]) + 130);
                        if (debug_print) printf("N%02d ", target[tp - 1] - 130);
                        sp++;
                    } else if (source[sp] & 0x80) {
                        target[tp++] = 235; /* FNC4 */
                        target[tp++] = (source[sp] - 128) + 1;
                        if (debug_print) printf("FN4 A%02X ", target[tp - 1] - 1);
                    } else if (gs1 && source[sp] == '[') {
                        if (gs1 == 2) {
                            target[tp++] = 29 + 1; /* GS */
                            if (debug_print) printf("GS ");
                        } else {
                            target[tp++] = 232; /* FNC1 */
                            if (debug_print) printf("FN1 ");
                        }
                    } else {
                        target[tp++] = source[sp] + 1;
                        if (debug_print) printf("A%02X ", target[tp - 1] - 1);
                    }
                }
            }
        }

    } else if (current_mode == DM_X12) {
        if (debug_print) printf("X12 ");
        if ((symbols_left == 1) && (process_p == 1)) {
            // Unlatch not required!
            target[tp++] = source[inputlen - 1] + 1;
            if (debug_print) printf("A%02X ", target[tp - 1] - 1);
        } else {
            if (symbols_left > 0) {
                target[tp++] = (254); // Unlatch.
                if (debug_print) printf("ASC ");
            }

            if (process_p == 1) {
                target[tp++] = source[inputlen - 1] + 1;
                if (debug_print) printf("A%02X ", target[tp - 1] - 1);
            } else if (process_p == 2) {
                target[tp++] = source[inputlen - 2] + 1;
                target[tp++] = source[inputlen - 1] + 1;
                if (debug_print) printf("A%02X A%02X ", target[tp - 2] - 1, target[tp - 1] - 1);
            }
        }

    } else if (current_mode == DM_EDIFACT) {
        if (debug_print) printf("EDI ");
        if (symbols_left <= 2 && process_p <= symbols_left) { // Unlatch not required!
            if (process_p == 1) {
                target[tp++] = source[inputlen - 1] + 1;
                if (debug_print) printf("A%02X ", target[tp - 1] - 1);
            } else if (process_p == 2) {
                target[tp++] = source[inputlen - 2] + 1;
                target[tp++] = source[inputlen - 1] + 1;
                if (debug_print) printf("A%02X A%02X ", target[tp - 2] - 1, target[tp - 1] - 1);
            }
        } else {
            // Append edifact unlatch value (31) and empty buffer
            if (process_p <= 3) {
                process_buffer[process_p++] = 31;
            }
            (void) dm_edi_buffer_xfer(process_buffer, process_p, target, &tp, 1 /*empty*/, debug_print);
        }

    } else if (current_mode == DM_BASE256) {
        if (symbols_left > 0) {
            tp = dm_update_b256_field_length(target, tp, b256_start);
        }
        /* B.2.1 255-state randomising algorithm */
        for (i = b256_start; i < tp; i++) {
            int prn = ((149 * (i + 1)) % 255) + 1;
            target[i] = (unsigned char) ((target[i] + prn) & 0xFF);
        }
    }

    if (debug_print) {
        printf("\nData (%d): ", tp);
        for (i = 0; i < tp; i++)
            printf("%d ", target[i]);

        printf("\n");
    }

    *p_binlen = tp;

    return 0;
}

#endif /* Z_DMATRIX_VARIANT_H */
