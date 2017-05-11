/* aztec.c - Handles Aztec 2D Symbols */

/*
    libzint - the open source barcode library
    Copyright (C) 2009-2017 Robin Stuart <rstuart114@gmail.com>

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <malloc.h> 
#endif
#include "common.h"
#include "aztec.h"
#include "reedsol.h"

static int AztecMap[22801];

/**
 * Shorten the string by one character
 */
static void mapshorten(int *charmap, int *typemap, const int start, const int length) {
    memmove(charmap + start + 1, charmap + start + 2, (length - 1) * sizeof (int));
    memmove(typemap + start + 1, typemap + start + 2, (length - 1) * sizeof (int));
}

/**
 * Insert a character into the middle of a string at position posn
 */
/*
static void insert(char binary_string[], const size_t posn, const char newbit) {
    size_t i, end;

    end = strlen(binary_string);
    for (i = end; i > posn; i--) {
        binary_string[i] = binary_string[i - 1];
    }
    binary_string[posn] = newbit;
}
*/

/**
 * Encode input data into a binary string
 */
static int aztec_text_process(const unsigned char source[], const unsigned int src_len, char binary_string[], const int gs1, const int eci, int debug) {
    int i, j, k, p, bytes;
    int curtable, newtable, lasttable, chartype, maplength, blocks;
#ifndef _MSC_VER
    int charmap[src_len * 2], typemap[src_len * 2];
    int blockmap[2][src_len];
#else
    int* charmap = (int*) _alloca(src_len * 2 * sizeof (int));
    int* typemap = (int*) _alloca(src_len * 2 * sizeof (int));
    int* blockmap[2];
    blockmap[0] = (int*) _alloca(src_len * sizeof (int));
    blockmap[1] = (int*) _alloca(src_len * sizeof (int));
#endif
    /* Lookup input string in encoding table */
    maplength = 0;

    if (gs1) {
        /* Add FNC1 to beginning of GS1 messages */
        charmap[maplength] = 0;
        typemap[maplength++] = PUNC;
        charmap[maplength] = 400;
        typemap[maplength++] = PUNC;
    } else if (eci != 3) {
        /* Set ECI mode */
        charmap[maplength] = 0;
        typemap[maplength++] = PUNC;
        if (eci < 10) {
            charmap[maplength] = 401; // FLG(1)
            typemap[maplength++] = PUNC;
            charmap[maplength] = 502 + eci;
            typemap[maplength++] = PUNC;
        } else {
            charmap[maplength] = 402; // FLG(2)
            typemap[maplength++] = PUNC;
            charmap[maplength] = 502 + (eci / 10);
            typemap[maplength++] = PUNC;
            charmap[maplength] = 502 + (eci % 10);
            typemap[maplength++] = PUNC;
        }

    }

    for (i = 0; i < (int) src_len; i++) {
        if ((gs1) && (source[i] == '[')) {
            /* FNC1 represented by FLG(0) */
            charmap[maplength] = 0;
            typemap[maplength++] = PUNC;
            charmap[maplength] = 400;
            typemap[maplength++] = PUNC;
        } else {
            if ((source[i] > 127) || (source[i] == 0)) {
                charmap[maplength] = source[i];
                typemap[maplength++] = BINARY;
            } else {
                charmap[maplength] = AztecSymbolChar[source[i]];
                typemap[maplength++] = AztecCodeSet[source[i]];
            }
        }
    }

    /* Look for double character encoding possibilities */
    i = 0;
    do {
        if (((charmap[i] == 300) && (charmap[i + 1] == 11))
                && ((typemap[i] == PUNC) && (typemap[i + 1] == PUNC))) {
            /* CR LF combination */
            charmap[i] = 2;
            typemap[i] = PUNC;
            mapshorten(charmap, typemap, i, maplength);
            maplength--;
        }

        if (((charmap[i] == 302) && (charmap[i + 1] == 1))
                && ((typemap[i] == 24) && (typemap[i + 1] == 23))) {
            /* . SP combination */
            charmap[i] = 3;
            typemap[i] = PUNC;
            mapshorten(charmap, typemap, i, maplength);
            maplength--;
        }

        if (((charmap[i] == 301) && (charmap[i + 1] == 1))
                && ((typemap[i] == 24) && (typemap[i + 1] == 23))) {
            /* , SP combination */
            charmap[i] = 4;
            typemap[i] = PUNC;
            mapshorten(charmap, typemap, i, maplength);
            maplength--;
        }

        if (((charmap[i] == 21) && (charmap[i + 1] == 1))
                && ((typemap[i] == PUNC) && (typemap[i + 1] == 23))) {
            /* : SP combination */
            charmap[i] = 5;
            typemap[i] = PUNC;
            mapshorten(charmap, typemap, i, maplength);
            maplength--;
        }

        i++;
    } while (i < (maplength - 1));

    /* look for blocks of characters which use the same table */
    blocks = 1;
    blockmap[0][0] = typemap[0];
    blockmap[1][0] = 1;
    for (i = 1; i < maplength; i++) {
        if (typemap[i] == typemap[i - 1]) {
            blockmap[1][blocks - 1]++;
        } else {
            blocks++;
            blockmap[0][blocks - 1] = typemap[i];
            blockmap[1][blocks - 1] = 1;
        }
    }

    if (blockmap[0][0] & 1) {
        blockmap[0][0] = 1;
    }
    if (blockmap[0][0] & 2) {
        blockmap[0][0] = 2;
    }
    if (blockmap[0][0] & 4) {
        blockmap[0][0] = 4;
    }
    if (blockmap[0][0] & 8) {
        blockmap[0][0] = 8;
    }

    if (blocks > 1) {
        /* look for adjacent blocks which can use the same table (left to right search) */
        for (i = 1; i < blocks; i++) {
            if (blockmap[0][i] & blockmap[0][i - 1]) {
                blockmap[0][i] = (blockmap[0][i] & blockmap[0][i - 1]);
            }
        }

        if (blockmap[0][blocks - 1] & 1) {
            blockmap[0][blocks - 1] = 1;
        }
        if (blockmap[0][blocks - 1] & 2) {
            blockmap[0][blocks - 1] = 2;
        }
        if (blockmap[0][blocks - 1] & 4) {
            blockmap[0][blocks - 1] = 4;
        }
        if (blockmap[0][blocks - 1] & 8) {
            blockmap[0][blocks - 1] = 8;
        }

        /* look for adjacent blocks which can use the same table (right to left search) */
        for (i = blocks - 1 - 1; i >= 0; i--) {
            if (blockmap[0][i] & blockmap[0][i + 1]) {
                blockmap[0][i] = (blockmap[0][i] & blockmap[0][i + 1]);
            }
        }

        /* determine the encoding table for characters which do not fit with adjacent blocks */
        for (i = 1; i < blocks; i++) {
            if (blockmap[0][i] & 8) {
                blockmap[0][i] = 8;
            }
            if (blockmap[0][i] & 4) {
                blockmap[0][i] = 4;
            }
            if (blockmap[0][i] & 2) {
                blockmap[0][i] = 2;
            }
            if (blockmap[0][i] & 1) {
                blockmap[0][i] = 1;
            }
        }

        /* Combine blocks of the same type */
        i = 0;
        do {
            if (blockmap[0][i] == blockmap[0][i + 1]) {
                blockmap[1][i] += blockmap[1][i + 1];
                for (j = i + 1; j < blocks - 1; j++) {
                    blockmap[0][j] = blockmap[0][j + 1];
                    blockmap[1][j] = blockmap[1][j + 1];
                }
                blocks--;
            } else {
                i++;
            }
        } while (i < blocks - 1);
    }

    /* Put the adjusted block data back into typemap */
    j = 0;
    for (i = 0; i < blocks; i++) {
        if ((blockmap[1][i] < 3) && (blockmap[0][i] != 32)) {
            /* Shift character(s) needed */
            for (k = 0; k < blockmap[1][i]; k++) {
                typemap[j + k] = blockmap[0][i] + 64;
            }
        } else { /* Latch character (or byte mode) needed */
            for (k = 0; k < blockmap[1][i]; k++) {
                typemap[j + k] = blockmap[0][i];
            }
        }
        j += blockmap[1][i];
    }

    /* Don't shift an initial capital letter */
    if (typemap[0] == 65) {
        typemap[0] = 1;
    };

    /* Problem characters (those that appear in different tables with
     * different values) can now be resolved into their tables */
    for (i = 0; i < maplength; i++) {
        if ((charmap[i] >= 300) && (charmap[i] < 400)) {
            curtable = typemap[i];
            if (curtable > 64) {
                curtable -= 64;
            }
            switch (charmap[i]) {
                case 300: /* Carriage Return */
                    switch (curtable) {
                        case PUNC: charmap[i] = 1;
                            break;
                        case MIXED: charmap[i] = 14;
                            break;
                    }
                    break;
                case 301: /* Comma */
                    switch (curtable) {
                        case PUNC: charmap[i] = 17;
                            break;
                        case DIGIT: charmap[i] = 12;
                            break;
                    }
                    break;
                case 302: /* Full Stop */
                    switch (curtable) {
                        case PUNC: charmap[i] = 19;
                            break;
                        case DIGIT: charmap[i] = 13;
                            break;
                    }
                    break;
            }
        }
    }
    *binary_string = '\0';

    curtable = UPPER; /* start with UPPER table */
    lasttable = UPPER;
    for (i = 0; i < maplength; i++) {
        newtable = curtable;
        if ((typemap[i] != curtable) && (charmap[i] < 400)) {
            /* Change table */
            if (curtable == BINARY) {
                /* If ending binary mode the current table is the same as when entering binary mode */
                curtable = lasttable;
                newtable = lasttable;
            }
            if (typemap[i] > 64) {
                /* Shift character */
                switch (typemap[i]) {
                    case (64 + UPPER): /* To UPPER */
                        switch (curtable) {
                            case LOWER: /* US */
                                strcat(binary_string, pentbit[28]);
                                if (debug) printf("US ");
                                break;
                            case MIXED: /* UL */
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("UL ");
                                newtable = UPPER;
                                break;
                            case PUNC: /* UL */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("UL ");
                                newtable = UPPER;
                                break;
                            case DIGIT: /* US */
                                strcat(binary_string, quadbit[15]);
                                if (debug) printf("US ");
                                break;
                        }
                        break;
                    case (64 + LOWER): /* To LOWER */
                        switch (curtable) {
                            case UPPER: /* LL */
                                strcat(binary_string, pentbit[28]);
                                if (debug) printf("LL ");
                                newtable = LOWER;
                                break;
                            case MIXED: /* LL */
                                strcat(binary_string, pentbit[28]);
                                if (debug) printf("LL ");
                                newtable = LOWER;
                                break;
                            case PUNC: /* UL LL */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[28]);
                                if (debug) printf("LL ");
                                newtable = LOWER;
                                break;
                            case DIGIT: /* UL LL */
                                strcat(binary_string, quadbit[14]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[28]);
                                if (debug) printf("LL ");
                                newtable = LOWER;
                                break;
                        }
                        break;
                    case (64 + MIXED): /* To MIXED */
                        switch (curtable) {
                            case UPPER: /* ML */
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                newtable = MIXED;
                                break;
                            case LOWER: /* ML */
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                newtable = MIXED;
                                break;
                            case PUNC: /* UL ML */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                newtable = MIXED;
                                break;
                            case DIGIT: /* UL ML */
                                strcat(binary_string, quadbit[14]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                newtable = MIXED;
                                break;
                        }
                        break;
                    case (64 + PUNC): /* To PUNC */
                        switch (curtable) {
                            case UPPER: /* PS */
                                strcat(binary_string, pentbit[0]);
                                if (debug) printf("PS ");
                                break;
                            case LOWER: /* PS */
                                strcat(binary_string, pentbit[0]);
                                if (debug) printf("PS ");
                                break;
                            case MIXED: /* PS */
                                strcat(binary_string, pentbit[0]);
                                if (debug) printf("PS ");
                                break;
                            case DIGIT: /* PS */
                                strcat(binary_string, quadbit[0]);
                                if (debug) printf("PS ");
                                break;
                        }
                        break;
                    case (64 + DIGIT): /* To DIGIT */
                        switch (curtable) {
                            case UPPER: /* DL */
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("DL ");
                                newtable = DIGIT;
                                break;
                            case LOWER: /* DL */
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("DL ");
                                newtable = DIGIT;
                                break;
                            case MIXED: /* UL DL */
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("DL ");
                                newtable = DIGIT;
                                break;
                            case PUNC: /* UL DL */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("DL ");
                                newtable = DIGIT;
                                break;
                        }
                        break;
                }
            } else {
                /* Latch character */
                switch (typemap[i]) {
                    case UPPER: /* To UPPER */
                        switch (curtable) {
                            case LOWER: /* ML UL */
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("UL ");
                                newtable = UPPER;
                                break;
                            case MIXED: /* UL */
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("UL ");
                                newtable = UPPER;
                                break;
                            case PUNC: /* UL */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("UL ");
                                newtable = UPPER;
                                break;
                            case DIGIT: /* UL */
                                strcat(binary_string, quadbit[14]);
                                if (debug) printf("UL ");
                                newtable = UPPER;
                                break;
                        }
                        break;
                    case LOWER: /* To LOWER */
                        switch (curtable) {
                            case UPPER: /* LL */
                                strcat(binary_string, pentbit[28]);
                                if (debug) printf("LL ");
                                newtable = LOWER;
                                break;
                            case MIXED: /* LL */
                                strcat(binary_string, pentbit[28]);
                                if (debug) printf("LL ");
                                newtable = LOWER;
                                break;
                            case PUNC: /* UL LL */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[28]);
                                if (debug) printf("LL ");
                                newtable = LOWER;
                                break;
                            case DIGIT: /* UL LL */
                                strcat(binary_string, quadbit[14]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[28]);
                                if (debug) printf("LL ");
                                newtable = LOWER;
                                break;
                        }
                        break;
                    case MIXED: /* To MIXED */
                        switch (curtable) {
                            case UPPER: /* ML */
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                newtable = MIXED;
                                break;
                            case LOWER: /* ML */
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                newtable = MIXED;
                                break;
                            case PUNC: /* UL ML */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                newtable = MIXED;
                                break;
                            case DIGIT: /* UL ML */
                                strcat(binary_string, quadbit[14]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                newtable = MIXED;
                                break;
                        }
                        break;
                    case PUNC: /* To PUNC */
                        switch (curtable) {
                            case UPPER: /* ML PL */
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("PL ");
                                newtable = PUNC;
                                break;
                            case LOWER: /* ML PL */
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("PL ");
                                newtable = PUNC;
                                break;
                            case MIXED: /* PL */
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("PL ");
                                newtable = PUNC;
                                break;
                            case DIGIT: /* UL ML PL */
                                strcat(binary_string, quadbit[14]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("ML ");
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("PL ");
                                newtable = PUNC;
                                break;
                        }
                        break;
                    case DIGIT: /* To DIGIT */
                        switch (curtable) {
                            case UPPER: /* DL */
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("DL ");
                                newtable = DIGIT;
                                break;
                            case LOWER: /* DL */
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("DL ");
                                newtable = DIGIT;
                                break;
                            case MIXED: /* UL DL */
                                strcat(binary_string, pentbit[29]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("DL ");
                                newtable = DIGIT;
                                break;
                            case PUNC: /* UL DL */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[30]);
                                if (debug) printf("DL ");
                                newtable = DIGIT;
                                break;
                        }
                        break;
                    case BINARY: /* To BINARY */
                        lasttable = curtable;
                        switch (curtable) {
                            case UPPER: /* BS */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("BS ");
                                newtable = BINARY;
                                break;
                            case LOWER: /* BS */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("BS ");
                                newtable = BINARY;
                                break;
                            case MIXED: /* BS */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("BS ");
                                newtable = BINARY;
                                break;
                            case PUNC: /* UL BS */
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("BS ");
                                lasttable = UPPER;
                                newtable = BINARY;
                                break;
                            case DIGIT: /* UL BS */
                                strcat(binary_string, quadbit[14]);
                                if (debug) printf("UL ");
                                strcat(binary_string, pentbit[31]);
                                if (debug) printf("BS ");
                                lasttable = UPPER;
                                newtable = BINARY;
                                break;
                        }

                        bytes = 0;
                        do {
                            bytes++;
                        } while (typemap[i + (bytes - 1)] == BINARY);
                        bytes--;

                        if (bytes > 2079) {
                            return ZINT_ERROR_TOO_LONG;
                        }

                        if (bytes > 31) {
                            /* Put 00000 followed by 11-bit number of bytes less 31 */
                            strcat(binary_string, "00000");

                            for (p = 0; p < 11; p++) {
                                if ((bytes - 31) & (0x400 >> p)) {
                                    strcat(binary_string, "1");
                                } else {
                                    strcat(binary_string, "0");
                                }
                            }
                        } else {
                            /* Put 5-bit number of bytes */
                            for (p = 0; p < 5; p++) {
                                if (bytes & (0x10 >> p)) {
                                    strcat(binary_string, "1");
                                } else {
                                    strcat(binary_string, "0");
                                }
                            }
                        }
                        if (debug) printf("(%d bytes) ", bytes);

                        break;
                }
            }
        }
        /* Add data to the binary string */
        curtable = newtable;
        chartype = typemap[i];
        if (chartype > 64) {
            chartype -= 64;
        }
        switch (chartype) {
            case UPPER:
            case LOWER:
            case MIXED:
            case PUNC:
                if ((charmap[i] >= 400) && (charmap[i] < 500)) {
                    strcat(binary_string, tribit[charmap[i] - 400]);
                    if (debug) printf("FLG(%d) ", charmap[i] - 400);
                } else if (charmap[i] >= 500) {
                    strcat(binary_string, quadbit[charmap[i] - 500]);
                    if (debug) printf("[%d] ", charmap[i] - 500);
                } else {
                    strcat(binary_string, pentbit[charmap[i]]);
                    if (!((chartype == PUNC) && (charmap[i] == 0)))
                        if (debug) printf("%d ", charmap[i]);
                }
                break;
            case DIGIT:
                strcat(binary_string, quadbit[charmap[i]]);
                if (debug) printf("%d ", charmap[i]);
                break;
            case BINARY:
                for (p = 0; p < 8; p++) {
                    if (charmap[i] & (0x80 >> p)) {
                        strcat(binary_string, "1");
                    } else {
                        strcat(binary_string, "0");
                    }
                }
                if (debug) printf("%d ", charmap[i]);
                break;
        }

    }

    if (debug) printf("\n");

    if (strlen(binary_string) > 14970) {
        return ZINT_ERROR_TOO_LONG;
    }

    return 0;
}

/* Prevent data from obscuring reference grid */
static int avoidReferenceGrid(int output) {

    if (output > 10) {
        output++;
    }
    if (output > 26) {
        output++;
    }
    if (output > 42) {
        output++;
    }
    if (output > 58) {
        output++;
    }
    if (output > 74) {
        output++;
    }
    if (output > 90) {
        output++;
    }
    if (output > 106) {
        output++;
    }
    if (output > 122) {
        output++;
    }
    if (output > 138) {
        output++;
    }

    return output;
}

/* Calculate the position of the bits in the grid */
static void populate_map() {
    int layer, start, length, n, i;
    int x, y;

    for (x = 0; x < 151; x++) {
        for (y = 0; y < 151; y++) {
            AztecMap[(x * 151) + y] = 0;
        }
    }

    for (layer = 1; layer < 33; layer++) {
        start = (112 * (layer - 1)) + (16 * (layer - 1) * (layer - 1)) + 2;
        length = 28 + ((layer - 1) * 4) + (layer * 4);
        /* Top */
        i = 0;
        x = 64 - ((layer - 1) * 2);
        y = 63 - ((layer - 1) * 2);
        for (n = start; n < (start + length); n += 2) {
            AztecMap[(avoidReferenceGrid(x + i) * 151) + avoidReferenceGrid(y)] = n;
            AztecMap[(avoidReferenceGrid(x + i) * 151) + avoidReferenceGrid(y - 1)] = n + 1;
            i++;
        }
        /* Right */
        i = 0;
        x = 78 + ((layer - 1) * 2);
        y = 64 - ((layer - 1) * 2);
        for (n = start + length; n < (start + (length * 2)); n += 2) {
            AztecMap[(avoidReferenceGrid(x) * 151) + avoidReferenceGrid(y + i)] = n;
            AztecMap[(avoidReferenceGrid(x + 1) * 151) + avoidReferenceGrid(y + i)] = n + 1;
            i++;
        }
        /* Bottom */
        i = 0;
        x = 77 + ((layer - 1) * 2);
        y = 78 + ((layer - 1) * 2);
        for (n = start + (length * 2); n < (start + (length * 3)); n += 2) {
            AztecMap[(avoidReferenceGrid(x - i) * 151) + avoidReferenceGrid(y)] = n;
            AztecMap[(avoidReferenceGrid(x - i) * 151) + avoidReferenceGrid(y + 1)] = n + 1;
            i++;
        }
        /* Left */
        i = 0;
        x = 63 - ((layer - 1) * 2);
        y = 77 + ((layer - 1) * 2);
        for (n = start + (length * 3); n < (start + (length * 4)); n += 2) {
            AztecMap[(avoidReferenceGrid(x) * 151) + avoidReferenceGrid(y - i)] = n;
            AztecMap[(avoidReferenceGrid(x - 1) * 151) + avoidReferenceGrid(y - i)] = n + 1;
            i++;
        }
    }

    /* Central finder pattern */
    for (y = 69; y <= 81; y++) {
        for (x = 69; x <= 81; x++) {
            AztecMap[(x * 151) + y] = 1;
        }
    }
    for (y = 70; y <= 80; y++) {
        for (x = 70; x <= 80; x++) {
            AztecMap[(x * 151) + y] = 0;
        }
    }
    for (y = 71; y <= 79; y++) {
        for (x = 71; x <= 79; x++) {
            AztecMap[(x * 151) + y] = 1;
        }
    }
    for (y = 72; y <= 78; y++) {
        for (x = 72; x <= 78; x++) {
            AztecMap[(x * 151) + y] = 0;
        }
    }
    for (y = 73; y <= 77; y++) {
        for (x = 73; x <= 77; x++) {
            AztecMap[(x * 151) + y] = 1;
        }
    }
    for (y = 74; y <= 76; y++) {
        for (x = 74; x <= 76; x++) {
            AztecMap[(x * 151) + y] = 0;
        }
    }

    /* Guide bars */
    for (y = 11; y < 151; y += 16) {
        for (x = 1; x < 151; x += 2) {
            AztecMap[(x * 151) + y] = 1;
            AztecMap[(y * 151) + x] = 1;
        }
    }

    /* Descriptor */
    for (i = 0; i < 10; i++) {
        /* Top */
        AztecMap[(avoidReferenceGrid(66 + i) * 151) + avoidReferenceGrid(64)] = 20000 + i;
    }
    for (i = 0; i < 10; i++) {
        /* Right */
        AztecMap[(avoidReferenceGrid(77) * 151) + avoidReferenceGrid(66 + i)] = 20010 + i;
    }
    for (i = 0; i < 10; i++) {
        /* Bottom */
        AztecMap[(avoidReferenceGrid(75 - i) * 151) + avoidReferenceGrid(77)] = 20020 + i;
    }
    for (i = 0; i < 10; i++) {
        /* Left */
        AztecMap[(avoidReferenceGrid(64) * 151) + avoidReferenceGrid(75 - i)] = 20030 + i;
    }

    /* Orientation */
    AztecMap[(avoidReferenceGrid(64) * 151) + avoidReferenceGrid(64)] = 1;
    AztecMap[(avoidReferenceGrid(65) * 151) + avoidReferenceGrid(64)] = 1;
    AztecMap[(avoidReferenceGrid(64) * 151) + avoidReferenceGrid(65)] = 1;
    AztecMap[(avoidReferenceGrid(77) * 151) + avoidReferenceGrid(64)] = 1;
    AztecMap[(avoidReferenceGrid(77) * 151) + avoidReferenceGrid(65)] = 1;
    AztecMap[(avoidReferenceGrid(77) * 151) + avoidReferenceGrid(76)] = 1;
}

int aztec(struct zint_symbol *symbol, unsigned char source[], int length) {
    int x, y, i, j, p, data_blocks, ecc_blocks, layers, total_bits;
    char binary_string[20000], bit_pattern[20045], descriptor[42];
    char adjusted_string[20000];
    unsigned char desc_data[4], desc_ecc[6];
    int err_code, ecc_level, compact, data_length, data_maxsize, codeword_size, adjusted_length;
    int remainder, padbits, count, gs1, adjustment_size;
    int debug = symbol->debug, reader = 0;
    int comp_loop = 4;

#ifdef _MSC_VER
    unsigned int* data_part;
    unsigned int* ecc_part;
#endif

    memset(binary_string, 0, 20000);
    memset(adjusted_string, 0, 20000);

    if (symbol->input_mode == GS1_MODE) {
        gs1 = 1;
    } else {
        gs1 = 0;
    }
    if (symbol->output_options & READER_INIT) {
        reader = 1;
        comp_loop = 1;
    }
    if (gs1 && reader) {
        strcpy(symbol->errtxt, "Cannot encode in GS1 and Reader Initialisation mode at the same time (E01)");
        return ZINT_ERROR_INVALID_OPTION;
    }

    populate_map();

    err_code = aztec_text_process(source, length, binary_string, gs1, symbol->eci, symbol->debug);

    if (err_code != 0) {
        strcpy(symbol->errtxt, "Input too long or too many extended ASCII characters (E02)");
        return err_code;
    }

    if (!((symbol->option_1 >= -1) && (symbol->option_1 <= 4))) {
        strcpy(symbol->errtxt, "Invalid error correction level - using default instead (E03)");
        err_code = ZINT_WARN_INVALID_OPTION;
        symbol->option_1 = -1;
    }

    ecc_level = symbol->option_1;

    if ((ecc_level == -1) || (ecc_level == 0)) {
        ecc_level = 2;
    }

    data_length = (int) strlen(binary_string);

    layers = 0; /* Keep compiler happy! */
    data_maxsize = 0; /* Keep compiler happy! */
    adjustment_size = 0;
    if (symbol->option_2 == 0) { /* The size of the symbol can be determined by Zint */
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
                strcpy(symbol->errtxt, "Input too long (too many bits for selected ECC) (E04)");
                return ZINT_ERROR_TOO_LONG;
            }

            /* Determine codeword bitlength - Table 3 */
            codeword_size = 6; /* if (layers <= 2) */
            if ((layers >= 3) && (layers <= 8)) {
                codeword_size = 8;
            }
            if ((layers >= 9) && (layers <= 22)) {
                codeword_size = 10;
            }
            if (layers >= 23) {
                codeword_size = 12;
            }

            j = 0;
            i = 0;
            do {
                if ((j + 1) % codeword_size == 0) {
                    /* Last bit of codeword */
                    int t, done = 0;
                    count = 0;

                    /* Discover how many '1's in current codeword */
                    for (t = 0; t < (codeword_size - 1); t++) {
                        if (binary_string[(i - (codeword_size - 1)) + t] == '1') count++;
                    }

                    if (count == (codeword_size - 1)) {
                        adjusted_string[j] = '0';
                        j++;
                        done = 1;
                    }

                    if (count == 0) {
                        adjusted_string[j] = '1';
                        j++;
                        done = 1;
                    }

                    if (done == 0) {
                        adjusted_string[j] = binary_string[i];
                        j++;
                        i++;
                    }
                }
                adjusted_string[j] = binary_string[i];
                j++;
                i++;
            } while (i <= (data_length + 1));
            adjusted_string[j] = '\0';
            adjusted_length = (int) strlen(adjusted_string);
            adjustment_size = adjusted_length - data_length;

            /* Add padding */
            remainder = adjusted_length % codeword_size;

            padbits = codeword_size - remainder;
            if (padbits == codeword_size) {
                padbits = 0;
            }

            for (i = 0; i < padbits; i++) {
                strcat(adjusted_string, "1");
            }
            adjusted_length = (int) strlen(adjusted_string);

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
                    printf("\n");
                }
            }

        } while (adjusted_length > data_maxsize);
        /* This loop will only repeat on the rare occasions when the rule about not having all 1s or all 0s
        means that the binary string has had to be lengthened beyond the maximum number of bits that can
        be encoded in a symbol of the selected size */

    } else { /* The size of the symbol has been specified by the user */
        if ((reader == 1) && ((symbol->option_2 >= 2) && (symbol->option_2 <= 4))) {
            symbol->option_2 = 5;
        }
        if ((symbol->option_2 >= 1) && (symbol->option_2 <= 4)) {
            compact = 1;
            layers = symbol->option_2;
        }
        if ((symbol->option_2 >= 5) && (symbol->option_2 <= 36)) {
            compact = 0;
            layers = symbol->option_2 - 4;
        }
        if ((symbol->option_2 < 0) || (symbol->option_2 > 36)) {
            strcpy(symbol->errtxt, "Invalid Aztec Code size");
            return ZINT_ERROR_INVALID_OPTION;
        }

        /* Determine codeword bitlength - Table 3 */
        if ((layers >= 0) && (layers <= 2)) {
            codeword_size = 6;
        }
        if ((layers >= 3) && (layers <= 8)) {
            codeword_size = 8;
        }
        if ((layers >= 9) && (layers <= 22)) {
            codeword_size = 10;
        }
        if (layers >= 23) {
            codeword_size = 12;
        }

        j = 0;
        i = 0;
        do {
            if ((j + 1) % codeword_size == 0) {
                /* Last bit of codeword */
                int t, done = 0;
                count = 0;

                /* Discover how many '1's in current codeword */
                for (t = 0; t < (codeword_size - 1); t++) {
                    if (binary_string[(i - (codeword_size - 1)) + t] == '1') count++;
                }

                if (count == (codeword_size - 1)) {
                    adjusted_string[j] = '0';
                    j++;
                    done = 1;
                }

                if (count == 0) {
                    adjusted_string[j] = '1';
                    j++;
                    done = 1;
                }

                if (done == 0) {
                    adjusted_string[j] = binary_string[i];
                    j++;
                    i++;
                }
            }
            adjusted_string[j] = binary_string[i];
            j++;
            i++;
        } while (i <= (data_length + 1));
        adjusted_string[j] = '\0';
        adjusted_length = (int) strlen(adjusted_string);

        remainder = adjusted_length % codeword_size;

        padbits = codeword_size - remainder;
        if (padbits == codeword_size) {
            padbits = 0;
        }

        for (i = 0; i < padbits; i++) {
            strcat(adjusted_string, "1");
        }
        adjusted_length = (int) strlen(adjusted_string);

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
            strcpy(symbol->errtxt, "Data too long for specified Aztec Code symbol size (E05)");
            return ZINT_ERROR_TOO_LONG;
        }

        if (debug) {
            printf("Codewords:\n");
            for (i = 0; i < (adjusted_length / codeword_size); i++) {
                for (j = 0; j < codeword_size; j++) {
                    printf("%c", adjusted_string[(i * codeword_size) + j]);
                }
                printf("\n");
            }
        }

    }

    if (reader && (layers > 22)) {
        strcpy(symbol->errtxt, "Data too long for reader initialisation symbol (E06)");
        return ZINT_ERROR_TOO_LONG;
    }

    data_blocks = adjusted_length / codeword_size;

    if (compact) {
        ecc_blocks = AztecCompactSizes[layers - 1] - data_blocks;
    } else {
        ecc_blocks = AztecSizes[layers - 1] - data_blocks;
    }

    if (debug) {
        printf("Generating a ");
        if (compact) {
            printf("compact");
        } else {
            printf("full-size");
        }
        printf(" symbol with %d layers\n", layers);
        printf("Requires ");
        if (compact) {
            printf("%d", AztecCompactSizes[layers - 1]);
        } else {
            printf("%d", AztecSizes[layers - 1]);
        }
        printf(" codewords of %d-bits\n", codeword_size);
        printf("    (%d data words, %d ecc words)\n", data_blocks, ecc_blocks);
    }

