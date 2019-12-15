/*  ultra.c - Ultracode

    libzint - the open source barcode library
    Copyright (C) 2019 Robin Stuart <rstuart114@gmail.com>

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

 /* This version was developed using AIMD/TSC15032-43 v0.99c Edit 60, dated 4th Nov 2015 */

#ifdef _MSC_VER
#include <malloc.h>
#endif
#include <stdio.h>
#include <string.h>
#include "common.h"

#define EIGHTBIT_MODE       10
#define ASCII_MODE          20
#define C43_MODE            30

#define PREDICT_WINDOW      12

#define GFMUL(i, j) ((((i) == 0)||((j) == 0)) ? 0 : gfPwr[(gfLog[i] + gfLog[j])])

static const char fragment[27][14] = {"http://", "https://", "http://www.", "https://www.",
        "ftp://", "www.", ".com", ".edu", ".gov", ".int", ".mil", ".net", ".org",
        ".mobi", ".coop", ".biz", ".info", "mailto:", "tel:", ".cgi", ".asp",
        ".aspx", ".php", ".htm", ".html", ".shtml", "file:"};

static const char ultra_c43_set1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .,%";
static const char ultra_c43_set2[] = "abcdefghijklmnopqrstuvwxyz:/?#[]@=_~!.,-";
static const char ultra_c43_set3[] = "{}`()\"+'<>|$;&\\^*";
static const char ultra_digit[] = "0123456789,/";

//static const int ultra_maxsize[] = {34, 78, 158, 282}; // According to Table 1
static const int ultra_maxsize[] = {34, 82, 158, 282}; // Adjusted to allow 79-82 codeword range in 3-row symbols

/* The following adapted from ECC283.C "RSEC codeword generator"
 * from Annex B of Ultracode draft
 * originally written by Ted Williams of Symbol Vision Corp.
 * Dated 2001-03-09
 * Corrected thanks to input from Terry Burton */

/* Generate divisor polynomial gQ(x) for GF283() given the required ECC size, 3 to 101 */
void ultra_genPoly(short EccSize, unsigned short gPoly[], unsigned short gfPwr[], unsigned short gfLog[]) {
    int i, j;

    gPoly[0] = 1;
    for (i = 1; i < (EccSize + 1); i++) gPoly[i] = 0;

    for (i = 0; i < EccSize; i++) {
        for (j = i; j >= 0; j--)
            gPoly[j + 1] = (gPoly[j] + GFMUL(gPoly[j + 1], gfPwr[i + 1])) % 283;
        gPoly[0] = GFMUL(gPoly[0], gfPwr[i + 1]);
    }
    for (i = EccSize - 1; i >= 0; i -= 2) gPoly[i] = 283 - gPoly[i];

    /* gPoly[i] is > 0 so modulo operation not needed */
}

/* Generate the log and antilog tables for GF283() multiplication & division */
void ultra_initLogTables(unsigned short gfPwr[], unsigned short gfLog[]) {
    int i, j;

    for (j = 0; j < 283; j++) gfLog[j] = 0;
    i = 1;
    for (j = 0; j < 282; j++) {
        /* j + 282 indicies save doing the modulo operation in GFMUL */
        gfPwr[j + 282] = gfPwr[j] = (short) i;
        gfLog[i] = (short) j;
        i = (i * 3) % 283;
    }
}

