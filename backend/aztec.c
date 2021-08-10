/* aztec.c - Handles Aztec 2D Symbols */

/*
    libzint - the open source barcode library
    Copyright (C) 2009-2021 Robin Stuart <rstuart114@gmail.com>

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

#include <assert.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"
#include "aztec.h"
#include "reedsol.h"

#define AZTEC_MAX_CAPACITY  19968 /* ISO/IEC 24778:2008 5.3 Table 1 Maximum Symbol Bit Capacity */
#define AZTEC_BIN_CAPACITY  17940 /* Above less 169 * 12 = 2028 bits (169 = 10% of 1664 + 3) */
#define AZTEC_MAP_SIZE      22801 /* AztecMap Version 32 151 x 151 */
#define AZTEC_MAP_POSN_MAX  20039 /* Maximum position index in AztecMap */

static int az_count_doubles(const unsigned char source[], int i, const int length) {
    int c = 0;

    while ((i + 1 < length) && ((source[i] == '.') || (source[i] == ',')) && (source[i + 1] == ' ')) {
        c++;
        i += 2;
    }

    return c;
}

static int az_count_dotcomma(const unsigned char source[], int i, const int length) {
    int c = 0;

    while (i < length && ((source[i] == '.') || (source[i] == ','))) {
        c++;
        i++;
    }

    return c;
}

static int az_count_chr(const unsigned char source[], int i, const int length, const unsigned char chr) {
    int c = 0;

    while (i < length && source[i] == chr) {
        c++;
        i++;
    }

    return c;
}

static char az_get_next_mode(const char encode_mode[], const int src_len, int i) {
    int current_mode = encode_mode[i];

    do {
        i++;
    } while ((i < src_len) && (encode_mode[i] == current_mode));
    if (i >= src_len) {
        return 'E';
    } else {
        return encode_mode[i];
    }
}

static int az_bin_append_posn(const int arg, const int length, char *binary, const int bin_posn) {

    if (bin_posn + length > AZTEC_BIN_CAPACITY) {
        return 0; /* Fail */
    }
    return bin_append_posn(arg, length, binary, bin_posn);
}

