/* plessey.c - Handles Plessey and MSI Plessey */

/*
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

#include <stdio.h>
#include "common.h"

#define SSET    "0123456789ABCDEF"

static const char *PlessTable[16] = {
    "13131313", "31131313", "13311313", "31311313",
    "13133113", "31133113", "13313113", "31313113",
    "13131331", "31131331", "13311331", "31311331",
    "13133131", "31133131", "13313131", "31313131"
};

static const char *MSITable[10] = {
    "12121212", "12121221", "12122112", "12122121", "12211212",
    "12211221", "12212112", "12212121", "21121212", "21121221"
};

/* Not MSI/Plessey but the older Plessey standard */
INTERNAL int plessey(struct zint_symbol *symbol, unsigned char source[], int length) {

    int i;
    unsigned char *checkptr;
    static const char grid[9] = {1, 1, 1, 1, 0, 1, 0, 0, 1};
    char dest[554]; /* 8 + 65 * 8 + 8 * 2 + 9 + 1 = 554 */
    int error_number;

    if (length > 65) {
        strcpy(symbol->errtxt, "370: Input too long (65 character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }
    error_number = is_sane(SSET, source, length);
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "371: Invalid character in data (digits and \"ABCDEF\" only)");
        return error_number;
    }

    if (!(checkptr = (unsigned char *) calloc(1, length * 4 + 8))) {
        strcpy(symbol->errtxt, "373: Insufficient memory for check digit CRC buffer");
        return ZINT_ERROR_MEMORY;
    }

    /* Start character */
    strcpy(dest, "31311331");

    /* Data area */
    for (i = 0; i < length; i++) {
        unsigned int check = posn(SSET, source[i]);
        lookup(SSET, PlessTable, source[i], dest);
        checkptr[4 * i] = check & 1;
        checkptr[4 * i + 1] = (check >> 1) & 1;
        checkptr[4 * i + 2] = (check >> 2) & 1;
        checkptr[4 * i + 3] = (check >> 3) & 1;
    }

    /* CRC check digit code adapted from code by Leonid A. Broukhis
       used in GNU Barcode */

    for (i = 0; i < (4 * length); i++) {
        if (checkptr[i]) {
            int j;
            for (j = 0; j < 9; j++)
                checkptr[i + j] ^= grid[j];
        }
    }

    for (i = 0; i < 8; i++) {
        switch (checkptr[length * 4 + i]) {
            case 0: strcat(dest, "13");
                break;
            case 1: strcat(dest, "31");
                break;
        }
    }

    /* Stop character */
    strcat(dest, "331311313");

    expand(symbol, dest);

    // TODO: Find documentation on BARCODE_PLESSEY dimensions/height

    symbol->text[0] = '\0';
    ustrncat(symbol->text, source, length);

    free(checkptr);
    return error_number;
}

/* Modulo 10 check digit - Luhn algorithm
   See https://en.wikipedia.org/wiki/Luhn_algorithm */
static char msi_check_digit_mod10(const unsigned char source[], const int length) {
    static const int vals[2][10] = {
        { 0, 2, 4, 6, 8, 1, 3, 5, 7, 9 }, /* Doubled and digits summed */
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, /* Single */
    };
    int i, x = 0, undoubled = 0;

    for (i = length - 1; i >= 0; i--) {
        /* Note overflow impossible for max length 65 * max weight 9 * max val 15 == 8775 */
        x += vals[undoubled][ctoi(source[i])];
        undoubled = !undoubled;
    }

    return itoc((10 - x % 10) % 10);
}

/* Modulo 11 check digit - IBM weight system wrap = 7, NCR system wrap = 9
   See https://en.wikipedia.org/wiki/MSI_Barcode */
static char msi_check_digit_mod11(const unsigned char source[], const int length, const int wrap) {
    int i, x = 0, weight = 2;

    for (i = length - 1; i >= 0; i--) {
        /* Note overflow impossible for max length 65 * max weight 9 * max val 15 == 8775 */
        x += weight * ctoi(source[i]);
        weight++;
        if (weight > wrap) {
            weight = 2;
        }
    }

    return itoc((11 - x % 11) % 11); /* Will return 'A' for 10 */
}

/* Plain MSI Plessey - does not calculate any check character */
static void msi_plessey(struct zint_symbol *symbol, const unsigned char source[], const int length, char dest[]) {

    int i;

    for (i = 0; i < length; i++) {
        lookup(NEON, MSITable, source[i], dest);
    }

    symbol->text[0] = '\0';
    ustrncat(symbol->text, source, length);
}

/* MSI Plessey with Modulo 10 check digit */
static void msi_plessey_mod10(struct zint_symbol *symbol, const unsigned char source[], const int length,
            const int no_checktext, char dest[]) {
    int i;
    char check_digit;

    /* draw data section */
    for (i = 0; i < length; i++) {
        lookup(NEON, MSITable, source[i], dest);
    }

    /* calculate check digit */
    check_digit = msi_check_digit_mod10(source, length);

    /* draw check digit */
    lookup(NEON, MSITable, check_digit, dest);

    symbol->text[0] = '\0';
    ustrncat(symbol->text, source, length);
    if (!no_checktext) {
        symbol->text[length] = check_digit;
        symbol->text[length + 1] = '\0';
    }
}