#ifndef _MSC_VER
    unsigned int data_part[data_blocks + 3], ecc_part[ecc_blocks + 3];
#else
    data_part = (unsigned int*) _alloca((data_blocks + 3) * sizeof (unsigned int));
    ecc_part = (unsigned int*) _alloca((ecc_blocks + 3) * sizeof (unsigned int));
#endif
    /* Copy across data into separate integers */
    memset(data_part, 0, (data_blocks + 2) * sizeof (int));
    memset(ecc_part, 0, (ecc_blocks + 2) * sizeof (int));

    /* Split into codewords and calculate reed-colomon error correction codes */
    switch (codeword_size) {
        case 6:
            for (i = 0; i < data_blocks; i++) {
                for (p = 0; p < 6; p++) {
                    if (adjusted_string[i * codeword_size + p] == '1') {
                        data_part[i] += (0x20 >> p);
                    }
                }
            }
            rs_init_gf(0x43);
            rs_init_code(ecc_blocks, 1);
            rs_encode_long(data_blocks, data_part, ecc_part);
            for (i = (ecc_blocks - 1); i >= 0; i--) {
                for (p = 0; p < 6; p++) {
                    if (ecc_part[i] & (0x20 >> p)) {
                        strcat(adjusted_string, "1");
                    } else {
                        strcat(adjusted_string, "0");
                    }
                }
            }
            rs_free();
            break;
        case 8:
            for (i = 0; i < data_blocks; i++) {
                for (p = 0; p < 8; p++) {
                    if (adjusted_string[i * codeword_size + p] == '1') {
                        data_part[i] += (0x80 >> p);
                    }
                }
            }
            rs_init_gf(0x12d);
            rs_init_code(ecc_blocks, 1);
            rs_encode_long(data_blocks, data_part, ecc_part);
            for (i = (ecc_blocks - 1); i >= 0; i--) {
                for (p = 0; p < 8; p++) {
                    if (ecc_part[i] & (0x80 >> p)) {
                        strcat(adjusted_string, "1");
                    } else {
                        strcat(adjusted_string, "0");
                    }
                }
            }
            rs_free();
            break;
        case 10:
            for (i = 0; i < data_blocks; i++) {
                for (p = 0; p < 10; p++) {
                    if (adjusted_string[i * codeword_size + p] == '1') {
                        data_part[i] += (0x200 >> p);
                    }
                }
            }
            rs_init_gf(0x409);
            rs_init_code(ecc_blocks, 1);
            rs_encode_long(data_blocks, data_part, ecc_part);
            for (i = (ecc_blocks - 1); i >= 0; i--) {
                for (p = 0; p < 10; p++) {
                    if (ecc_part[i] & (0x200 >> p)) {
                        strcat(adjusted_string, "1");
                    } else {
                        strcat(adjusted_string, "0");
                    }
                }
            }
            rs_free();
            break;
        case 12:
            for (i = 0; i < data_blocks; i++) {
                for (p = 0; p < 12; p++) {
                    if (adjusted_string[i * codeword_size + p] == '1') {
                        data_part[i] += (0x800 >> p);
                    }
                }
            }
            rs_init_gf(0x1069);
            rs_init_code(ecc_blocks, 1);
            rs_encode_long(data_blocks, data_part, ecc_part);
            for (i = (ecc_blocks - 1); i >= 0; i--) {
                for (p = 0; p < 12; p++) {
                    if (ecc_part[i] & (0x800 >> p)) {
                        strcat(adjusted_string, "1");
                    } else {
                        strcat(adjusted_string, "0");
                    }
                }
            }
            rs_free();
            break;
    }

    /* Invert the data so that actual data is on the outside and reed-solomon on the inside */
    memset(bit_pattern, '0', 20045);

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

        descriptor[8] = '\0';
        if (debug) printf("Mode Message = %s\n", descriptor);
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
        descriptor[16] = '\0';
        if (debug) printf("Mode Message = %s\n", descriptor);
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

    rs_init_gf(0x13);
    if (compact) {
        rs_init_code(5, 1);
        rs_encode(2, desc_data, desc_ecc);
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
        rs_init_code(6, 1);
        rs_encode(4, desc_data, desc_ecc);
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
    rs_free();

    /* Merge descriptor with the rest of the symbol */
    for (i = 0; i < 40; i++) {
        if (compact) {
            bit_pattern[2000 + i - 2] = descriptor[i];
        } else {
            bit_pattern[20000 + i - 2] = descriptor[i];
        }
    }

    /* Plot all of the data into the symbol in pre-defined spiral pattern */
    if (compact) {

        for (y = AztecCompactOffset[layers - 1]; y < (27 - AztecCompactOffset[layers - 1]); y++) {
            for (x = AztecCompactOffset[layers - 1]; x < (27 - AztecCompactOffset[layers - 1]); x++) {
                if (CompactAztecMap[(y * 27) + x] == 1) {
                    set_module(symbol, y - AztecCompactOffset[layers - 1], x - AztecCompactOffset[layers - 1]);
                }
                if (CompactAztecMap[(y * 27) + x] >= 2) {
                    if (bit_pattern[CompactAztecMap[(y * 27) + x] - 2] == '1') {
                        set_module(symbol, y - AztecCompactOffset[layers - 1], x - AztecCompactOffset[layers - 1]);
                    }
                }
            }
            symbol->row_height[y - AztecCompactOffset[layers - 1]] = 1;
        }
        symbol->rows = 27 - (2 * AztecCompactOffset[layers - 1]);
        symbol->width = 27 - (2 * AztecCompactOffset[layers - 1]);
    } else {

        for (y = AztecOffset[layers - 1]; y < (151 - AztecOffset[layers - 1]); y++) {
            for (x = AztecOffset[layers - 1]; x < (151 - AztecOffset[layers - 1]); x++) {
                if (AztecMap[(y * 151) + x] == 1) {
                    set_module(symbol, y - AztecOffset[layers - 1], x - AztecOffset[layers - 1]);
                }
                if (AztecMap[(y * 151) + x] >= 2) {
                    if (bit_pattern[AztecMap[(y * 151) + x] - 2] == '1') {
                        set_module(symbol, y - AztecOffset[layers - 1], x - AztecOffset[layers - 1]);
                    }
                }
            }
            symbol->row_height[y - AztecOffset[layers - 1]] = 1;
        }
        symbol->rows = 151 - (2 * AztecOffset[layers - 1]);
        symbol->width = 151 - (2 * AztecOffset[layers - 1]);
    }

    return err_code;
}