void ultra_gf283(short DataSize, short EccSize, int Message[]) {
    /* Input is complete message codewords in array Message[282]
     * DataSize is number of message codewords
     * EccSize is number of Reed-Solomon GF(283) check codewords to generate
     *
     * Upon exit, Message[282] contains complete 282 codeword Symbol Message
     * including leading zeroes corresponding to each truncated codeword */

    unsigned short gPoly[283], gfPwr[(282 * 2)], gfLog[283];
    int i, j, n;
    unsigned short t;

    /* first build the log & antilog tables used in multiplication & division */
    ultra_initLogTables(gfPwr, gfLog);

    /* then generate the division polynomial of length EccSize */
    ultra_genPoly(EccSize, gPoly, gfPwr, gfLog);

    /* zero all EccSize codeword values */
    for (j = 281; (j > (281 - EccSize)); j--) Message[j] = 0;

    /* shift message codewords to the right, leave space for ECC checkwords */
    for (i = DataSize - 1; (i >= 0); j--, i--) Message[j] = Message[i];

    /* add zeroes to pad left end Message[] for truncated codewords */
    j++;
    for (i = 0; i < j; i++) Message[i] = 0;

    /* generate (EccSize) Reed-Solomon checkwords */
    for (n = j; n < (j + DataSize); n++) {
        t = (Message[j + DataSize] + Message[n]) % 283;
        for (i = 0; i < (EccSize - 1); i++) {
            Message[j + DataSize + i] = (Message[j + DataSize + i + 1] + 283
            - GFMUL(t, gPoly[EccSize - 1 - i])) % 283;
        }
        Message[j + DataSize + EccSize - 1] = (283 - GFMUL(t, gPoly[0])) % 283;
    }
    for (i = j + DataSize; i < (j + DataSize + EccSize); i++)
        Message[i] = (283 - Message[i]) % 283;
}

/* End of Ted Williams code */

int ultra_find_fragment(unsigned char source[], int source_length, int position) {
    int retval = -1;
    int j, k, latch;

    for (j = 0; j < 27; j++) {
        latch = 0;
        if ((position + strlen(fragment[j])) <= source_length) {
            latch = 1;
            for (k = 0; k < strlen(fragment[j]); k++) {
                if (source[position + k] != fragment[j][k]) {
                    latch = 0;
                }
            }
        }

        if (latch) {
            retval = j;
        }
    }

    return retval;
}

/* Encode characters in 8-bit mode */
float look_ahead_eightbit(unsigned char source[], int in_length, int in_posn, char current_mode, int end_char, int cw[], int* cw_len, int gs1)
{
    int codeword_count = 0;
    int i;
    int letters_encoded = 0;

    if (current_mode != EIGHTBIT_MODE) {
        cw[codeword_count] = 282; // Unlatch
        codeword_count += 1;
    }

    i = in_posn;
    do {
        if ((source[i] == '[') && gs1) {
            cw[codeword_count] = 268; // FNC1
        } else {
            cw[codeword_count] = source[i];
        }
        i++;
        codeword_count++;
    } while ((i < in_length) && (i < end_char));

    letters_encoded = i - in_posn;

    //printf("8BIT FRAG: ");
    //for (i = 0; i < codeword_count; i++) {
    //    printf("%d ", cw[i]);
    //}
    //printf("\n");

    *cw_len = codeword_count;

    //printf("%d letters in %d codewords\n", letters_encoded, codeword_count);
    if (codeword_count == 0) {
        return 0.0;
    } else {
        return (float)letters_encoded / (float)codeword_count;
    }
}

