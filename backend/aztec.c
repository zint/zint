/* aztec.c - Handles Aztec 2D Symbols */
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

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "common.h"
#include "aztec.h"
#include "reedsol.h"

#define AZTEC_MAX_CAPACITY  19968 /* ISO/IEC 24778:2008 5.3 Table 1 Maximum Symbol Bit Capacity */
/* Allow up to absolute minimum 3 ECC codewords, but now warn if results in less than the 5% minimum (ISO/IEC
   24778:2008 4.1.e) - previously could go down to 3 ECC codewords anyway if version given, due to bit-stuffing */
#define AZTEC_BIN_CAPACITY  19932 /* AZTEC_MAX_CAPACITY less 3 * 12 = 36 */
#define AZTEC_MAP_SIZE      22801 /* AztecMap Version 32 151 x 151 */
#define AZTEC_MAP_POSN_MAX  20039 /* Maximum position index in AztecMap */

#define AZ_BIN_CAP_CWDS_S   "1661" /* String version of (AZTEC_BIN_CAPACITY / 12) */

/* Count number of consecutive (. SP) or (, SP) Punct mode doubles for comparison against Digit mode encoding */
static int az_count_doubles(const unsigned char source[], const int length, const int position) {
    int i;

    for (i = position; i + 1 < length && (source[i] == '.' || source[i] == ',') && source[i + 1] == ' '; i += 2);

    return (i - position) >> 1;
}

/* Count number of consecutive full stops or commas (can be encoded in Punct or Digit mode) */
static int az_count_dotcomma(const unsigned char source[], const int length, const int position) {
    int i;

    for (i = position; i < length && (source[i] == '.' || source[i] == ','); i++);

    return i - position;
}

/* Count number of consecutive `chr`s */
static int az_count_chr(const unsigned char source[], const int length, const int position, const unsigned char chr) {
    int i;

    for (i = position; i < length && source[i] == chr; i++);

    return i - position;
}

/* Return mode following current, or AZ_E if none */
static char az_get_next_mode(const char modes[], const int length, int i) {
    const char current_mode = AZ_MASK(modes[i]);

    do {
        i++;
    } while (i < length && AZ_MASK(modes[i]) == current_mode);

    if (i >= length) {
        return AZ_E;
    }
    return AZ_MASK(modes[i]);
}

#define AZ_DOUBLE_PUNCT_NO_LEN_CHECK(s, i) \
    (((s)[i] == '\r' && (s)[(i) + 1] == '\n') \
    || ((s)[(i) + 1] == ' ' && ((s)[i] == '.' || (s)[i] == ',' || (s)[i] == ':')))

#define AZ_DOUBLE_PUNCT(s, l, i) ((i) + 1 < (l) && AZ_DOUBLE_PUNCT_NO_LEN_CHECK(s, i))

/* Reduce two letter combinations to one codeword marked as [abcd] in Punct mode */
static int az_reduce(char *modes, unsigned char *source, const int length) {
    int i = 0, j = 0;

    while (i + 1 < length) {
        modes[j] = modes[i];
        if ((modes[i] == AZ_P || (modes[i] & AZ_PS)) && AZ_DOUBLE_PUNCT_NO_LEN_CHECK(source, i)) {
            if (source[i] == '\r') {
                source[j] = 'a';
            } else if (source[i] == '.') {
                source[j] = 'b';
            } else if (source[i] == ',') {
                source[j] = 'c';
            } else {
                source[j] = 'd';
            }
            i += 2;
        } else {
            source[j] = source[i++];
        }
        j++;
    }
    if (i < length) {
        modes[j] = modes[i];
        source[j++] = source[i];
    }

    return j;
}

/* Return mapped mode */
static char az_mode_char(const char mode) {
	/* Same order as AZ_U, AZ_L etc */
	static const char mode_chars[] = { '?', 'U', 'L', 'M', 'P', 'D', 'B', 'X', 'E' };
    char ch;

    assert(ARRAY_SIZE(mode_chars) == AZ_E + 1);

    if (mode & AZ_US) {
        assert(AZ_MASK(mode) == AZ_L || AZ_MASK(mode) == AZ_D);
        return AZ_MASK(mode) == AZ_L ? 'r' : 't';
    }
    assert(AZ_MASK(mode) < ARRAY_SIZE(mode_chars));

    ch = mode_chars[AZ_MASK(mode)];
    if (mode & AZ_PS) {
        assert(AZ_MASK(mode) != AZ_P);
        return ch | 0x20; /* Make lower case */
    }
    return ch;
}

/* Print out the modes */
static void az_print_modes(const char *modes, const int length) {
    int i;
    for (i = 0; i < length; i++) {
        fputc(az_mode_char(modes[i]), stdout);
    }
    fputc('\n', stdout);
}

/* Determine encoding modes using modified Annex H algorithm (`FAST_MODE`) */
static int az_text_modes(char modes[], unsigned char source[], int length, const int gs1, const char initial_mode,
            const int debug_print) {
    int i;
    char current_mode;
    int count;
    char next_mode;
    int reduced_length;

    for (i = 0; i < length; i++) {
        if (!z_isascii(source[i]) || !AztecFlags[source[i]]) {
            modes[i] = AZ_B;
        } else if (gs1 && source[i] == '\x1D') {
            modes[i] = AZ_P; /* For FLG(n) & FLG(0) = FNC1 */
        } else {
            modes[i] = AztecModes[source[i]];
        }
    }

    /* Deal first with letter combinations which can be combined to one codeword
       Combinations are (CR LF) (. SP) (, SP) (: SP) in Punct mode */
    current_mode = initial_mode;
    for (i = 0; i + 1 < length; i++) {
        /* Combination (CR LF) should always be in Punct mode */
        if (source[i] == '\r' && source[i + 1] == '\n') {
            modes[i] = AZ_P;
            modes[i + 1] = AZ_P;

        /* Combination (: SP) should always be in Punct mode */
        } else if (source[i] == ':' && source[i + 1] == ' ') {
            modes[i + 1] = AZ_P;

        /* Combinations (. SP) and (, SP) sometimes use fewer bits in Digit mode */
        } else if ((source[i] == '.' || source[i] == ',') && source[i + 1] == ' ' && modes[i] == AZ_X) {
            count = az_count_doubles(source, length, i);
            next_mode = az_get_next_mode(modes, length, i);

            if (current_mode == AZ_U) {
                if (next_mode == AZ_D && count <= 5) {
                    memset(modes + i, AZ_D, 2 * count);
                }

            } else if (current_mode == AZ_L) {
                if (next_mode == AZ_D && count <= 4) {
                    memset(modes + i, AZ_D, 2 * count);
                }

            } else if (current_mode == AZ_M) {
                if (next_mode == AZ_D && count == 1) {
                    modes[i] = AZ_D;
                    modes[i + 1] = AZ_D;
                }

            } else if (current_mode == AZ_D) {
                if (next_mode != AZ_D && count <= 4) {
                    memset(modes + i, AZ_D, 2 * count);
                } else if (next_mode == AZ_D && count <= 7) {
                    memset(modes + i, AZ_D, 2 * count);
                }
            }

            /* Default is Punct mode */
            if (modes[i] == AZ_X) {
                modes[i] = AZ_P;
                modes[i + 1] = AZ_P;
            }
        }

        if (modes[i] != AZ_X && modes[i] != AZ_B) {
            current_mode = modes[i];
        }
    }

    if (debug_print) {
        printf("Initial Mode: %c\nFirst Pass (%d):\n", az_mode_char(initial_mode), length);
        az_print_modes(modes, length);
    }

    /* Reduce two letter combinations to one codeword marked as [abcd] in Punct mode */
    reduced_length = az_reduce(modes, source, length);
    assert(reduced_length > 0);

    current_mode = initial_mode;
    for (i = 0; i < reduced_length; i++) {
        if (modes[i] == AZ_B) {
            if (current_mode == AZ_D || current_mode == AZ_P) {
                current_mode = AZ_U;
            }
            continue;
        }
        /* Resolve Carriage Return (CR) which can be Punct or Mixed mode */
        if (source[i] == '\r') {
            count = az_count_chr(source, reduced_length, i, '\r');
            next_mode = az_get_next_mode(modes, reduced_length, i);

            if (current_mode == AZ_U && (next_mode == AZ_U || next_mode == AZ_B) && count == 1) {
                modes[i] = AZ_P;

            } else if (current_mode == AZ_L && (next_mode == AZ_L || next_mode == AZ_B) && count == 1) {
                modes[i] = AZ_P;

            } else if (current_mode == AZ_P || next_mode == AZ_P) {
                modes[i] = AZ_P;
            }

            if (current_mode == AZ_D) {
                if ((next_mode == AZ_E || next_mode == AZ_U || next_mode == AZ_D || next_mode == AZ_B)
                        && count <= 2) {
                    memset(modes + i, AZ_P, count);
                } else if (next_mode == AZ_L && count == 1) {
                    modes[i] = AZ_P;
                }
            }

            /* Default is Mixed mode */
            if (modes[i] == AZ_X) {
                modes[i] = AZ_M;
            }

        /* Resolve full stop and comma which can be in Punct or Digit mode */
        } else if (source[i] == '.' || source[i] == ',') {
            count = az_count_dotcomma(source, reduced_length, i);
            next_mode = az_get_next_mode(modes, reduced_length, i);

            if (current_mode == AZ_U) {
                if ((next_mode == AZ_U || next_mode == AZ_L || next_mode == AZ_M || next_mode == AZ_B)
                        && count == 1) {
                    modes[i] = AZ_P;
                }

            } else if (current_mode == AZ_L) {
                if (next_mode == AZ_L && count <= 2) {
                    memset(modes + i, AZ_P, count);
                } else if ((next_mode == AZ_M || next_mode == AZ_B) && count == 1) {
                    modes[i] = AZ_P;
                }

            } else if (current_mode == AZ_M) {
                if ((next_mode == AZ_E || next_mode == AZ_U || next_mode == AZ_L || next_mode == AZ_M)
                        && count <= 4) {
                    memset(modes + i, AZ_P, count);
                } else if (next_mode == AZ_B && count <= 2) {
                    memset(modes + i, AZ_P, count);
                }

            } else if (current_mode == AZ_P && next_mode != AZ_D && count <= 9) {
                memset(modes + i, AZ_P, count);
            }

            /* Default is Digit mode */
            if (modes[i] == AZ_X) {
                modes[i] = AZ_D;
            }

        /* Resolve Space (SP) which can be any mode except Punct */
        } else if (source[i] == ' ') {
            count = az_count_chr(source, reduced_length, i, ' ');
            next_mode = az_get_next_mode(modes, reduced_length, i);

            if (current_mode == AZ_U) {
                if (next_mode == AZ_E && count <= 5) {
                    memset(modes + i, AZ_U, count);
                } else if ((next_mode == AZ_U || next_mode == AZ_L || next_mode == AZ_M || next_mode == AZ_P
                            || next_mode == AZ_B) && count <= 9) {
                    memset(modes + i, AZ_U, count);
                }

            } else if (current_mode == AZ_L) {
                if (next_mode == AZ_E && count <= 5) {
                    memset(modes + i, AZ_L, count);

                } else if (next_mode == AZ_U && count == 1) {
                    modes[i] = AZ_L;

                } else if (next_mode == AZ_L && count <= 14) {
                    memset(modes + i, AZ_L, count);

                } else if ((next_mode == AZ_M || next_mode == AZ_P || next_mode == AZ_B) && count <= 9) {
                    memset(modes + i, AZ_L, count);
                }

            } else if (current_mode == AZ_M) {
                if ((next_mode == AZ_E || next_mode == AZ_U) && count <= 9) {
                    memset(modes + i, AZ_M, count);

                } else if ((next_mode == AZ_L || next_mode == AZ_B) && count <= 14) {
                    memset(modes + i, AZ_M, count);

                } else if ((next_mode == AZ_M || next_mode == AZ_P) && count <= 19) {
                    memset(modes + i, AZ_M, count);
                }

            } else if (current_mode == AZ_P) {
                if (next_mode == AZ_E && count <= 5) {
                    memset(modes + i, AZ_U, count);

                } else if ((next_mode == AZ_U || next_mode == AZ_L || next_mode == AZ_M || next_mode == AZ_P
                            || next_mode == AZ_B) && count <= 9) {
                    memset(modes + i, AZ_U, count);
                }
            }

            /* Default is Digit mode */
            if (modes[i] == AZ_X) {
                modes[i] = AZ_D;
            }
        }

        current_mode = modes[i];
    }

    /* Decide when to use P/S instead of P/L and U/S instead of U/L */
    current_mode = initial_mode;
    for (i = 0; i < reduced_length; i++) {

        if (modes[i] == AZ_B) {
            if (current_mode == AZ_D || current_mode == AZ_P) {
                current_mode = AZ_U;
            }
            continue;
        }
        if (modes[i] != current_mode) {

            for (count = 0; i + count < reduced_length && modes[i + count] == modes[i]; count++);
            next_mode = az_get_next_mode(modes, reduced_length, i);

            if (modes[i] == AZ_P) {
                if (current_mode == AZ_U && count <= 2) {
                    memset(modes + i, AZ_U_PS, count);

                } else if (current_mode == AZ_L && next_mode != AZ_U && count <= 2) {
                    memset(modes + i, AZ_L_PS, count);

                } else if (current_mode == AZ_L && next_mode == AZ_U && count == 1) {
                    modes[i] = AZ_L_PS;

                } else if (current_mode == AZ_M && next_mode != AZ_M && count == 1) {
                    modes[i] = AZ_M_PS;

                } else if (current_mode == AZ_M && next_mode == AZ_M && count <= 2) {
                    memset(modes + i, AZ_M_PS, count);

                } else if (current_mode == AZ_D && next_mode != AZ_D && count <= 3) {
                    memset(modes + i, AZ_D_PS, count);

                } else if (current_mode == AZ_D && next_mode == AZ_D && count <= 6) {
                    memset(modes + i, AZ_D_PS, count);
                }

            } else if (modes[i] == AZ_U) {
                if (current_mode == AZ_L && (next_mode == AZ_L || next_mode == AZ_M) && count <= 2) {
                    memset(modes + i, AZ_L_US, count);

                } else if (current_mode == AZ_L && (next_mode == AZ_E || next_mode == AZ_D || next_mode == AZ_B
                                                    || next_mode == AZ_P) && count == 1) {
                    modes[i] = AZ_L_US;

                } else if (current_mode == AZ_D && next_mode == AZ_D && count == 1) {
                    modes[i] = AZ_D_US;

                } else if (current_mode == AZ_D && next_mode == AZ_P && count <= 2) {
                    memset(modes + i, AZ_D_US, count);
                }
            }
        }

        current_mode = AZ_MASK(modes[i]);
    }

    if (debug_print) {
        printf("Final Pass (%d):\n%.*s\n", reduced_length, reduced_length, source);
        az_print_modes(modes, reduced_length);
    }

    return reduced_length;
}