static int aztec_text_process(const unsigned char source[], int src_len, char binary_string[], const int gs1,
            const int eci, int *data_length, const int debug) {

    int i, j;
    char current_mode;
    int count;
    char next_mode;
    int reduced_length;
    int byte_mode = 0;
    int bp;

#ifndef _MSC_VER
    char encode_mode[src_len + 1];
    unsigned char reduced_source[src_len + 1];
    char reduced_encode_mode[src_len + 1];
#else
    char *encode_mode = (char *) _alloca(src_len + 1);
    unsigned char *reduced_source = (unsigned char *) _alloca(src_len + 1);
    char *reduced_encode_mode = (char *) _alloca(src_len + 1);
#endif

    for (i = 0; i < src_len; i++) {
        if (source[i] >= 128) {
            encode_mode[i] = 'B';
        } else {
            encode_mode[i] = AztecModes[(int) source[i]];
        }
    }

    // Deal first with letter combinations which can be combined to one codeword
    // Combinations are (CR LF) (. SP) (, SP) (: SP) in Punct mode
    current_mode = 'U';
    for (i = 0; i + 1 < src_len; i++) {
        // Combination (CR LF) should always be in Punct mode
        if ((source[i] == 13) && (source[i + 1] == 10)) {
            encode_mode[i] = 'P';
            encode_mode[i + 1] = 'P';

        // Combination (: SP) should always be in Punct mode
        } else if ((source[i] == ':') && (source[i + 1] == ' ')) {
            encode_mode[i + 1] = 'P';

        // Combinations (. SP) and (, SP) sometimes use fewer bits in Digit mode
        } else if (((source[i] == '.') || (source[i] == ',')) && (source[i + 1] == ' ') && (encode_mode[i] == 'X')) {
            count = az_count_doubles(source, i, src_len);
            next_mode = az_get_next_mode(encode_mode, src_len, i);

            if (current_mode == 'U') {
                if ((next_mode == 'D') && (count <= 5)) {
                    for (j = 0; j < (2 * count); j++) {
                        encode_mode[i + j] = 'D';
                    }
                }

            } else if (current_mode == 'L') {
                if ((next_mode == 'D') && (count <= 4)) {
                    for (j = 0; j < (2 * count); j++) {
                        encode_mode[i + j] = 'D';
                    }
                }

            } else if (current_mode == 'M') {
                if ((next_mode == 'D') && (count == 1)) {
                    encode_mode[i] = 'D';
                    encode_mode[i + 1] = 'D';
                }

            } else if (current_mode == 'D') {
                if ((next_mode != 'D') && (count <= 4)) {
                    for (j = 0; j < (2 * count); j++) {
                        encode_mode[i + j] = 'D';
                    }
                } else if ((next_mode == 'D') && (count <= 7)) {
                    for (j = 0; j < (2 * count); j++) {
                        encode_mode[i + j] = 'D';
                    }
                }
            }

            // Default is Punct mode
            if (encode_mode[i] == 'X') {
                encode_mode[i] = 'P';
                encode_mode[i + 1] = 'P';
            }
        }

        if ((encode_mode[i] != 'X') && (encode_mode[i] != 'B')) {
            current_mode = encode_mode[i];
        }
    }

    if (debug) {
        printf("First Pass:\n");
        printf("%.*s\n", src_len, encode_mode);
    }

    // Reduce two letter combinations to one codeword marked as [abcd] in Punct mode
    i = 0;
    j = 0;
    while (i < src_len) {
        if (i + 1 < src_len) {
            if ((source[i] == 13) && (source[i + 1] == 10)) { // CR LF
                reduced_source[j] = 'a';
                reduced_encode_mode[j] = encode_mode[i];
                i += 2;
            } else if ((source[i] == '.') && (source[i + 1] == ' ') && (encode_mode[i] == 'P')) {
                reduced_source[j] = 'b';
                reduced_encode_mode[j] = encode_mode[i];
                i += 2;
            } else if ((source[i] == ',') && (source[i + 1] == ' ') && (encode_mode[i] == 'P')) {
                reduced_source[j] = 'c';
                reduced_encode_mode[j] = encode_mode[i];
                i += 2;
            } else if ((source[i] == ':') && (source[i + 1] == ' ')) {
                reduced_source[j] = 'd';
                reduced_encode_mode[j] = encode_mode[i];
                i += 2;
            } else {
                reduced_source[j] = source[i];
                reduced_encode_mode[j] = encode_mode[i];
                i++;
            }
        } else {
            reduced_source[j] = source[i];
            reduced_encode_mode[j] = encode_mode[i];
            i++;
        }
        j++;
    }

    reduced_length = j;

    current_mode = 'U';
    for (i = 0; i < reduced_length; i++) {
        // Resolve Carriage Return (CR) which can be Punct or Mixed mode
        if (reduced_source[i] == 13) {
            count = az_count_chr(reduced_source, i, reduced_length, 13);
            next_mode = az_get_next_mode(reduced_encode_mode, reduced_length, i);

            if ((current_mode == 'U') && ((next_mode == 'U') || (next_mode == 'B')) && (count == 1)) {
                reduced_encode_mode[i] = 'P';

            } else if ((current_mode == 'L') && ((next_mode == 'L') || (next_mode == 'B')) && (count == 1)) {
                reduced_encode_mode[i] = 'P';

            } else if ((current_mode == 'P') || (next_mode == 'P')) {
                reduced_encode_mode[i] = 'P';
            }

            if (current_mode == 'D') {
                if (((next_mode == 'E') || (next_mode == 'U') || (next_mode == 'D') || (next_mode == 'B'))
                        && (count <= 2)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'P';
                    }
                } else if ((next_mode == 'L') && (count == 1)) {
                    reduced_encode_mode[i] = 'P';
                }
            }

            // Default is Mixed mode
            if (reduced_encode_mode[i] == 'X') {
                reduced_encode_mode[i] = 'M';
            }

        // Resolve full stop and comma which can be in Punct or Digit mode
        } else if ((reduced_source[i] == '.') || (reduced_source[i] == ',')) {
            count = az_count_dotcomma(reduced_source, i, reduced_length);
            next_mode = az_get_next_mode(reduced_encode_mode, reduced_length, i);

            if (current_mode == 'U') {
                if (((next_mode == 'U') || (next_mode == 'L') || (next_mode == 'M') || (next_mode == 'B'))
                        && (count == 1)) {
                    reduced_encode_mode[i] = 'P';
                }

            } else if (current_mode == 'L') {
                if ((next_mode == 'L') && (count <= 2)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'P';
                    }
                } else if (((next_mode == 'M') || (next_mode == 'B')) && (count == 1)) {
                    reduced_encode_mode[i] = 'P';
                }

            } else if (current_mode == 'M') {
                if (((next_mode == 'E') || (next_mode == 'U') || (next_mode == 'L') || (next_mode == 'M'))
                        && (count <= 4)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'P';
                    }
                } else if ((next_mode == 'B') && (count <= 2)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'P';
                    }
                }

            } else if ((current_mode == 'P') && (next_mode != 'D') && (count <= 9)) {
                for (j = 0; j < count; j++) {
                    reduced_encode_mode[i + j] = 'P';
                }
            }

            // Default is Digit mode
            if (reduced_encode_mode[i] == 'X') {
                reduced_encode_mode[i] = 'D';
            }

        // Resolve Space (SP) which can be any mode except Punct
        } else if (reduced_source[i] == ' ') {
            count = az_count_chr(reduced_source, i, reduced_length, ' ');
            next_mode = az_get_next_mode(reduced_encode_mode, reduced_length, i);

            if (current_mode == 'U') {
                if ((next_mode == 'E') && (count <= 5)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'U';
                    }
                } else if (((next_mode == 'U') || (next_mode == 'L') || (next_mode == 'M') || (next_mode == 'P')
                        || (next_mode == 'B')) && (count <= 9)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'U';
                    }
                }

            } else if (current_mode == 'L') {
                if ((next_mode == 'E') && (count <= 5)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'L';
                    }

                } else if ((next_mode == 'U') && (count == 1)) {
                    reduced_encode_mode[i] = 'L';

                } else if ((next_mode == 'L') && (count <= 14)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'L';
                    }

                } else if (((next_mode == 'M') || (next_mode == 'P') || (next_mode == 'B')) && (count <= 9)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'L';
                    }
                }

            } else if (current_mode == 'M') {
                if (((next_mode == 'E') || (next_mode == 'U')) && (count <= 9)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'M';
                    }

                } else if (((next_mode == 'L') || (next_mode == 'B')) && (count <= 14)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'M';
                    }

                } else if (((next_mode == 'M') || (next_mode == 'P')) && (count <= 19)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'M';
                    }
                }

            } else if (current_mode == 'P') {
                if ((next_mode == 'E') && (count <= 5)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'U';
                    }

                } else if (((next_mode == 'U') || (next_mode == 'L') || (next_mode == 'M') || (next_mode == 'P')
                        || (next_mode == 'B')) && (count <= 9)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'U';
                    }
                }
            }

            // Default is Digit mode
            if (reduced_encode_mode[i] == 'X') {
                reduced_encode_mode[i] = 'D';
            }
        }

        if (reduced_encode_mode[i] != 'B') {
            current_mode = reduced_encode_mode[i];
        }
    }

    // Decide when to use P/S instead of P/L and U/S instead of U/L
    current_mode = 'U';
    for (i = 0; i < reduced_length; i++) {

        if (reduced_encode_mode[i] != current_mode) {

            for (count = 0; ((i + count) < reduced_length)
                            && (reduced_encode_mode[i + count] == reduced_encode_mode[i]); count++);
            next_mode = az_get_next_mode(reduced_encode_mode, reduced_length, i);

            if (reduced_encode_mode[i] == 'P') {
                if ((current_mode == 'U') && (count <= 2)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'p';
                    }

                } else if ((current_mode == 'L') && (next_mode != 'U') && (count <= 2)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'p';
                    }

                } else if ((current_mode == 'L') && (next_mode == 'U') && (count == 1)) {
                    reduced_encode_mode[i] = 'p';

                } else if ((current_mode == 'M') && (next_mode != 'M') && (count == 1)) {
                    reduced_encode_mode[i] = 'p';

                } else if ((current_mode == 'M') && (next_mode == 'M') && (count <= 2)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'p';
                    }

                } else if ((current_mode == 'D') && (next_mode != 'D') && (count <= 3)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'p';
                    }

                } else if ((current_mode == 'D') && (next_mode == 'D') && (count <= 6)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'p';
                    }
                }

            } else if (reduced_encode_mode[i] == 'U') {
                if ((current_mode == 'L') && ((next_mode == 'L') || (next_mode == 'M')) && (count <= 2)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'u';
                    }

                } else if ((current_mode == 'L') && ((next_mode == 'E') || (next_mode == 'D') || (next_mode == 'B')
                        || (next_mode == 'P')) && (count == 1)) {
                    reduced_encode_mode[i] = 'u';

                } else if ((current_mode == 'D') && (next_mode == 'D') && (count == 1)) {
                    reduced_encode_mode[i] = 'u';

                } else if ((current_mode == 'D') && (next_mode == 'P') && (count <= 2)) {
                    for (j = 0; j < count; j++) {
                        reduced_encode_mode[i + j] = 'u';
                    }
                }
            }
        }

        if ((reduced_encode_mode[i] != 'p') && (reduced_encode_mode[i] != 'u') && (reduced_encode_mode[i] != 'B')) {
            current_mode = reduced_encode_mode[i];
        }
    }

    if (debug) {
        printf("%.*s\n", reduced_length, reduced_source);
        printf("%.*s\n", reduced_length, reduced_encode_mode);
    }

    bp = 0;

    if (gs1) {
        bp = bin_append_posn(0, 5, binary_string, bp); // P/S
        bp = bin_append_posn(0, 5, binary_string, bp); // FLG(n)
        bp = bin_append_posn(0, 3, binary_string, bp); // FLG(0)
    }

    if (eci != 0) {
        bp = bin_append_posn(0, 5, binary_string, bp); // P/S
        bp = bin_append_posn(0, 5, binary_string, bp); // FLG(n)
        if (eci < 10) {
            bp = bin_append_posn(1, 3, binary_string, bp); // FLG(1)
            bp = bin_append_posn(2 + eci, 4, binary_string, bp);
        } else if (eci <= 99) {
            bp = bin_append_posn(2, 3, binary_string, bp); // FLG(2)
            bp = bin_append_posn(2 + (eci / 10), 4, binary_string, bp);
            bp = bin_append_posn(2 + (eci % 10), 4, binary_string, bp);
        } else if (eci <= 999) {
            bp = bin_append_posn(3, 3, binary_string, bp); // FLG(3)
            bp = bin_append_posn(2 + (eci / 100), 4, binary_string, bp);
            bp = bin_append_posn(2 + ((eci % 100) / 10), 4, binary_string, bp);
            bp = bin_append_posn(2 + (eci % 10), 4, binary_string, bp);
        } else if (eci <= 9999) {
            bp = bin_append_posn(4, 3, binary_string, bp); // FLG(4)
            bp = bin_append_posn(2 + (eci / 1000), 4, binary_string, bp);
            bp = bin_append_posn(2 + ((eci % 1000) / 100), 4, binary_string, bp);
            bp = bin_append_posn(2 + ((eci % 100) / 10), 4, binary_string, bp);
            bp = bin_append_posn(2 + (eci % 10), 4, binary_string, bp);
        } else if (eci <= 99999) {
            bp = bin_append_posn(5, 3, binary_string, bp); // FLG(5)
            bp = bin_append_posn(2 + (eci / 10000), 4, binary_string, bp);
            bp = bin_append_posn(2 + ((eci % 10000) / 1000), 4, binary_string, bp);
            bp = bin_append_posn(2 + ((eci % 1000) / 100), 4, binary_string, bp);
            bp = bin_append_posn(2 + ((eci % 100) / 10), 4, binary_string, bp);
            bp = bin_append_posn(2 + (eci % 10), 4, binary_string, bp);
        } else {
            bp = bin_append_posn(6, 3, binary_string, bp); // FLG(6)
            bp = bin_append_posn(2 + (eci / 100000), 4, binary_string, bp);
            bp = bin_append_posn(2 + ((eci % 100000) / 10000), 4, binary_string, bp);
            bp = bin_append_posn(2 + ((eci % 10000) / 1000), 4, binary_string, bp);
            bp = bin_append_posn(2 + ((eci % 1000) / 100), 4, binary_string, bp);
            bp = bin_append_posn(2 + ((eci % 100) / 10), 4, binary_string, bp);
            bp = bin_append_posn(2 + (eci % 10), 4, binary_string, bp);
        }
    }

    current_mode = 'U';
    for (i = 0; i < reduced_length; i++) {

        if (reduced_encode_mode[i] != 'B') {
            byte_mode = 0;
        }

        if ((reduced_encode_mode[i] != current_mode) && (!byte_mode)) {
            // Change mode
            if (current_mode == 'U') {
                switch (reduced_encode_mode[i]) {
                    case 'L':
                        if (!(bp = az_bin_append_posn(28, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // L/L
                        break;
                    case 'M':
                        if (!(bp = az_bin_append_posn(29, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // M/L
                        break;
                    case 'P':
                        if (!(bp = az_bin_append_posn(29, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // M/L
                        if (!(bp = az_bin_append_posn(30, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // P/L
                        break;
                    case 'p':
                        if (!(bp = az_bin_append_posn(0, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // P/S
                        break;
                    case 'D':
                        if (!(bp = az_bin_append_posn(30, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // D/L
                        break;
                    case 'B':
                        if (!(bp = az_bin_append_posn(31, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // B/S
                        break;
                }
            } else if (current_mode == 'L') {
                switch (reduced_encode_mode[i]) {
                    case 'U':
                        if (!(bp = az_bin_append_posn(30, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // D/L
                        if (!(bp = az_bin_append_posn(14, 4, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        break;
                    case 'u':
                        if (!(bp = az_bin_append_posn(28, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/S
                        break;
                    case 'M':
                        if (!(bp = az_bin_append_posn(29, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // M/L
                        break;
                    case 'P':
                        if (!(bp = az_bin_append_posn(29, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // M/L
                        if (!(bp = az_bin_append_posn(30, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // P/L
                        break;
                    case 'p':
                        if (!(bp = az_bin_append_posn(0, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // P/S
                        break;
                    case 'D':
                        if (!(bp = az_bin_append_posn(30, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // D/L
                        break;
                    case 'B':
                        if (!(bp = az_bin_append_posn(31, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // B/S
                        break;
                }
            } else if (current_mode == 'M') {
                switch (reduced_encode_mode[i]) {
                    case 'U':
                        if (!(bp = az_bin_append_posn(29, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        break;
                    case 'L':
                        if (!(bp = az_bin_append_posn(28, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // L/L
                        break;
                    case 'P':
                        if (!(bp = az_bin_append_posn(30, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // P/L
                        break;
                    case 'p':
                        if (!(bp = az_bin_append_posn(0, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // P/S
                        break;
                    case 'D':
                        if (!(bp = az_bin_append_posn(29, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        if (!(bp = az_bin_append_posn(30, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // D/L
                        break;
                    case 'B':
                        if (!(bp = az_bin_append_posn(31, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // B/S
                        break;
                }
            } else if (current_mode == 'P') {
                switch (reduced_encode_mode[i]) {
                    case 'U':
                        if (!(bp = az_bin_append_posn(31, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        break;
                    case 'L':
                        if (!(bp = az_bin_append_posn(31, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        if (!(bp = az_bin_append_posn(28, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // L/L
                        break;
                    case 'M':
                        if (!(bp = az_bin_append_posn(31, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        if (!(bp = az_bin_append_posn(29, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // M/L
                        break;
                    case 'D':
                        if (!(bp = az_bin_append_posn(31, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        if (!(bp = az_bin_append_posn(30, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // D/L
                        break;
                    case 'B':
                        if (!(bp = az_bin_append_posn(31, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        current_mode = 'U';
                        if (!(bp = az_bin_append_posn(31, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // B/S
                        break;
                }
            } else if (current_mode == 'D') {
                switch (reduced_encode_mode[i]) {
                    case 'U':
                        if (!(bp = az_bin_append_posn(14, 4, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        break;
                    case 'u':
                        if (!(bp = az_bin_append_posn(15, 4, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/S
                        break;
                    case 'L':
                        if (!(bp = az_bin_append_posn(14, 4, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        if (!(bp = az_bin_append_posn(28, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // L/L
                        break;
                    case 'M':
                        if (!(bp = az_bin_append_posn(14, 4, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        if (!(bp = az_bin_append_posn(29, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // M/L
                        break;
                    case 'P':
                        if (!(bp = az_bin_append_posn(14, 4, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        if (!(bp = az_bin_append_posn(29, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // M/L
                        if (!(bp = az_bin_append_posn(30, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // P/L
                        break;
                    case 'p':
                        if (!(bp = az_bin_append_posn(0, 4, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // P/S
                        break;
                    case 'B':
                        if (!(bp = az_bin_append_posn(14, 4, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // U/L
                        current_mode = 'U';
                        if (!(bp = az_bin_append_posn(31, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // B/S
                        break;
                }
            }

            // Byte mode length descriptor
            if ((reduced_encode_mode[i] == 'B') && (!byte_mode)) {
                for (count = 0; ((i + count) < reduced_length) && (reduced_encode_mode[i + count] == 'B'); count++);

                if (count > 2079) {
                    return ZINT_ERROR_TOO_LONG;
                }

                if (count > 31) {
                    /* Put 00000 followed by 11-bit number of bytes less 31 */
                    if (!(bp = az_bin_append_posn(0, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG;
                    if (!(bp = az_bin_append_posn(count - 31, 11, binary_string, bp))) return ZINT_ERROR_TOO_LONG;
                } else {
                    /* Put 5-bit number of bytes */
                    if (!(bp = az_bin_append_posn(count, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG;
                }
                byte_mode = 1;
            }

            if ((reduced_encode_mode[i] != 'B') && (reduced_encode_mode[i] != 'u')
                    && (reduced_encode_mode[i] != 'p')) {
                current_mode = reduced_encode_mode[i];
            }
        }

        if ((reduced_encode_mode[i] == 'U') || (reduced_encode_mode[i] == 'u')) {
            if (reduced_source[i] == ' ') {
                if (!(bp = az_bin_append_posn(1, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // SP
            } else {
                if (!(bp = az_bin_append_posn(AztecSymbolChar[(int) reduced_source[i]], 5, binary_string, bp)))
                    return ZINT_ERROR_TOO_LONG;
            }
        } else if (reduced_encode_mode[i] == 'L') {
            if (reduced_source[i] == ' ') {
                if (!(bp = az_bin_append_posn(1, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // SP
            } else {
                if (!(bp = az_bin_append_posn(AztecSymbolChar[(int) reduced_source[i]], 5, binary_string, bp)))
                    return ZINT_ERROR_TOO_LONG;
            }
        } else if (reduced_encode_mode[i] == 'M') {
            if (reduced_source[i] == ' ') {
                if (!(bp = az_bin_append_posn(1, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // SP
            } else if (reduced_source[i] == 13) {
                if (!(bp = az_bin_append_posn(14, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // CR
            } else {
                if (!(bp = az_bin_append_posn(AztecSymbolChar[(int) reduced_source[i]], 5, binary_string, bp)))
                    return ZINT_ERROR_TOO_LONG;
            }
        } else if ((reduced_encode_mode[i] == 'P') || (reduced_encode_mode[i] == 'p')) {
            if (gs1 && (reduced_source[i] == '[')) {
                if (!(bp = az_bin_append_posn(0, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // FLG(n)
                if (!(bp = az_bin_append_posn(0, 3, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // FLG(0) = FNC1
            } else if (reduced_source[i] == 13) {
                if (!(bp = az_bin_append_posn(1, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // CR
            } else if (reduced_source[i] == 'a') {
                if (!(bp = az_bin_append_posn(2, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // CR LF
            } else if (reduced_source[i] == 'b') {
                if (!(bp = az_bin_append_posn(3, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // . SP
            } else if (reduced_source[i] == 'c') {
                if (!(bp = az_bin_append_posn(4, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // , SP
            } else if (reduced_source[i] == 'd') {
                if (!(bp = az_bin_append_posn(5, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // : SP
            } else if (reduced_source[i] == ',') {
                if (!(bp = az_bin_append_posn(17, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // Comma
            } else if (reduced_source[i] == '.') {
                if (!(bp = az_bin_append_posn(19, 5, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // Full stop
            } else {
                if (!(bp = az_bin_append_posn(AztecSymbolChar[(int) reduced_source[i]], 5, binary_string, bp)))
                    return ZINT_ERROR_TOO_LONG;
            }
        } else if (reduced_encode_mode[i] == 'D') {
            if (reduced_source[i] == ' ') {
                if (!(bp = az_bin_append_posn(1, 4, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // SP
            } else if (reduced_source[i] == ',') {
                if (!(bp = az_bin_append_posn(12, 4, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // Comma
            } else if (reduced_source[i] == '.') {
                if (!(bp = az_bin_append_posn(13, 4, binary_string, bp))) return ZINT_ERROR_TOO_LONG; // Full stop
            } else {
                if (!(bp = az_bin_append_posn(AztecSymbolChar[(int) reduced_source[i]], 4, binary_string, bp)))
                    return ZINT_ERROR_TOO_LONG;
            }
        } else if (reduced_encode_mode[i] == 'B') {
            if (!(bp = az_bin_append_posn(reduced_source[i], 8, binary_string, bp))) return ZINT_ERROR_TOO_LONG;
        }
    }

    if (debug) {
        printf("Binary String:\n");
        printf("%.*s\n", bp, binary_string);
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
    int layer, n, i;
    int x, y;
    const int offset = AztecOffset[layers - 1];
    const int endoffset = 151 - offset;

    for (layer = 0; layer < layers; layer++) {
        const int start = (112 * layer) + (16 * layer * layer) + 2;
        const int length = 28 + (layer * 4) + (layer + 1) * 4;
        int av0, av1;
        /* Top */
        i = 0;
        x = 64 - (layer * 2);
        y = 63 - (layer * 2);
        av0 = az_avoidReferenceGrid(y) * 151;
        av1 = az_avoidReferenceGrid(y - 1) * 151;
        for (n = start; n < (start + length); n += 2) {
            int avxi = az_avoidReferenceGrid(x + i);
            AztecMap[av0 + avxi] = n;
            AztecMap[av1 + avxi] = n + 1;
            i++;
        }
        /* Right */
        i = 0;
        x = 78 + (layer * 2);
        y = 64 - (layer * 2);
        av0 = az_avoidReferenceGrid(x);
        av1 = az_avoidReferenceGrid(x + 1);
        for (n = start + length; n < (start + (length * 2)); n += 2) {
            int avyi = az_avoidReferenceGrid(y + i) * 151;
            AztecMap[avyi + av0] = n;
            AztecMap[avyi + av1] = n + 1;
            i++;
        }
        /* Bottom */
        i = 0;
        x = 77 + (layer * 2);
        y = 78 + (layer * 2);
        av0 = az_avoidReferenceGrid(y) * 151;
        av1 = az_avoidReferenceGrid(y + 1) * 151;
        for (n = start + (length * 2); n < (start + (length * 3)); n += 2) {
            int avxi = az_avoidReferenceGrid(x - i);
            AztecMap[av0 + avxi] = n;
            AztecMap[av1 + avxi] = n + 1;
            i++;
        }
        /* Left */
        i = 0;
        x = 63 - (layer * 2);
        y = 77 + (layer * 2);
        av0 = az_avoidReferenceGrid(x);
        av1 = az_avoidReferenceGrid(x - 1);
        for (n = start + (length * 3); n < (start + (length * 4)); n += 2) {
            int avyi = az_avoidReferenceGrid(y - i) * 151;
            AztecMap[avyi + av0] = n;
            AztecMap[avyi + av1] = n + 1;
            i++;
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

INTERNAL int aztec(struct zint_symbol *symbol, unsigned char source[], int length) {
    int x, y, i, j, p, data_blocks, ecc_blocks, layers, total_bits;
    char bit_pattern[AZTEC_MAP_POSN_MAX + 1]; /* Note AZTEC_MAP_POSN_MAX > AZTEC_BIN_CAPACITY */
    /* To lessen stack usage, share binary_string buffer with bit_pattern, as accessed separately */
    char *binary_string = bit_pattern;
    char descriptor[42];
    char adjusted_string[AZTEC_MAX_CAPACITY];
    short AztecMap[AZTEC_MAP_SIZE];
    unsigned char desc_data[4], desc_ecc[6];
    int error_number, compact, data_length, data_maxsize, codeword_size, adjusted_length;
    int remainder, padbits, count, gs1, adjustment_size;
    int debug = (symbol->debug & ZINT_DEBUG_PRINT), reader = 0;
    int comp_loop = 4;
    rs_t rs;
    rs_uint_t rs_uint;

#ifdef _MSC_VER
    unsigned int *data_part;
    unsigned int *ecc_part;
#endif

    if ((symbol->input_mode & 0x07) == GS1_MODE) {
        gs1 = 1;
    } else {
        gs1 = 0;
    }
    if (symbol->output_options & READER_INIT) {
        reader = 1;
        comp_loop = 1;
    }
    if (gs1 && reader) {
        strcpy(symbol->errtxt, "501: Cannot encode in GS1 and Reader Initialisation mode at the same time");
        return ZINT_ERROR_INVALID_OPTION;
    }

    error_number = aztec_text_process(source, length, binary_string, gs1, symbol->eci, &data_length, debug);

    if (error_number != 0) {
        strcpy(symbol->errtxt, "502: Input too long or too many extended ASCII characters");
        return error_number;
    }
    assert(data_length > 0); /* Suppress clang-tidy warning: clang-analyzer-core.UndefinedBinaryOperatorResult */

    if (!((symbol->option_1 >= -1) && (symbol->option_1 <= 4))) {
        strcpy(symbol->errtxt, "503: Invalid error correction level - using default instead");
        if (symbol->warn_level == WARN_FAIL_ALL) {
            return ZINT_ERROR_INVALID_OPTION;
        }
        error_number = ZINT_WARN_INVALID_OPTION;
        symbol->option_1 = -1;
    }

    data_maxsize = 0; /* Keep compiler happy! */
    adjustment_size = 0;
    if (symbol->option_2 == 0) { /* The size of the symbol can be determined by Zint */
        int ecc_level = symbol->option_1;

        if ((ecc_level == -1) || (ecc_level == 0)) {
            ecc_level = 2;
        }

        do {
            /* Decide what size symbol to use - the smallest that fits the data */
            compact = 0; /* 1 = Aztec Compact, 0 = Normal Aztec */
            layers = 0;

            switch (ecc_level) {
                    /* For each level of error correction work out the smallest symbol which
                    the data will fit in */
                case 1: for (i = 32; i > 0; i--) {
                        if ((data_length + adjustment_size) < Aztec10DataSizes[i - 1]) {
                            layers = i;
                            compact = 0;
                            data_maxsize = Aztec10DataSizes[i - 1];
                        }
                    }
                    for (i = comp_loop; i > 0; i--) {
                        if ((data_length + adjustment_size) < AztecCompact10DataSizes[i - 1]) {
                            layers = i;
                            compact = 1;
                            data_maxsize = AztecCompact10DataSizes[i - 1];
                        }
                    }
                    break;
                case 2: for (i = 32; i > 0; i--) {
                        if ((data_length + adjustment_size) < Aztec23DataSizes[i - 1]) {
                            layers = i;
                            compact = 0;
                            data_maxsize = Aztec23DataSizes[i - 1];
                        }
                    }
                    for (i = comp_loop; i > 0; i--) {
                        if ((data_length + adjustment_size) < AztecCompact23DataSizes[i - 1]) {
                            layers = i;
                            compact = 1;
                            data_maxsize = AztecCompact23DataSizes[i - 1];
                        }
                    }
                    break;
                case 3: for (i = 32; i > 0; i--) {
                        if ((data_length + adjustment_size) < Aztec36DataSizes[i - 1]) {
                            layers = i;
                            compact = 0;
                            data_maxsize = Aztec36DataSizes[i - 1];
                        }
                    }
                    for (i = comp_loop; i > 0; i--) {
                        if ((data_length + adjustment_size) < AztecCompact36DataSizes[i - 1]) {
                            layers = i;
                            compact = 1;
                            data_maxsize = AztecCompact36DataSizes[i - 1];
                        }
                    }
                    break;
                case 4: for (i = 32; i > 0; i--) {
                        if ((data_length + adjustment_size) < Aztec50DataSizes[i - 1]) {
                            layers = i;
                            compact = 0;
                            data_maxsize = Aztec50DataSizes[i - 1];
                        }
                    }
                    for (i = comp_loop; i > 0; i--) {
                        if ((data_length + adjustment_size) < AztecCompact50DataSizes[i - 1]) {
                            layers = i;
                            compact = 1;
                            data_maxsize = AztecCompact50DataSizes[i - 1];
                        }
                    }
                    break;
            }

            if (layers == 0) { /* Couldn't find a symbol which fits the data */
                strcpy(symbol->errtxt, "504: Input too long (too many bits for selected ECC)");
                return ZINT_ERROR_TOO_LONG;
            }

            /* Determine codeword bitlength - Table 3 */
            if (layers <= 2) {
                codeword_size = 6;
            } else if (layers <= 8) {
                codeword_size = 8;
            } else if (layers <= 22) {
                codeword_size = 10;
            } else {
                codeword_size = 12;
            }

            j = 0;
            count = 0;
            for (i = 0; i < data_length; i++) {
                if ((j + 1) % codeword_size == 0) {
                    // Last bit of codeword
                    /* 7.3.1.2 "whenever the first B-1 bits ... are all “0”s, then a dummy “1” is inserted..."
                     * "Similarly a message codeword that starts with B-1 “1”s has a dummy “0” inserted..." */

                    if (count == 0 || count == (codeword_size - 1)) {
                        // Codeword of B-1 '0's or B-1 '1's
                        adjusted_string[j] = count == 0 ? '1' : '0';
                        j++;
                        count = binary_string[i] == '1' ? 1 : 0;
                    } else {
                        count = 0;
                    }

                } else if (binary_string[i] == '1') { /* Skip B so only counting B-1 */
                    count++;
                }

                adjusted_string[j] = binary_string[i];
                j++;
            }
            adjusted_length = j;
            adjustment_size = adjusted_length - data_length;

            /* Add padding */
            remainder = adjusted_length % codeword_size;

            padbits = codeword_size - remainder;
            if (padbits == codeword_size) {
                padbits = 0;
            }
            if (debug) printf("Remainder: %d  Pad bits: %d\n", remainder, padbits);

            for (i = 0; i < padbits; i++) {
                adjusted_string[adjusted_length++] = '1';
            }

            count = 0;
            for (i = (adjusted_length - codeword_size); i < adjusted_length; i++) {
                if (adjusted_string[i] == '1') {
                    count++;
                }
            }
            if (count == codeword_size) {
                adjusted_string[adjusted_length - 1] = '0';
            }

            if (debug) {
                printf("Codewords:\n");
                for (i = 0; i < (adjusted_length / codeword_size); i++) {
                    for (j = 0; j < codeword_size; j++) {
                        printf("%c", adjusted_string[(i * codeword_size) + j]);
                    }
                    printf(" ");
                }
                printf("\n");
            }

        } while (adjusted_length > data_maxsize);
        /* This loop will only repeat on the rare occasions when the rule about not having all 1s or all 0s
        means that the binary string has had to be lengthened beyond the maximum number of bits that can
        be encoded in a symbol of the selected size */

    } else { /* The size of the symbol has been specified by the user */
        if ((symbol->option_2 < 0) || (symbol->option_2 > 36)) {
            strcpy(symbol->errtxt, "510: Invalid Aztec Code size");
            return ZINT_ERROR_INVALID_OPTION;
        }
        if ((reader == 1) && ((symbol->option_2 >= 2) && (symbol->option_2 <= 4))) {
            symbol->option_2 = 5;
        }
        if (symbol->option_2 <= 4) {
            compact = 1;
            layers = symbol->option_2;
        } else {
            compact = 0;
            layers = symbol->option_2 - 4;
        }

        /* Determine codeword bitlength - Table 3 */
        if (layers <= 2) {
            codeword_size = 6;
        } else if (layers <= 8) {
            codeword_size = 8;
        } else if (layers <= 22) {
            codeword_size = 10;
        } else {
            codeword_size = 12;
        }

        j = 0;
        count = 0;
        for (i = 0; i < data_length; i++) {

            if ((j + 1) % codeword_size == 0) {
                // Last bit of codeword

                if (count == 0 || count == (codeword_size - 1)) {
                    // Codeword of B-1 '0's or B-1 '1's
                    adjusted_string[j] = count == 0 ? '1' : '0';
                    j++;
                    count = binary_string[i] == '1' ? 1 : 0;
                } else {
                    count = 0;
                }

            } else if (binary_string[i] == '1') { /* Skip B so only counting B-1 */
                count++;
            }

            adjusted_string[j] = binary_string[i];
            j++;
        }
        adjusted_length = j;

        remainder = adjusted_length % codeword_size;

        padbits = codeword_size - remainder;
        if (padbits == codeword_size) {
            padbits = 0;
        }
        if (debug) printf("Remainder: %d  Pad bits: %d\n", remainder, padbits);

        for (i = 0; i < padbits; i++) {
            adjusted_string[adjusted_length++] = '1';
        }

        count = 0;
        for (i = (adjusted_length - codeword_size); i < adjusted_length; i++) {
            if (adjusted_string[i] == '1') {
                count++;
            }
        }
        if (count == codeword_size) {
            adjusted_string[adjusted_length - 1] = '0';
        }

        /* Check if the data actually fits into the selected symbol size */
        if (compact) {
            data_maxsize = codeword_size * (AztecCompactSizes[layers - 1] - 3);
        } else {
            data_maxsize = codeword_size * (AztecSizes[layers - 1] - 3);
        }

        if (adjusted_length > data_maxsize) {
            strcpy(symbol->errtxt, "505: Data too long for specified Aztec Code symbol size");
            return ZINT_ERROR_TOO_LONG;
        }

        if (debug) {
            printf("Codewords:\n");
            for (i = 0; i < (adjusted_length / codeword_size); i++) {
                printf("%.*s ", codeword_size, adjusted_string + i * codeword_size);
            }
            printf("\n");
        }

    }

    if (reader && (layers > 22)) {
        strcpy(symbol->errtxt, "506: Data too long for reader initialisation symbol");
        return ZINT_ERROR_TOO_LONG;
    }

    data_blocks = adjusted_length / codeword_size;

    if (compact) {
        ecc_blocks = AztecCompactSizes[layers - 1] - data_blocks;
    } else {
        ecc_blocks = AztecSizes[layers - 1] - data_blocks;
    }

    if (debug) {
        printf("Generating a %s symbol with %d layers\n", compact ? "compact" : "full-size", layers);
        printf("Requires %d", compact ? AztecCompactSizes[layers - 1] : AztecSizes[layers - 1]);
        printf(" codewords of %d-bits\n", codeword_size);
        printf("    (%d data words, %d ecc words)\n", data_blocks, ecc_blocks);
    }

#ifndef _MSC_VER
    unsigned int data_part[data_blocks], ecc_part[ecc_blocks];
#else
    data_part = (unsigned int *) _alloca(sizeof(unsigned int) * data_blocks);
    ecc_part = (unsigned int *) _alloca(sizeof(unsigned int) * ecc_blocks);
#endif
    /* Copy across data into separate integers */
    memset(data_part, 0, sizeof(unsigned int) * data_blocks);
    memset(ecc_part, 0, sizeof(unsigned int) * ecc_blocks);

    /* Split into codewords and calculate reed-solomon error correction codes */
    for (i = 0; i < data_blocks; i++) {
        for (p = 0; p < codeword_size; p++) {
            if (adjusted_string[i * codeword_size + p] == '1') {
                data_part[i] += 0x01 << (codeword_size - (p + 1));
            }
        }
    }

    switch (codeword_size) {
        case 6:
            rs_init_gf(&rs, 0x43);
            rs_init_code(&rs, ecc_blocks, 1);
            rs_encode_uint(&rs, data_blocks, data_part, ecc_part);
            break;
        case 8:
            rs_init_gf(&rs, 0x12d);
            rs_init_code(&rs, ecc_blocks, 1);
            rs_encode_uint(&rs, data_blocks, data_part, ecc_part);
            break;
        case 10:
            if (!rs_uint_init_gf(&rs_uint, 0x409, 1023)) { /* Can fail on malloc() */
                strcpy(symbol->errtxt, "500: Insufficient memory for Reed-Solomon log tables");
                return ZINT_ERROR_MEMORY;
            }
            rs_uint_init_code(&rs_uint, ecc_blocks, 1);
            rs_uint_encode(&rs_uint, data_blocks, data_part, ecc_part);
            rs_uint_free(&rs_uint);
            break;
        case 12:
            if (!rs_uint_init_gf(&rs_uint, 0x1069, 4095)) { /* Can fail on malloc() */
                /* Note using AUSPOST error nos range as out of 50x ones & 51x taken by CODEONE */
                strcpy(symbol->errtxt, "400: Insufficient memory for Reed-Solomon log tables");
                return ZINT_ERROR_MEMORY;
            }
            rs_uint_init_code(&rs_uint, ecc_blocks, 1);
            rs_uint_encode(&rs_uint, data_blocks, data_part, ecc_part);
            rs_uint_free(&rs_uint);
            break;
    }

    for (i = (ecc_blocks - 1); i >= 0; i--) {
        adjusted_length = bin_append_posn(ecc_part[i], codeword_size, adjusted_string, adjusted_length);
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
        if ((layers - 1) & 0x02) {
            descriptor[0] = '1';
        } else {
            descriptor[0] = '0';
        }
        if ((layers - 1) & 0x01) {
            descriptor[1] = '1';
        } else {
            descriptor[1] = '0';
        }
        /* The next 6 bits represent the number of data blocks minus 1 */
        if (reader) {
            descriptor[2] = '1';
        } else {
            if ((data_blocks - 1) & 0x20) {
                descriptor[2] = '1';
            } else {
                descriptor[2] = '0';
            }
        }

        for (i = 3; i < 8; i++) {
            if ((data_blocks - 1) & (0x10 >> (i - 3))) {
                descriptor[i] = '1';
            } else {
                descriptor[i] = '0';
            }
        }
        if (debug) printf("Mode Message = %.8s\n", descriptor);
    } else {
        /* The first 5 bits represent the number of layers minus 1 */
        for (i = 0; i < 5; i++) {
            if ((layers - 1) & (0x10 >> i)) {
                descriptor[i] = '1';
            } else {
                descriptor[i] = '0';
            }
        }

        /* The next 11 bits represent the number of data blocks minus 1 */
        if (reader) {
            descriptor[5] = '1';
        } else {
            if ((data_blocks - 1) & 0x400) {
                descriptor[5] = '1';
            } else {
                descriptor[5] = '0';
            }
        }
        for (i = 6; i < 16; i++) {
            if ((data_blocks - 1) & (0x200 >> (i - 6))) {
                descriptor[i] = '1';
            } else {
                descriptor[i] = '0';
            }
        }
        if (debug) printf("Mode Message = %.16s\n", descriptor);
    }

    /* Split into 4-bit codewords */
    for (i = 0; i < 4; i++) {
        if (descriptor[i * 4] == '1') {
            desc_data[i] += 8;
        }
        if (descriptor[(i * 4) + 1] == '1') {
            desc_data[i] += 4;
        }
        if (descriptor[(i * 4) + 2] == '1') {
            desc_data[i] += 2;
        }
        if (descriptor[(i * 4) + 3] == '1') {
            desc_data[i] += 1;
        }
    }

    /* Add reed-solomon error correction with Galois field GF(16) and prime modulus
    x^4 + x + 1 (section 7.2.3)*/

    rs_init_gf(&rs, 0x13);
    if (compact) {
        rs_init_code(&rs, 5, 1);
        rs_encode(&rs, 2, desc_data, desc_ecc);
        for (i = 0; i < 5; i++) {
            if (desc_ecc[4 - i] & 0x08) {
                descriptor[(i * 4) + 8] = '1';
            } else {
                descriptor[(i * 4) + 8] = '0';
            }
            if (desc_ecc[4 - i] & 0x04) {
                descriptor[(i * 4) + 9] = '1';
            } else {
                descriptor[(i * 4) + 9] = '0';
            }
            if (desc_ecc[4 - i] & 0x02) {
                descriptor[(i * 4) + 10] = '1';
            } else {
                descriptor[(i * 4) + 10] = '0';
            }
            if (desc_ecc[4 - i] & 0x01) {
                descriptor[(i * 4) + 11] = '1';
            } else {
                descriptor[(i * 4) + 11] = '0';
            }
        }
    } else {
        rs_init_code(&rs, 6, 1);
        rs_encode(&rs, 4, desc_data, desc_ecc);
        for (i = 0; i < 6; i++) {
            if (desc_ecc[5 - i] & 0x08) {
                descriptor[(i * 4) + 16] = '1';
            } else {
                descriptor[(i * 4) + 16] = '0';
            }
            if (desc_ecc[5 - i] & 0x04) {
                descriptor[(i * 4) + 17] = '1';
            } else {
                descriptor[(i * 4) + 17] = '0';
            }
            if (desc_ecc[5 - i] & 0x02) {
                descriptor[(i * 4) + 18] = '1';
            } else {
                descriptor[(i * 4) + 18] = '0';
            }
            if (desc_ecc[5 - i] & 0x01) {
                descriptor[(i * 4) + 19] = '1';
            } else {
                descriptor[(i * 4) + 19] = '0';
            }
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
        int offset = AztecCompactOffset[layers - 1];
        int end_offset = 27 - offset;
        for (y = offset; y < end_offset; y++) {
            int y_map = y * 27;
            for (x = offset; x < end_offset; x++) {
                int map = CompactAztecMap[y_map + x];
                if (map == 1) {
                    set_module(symbol, y - offset, x - offset);
                } else if (map >= 2 && bit_pattern[map - 2] == '1') {
                    set_module(symbol, y - offset, x - offset);
                }
            }
            symbol->row_height[y - offset] = 1;
        }
        symbol->height = 27 - (2 * offset);
        symbol->rows = 27 - (2 * offset);
        symbol->width = 27 - (2 * offset);
    } else {
        int offset = AztecOffset[layers - 1];
        int end_offset = 151 - offset;
        az_populate_map(AztecMap, layers);
        for (y = offset; y < end_offset; y++) {
            int y_map = y * 151;
            for (x = offset; x < end_offset; x++) {
                int map = AztecMap[y_map + x];
                if (map == 1) {
                    set_module(symbol, y - offset, x - offset);
                } else if (map >= 2 && bit_pattern[map - 2] == '1') {
                    set_module(symbol, y - offset, x - offset);
                }
            }
            symbol->row_height[y - offset] = 1;
        }
        symbol->height = 151 - (2 * offset);
        symbol->rows = 151 - (2 * offset);
        symbol->width = 151 - (2 * offset);
    }

    return error_number;
}

/* Encodes Aztec runes as specified in ISO/IEC 24778:2008 Annex A */
INTERNAL int aztec_runes(struct zint_symbol *symbol, unsigned char source[], int length) {
    unsigned int input_value;
    int error_number, i, y, x, r;
    char binary_string[28];
    unsigned char data_codewords[3], ecc_codewords[6];
    int bp = 0;
    int debug = symbol->debug & ZINT_DEBUG_PRINT;
    rs_t rs;

    input_value = 0;
    if (length > 3) {
        strcpy(symbol->errtxt, "507: Input too large (3 character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }
    error_number = is_sane(NEON, source, length);
    if (error_number != 0) {
        strcpy(symbol->errtxt, "508: Invalid character in data (digits only)");
        return ZINT_ERROR_INVALID_DATA;
    }
    switch (length) {
        case 3: input_value = 100 * ctoi(source[0]);
            input_value += 10 * ctoi(source[1]);
            input_value += ctoi(source[2]);
            break;
        case 2: input_value = 10 * ctoi(source[0]);
            input_value += ctoi(source[1]);
            break;
        case 1: input_value = ctoi(source[0]);
            break;
    }

    if (input_value > 255) {
        strcpy(symbol->errtxt, "509: Input out of range (0 to 255)");
        return ZINT_ERROR_INVALID_DATA;
    }

    bp = bin_append_posn(input_value, 8, binary_string, bp);

    data_codewords[0] = (unsigned char) (input_value >> 4);
    data_codewords[1] = (unsigned char) (input_value & 0xF);

    rs_init_gf(&rs, 0x13);
    rs_init_code(&rs, 5, 1);
    rs_encode(&rs, 2, data_codewords, ecc_codewords);

    for (i = 0; i < 5; i++) {
        bp = bin_append_posn(ecc_codewords[4 - i], 4, binary_string, bp);
    }

    for (i = 0; i < 28; i += 2) {
        if (binary_string[i] == '1') {
            binary_string[i] = '0';
        } else {
            binary_string[i] = '1';
        }
    }

    if (debug) {
        printf("Binary String: %.28s\n", binary_string);
    }

    for (y = 8; y < 19; y++) {
        r = y * 27;
        for (x = 8; x < 19; x++) {
            if (CompactAztecMap[r + x] == 1) {
                set_module(symbol, y - 8, x - 8);
            } else if (CompactAztecMap[r + x] && binary_string[CompactAztecMap[r + x] - 2000] == '1') {
                set_module(symbol, y - 8, x - 8);
            }
        }
        symbol->row_height[y - 8] = 1;
    }
    symbol->height = 11;
    symbol->rows = 11;
    symbol->width = 11;

    return 0;
}
