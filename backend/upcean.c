/*  upcean.c - Handles UPC, EAN and ISBN

    libzint - the open source barcode library
    Copyright (C) 2008 - 2021 Robin Stuart <rstuart114@gmail.com>

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

#define SODIUM          "0123456789+"
#define ISBN_SANE       "0123456789X"
#define ISBN_ADDON_SANE "0123456789Xx+"

#define EAN2    102
#define EAN5    105

#include <stdio.h>
#include "common.h"
#include "gs1.h"

/* UPC and EAN tables checked against EN 797:1996 */

static const char *UPCParity0[10] = {
    /* Number set for UPC-E symbol (EN Table 4) */
    "BBBAAA", "BBABAA", "BBAABA", "BBAAAB", "BABBAA", "BAABBA", "BAAABB",
    "BABABA", "BABAAB", "BAABAB"
};

static const char *UPCParity1[10] = {
    /* Not covered by BS EN 797:1995 */
    "AAABBB", "AABABB", "AABBAB", "AABBBA", "ABAABB", "ABBAAB", "ABBBAA",
    "ABABAB", "ABABBA", "ABBABA"
};

static const char *EAN2Parity[4] = {
    /* Number sets for 2-digit add-on (EN Table 6) */
    "AA", "AB", "BA", "BB"
};

static const char *EAN5Parity[10] = {
    /* Number set for 5-digit add-on (EN Table 7) */
    "BBAAA", "BABAA", "BAABA", "BAAAB", "ABBAA", "AABBA", "AAABB", "ABABA",
    "ABAAB", "AABAB"
};

static const char *EAN13Parity[10] = {
    /* Left hand of the EAN-13 symbol (EN Table 3) */
    "AAAAA", "ABABB", "ABBAB", "ABBBA", "BAABB", "BBAAB", "BBBAA", "BABAB",
    "BABBA", "BBABA"
};

static const char *EANsetA[10] = {
    /* Representation set A and C (EN Table 1) */
    "3211", "2221", "2122", "1411", "1132", "1231", "1114", "1312", "1213", "3112"
};

static const char *EANsetB[10] = {
    /* Representation set B (EN Table 1) */
    "1123", "1222", "2212", "1141", "2311", "1321", "4111", "2131", "3121", "2113"
};

/* UPC A is usually used for 12 digit numbers, but this function takes a source of any length */
static void upca_draw(const unsigned char source[], const int length, char dest[]) {
    int i, half_way;

    half_way = length / 2;

    /* start character */
    strcat(dest, "111");

    for (i = 0; i < length; i++) {
        if (i == half_way) {
            /* middle character - separates manufacturer no. from product no. */
            /* also inverts right hand characters */
            strcat(dest, "11111");
        }

        lookup(NEON, EANsetA, source[i], dest);
    }

    /* stop character */
    strcat(dest, "111");
}

/* Make a UPC-A barcode, allowing for composite if `cc_rows` set */
static int upca_cc(struct zint_symbol *symbol, const unsigned char source[], int length, char dest[], int cc_rows) {
    unsigned char *gtin = symbol->text;
    float height;
    int error_number = 0;

    ustrcpy(gtin, source);

    if (length == 11) {
        gtin[length++] = gs1_check_digit(gtin, 11);
        gtin[length] = '\0';
    } else {
        if (source[length - 1] != gs1_check_digit(gtin, 11)) {
            sprintf(symbol->errtxt, "270: Invalid check digit '%c', expecting '%c'",
                    source[length - 1], gs1_check_digit(gtin, 11));
            return ZINT_ERROR_INVALID_CHECK;
        }
    }
    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("UPC-A: %s, gtin: %s, Check digit: %c\n", source, gtin,
            length == 11 ? gtin[length] : gtin[length - 1]);
    }

    upca_draw(gtin, length, dest);

#ifdef COMPLIANT_HEIGHTS
    /* BS EN 797:1996 4.5.1 Nominal dimensions 22.85mm / 0.33mm (X) ~ 69.24,
       same as minimum GS1 General Specifications 21.0.1 5.12.3.1 */
    height = (float) (22.85 / 0.33);
    if (symbol->symbology == BARCODE_UPCA_CC) {
        symbol->height = height; /* Pass back min row == default height */
    } else {
        error_number = set_height(symbol, height, height, 0.0f, 0 /*no_errtxt*/);
    }