/* Cheapo to check if input all of one type of Byte-only, Upper, Lower or Digit, returning AZ_B, AZ_U, AZ_L or AZ_D
   resp., or 0 if not */
static char az_all_byte_only_or_uld(const unsigned char source[], const int length) {
    int i;
    int byte_only, upper, lower, digit;

    for (i = 0, byte_only = 0; i < length; i++) {
        byte_only += !z_isascii(source[i]) || !AztecFlags[source[i]];
    }
    if (byte_only) {
        return byte_only == length ? AZ_B : 0;
    }
    for (i = 0, upper = 0, lower = 0, digit = 0; i < length; i++) {
        upper += !!(AztecFlags[source[i]] & AZ_U_F);
        lower += !!(AztecFlags[source[i]] & AZ_L_F);
        /* Dot, comma & space only non-digit AZ_D_F, exclude in case they're AZ_P doubles */
        digit += z_isdigit(source[i]);
    }
    return upper == length ? AZ_U : lower == length ? AZ_L : digit == length ? AZ_D : 0;
}

/* Count number of initial consecutive punct chars (with no special treatment of multi-mode chars, unlike
   following `az_count_punct()`), and assuming not GS1_MODE */
static int az_count_initial_puncts(const unsigned char source[], const int length) {
    int i;

    for (i = 0; i < length; i++) {
        if (AZ_DOUBLE_PUNCT(source, length, i)) {
            i++;
        } else if (!z_isascii(source[i]) || !(AztecFlags[source[i]] & AZ_P_F)) {
            break;
        }
    }
    return i;
}

/* Count number of consecutive punct chars, treating multi-modes CR (without LF), dot and comma singularly */
static int az_count_punct(const unsigned char source[], const int length, const int position, const int gs1,
            int *begins_double) {
    const unsigned ch = source[position];

    assert(z_isascii(ch));

    *begins_double = AZ_DOUBLE_PUNCT(source, length, position);

    if (!*begins_double && (ch == '\r' || ch == '.' || ch == ',')) {
        return 1;
    }
    if ((gs1 && ch == '\x1D') || (AztecFlags[ch] & AZ_P_F)) {
        int i;
        if (gs1) {
            for (i = position + 1 + *begins_double; i < length && z_isascii(source[i]); i++) {
                if (AZ_DOUBLE_PUNCT(source, length, i)) {
                    i++;
                } else if (source[i] != '\x1D' && (!(AztecFlags[source[i]] & AZ_P_F)
                                                    || source[i] == '\r' || source[i] == '.' || source[i] == ',')) {
                    break;
                }
            }
            return i - position;
        } else {
            for (i = position + 1 + *begins_double; i < length && z_isascii(source[i]); i++) {
                if (AZ_DOUBLE_PUNCT(source, length, i)) {
                    i++;
                } else if (!(AztecFlags[source[i]] & AZ_P_F)
                            || source[i] == '\r' || source[i] == '.' || source[i] == ',') {
                    break;
                }
            }
            return i - position;
        }
    }

    return 0;
}

/* Count number of consecutive Upper chars, treating multi-mode space singularly */
static int az_count_upper(const unsigned char source[], const int length, const int position) {
    int i;

    if (source[position] == ' ') {
        return 1;
    }
    for (i = position; i < length && source[i] != ' ' && z_isascii(source[i]) && (AztecFlags[source[i]] & AZ_U_F);
            i++);

    return i - position;
}

/* Count number of consecutive Lower chars, treating multi-mode space singularly */
static int az_count_lower(const unsigned char source[], const int length, const int position) {
    int i;

    if (source[position] == ' ') {
        return 1;
    }
    for (i = position; i < length && source[i] != ' ' && z_isascii(source[i]) && (AztecFlags[source[i]] & AZ_L_F);
            i++);

    return i - position;
}

/* Count number of consecutive Mixed chars, treating multi-mode CR singularly */
static int az_count_mixed(const unsigned char source[], const int length, const int position, const int gs1) {
    int i;

    if (source[position] == '\r') {
        return 1;
    }
    for (i = position; i < length && z_isascii(source[i]) && (AztecFlags[source[i]] & AZ_M_F)
                        && (!gs1 || source[i] != '\x1D') && source[i] != '\r'; i++);

    return i - position;
}

/* Count number of consecutive Digit chars, treating multi-modes dot, comma and space singularly */
static int az_count_digit(const unsigned char source[], const int length, const int position) {
    int i;

    if (source[position] == '.' || source[position] == ',' || source[position] == ' ') {
        return 1;
    }
    for (i = position; i < length && z_isdigit(source[i]); i++); /* Dot, comma & space only non-digit AZ_D_F */

    return i - position;
}

/* Count number of consecutive Byte-only chars */
static int az_count_byte_only(const unsigned char source[], const int length, const int position) {
    int i;

    for (i = position; i < length && (!z_isascii(source[i]) || !AztecFlags[source[i]]); i++);

    return i - position;
}

/* Bit-size of encoding punct chars */
static int az_punct_size(const unsigned char source[], const int len, const int gs1) {
    int i;
    int cnt_doubles = 0;
    int cnt_fnc1s = 0;
    int size;

    if (gs1) {
        for (i = 0; i < len; i++) {
            cnt_doubles += AZ_DOUBLE_PUNCT(source, len, i);
            cnt_fnc1s += (source[i] == '\x1D');
        }
    } else {
        for (i = 0; i < len; i++) {
            cnt_doubles += AZ_DOUBLE_PUNCT(source, len, i);
        }
    }
    size = 5 * (len - cnt_doubles - cnt_fnc1s) + 8 * cnt_fnc1s;

    return size;
}

