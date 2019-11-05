/* composite.c - Handles GS1 Composite Symbols */

/*
    libzint - the open source barcode library
    Copyright (C) 2008-2019 Robin Stuart <rstuart114@gmail.com>

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

/* The functions "getBit", "init928" and "encode928" are copyright BSI and are
   released with permission under the following terms:

   "Copyright subsists in all BSI publications. BSI also holds the copyright, in the
   UK, of the international standardisation bodies. Except as
   permitted under the Copyright, Designs and Patents Act 1988 no extract may be
   reproduced, stored in a retrieval system or transmitted in any form or by any
   means - electronic, photocopying, recording or otherwise - without prior written
   permission from BSI.

   "This does not preclude the free use, in the course of implementing the standard,
   of necessary details such as symbols, and size, type or grade designations. If these
   details are to be used for any other purpose than implementation then the prior
   written permission of BSI must be obtained."

   The date of publication for these functions is 31 May 2006
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"
#include "composite.h"
#include "pdf417.h"
#include "gs1.h"
#include "general_field.h"

#define UINT unsigned short

extern int eanx(struct zint_symbol *symbol, unsigned char source[], int length);
extern int ean_128(struct zint_symbol *symbol, unsigned char source[], const size_t length);
extern void ean_leading_zeroes(struct zint_symbol *symbol, unsigned char source[], unsigned char local_source[]);
extern int rss14(struct zint_symbol *symbol, unsigned char source[], int length);
extern int rsslimited(struct zint_symbol *symbol, unsigned char source[], int length);
extern int rssexpanded(struct zint_symbol *symbol, unsigned char source[], int length);

static UINT pwr928[69][7];

static int _min(int first, int second) {

    if (first <= second)
        return first;
    else
        return second;
}

/* gets bit in bitString at bitPos */
static int getBit(UINT *bitStr, int bitPos) {
    return !!(bitStr[bitPos >> 4] & (0x8000 >> (bitPos & 15)));
}

/* initialize pwr928 encoding table */
static void init928(void) {
    int i, j, v;
    int cw[7];
    cw[6] = 1L;
    for (i = 5; i >= 0; i--)
        cw[i] = 0;

    for (i = 0; i < 7; i++)
        pwr928[0][i] = cw[i];
    for (j = 1; j < 69; j++) {
        for (v = 0, i = 6; i >= 1; i--) {
            v = (2 * cw[i]) + (v / 928);
            pwr928[j][i] = cw[i] = v % 928;
        }
        pwr928[j][0] = cw[0] = (2 * cw[0]) + (v / 928);
    }
    return;
}

/* converts bit string to base 928 values, codeWords[0] is highest order */
static int encode928(UINT bitString[], UINT codeWords[], int bitLng) {
    int i, j, b, cwNdx, cwLng;
    for (cwNdx = cwLng = b = 0; b < bitLng; b += 69, cwNdx += 7) {
        int bitCnt = _min(bitLng - b, 69);
        int cwCnt;
        cwLng += cwCnt = bitCnt / 10 + 1;
        for (i = 0; i < cwCnt; i++)
            codeWords[cwNdx + i] = 0; /* init 0 */
        for (i = 0; i < bitCnt; i++) {
            if (getBit(bitString, b + bitCnt - i - 1)) {
                for (j = 0; j < cwCnt; j++)
                    codeWords[cwNdx + j] += pwr928[i][j + 7 - cwCnt];
            }
        }
        for (i = cwCnt - 1; i > 0; i--) {
            /* add "carries" */
            codeWords[cwNdx + i - 1] += codeWords[cwNdx + i] / 928L;
            codeWords[cwNdx + i] %= 928L;
        }
    }
    return (cwLng);
}