/* Encode character in the ASCII mode/submode (including numeric compression) */
float look_ahead_ascii(unsigned char source[], int in_length, int in_posn, char current_mode, int symbol_mode, int end_char, int cw[], int* cw_len, int gs1){
    int codeword_count = 0;
    int i;
    int first_digit, second_digit;
    int letters_encoded = 0;

    if (current_mode == EIGHTBIT_MODE) {
        cw[codeword_count] = 267; // Latch ASCII Submode
        codeword_count++;
    }

    if (current_mode == C43_MODE) {
        cw[codeword_count] = 282; // Unlatch
        codeword_count++;
        if (symbol_mode == EIGHTBIT_MODE) {
            cw[codeword_count] = 267; // Latch ASCII Submode
            codeword_count++;
        }
    }

    i = in_posn;
    do {
        /* Check for double digits */
        if (in_posn != (in_length - 1)) {
            first_digit = posn(ultra_digit, source[i]);
            second_digit = posn(ultra_digit, source[i + 1]);
            if ((first_digit != -1) && (second_digit != -1)) {
                /* Double digit can be encoded */
                if ((first_digit >= 0) && (first_digit <= 9) && (second_digit >= 0) && (second_digit <= 9)) {
                    /* Double digit numerics */
                    cw[codeword_count] = (10 * first_digit) + second_digit + 128;
                    codeword_count++;
                    i += 2;
                } else if ((first_digit >= 0) && (first_digit <= 9) && (second_digit == 10)) {
                    /* Single digit followed by selected decimal point character */
                    cw[codeword_count] = first_digit + 228;
                    codeword_count++;
                    i += 2;
                } else if ((first_digit == 10) && (second_digit >= 0) && (second_digit <= 9)) {
                    /* Selected decimal point character followed by single digit */
                    cw[codeword_count] = second_digit + 238;
                    codeword_count++;
                    i += 2;
                } else if ((first_digit >= 0) && (first_digit <= 10) && (second_digit == 11)) {
                    /* Single digit or decimal point followed by field deliminator */
                    cw[codeword_count] = first_digit + 248;
                    codeword_count++;
                    i += 2;
                } else if ((first_digit == 11) && (second_digit >= 0) && (second_digit <= 10)) {
                    /* Field deliminator followed by single digit or decimal point */
                    cw[codeword_count] = second_digit + 259;
                    codeword_count++;
                    i += 2;
                }
            }
        }

        if (source[i] < 0x7F) {
            if ((source[i] == '[') && gs1) {
                cw[codeword_count] = 272; // FNC1
            } else {
                cw[codeword_count] = source[i];
            }
            codeword_count++;
            i++;
        }
    } while ((i < in_length) && (i < end_char) && (source[i] < 0x80));

    letters_encoded = i - in_posn;

    //printf("ASCII FRAG: ");
    //for (i = 0; i < codeword_count; i++) {
    //    printf("%d ", cw[i]);
    //}
    //printf("\n");

    *cw_len = codeword_count;

    //printf("%d letters in %d codewords\n", letters_encoded, codeword_count);
    if (codeword_count == 0) {
        return 0.0;
    } else {
        return (float)letters_encoded / (float)codeword_count;
    }
}

int get_subset(unsigned char source[], int in_length, int in_posn) {
    int fragno;
    int subset = 0;

    if (posn(ultra_c43_set1, source[in_posn]) != -1) {
        subset = 1;
    }

    if (posn(ultra_c43_set2, source[in_posn]) != -1) {
        subset = 2;
    }

    if (posn(ultra_c43_set3, source[in_posn]) != -1) {
        subset = 3;
    }

    fragno = ultra_find_fragment(source, in_length, in_posn);
    if ((fragno != -1) && (fragno != 26)) {
        subset = 3;
    }

    return subset;
}