struct az_edge {
    unsigned char mode;
    unsigned char startMode; /* For Byte edges */
    unsigned short from; /* Position in input data, 0-based */
    unsigned short len;
    unsigned short size; /* Cumulative number of bits */
    unsigned short bytes; /* Byte count for AZ_X */
    unsigned short previous; /* Index into edges array */
};

/* Note 1st row of edges not used so valid previous cannot point there, i.e. won't be zero */
#define AZ_PREVIOUS(edges, edge) \
    ((edge)->previous ? (edges) + (edge)->previous : NULL)

#if 0
#include "aztec_trace.h"
#else
#define AZ_TRACE_Edges(px, s, l, im, p, v)
#define AZ_TRACE_AddEdge(s, l, es, p, v, e) do { (void)(s); (void)(l); } while (0)
#define AZ_TRACE_NotAddEdge(s, l, es, p, v, ij, e) do { (void)(s); (void)(l); } while (0)
#endif

/* Initialize a new edge. */
static void az_new_Edge(const struct az_edge *edges, const char mode, const unsigned char *source, const int from,
            const int len, const int gs1, const int initial_mode, const struct az_edge *previous,
            struct az_edge *edge) {

    /* Bit-size of switching modes from row to col */
    static const unsigned int switch_sizes[AZ_NUM_MODES - 1][AZ_NUM_MODES] = {
        /*          U      L      M          P      D          B */
        /*U*/ {     0,     5,     5,     5 + 5,     5,     5 + 5, },
        /*L*/ { 5 + 4,     0,     5,     5 + 5,     5,     5 + 5, },
        /*M*/ {     5,     5,     0,         5, 5 + 5,     5 + 5, },
        /*P*/ {     5, 5 + 5, 5 + 5,         0, 5 + 5, 5 + 5 + 5, },
        /*D*/ {     4, 4 + 5, 4 + 5, 4 + 5 + 5,     0, 4 + 5 + 5, },
        /*B - not used*/
    };
    const int mask_mode = AZ_MASK(mode);
    int previousMode;
    int previousStartMode;

    assert(len > 0);

    edge->mode = mode;
    edge->from = from;
    edge->len = len;
    if (previous) {
        edge->size = previous->size;
        edge->previous = previous - edges;
        previousMode = AZ_MASK(previous->mode);
        previousStartMode = previous->startMode;
    } else {
        edge->size = 0;
        edge->previous = 0;
        previousMode = AZ_MASK(initial_mode);
        previousStartMode = previousMode;
    }

    switch (mask_mode) {
        case AZ_U:
        case AZ_L:
        case AZ_M:
            if (mode & AZ_PS) {
                assert(len <= 2);
                edge->size += 5 /*P/S*/ + az_punct_size(source + from, len, gs1);
            } else if (mode & AZ_US) {
                assert(len == 1);
                assert(mask_mode == AZ_L);
                edge->size += 5 /*U/S*/ + 5;
            } else {
                edge->size += 5 * len;
            }
            edge->size += switch_sizes[(previousMode == AZ_B ? previousStartMode : previousMode) - 1][mask_mode - 1];
            edge->startMode = mask_mode;
            break;
        case AZ_P:
            assert(mask_mode == mode);
            edge->size += az_punct_size(source + from, len, gs1);
            edge->size += switch_sizes[(previousMode == AZ_B ? previousStartMode : previousMode) - 1][mask_mode - 1];
            edge->startMode = AZ_U;
            break;
        case AZ_D:
            if (mode & AZ_PS) {
                assert(len <= 2);
                edge->size += 4 /*P/S*/ + az_punct_size(source + from, len, gs1);
            } else if (mode & AZ_US) {
                assert(len == 1);
                edge->size += 4 /*U/S*/ + 5;
            } else {
                edge->size += 4 * len;
            }
            edge->size += switch_sizes[(previousMode == AZ_B ? previousStartMode : previousMode) - 1][mask_mode - 1];
            edge->startMode = AZ_U;
            break;
        case AZ_B:
            assert(mask_mode == mode);
            if (previousMode != mask_mode) {
                edge->size += switch_sizes[previousMode - 1][mask_mode - 1];
                if (len > 31) {
                    edge->size += 11;
                }
                edge->bytes = len;
            } else {
                if (len + previous->bytes > 31) {
                    if (previous->bytes <= 31) {
                        /* Note this may be sub-optimal as not taken into account when previous edge added */
                        edge->size += 11;
                    }
                }
                edge->bytes = len + previous->bytes;
            }
            edge->size += 8 * len;
            edge->startMode = previousStartMode;
            break;
    }
    assert(edge->startMode && AZ_MASK(edge->startMode) == edge->startMode && edge->startMode != AZ_B);
}

/* Add an edge for a mode at a vertex if no existing edge or if more optimal than existing edge */
static void az_addEdge(const unsigned char *source, const int length, struct az_edge *edges, const char mode,
            const int from, const int len, const int gs1, const int initial_mode, struct az_edge *previous) {
    struct az_edge edge;
    const int vertexIndex = from + len;
    const int v_ij = vertexIndex * AZ_NUM_MODES + AZ_MASK(mode) - 1;

    az_new_Edge(edges, mode, source, from, len, gs1, initial_mode, previous, &edge);

    if (edges[v_ij].mode == 0 || edges[v_ij].size > edge.size) {
        AZ_TRACE_AddEdge(source, length, edges, previous, vertexIndex, &edge);
        edges[v_ij] = edge;
    } else {
        AZ_TRACE_NotAddEdge(source, length, edges, previous, vertexIndex, v_ij, &edge);
    }
}

/* Add edges for the various modes at a vertex */
static void az_addEdges(const unsigned char source[], const int length, const int gs1, const int initial_mode,
            struct az_edge *edges, const int from, struct az_edge *previous) {
    const unsigned char ch = source[from];
    int len;

    if (z_isascii(ch) && AztecFlags[ch]) {
        int begins_double;

        if ((len = az_count_punct(source, length, from, gs1, &begins_double))) {
            az_addEdge(source, length, edges, AZ_P, from, len, gs1, initial_mode, previous);

            az_addEdge(source, length, edges, AZ_U_PS, from, 1 + begins_double, gs1, initial_mode, previous);
            az_addEdge(source, length, edges, AZ_L_PS, from, 1 + begins_double, gs1, initial_mode, previous);
            if (ch != '\r') {
                az_addEdge(source, length, edges, AZ_M_PS, from, 1 + begins_double, gs1, initial_mode, previous);
            }
            if (ch != '.' && ch != ',') {
                az_addEdge(source, length, edges, AZ_D_PS, from, 1 + begins_double, gs1, initial_mode, previous);
            }
        }
        if ((len = az_count_upper(source, length, from))) {
            az_addEdge(source, length, edges, AZ_U, from, len, gs1, initial_mode, previous);
        }
        if ((len = az_count_lower(source, length, from))) {
            az_addEdge(source, length, edges, AZ_L, from, len, gs1, initial_mode, previous);
        }
        if ((len = az_count_mixed(source, length, from, gs1))) {
            az_addEdge(source, length, edges, AZ_M, from, len, gs1, initial_mode, previous);
        }
        if ((len = az_count_digit(source, length, from))) {
            az_addEdge(source, length, edges, AZ_D, from, len, gs1, initial_mode, previous);
        }

        if (z_isupper(ch)) { /* Space only non-upper AZ_U_F */
            az_addEdge(source, length, edges, AZ_L_US, from, 1, gs1, initial_mode, previous);
            az_addEdge(source, length, edges, AZ_D_US, from, 1, gs1, initial_mode, previous);
        }
    }

    if (!gs1 || ch != '\x1D') {
        if (ch == '\r' || ch == ' ' || ch == '.' || ch == ',') { /* Multi-mode chars */
            len = 1;
        } else {
            len = 1 + az_count_byte_only(source, length, from + 1);
        }
        az_addEdge(source, length, edges, AZ_B, from, len, gs1, initial_mode, previous);
    }
}

/* Default, close to optimal encoding, using Dijkstra-based algorithm adapted from Data Matrix one by Alex Geller.
   Note that a bitstream that is encoded to be shortest based on mode choices may not be so after bit-stuffing */
static int az_define_modes(char modes[], unsigned char source[], const int length, const int gs1,
            const char initial_mode, const int debug_print) {
    int i, j, v_i;
    int minimalJ, minimalSize;
    struct az_edge *edge;
    int mode_end, mode_len;
    int reduced_length;
    struct az_edge *edges;

    if ((length + 1) * AZ_NUM_MODES > USHRT_MAX
            || !(edges = (struct az_edge *) calloc((length + 1) * AZ_NUM_MODES, sizeof(struct az_edge)))) {
        return 0;
    }
    az_addEdges(source, length, gs1, initial_mode, edges, 0, NULL);

    AZ_TRACE_Edges("DEBUG Initial situation\n", source, length, initial_mode, edges, 0);

    assert(length > 0); /* Suppress clang-tidy clang-analyzer-security.ArrayBound warning */

    for (i = 1; i < length; i++) {
        v_i = i * AZ_NUM_MODES;
        for (j = 0; j < AZ_NUM_MODES; j++) {
            if (edges[v_i + j].mode) {
                az_addEdges(source, length, gs1, initial_mode, edges, i, edges + v_i + j);
            }
        }
        AZ_TRACE_Edges("DEBUG situation after adding edges to vertices at position %d\n", source, length,
                        initial_mode, edges, i);
    }

    AZ_TRACE_Edges("DEBUG Final situation\n", source, length, initial_mode, edges, length);

    #if 0
    {
        const int fw = length * AZ_NUM_MODES > 100 ? 3 : 2;
        fputs("Dump of edges:\n", stdout);
        for (i = 0; i < length + 1; i++) {
            v_i = i * AZ_NUM_MODES;
            for (j = 0; j < AZ_NUM_MODES; j++) {
                fprintf(stdout, " %*d(%02X,%*d,%*d,%*d)",
                        fw, v_i + j, edges[v_i + j].mode, fw, edges[v_i + j].len, fw, edges[v_i + j].size,
                        fw, edges[v_i + j].previous);
            }
            fputc('\n', stdout);
        }
        fputc('\n', stdout);
    }
    #endif

    v_i = length * AZ_NUM_MODES;
    minimalJ = -1;
    minimalSize = INT_MAX;
    for (j = 0; j < AZ_NUM_MODES; j++) {
        edge = edges + v_i + j;
        if (edge->mode) {
            if (debug_print) {
                printf("edges[%d][%d][0] size %d\n", length, j, edge->size);
            }
            if (edge->size < minimalSize) {
                minimalSize = edge->size;
                minimalJ = j;
                if (debug_print) printf(" set minimalJ %d\n", minimalJ);
            }
        } else {
            if (debug_print) printf("edges[%d][%d][0] NULL\n", length, j);
        }
    }
    assert(minimalJ >= 0);

    edge = edges + v_i + minimalJ;
    mode_len = 0;
    mode_end = length;
    while (edge) {
        const char current_mode = edge->mode;
        mode_len += edge->len;
        edge = AZ_PREVIOUS(edges, edge);
        if (!edge || edge->mode != current_mode) {
            for (i = mode_end - mode_len; i < mode_end; i++) {
                modes[i] = current_mode;
            }
            mode_end = mode_end - mode_len;
            mode_len = 0;
        }
    }

    reduced_length = az_reduce(modes, source, length);

    if (debug_print) {
        printf("  Modes (%d):\n", reduced_length);
        az_print_modes(modes, reduced_length);
    }
    assert(mode_end == 0);

    free(edges);

    return reduced_length;
}