/* MSI Plessey with two Modulo 10 check digits */
static void msi_plessey_mod1010(struct zint_symbol *symbol, const unsigned char source[], const int length,
            const int no_checktext, char dest[]) {

    int i;
    unsigned char temp[65 + 2 + 1];

    /* Append check digits */
    temp[0] = '\0';
    ustrncat(temp, source, length);
    temp[length] = msi_check_digit_mod10(source, length);
    temp[length + 1] = msi_check_digit_mod10(temp, length + 1);
    temp[length + 2] = '\0';

    /* draw data section */
    for (i = 0; i < length + 2; i++) {
        lookup(NEON, MSITable, temp[i], dest);
    }

    if (no_checktext) {
        symbol->text[0] = '\0';
        ustrncat(symbol->text, source, length);
    } else {
        ustrcpy(symbol->text, temp);
    }
}

/* MSI Plessey with Modulo 11 check digit */
static void msi_plessey_mod11(struct zint_symbol *symbol, const unsigned char source[], const int length,
            const int no_checktext, const int wrap, char dest[]) {
    /* Uses the IBM weight system if wrap = 7, and the NCR system if wrap = 9 */
    int i;
    char check_digit;

    /* draw data section */
    for (i = 0; i < length; i++) {
        lookup(NEON, MSITable, source[i], dest);
    }

    /* Append check digit */
    check_digit = msi_check_digit_mod11(source, length, wrap);
    if (check_digit == 'A') {
        lookup(NEON, MSITable, '1', dest);
        lookup(NEON, MSITable, '0', dest);
    } else {
        lookup(NEON, MSITable, check_digit, dest);
    }

    symbol->text[0] = '\0';
    ustrncat(symbol->text, source, length);
    if (!no_checktext) {
        if (check_digit == 'A') {
            ustrcat(symbol->text, "10");
        } else {
            symbol->text[length] = check_digit;
            symbol->text[length + 1] = '\0';
        }
    }
}

/* MSI Plessey with Modulo 11 check digit and Modulo 10 check digit */
static void msi_plessey_mod1110(struct zint_symbol *symbol, const unsigned char source[], const int length,
            const int no_checktext, const int wrap, char dest[]) {
    /* Uses the IBM weight system if wrap = 7, and the NCR system if wrap = 9 */
    int i;
    char check_digit;
    unsigned char temp[65 + 3 + 1];
    int temp_len = length;

    temp[0] = '\0';
    ustrncat(temp, source, length);

    /* Append first (mod 11) digit */
    check_digit = msi_check_digit_mod11(source, length, wrap);
    if (check_digit == 'A') {
        temp[temp_len++] = '1';
        temp[temp_len++] = '0';
    } else {
        temp[temp_len++] = check_digit;
    }

    /* Append second (mod 10) check digit */
    temp[temp_len] = msi_check_digit_mod10(temp, temp_len);
    temp[++temp_len] = '\0';

    /* draw data section */
    for (i = 0; i < temp_len; i++) {
        lookup(NEON, MSITable, temp[i], dest);
    }

    if (no_checktext) {
        symbol->text[0] = '\0';
        ustrncat(symbol->text, source, length);
    } else {
        ustrcpy(symbol->text, temp);
    }
}

INTERNAL int msi_handle(struct zint_symbol *symbol, unsigned char source[], int length) {
    int error_number;
    char dest[550]; /* 2 + 65 * 8 + 3 * 8 + 3 + 1 = 550 */
    int check_option = symbol->option_2;
    int no_checktext = 0;

    if (length > 65) {
        strcpy(symbol->errtxt, "372: Input too long (65 character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }
    error_number = is_sane(NEON, source, length);
    if (error_number != 0) {
        strcpy(symbol->errtxt, "377: Invalid character in data (digits only)");
        return ZINT_ERROR_INVALID_DATA;
    }

    if (check_option >= 11 && check_option <= 16) { /* +10 means don't print check digits in HRT */
        check_option -= 10;
        no_checktext = 1;
    }
    if ((check_option < 0) || (check_option > 6)) {
        check_option = 0;
    }

    /* Start character */
    strcpy(dest, "21");

    switch (check_option) {
        case 0: msi_plessey(symbol, source, length, dest);
            break;
        case 1: msi_plessey_mod10(symbol, source, length, no_checktext, dest);
            break;
        case 2: msi_plessey_mod1010(symbol, source, length, no_checktext, dest);
            break;
        case 3: msi_plessey_mod11(symbol, source, length, no_checktext, 7 /*IBM wrap*/, dest);
            break;
        case 4: msi_plessey_mod1110(symbol, source, length, no_checktext, 7 /*IBM wrap*/, dest);
            break;
        case 5: msi_plessey_mod11(symbol, source, length, no_checktext, 9 /*NCR wrap*/, dest);
            break;
        case 6: msi_plessey_mod1110(symbol, source, length, no_checktext, 9 /*NCR wrap*/, dest);
            break;
    }

    /* Stop character */
    strcat(dest, "121");

    expand(symbol, dest);

    // TODO: Find documentation on BARCODE_MSI_PLESSEY dimensions/height

    return error_number;
}