/* Encode characters in the C43 compaction submode */
float look_ahead_c43(unsigned char source[], int in_length, int in_posn, char current_mode, int end_char, int cw[], int* cw_len, int gs1){
    int codeword_count = 0;
    int subcodeword_count = 0;
    int i;
    int subset = 0;
    int fragno;
    int subposn = in_posn;
    int new_subset;
    int unshift_set;
    int base43_value;
    int letters_encoded = 0;
    int pad;
    int gs1_latch = 0;

#ifndef _MSC_VER
    int subcw[in_length];
#else
    int * subcw = (int *) _alloca(in_length * sizeof (int));
#endif /* _MSC_VER */

    subset = get_subset(source, in_length, subposn);

    if (subset == 0) {
        return 0.0;
    }

    if (current_mode == EIGHTBIT_MODE) {
        /* Check for permissable URL C43 macro sequences, otherwise encode directly */
        fragno = ultra_find_fragment(source, in_length, subposn);

        if ((fragno == 2) || (fragno == 3)) {
            // http://www. > http://
            // https://www. > https://
            fragno -= 2;
        }

        switch(fragno) {
            case 17: // mailto:
                cw[codeword_count] = 276;
                subposn += strlen(fragment[fragno]);
                codeword_count++;
                break;
            case 18: // tel:
                cw[codeword_count] = 277;
                subposn += strlen(fragment[fragno]);
                codeword_count++;
                break;
            case 26: // file:
                cw[codeword_count] = 278;
                subposn += strlen(fragment[fragno]);
                codeword_count++;
                break;
            case 0: // http://
                cw[codeword_count] = 279;
                subposn += strlen(fragment[fragno]);
                codeword_count++;
                break;
            case 1: // https://
                cw[codeword_count] = 280;
                subposn += strlen(fragment[fragno]);
                codeword_count++;
                break;
            case 4: // ftp://
                cw[codeword_count] = 281;
                subposn += strlen(fragment[fragno]);
                codeword_count++;
                break;
            default:
                if (subset == 1) {
                    cw[codeword_count] = 260; // C43 Compaction Submode C1
                    codeword_count++;
                }

                if ((subset == 2) || (subset == 3)) {
                    cw[codeword_count] = 266; // C43 Compaction Submode C2
                    codeword_count++;
                }
                break;
        }
    }

    if (current_mode == ASCII_MODE) {
        if (subset == 1) {
            cw[codeword_count] = 278; // C43 Compaction Submode C1
            codeword_count++;
        }

        if ((subset == 2) || (subset == 3)) {
            cw[codeword_count] = 280; // C43 Compaction Submode C2
            codeword_count++;
        }
    }
    unshift_set = subset;

    do {
        if (subset == 1) {
            subcw[subcodeword_count] = posn(ultra_c43_set1, source[subposn]);
            subcodeword_count++;
            subposn++;
        }

        if (subset == 2) {
            subcw[subcodeword_count] = posn(ultra_c43_set2, source[subposn]);
            subcodeword_count++;
            subposn++;
        }

        if (subset == 3) {
            subcw[subcodeword_count] = 41; // Shift to set 3
            subcodeword_count++;

            fragno = ultra_find_fragment(source, in_length, subposn);
            if (fragno == 26) {
                fragno = -1;
            }
            if ((fragno >= 0) && (fragno <= 18)) {
                subcw[subcodeword_count] = fragno;
                subcodeword_count++;
                subposn += strlen(fragment[fragno]);
            }
            if ((fragno >= 18) && (fragno <= 25)) {
                subcw[subcodeword_count] = fragno + 17;
                subcodeword_count++;
                subposn += strlen(fragment[fragno]);
            }
            if (fragno == -1) {
                subcw[subcodeword_count] = posn(ultra_c43_set3, source[subposn]);
                subcodeword_count++;
                subposn++;
            }
            subset = unshift_set;
        }

        if (subposn < in_length) {
            new_subset = get_subset(source, in_length, subposn);

            if (((subset == 1) && (new_subset == 2)) && ((source[subposn] == '.') || (source[subposn] == ','))) {
                /* <period> and <comma> characters available in both subsets */
                new_subset = 1;
            }

            if ((new_subset != subset) && ((new_subset == 1) || (new_subset == 2))) {
                subcw[subcodeword_count] = 42; // Latch to other C43 set
                subcodeword_count++;
                unshift_set = new_subset;
            }

            subset = new_subset;
        }

        /* Check for FNC1 */
        if (subposn < (in_length - 1)) {
            if ((source[subposn + 1] == '[') && gs1) {
                gs1_latch = 1;
            }
        }
    } while ((subposn < in_length) && (subposn < end_char) && (subset != 0) && (gs1_latch == 0));

    pad = 3 - (subcodeword_count % 3);
    if (pad == 3) {
        pad = 0;
    }
    //printf("Pad = %d\n", pad);
    for (i = 0; i < pad; i++) {
        subcw[subcodeword_count] = 42; // Latch to other C43 set used as pad
        subcodeword_count++;
    }

    letters_encoded = subposn - in_posn;

    //printf("C43 SUBFRAG: ");
    //for (i = 0; i < subcodeword_count; i++) {
    //    printf("%d ", subcw[i]);
    //}
    //printf("\n");

    for (i = 0; i < subcodeword_count; i += 3) {
        base43_value = (43 * 43 * subcw[i]) + (43 * subcw[i + 1]) + subcw[i + 2];
        cw[codeword_count] = base43_value / 282;
        codeword_count++;
        cw[codeword_count] = base43_value % 282;
        codeword_count++;
    }

     //printf("C43 FRAG: ");
    //for (i = 0; i < codeword_count; i++) {
     //   printf("%d ", cw[i]);
    //}
    //printf("\n");

    *cw_len = codeword_count;

    //printf("%d letters in %d codewords\n", letters_encoded, codeword_count);
    if (codeword_count == 0) {
        return 0.0;
    } else {
        return (float)letters_encoded / (float)codeword_count;
    }
}