/* Calculate the binary size */
static int az_text_size(const char *modes, const unsigned char *source, int length, const int gs1, const int set_gs1,
            const int eci, const char initial_mode, const int eci_latch, int *byte_counts) {
    int i;
    int byte_i = 0;
    char current_mode;
    int size = 0;

    if (set_gs1 && gs1) {
        size += 5 + 5 + 3;
    }
    if (eci != 0) {
        if (initial_mode != AZ_P) {
            if (eci_latch) {
                if (initial_mode != AZ_M) {
                    if (initial_mode == AZ_D) {
                        size += 4;
                    }
                    size += 5;
                }
                size += 5;
            } else {
                size += initial_mode == AZ_D ? 4 : 5;
            }
        }
        size += 5 + 3 + 4 + 4 * ((eci > 9) + (eci > 99) + (eci > 999) + (eci > 9999) + (eci > 99999));
    }
    current_mode = eci_latch ? AZ_P : initial_mode;
    for (i = 0; i < length; i++) {
        int current_mode_set = 0;
        if (modes[i] != current_mode) {
            /* Change mode */
            const char mask_mode = AZ_MASK(modes[i]);
            if (mask_mode != current_mode) {
                size += 4 + (current_mode != AZ_D);
            }
            if (current_mode == AZ_U) {
                if (mask_mode == AZ_P) {
                    size += 5;
                }
            } else if (current_mode == AZ_L) {
                if (mask_mode == AZ_P) {
                    size += 5;
                } else if (mask_mode == AZ_U) {
                    size += 4;
                }
            } else if (current_mode == AZ_M) {
                if (mask_mode == AZ_D) {
                    size += 5;
                }
            } else if (current_mode == AZ_P) {
                if (mask_mode != AZ_U) {
                    size += 5;
                    if (mask_mode == AZ_B) {
                        current_mode = AZ_U;
                        current_mode_set = 1;
                    }
                }
            } else if (current_mode == AZ_D) {
                if (mask_mode == AZ_L || mask_mode == AZ_M) {
                    size += 5;
                } else if (mask_mode == AZ_P) {
                    size += 5 + 5;
                } else if (mask_mode == AZ_B) {
                    size += 5;
                    current_mode = AZ_U;
                    current_mode_set = 1;
                }
            }
            if (modes[i] & AZ_PS) {
                size += 4 + (mask_mode != AZ_D);
            } else if (modes[i] & AZ_US) {
                size += 4 + (mask_mode != AZ_D);
            }

            /* Byte mode - process full block here */
            if (modes[i] == AZ_B) {
                int big_batch = 0, count;
                for (count = 0; i + count < length && modes[i + count] == AZ_B; count++);

                if (count > 2047 + 2078) { /* Can't be more than 19968 / 8 = 2496 */
                    return 0;
                }
                byte_counts[byte_i++] = count;

                if (count > 2047) { /* Max 11-bit number */
                    big_batch = count > 2078 ? 2078 : count;
                    /* Put 00000 followed by 11-bit number of bytes less 31 */
                    size += 16 + 8 * big_batch;
                    i += big_batch;
                    count -= big_batch;
                }
                if (count) {
                    if (big_batch) {
                        size += 5;
                    }
                    if (count > 31) {
                        assert(count <= 2078);
                        /* Put 00000 followed by 11-bit number of bytes less 31 */
                        size += 16;
                    } else {
                        /* Put 5-bit number of bytes */
                        size += 5;
                    }
                    size += 8 * count;
                    i += count;
                }
                i--;
                continue;
            }

            if (current_mode != mask_mode && !current_mode_set) {
                current_mode = mask_mode;
            }
        }

        if (modes[i] == AZ_U || (modes[i] & AZ_US) || modes[i] == AZ_L || modes[i] == AZ_M) {
            size += 5;
        } else if (modes[i] == AZ_P || (modes[i] & AZ_PS)) {
            size += 5;
            if (gs1 && source[i] == '\x1D') {
                size += 3;
            }
        } else if (modes[i] == AZ_D) {
            size += 4;
        }
    }

    return size;
}