/* CC-A 2D component */
static int cc_a(struct zint_symbol *symbol, char source[], int cc_width) {
    int i, segment, bitlen, cwCnt, variant, rows;
    int k, offset, j, total, rsCodeWords[8];
    int LeftRAPStart, RightRAPStart, CentreRAPStart, StartCluster;
    int LeftRAP, RightRAP, CentreRAP, Cluster, dummy[5];
    int loop;
    UINT codeWords[28];
    UINT bitStr[13];
    char pattern[580];
    char local_source[210]; /* A copy of source but with padding zeroes to make 208 bits */

    variant = 0;

    for (i = 0; i < 13; i++) {
        bitStr[i] = 0;
    }
    for (i = 0; i < 28; i++) {
        codeWords[i] = 0;
    }

    bitlen = (int)strlen(source);

    for (i = 0; i < 208; i++) {
        local_source[i] = '0';
    }
    for (i = 0; i < bitlen; i++) {
        local_source[i] = source[i];
    }
    local_source[208] = '\0';

    for (segment = 0; segment < 13; segment++) {
        int strpos = segment * 16;
        for (i = 0; i < 16; i++) {
            if (local_source[strpos + i] == '1') {
                bitStr[segment] += (0x8000 >> i);
            }
        }
    }

    init928();
    /* encode codeWords from bitStr */
    cwCnt = encode928(bitStr, codeWords, bitlen);

    switch (cc_width) {
        case 2:
            switch (cwCnt) {
                case 6: variant = 0;
                    break;
                case 8: variant = 1;
                    break;
                case 9: variant = 2;
                    break;
                case 11: variant = 3;
                    break;
                case 12: variant = 4;
                    break;
                case 14: variant = 5;
                    break;
                case 17: variant = 6;
                    break;
            }
            break;
        case 3:
            switch (cwCnt) {
                case 8: variant = 7;
                    break;
                case 10: variant = 8;
                    break;
                case 12: variant = 9;
                    break;
                case 14: variant = 10;
                    break;
                case 17: variant = 11;
                    break;
            }
            break;
        case 4:
            switch (cwCnt) {
                case 8: variant = 12;
                    break;
                case 11: variant = 13;
                    break;
                case 14: variant = 14;
                    break;
                case 17: variant = 15;
                    break;
                case 20: variant = 16;
                    break;
            }
            break;
    }

    rows = ccaVariants[variant];
    k = ccaVariants[17 + variant];
    offset = ccaVariants[34 + variant];

    /* Reed-Solomon error correction */

    for (i = 0; i < 8; i++) {
        rsCodeWords[i] = 0;
    }
    total = 0;
    for (i = 0; i < cwCnt; i++) {
        total = (codeWords[i] + rsCodeWords[k - 1]) % 929;
        for (j = k - 1; j >= 0; j--) {
            if (j == 0) {
                rsCodeWords[j] = (929 - (total * ccaCoeffs[offset + j]) % 929) % 929;
            } else {
                rsCodeWords[j] = (rsCodeWords[j - 1] + 929 - (total * ccaCoeffs[offset + j]) % 929) % 929;
            }
        }
    }

    for (j = 0; j < k; j++) {
        if (rsCodeWords[j] != 0) {
            rsCodeWords[j] = 929 - rsCodeWords[j];
        }
    }

    for (i = k - 1; i >= 0; i--) {
        codeWords[cwCnt] = rsCodeWords[i];
        cwCnt++;
    }

    /* Place data into table */
    LeftRAPStart = aRAPTable[variant];
    CentreRAPStart = aRAPTable[variant + 17];
    RightRAPStart = aRAPTable[variant + 34];
    StartCluster = aRAPTable[variant + 51] / 3;

    LeftRAP = LeftRAPStart;
    CentreRAP = CentreRAPStart;
    RightRAP = RightRAPStart;
    Cluster = StartCluster; /* Cluster can be 0, 1 or 2 for Cluster(0), Cluster(3) and Cluster(6) */

    for (i = 0; i < rows; i++) {
        strcpy(pattern, "");
        offset = 929 * Cluster;
        for (j = 0; j < 5; j++) {
            dummy[j] = 0;
        }
        for (j = 0; j < cc_width; j++) {
            dummy[j + 1] = codeWords[i * cc_width + j];
        }
        /* Copy the data into codebarre */
        if (cc_width != 3) {
            bin_append(rap_side[LeftRAP - 1], 10, pattern);
        }
        bin_append(pdf_bitpattern[offset + dummy[1]], 16, pattern);
        strcat(pattern, "0");
        if (cc_width == 3) {
            bin_append(rap_centre[CentreRAP - 1], 10, pattern);
        }
        if (cc_width >= 2) {
            bin_append(pdf_bitpattern[offset + dummy[2]], 16, pattern);
            strcat(pattern, "0");
        }
        if (cc_width == 4) {
            bin_append(rap_centre[CentreRAP - 1], 10, pattern);
        }
        if (cc_width >= 3) {
            bin_append(pdf_bitpattern[offset + dummy[3]], 16, pattern);
            strcat(pattern, "0");
        }
        if (cc_width == 4) {
            bin_append(pdf_bitpattern[offset + dummy[4]], 16, pattern);
            strcat(pattern, "0");
        }
        bin_append(rap_side[RightRAP - 1], 10, pattern);
        strcat(pattern, "1"); /* stop */

        /* so now pattern[] holds the string of '1's and '0's. - copy this to the symbol */
        for (loop = 0; loop < (int) strlen(pattern); loop++) {
            if (pattern[loop] == '1') {
                set_module(symbol, i, loop);
            }
        }
        symbol->row_height[i] = 2;
        symbol->rows++;
        symbol->width = strlen(pattern);

        /* Set up RAPs and Cluster for next row */
        LeftRAP++;
        CentreRAP++;
        RightRAP++;
        Cluster++;

        if (LeftRAP == 53) {
            LeftRAP = 1;
        }
        if (CentreRAP == 53) {
            CentreRAP = 1;
        }
        if (RightRAP == 53) {
            RightRAP = 1;
        }
        if (Cluster == 3) {
            Cluster = 0;
        }
    }

    return 0;
}