#else
    height = 50.0f;
    if (symbol->symbology == BARCODE_UPCA_CC) {
        symbol->height = height - cc_rows * 2 - 6.0f;
    } else {
        (void) set_height(symbol, 0.0f, height, 0.0f, 1 /*no_errtxt*/);
    }
#endif

    return error_number;
}

/* UPC-A */
static int upca(struct zint_symbol *symbol, const unsigned char source[], int length, char dest[]) {
    return upca_cc(symbol, source, length, dest, 0 /*cc_rows*/);
}

/* UPC-E, allowing for composite if `cc_rows` set */
static int upce_cc(struct zint_symbol *symbol, unsigned char source[], int length, char dest[], int cc_rows) {
    int i, num_system;
    char emode, check_digit, parity[8];
    char src_check_digit = '\0';
    unsigned char equivalent[12];
    unsigned char *hrt = symbol->text;
    float height;
    int error_number = 0;

    if (length == 8 || symbol->symbology == BARCODE_UPCE_CHK) {
        /* Will validate later */
        src_check_digit = source[--length];
    }

    /* Two number systems can be used - system 0 and system 1 */
    hrt[0] = '\0';
    if (length == 7) {
        switch (source[0]) {
            case '0': num_system = 0;
                ustrncat(hrt, source, length);
                break;
            case '1': num_system = 1;
                ustrncat(hrt, source, length);
                break;
            default: num_system = 0;
                /* First source char ignored */
                ustrncat(hrt, source, length);
                hrt[0] = '0'; /* Overwrite HRT first char with '0' to correct TODO: error/warn in future */
                break;
        }
        for (i = 1; i <= length; i++) {
            source[i - 1] = hrt[i];
        }
        length--;
    } else {
        /* Length 6, insert leading zero */
        num_system = 0;
        hrt[0] = '0';
        hrt[1] = '\0';
        ustrncat(hrt, source, length);
    }

    /* Expand the zero-compressed UPCE code to make a UPCA equivalent (EN Table 5) */
    emode = source[5];
    for (i = 0; i < 11; i++) {
        equivalent[i] = '0';
    }
    if (num_system == 1) {
        equivalent[0] = hrt[0];
    }
    equivalent[1] = source[0];
    equivalent[2] = source[1];
    equivalent[11] = '\0';

    switch (emode) {
        case '0':
        case '1':
        case '2':
            equivalent[3] = emode;
            equivalent[8] = source[2];
            equivalent[9] = source[3];
            equivalent[10] = source[4];
            break;
        case '3':
            equivalent[3] = source[2];
            equivalent[9] = source[3];
            equivalent[10] = source[4];
            if (((source[2] == '0') || (source[2] == '1')) || (source[2] == '2')) {
                /* Note 1 - "X3 shall not be equal to 0, 1 or 2" */
                strcpy(symbol->errtxt, "271: Invalid UPC-E data"); // TODO: Better error message
                return ZINT_ERROR_INVALID_DATA;
            }
            break;
        case '4':
            equivalent[3] = source[2];
            equivalent[4] = source[3];
            equivalent[10] = source[4];
            if (source[3] == '0') {
                /* Note 2 - "X4 shall not be equal to 0" */
                strcpy(symbol->errtxt, "272: Invalid UPC-E data"); // TODO: Better error message
                return ZINT_ERROR_INVALID_DATA;
            }
            break;
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            equivalent[3] = source[2];
            equivalent[4] = source[3];
            equivalent[5] = source[4];
            equivalent[10] = emode;
            if (source[4] == '0') {
                /* Note 3 - "X5 shall not be equal to 0" */
                strcpy(symbol->errtxt, "273: Invalid UPC-E data"); // TODO: Better error message
                return ZINT_ERROR_INVALID_DATA;
            }
            break;
    }

    /* Get the check digit from the expanded UPCA code */

    check_digit = gs1_check_digit(equivalent, 11);

    if (src_check_digit && src_check_digit != check_digit) {
        sprintf(symbol->errtxt, "274: Invalid check digit '%c', expecting '%c'", src_check_digit, check_digit);
        return ZINT_ERROR_INVALID_CHECK;
    }

    /* Use the number system and check digit information to choose a parity scheme */
    if (num_system == 1) {
        strcpy(parity, UPCParity1[ctoi(check_digit)]);
    } else {
        strcpy(parity, UPCParity0[ctoi(check_digit)]);
    }

    /* Take all this information and make the barcode pattern */

    /* start character */
    strcat(dest, "111");

    for (i = 0; i < length; i++) {
        switch (parity[i]) {
            case 'A': lookup(NEON, EANsetA, source[i], dest);
                break;
            case 'B': lookup(NEON, EANsetB, source[i], dest);
                break;
        }
    }

    /* stop character */
    strcat(dest, "111111");

    hrt[7] = check_digit;
    hrt[8] = '\0';

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("UPC-E: %s, equivalent: %s, hrt: %s, Check digit: %c\n", source, equivalent, hrt, check_digit);
    }