/* Determine encoding modes and encode */
static int az_text_process(unsigned char *source, int length, int bp, char *binary_string, const int gs1,
            const int gs1_bp, const int eci, const int fast_encode, char *p_current_mode, int *data_length,
            const int debug_print) {
    int i, j;
    char current_mode;
    int reduced_length;
    char *modes = (char *) z_alloca(length + 1);
    int *byte_counts = (int *) z_alloca(sizeof(int) * length); /* Cache of Byte-run counts */
    int byte_i = 0;
    int size;
    int eci_latch = 0;
    const char initial_mode = p_current_mode ? *p_current_mode : AZ_U;
    const int set_gs1 = bp == gs1_bp;
    const int all_byte_only_or_uld = az_all_byte_only_or_uld(source, length);
#ifndef NDEBUG
    const int initial_bp = bp;
#endif

    /* See if it's worthwhile latching to AZ_P when have ECI */
    if (!all_byte_only_or_uld && eci && initial_mode != AZ_P && az_count_initial_puncts(source, length)
            > 2 + (initial_mode == AZ_D)) {
        assert(!gs1);
        eci_latch = 1;
    }

    if (all_byte_only_or_uld) {
        memset(modes, all_byte_only_or_uld, length);
        reduced_length = length;
    } else if (fast_encode) {
        reduced_length = az_text_modes(modes, source, length, gs1, eci_latch ? AZ_P : initial_mode, debug_print);
    } else {
        reduced_length = az_define_modes(modes, source, length, gs1, eci_latch ? AZ_P : initial_mode, debug_print);
    }

    size = az_text_size(modes, source, reduced_length, gs1, set_gs1, eci, initial_mode, eci_latch, byte_counts);
    if (size == 0 || bp + size > AZTEC_BIN_CAPACITY) {
        return 0;
    }

    if (set_gs1 && gs1) {
        assert(initial_mode == AZ_U);
        bp = z_bin_append_posn(0, 5, binary_string, bp); /* P/S */
        bp = z_bin_append_posn(0, 5, binary_string, bp); /* FLG(n) */
        bp = z_bin_append_posn(0, 3, binary_string, bp); /* FLG(0) */
    }

    if (eci != 0) {
        if (initial_mode != AZ_P) {
            if (eci_latch) {
                if (initial_mode != AZ_M) {
                    if (initial_mode == AZ_D) {
                        bp = z_bin_append_posn(14, 4, binary_string, bp); /* U/L */
                    }
                    bp = z_bin_append_posn(29, 5, binary_string, bp); /* M/L */
                }
                bp = z_bin_append_posn(30, 5, binary_string, bp); /* P/L */
            } else {
                bp = z_bin_append_posn(0, initial_mode == AZ_D ? 4 : 5, binary_string, bp); /* P/S */
            }
        }
        bp = z_bin_append_posn(0, 5, binary_string, bp); /* FLG(n) */
        if (eci <= 9) {
            bp = z_bin_append_posn(1, 3, binary_string, bp); /* FLG(1) */
            bp = z_bin_append_posn(2 + eci, 4, binary_string, bp);
        } else if (eci <= 99) {
            bp = z_bin_append_posn(2, 3, binary_string, bp); /* FLG(2) */
            bp = z_bin_append_posn(2 + (eci / 10), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + (eci % 10), 4, binary_string, bp);
        } else if (eci <= 999) {
            bp = z_bin_append_posn(3, 3, binary_string, bp); /* FLG(3) */
            bp = z_bin_append_posn(2 + (eci / 100), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + ((eci % 100) / 10), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + (eci % 10), 4, binary_string, bp);
        } else if (eci <= 9999) {
            bp = z_bin_append_posn(4, 3, binary_string, bp); /* FLG(4) */
            bp = z_bin_append_posn(2 + (eci / 1000), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + ((eci % 1000) / 100), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + ((eci % 100) / 10), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + (eci % 10), 4, binary_string, bp);
        } else if (eci <= 99999) {
            bp = z_bin_append_posn(5, 3, binary_string, bp); /* FLG(5) */
            bp = z_bin_append_posn(2 + (eci / 10000), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + ((eci % 10000) / 1000), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + ((eci % 1000) / 100), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + ((eci % 100) / 10), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + (eci % 10), 4, binary_string, bp);
        } else {
            bp = z_bin_append_posn(6, 3, binary_string, bp); /* FLG(6) */
            bp = z_bin_append_posn(2 + (eci / 100000), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + ((eci % 100000) / 10000), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + ((eci % 10000) / 1000), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + ((eci % 1000) / 100), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + ((eci % 100) / 10), 4, binary_string, bp);
            bp = z_bin_append_posn(2 + (eci % 10), 4, binary_string, bp);
        }
    }

    current_mode = eci_latch ? AZ_P : initial_mode;
    for (i = 0; i < reduced_length; i++) {
        int current_mode_set = 0;
        if (modes[i] != current_mode) {
            /* Change mode */
            const char mask_mode = AZ_MASK(modes[i]);
            if (current_mode == AZ_U) {
                if (mask_mode == AZ_L) {
                    bp = z_bin_append_posn(28, 5, binary_string, bp); /* L/L */
                } else if (mask_mode == AZ_M) {
                    bp = z_bin_append_posn(29, 5, binary_string, bp); /* M/L */
                } else if (mask_mode == AZ_P) {
                    bp = z_bin_append_posn(29, 5, binary_string, bp); /* M/L */
                    bp = z_bin_append_posn(30, 5, binary_string, bp); /* P/L */
                } else if (mask_mode == AZ_D) {
                    bp = z_bin_append_posn(30, 5, binary_string, bp); /* D/L */
                } else if (mask_mode == AZ_B) {
                    bp = z_bin_append_posn(31, 5, binary_string, bp); /* B/S */
                }
            } else if (current_mode == AZ_L) {
                if (mask_mode == AZ_U) {
                    bp = z_bin_append_posn(30, 5, binary_string, bp); /* D/L */
                    bp = z_bin_append_posn(14, 4, binary_string, bp); /* U/L */
                } else if (mask_mode == AZ_M) {
                    bp = z_bin_append_posn(29, 5, binary_string, bp); /* M/L */
                } else if (mask_mode == AZ_P) {
                    bp = z_bin_append_posn(29, 5, binary_string, bp); /* M/L */
                    bp = z_bin_append_posn(30, 5, binary_string, bp); /* P/L */
                } else if (mask_mode == AZ_D) {
                    bp = z_bin_append_posn(30, 5, binary_string, bp); /* D/L */
                } else if (mask_mode == AZ_B) {
                    bp = z_bin_append_posn(31, 5, binary_string, bp); /* B/S */
                }
            } else if (current_mode == AZ_M) {
                if (mask_mode == AZ_U) {
                    bp = z_bin_append_posn(29, 5, binary_string, bp); /* U/L */
                } else if (mask_mode == AZ_L) {
                    bp = z_bin_append_posn(28, 5, binary_string, bp); /* L/L */
                } else if (mask_mode == AZ_P) {
                    bp = z_bin_append_posn(30, 5, binary_string, bp); /* P/L */
                } else if (mask_mode == AZ_D) {
                    bp = z_bin_append_posn(29, 5, binary_string, bp); /* U/L */
                    bp = z_bin_append_posn(30, 5, binary_string, bp); /* D/L */
                } else if (mask_mode == AZ_B) {
                    bp = z_bin_append_posn(31, 5, binary_string, bp); /* B/S */
                }
            } else if (current_mode == AZ_P) {
                if (mask_mode != current_mode) {
                    bp = z_bin_append_posn(31, 5, binary_string, bp); /* U/L */
                    if (mask_mode == AZ_L) {
                        bp = z_bin_append_posn(28, 5, binary_string, bp); /* L/L */
                    } else if (mask_mode == AZ_M) {
                        bp = z_bin_append_posn(29, 5, binary_string, bp); /* M/L */
                    } else if (mask_mode == AZ_D) {
                        bp = z_bin_append_posn(30, 5, binary_string, bp); /* D/L */
                    } else if (mask_mode == AZ_B) {
                        current_mode = AZ_U;
                        current_mode_set = 1;
                        bp = z_bin_append_posn(31, 5, binary_string, bp); /* B/S */
                    }
                }
            } else if (current_mode == AZ_D) {
                if (mask_mode != current_mode) {
                    bp = z_bin_append_posn(14, 4, binary_string, bp); /* U/L */
                    if (mask_mode == AZ_L) {
                        bp = z_bin_append_posn(28, 5, binary_string, bp); /* L/L */
                    } else if (mask_mode == AZ_M) {
                        bp = z_bin_append_posn(29, 5, binary_string, bp); /* M/L */
                    } else if (mask_mode == AZ_P) {
                        bp = z_bin_append_posn(29, 5, binary_string, bp); /* M/L */
                        bp = z_bin_append_posn(30, 5, binary_string, bp); /* P/L */
                    } else if (mask_mode == AZ_B) {
                        current_mode = AZ_U;
                        current_mode_set = 1;
                        bp = z_bin_append_posn(31, 5, binary_string, bp); /* B/S */
                    }
                }
            }
            if (modes[i] & AZ_PS) {
                assert(mask_mode != AZ_P);
                bp = z_bin_append_posn(0, 4 + (mask_mode != AZ_D), binary_string, bp); /* P/S */
            } else if (modes[i] & AZ_US) {
                assert(mask_mode == AZ_L || mask_mode == AZ_D);
                if (mask_mode == AZ_L) {
                    bp = z_bin_append_posn(28, 5, binary_string, bp); /* U/S */
                } else {
                    bp = z_bin_append_posn(15, 4, binary_string, bp); /* U/S */
                }
            }

            /* Byte mode - process full block here */
            if (modes[i] == AZ_B) {
                int big_batch = 0;
                int count = byte_counts[byte_i++];

                assert(count <= 2047 + 2078); /* Can't be more than 19968 / 8 = 2496 */

                if (count > 2047) { /* Max 11-bit number */
                    big_batch = count > 2078 ? 2078 : count;
                    /* Put 00000 followed by 11-bit number of bytes less 31 */
                    bp = z_bin_append_posn(big_batch - 31, 16, binary_string, bp);
                    for (j = 0; j < big_batch; j++) {
                        bp = z_bin_append_posn(source[i++], 8, binary_string, bp);
                    }
                    count -= big_batch;
                }
                if (count) {
                    if (big_batch) {
                        bp = z_bin_append_posn(31, 5, binary_string, bp); /* B/S */
                    }
                    if (count > 31) {
                        assert(count <= 2078);
                        /* Put 00000 followed by 11-bit number of bytes less 31 */
                        bp = z_bin_append_posn(count - 31, 16, binary_string, bp);
                    } else {
                        /* Put 5-bit number of bytes */
                        bp = z_bin_append_posn(count, 5, binary_string, bp);
                    }
                    for (j = 0; j < count; j++) {
                        bp = z_bin_append_posn(source[i++], 8, binary_string, bp);
                    }
                }
                i--;
                continue;
            }

            if (current_mode != mask_mode && !current_mode_set) {
                current_mode = mask_mode;
            }
        }

        if (modes[i] == AZ_U || (modes[i] & AZ_US)) {
            if (source[i] == ' ') {
                bp = z_bin_append_posn(1, 5, binary_string, bp); /* SP */
            } else {
                bp = z_bin_append_posn(AztecSymbolChar[source[i]], 5, binary_string, bp);
            }
        } else if (modes[i] == AZ_L) {
            if (source[i] == ' ') {
                bp = z_bin_append_posn(1, 5, binary_string, bp); /* SP */
            } else {
                bp = z_bin_append_posn(AztecSymbolChar[source[i]], 5, binary_string, bp);
            }
        } else if (modes[i] == AZ_M) {
            if (source[i] == ' ') {
                bp = z_bin_append_posn(1, 5, binary_string, bp); /* SP */
            } else if (source[i] == '\r') {
                bp = z_bin_append_posn(14, 5, binary_string, bp); /* CR */
            } else {
                bp = z_bin_append_posn(AztecSymbolChar[source[i]], 5, binary_string, bp);
            }
        } else if (modes[i] == AZ_P || (modes[i] & AZ_PS)) {
            if (gs1 && source[i] == '\x1D') {
                bp = z_bin_append_posn(0, 5, binary_string, bp); /* FLG(n) */
                bp = z_bin_append_posn(0, 3, binary_string, bp); /* FLG(0) = FNC1 */
            } else if (source[i] == '\r') {
                bp = z_bin_append_posn(1, 5, binary_string, bp); /* CR */
            } else if (source[i] == 'a') {
                bp = z_bin_append_posn(2, 5, binary_string, bp); /* CR LF */
            } else if (source[i] == 'b') {
                bp = z_bin_append_posn(3, 5, binary_string, bp); /* . SP */
            } else if (source[i] == 'c') {
                bp = z_bin_append_posn(4, 5, binary_string, bp); /* , SP */
            } else if (source[i] == 'd') {
                bp = z_bin_append_posn(5, 5, binary_string, bp); /* : SP */
            } else if (source[i] == ',') {
                bp = z_bin_append_posn(17, 5, binary_string, bp); /* Comma */
            } else if (source[i] == '.') {
                bp = z_bin_append_posn(19, 5, binary_string, bp); /* Full stop */
            } else {
                bp = z_bin_append_posn(AztecSymbolChar[source[i]], 5, binary_string, bp);
            }
        } else if (modes[i] == AZ_D) {
            if (source[i] == ' ') {
                bp = z_bin_append_posn(1, 4, binary_string, bp); /* SP */
            } else if (source[i] == ',') {
                bp = z_bin_append_posn(12, 4, binary_string, bp); /* Comma */
            } else if (source[i] == '.') {
                bp = z_bin_append_posn(13, 4, binary_string, bp); /* Full stop */
            } else {
                bp = z_bin_append_posn(AztecSymbolChar[source[i]], 4, binary_string, bp);
            }
        }
    }

    if (debug_print) {
        printf("Binary String (%d): %.*s\n", bp, bp, binary_string);
    }

    *data_length = bp;
    if (p_current_mode) {
        *p_current_mode = current_mode;
    }

    assert(size == bp - initial_bp);

    return 1;
}