/* CC-B 2D component */
static int cc_b(struct zint_symbol *symbol, char source[], int cc_width) {
    int length, i;
#ifndef _MSC_VER
    unsigned char data_string[(strlen(source) / 8) + 3];
#else
    unsigned char* data_string = (unsigned char*) _alloca((strlen(source) / 8) + 3);
#endif
    int chainemc[180], mclength;
    int k, j, p, longueur, mccorrection[50], offset;
    int total, dummy[5];
    char pattern[580];
    int variant, LeftRAPStart, CentreRAPStart, RightRAPStart, StartCluster;
    int LeftRAP, CentreRAP, RightRAP, Cluster, loop;

    length = strlen(source) / 8;

    for (i = 0; i < length; i++) {
        int binloc = i * 8;

        data_string[i] = 0;
        for (p = 0; p < 8; p++) {
            if (source[binloc + p] == '1') {
                data_string[i] += (0x80 >> p);
            }
        }
    }


    mclength = 0;

    /* "the CC-B component shall have codeword 920 in the first symbol character position" (section 9a) */
    chainemc[mclength] = 920;
    mclength++;

    byteprocess(chainemc, &mclength, data_string, 0, length);

    /* Now figure out which variant of the symbol to use and load values accordingly */

    variant = 0;

    if (cc_width == 2) {
        variant = 13;
        if (mclength <= 33) {
            variant = 12;
        }
        if (mclength <= 29) {
            variant = 11;
        }
        if (mclength <= 24) {
            variant = 10;
        }
        if (mclength <= 19) {
            variant = 9;
        }
        if (mclength <= 13) {
            variant = 8;
        }
        if (mclength <= 8) {
            variant = 7;
        }
    }

    if (cc_width == 3) {
        variant = 23;
        if (mclength <= 70) {
            variant = 22;
        }
        if (mclength <= 58) {
            variant = 21;
        }
        if (mclength <= 46) {
            variant = 20;
        }
        if (mclength <= 34) {
            variant = 19;
        }
        if (mclength <= 24) {
            variant = 18;
        }
        if (mclength <= 18) {
            variant = 17;
        }
        if (mclength <= 14) {
            variant = 16;
        }
        if (mclength <= 10) {
            variant = 15;
        }
        if (mclength <= 6) {
            variant = 14;
        }
    }

    if (cc_width == 4) {
        variant = 34;
        if (mclength <= 108) {
            variant = 33;
        }
        if (mclength <= 90) {
            variant = 32;
        }
        if (mclength <= 72) {
            variant = 31;
        }
        if (mclength <= 54) {
            variant = 30;
        }
        if (mclength <= 39) {
            variant = 29;
        }
        if (mclength <= 30) {
            variant = 28;
        }
        if (mclength <= 24) {
            variant = 27;
        }
        if (mclength <= 18) {
            variant = 26;
        }
        if (mclength <= 12) {
            variant = 25;
        }
        if (mclength <= 8) {
            variant = 24;
        }
    }

    /* Now we have the variant we can load the data - from here on the same as MicroPDF417 code */
    variant--;
    assert(variant >= 0);
    symbol->option_2 = MicroVariants[variant]; /* columns */
    symbol->rows = MicroVariants[variant + 34]; /* rows */
    k = MicroVariants[variant + 68]; /* number of EC CWs */
    longueur = (symbol->option_2 * symbol->rows) - k; /* number of non-EC CWs */
    i = longueur - mclength; /* amount of padding required */
    offset = MicroVariants[variant + 102]; /* coefficient offset */

    /* We add the padding */
    while (i > 0) {
        chainemc[mclength] = 900;
        mclength++;
        i--;
    }

    /* Reed-Solomon error correction */
    longueur = mclength;
    for (loop = 0; loop < 50; loop++) {
        mccorrection[loop] = 0;
    }
    total = 0;
    for (i = 0; i < longueur; i++) {
        total = (chainemc[i] + mccorrection[k - 1]) % 929;
        for (j = k - 1; j >= 0; j--) {
            if (j == 0) {
                mccorrection[j] = (929 - (total * Microcoeffs[offset + j]) % 929) % 929;
            } else {
                mccorrection[j] = (mccorrection[j - 1] + 929 - (total * Microcoeffs[offset + j]) % 929) % 929;
            }
        }
    }

    for (j = 0; j < k; j++) {
        if (mccorrection[j] != 0) {
            mccorrection[j] = 929 - mccorrection[j];
        }
    }
    /* we add these codes to the string */
    for (i = k - 1; i >= 0; i--) {
        chainemc[mclength] = mccorrection[i];
        mclength++;
    }

    /* Now get the RAP (Row Address Pattern) start values */
    LeftRAPStart = RAPTable[variant];
    CentreRAPStart = RAPTable[variant + 34];
    RightRAPStart = RAPTable[variant + 68];
    StartCluster = RAPTable[variant + 102] / 3;

    /* That's all values loaded, get on with the encoding */

    LeftRAP = LeftRAPStart;
    CentreRAP = CentreRAPStart;
    RightRAP = RightRAPStart;
    Cluster = StartCluster;
    /* Cluster can be 0, 1 or 2 for Cluster(0), Cluster(3) and Cluster(6) */

    for (i = 0; i < symbol->rows; i++) {
        strcpy(pattern, "");
        offset = 929 * Cluster;
        for (j = 0; j < 5; j++) {
            dummy[j] = 0;
        }
        for (j = 0; j < symbol->option_2; j++) {
            dummy[j + 1] = chainemc[i * symbol->option_2 + j];
        }
        /* Copy the data into codebarre */
        bin_append(rap_side[LeftRAP - 1], 10, pattern);
        bin_append(pdf_bitpattern[offset + dummy[1]], 16, pattern);
        strcat(pattern, "0");
        if (cc_width == 3) {
            bin_append(rap_centre[CentreRAP - 1], 10, pattern);
        }
        if (cc_width >= 2) {
            bin_append(pdf_bitpattern[offset + dummy[2]], 16, pattern);
            strcat(pattern, "0");
        }
        if (cc_width == 4) {
            bin_append(rap_centre[CentreRAP - 1], 10, pattern);
        }
        if (cc_width >= 3) {
            bin_append(pdf_bitpattern[offset + dummy[3]], 16, pattern);
            strcat(pattern, "0");
        }
        if (cc_width == 4) {
            bin_append(pdf_bitpattern[offset + dummy[4]], 16, pattern);
            strcat(pattern, "0");
        }
        bin_append(rap_side[RightRAP - 1], 10, pattern);
        strcat(pattern, "1"); /* stop */

        /* so now pattern[] holds the string of '1's and '0's. - copy this to the symbol */
        for (loop = 0; loop < (int) strlen(pattern); loop++) {
            if (pattern[loop] == '1') {
                set_module(symbol, i, loop);
            }
        }
        symbol->row_height[i] = 2;
        symbol->width = strlen(pattern);

        /* Set up RAPs and Cluster for next row */
        LeftRAP++;
        CentreRAP++;
        RightRAP++;
        Cluster++;

        if (LeftRAP == 53) {
            LeftRAP = 1;
        }
        if (CentreRAP == 53) {
            CentreRAP = 1;
        }
        if (RightRAP == 53) {
            RightRAP = 1;
        }
        if (Cluster == 3) {
            Cluster = 0;
        }
    }

    return 0;
}

/* CC-C 2D component - byte compressed PDF417 */
static int cc_c(struct zint_symbol *symbol, char source[], int cc_width, int ecc_level) {
    int length, i, p;
#ifndef _MSC_VER
    unsigned char data_string[(strlen(source) / 8) + 4];
#else
    unsigned char* data_string = (unsigned char*) _alloca((strlen(source) / 8) + 4);
#endif
    int chainemc[1000], mclength, k;
    int offset, longueur, loop, total, j, mccorrection[520];
    int c1, c2, c3, dummy[35];
    char pattern[580];

    length = strlen(source) / 8;

    for (i = 0; i < length; i++) {
        int binloc = i * 8;

        data_string[i] = 0;
        for (p = 0; p < 8; p++) {
            if (source[binloc + p] == '1') {
                data_string[i] += (0x80 >> p);
            }
        }
    }

    mclength = 0;

    chainemc[mclength] = 0; /* space for length descriptor */
    mclength++;
    chainemc[mclength] = 920; /* CC-C identifier */
    mclength++;

    byteprocess(chainemc, &mclength, data_string, 0, length);

    chainemc[0] = mclength;

    k = 1;
    for (i = 1; i <= (ecc_level + 1); i++) {
        k *= 2;
    }

    /* 796 - we now take care of the Reed Solomon codes */
    switch (ecc_level) {
        case 1: offset = 2;
            break;
        case 2: offset = 6;
            break;
        case 3: offset = 14;
            break;
        case 4: offset = 30;
            break;
        case 5: offset = 62;
            break;
        case 6: offset = 126;
            break;
        case 7: offset = 254;
            break;
        case 8: offset = 510;
            break;
        default: offset = 0;
            break;
    }

    longueur = mclength;
    for (loop = 0; loop < 520; loop++) {
        mccorrection[loop] = 0;
    }
    total = 0;
    for (i = 0; i < longueur; i++) {
        total = (chainemc[i] + mccorrection[k - 1]) % 929;
        for (j = k - 1; j >= 0; j--) {
            if (j == 0) {
                mccorrection[j] = (929 - (total * coefrs[offset + j]) % 929) % 929;
            } else {
                mccorrection[j] = (mccorrection[j - 1] + 929 - (total * coefrs[offset + j]) % 929) % 929;
            }
        }
    }

    for (j = 0; j < k; j++) {
        if (mccorrection[j] != 0) {
            mccorrection[j] = 929 - mccorrection[j];
        }
    }
    /* we add these codes to the string */
    for (i = k - 1; i >= 0; i--) {
        chainemc[mclength] = mccorrection[i];
        mclength++;
    }

    /* 818 - The CW string is finished */
    c1 = (mclength / cc_width - 1) / 3;
    c2 = ecc_level * 3 + (mclength / cc_width - 1) % 3;
    c3 = cc_width - 1;

    /* we now encode each row */
    for (i = 0; i <= (mclength / cc_width) - 1; i++) {
        for (j = 0; j < cc_width; j++) {
            dummy[j + 1] = chainemc[i * cc_width + j];
        }
        k = (i / 3) * 30;
        switch (i % 3) {
            case 0:
                dummy[0] = k + c1;
                dummy[cc_width + 1] = k + c3;
                offset = 0; /* cluster(0) */
                break;
            case 1:
                dummy[0] = k + c2;
                dummy[cc_width + 1] = k + c1;
                offset = 929; /* cluster(3) */
                break;
            case 2:
                dummy[0] = k + c3;
                dummy[cc_width + 1] = k + c2;
                offset = 1858; /* cluster(6) */
                break;
        }
        strcpy(pattern, "");
        bin_append(0x1FEA8, 17, pattern); /* Row start */

        for (j = 0; j <= cc_width + 1; j++) {
            bin_append(pdf_bitpattern[offset + dummy[j]], 16, pattern);
            strcat(pattern, "0");
        }
        bin_append(0x3FA29, 18, pattern); /* Row Stop */

        for (loop = 0; loop < (int) strlen(pattern); loop++) {
            if (pattern[loop] == '1') {
                set_module(symbol, i, loop);
            }
        }
        symbol->row_height[i] = 3;
    }
    symbol->rows = (mclength / cc_width);
    symbol->width = (int)strlen(pattern);

    return 0;
}