/* Encodes Aztec runes as specified in ISO/IEC 24778:2008 Annex A */
int aztec_runes(struct zint_symbol *symbol, unsigned char source[], int length) {
    int input_value, error_number, i, p, y, x;
    char binary_string[28];
    unsigned char data_codewords[3], ecc_codewords[6];

    error_number = 0;
    input_value = 0;
    if (length > 3) {
        strcpy(symbol->errtxt, "Input too large (E07)");
        return ZINT_ERROR_INVALID_DATA;
    }
    error_number = is_sane(NEON, source, length);
    if (error_number != 0) {
        strcpy(symbol->errtxt, "Invalid characters in input (E08)");
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
        strcpy(symbol->errtxt, "Input too large (E09)");
        return ZINT_ERROR_INVALID_DATA;
    }

    strcpy(binary_string, "");
    for (p = 0; p < 8; p++) {
        if (input_value & (0x80 >> p)) {
            strcat(binary_string, "1");
        } else {
            strcat(binary_string, "0");
        }
    }

    data_codewords[0] = 0;
    data_codewords[1] = 0;

    for (i = 0; i < 2; i++) {
        if (binary_string[i * 4] == '1') {
            data_codewords[i] += 8;
        }
        if (binary_string[(i * 4) + 1] == '1') {
            data_codewords[i] += 4;
        }
        if (binary_string[(i * 4) + 2] == '1') {
            data_codewords[i] += 2;
        }
        if (binary_string[(i * 4) + 3] == '1') {
            data_codewords[i] += 1;
        }
    }

    rs_init_gf(0x13);
    rs_init_code(5, 1);
    rs_encode(2, data_codewords, ecc_codewords);
    rs_free();

    strcpy(binary_string, "");

    for (i = 0; i < 5; i++) {
        if (ecc_codewords[4 - i] & 0x08) {
            binary_string[(i * 4) + 8] = '1';
        } else {
            binary_string[(i * 4) + 8] = '0';
        }
        if (ecc_codewords[4 - i] & 0x04) {
            binary_string[(i * 4) + 9] = '1';
        } else {
            binary_string[(i * 4) + 9] = '0';
        }
        if (ecc_codewords[4 - i] & 0x02) {
            binary_string[(i * 4) + 10] = '1';
        } else {
            binary_string[(i * 4) + 10] = '0';
        }
        if (ecc_codewords[4 - i] & 0x01) {
            binary_string[(i * 4) + 11] = '1';
        } else {
            binary_string[(i * 4) + 11] = '0';
        }
    }

    for (i = 0; i < 28; i += 2) {
        if (binary_string[i] == '1') {
            binary_string[i] = '0';
        } else {
            binary_string[i] = '1';
        }
    }

    for (y = 8; y < 19; y++) {
        for (x = 8; x < 19; x++) {
            if (CompactAztecMap[(y * 27) + x] == 1) {
                set_module(symbol, y - 8, x - 8);
            }
            if (CompactAztecMap[(y * 27) + x] >= 2) {
                if (binary_string[CompactAztecMap[(y * 27) + x] - 2000] == '1') {
                    set_module(symbol, y - 8, x - 8);
                }
            }
        }
        symbol->row_height[y - 8] = 1;
    }
    symbol->rows = 11;
    symbol->width = 11;

    return 0;
}