/* Call `az_text_process()` for each segment */
static int az_text_process_segs(struct zint_symbol *symbol, struct zint_seg segs[], const int seg_count, int bp,
            char binary_string[], const int gs1, const int gs1_bp, int *data_length, const int debug_print) {
    int i;
    char current_mode = AZ_U;
    const int fast_encode = symbol->input_mode & FAST_MODE;
    /* Raw text dealt with by `ZBarcode_Encode_Segs()`, except for `eci` feedback.
       Note not updating `eci` for GS1 mode as not converted */
    const int content_segs = !gs1 && (symbol->output_options & BARCODE_CONTENT_SEGS);

    for (i = 0; i < seg_count; i++) {
        if (!az_text_process(segs[i].source, segs[i].length, bp, binary_string, gs1, gs1_bp, segs[i].eci, fast_encode,
                &current_mode, &bp, debug_print)) {
            return ZINT_ERROR_TOO_LONG; /* `az_text_process()` only fails with too long */
        }
        if (content_segs && segs[i].eci) {
            z_ct_set_seg_eci(symbol, i, segs[i].eci);
        }
    }

    *data_length = bp;

    return 0;
}

/* Prevent data from obscuring reference grid */
static int az_avoidReferenceGrid(int output) {

    if (output > 10) {
        output += (output - 11) / 15 + 1;
    }

    return output;
}

/* Calculate the position of the bits in the grid (non-compact) */
static void az_populate_map(short AztecMap[], const int layers) {
    int layer;
    int x, y;
    const int offset = AztecOffset[layers - 1];
    const int endoffset = 151 - offset;

    for (layer = 0; layer < layers; layer++) {
        const int start = (112 * layer) + (16 * layer * layer) + 2;
        const int length = 28 + (layer * 4) + (layer + 1) * 4;
        int av0, av1;
        int n = start, end;
        /* Top */
        x = 64 - (layer * 2);
        y = 63 - (layer * 2);
        av0 = az_avoidReferenceGrid(y) * 151;
        av1 = az_avoidReferenceGrid(y - 1) * 151;
        end = start + length;
        while (n < end) {
            const int avxi = az_avoidReferenceGrid(x++);
            AztecMap[av0 + avxi] = n++;
            AztecMap[av1 + avxi] = n++;
        }
        /* Right */
        x = 78 + (layer * 2);
        y = 64 - (layer * 2);
        av0 = az_avoidReferenceGrid(x);
        av1 = az_avoidReferenceGrid(x + 1);
        end += length;
        while (n < end) {
            const int avyi = az_avoidReferenceGrid(y++) * 151;
            AztecMap[avyi + av0] = n++;
            AztecMap[avyi + av1] = n++;
        }
        /* Bottom */
        x = 77 + (layer * 2);
        y = 78 + (layer * 2);
        av0 = az_avoidReferenceGrid(y) * 151;
        av1 = az_avoidReferenceGrid(y + 1) * 151;
        end += length;
        while (n < end) {
            const int avxi = az_avoidReferenceGrid(x--);
            AztecMap[av0 + avxi] = n++;
            AztecMap[av1 + avxi] = n++;
        }
        /* Left */
        x = 63 - (layer * 2);
        y = 77 + (layer * 2);
        av0 = az_avoidReferenceGrid(x);
        av1 = az_avoidReferenceGrid(x - 1);
        end += length;
        while (n < end) {
            const int avyi = az_avoidReferenceGrid(y--) * 151;
            AztecMap[avyi + av0] = n++;
            AztecMap[avyi + av1] = n++;
        }
    }

    /* Copy "Core Symbol" (finder, descriptor, orientation) */
    for (y = 0; y < 15; y++) {
        memcpy(AztecMap + (y + 68) * 151 + 68, AztecMapCore[y], sizeof(short) * 15);
    }

    /* Reference grid guide bars */
    for (y = offset <= 11 ? 11 : AztecMapGridYOffsets[(offset - 11) / 16]; y < endoffset; y += 16) {
        for (x = offset; x < endoffset; x++) {
            AztecMap[(x * 151) + y] = x & 1;
            AztecMap[(y * 151) + x] = x & 1;
        }
    }
}

/* Helper to insert dummy '0' or '1's into runs of same bits. See ISO/IEC 24778:2008 7.3.1.2 */
static int az_bitrun_stuff(const char *binary_string, const int data_length, const int codeword_size,
            const int data_maxsize, char adjusted_string[AZTEC_MAX_CAPACITY]) {
    int i, j = 0, count = 0;

    for (i = 0; i < data_length; i++) {

        if ((j + 1) % codeword_size == 0) {
            /* Last bit of codeword */
            /* 7.3.1.2 "whenever the first B-1 bits ... are all “0”s, then a dummy “1” is inserted..."
               "Similarly a message codeword that starts with B-1 “1”s has a dummy “0” inserted..." */

            if (count == 0 || count == codeword_size - 1) {
                /* Codeword of B-1 '0's or B-1 '1's */
                if (j > data_maxsize) {
                    return 0; /* Fail */
                }
                adjusted_string[j++] = count == 0 ? '1' : '0';
                count = binary_string[i] == '1' ? 1 : 0;
            } else {
                count = 0;
            }

        } else if (binary_string[i] == '1') { /* Skip B so only counting B-1 */
            count++;
        }
        if (j > data_maxsize) {
            return 0; /* Fail */
        }
        adjusted_string[j++] = binary_string[i];
    }

    return j;
}

/* Helper to add padding, accounting for bitrun stuffing */
static int az_add_padding(const int codeword_size, char adjusted_string[AZTEC_MAX_CAPACITY], int adjusted_length,
            const int debug_print) {
    const int remainder = adjusted_length % codeword_size;
    const int padbits = remainder ? codeword_size - remainder : 0;

    if (padbits) {
        int i, count = 0;

        assert(adjusted_length + padbits <= AZTEC_BIN_CAPACITY);

        for (i = 0; i < padbits; i++) {
            adjusted_string[adjusted_length++] = '1';
        }

        for (i = (adjusted_length - codeword_size); i < adjusted_length; i++) {
            count += adjusted_string[i] == '1';
        }
        if (count == codeword_size) {
            adjusted_string[adjusted_length - 1] = '0';
        }
    }
    if (debug_print) printf("Remainder: %d  Pad bits: %d\n", remainder, padbits);

    return adjusted_length;
}

/* Determine codeword bitlength - Table 3 */
static int az_codeword_size(const int layers) {
    int codeword_size;

    if (layers <= 2) {
        codeword_size = 6;
    } else if (layers <= 8) {
        codeword_size = 8;
    } else if (layers <= 22) {
        codeword_size = 10;
    } else {
        codeword_size = 12;
    }
    return codeword_size;
}