#ifdef COMPLIANT_HEIGHTS
    /* BS EN 797:1996 4.5.1 Nominal dimensions 22.85mm / 0.33mm (X) ~ 69.24,
       same as minimum GS1 General Specifications 21.0.1 5.12.3.1 */
    height = (float) (22.85 / 0.33);
    if (symbol->symbology == BARCODE_UPCE_CC) {
        symbol->height = height; /* Pass back min row == default height */
    } else {
        error_number = set_height(symbol, height, height, 0.0f, 0 /*no_errtxt*/);
    }
#else
    height = 50.0f;
    if (symbol->symbology == BARCODE_UPCE_CC) {
        symbol->height = height - cc_rows * 2 - 6.0f;
    } else {
        (void) set_height(symbol, 0.0f, height, 0.0f, 1 /*no_errtxt*/);
    }
#endif

    return error_number;
}

/* UPC-E is a zero-compressed version of UPC-A */
static int upce(struct zint_symbol *symbol, unsigned char source[], int length, char dest[]) {
    return upce_cc(symbol, source, length, dest, 0 /*cc_rows*/);
}

/* EAN-2 and EAN-5 add-on codes */
static void ean_add_on(const unsigned char source[], const int length, char dest[], const int addon_gap) {
    char parity[6];
    int i, code_type;

    /* If an add-on then append with space */
    if (addon_gap != 0) {
        i = (int) strlen(dest);
        dest[i] = itoc(addon_gap);
        dest[i + 1] = '\0';
    }

    /* Start character */
    strcat(dest, "112");

    /* Determine EAN2 or EAN5 add-on */
    if (length == 2) {
        code_type = EAN2;
    } else {
        code_type = EAN5;
    }

    /* Calculate parity for EAN2 */
    if (code_type == EAN2) {
        int code_value, parity_bit;

        code_value = (10 * ctoi(source[0])) + ctoi(source[1]);
        parity_bit = code_value % 4;
        strcpy(parity, EAN2Parity[parity_bit]);
    }

    if (code_type == EAN5) {
        int values[6], parity_sum, parity_bit;

        for (i = 0; i < 6; i++) {
            values[i] = ctoi(source[i]);
        }

        parity_sum = (3 * (values[0] + values[2] + values[4]));
        parity_sum += (9 * (values[1] + values[3]));

        parity_bit = parity_sum % 10;
        strcpy(parity, EAN5Parity[parity_bit]);
    }

    for (i = 0; i < length; i++) {
        switch (parity[i]) {
            case 'A': lookup(NEON, EANsetA, source[i], dest);
                break;
            case 'B': lookup(NEON, EANsetB, source[i], dest);
                break;
        }

        /* Glyph separator */
        if (i != (length - 1)) {
            strcat(dest, "11");
        }
    }
}

/* ************************ EAN-13 ****************** */

static int ean13_cc(struct zint_symbol *symbol, const unsigned char source[], int length, char dest[],
            int cc_rows) {
    int i, half_way;
    char parity[6];
    unsigned char *gtin = symbol->text;
    float height;
    int error_number = 0;

    parity[0] = '\0';
    ustrcpy(gtin, source);

    /* Add the appropriate check digit */

    if (length == 12) {
        gtin[length++] = gs1_check_digit(gtin, 12);
        gtin[length] = '\0';
    } else {
        if (source[length - 1] != gs1_check_digit(gtin, 12)) {
            sprintf(symbol->errtxt, "275: Invalid check digit '%c', expecting '%c'",
                    source[length - 1], gs1_check_digit(gtin, 12));
            return ZINT_ERROR_INVALID_CHECK;
        }
    }
    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("EAN-13: %s, gtin: %s, Check digit: %c\n", source, gtin, gtin[length - 1]);
    }

    /* Get parity for first half of the symbol */
    lookup(SODIUM, EAN13Parity, gtin[0], parity);

    /* Now get on with the cipher */
    half_way = 7;

    /* start character */
    strcat(dest, "111");
    for (i = 1; i < length; i++) {
        if (i == half_way) {
            /* middle character - separates manufacturer no. from product no. */
            /* also inverses right hand characters */
            strcat(dest, "11111");
        }

        if (((i > 1) && (i < 7)) && (parity[i - 2] == 'B')) {
            lookup(NEON, EANsetB, gtin[i], dest);
        } else {
            lookup(NEON, EANsetA, gtin[i], dest);
        }
    }

    /* stop character */
    strcat(dest, "111");