static int calc_padding_cca(int binary_length, int cc_width) {
    int target_bitsize = 0;

    switch (cc_width) {
        case 2:
            if (binary_length <= 167) {
                target_bitsize = 167;
            }
            if (binary_length <= 138) {
                target_bitsize = 138;
            }
            if (binary_length <= 118) {
                target_bitsize = 118;
            }
            if (binary_length <= 108) {
                target_bitsize = 108;
            }
            if (binary_length <= 88) {
                target_bitsize = 88;
            }
            if (binary_length <= 78) {
                target_bitsize = 78;
            }
            if (binary_length <= 59) {
                target_bitsize = 59;
            }
            break;
        case 3:
            if (binary_length <= 167) {
                target_bitsize = 167;
            }
            if (binary_length <= 138) {
                target_bitsize = 138;
            }
            if (binary_length <= 118) {
                target_bitsize = 118;
            }
            if (binary_length <= 98) {
                target_bitsize = 98;
            }
            if (binary_length <= 78) {
                target_bitsize = 78;
            }
            break;
        case 4:
            if (binary_length <= 197) {
                target_bitsize = 197;
            }
            if (binary_length <= 167) {
                target_bitsize = 167;
            }
            if (binary_length <= 138) {
                target_bitsize = 138;
            }
            if (binary_length <= 108) {
                target_bitsize = 108;
            }
            if (binary_length <= 78) {
                target_bitsize = 78;
            }
            break;
        }

        return target_bitsize;
}

int calc_padding_ccb(int binary_length, int cc_width) {
    int target_bitsize = 0;

    switch (cc_width) {
        case 2:
            if (binary_length <= 336) {
                target_bitsize = 336;
            }
            if (binary_length <= 296) {
                target_bitsize = 296;
            }
            if (binary_length <= 256) {
                target_bitsize = 256;
            }
            if (binary_length <= 208) {
                target_bitsize = 208;
            }
            if (binary_length <= 160) {
                target_bitsize = 160;
            }
            if (binary_length <= 104) {
                target_bitsize = 104;
            }
            if (binary_length <= 56) {
                target_bitsize = 56;
            }
            break;
        case 3:
            if (binary_length <= 768) {
                target_bitsize = 768;
            }
            if (binary_length <= 648) {
                target_bitsize = 648;
            }
            if (binary_length <= 536) {
                target_bitsize = 536;
            }
            if (binary_length <= 416) {
                target_bitsize = 416;
            }
            if (binary_length <= 304) {
                target_bitsize = 304;
            }
            if (binary_length <= 208) {
                target_bitsize = 208;
            }
            if (binary_length <= 152) {
                target_bitsize = 152;
            }
            if (binary_length <= 112) {
                target_bitsize = 112;
            }
            if (binary_length <= 72) {
                target_bitsize = 72;
            }
            if (binary_length <= 32) {
                target_bitsize = 32;
            }
            break;
        case 4:
            if (binary_length <= 1184) {
                target_bitsize = 1184;
            }
            if (binary_length <= 1016) {
                target_bitsize = 1016;
            }
            if (binary_length <= 840) {
                target_bitsize = 840;
            }
            if (binary_length <= 672) {
                target_bitsize = 672;
            }
            if (binary_length <= 496) {
                target_bitsize = 496;
            }
            if (binary_length <= 352) {
                target_bitsize = 352;
            }
            if (binary_length <= 264) {
                target_bitsize = 264;
            }
            if (binary_length <= 208) {
                target_bitsize = 208;
            }
            if (binary_length <= 152) {
                target_bitsize = 152;
            }
            if (binary_length <= 96) {
                target_bitsize = 96;
            }
            if (binary_length <= 56) {
                target_bitsize = 56;
            }
            break;
    }

    return target_bitsize;
}