/* Produces a set of codewords which are "somewhat" optimised - this could be improved on */
int ultra_generate_codewords(struct zint_symbol *symbol, const unsigned char source[], const size_t in_length, int codewords[]) {
    int i;
    int crop_length;
    int codeword_count = 0;
    int input_posn = 0;
    char symbol_mode;
    char current_mode;
    float eightbit_score;
    float ascii_score;
    float c43_score;
    int end_char;
    int block_length;
    int fragment_length;
    int fragno;
    int gs1 = 0;

#ifndef _MSC_VER
    unsigned char crop_source[in_length];
    char mode[in_length];
    int cw_fragment[in_length];
#else
    unsigned char * crop_source = (unsigned char *) _alloca(in_length * sizeof (unsigned char));
    char * mode = (char *) _alloca(in_length * sizeof (char));
    int * cw_fragment = (int *) _alloca(in_length * sizeof (int));
#endif /* _MSC_VER */

    /* Section 7.6.2 indicates that ECI \000003 to \811799 are supported */
    /* but this seems to contradict Table 5 which only shows up to \000898 */
    if (symbol->eci > 898) {
        strcpy(symbol->errtxt, "ECI value out of range");
        return ZINT_ERROR_INVALID_OPTION;
    }

    // Decide start character codeword (from Table 5)
    symbol_mode = ASCII_MODE;
    for (i = 0; i < in_length; i++) {
        if (source[i] >= 0x80) {
            symbol_mode = EIGHTBIT_MODE;
        }
    }

    if (symbol->output_options & READER_INIT) {
        /* Reader Initialisation mode */
        if (symbol_mode == ASCII_MODE) {
            codewords[0] = 272; // 7-bit ASCII mode
            codewords[1] = 271; // FNC3
        } else {
            codewords[0] = 257; // 8859-1
            codewords[1] = 269; // FNC3
        }
        codeword_count = 2;
    } else {
        /* Calculate start character codeword */
        if (symbol_mode == ASCII_MODE) {
            if (symbol->input_mode == GS1_MODE) {
                codewords[0] = 273;
            } else {
                codewords[0] = 272;
            }
        } else {
            if ((symbol->eci >= 3) && (symbol->eci <= 18) && (symbol->eci != 14)) {
                // ECI indicate use of character set within ISO/IEC 8859
                codewords[0] = 257 + (symbol->eci - 3);
                if (codewords[0] > 267) {
                    // Avoids ECI 14 for non-existant ISO/IEC 8859-12
                    codewords[0]--;
                }
            } else if (symbol->eci > 18) {
                // ECI indicates use of character set outside ISO/IEC 8859
                codewords[0] = 273 + (symbol->eci / 256);
                codewords[1] = symbol->eci % 256;
                codeword_count++;
            } else {
                codewords[0] = 257; // Default is assumed to be ISO/IEC 8859-1 (ECI 3)
            }
        }

        if ((codewords[0] == 257) || (codewords[0] == 272)) {
            fragno = ultra_find_fragment((unsigned char *)source, in_length, 0);

            // Check for http:// at start of input
            if ((fragno == 0) || (fragno == 2)) {
                codewords[0] = 281;
                input_posn = 7;
                symbol_mode = EIGHTBIT_MODE;
            }


            // Check for https:// at start of input
            if ((fragno == 1) || (fragno == 3)) {
                codewords[0] = 282;
                input_posn = 8;
                symbol_mode = EIGHTBIT_MODE;
            }
        }
    }

    codeword_count++;

    /* Check for 06 Macro Sequence and crop accordingly */
    if (in_length >= 9
            && source[0] == '[' && source[1] == ')' && source[2] == '>' && source[3] == '\x1e'
            && source[4] == '0' && source[5] == '6' && source[6] == '\x1d'
            && source[in_length - 2] == '\x1e' && source[in_length - 1] == '\x04') {

            if (symbol_mode == EIGHTBIT_MODE) {
                codewords[codeword_count] = 271; // 06 Macro
            } else {
                codewords[codeword_count] = 273; // 06 Macro
            }
            codeword_count++;

            for (i = 7; i < (in_length - 2); i++) {
                crop_source[i - 7] = source[i];
            }
            crop_length = in_length - 9;
            crop_source[crop_length] = '\0';
   } else {
        /* Make a cropped version of input data - removes http:// and https:// if needed */
        for (i = input_posn; i < in_length; i++) {
            crop_source[i - input_posn] = source[i];
        }
        crop_length = in_length - input_posn;
        crop_source[crop_length] = '\0';
    }

    if ((symbol->input_mode & 0x07) == GS1_MODE) {
        gs1 = 1;
    }

    /* Attempt encoding in all three modes to see which offers best compaction and store results */
    current_mode = symbol_mode;
    input_posn = 0;
    do {
        end_char = input_posn + PREDICT_WINDOW;
        eightbit_score = look_ahead_eightbit(crop_source, crop_length, input_posn, current_mode, end_char, cw_fragment, &fragment_length, gs1);
        ascii_score = look_ahead_ascii(crop_source, crop_length, input_posn, current_mode, symbol_mode, end_char, cw_fragment, &fragment_length, gs1);
        c43_score = look_ahead_c43(crop_source, crop_length, input_posn, current_mode, end_char, cw_fragment, &fragment_length, gs1);

        mode[input_posn] = 'a';
        current_mode = ASCII_MODE;

        if ((c43_score > ascii_score) && (c43_score > eightbit_score)) {
            mode[input_posn] = 'c';
            current_mode = C43_MODE;
        }

        if ((eightbit_score > ascii_score) && (eightbit_score > c43_score)) {
            mode[input_posn] = '8';
            current_mode = EIGHTBIT_MODE;
        }
        input_posn++;
    } while (input_posn < crop_length);
    mode[input_posn] = '\0';

    /* Use results from test to perform actual mode switching */
    current_mode = symbol_mode;
    input_posn = 0;
    do {
        block_length = 0;
        do {
            block_length++;
        } while (mode[input_posn + block_length] == mode[input_posn]);

        switch(mode[input_posn]) {
            case 'a':
                ascii_score = look_ahead_ascii(crop_source, crop_length, input_posn, current_mode, symbol_mode, input_posn + block_length, cw_fragment, &fragment_length, gs1);
                current_mode = ASCII_MODE;
                break;
            case 'c':
                c43_score = look_ahead_c43(crop_source, crop_length, input_posn, current_mode, input_posn + block_length, cw_fragment, &fragment_length, gs1);

                /* Substitute temporary latch if possible */
                if ((current_mode == EIGHTBIT_MODE) && (cw_fragment[0] == 261) && (fragment_length >= 5) && (fragment_length <= 11)) {
                    /* Temporary latch to submode 1 from Table 11 */
                    cw_fragment[0] = 256 + ((fragment_length - 5) / 2);
                } else if ((current_mode == EIGHTBIT_MODE) && (cw_fragment[0] == 266) && (fragment_length >= 5) && (fragment_length <= 11)) {
                    /* Temporary latch to submode 2 from Table 11 */
                    cw_fragment[0] = 262 + ((fragment_length - 5) / 2);
                } else if ((current_mode == ASCII_MODE) && (cw_fragment[0] == 278) && (fragment_length >= 5) && (fragment_length <= 11)) {
                    /* Temporary latch to submode 1 from Table 9 */
                    cw_fragment[0] = 274 + ((fragment_length - 5) / 2);
                } else {
                    current_mode = C43_MODE;
                }
                break;
            case '8':
                eightbit_score = look_ahead_eightbit(crop_source, crop_length, input_posn, current_mode, input_posn + block_length, cw_fragment, &fragment_length, gs1);
                current_mode = EIGHTBIT_MODE;
                break;
        }

        for (i = 0; i < fragment_length; i++) {
            codewords[codeword_count + i] = cw_fragment[i];
        }
        codeword_count += fragment_length;

        input_posn += block_length;
    } while (input_posn < crop_length);

    //printf("RED: %s\n", crop_source);
    //printf("MOD: %s\n", mode);

    return codeword_count;
}