#ifdef COMPLIANT_HEIGHTS
    /* BS EN 797:1996 4.5.1 Nominal dimensions 22.85mm / 0.33mm (X) ~ 69.24,
       same as minimum GS1 General Specifications 21.0.1 5.12.3.1 */
    height = (float) (22.85 / 0.33);
    if (symbol->symbology == BARCODE_EANX_CC) {
        symbol->height = height; /* Pass back min row == default height */
    } else {
        error_number = set_height(symbol, height, height, 0.0f, 0 /*no_errtxt*/);
    }
#else
    height = 50.0f;
    if (symbol->symbology == BARCODE_EANX_CC) {
        symbol->height = height - cc_rows * 2 - 6.0f;
    } else {
        (void) set_height(symbol, 0.0f, height, 0.0f, 1 /*no_errtxt*/);
    }
#endif

    return error_number;
}

static int ean13(struct zint_symbol *symbol, const unsigned char source[], int length, char dest[]) {
    return ean13_cc(symbol, source, length, dest, 0 /*cc_rows*/);
}

static int ean8_cc(struct zint_symbol *symbol, const unsigned char source[], int length, char dest[], int cc_rows) {
    /* EAN-8 is basically the same as UPC-A but with fewer digits */
    unsigned char *gtin = symbol->text;
    float height;
    int error_number = 0;

    ustrcpy(gtin, source);

    if (length == 7) {
        gtin[length++] = gs1_check_digit(gtin, 7);
        gtin[length] = '\0';
    } else {
        if (source[length - 1] != gs1_check_digit(gtin, 7)) {
            sprintf(symbol->errtxt, "276: Invalid check digit '%c', expecting '%c'",
                    source[length - 1], gs1_check_digit(gtin, 7));
            return ZINT_ERROR_INVALID_CHECK;
        }
    }
    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("EAN-8: %s, gtin: %s, Check digit: %c\n", source, gtin,
            length == 7 ? gtin[length] : gtin[length - 1]);
    }

    upca_draw(gtin, length, dest);

#ifdef COMPLIANT_HEIGHTS
    /* BS EN 797:1996 4.5.1 Nominal dimensions 18.23mm / 0.33mm (X) ~ 55.24,
       same as minimum GS1 General Specifications 21.0.1 5.12.3.1 */
    height = (float) (18.23 / 0.33);
    if (symbol->symbology == BARCODE_EANX_CC) {
        symbol->height = height; /* Pass back min row == default height */
    } else {
        error_number = set_height(symbol, height, height, 0.0f, 0 /*no_errtxt*/);
    }
#else
    height = 50.0f;
    if (symbol->symbology == BARCODE_EANX_CC) {
        symbol->height = height - cc_rows * 2 - 6.0f;
    } else {
        (void) set_height(symbol, 0.0f, height, 0.0f, 1 /*no_errtxt*/);
    }
#endif

    return error_number;
}

/* Make an EAN-8 barcode when we haven't been given the check digit */
static int ean8(struct zint_symbol *symbol, const unsigned char source[], int length, char dest[]) {
    return ean8_cc(symbol, source, length, dest, 0 /*cc_rows*/);
}

/* For ISBN(10) and SBN only */
static char isbn_check(const unsigned char source[], const int length) {
    int i, weight, sum, check;
    char check_char;

    sum = 0;
    weight = 1;

    for (i = 0; i < length; i++) { /* Length will always be 9 */
        sum += ctoi(source[i]) * weight;
        weight++;
    }

    check = sum % 11;
    check_char = itoc(check);
    if (check == 10) {
        check_char = 'X';
    }
    return check_char;
}