int calc_padding_ccc(int binary_length, int *cc_width, int lin_width, int *ecc) {
    int target_bitsize = 0;
    int byte_length, codewords_used, ecc_level, ecc_codewords, rows;
    int codewords_total, target_codewords, target_bytesize;

    byte_length = binary_length / 8;
    if (binary_length % 8 != 0) {
        byte_length++;
    }

    codewords_used = (byte_length / 6) * 5;
    codewords_used += byte_length % 6;

    /* Recommended minimum ecc levels ISO/IEC 1543:2015 (PDF417) Annex E Table E.1,
       restricted by CC-C codeword max 900 (30 cols * 30 rows), GS1 General Specifications 19.1 5.9.2.3 */
    if (codewords_used <= 40) {
        ecc_level = 2;
    } else if (codewords_used <= 160) {
        ecc_level = 3;
    } else if (codewords_used <= 320) {
        ecc_level = 4;
    } else if (codewords_used <= 833) { /* 900 - 3 - 64 */
        ecc_level = 5;
    } else if (codewords_used <= 865) { /* 900 - 3 - 32 */
        ecc_level = 4; /* Not recommended but allow to meet advertised "up to 2361 digits" (allows max 2372) */
    } else {
        return 0;
    }
    *(ecc) = ecc_level;
    ecc_codewords = 1 << (ecc_level + 1);

    codewords_used += ecc_codewords;
    codewords_used += 3;

    *(cc_width) = (lin_width - 53) / 17; // -53 = (6 left quiet zone + 10 right quiet zone - (17 * 3 + 18))
    if (*(cc_width) > 30) {
        *(cc_width) = 30;
    }
    rows = ceil((double) codewords_used / *(cc_width));
    /* stop the symbol from becoming too high */
    while (rows > 30 && *(cc_width) < 30) {
        *(cc_width) = *(cc_width) + 1;
        rows = ceil((double) codewords_used / *(cc_width));
    }

    if (rows > 30) {
        return 0;
    }
    if (rows < 3) {
        rows = 3;
    }

    codewords_total = *(cc_width) * rows;

    target_codewords = codewords_total - ecc_codewords;
    target_codewords -= 3;

    target_bytesize = 6 * (target_codewords / 5);
    target_bytesize += target_codewords % 5;

    target_bitsize = 8 * target_bytesize;

    return target_bitsize;
}