int ultracode(struct zint_symbol *symbol, const unsigned char source[], const size_t in_length) {
    int data_cw_count = 0;
    int ecc_cw; // = Q in section 7.7.1
    int ecc_level;
    int misdecode_cw; // = P in section 7.7.1
    int rows, columns;
    int total_cws;
    int pads;
    int cw_memalloc;
    int codeword[283];
    int i, posn;
    int acc;

    cw_memalloc = in_length * 2;
    if (cw_memalloc < 283) {
        cw_memalloc = 283;
    }

#ifndef _MSC_VER
    int data_codewords[cw_memalloc];
#else
    int* data_codewords = (int *) _alloca(cw_memalloc * sizeof (int));
#endif /* _MSC_VER */

    data_cw_count = ultra_generate_codewords(symbol, source, in_length, data_codewords);

    //printf("Codewords returned = %d\n", data_cw_count);

    //for (int i = 0; i < data_cw_count; i++) {
    //    printf("%d ", data_codewords[i]);
    //}
    //printf("\n");

    /* Default ECC level is EC2 */
    if ((symbol->option_1 <= 0) || (symbol->option_1 > 6)) {
        ecc_level = 2;
    } else {
        ecc_level = symbol->option_1 - 1;
    }

    if (ecc_level == 0) {
        misdecode_cw = 0;
    } else {
        misdecode_cw = 3;
    }

    /* ECC calculation from section 7.7.2 */
    if ((data_cw_count % 25) == 0) {
        ecc_cw = (ecc_level * (data_cw_count / 25)) + misdecode_cw + 2;
    } else {
        ecc_cw = (ecc_level * ((data_cw_count / 25) + 1)) + misdecode_cw + 2;
    }

    //printf("ECC codewords: %d\n", ecc_cw);

    /* Maximum capacity is 282 codewords */
    if (data_cw_count + ecc_cw > 282) {
        strcpy(symbol->errtxt, "Data too long for selected error correction capacity");
        return ZINT_ERROR_TOO_LONG;
    }

    total_cws = data_cw_count + ecc_cw;

    rows = 5;
    for (i = 2; i >= 0; i--) {
        if (total_cws < ultra_maxsize[i]) {
            rows--;
        }
    }

    if ((total_cws % rows) == 0) {
        pads = 0;
        columns = total_cws / rows;
    } else {
        pads = rows - (total_cws % rows);
        columns = (total_cws + pads) / rows;
    }

    printf("Calculated size is %d rows by %d columns\n", rows, columns);

    acc = ecc_cw - misdecode_cw; // ACC = (Q-P) - section 6.11.6

    /* Insert MCC and ACC into data codewords */
    for (i = 282; i > 2; i--) {
        data_codewords[i] = data_codewords[i - 2];
    }
    data_codewords[1] = data_cw_count += 2; // MCC
    data_codewords[2] = acc; // ACC

    /* Calculate error correction codewords (RSEC) */
    ultra_gf283((short) data_cw_count, (short) ecc_cw, data_codewords);

    /* Rearrange to make final codeword sequence */
    posn = 0;
    codeword[posn++] = data_codewords[282 - (data_cw_count + ecc_cw)]; // Start Character
    codeword[posn++] = data_cw_count; // MCC
    for (i = 0; i < ecc_cw; i++) {
        codeword[posn++] = data_codewords[(282 - ecc_cw) + i]; // RSEC Region
    }
    codeword[posn++] = data_cw_count + ecc_cw; // TCC = C + Q - section 6.11.4
    codeword[posn++] = 283; // Separator
    codeword[posn++] = acc; // ACC
    for (i = 0; i < (data_cw_count - 3); i++) {
        codeword[posn++] = data_codewords[(282 - ((data_cw_count - 3) + ecc_cw)) + i]; // Data Region
    }
    for (i = 0; i < pads; i++) {
        codeword[posn++] = 284; // Pad pattern
    }
    codeword[posn++] = ecc_cw; // QCC

    printf("Rearranged codewords with ECC:\n");
    for (i = 0; i < posn; i++) {
        printf("%d ", codeword[i]);
    }
    printf("\n");

    strcpy(symbol->errtxt, "1000: Ultracode has not been implemented - yet!");
    return ZINT_ERROR_INVALID_OPTION;
}