/* Make an EAN-13 barcode from an SBN or ISBN */
static int isbn(struct zint_symbol *symbol, unsigned char source[], const int src_len, char dest[]) {
    int i, error_number;
    char check_digit;

    to_upper(source);
    error_number = is_sane(ISBN_SANE, source, src_len);
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "277: Invalid character in data (digits and \"X\" only)");
        return error_number;
    }

    /* Input must be 9, 10 or 13 characters */
    if (src_len != 9 && src_len != 10 && src_len != 13) {
        strcpy(symbol->errtxt, "278: Input wrong length (9, 10, or 13 characters only)");
        return ZINT_ERROR_TOO_LONG;
    }

    if (src_len == 13) /* Using 13 character ISBN */ {
        if (!(((source[0] == '9') && (source[1] == '7')) &&
                ((source[2] == '8') || (source[2] == '9')))) {
            strcpy(symbol->errtxt, "279: Invalid ISBN (must begin with \"978\" or \"979\")");
            return ZINT_ERROR_INVALID_DATA;
        }

        /* "X" can only occur in last position */
        error_number = is_sane(NEON, source, 12);
        if (error_number == ZINT_ERROR_INVALID_DATA) {
            strcpy(symbol->errtxt, "277: Invalid character in data, \"X\" allowed in last position only");
            return error_number;
        }

        check_digit = gs1_check_digit(source, 12);
        if (source[src_len - 1] != check_digit) {
            sprintf(symbol->errtxt, "280: Invalid ISBN check digit '%c', expecting '%c'",
                    source[src_len - 1], check_digit);
            return ZINT_ERROR_INVALID_CHECK;
        }
        source[12] = '\0';

    } else { /* Using 10 digit ISBN or 9 digit SBN padded with leading zero */
        if (src_len == 9) /* Using 9 digit SBN */ {
            /* Add leading zero */
            for (i = 10; i > 0; i--) {
                source[i] = source[i - 1];
            }
            source[0] = '0';
        }

        /* "X" can only occur in last position */
        error_number = is_sane(NEON, source, 9);
        if (error_number == ZINT_ERROR_INVALID_DATA) {
            strcpy(symbol->errtxt, "277: Invalid character in data, \"X\" allowed in last position only");
            return error_number;
        }

        check_digit = isbn_check(source, 9);
        if (check_digit != source[9]) {
            sprintf(symbol->errtxt, "281: Invalid %s check digit '%c', expecting '%c'", src_len == 9 ? "SBN" : "ISBN",
                    source[9], check_digit);
            return ZINT_ERROR_INVALID_CHECK;
        }
        for (i = 11; i > 2; i--) {
            source[i] = source[i - 3];
        }
        source[0] = '9';
        source[1] = '7';
        source[2] = '8';
        source[12] = '\0';
    }

    return ean13(symbol, source, 12, dest);
}