static int cc_binary_string(struct zint_symbol *symbol, const char source[], char binary_string[], int cc_mode, int *cc_width, int *ecc, int lin_width) { /* Handles all data encodation from section 5 of ISO/IEC 24723 */
    int encoding_method, read_posn, alpha_pad;
    int i, j, ai_crop, ai_crop_posn, fnc1_latch;
    int ai90_mode, last_digit, remainder, binary_length;
    int mode;
    int source_len = strlen(source);
#ifndef _MSC_VER
    char general_field[source_len + 1];
#else
    char* general_field = (char*) _alloca(source_len + 1);
#endif
    int target_bitsize;

    encoding_method = 1;
    read_posn = 0;
    ai_crop = 0;
    ai_crop_posn = -1;
    fnc1_latch = 0;
    alpha_pad = 0;
    ai90_mode = 0;
    *ecc = 0;
    target_bitsize = 0;
    mode = NUMERIC;

    if ((source[0] == '1') && ((source[1] == '0') || (source[1] == '1') || (source[1] == '7'))) {
        /* Source starts (10), (11) or (17) */
        encoding_method = 2;
    }

    if ((source[0] == '9') && (source[1] == '0')) {
        /* Source starts (90) */
        encoding_method = 3;
    }

    if (encoding_method == 1) {
        strcat(binary_string, "0");
    }

    if (encoding_method == 2) {
        /* Encoding Method field "10" - date and lot number */

        strcat(binary_string, "10");

        if (source[1] == '0') {
            /* No date data */
            strcat(binary_string, "11");
            read_posn = 2;
        } else {
            long int group_val;
            /* Production Date (11) or Expiration Date (17) */
            char date_str[4];
            date_str[0] = source[2];
            date_str[1] = source[3];
            date_str[2] = '\0';
            group_val = atoi(date_str) * 384;

            date_str[0] = source[4];
            date_str[1] = source[5];
            group_val += (atoi(date_str) - 1) * 32;

            date_str[0] = source[6];
            date_str[1] = source[7];
            group_val += atoi(date_str);

            bin_append(group_val, 16, binary_string);

            if (source[1] == '1') {
                /* Production Date AI 11 */
                strcat(binary_string, "0");
            } else {
                /* Expiration Date AI 17 */
                strcat(binary_string, "1");
            }
            read_posn = 8;

            if ((source[read_posn] == '1') && (source[read_posn + 1] == '0')) {
                /* Followed by AI 10 - strip this from general field */
                read_posn += 2;
            } else if (source[read_posn]) {
                /* ISO/IEC 24723:2010 5.3.1 "If a lot number does not directly follow the date element string, a FNC1 is encoded following the date element string ..." */
                fnc1_latch = 1;
            } else {
                /* "... even if no more data follows the date element string" */
                /* So still need FNC1 character but can't do single FNC1 in numeric mode, so insert alphanumeric latch "0000" and alphanumeric FNC1 "01111"
                   (this implementation detail taken from BWIPP https://github.com/bwipp/postscriptbarcode Copyright (c) 2004-2019 Terry Burton) */
                strcat(binary_string, "000001111");
                /* Note an alphanumeric FNC1 is also a numeric latch, so now in numeric mode */
            }
        }
    }

    if (encoding_method == 3) {
        /* Encodation Method field of "11" - AI 90 */
#ifndef _MSC_VER
        char ninety[source_len + 1];
#else
        char* ninety = (char*) _alloca(source_len + 1);
#endif
        int ninety_len, alpha, alphanum, numeric, test1, test2, test3;

        /* "This encodation method may be used if an element string with an AI
        90 occurs at the start of the data message, and if the data field
        following the two-digit AI 90 starts with an alphanumeric string which
        complies with a specific format." (para 5.3.2) */

        i = 0;
        do {
            ninety[i] = source[i + 2];
            i++;
        } while ((source_len > i + 2) && ('[' != source[i + 2]));
        ninety[i] = '\0';
        ninety_len = strlen(ninety);

        /* Find out if the AI 90 data is alphabetic or numeric or both */

        alpha = 0;
        alphanum = 0;
        numeric = 0;

        for (i = 0; i < ninety_len; i++) {

            if ((ninety[i] >= 'A') && (ninety[i] <= 'Z')) {
                /* Character is alphabetic */
                alpha += 1;
            } else if ((ninety[i] >= '0') && (ninety[i] <= '9')) {
                /* Character is numeric */
                numeric += 1;
            } else {
                alphanum += 1;
            }
        }

        /* must start with 0, 1, 2 or 3 digits followed by an uppercase character */
        test1 = -1;
        for (i = 3; i >= 0; i--) {
            if ((ninety[i] >= 'A') && (ninety[i] <= 'Z')) {
                test1 = i;
            }
        }

        test2 = 0;
        for (i = 0; i < test1; i++) {
            if (!((ninety[i] >= '0') && (ninety[i] <= '9'))) {
                test2 = 1;
            }
        }

        /* leading zeros are not permitted */
        test3 = 0;
        if ((test1 >= 1) && (ninety[0] == '0')) {
            test3 = 1;
        }

        if ((test1 != -1) && (test2 != 1) && (test3 == 0)) {
            int next_ai_posn;
            char numeric_part[4];
            int numeric_value;
            int table3_letter;
            /* Encodation method "11" can be used */
            strcat(binary_string, "11");

            numeric -= test1;
            alpha--;

            /* Decide on numeric, alpha or alphanumeric mode */
            /* Alpha mode is a special mode for AI 90 */

            if (alphanum == 0 && alpha > numeric) {
                /* Alphabetic mode */
                strcat(binary_string, "11");
                ai90_mode = 2;
            } else if (alphanum == 0 && alpha == 0) {
                /* Numeric mode */
                strcat(binary_string, "10");
                ai90_mode = 3;
            } else {
                /* Alphanumeric mode */
                strcat(binary_string, "0");
                ai90_mode = 1;
                mode = ALPHA;
            }

            next_ai_posn = 2 + ninety_len;

            if (source[next_ai_posn] == '[') {
                /* There are more AIs afterwards */
                if ((source[next_ai_posn + 1] == '2') && (source[next_ai_posn + 2] == '1')) {
                    /* AI 21 follows */
                    ai_crop = 1;
                }

                if ((source[next_ai_posn + 1] == '8') && (source[next_ai_posn + 2] == '0') && (source[next_ai_posn + 3] == '0') && (source[next_ai_posn + 4] == '4')) {
                    /* AI 8004 follows */
                    ai_crop = 3;
                }
            }

            switch (ai_crop) {
                case 0: strcat(binary_string, "0");
                    break;
                case 1: strcat(binary_string, "10");
                    ai_crop_posn = next_ai_posn + 1;
                    break;
                case 3: strcat(binary_string, "11");
                    ai_crop_posn = next_ai_posn + 1;
                    break;
            }

            if (test1 == 0) {
                strcpy(numeric_part, "0");
            } else {
                for (i = 0; i < test1; i++) {
                    numeric_part[i] = ninety[i];
                }
                numeric_part[i] = '\0';
            }

            numeric_value = atoi(numeric_part);

            table3_letter = -1;
            if (numeric_value < 31) {
                table3_letter = posn("BDHIJKLNPQRSTVWZ", ninety[test1]);
            }

            if (table3_letter != -1) {
                /* Encoding can be done according to 5.3.2 c) 2) */
                /* five bit binary string representing value before letter */
                bin_append(numeric_value, 5, binary_string);

                /* followed by four bit representation of letter from Table 3 */
                bin_append(table3_letter, 4, binary_string);
            } else {
                /* Encoding is done according to 5.3.2 c) 3) */
                bin_append(31, 5, binary_string);
                /* ten bit representation of number */
                bin_append(numeric_value, 10, binary_string);

                /* five bit representation of ASCII character */
                bin_append(ninety[test1] - 65, 5, binary_string);
            }

            read_posn = test1 + 3;

            /* Do Alpha mode encoding of the rest of the AI 90 data field here */
            if (ai90_mode == 2) {
                /* Alpha encodation (section 5.3.3) */
                do {
                    if ((source[read_posn] >= 'A') && (source[read_posn] <= 'Z')) {
                        bin_append(source[read_posn] - 65, 5, binary_string);

                    } else if ((source[read_posn] >= '0') && (source[read_posn] <= '9')) {
                        bin_append(source[read_posn] + 4, 6, binary_string);

                    } else if (source[read_posn] == '[') {
                        bin_append(31, 5, binary_string);
                    }

                    read_posn++;
                } while ((source[read_posn - 1] != '[') && (source[read_posn - 1] != '\0'));
                alpha_pad = 1; /* This is overwritten if a general field is encoded */
            }

        } else {
            /* Use general field encodation instead */
            strcat(binary_string, "0");
            read_posn = 0;
        }
    }

    /* The compressed data field has been processed if appropriate - the
    rest of the data (if any) goes into a general-purpose data compaction field */

    j = 0;
    if (fnc1_latch == 1) {
        /* Encodation method "10" has been used but it is not followed by
           AI 10, so a FNC1 character needs to be added */
        general_field[j] = '[';
        j++;
    }

    for (i = read_posn; i < source_len; i++) {
        /* Skip "[21" or "[8004" AIs if encodation method "11" used */
        if (i == ai_crop_posn) {
            i += ai_crop;
        } else {
            general_field[j] = source[i];
            j++;
        }
    }
    general_field[j] = '\0';

    if (strlen(general_field) != 0) {
        alpha_pad = 0;
    }

    if (!general_field_encode(general_field, &mode, &last_digit, binary_string)) {
        /* Invalid characters in input data */
        strcpy(symbol->errtxt, "441: Invalid characters in input data");
        return ZINT_ERROR_INVALID_DATA;
    }

    binary_length = (int)strlen(binary_string);
    switch (cc_mode) {
        case 1:
            target_bitsize = calc_padding_cca(binary_length, *(cc_width));
            break;
        case 2:
            target_bitsize = calc_padding_ccb(binary_length, *(cc_width));
            break;
        case 3:
            target_bitsize = calc_padding_ccc(binary_length, cc_width, lin_width, ecc);
            break;
    }

    if (target_bitsize == 0) {
        strcpy(symbol->errtxt, "442: Input too long for selected 2d component");
        return ZINT_ERROR_TOO_LONG;
    }

    remainder = target_bitsize - binary_length;

    if (last_digit) {
        /* There is still one more numeric digit to encode */

        if ((remainder >= 4) && (remainder <= 6)) {
            /* ISO/IEC 24723:2010 5.4.1 c) 2) "If four to six bits remain, add 1 to the digit value and encode the result in the next four bits. ..." */
            bin_append(ctoi(last_digit) + 1, 4, binary_string);
            if (remainder > 4) {
                /* "... The fifth and sixth bits, if present, shall be “0”s." (Covered by adding truncated alphanumeric latch below but do explicitly anyway) */
                bin_append(0, remainder - 4, binary_string);
            }
        } else {
            bin_append((11 * ctoi(last_digit)) + 18, 7, binary_string);
            /* This may push the symbol up to the next size */
        }
    }

    if (strlen(binary_string) > 11805) { /* (2361 * 5) */
        strcpy(symbol->errtxt, "443: Input too long");
        return ZINT_ERROR_TOO_LONG;
    }

    binary_length = (int)strlen(binary_string);
    switch (cc_mode) {
        case 1:
            target_bitsize = calc_padding_cca(binary_length, *(cc_width));
            break;
        case 2:
            target_bitsize = calc_padding_ccb(binary_length, *(cc_width));
            break;
        case 3:
            target_bitsize = calc_padding_ccc(binary_length, cc_width, lin_width, ecc);
            break;
    }

    if (target_bitsize == 0) {
        strcpy(symbol->errtxt, "444: Input too long for selected 2d component");
        return ZINT_ERROR_TOO_LONG;
    }

    if (binary_length < target_bitsize) {
        /* Now add padding to binary string */
        if (alpha_pad == 1) {
            strcat(binary_string, "11111");
            alpha_pad = 0;
            /* Extra FNC1 character required after Alpha encodation (section 5.3.3) */
        }

        if (mode == NUMERIC) {
            strcat(binary_string, "0000");
        }

        while (strlen(binary_string) < (unsigned int) target_bitsize) {
            strcat(binary_string, "00100");
        }

        if (strlen(binary_string) > (unsigned int) target_bitsize) {
            binary_string[target_bitsize] = '\0';
        }
    }

    return 0;
}