/* Encodes Aztec Code as specified in ISO/IEC 24778:2008 */
INTERNAL int zint_aztec(struct zint_symbol *symbol, struct zint_seg segs[], const int seg_count) {
    int x, y, i, p, data_blocks, ecc_blocks, layers, total_bits;
    char bit_pattern[AZTEC_MAP_POSN_MAX + 1]; /* Note AZTEC_MAP_POSN_MAX > AZTEC_BIN_CAPACITY */
    /* To lessen stack usage, share binary_string buffer with bit_pattern, as accessed separately */
    char *binary_string = bit_pattern;
    char descriptor[42];
    char adjusted_string[AZTEC_MAX_CAPACITY];
    short AztecMap[AZTEC_MAP_SIZE];
    unsigned char desc_data[4], desc_ecc[6];
    int error_number;
    int compact, data_length, data_maxsize, codeword_size, adjusted_length;
    int bp = 0;
    int gs1_bp = 0;
    const int gs1 = (symbol->input_mode & 0x07) == GS1_MODE;
    const int reader_init = symbol->output_options & READER_INIT;
    const int compact_loop_start = reader_init ? 1 : 4; /* Compact 2-4 excluded from Reader Initialisation */
    const int debug_print = symbol->debug & ZINT_DEBUG_PRINT;
    rs_t rs;
    rs_uint_t rs_uint;
    unsigned int *data_part;
    unsigned int *ecc_part;
    float ecc_ratio;
    int dim;

    if (gs1 && reader_init) {
        return z_errtxt(ZINT_ERROR_INVALID_OPTION, symbol, 501, "Cannot use Reader Initialisation in GS1 mode");
    }

    if (symbol->structapp.count) {
        /* Structured Append info as string <SP> + ID + <SP> + index + count + NUL */
        unsigned char sa_src[1 + sizeof(symbol->structapp.id) + 1 + 1 + 1 + 1] = {0};
        int sa_len;
        int id_len;

        if (symbol->structapp.count < 2 || symbol->structapp.count > 26) {
            return z_errtxtf(ZINT_ERROR_INVALID_OPTION, symbol, 701,
                            "Structured Append count '%d' out of range (2 to 26)", symbol->structapp.count);
        }
        if (symbol->structapp.index < 1 || symbol->structapp.index > symbol->structapp.count) {
            return ZEXT z_errtxtf(ZINT_ERROR_INVALID_OPTION, symbol, 702,
                                    "Structured Append index '%1$d' out of range (1 to count %2$d)",
                                    symbol->structapp.index, symbol->structapp.count);
        }

        for (id_len = 0; id_len < 32 && symbol->structapp.id[id_len]; id_len++);

        if (id_len && z_chr_cnt(ZCUCP(symbol->structapp.id), id_len, ' ')) {
            /* Note ID can contain any old chars apart from space so don't print in error message */
            return z_errtxt(ZINT_ERROR_INVALID_OPTION, symbol, 703, "Structured Append ID cannot contain spaces");
        }

        bp = z_bin_append_posn(29, 5, binary_string, bp); /* M/L */
        bp = z_bin_append_posn(29, 5, binary_string, bp); /* U/L */

        sa_len = 0;
        if (id_len) { /* ID has a space on either side */
            sa_src[sa_len++] = ' ';
            memcpy(sa_src + sa_len, symbol->structapp.id, id_len);
            sa_len += id_len;
            sa_src[sa_len++] = ' ';
        }
        sa_src[sa_len++] = 'A' + symbol->structapp.index - 1;
        sa_src[sa_len++] = 'A' + symbol->structapp.count - 1;
        if (debug_print) {
            printf("Structured Append Count: %d, Index: %d, ID: %.32s, String: %s\n",
                    symbol->structapp.count, symbol->structapp.index, symbol->structapp.id, sa_src);
        }

        (void) az_text_process(sa_src, sa_len, bp, binary_string, 0 /*gs1*/, 0 /*gs1_bp*/, 0 /*eci*/,
                                0 /*fast_encode*/, NULL /*p_current_mode*/, &bp, debug_print);
        /* Will be in U/L due to uppercase A-Z index/count indicators at end */
        gs1_bp = bp; /* Initial FNC1 (FLG0) position */
    }

    if ((error_number = az_text_process_segs(symbol, segs, seg_count, bp, binary_string, gs1, gs1_bp, &data_length,
                                                debug_print))) {
        assert(error_number == ZINT_ERROR_TOO_LONG || error_number == ZINT_ERROR_MEMORY);
        if (error_number == ZINT_ERROR_TOO_LONG) {
            return z_errtxt(error_number, symbol, 502,
                            "Input too long, requires too many codewords (maximum " AZ_BIN_CAP_CWDS_S ")");
        }
        return error_number;
    }
    assert(data_length > 0); /* Suppress clang-tidy warning: clang-analyzer-core.UndefinedBinaryOperatorResult */

    if (symbol->option_1 < -1 || symbol->option_1 > 4) {
        z_errtxtf(0, symbol, 503, "Error correction level '%d' out of range (1 to 4)", symbol->option_1);
        if (symbol->warn_level == WARN_FAIL_ALL) {
            return ZINT_ERROR_INVALID_OPTION;
        }
        error_number = z_errtxt_adj(ZINT_WARN_INVALID_OPTION, symbol, "%1$s%2$s", ", ignoring");
        symbol->option_1 = -1; /* Feedback options */
    }

    data_maxsize = 0; /* Keep compiler happy! */

    if (symbol->option_2 == 0) {
        /* The size of the symbol can be determined by Zint */
        int ecc_level = symbol->option_1;
        int adjustment_size = 0;

        if (ecc_level <= 0) {
            ecc_level = 2;
        }

        do {
            /* Decide what size symbol to use - the smallest that fits the data */
            compact = 0; /* 1 = Aztec Compact, 0 = Normal Aztec */
            layers = 0;

            /* For each level of error correction work out the smallest symbol which the data will fit in */
            if ((symbol->option_3 & 0xFF) != ZINT_AZTEC_FULL && data_length + adjustment_size
                                                    <= AztecCompactDataSizes[ecc_level - 1][compact_loop_start - 1]) {
                for (i = compact_loop_start; i > 0; i--) {
                    if (data_length + adjustment_size <= AztecCompactDataSizes[ecc_level - 1][i - 1]) {
                        layers = i;
                        compact = 1;
                        data_maxsize = AztecCompactDataSizes[ecc_level - 1][i - 1];
                    }
                }
            }
            if (!compact && data_length + adjustment_size <= AztecDataSizes[ecc_level - 1][32 - 1]) {
                for (i = 32; i > 0; i--) {
                    if (data_length + adjustment_size <= AztecDataSizes[ecc_level - 1][i - 1]) {
                        layers = i;
                        compact = 0;
                        data_maxsize = AztecDataSizes[ecc_level - 1][i - 1];
                    }
                }
            }

            if (layers == 0) { /* Couldn't find a symbol which fits the data */
                if (adjustment_size == 0) {
                    return ZEXT z_errtxtf(ZINT_ERROR_TOO_LONG, symbol, 707,
                                    "Input too long for ECC level %1$d, requires too many codewords (maximum %2$d)",
                                    ecc_level, AztecDataSizes[ecc_level - 1][31] / 12);
                }
                return ZEXT z_errtxtf(ZINT_ERROR_TOO_LONG, symbol, 504,
                                        "Input too long for ECC level %1$d, requires %2$d codewords (maximum %3$d)",
                                        ecc_level, (data_length + adjustment_size + 11) / 12,
                                        AztecDataSizes[ecc_level - 1][31] / 12);
            }

            codeword_size = az_codeword_size(layers);

            adjusted_length = az_bitrun_stuff(binary_string, data_length, codeword_size,
                                                adjustment_size ? data_maxsize : AZTEC_BIN_CAPACITY, adjusted_string);
            if (adjusted_length) {
                /* Add padding */
                adjusted_length = az_add_padding(codeword_size, adjusted_string, adjusted_length, debug_print);
                adjustment_size = adjusted_length - data_length;
            }
            if (debug_print) {
                printf("Adjusted Length: %d, Data Max Size %d, Data Length %d\n",
                        adjusted_length, data_maxsize, data_length);
            }

            if (adjusted_length == 0) {
                return ZEXT z_errtxtf(ZINT_ERROR_TOO_LONG, symbol, 705,
                                    "Input too long for ECC level %1$d, requires too many codewords (maximum %2$d)",
                                    ecc_level, (adjustment_size ? data_maxsize : AZTEC_BIN_CAPACITY) / codeword_size);
            }

        } while (adjusted_length > data_maxsize);
        /* This loop will only repeat on the rare occasions when the rule about not having all 1s or all 0s
        means that the binary string has had to be lengthened beyond the maximum number of bits that can
        be encoded in a symbol of the selected size */

        symbol->option_2 = compact ? layers : layers + 4; /* Feedback options */

    } else {
        /* The size of the symbol has been specified by the user */
        if (symbol->option_2 < 0 || symbol->option_2 > 36) {
            return z_errtxtf(ZINT_ERROR_INVALID_OPTION, symbol, 510, "Version '%d' out of range (1 to 36)",
                                symbol->option_2);
        }
        if (reader_init) {
            /* For back-compatibility, silently ignore compact 2-4 requests but error on layers > 22 */
            if (symbol->option_2 >= 2 && symbol->option_2 <= 4) {
                symbol->option_2 = 5;
            } else if (symbol->option_2 > 26) {
                /* Caught below anyway but catch here also for better feedback */
                return z_errtxtf(ZINT_ERROR_INVALID_OPTION, symbol, 709,
                                "Version '%d' out of range for Reader Initialisation symbols (maximum 26)",
                                symbol->option_2);
            }
        }
        compact = symbol->option_2 <= 4;
        layers = compact ? symbol->option_2 : symbol->option_2 - 4;

        codeword_size = az_codeword_size(layers);
        if (compact) {
            data_maxsize = codeword_size * (AztecCompactSizes[layers - 1] - 3);
        } else {
            data_maxsize = codeword_size * (AztecSizes[layers - 1] - 3);
        }

        adjusted_length = az_bitrun_stuff(binary_string, data_length, codeword_size, data_maxsize, adjusted_string);
        if (adjusted_length && adjusted_length <= data_maxsize) {
            /* Add padding */
            adjusted_length = az_add_padding(codeword_size, adjusted_string, adjusted_length, debug_print);
        }
        if (debug_print) {
            printf("Adjusted Length: %d, Data Max Size %d, Data Length %d\n",
                    adjusted_length, data_maxsize, data_length);
        }

        /* Check if the data actually fits into the selected symbol size */
        if (adjusted_length == 0) {
            return ZEXT z_errtxtf(ZINT_ERROR_TOO_LONG, symbol, 704,
                                    "Input too long for Version %1$d, requires too many codewords (maximum %2$d)",
                                    symbol->option_2, (data_maxsize + codeword_size - 1) / codeword_size);
        } else if (adjusted_length > data_maxsize) {
            return ZEXT z_errtxtf(ZINT_ERROR_TOO_LONG, symbol, 505,
                                    "Input too long for Version %1$d, requires %2$d codewords (maximum %3$d)",
                                    symbol->option_2, (adjusted_length + codeword_size - 1) / codeword_size,
                                    data_maxsize / codeword_size);
        }
    }

    if (debug_print) {
        printf("Codewords (%d):\n", adjusted_length / codeword_size);
        for (i = 0; i < (adjusted_length / codeword_size); i++) {
            printf(" %.*s", codeword_size, adjusted_string + i * codeword_size);
        }
        fputc('\n', stdout);
    }

    if (reader_init && layers > 22) {
        return z_errtxtf(ZINT_ERROR_TOO_LONG, symbol, 506,
                        "Input too long for Reader Initialisation, requires %d layers (maximum 22)", layers);
    }

    data_blocks = adjusted_length / codeword_size;

    if (compact) {
        ecc_blocks = AztecCompactSizes[layers - 1] - data_blocks;
        if (layers == 4) { /* Can use spare blocks for ECC (76 available - 64 max data blocks) */
            ecc_blocks += 12;
        }
    } else {
        ecc_blocks = AztecSizes[layers - 1] - data_blocks;
    }
    if (ecc_blocks == 3) {
        ecc_ratio = 0.0f;
        error_number = z_errtxt(ZINT_WARN_NONCOMPLIANT, symbol, 706, "Number of ECC codewords 3 at minimum");
        symbol->option_1 = -1; /* Feedback options: indicate minimum 3 with -1 */
    } else {
        ecc_ratio = z_stripf((float) (ecc_blocks - 3) / (data_blocks + ecc_blocks));
        if (ecc_ratio < 0.05f) {
            error_number = ZEXT z_errtxtf(ZINT_WARN_NONCOMPLIANT, symbol, 708,
                                            "Number of ECC codewords %1$d less than 5%% + 3 of data codewords %2$d",
                                            ecc_blocks, data_blocks);
            symbol->option_1 = 0; /* Feedback options: indicate < 5% + 3 with 0 */
        } else {
            /* Feedback options: 0.165 = (.1 + .23) / 2 etc */
            symbol->option_1 = ecc_ratio < 0.165f ? 1 : ecc_ratio < 0.295f ? 2 : ecc_ratio < 0.43f ? 3 : 4;
        }
        /* Feedback percentage in top byte */
        symbol->option_1 |= ((int) z_stripf(ecc_ratio * 100.0f)) << 8;
    }

    data_part = (unsigned int *) z_alloca(sizeof(unsigned int) * data_blocks);
    ecc_part = (unsigned int *) z_alloca(sizeof(unsigned int) * ecc_blocks);

    /* Copy across data into separate integers */
    memset(data_part, 0, sizeof(unsigned int) * data_blocks);
    memset(ecc_part, 0, sizeof(unsigned int) * ecc_blocks);

    /* Split into codewords and calculate reed-solomon error correction codes */
    for (i = 0; i < data_blocks; i++) {
        for (p = 0; p < codeword_size; p++) {
            if (adjusted_string[i * codeword_size + p] == '1') {
                data_part[i] |= 0x01 << (codeword_size - (p + 1));
            }
        }
    }

    switch (codeword_size) {
        case 6:
            zint_rs_init_gf(&rs, 0x43);
            zint_rs_init_code(&rs, ecc_blocks, 1);
            zint_rs_encode_uint(&rs, data_blocks, data_part, ecc_part);
            break;
        case 8:
            zint_rs_init_gf(&rs, 0x12d);
            zint_rs_init_code(&rs, ecc_blocks, 1);
            zint_rs_encode_uint(&rs, data_blocks, data_part, ecc_part);
            break;
        case 10:
            if (!zint_rs_uint_init_gf(&rs_uint, 0x409, 1023)) { /* Can fail on malloc() */
                return z_errtxt(ZINT_ERROR_MEMORY, symbol, 500, "Insufficient memory for Reed-Solomon log tables");
            }
            zint_rs_uint_init_code(&rs_uint, ecc_blocks, 1);
            zint_rs_uint_encode(&rs_uint, data_blocks, data_part, ecc_part);
            zint_rs_uint_free(&rs_uint);
            break;
        case 12:
            if (!zint_rs_uint_init_gf(&rs_uint, 0x1069, 4095)) { /* Can fail on malloc() */
                /* Note using AUSPOST error nos range as out of 50x ones & 51x taken by CODEONE */
                return z_errtxt(ZINT_ERROR_MEMORY, symbol, 700, "Insufficient memory for Reed-Solomon log tables");
            }
            zint_rs_uint_init_code(&rs_uint, ecc_blocks, 1);
            zint_rs_uint_encode(&rs_uint, data_blocks, data_part, ecc_part);
            zint_rs_uint_free(&rs_uint);
            break;
    }

    for (i = 0; i < ecc_blocks; i++) {
        adjusted_length = z_bin_append_posn(ecc_part[i], codeword_size, adjusted_string, adjusted_length);
    }

    /* Invert the data so that actual data is on the outside and reed-solomon on the inside */
    memset(bit_pattern, '0', AZTEC_MAP_POSN_MAX + 1);

    total_bits = (data_blocks + ecc_blocks) * codeword_size;
    for (i = 0; i < total_bits; i++) {
        bit_pattern[i] = adjusted_string[total_bits - i - 1];
    }

    /* Now add the symbol descriptor */
    memset(desc_data, 0, 4);
    memset(desc_ecc, 0, 6);
    memset(descriptor, 0, 42);

    if (compact) {
        /* The first 2 bits represent the number of layers minus 1 */
        descriptor[0] = ((layers - 1) & 0x02) ? '1' : '0';
        descriptor[1] = ((layers - 1) & 0x01) ? '1' : '0';

        /* The next 6 bits represent the number of data blocks minus 1 */
        descriptor[2] = reader_init || ((data_blocks - 1) & 0x20) ? '1' : '0';
        for (i = 3; i < 8; i++) {
            descriptor[i] = ((data_blocks - 1) & (0x10 >> (i - 3))) ? '1' : '0';
        }
        if (debug_print) printf("Mode Message = %.8s\n", descriptor);
    } else {
        /* The first 5 bits represent the number of layers minus 1 */
        for (i = 0; i < 5; i++) {
            descriptor[i] = ((layers - 1) & (0x10 >> i)) ? '1' : '0';
        }

        /* The next 11 bits represent the number of data blocks minus 1 */
        descriptor[5] = reader_init || ((data_blocks - 1) & 0x400) ? '1' : '0';
        for (i = 6; i < 16; i++) {
            descriptor[i] = ((data_blocks - 1) & (0x200 >> (i - 6))) ? '1' : '0';
        }
        if (debug_print) printf("Mode Message = %.16s\n", descriptor);
    }

    /* Split into 4-bit codewords */
    for (i = 0; i < 4; i++) {
        desc_data[i] = ((descriptor[i * 4] == '1') << 3) | ((descriptor[(i * 4) + 1] == '1') << 2)
                        | ((descriptor[(i * 4) + 2] == '1') << 1) | (descriptor[(i * 4) + 3] == '1');
    }

    /* Add Reed-Solomon error correction with Galois field GF(16) and prime modulus x^4 + x + 1 (section 7.2.3) */

    zint_rs_init_gf(&rs, 0x13);
    if (compact) {
        zint_rs_init_code(&rs, 5, 1);
        zint_rs_encode(&rs, 2, desc_data, desc_ecc);
        for (i = 0; i < 5; i++) {
            descriptor[(i * 4) + 8] = (desc_ecc[i] & 0x08) ? '1' : '0';
            descriptor[(i * 4) + 9] = (desc_ecc[i] & 0x04) ? '1' : '0';
            descriptor[(i * 4) + 10] = (desc_ecc[i] & 0x02) ? '1' : '0';
            descriptor[(i * 4) + 11] = (desc_ecc[i] & 0x01) ? '1' : '0';
        }
    } else {
        zint_rs_init_code(&rs, 6, 1);
        zint_rs_encode(&rs, 4, desc_data, desc_ecc);
        for (i = 0; i < 6; i++) {
            descriptor[(i * 4) + 16] = (desc_ecc[i] & 0x08) ? '1' : '0';
            descriptor[(i * 4) + 17] = (desc_ecc[i] & 0x04) ? '1' : '0';
            descriptor[(i * 4) + 18] = (desc_ecc[i] & 0x02) ? '1' : '0';
            descriptor[(i * 4) + 19] = (desc_ecc[i] & 0x01) ? '1' : '0';
        }
    }

    /* Merge descriptor with the rest of the symbol */
    if (compact) {
        memcpy(bit_pattern + 2000 - 2, descriptor, 40);
    } else {
        memcpy(bit_pattern + 20000 - 2, descriptor, 40);
    }

    /* Plot all of the data into the symbol in pre-defined spiral pattern */
    if (compact) {
        const int offset = AztecCompactOffset[layers - 1];
        const int end_offset = 27 - offset;
        for (y = offset; y < end_offset; y++) {
            const int y_map = y * 27;
            for (x = offset; x < end_offset; x++) {
                const int map = AztecCompactMap[y_map + x];
                if (map == 1 || (map >= 2 && bit_pattern[map - 2] == '1')) {
                    z_set_module(symbol, y - offset, x - offset);
                }
            }
            symbol->row_height[y - offset] = 1;
        }
        dim = 27 - (2 * offset);
    } else {
        const int offset = AztecOffset[layers - 1];
        const int end_offset = 151 - offset;
        az_populate_map(AztecMap, layers);
        for (y = offset; y < end_offset; y++) {
            const int y_map = y * 151;
            for (x = offset; x < end_offset; x++) {
                const int map = AztecMap[y_map + x];
                if (map == 1 || (map >= 2 && bit_pattern[map - 2] == '1')) {
                    z_set_module(symbol, y - offset, x - offset);
                }
            }
            symbol->row_height[y - offset] = 1;
        }
        dim = 151 - (2 * offset);
    }
    symbol->height = dim;
    symbol->rows = dim;
    symbol->width = dim;

    if (debug_print) {
        printf("Generating a %dx%d %s symbol with %d layers\n", dim, dim, compact ? "compact" : "full-size", layers);
        printf("Requires %d codewords of %d-bits\n", data_blocks + ecc_blocks, codeword_size);
        printf("    (%d data words, %d ecc words, %.1f%%, output option_1 0x%X, option_2 %d)\n",
                data_blocks, ecc_blocks, ecc_ratio * 100, symbol->option_1, symbol->option_2);
    }

    return error_number;
}