/* Add leading zeroes to EAN and UPC strings */
INTERNAL int ean_leading_zeroes(struct zint_symbol *symbol, const unsigned char source[],
                unsigned char local_source[], int *p_with_addon) {
    unsigned char first_part[14], second_part[6], zfirst_part[14], zsecond_part[6];
    int with_addon = 0;
    int first_len = 0, second_len = 0, zfirst_len = 0, zsecond_len = 0, i, h;

    h = (int) ustrlen(source);
    for (i = 0; i < h; i++) {
        if (source[i] == '+') {
            with_addon = 1;
        } else {
            if (with_addon == 0) {
                first_len++;
            } else {
                second_len++;
            }
        }
    }
    if (first_len > 13 || second_len > 5) {
        if (p_with_addon) {
            *p_with_addon = second_len > 5 ? with_addon : 0;
        }
        return 0;
    }

    /* Split input into two strings */
    for (i = 0; i < first_len; i++) {
        first_part[i] = source[i];
    }
    first_part[first_len] = '\0';

    for (i = 0; i < second_len; i++) {
        second_part[i] = source[i + first_len + 1];
    }
    second_part[second_len] = '\0';

    /* Calculate target lengths */
    if (second_len == 0) {
        zsecond_len = 0;
    } else {
        if (second_len <= 5) {
            if (second_len <= 2) {
                zsecond_len = 2;
            } else {
                zsecond_len = 5;
            }
        }
    }
    switch (symbol->symbology) {
        case BARCODE_EANX:
        case BARCODE_EANX_CC:
            if (first_len <= 12) {
                if (first_len <= 7) {
                    zfirst_len = 7;
                } else {
                    zfirst_len = 12;
                }
            }
            if (second_len == 0 && symbol->symbology == BARCODE_EANX) { /* No composite EAN-2/5 */
                if (first_len <= 5) {
                    if (first_len <= 2) {
                        zfirst_len = 2;
                    } else {
                        zfirst_len = 5;
                    }
                }
            }
            break;
        case BARCODE_EANX_CHK:
            if (first_len <= 13) {
                if (first_len <= 8) {
                    zfirst_len = 8;
                } else {
                    zfirst_len = 13;
                }
            }
            if (second_len == 0) {
                if (first_len <= 5) {
                    if (first_len <= 2) {
                        zfirst_len = 2;
                    } else {
                        zfirst_len = 5;
                    }
                }
            }
            break;
        case BARCODE_UPCA:
        case BARCODE_UPCA_CC:
            zfirst_len = 11;
            break;
        case BARCODE_UPCA_CHK:
            zfirst_len = 12;
            break;
        case BARCODE_UPCE:
        case BARCODE_UPCE_CC:
            if (first_len == 7) {
                zfirst_len = 7;
            } else if (first_len <= 6) {
                zfirst_len = 6;
            }
            break;
        case BARCODE_UPCE_CHK:
            if (first_len == 8) {
                zfirst_len = 8;
            } else if (first_len <= 7) {
                zfirst_len = 7;
            }
            break;
        case BARCODE_ISBNX:
            if (first_len <= 9) {
                zfirst_len = 9;
            }
            break;
    }


    /* Add leading zeroes */
    zfirst_part[0] = '\0';
    for (i = 0; i < (zfirst_len - first_len); i++) {
        ustrcat(zfirst_part, "0");
    }
    ustrcat(zfirst_part, first_part);

    zsecond_part[0] = '\0';
    for (i = 0; i < (zsecond_len - second_len); i++) {
        ustrcat(zsecond_part, "0");
    }
    ustrcat(zsecond_part, second_part);

    /* Copy adjusted data back to local_source */
    ustrcat(local_source, zfirst_part);
    if (*zsecond_part) {
        ustrcat(local_source, "+");
        ustrcat(local_source, zsecond_part);
    }

    if (p_with_addon) {
        *p_with_addon = with_addon;
    }

    return 1; /* Success */
}