int linear_dummy_run(unsigned char *source, int length) {
    struct zint_symbol *dummy;
    int error_number;
    int linear_width;

    dummy = ZBarcode_Create();
    dummy->symbology = BARCODE_EAN128_CC;
    dummy->option_1 = 3;
    error_number = ean_128(dummy, source, length);
    linear_width = dummy->width;
    ZBarcode_Delete(dummy);

    if (error_number == 0) {
        return linear_width;
    } else {
        return 0;
    }
}

int composite(struct zint_symbol *symbol, unsigned char source[], int length) {
    int error_number, cc_mode, cc_width, ecc_level;
    int j, i, k;
    unsigned int bs = 13 * length + 500 + 1; /* Allow for 8 bits + 5-bit latch per char + 500 bits overhead/padding */
#ifndef _MSC_VER
    char binary_string[bs];
#else
    char* binary_string = (char*) _alloca(bs);
#endif
    unsigned int pri_len;
    struct zint_symbol *linear;
    int top_shift, bottom_shift;
    int linear_width = 0;

    /* Perform sanity checks on input options first */
    error_number = 0;
    pri_len = (int)strlen(symbol->primary);
    if (pri_len == 0) {
        strcpy(symbol->errtxt, "445: No primary (linear) message in 2D composite");
        return ZINT_ERROR_INVALID_OPTION;
    }

    if (length > 2990) {
        strcpy(symbol->errtxt, "446: 2D component input data too long");
        return ZINT_ERROR_TOO_LONG;
    }

    cc_mode = symbol->option_1;
    if ((cc_mode == 3) && (symbol->symbology != BARCODE_EAN128_CC)) {
        /* CC-C can only be used with a GS1-128 linear part */
        strcpy(symbol->errtxt, "447: Invalid mode (CC-C only valid with GS1-128 linear component)");
        return ZINT_ERROR_INVALID_OPTION;
    }

    if (symbol->symbology == BARCODE_EAN128_CC) {
        /* Do a test run of encoding the linear component to establish its width */
        linear_width = linear_dummy_run((unsigned char *) symbol->primary, pri_len);
        if (linear_width == 0) {
            strcpy(symbol->errtxt, "448: Invalid data");
            return ZINT_ERROR_INVALID_DATA;
        }
    }

    switch (symbol->symbology) {
            /* Determine width of 2D component according to ISO/IEC 24723 Table 1 */
        case BARCODE_EANX_CC:
            cc_width = 0;
            if (pri_len < 20) {
                int padded_pri_len;
                char padded_pri[20];
                padded_pri[0] = '\0';
                ean_leading_zeroes(symbol, (unsigned char*) symbol->primary, (unsigned char*) padded_pri);
                padded_pri_len = strlen(padded_pri);
                if (padded_pri_len <= 7) { /* EAN-8 */
                    cc_width = 3;
                } else {
                    switch (padded_pri_len) {
                        case 10: /* EAN-8 + 2 */
                        case 13: /* EAN-8 + 5 */
                            cc_width = 3;
                            break;
                        case 12: /* EAN-13 */
                        case 15: /* EAN-13 + 2 */
                        case 18: /* EAN-13 + 5 */
                            cc_width = 4;
                            break;
                    }
                }
            }
            if (cc_width == 0) {
                strcpy(symbol->errtxt, "449: Invalid length EAN input in linear component");
                return ZINT_ERROR_TOO_LONG;
            }
            break;
        case BARCODE_EAN128_CC: cc_width = 4;
            break;
        case BARCODE_RSS14_CC: cc_width = 4;
            break;
        case BARCODE_RSS_LTD_CC: cc_width = 3;
            break;
        case BARCODE_RSS_EXP_CC: cc_width = 4;
            break;
        case BARCODE_UPCA_CC: cc_width = 4;
            break;
        case BARCODE_UPCE_CC: cc_width = 2;
            break;
        case BARCODE_RSS14STACK_CC: cc_width = 2;
            break;
        case BARCODE_RSS14_OMNI_CC: cc_width = 2;
            break;
        case BARCODE_RSS_EXPSTACK_CC: cc_width = 4;
            break;
    }

    memset(binary_string, 0, bs);

    if (cc_mode < 1 || cc_mode > 3) {
        cc_mode = 1;
    }

    if (cc_mode == 1) {
        i = cc_binary_string(symbol, (char *) source, binary_string, cc_mode, &cc_width, &ecc_level, linear_width);
        if (i == ZINT_ERROR_TOO_LONG) {
            cc_mode = 2;
            memset(binary_string, 0, bs);
        } else if (i != 0) {
            return i;
        }
    }

    if (cc_mode == 2) {
        /* If the data didn't fit into CC-A it is recalculated for CC-B */
        i = cc_binary_string(symbol, (char *) source, binary_string, cc_mode, &cc_width, &ecc_level, linear_width);
        if (i == ZINT_ERROR_TOO_LONG) {
            if (symbol->symbology != BARCODE_EAN128_CC) {
                return ZINT_ERROR_TOO_LONG;
            }
            cc_mode = 3;
            memset(binary_string, 0, bs);
        } else if (i != 0) {
            return i;
        }
    }

    if (cc_mode == 3) {
        /* If the data didn't fit in CC-B (and linear part is GS1-128) it is recalculated for CC-C */
        i = cc_binary_string(symbol, (char *) source, binary_string, cc_mode, &cc_width, &ecc_level, linear_width);
        if (i != 0) {
            return i;
        }
    }

    switch (cc_mode) {
            /* Note that ecc_level is only relevant to CC-C */
        case 1: error_number = cc_a(symbol, binary_string, cc_width);
            break;
        case 2: error_number = cc_b(symbol, binary_string, cc_width);
            break;
        case 3: error_number = cc_c(symbol, binary_string, cc_width, ecc_level);
            break;
    }

    if (error_number != 0) {
        return ZINT_ERROR_ENCODING_PROBLEM;
    }

    /* 2D component done, now calculate linear component */
    linear = ZBarcode_Create(); /* Symbol contains the 2D component and Linear contains the rest */

    linear->symbology = symbol->symbology;

    if (linear->symbology != BARCODE_EAN128_CC) {
        /* Set the "component linkage" flag in the linear component */
        linear->option_1 = 2;
    } else {
        /* GS1-128 needs to know which type of 2D component is used */
        linear->option_1 = cc_mode;
    }

    switch (symbol->symbology) {
        case BARCODE_EANX_CC: error_number = eanx(linear, (unsigned char *) symbol->primary, pri_len);
            break;
        case BARCODE_EAN128_CC: error_number = ean_128(linear, (unsigned char *) symbol->primary, pri_len);
            break;
        case BARCODE_RSS14_CC: error_number = rss14(linear, (unsigned char *) symbol->primary, pri_len);
            break;
        case BARCODE_RSS_LTD_CC: error_number = rsslimited(linear, (unsigned char *) symbol->primary, pri_len);
            break;
        case BARCODE_RSS_EXP_CC: error_number = rssexpanded(linear, (unsigned char *) symbol->primary, pri_len);
            break;
        case BARCODE_UPCA_CC: error_number = eanx(linear, (unsigned char *) symbol->primary, pri_len);
            break;
        case BARCODE_UPCE_CC: error_number = eanx(linear, (unsigned char *) symbol->primary, pri_len);
            break;
        case BARCODE_RSS14STACK_CC: error_number = rss14(linear, (unsigned char *) symbol->primary, pri_len);
            break;
        case BARCODE_RSS14_OMNI_CC: error_number = rss14(linear, (unsigned char *) symbol->primary, pri_len);
            break;
        case BARCODE_RSS_EXPSTACK_CC: error_number = rssexpanded(linear, (unsigned char *) symbol->primary, pri_len);
            break;
    }

    if (error_number != 0) {
        strcpy(symbol->errtxt, linear->errtxt);
        strcat(symbol->errtxt, " in linear component ");
        ZBarcode_Delete(linear);
        return error_number;
    }

    /* Merge the linear component with the 2D component */

    top_shift = 0;
    bottom_shift = 0;

    switch (symbol->symbology) {
            /* Determine horizontal alignment (according to section 12.3) */
        case BARCODE_EANX_CC:
            switch (ustrlen(linear->text)) { /* Use zero-padded length */
                case 8: /* EAN-8 */
                case 11: /* EAN-8 + 2 */
                case 14: /* EAN-8 + 5 */
                    if (cc_mode == 1) {
                        bottom_shift = 3;
                    } else {
                        bottom_shift = 13;
                    }
                    break;
                case 13: /* EAN-13 */
                case 16: /* EAN-13 + 2 */
                case 19: /* EAN-13 + 5 */
                    bottom_shift = 2;
                    break;
            }
            break;
        case BARCODE_EAN128_CC: if (cc_mode == 3) {
                bottom_shift = 7;
            } else {
                /* ISO/IEC 24723:2010 12.3 g) "GS1-128 components linked to the right quiet zone of the CC-A or CC-B: the CC-A or CC-B component is
                   aligned with the last space module of one of the rightmost symbol characters of the linear component. To
                   calculate the target Code 128 symbol character position for alignment, number the positions from right to
                   left (0 is the Stop character, 1 is the Check character, etc.), and then Position = (total number of Code 128 symbol characters – 9) div 2"
                 */
                int num_symbols = (linear_width - 2) / 11;
                int position = (num_symbols - 9) / 2;
                int calc_shift = linear->width - position * 11 - 1 - symbol->width; /* Less 1 to align with last space module */
                if (position) {
                    calc_shift -= 2; /* Less additional stop modules */
                }
                if (calc_shift > 0) {
                    top_shift = calc_shift;
                } else if (calc_shift < 0) {
                    bottom_shift = -calc_shift;
                }
            }
            break;
        case BARCODE_RSS14_CC: bottom_shift = 4;
            break;
        case BARCODE_RSS_LTD_CC: 
            if (cc_mode == 1) {
                top_shift = 1;
            } else {
                bottom_shift = 9;
            }
            break;
        case BARCODE_RSS_EXP_CC: k = 1;
            while ((!(module_is_set(linear, 1, k - 1))) && module_is_set(linear, 1, k)) {
                k++;
            }
            top_shift = k;
            break;
        case BARCODE_UPCA_CC: bottom_shift = 2;
            break;
        case BARCODE_UPCE_CC: bottom_shift = 2;
            break;
        case BARCODE_RSS14STACK_CC: top_shift = 1;
            break;
        case BARCODE_RSS14_OMNI_CC: top_shift = 1;
            break;
        case BARCODE_RSS_EXPSTACK_CC: k = 1;
            while ((!(module_is_set(linear, 1, k - 1))) && module_is_set(linear, 1, k)) {
                k++;
            }
            top_shift = k;
            break;
    }

    if (top_shift != 0) {
        /* Move the 2d component of the symbol horizontally */
        for (i = 0; i <= symbol->rows; i++) {
            for (j = (symbol->width + top_shift); j >= top_shift; j--) {
                if (module_is_set(symbol, i, j - top_shift)) {
                    set_module(symbol, i, j);
                } else {
                    unset_module(symbol, i, j);
                }
            }
            for (j = 0; j < top_shift; j++) {
                unset_module(symbol, i, j);
            }
        }
    }

    /* Merge linear and 2D components into one structure */
    for (i = 0; i <= linear->rows; i++) {
        symbol->row_height[symbol->rows + i] = linear->row_height[i];
        for (j = 0; j <= linear->width; j++) {
            if (module_is_set(linear, i, j)) {
                set_module(symbol, i + symbol->rows, j + bottom_shift);
            } else {
                unset_module(symbol, i + symbol->rows, j + bottom_shift);
            }
        }
    }
    if ((linear->width + bottom_shift) > symbol->width + top_shift) {
        symbol->width = linear->width + bottom_shift;
    } else if ((symbol->width + top_shift) > linear->width + bottom_shift) {
        symbol->width += top_shift;
    }
    symbol->rows += linear->rows;
    ustrcpy(symbol->text, (unsigned char *) linear->text);

    ZBarcode_Delete(linear);

    return error_number;
}
