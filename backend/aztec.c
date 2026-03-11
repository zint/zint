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

    for (i++; i < length && AZ_MASK(modes[i]) == current_mode; i++);

    return i < length ? AZ_MASK(modes[i]) : AZ_E;
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
                source[j] = 'a'; /* "\r\n" */
            } else {
                source[j] = 'b' + 7 - ((source[i] & 0x0F) >> 1); /* ". " -> 'b', ", " -> 'c', ": " -> 'd' */
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
    static const char mode_chars[] = { 'U', 'L', 'M', 'P', 'D', 'B', 'X', 'E' };
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
            modes[i] = AZ_P; /* For FLG(0) = FNC1 */
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
                if ((next_mode != AZ_D && count <= 4) || (next_mode == AZ_D && count <= 7)) {
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

            if ((current_mode == AZ_U && (next_mode == AZ_U || next_mode == AZ_B) && count == 1)
                    || (current_mode == AZ_L && (next_mode == AZ_L || next_mode == AZ_B) && count == 1)
                    || current_mode == AZ_P || next_mode == AZ_P) {
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
                if (((next_mode == AZ_E || next_mode == AZ_U || next_mode == AZ_L || next_mode == AZ_M) && count <= 4)
                        || (next_mode == AZ_B && count <= 2)) {
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
                if ((next_mode == AZ_E && count <= 5) || (next_mode != AZ_D && count <= 9)) {
                    memset(modes + i, AZ_U, count);
                }

            } else if (current_mode == AZ_L) {
                if ((next_mode == AZ_E && count <= 5)
                        || (next_mode == AZ_L && count <= 14)
                        || ((next_mode == AZ_M || next_mode == AZ_P || next_mode == AZ_B) && count <= 9)) {
                    memset(modes + i, AZ_L, count);

                } else if (next_mode == AZ_U && count == 1) {
                    modes[i] = AZ_L;
                }

            } else if (current_mode == AZ_M) {
                if (((next_mode == AZ_E || next_mode == AZ_U) && count <= 9)
                        || ((next_mode == AZ_L || next_mode == AZ_B) && count <= 14)
                        || ((next_mode == AZ_M || next_mode == AZ_P) && count <= 19)) {
                    memset(modes + i, AZ_M, count);
                }

            } else if (current_mode == AZ_P) {
                if ((next_mode == AZ_E && count <= 5) || (next_mode != AZ_D && count <= 9)) {
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

                } else if (current_mode == AZ_D
                            && ((next_mode != AZ_D && count <= 3) || (next_mode == AZ_D && count <= 6))) {
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
   resp., or -1 if not */
static char az_all_byte_only_or_uld(const unsigned char source[], const int length) {
    int i;
    int byte_only, upper, lower, digit;

    for (i = 0, byte_only = 0; i < length; i++) {
        byte_only += !z_isascii(source[i]) || !AztecFlags[source[i]];
    }
    if (byte_only) {
        return byte_only == length ? AZ_B : -1;
    }
    for (i = 0, upper = 0, lower = 0, digit = 0; i < length; i++) {
        upper += !!(AztecFlags[source[i]] & AZ_U_F);
        lower += !!(AztecFlags[source[i]] & AZ_L_F);
        /* Dot, comma & space only non-digit AZ_D_F, exclude in case they're AZ_P doubles */
        digit += z_isdigit(source[i]);
    }
    return upper == length ? AZ_U : lower == length ? AZ_L : digit == length ? AZ_D : -1;
}

/* Count number of initial consecutive punct chars, assuming not GS1_MODE */
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

/* Optimized encoding stuff - see `az_binary_string()` */

struct az_token {
    short value;
    short count; /* If negative, bit count of simple (not binary shift) token, else byte count */
};

struct az_token_list {
    struct az_token *tokens;
    unsigned short used;
    unsigned short size; /* Capacity */
};

/* Represents all information about a sequence necessary to generate the current output */
struct az_state {
    /* The list of tokens output. If in B/S mode, this does *not* yet include the token for those bytes */
    struct az_token_list tokens;
    short mode; /* The current mode of the encoding, or the mode to return to if in B/S mode */
    short byteCount; /* If non-zero, the number of most recent bytes that should be output in B/S mode */
    unsigned short bitCount; /* The total number of bits generated (including B/S) */
};

struct az_state_list {
    struct az_state* states;
    unsigned short used;
    unsigned short size;
};

#define AZ_MIN_STATES_SIZE 4
#define AZ_MIN_TOKENS_SIZE 32

/* Initialize a state list */
static int az_state_list_init(struct az_state_list *list, const unsigned short initial_size) {
    const unsigned short size = initial_size < AZ_MIN_STATES_SIZE ? AZ_MIN_STATES_SIZE : initial_size;

    if (!(list->states = (struct az_state *) malloc(sizeof(struct az_state) * size))) {
        list->used = list->size = 0;
        return 0;
    }
    list->size = size;
    list->used = 0;
    return 1;
}

/* Copy a state `src` to `dst` */
static int az_state_cpy(const struct az_state *src, struct az_state *dst) {
    const unsigned short size = src->tokens.size < AZ_MIN_TOKENS_SIZE ? AZ_MIN_TOKENS_SIZE : src->tokens.size;

    *dst = *src;
    if (!(dst->tokens.tokens = (struct az_token *) malloc(sizeof(struct az_token) * size))) {
        return 0;
    }
    if (src->tokens.used) {
        memcpy(dst->tokens.tokens, src->tokens.tokens, sizeof(struct az_token) * src->tokens.used);
    }
    dst->tokens.size = size;
    return 1;
}

/* Free `state`, i.e. free its token list */
static void az_state_free(struct az_state *state) {
    if (state->tokens.tokens) {
        free(state->tokens.tokens);
        state->tokens.tokens = NULL;
    }
}

/* Check that there's enough room for `extra` more tokens in `state` */
static int az_tokens_add_chk(struct az_state *state, const int extra) {
    assert(extra < AZ_MIN_TOKENS_SIZE);
    if (!state->tokens.tokens) {
        const unsigned short size = AZ_MIN_TOKENS_SIZE;
        if (!(state->tokens.tokens = (struct az_token *) malloc(sizeof(struct az_token) * size))) {
            return 0;
        }
        state->tokens.size = size;
        state->tokens.used = 0;
    } else if (state->tokens.used + extra >= state->tokens.size) {
        struct az_token *tokens;
        const unsigned short size = state->tokens.size * 2;
        if (size <= state->tokens.size /* Overflow */
                || !(tokens = (struct az_token *) realloc(state->tokens.tokens, sizeof(struct az_token) * size))) {
            return 0;
        }
        assert(size > state->tokens.used + extra);
        state->tokens.tokens = tokens;
        state->tokens.size = size;
    }
    return 1;
}

/* Check have enough room to add a state to state list `list` */
static int az_state_list_add_chk(struct az_state_list *list) {
    if (list->used == list->size) {
        struct az_state *states;
        const unsigned short size = list->size * 2;
        if (size <= list->size /* Overflow */
                || !(states = (struct az_state *) realloc(list->states, sizeof(struct az_state) * size))) {
            return 0;
        }
        list->states = states;
        list->size = size;
    }
    return 1;
}

/* Free all states (including their tokens) of state list `list` */
static void az_state_list_free(struct az_state_list *list) {
    int i;
    if (list->states) {
        for (i = 0; i < list->used; i++) {
            if (list->states[i].tokens.tokens) {
                free(list->states[i].tokens.tokens);
            }
        }
        free(list->states);
        list->states = NULL;
    }
    list->used = list->size = 0;
}

#define AZ_FNC1_VAL 32          /* Pseudo-value for FNC1 - converted to 0 on setting token value */

/* Shorthand to add a token to state `s` */
#define AZ_ADD_TOKEN(s, v, c) \
    (s)->tokens.tokens[(s)->tokens.used].value = (v); \
    (s)->tokens.tokens[(s)->tokens.used++].count = (c)

/* Add a new state to `list` from `state`, with a latch if modes differ, and then a char, or 2 if `value2` set */
static int az_LatchMaybeAndAppend(const struct az_state *state, const int from, const int mode, const int value,
            const int value2, struct az_state_list *list) {
    int latchModeBitCount = mode == AZ_D ? 4 : 5;
    int val = value;
    int bitCount = state->bitCount;
    struct az_state *new_state;

    if (!az_state_list_add_chk(list)) {
        return 0;
    }
    new_state = list->states + list->used++;
    if (!az_state_cpy(state, new_state) || !az_tokens_add_chk(new_state, 4 /*extra*/)) {
        return 0;
    }
    /* End B/S if any */
    if (state->byteCount != 0) {
        AZ_ADD_TOKEN(new_state, from - state->byteCount, state->byteCount);
        new_state->byteCount = 0;
    }

    /* Latch if necessary */
    if (mode != state->mode) {
        AZ_ADD_TOKEN(new_state, AztecLatch[state->mode][mode], -AztecLatchNum[state->mode][mode]);
        bitCount += AztecLatchNum[state->mode][mode];
    }
    if (val == AZ_FNC1_VAL) { /* FNC1 */
        latchModeBitCount += 3;
        val = 0;
    }
    AZ_ADD_TOKEN(new_state, val, -latchModeBitCount);
    bitCount += latchModeBitCount;
    if (value2 != -1) {
        latchModeBitCount = mode == AZ_D ? 4 : 5;
        val = value2;
        if (val == AZ_FNC1_VAL) { /* FNC1 */
            latchModeBitCount += 3;
            val = 0;
        }
        AZ_ADD_TOKEN(new_state, val, -latchModeBitCount);
        bitCount += latchModeBitCount;
    }
    new_state->mode = mode;
    new_state->bitCount = bitCount;
    return 1;
}

/* Add a new state to `list` from `state`, with a temporary shift to a different mode to output a single value */
static int az_ShiftAndAppend(const struct az_state *state, const int from, const int mode, const int value,
            struct az_state_list *list) {
    const int thisModeBitCount = state->mode == AZ_D ? 4 : 5;
    int bitCount = 5;
    int val = value;
    struct az_state *new_state;

    assert(state->mode != mode);

    if (!az_state_list_add_chk(list)) {
        return 0;
    }
    new_state = list->states + list->used++;
    if (!az_state_cpy(state, new_state) || !az_tokens_add_chk(new_state, 3 /*extra*/)) {
        return 0;
    }
    /* End B/S if any */
    if (state->byteCount != 0) {
        AZ_ADD_TOKEN(new_state, from - state->byteCount, state->byteCount);
        new_state->byteCount = 0;
    }

    /* Shifts exist only to AZ_U and AZ_P, both with tokens size 5 */
    AZ_ADD_TOKEN(new_state, AztecShift[state->mode][mode], -thisModeBitCount);
    if (val == AZ_FNC1_VAL) { /* FNC1 */
        val = 0;
        bitCount += 3;
    }
    AZ_ADD_TOKEN(new_state, val, -bitCount);
    new_state->mode = state->mode;
    new_state->bitCount = state->bitCount + thisModeBitCount + bitCount;
    return 1;
}

/* Add a new state to `list` from `state`, with a latch to AZ_D and shift to AZ_P or AZ_U encoding `value`
   - may save a bit due to AZ_D's 4-bit shift */
static int az_DigitLatchShiftAndAppend(const struct az_state *state, const int from, const int shiftMode,
            const int value, struct az_state_list *list) {
    const int bitCount = state->bitCount + AztecLatchNum[state->mode][AZ_D] + 4 /*P/S or U/S*/;
    int val = value;
    int valBitCount = 5;
    struct az_state *new_state;

    assert(state->mode != AZ_D);
    assert(shiftMode == AZ_P || shiftMode == AZ_U);
    assert(state->mode != AZ_U || shiftMode != AZ_U);

    if (!az_state_list_add_chk(list)) {
        return 0;
    }
    new_state = list->states + list->used++;
    if (!az_state_cpy(state, new_state) || !az_tokens_add_chk(new_state, 4 /*extra*/)) {
        return 0;
    }
    /* End B/S if any */
    if (state->byteCount != 0) {
        AZ_ADD_TOKEN(new_state, from - state->byteCount, state->byteCount);
        new_state->byteCount = 0;
    }

    /* Latch to AZ_D */
    AZ_ADD_TOKEN(new_state, AztecLatch[state->mode][AZ_D], -AztecLatchNum[state->mode][AZ_D]);
    /* P/S or U/S */
    AZ_ADD_TOKEN(new_state, AztecShift[AZ_D][shiftMode], -4);
    if (val == AZ_FNC1_VAL) { /* FNC1 */
        valBitCount += 3;
        val = 0;
    }
    AZ_ADD_TOKEN(new_state, val, -valBitCount);
    new_state->mode = AZ_D;
    new_state->bitCount = bitCount + valBitCount;
    return 1;
}

/* Add a new state to `list` from `state`, but with an additional char in B/S mode, or 2 if `from2` set */
static int az_AddByteShiftChar(const struct az_state *state, const int from, const int from2,
            struct az_state_list *list) {
    int mode = state->mode;
    int bitCount = state->bitCount;
    const int deltaBitCount = state->byteCount == 0 || state->byteCount == 31 ? 18 : state->byteCount == 62 ? 9 : 8;
    struct az_state *new_state;

    if (!az_state_list_add_chk(list)) {
        return 0;
    }
    new_state = list->states + list->used++;
    if (!az_state_cpy(state, new_state) || !az_tokens_add_chk(new_state, 3 /*extra*/)) {
        return 0;
    }

    if (state->mode == AZ_P || state->mode == AZ_D) {
        assert(state->byteCount == 0);
        AZ_ADD_TOKEN(new_state, AztecLatch[mode][AZ_U], -AztecLatchNum[mode][AZ_U]);
        bitCount += AztecLatchNum[mode][AZ_U];
        mode = AZ_U;
    }
    new_state->mode = mode;
    new_state->byteCount = state->byteCount + 1;
    new_state->bitCount = bitCount + deltaBitCount;
    if (new_state->byteCount == 2047 + 31) {
        /* The string is as long as it's allowed to be - end it */
        AZ_ADD_TOKEN(new_state, from + 1 - new_state->byteCount, new_state->byteCount);
        new_state->byteCount = 0;
    }
    if (from2 != -1) {
        const int deltaBitCount2 = new_state->byteCount == 0 || new_state->byteCount == 31
                                    ? 18 : new_state->byteCount == 62 ? 9 : 8;
        new_state->byteCount++;
        new_state->bitCount += deltaBitCount2;
        if (new_state->byteCount == 2047 + 31) {
            /* The string is as long as it's allowed to be - end it */
            AZ_ADD_TOKEN(new_state, from2 + 1 - new_state->byteCount, new_state->byteCount);
            new_state->byteCount = 0;
        }
    }
    return 1;
}

/* Set the state `ret_state` identical to `state`, but no longer in B/S mode */
static int az_EndByteShift(const struct az_state *state, struct az_state *ret_state, const int from) {

    if (!az_state_cpy(state, ret_state)) {
        return 0;
    }
    if (state->byteCount == 0) {
        return 1;
    }
    if (!az_tokens_add_chk(ret_state, 1 /*extra*/)) {
        return 0;
    }
    AZ_ADD_TOKEN(ret_state, from - state->byteCount, state->byteCount);
    ret_state->byteCount = 0;
    return 1;
}

/* Helper for `az_IsBetterThanOrEqualTo()` to return bit-cost of B/S */
static int az_CalculateByteShiftCost(const struct az_state *state) {
    if (state->byteCount > 0) {
        if (state->byteCount > 31) {
            return 20 + (state->byteCount > 62); /* Two B/S's or one B/S with extended length */
        }
        return 10; /* One B/S */
    }
    return 0;
}

/* Return 1 if `state` is better (or equal) to be in than `other` state under all possible circumstances */
static int az_IsBetterThanOrEqualTo(const struct az_state *state, const struct az_state *other) {
    int newModeBitCount = state->bitCount + AztecLatchNum[state->mode][other->mode];

    if (other->byteCount > 0) {
        if (state->byteCount < other->byteCount) {
            /* Add additional B/S encoding cost of other, if any */
            newModeBitCount += az_CalculateByteShiftCost(other) - az_CalculateByteShiftCost(state);
        } else if (state->byteCount > other->byteCount) {
            /* Maximum possible additional cost (`state` ends up exceeding the 31 byte boundary
               and `other` state can stay beneath it) */
            newModeBitCount += 10;
        }
    }
    return newModeBitCount <= other->bitCount;
}

#if 0
#define AZ_DEBUG_LIST
#endif
#ifndef AZ_DEBUG_LIST
static void az_dump_list(const struct az_state_list *list, const char *prefix) {
    (void)list, (void)prefix;
}
#else
static void az_dump_list(const struct az_state_list *list, const char *prefix) {
    static char spaces[50 + 1] = "                                                  ";
    const int pl = (int) strlen(prefix);
    int i;
    fprintf(stderr, "%sSize %d\n", prefix, (int) list->used);
    for (i = 0; i < list->used; i++) {
        const struct az_state *state = list->states + i;
        fprintf(stderr, "%.*s %d: mode %d, Size(tokens) %d\n", pl, spaces, (int) i, state->mode,
                (int) state->tokens.used);
    }
}
#endif

/* Iterate through states, removing those that are sub-optimal */
static void az_SimplifyStates(struct az_state_list *list) {
    int i, j;
    const int rmap_size = (list->used + 7) >> 3;
    unsigned char *rmap = (unsigned char *) z_alloca(rmap_size); /* Map of entries removed */
    const struct az_state *new_state, *old_state;

    memset(rmap, 0, rmap_size);

    az_dump_list(list, " SS in  ");

    for (i = 0; i < list->used; i++) {
        if (!(rmap[i >> 3] & (1 << (i & 0x7)))) {
            new_state = list->states + i;
            for (j = i + 1; j < list->used; j++) {
                if (!(rmap[j >> 3] & (1 << (j & 0x7)))) {
                    old_state = list->states + j;
                    if (az_IsBetterThanOrEqualTo(old_state, new_state)) {
                        rmap[i >> 3] |= 1 << (i & 0x7);
                        break;
                    }
                    if (az_IsBetterThanOrEqualTo(new_state, old_state)) {
                        rmap[j >> 3] |= 1 << (j & 0x7);
                    }
                }
            }
        }
    }
    for (i = list->used - 1; i >= 0; i--) {
        if (rmap[i >> 3] & (1 << (i & 0x7))) {
            const int k = i + 1;
            /* Remove consecutive entries in one go */
            for (j = i - 1; j >= 0 && (rmap[j >> 3] & (1 << (j & 0x7))); j--) {
                az_state_free(list->states + j + 1);
            }
            az_state_free(list->states + j + 1);
            if (k < list->used) {
                memmove(list->states + j + 1, list->states + k, sizeof(struct az_state) * (list->used - k));
            }
            list->used -= i - j;
            i = j + 1;
        }
    }

    az_dump_list(list, " SS out ");
}

/* Return a set of states for a Punct double - see `az_UpdateStateForChar()` below */
static int az_UpdateStateForPair(const struct az_state *state, const int from, const int pairCode,
            struct az_state_list *ret_list) {
    /* Possibility 1: latch to AZ_P, and then append this code */
    if (!az_LatchMaybeAndAppend(state, from, AZ_P, pairCode, -1 /*value2*/, ret_list)) {
        return 0;
    }
    if (state->mode != AZ_P) {
        /* Possibility 2: shift to AZ_P, and then append this code.
           Every state except AZ_P (handled above) can shift */
        if (!az_ShiftAndAppend(state, from, AZ_P, pairCode, ret_list)) {
            return 0;
        }
    }
    if (pairCode == 3 || pairCode == 4) {
        /* Both characters are in AZ_D. Sometimes better to just add two digits */
        if (!az_LatchMaybeAndAppend(state, from, AZ_D, 16 - pairCode /*'.' or ','*/, 1 /*space*/, ret_list)) {
            return 0;
        }
    }
    if (state->byteCount > 0) {
        /* It only makes sense to do the characters as binary if already in B/S mode */
        if (!az_AddByteShiftChar(state, from, from + 1, ret_list)) {
            return 0;
        }
    }
    return 1;
}

/* Return a set of states that represent the possible ways of updating this state for the next character.
   The resulting set of states are added to `ret_list` */
static int az_UpdateStateForChar(const struct az_state *state, const unsigned char *source, const int from,
            const int gs1, struct az_state_list *ret_list) {
    unsigned char ch = source[from];
    const int fnc1 = gs1 && ch == '\x1D';
    const int charInCurrentTable = (fnc1 && state->mode == AZ_P)
                                    || (z_isascii(ch) && (AztecFlags[ch] & (1 << state->mode)));
    if (z_isascii(ch)) {
        int mode;
        for (mode = 0; mode < AZ_NUM_MODES; mode++) {
            const int charInModeTable = (fnc1 && mode == AZ_P) || (AztecFlags[ch] & (1 << mode));
            if (charInModeTable) {
                /* Try generating the char by latching to its mode */
                if (!charInCurrentTable || mode == state->mode || (mode == AZ_D && !fnc1)) {
                    /* If the char is in the current table, we don't want to latch to any other mode except possibly
                       digit (which uses only 4 bits). Any other latch would be equally successful *after* this char,
                       and so wouldn't save any bits */
                    const int latchChar = fnc1 ? AZ_FNC1_VAL : AztecChar[mode][ch];
                    if (!az_LatchMaybeAndAppend(state, from, mode, latchChar, -1 /*value2*/, ret_list)) {
                        return 0;
                    }
                }
                /* Try generating the char by switching to its mode */
                if (!charInCurrentTable && AztecShift[state->mode][mode] >= 0) {
                    /* It never makes sense to temporarily shift to another mode if the char exists in the current
                       mode. That can never save bits */
                    const int shiftChar = fnc1 ? AZ_FNC1_VAL : AztecChar[mode][ch];
                    if (!az_ShiftAndAppend(state, from, mode, shiftChar, ret_list)) {
                        return 0;
                    }
                }
            }
        }
        /* It may be better to latch to AZ_D early if have shift (saves 1 bit) */
        if (state->mode != AZ_D && !charInCurrentTable && !(AztecFlags[ch] & AZ_D_F)
                && (fnc1 || (AztecFlags[ch] & (AZ_U_F | AZ_P_F)))) {
            const int shiftMode = AztecFlags[ch] & AZ_U_F ? AZ_U : AZ_P;
            const int shiftChar = fnc1 ? AZ_FNC1_VAL : AztecChar[shiftMode][ch];
            if (!az_DigitLatchShiftAndAppend(state, from, shiftMode, shiftChar, ret_list)) {
                return 0;
            }
        }
    }
    if (!fnc1 && (state->byteCount > 0 || !charInCurrentTable)) {
        /* It's never worthwhile to go into B/S mode if not already in B/S mode, and the char exists in current mode.
           That can never save bits over just outputting the char in the current mode */
        if (!az_AddByteShiftChar(state, from, -1 /*from2*/, ret_list)) {
            return 0;
        }
    }
    return 1;
}

/* Update a set of states for a Punct double - see `az_UpdateStateListForChar()` below */
static int az_UpdateStateListForPair(struct az_state_list *list, const int from, const int pairCode) {
    int i;
    struct az_state_list s_ret_list;
    struct az_state_list *ret_list = &s_ret_list;
    if (!az_state_list_init(ret_list, list->size)) {
        return 0;
    }
    for (i = 0; i < list->used; i++) {
        if (!az_UpdateStateForPair(list->states + i, from, pairCode, ret_list)) {
            az_state_list_free(ret_list);
            return 0;
        }
    }
    az_SimplifyStates(ret_list);

    az_state_list_free(list);
    list->states = ret_list->states;
    list->size = ret_list->size;
    list->used = ret_list->used;

    return 1;
}

/* Update a set of states for a new character by updating each state for the new character, merging the results,
   and then removing the non-optimal states */
static int az_UpdateStateListForChar(struct az_state_list *list, const unsigned char *source, const int from,
            const int gs1) {
    int i;
    struct az_state_list s_ret_list;
    struct az_state_list *ret_list = &s_ret_list;
    if (!az_state_list_init(ret_list, list->size)) {
        return 0;
    }
    for (i = 0; i < list->used; i++) {
        if (!az_UpdateStateForChar(list->states + i, source, from, gs1, ret_list)) {
            az_state_list_free(ret_list);
            return 0;
        }
    }
    if (ret_list->used > 1) {
        az_SimplifyStates(ret_list);
    }

    az_state_list_free(list);
    list->states = ret_list->states;
    list->size = ret_list->size;
    list->used = ret_list->used;

    return 1;
}

/* Default, optimized encodation algorithm by Frank Yellin and Rustam Abdullaev, adapted from ZXing via zxing-cpp's
   `HighLevelEncoder::Encode()` & slightly improved */
/* SPDX-License-Identifier: Apache-2.0 */
/* Note that a bitstream that is encoded to be shortest based on mode choices may not be so after bit-stuffing */
static int az_binary_string(unsigned char source[], const int length, int bp,
            char binary_string[AZTEC_MAP_POSN_MAX + 1], const int gs1, const char initial_mode,
            char *p_current_mode) {
    struct az_state_list s_state_list;
    struct az_state_list *list = &s_state_list;
    struct az_state stateEnd;
    int minStateIdx = -1;
    int minBitCount = INT_MAX;
    int i;
#ifndef NDEBUG
    const int entry_bp = bp;
#endif

    if (!az_state_list_init(list, length)) {
        return 0;
    }
    memset(list->states, 0, sizeof(struct az_state));
    list->states[list->used++].mode = initial_mode;

    for (i = 0; i < length; i++) {
#ifdef AZ_DEBUG_LIST
        fprintf(stderr, "index %d\n", (int) i);
#endif
        if (AZ_DOUBLE_PUNCT(source, length, i)) {
            /* "\r\n" -> 2, ". " -> 3, ", " -> 4, ": " -> 5 */
            const int pairCode = source[i] == '\r' ? 2 : 3 + 7 - ((source[i] & 0x0F) >> 1);
            if (!az_UpdateStateListForPair(list, i, pairCode)) {
                az_state_list_free(list);
                return 0;
            }
            i++;
        } else {
            if (!az_UpdateStateListForChar(list, source, i, gs1)) {
                az_state_list_free(list);
                return 0;
            }
        }
    }
    az_dump_list(list, "End ");

    for (i = 0; i < list->used; i++) {
        if (list->states[i].bitCount < minBitCount) {
            minStateIdx = i;
            minBitCount = list->states[i].bitCount;
        }
    }
    assert(minStateIdx >= 0);

    if (!az_EndByteShift(list->states + minStateIdx, &stateEnd, length)) {
        az_state_free(&stateEnd);
        az_state_list_free(list);
        return 0;
    }

    if (stateEnd.bitCount > AZTEC_BIN_CAPACITY) {
        az_state_free(&stateEnd);
        az_state_list_free(list);
        return stateEnd.bitCount;
    }

    for (i = 0; i < stateEnd.tokens.used; i++) {
        const struct az_token *token = stateEnd.tokens.tokens + i;
        const int count = token->count;
        if (count < 0) {
            bp = z_bin_append_posn(token->value, -count, binary_string, bp);
        } else {
            int j;
            for (j = 0; j < count; j++) {
                if (j == 0 || (j == 31 && count <= 62)) {
                    bp = z_bin_append_posn(31, 5, binary_string, bp); /* B/S */
                    if (count > 62) {
                        bp = z_bin_append_posn(count - 31, 16, binary_string, bp);
                    } else if (j == 0) {
                        bp = z_bin_append_posn(count <= 31 ? count : 31, 5, binary_string, bp);
                    } else {
                        bp = z_bin_append_posn(count - 31, 5, binary_string, bp);
                    }
                }
                bp = z_bin_append_posn(source[token->value + j], 8, binary_string, bp);
            }
        }
    }
    assert(bp - entry_bp == stateEnd.bitCount);

    *p_current_mode = stateEnd.mode;

    az_state_free(&stateEnd);
    az_state_list_free(list);

    return bp;
}

/* Calculate the binary size */
static int az_text_size(const char *modes, const unsigned char *source, int length, const int gs1, const int set_gs1,
            const int eci, const char initial_mode, const int eci_latch, int *byte_counts) {
    int i;
    int byte_i = 0;
    char current_mode = initial_mode;
    int size = 0;

    if (set_gs1 && gs1) {
        if (eci == 0) {
            current_mode = AZ_D;
            size += 5 + 4 + 5 + 3;
        } else {
            size += 5 + 5 + 3;
        }
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
                current_mode = AZ_P;
            } else {
                size += initial_mode == AZ_D ? 4 : 5;
            }
        }
        size += 5 + 3 + 4 + 4 * ((eci > 9) + (eci > 99) + (eci > 999) + (eci > 9999) + (eci > 99999));
    }
    for (i = 0; i < length; i++) {
        if (modes[i] != current_mode) {
            /* Change mode */
            const int mask_mode = AZ_MASK(modes[i]);
            if (mask_mode != current_mode) {
                size += AztecLatchNum[(int) current_mode][mask_mode];
                if (mask_mode != AZ_B) {
                    current_mode = mask_mode;
                } else if (current_mode == AZ_P || current_mode == AZ_D) {
                    current_mode = AZ_U;
                }
            }
            if (modes[i] & (AZ_PS | AZ_US)) {
                size += 4 + (mask_mode != AZ_D);
            }

            /* Byte mode - process full block here */
            if (modes[i] == AZ_B) {
                int big_batch = 0, count;
                for (count = 0; i + count < length && modes[i + count] == AZ_B; count++);

                if (count > 2047 + 2078) { /* Can't be more than 19968 / 8 = 2496 */
                    return 0;
                }
                byte_counts[byte_i++] = (short) count;

                if (count > 2047) { /* Max 11-bit number */
                    big_batch = count > 2078 ? 2078 : count;
                    /* Put 00000 followed by 11-bit number of bytes less 31 */
                    size += 16 + (big_batch << 3);
                    i += big_batch;
                    count -= big_batch;
                }
                if (count) {
                    if (big_batch) {
                        size += 5;
                    }
                    if (count > 62) {
                        assert(count <= 2078);
                        /* Put 00000 followed by 11-bit number of bytes less 31 */
                        size += 16;
                    } else {
                        if (count > 31) {
                            /* 2 5-bit B/Ss beats 1 11-bit */
                            size += 10 + (31 << 3);
                            count -= 31;
                            i += 31;
                        }
                        /* Put 5-bit number of bytes */
                        size += 5;
                    }
                    size += count << 3;
                    i += count;
                }
                i--;
                continue;
            }
        }

        if (modes[i] == AZ_P || (modes[i] & AZ_PS)) {
            size += 5;
            if (gs1 && source[i] == '\x1D') {
                size += 3;
            }
        } else if (modes[i] == AZ_D && !(modes[i] & AZ_US)) {
            size += 4;
        } else {
            size += 5;
        }
    }

    return size;
}

/* Determine encoding modes and encode */
static int az_text_process(unsigned char *source, int length, int bp, char *binary_string, const int gs1,
            const int gs1_bp, const int eci, const int fast_encode, char *p_current_mode, int *data_length,
            const int debug) {
    int i, j;
    int reduced_length = 0; /* Suppress gcc-14 warning -Wmaybe-uninitialized */
    char *modes = (char *) z_alloca(length + 1);
    int *byte_counts = (int *) z_alloca(sizeof(int) * length); /* Cache of Byte-run counts */
    int byte_i = 0;
    int size = 0;
    int eci_latch = 0;
    char current_mode = p_current_mode ? *p_current_mode : AZ_U;
    const char initial_mode = current_mode;
    const int set_gs1 = bp == gs1_bp;
    const int initial_bp = bp;
    const int all_byte_only_or_uld = az_all_byte_only_or_uld(source, length); /* -1 if not */
    const int debug_print = debug & ZINT_DEBUG_PRINT;
#ifdef ZINT_TEST
    const int debug_skip_all = debug & 2048; /* ZINT_DEBUG_TEST_AZTEC_SKIP_ALL - skip using `all_byte_only_or_uld` */
#else
    const int debug_skip_all = 0;
#endif

    if (set_gs1 && gs1) {
        assert(initial_mode == AZ_U);
        if (eci == 0) {
            /* Latch to D/L to save a bit */
            current_mode = AZ_D;
            bp = z_bin_append_posn(30, 5, binary_string, bp); /* D/L */
            bp = z_bin_append_posn(0, 4 + 5 + 3, binary_string, bp); /* P/S FLG(0) = FNC1 */
        } else {
            bp = z_bin_append_posn(0, 5 + 5 + 3, binary_string, bp); /* P/S FLG(0) = FNC1 */
        }
    /* See if it's worthwhile latching to AZ_P when have ECI */
    } else if ((debug_skip_all || all_byte_only_or_uld == -1) && eci && initial_mode != AZ_P
            && az_count_initial_puncts(source, length) > 2 + (initial_mode == AZ_D)) {
        assert(!gs1);
        eci_latch = 1;
        current_mode = AZ_P;
    }

    if (eci != 0) {
        const int flg = 1 + (eci > 9) + (eci > 99) + (eci > 999) + (eci > 9999) + (eci > 99999);
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
        bp = z_bin_append_posn(flg, 5 + 3, binary_string, bp); /* FLG(n) */
        if (flg == 1) {
            bp = z_bin_append_posn(2 + eci, 4, binary_string, bp);
        } else if (flg == 2) {
            bp = z_bin_append_posn(((2 + eci / 10) << 4) + 2 + eci % 10, 8, binary_string, bp);
        } else if (flg == 3) {
            bp = z_bin_append_posn(2 + eci / 100, 4, binary_string, bp);
            bp = z_bin_append_posn(((2 + eci % 100 / 10) << 4) + 2 + eci % 10, 8, binary_string, bp);
        } else if (flg == 4) {
            bp = z_bin_append_posn(((2 + eci / 1000) << 4) + 2 + eci % 1000 / 100, 8, binary_string, bp);
            bp = z_bin_append_posn(((2 + eci % 100 / 10) << 4) + 2 + eci % 10, 8, binary_string, bp);
        } else if (flg == 5) {
            bp = z_bin_append_posn(2 + eci / 10000, 4, binary_string, bp);
            bp = z_bin_append_posn(((2 + eci % 10000 / 1000) << 4) + 2 + eci % 1000 / 100, 8, binary_string, bp);
            bp = z_bin_append_posn(((2 + eci % 100 / 10) << 4) + 2 + eci % 10, 8, binary_string, bp);
        } else {
            bp = z_bin_append_posn(((2 + eci / 100000) << 4) + 2 + eci % 100000 / 10000, 8, binary_string, bp);
            bp = z_bin_append_posn(((2 + eci % 10000 / 1000) << 4) + 2 + eci % 1000 / 100, 8, binary_string, bp);
            bp = z_bin_append_posn(((2 + eci % 100 / 10) << 4) + 2 + eci % 10, 8, binary_string, bp);
        }
    }

    if (!debug_skip_all && all_byte_only_or_uld != -1) {
        memset(modes, all_byte_only_or_uld, length);
        reduced_length = length;
    } else if (fast_encode) {
        reduced_length = az_text_modes(modes, source, length, gs1, current_mode, debug_print);
    } else {
        bp = az_binary_string(source, length, bp, binary_string, gs1, current_mode, &current_mode);
        if (bp == 0 || bp > AZTEC_BIN_CAPACITY) {
            return bp == 0 ? ZINT_ERROR_MEMORY : ZINT_ERROR_TOO_LONG;
        }
        assert(bp > initial_bp);
        size = bp - initial_bp;
    }

    if (!size) {
        size = az_text_size(modes, source, reduced_length, gs1, set_gs1, eci, initial_mode, eci_latch, byte_counts);
        if (size == 0 || bp + size > AZTEC_BIN_CAPACITY) {
            return ZINT_ERROR_TOO_LONG;
        }

        for (i = 0; i < reduced_length; i++) {
            int current_mode_set = 0;
            if (modes[i] != current_mode) {
                /* Change mode */
                const int mask_mode = AZ_MASK(modes[i]);
                if (current_mode != mask_mode) {
                    assert(current_mode != AZ_B); /* Suppress clang-tidy-22 clang-analyzer-security.ArrayBound */
                    bp = z_bin_append_posn(AztecLatch[(int) current_mode][mask_mode],
                                            AztecLatchNum[(int) current_mode][mask_mode], binary_string, bp);
                    if (mask_mode == AZ_B && (current_mode == AZ_P || current_mode == AZ_D)) {
                        current_mode = AZ_U;
                        current_mode_set = 1;
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
                        if (count > 62) {
                            assert(count <= 2078);
                            /* Put 00000 followed by 11-bit number of bytes less 31 */
                            bp = z_bin_append_posn(count - 31, 16, binary_string, bp);
                        } else {
                            if (count > 31) {
                                /* 2 5-bit B/Ss beats 1 11-bit */
                                bp = z_bin_append_posn(31, 5, binary_string, bp); /* 5-bit byte count */
                                for (j = 0; j < 31; j++) {
                                    bp = z_bin_append_posn(source[i++], 8, binary_string, bp);
                                }
                                bp = z_bin_append_posn(31, 5, binary_string, bp); /* B/S */
                                count -= 31;
                            }
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

            if (modes[i] == AZ_P || (modes[i] & AZ_PS)) {
                if (gs1 && source[i] == '\x1D') {
                    bp = z_bin_append_posn(0, 5 + 3, binary_string, bp); /* FLG(0) = FNC1 */
                } else {
                    bp = z_bin_append_posn(AztecChar[AZ_P][source[i]], 5, binary_string, bp);
                }
            } else {
                const int char_mode = (modes[i] & AZ_US) ? AZ_U : modes[i];
                assert(char_mode != AZ_B); /* Suppress clang-tidy-22 clang-analyzer-security.ArrayBound */
                bp = z_bin_append_posn(AztecChar[char_mode][source[i]], 4 + (modes[i] != AZ_D), binary_string, bp);
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

    return 0;
}

/* Call `az_text_process()` for each segment */
static int az_text_process_segs(struct zint_symbol *symbol, struct zint_seg segs[], const int seg_count, int bp,
            char binary_string[], const int gs1, const int gs1_bp, int *data_length) {
    int i;
    char current_mode = AZ_U;
    const int fast_encode = symbol->input_mode & FAST_MODE;
    /* Raw text dealt with by `ZBarcode_Encode_Segs()`, except for `eci` feedback.
       Note not updating `eci` for GS1 mode as not converted */
    const int content_segs = !gs1 && (symbol->output_options & BARCODE_CONTENT_SEGS);

    for (i = 0; i < seg_count; i++) {
        int error_number;
        if ((error_number = az_text_process(segs[i].source, segs[i].length, bp, binary_string, gs1, gs1_bp,
                segs[i].eci, fast_encode, &current_mode, &bp, symbol->debug))) {
            return error_number;
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

    if ((i = z_segs_length(segs, seg_count)) > 4981) { /* Max is 4981 digits */
        return z_errtxtf(ZINT_ERROR_TOO_LONG, symbol, 803, "Input length %d too long (maximum 4981)", i);
    }

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
                                0 /*fast_encode*/, NULL /*p_current_mode*/, &bp, symbol->debug);
        /* Will be in U/L due to uppercase A-Z index/count indicators at end */
        gs1_bp = bp; /* Initial FNC1 (FLG0) position */
    }

    if ((error_number = az_text_process_segs(symbol, segs, seg_count, bp, binary_string, gs1, gs1_bp,
                                            &data_length))) {
        assert(error_number == ZINT_ERROR_TOO_LONG || error_number == ZINT_ERROR_MEMORY);
        if (error_number == ZINT_ERROR_TOO_LONG) {
            return z_errtxt(error_number, symbol, 502,
                            "Input too long, requires too many codewords (maximum " AZ_BIN_CAP_CWDS_S ")");
        }
        return z_errtxt(error_number, symbol, 804, "Insufficient memory for optimized encodation");
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