/* Encodes Aztec runes as specified in ISO/IEC 24778:2008 Annex A */
INTERNAL int zint_azrune(struct zint_symbol *symbol, unsigned char source[], int length) {
    unsigned int input_value;
    int i, y, x, r;
    char binary_string[28];
    unsigned char data_codewords[3], ecc_codewords[6];
    int bp = 0;
    rs_t rs;
    const int content_segs = symbol->output_options & BARCODE_CONTENT_SEGS;
    const int debug_print = symbol->debug & ZINT_DEBUG_PRINT;

    if (length > 3) {
        return z_errtxtf(ZINT_ERROR_TOO_LONG, symbol, 507, "Input length %d too long (maximum 3)", length);
    }
    if ((i = z_not_sane(NEON_F, source, length))) {
        return z_errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 508,
                        "Invalid character at position %d in input (digits only)", i);
    }
    input_value = z_to_int(source, length);

    if (input_value > 255) {
        return z_errtxt(ZINT_ERROR_INVALID_DATA, symbol, 509, "Input value out of range (0 to 255)");
    }

    bp = z_bin_append_posn(input_value, 8, binary_string, bp);

    data_codewords[0] = (unsigned char) (input_value >> 4);
    data_codewords[1] = (unsigned char) (input_value & 0xF);

    zint_rs_init_gf(&rs, 0x13);
    zint_rs_init_code(&rs, 5, 1);
    zint_rs_encode(&rs, 2, data_codewords, ecc_codewords);

    for (i = 0; i < 5; i++) {
        bp = z_bin_append_posn(ecc_codewords[i], 4, binary_string, bp);
    }

    for (i = 0; i < 28; i += 2) {
        binary_string[i] = '0' + (binary_string[i] != '1');
    }

    if (debug_print) {
        printf("Binary String: %.28s\n", binary_string);
    }

    for (y = 8; y < 19; y++) {
        r = y * 27;
        for (x = 8; x < 19; x++) {
            if (AztecCompactMap[r + x] == 1) {
                z_set_module(symbol, y - 8, x - 8);
            } else if (AztecCompactMap[r + x] && binary_string[AztecCompactMap[r + x] - 2000] == '1') {
                z_set_module(symbol, y - 8, x - 8);
            }
        }
        symbol->row_height[y - 8] = 1;
    }
    symbol->height = 11;
    symbol->rows = 11;
    symbol->width = 11;

    if (content_segs && z_ct_printf_256(symbol, "%03d", input_value)) {
        return ZINT_ERROR_MEMORY; /* `z_ct_printf_256()` only fails with OOM */
    }

    return 0;
}

/* vim: set ts=4 sw=4 et : */