INTERNAL int eanx_cc(struct zint_symbol *symbol, unsigned char source[], int src_len, int cc_rows) {
    unsigned char first_part[14] = {0}, second_part[6] = {0};
    unsigned char local_source[20] = {0}; /* Allow 13 + "+" + 5 + 1 */
    char dest[1000] = {0};
    int latch, reader, writer;
    int with_addon;
    int error_number, i, plus_count;
    int addon_gap = 0;
    int first_part_len, second_part_len;
    float height;

    latch = FALSE;
    writer = 0;

    if (src_len > 19) {
        strcpy(symbol->errtxt, "283: Input too long (19 character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }
    if (symbol->symbology != BARCODE_ISBNX) {
        /* ISBN has its own sanity routine */
        error_number = is_sane(SODIUM, source, src_len);
        if (error_number == ZINT_ERROR_INVALID_DATA) {
            strcpy(symbol->errtxt, "284: Invalid character in data (digits and \"+\" only)");
            return error_number;
        }
    } else {
        error_number = is_sane(ISBN_ADDON_SANE, source, src_len);
        if (error_number == ZINT_ERROR_INVALID_DATA) {
            strcpy(symbol->errtxt, "285: Invalid character in data (digits, \"X\" and \"+\" only)");
            return error_number;
        }
        /* Add-on will be checked separately to be numeric only below */
    }

    /* Check for multiple '+' characters */
    plus_count = 0;
    for (i = 0; i < src_len; i++) {
        if (source[i] == '+') {
            plus_count++;
            if (plus_count > 1) {
                strcpy(symbol->errtxt, "293: Invalid add-on data (one \"+\" only)");
                return ZINT_ERROR_INVALID_DATA;
            }
        }
    }

    /* Add leading zeroes, checking max lengths of parts */
    if (!ean_leading_zeroes(symbol, source, local_source, &with_addon)) {
        sprintf(symbol->errtxt, "294: Input too long (%s)",
                with_addon ? "5 character maximum for add-on" : "13 character maximum");
        return ZINT_ERROR_TOO_LONG;
    }

    reader = 0;
    if (with_addon) {
        int local_length = (int) ustrlen(local_source);
        do {
            if (local_source[reader] == '+') {
                first_part[writer] = '\0';
                latch = TRUE;
                reader++;
                writer = 0;
            }

            if (latch) {
                second_part[writer] = local_source[reader];
                reader++;
                writer++;
            } else {
                first_part[writer] = local_source[reader];
                reader++;
                writer++;
            }
        } while (reader <= local_length);

        if (symbol->symbology == BARCODE_UPCA || symbol->symbology == BARCODE_UPCA_CHK
                || symbol->symbology == BARCODE_UPCA_CC) {
            addon_gap = symbol->option_2 >= 9 && symbol->option_2 <= 12 ? symbol->option_2 : 9;
        } else {
            addon_gap = symbol->option_2 >= 7 && symbol->option_2 <= 12 ? symbol->option_2 : 7;
        }
    } else {
        ustrcpy(first_part, local_source);
    }

    first_part_len = (int) ustrlen(first_part);

    switch (symbol->symbology) {
        case BARCODE_EANX:
        case BARCODE_EANX_CHK:
            switch (first_part_len) {
                case 2: ean_add_on(first_part, first_part_len, dest, 0);
                    ustrcpy(symbol->text, first_part);
#ifdef COMPLIANT_HEIGHTS
                    /* 21.9mm from GS1 General Specifications 5.2.6.6, Figure 5.2.6.6-5 */
                    height = (float) (21.9 / 0.33); /* 21.9mm / 0.33mm ~ 66.36 */
                    error_number = set_height(symbol, height, height, 0.0f, 0 /*no_errtxt*/);
#else
                    height = 50.0f;
                    (void) set_height(symbol, 0.0f, height, 0.0f, 1 /*no_errtxt*/);
#endif
                    break;
                case 5: ean_add_on(first_part, first_part_len, dest, 0);
                    ustrcpy(symbol->text, first_part);
#ifdef COMPLIANT_HEIGHTS
                    /* 21.9mm from GS1 General Specifications 5.2.6.6, Figure 5.2.6.6-6 */
                    height = (float) (21.9 / 0.33); /* 21.9mm / 0.33mm ~ 66.36 */
                    error_number = set_height(symbol, height, height, 0.0f, 0 /*no_errtxt*/);
#else
                    height = 50.0f;
                    (void) set_height(symbol, 0.0f, height, 0.0f, 1 /*no_errtxt*/);
#endif
                    break;
                case 7:
                case 8: error_number = ean8(symbol, first_part, first_part_len, dest);
                    break;
                case 12:
                case 13: error_number = ean13(symbol, first_part, first_part_len, dest);
                    break;
                default: strcpy(symbol->errtxt, "286: Input wrong length (2, 5, 7, 8, 12 or 13 characters only)");
                    return ZINT_ERROR_TOO_LONG;
            }
            break;
        case BARCODE_EANX_CC:
            switch (first_part_len) { /* Adds vertical separator bars according to ISO/IEC 24723 section 11.4 */
                case 7: set_module(symbol, symbol->rows, 1);
                    set_module(symbol, symbol->rows, 67);
                    set_module(symbol, symbol->rows + 1, 0);
                    set_module(symbol, symbol->rows + 1, 68);
                    set_module(symbol, symbol->rows + 2, 1);
                    set_module(symbol, symbol->rows + 2, 67);
                    symbol->row_height[symbol->rows] = 2;
                    symbol->row_height[symbol->rows + 1] = 2;
                    symbol->row_height[symbol->rows + 2] = 2;
                    symbol->rows += 3;
                    error_number = ean8_cc(symbol, first_part, first_part_len, dest, cc_rows);
                    break;
                case 12:
                case 13:set_module(symbol, symbol->rows, 1);
                    set_module(symbol, symbol->rows, 95);
                    set_module(symbol, symbol->rows + 1, 0);
                    set_module(symbol, symbol->rows + 1, 96);
                    set_module(symbol, symbol->rows + 2, 1);
                    set_module(symbol, symbol->rows + 2, 95);
                    symbol->row_height[symbol->rows] = 2;
                    symbol->row_height[symbol->rows + 1] = 2;
                    symbol->row_height[symbol->rows + 2] = 2;
                    symbol->rows += 3;
                    error_number = ean13_cc(symbol, first_part, first_part_len, dest, cc_rows);
                    break;
                default: strcpy(symbol->errtxt, "287: Input wrong length (7, 12 or 13 characters only)");
                    return ZINT_ERROR_TOO_LONG;
            }
            break;
        case BARCODE_UPCA:
        case BARCODE_UPCA_CHK:
            if ((first_part_len == 11) || (first_part_len == 12)) {
                error_number = upca(symbol, first_part, first_part_len, dest);
            } else {
                strcpy(symbol->errtxt, "288: Input wrong length (12 character maximum)");
                return ZINT_ERROR_TOO_LONG;
            }
            break;
        case BARCODE_UPCA_CC:
            if (first_part_len == 11 || first_part_len == 12) {
                set_module(symbol, symbol->rows, 1);
                set_module(symbol, symbol->rows, 95);
                set_module(symbol, symbol->rows + 1, 0);
                set_module(symbol, symbol->rows + 1, 96);
                set_module(symbol, symbol->rows + 2, 1);
                set_module(symbol, symbol->rows + 2, 95);
                symbol->row_height[symbol->rows] = 2;
                symbol->row_height[symbol->rows + 1] = 2;
                symbol->row_height[symbol->rows + 2] = 2;
                symbol->rows += 3;
                error_number = upca_cc(symbol, first_part, first_part_len, dest, cc_rows);
            } else {
                strcpy(symbol->errtxt, "289: Input wrong length (12 character maximum)");
                return ZINT_ERROR_TOO_LONG;
            }
            break;
        case BARCODE_UPCE:
        case BARCODE_UPCE_CHK:
            if ((first_part_len >= 6) && (first_part_len <= 8)) {
                error_number = upce(symbol, first_part, first_part_len, dest);
            } else {
                strcpy(symbol->errtxt, "290: Input wrong length (8 character maximum)");
                return ZINT_ERROR_TOO_LONG;
            }
            break;
        case BARCODE_UPCE_CC:
            if ((first_part_len >= 6) && (first_part_len <= 8)) {
                set_module(symbol, symbol->rows, 1);
                set_module(symbol, symbol->rows, 51);
                set_module(symbol, symbol->rows + 1, 0);
                set_module(symbol, symbol->rows + 1, 52);
                set_module(symbol, symbol->rows + 2, 1);
                set_module(symbol, symbol->rows + 2, 51);
                symbol->row_height[symbol->rows] = 2;
                symbol->row_height[symbol->rows + 1] = 2;
                symbol->row_height[symbol->rows + 2] = 2;
                symbol->rows += 3;
                error_number = upce_cc(symbol, first_part, first_part_len, dest, cc_rows);
            } else {
                strcpy(symbol->errtxt, "291: Input wrong length (8 character maximum)");
                return ZINT_ERROR_TOO_LONG;
            }
            break;
        case BARCODE_ISBNX:
            error_number = isbn(symbol, first_part, first_part_len, dest);
            break;
    }

    if (error_number >= ZINT_ERROR) {
        return error_number;
    }

    second_part_len = (int) ustrlen(second_part);

    if (symbol->symbology == BARCODE_ISBNX) { /* Need to further check that add-on numeric only */
        error_number = is_sane(NEON, second_part, second_part_len);
        if (error_number == ZINT_ERROR_INVALID_DATA) {
            strcpy(symbol->errtxt, "295: Invalid add-on data (digits only)");
            return error_number;
        }
    }

    switch (second_part_len) {
        case 0: break;
        case 2:
            ean_add_on(second_part, second_part_len, dest, addon_gap);
            ustrcat(symbol->text, "+");
            ustrcat(symbol->text, second_part);
            break;
        case 5:
            ean_add_on(second_part, second_part_len, dest, addon_gap);
            ustrcat(symbol->text, "+");
            ustrcat(symbol->text, second_part);
            break;
        default:
            strcpy(symbol->errtxt, "292: Add-on data wrong length (2 or 5 characters only)");
            return ZINT_ERROR_TOO_LONG;
    }

    expand(symbol, (const char *) dest);

    switch (symbol->symbology) {
        case BARCODE_EANX_CC:
        case BARCODE_UPCA_CC:
        case BARCODE_UPCE_CC:
            /* shift the symbol to the right one space to allow for separator bars */
            for (i = (symbol->width + 1); i >= 1; i--) {
                if (module_is_set(symbol, symbol->rows - 1, i - 1)) {
                    set_module(symbol, symbol->rows - 1, i);
                } else {
                    unset_module(symbol, symbol->rows - 1, i);
                }
            }
            unset_module(symbol, symbol->rows - 1, 0);
            symbol->width += 2;
            break;
    }

    return error_number;
}

/* Handle UPC, EAN, ISBN */
INTERNAL int eanx(struct zint_symbol *symbol, unsigned char source[], int src_len) {
    return eanx_cc(symbol, source, src_len, 0 /*cc_rows*/);
}
