/* maxicode.c - Handles Maxicode */

/*
    libzint - the open source barcode library
    Copyright (C) 2010-2021 Robin Stuart <rstuart114@gmail.com>

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

/* Includes corrections thanks to Monica Swanson @ Source Technologies */
#include <stdio.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"
#include "maxicode.h"
#include "reedsol.h"

/* Handles error correction of primary message */
static void maxi_do_primary_check(unsigned char maxi_codeword[144]) {
    unsigned char results[15];
    int j;
    int datalen = 10;
    int ecclen = 10;
    rs_t rs;

    rs_init_gf(&rs, 0x43);
    rs_init_code(&rs, ecclen, 1);

    rs_encode(&rs, datalen, maxi_codeword, results);

    for (j = 0; j < ecclen; j += 1)
        maxi_codeword[ datalen + j] = results[ecclen - 1 - j];
}

/* Handles error correction of odd characters in secondary */
static void maxi_do_secondary_chk_odd(unsigned char maxi_codeword[144], const int ecclen) {
    unsigned char data[100];
    unsigned char results[30];
    int j;
    int datalen = 68;
    rs_t rs;

    rs_init_gf(&rs, 0x43);
    rs_init_code(&rs, ecclen, 1);

    if (ecclen == 20)
        datalen = 84;

    for (j = 1; j < datalen; j += 2)
        data[(j - 1) / 2] = maxi_codeword[j + 20];

    rs_encode(&rs, datalen / 2, data, results);

    for (j = 0; j < (ecclen); j += 1)
        maxi_codeword[ datalen + (2 * j) + 1 + 20 ] = results[ecclen - 1 - j];
}

/* Handles error correction of even characters in secondary */
static void maxi_do_secondary_chk_even(unsigned char maxi_codeword[144], const int ecclen) {
    unsigned char data[100];
    unsigned char results[30];
    int j;
    int datalen = 68;
    rs_t rs;

    if (ecclen == 20)
        datalen = 84;

    rs_init_gf(&rs, 0x43);
    rs_init_code(&rs, ecclen, 1);

    for (j = 0; j < datalen + 1; j += 2)
        data[j / 2] = maxi_codeword[j + 20];

    rs_encode(&rs, datalen / 2, data, results);

    for (j = 0; j < (ecclen); j += 1)
        maxi_codeword[ datalen + (2 * j) + 20] = results[ecclen - 1 - j];
}

/* Moves everything up so that a shift or latch can be inserted */
static void maxi_bump(unsigned char set[], unsigned char character[], const int bump_posn, int *p_length) {

    if (bump_posn < 143) {
        memmove(set + bump_posn + 1, set + bump_posn, 143 - bump_posn);
        memmove(character + bump_posn + 1, character + bump_posn, 143 - bump_posn);
    }
    (*p_length)++; /* Increment length regardless to make sure too long always triggered */
}

/* If the value is present in  array, return the value, else return badvalue */
static int value_in_array(const unsigned char val, const unsigned char arr[], const int badvalue,
            const int arrLength) {
    int i;
    for (i = 0; i < arrLength; i++) {
        if (arr[i] == val) return val;
    }
    return badvalue;
}

/* Choose the best set from previous and next set in the range of the setval array, if no value can be found we
 * return setval[0] */
static int bestSurroundingSet(const int index, const int length, const unsigned char set[],
            const unsigned char setval[], const int setLength) {
    int badValue = -1;
    int option1 = value_in_array(set[index - 1], setval, badValue, setLength);
    if (index + 1 < length) {
        // we have two options to check (previous & next)
        int option2 = value_in_array(set[index + 1], setval, badValue, setLength);
        if (option2 != badValue && option1 > option2) {
            return option2;
        }
    }

    if (option1 != badValue) {
        return option1;
    }
    return setval[0];
}

/* Format text according to Appendix A */
static int maxi_text_process(unsigned char maxi_codeword[144], const int mode, const unsigned char in_source[],
            int length, const int eci, const int scm_vv, const int debug_print) {

    unsigned char set[144], character[144] = {0};
    int i, count, current_set, padding_set;

    static const unsigned char set15[2] = { 1, 5 };
    static const unsigned char set12[2] = { 1, 2 };
    static const unsigned char set12345[5] = { 1, 2, 3, 4, 5 };

    const unsigned char *source = in_source;
#ifndef _MSC_VER
    unsigned char source_buf[length + 9]; /* For prefixing 9-character SCM sequence */
#else
    unsigned char *source_buf = (unsigned char *) _alloca(length + 9);
#endif

    if (length > 144) {
        return ZINT_ERROR_TOO_LONG;
    }

    if (scm_vv != -1) { /* Add SCM prefix */
        if (length > 135) {
            return ZINT_ERROR_TOO_LONG;
        }
        sprintf((char *) source_buf, "[)>\03601\035%02d", scm_vv); /* [)>\R01\Gvv */
        memcpy(source_buf + 9, in_source, length);
        source = source_buf;
        length += 9;
    }

    memset(set, 255, 144);

    for (i = 0; i < length; i++) {
        /* Look up characters in table from Appendix A - this gives
         value and code set for most characters */
        set[i] = maxiCodeSet[source[i]];
        character[i] = maxiSymbolChar[source[i]];
    }

    /* If a character can be represented in more than one code set,
    pick which version to use */
    if (set[0] == 0) {
        if (character[0] == 13) {
            character[0] = 0;
        }
        set[0] = 1;
    }

    for (i = 1; i < length; i++) {
        if (set[i] == 0) {
            /* Special character */
            if (character[i] == 13) {
                /* Carriage Return */
                set[i] = bestSurroundingSet(i, length, set, set15, 2);
                if (set[i] == 5) {
                    character[i] = 13;
                } else {
                    character[i] = 0;
                }

            } else if (character[i] == 28) {
                /* FS */
                set[i] = bestSurroundingSet(i, length, set, set12345, 5);
                if (set[i] == 5) {
                    character[i] = 32;
                }

            } else if (character[i] == 29) {
                /* GS */
                set[i] = bestSurroundingSet(i, length, set, set12345, 5);
                if (set[i] == 5) {
                    character[i] = 33;
                }

            } else if (character[i] == 30) {
                /* RS */
                set[i] = bestSurroundingSet(i, length, set, set12345, 5);
                if (set[i] == 5) {
                    character[i] = 34;
                }

            } else if (character[i] == 32) {
                /* Space */
                set[i] = bestSurroundingSet(i, length, set, set12345, 5);
                if (set[i] == 1) {
                    character[i] = 32;
                } else if (set[i] == 2) {
                    character[i] = 47;
                } else {
                    character[i] = 59;
                }

            } else if (character[i] == 44) {
                /* Comma */
                set[i] = bestSurroundingSet(i, length, set, set12, 2);
                if (set[i] == 2) {
                    character[i] = 48;
                }

            } else if (character[i] == 46) {
                /* Full Stop */
                set[i] = bestSurroundingSet(i, length, set, set12, 2);
                if (set[i] == 2) {
                    character[i] = 49;
                }

            } else if (character[i] == 47) {
                /* Slash */
                set[i] = bestSurroundingSet(i, length, set, set12, 2);
                if (set[i] == 2) {
                    character[i] = 50;
                }

            } else if (character[i] == 58) {
                /* Colon */
                set[i] = bestSurroundingSet(i, length, set, set12, 2);
                if (set[i] == 2) {
                    character[i] = 51;
                }
            }
        }
    }

    padding_set = set[length - 1] == 2 ? 2 : 1;
    for (i = length; i < 144; i++) {
        /* Add the padding */
        set[i] = padding_set;
        character[i] = 33;
    }

    /* Find candidates for number compression */
    /* Note the prohibition on number compression in the primary message in ISO/IEC 16023:2000 B.1 (1)
       applies to modes 2 & 3 only */
    count = 0;
    for (i = 0; i < 144; i++) {
        if ((set[i] == 1) && ((character[i] >= 48) && (character[i] <= 57))) {
            /* Character is a number */
            count++;
            if (count == 9) {
                /* Nine digits in a row can be compressed */
                memset(set + i - 8, 6, 9); /* Set set of nine digits to 6 */
                count = 0;
            }
        } else {
            count = 0;
        }
    }

    /* Add shift and latch characters */
    current_set = 1;
    i = 0;
    do {

        if ((set[i] != current_set) && (set[i] != 6)) {
            switch (set[i]) {
                case 1:
                    if (current_set == 2) { /* Set B */
                        if (i + 1 < 144 && set[i + 1] == 1) {
                            if (i + 2 < 144 && set[i + 2] == 1) {
                                if (i + 3 < 144 && set[i + 3] == 1) {
                                    /* Latch A */
                                    maxi_bump(set, character, i, &length);
                                    character[i] = 63; /* Set B Latch A */
                                    current_set = 1;
                                    i += 3; /* Next 3 Set A so skip over */
                                    if (debug_print) printf("LCHA ");
                                } else {
                                    /* 3 Shift A */
                                    maxi_bump(set, character, i, &length);
                                    character[i] = 57; /* Set B triple shift A */
                                    i += 2; /* Next 2 Set A so skip over */
                                    if (debug_print) printf("3SHA ");
                                }
                            } else {
                                /* 2 Shift A */
                                maxi_bump(set, character, i, &length);
                                character[i] = 56; /* Set B double shift A */
                                i++; /* Next Set A so skip over */
                                if (debug_print) printf("2SHA ");
                            }
                        } else {
                            /* Shift A */
                            maxi_bump(set, character, i, &length);
                            character[i] = 59; /* Set A Shift B */
                            if (debug_print) printf("SHA ");
                        }
                    } else { /* All sets other than B only have latch */
                        /* Latch A */
                        maxi_bump(set, character, i, &length);
                        character[i] = 58; /* Sets C,D,E Latch A */
                        current_set = 1;
                        if (debug_print) printf("LCHA ");
                    }
                    break;
                case 2: /* Set B */
                    if (current_set != 1 || (i + 1 < 144 && set[i + 1] == 2)) { /* If not Set A or next Set B */
                        /* Latch B */
                        maxi_bump(set, character, i, &length);
                        character[i] = 63; /* Sets A,C,D,E Latch B */
                        current_set = 2;
                        if (debug_print) printf("LCHB ");
                    } else { /* Only available from Set A */
                        /* Shift B */
                        maxi_bump(set, character, i, &length);
                        character[i] = 59; /* Set B Shift A */
                        if (debug_print) printf("SHB ");
                    }
                    break;
                case 3: /* Set C */
                case 4: /* Set D */
                case 5: /* Set E */
                    /* If first and next 3 same set, or not first and previous and next 2 same set */
                    if ((i == 0 && i + 3 < 144 && set[i + 1] == set[i] && set[i + 2] == set[i]
                                && set[i + 3] == set[i])
                            || (i > 0 && set[i - 1] == set[i] && i + 2 < 144 && set[i + 1] == set[i]
                                && set[i + 2] == set[i])) {
                        /* Lock in C/D/E */
                        if (i == 0) {
                            maxi_bump(set, character, i, &length);
                            character[i] = 60 + set[i] - 3;
                            i++; /* Extra bump */
                            maxi_bump(set, character, i, &length);
                            character[i] = 60 + set[i] - 3;
                            i += 3; /* Next 3 same set so skip over */
                        } else {
                            /* Add single Shift to previous Shift */
                            maxi_bump(set, character, i - 1, &length);
                            character[i - 1] = 60 + set[i] - 3;
                            i += 2; /* Next 2 same set so skip over */
                        }
                        current_set = set[i];
                        if (debug_print) printf("LCK%c ", 'C' + set[i] - 3);
                    } else {
                        /* Shift C/D/E */
                        maxi_bump(set, character, i, &length);
                        character[i] = 60 + set[i] - 3;
                        if (debug_print) printf("SH%c ", 'C' + set[i] - 3);
                    }
                    break;
            }
            i++; /* Allow for bump */
        }
        i++;
    } while (i < 144);

    if (debug_print) printf("\n");

    /* Number compression has not been forgotten! - It's handled below */
    i = 0;
    do {
        if (set[i] == 6) {
            /* Number compression */
            int value = to_int(character + i, 9);

            character[i] = 31; /* NS */
            character[i + 1] = (value & 0x3f000000) >> 24;
            character[i + 2] = (value & 0xfc0000) >> 18;
            character[i + 3] = (value & 0x3f000) >> 12;
            character[i + 4] = (value & 0xfc0) >> 6;
            character[i + 5] = (value & 0x3f);

            i += 6;
            memmove(set + i, set + i + 3, 141 - i);
            memmove(character + i, character + i + 3, 141 - i);
            length -= 3;
        } else {
            i++;
        }
    } while (i <= 135); /* 144 - 9 */

    /* Insert ECI at the beginning of message if needed */
    /* Encode ECI assignment numbers according to table 3 */
    if (eci != 0) {
        maxi_bump(set, character, 0, &length);
        character[0] = 27; // ECI
        if (eci <= 31) {
            maxi_bump(set, character, 1, &length);
            character[1] = eci;
        } else if (eci <= 1023) {
            maxi_bump(set, character, 1, &length);
            maxi_bump(set, character, 1, &length);
            character[1] = 0x20 | ((eci >> 6) & 0x0F);
            character[2] = eci & 0x3F;
        } else if (eci <= 32767) {
            maxi_bump(set, character, 1, &length);
            maxi_bump(set, character, 1, &length);
            maxi_bump(set, character, 1, &length);
            character[1] = 0x30 | ((eci >> 12) & 0x07);
            character[2] = (eci >> 6) & 0x3F;
            character[3] = eci & 0x3F;
        } else {
            maxi_bump(set, character, 1, &length);
            maxi_bump(set, character, 1, &length);
            maxi_bump(set, character, 1, &length);
            maxi_bump(set, character, 1, &length);
            character[1] = 0x38 | ((eci >> 18) & 0x03);
            character[2] = (eci >> 12) & 0x3F;
            character[3] = (eci >> 6) & 0x3F;
            character[4] = eci & 0x3F;
        }
    }

    if (debug_print) printf("Length: %d\n", length);

    if (((mode == 2) || (mode == 3)) && (length > 84)) {
        return ZINT_ERROR_TOO_LONG;

    } else if (((mode == 4) || (mode == 6)) && (length > 93)) {
        return ZINT_ERROR_TOO_LONG;

    } else if ((mode == 5) && (length > 77)) {
        return ZINT_ERROR_TOO_LONG;
    }

    /* Copy the encoded text into the codeword array */
    if ((mode == 2) || (mode == 3)) {
        for (i = 0; i < 84; i++) { /* secondary only */
            maxi_codeword[i + 20] = character[i];
        }

    } else if ((mode == 4) || (mode == 6)) {
        for (i = 0; i < 9; i++) { /* primary */
            maxi_codeword[i + 1] = character[i];
        }
        for (i = 0; i < 84; i++) { /* secondary */
            maxi_codeword[i + 20] = character[i + 9];
        }

    } else { /* Mode 5 */
        for (i = 0; i < 9; i++) { /* primary */
            maxi_codeword[i + 1] = character[i];
        }
        for (i = 0; i < 68; i++) { /* secondary */
            maxi_codeword[i + 20] = character[i + 9];
        }
    }

    return 0;
}

/* Format structured primary for Mode 2 */
static void maxi_do_primary_2(unsigned char maxi_codeword[144], const unsigned char postcode[],
            const int postcode_length, const int country, const int service) {
    int postcode_num;

    postcode_num = atoi((const char *) postcode);

    maxi_codeword[0] = ((postcode_num & 0x03) << 4) | 2;
    maxi_codeword[1] = ((postcode_num & 0xfc) >> 2);
    maxi_codeword[2] = ((postcode_num & 0x3f00) >> 8);
    maxi_codeword[3] = ((postcode_num & 0xfc000) >> 14);
    maxi_codeword[4] = ((postcode_num & 0x3f00000) >> 20);
    maxi_codeword[5] = ((postcode_num & 0x3c000000) >> 26) | ((postcode_length & 0x3) << 4);
    maxi_codeword[6] = ((postcode_length & 0x3c) >> 2) | ((country & 0x3) << 4);
    maxi_codeword[7] = (country & 0xfc) >> 2;
    maxi_codeword[8] = ((country & 0x300) >> 8) | ((service & 0xf) << 2);
    maxi_codeword[9] = ((service & 0x3f0) >> 4);
}

/* Format structured primary for Mode 3 */
static void maxi_do_primary_3(unsigned char maxi_codeword[144], unsigned char postcode[], const int country,
            const int service) {
    int i;

    /* Convert to Code Set A */
    for (i = 0; i < 6; i++) {
        postcode[i] = maxiSymbolChar[postcode[i]];
    }

    maxi_codeword[0] = ((postcode[5] & 0x03) << 4) | 3;
    maxi_codeword[1] = ((postcode[4] & 0x03) << 4) | ((postcode[5] & 0x3c) >> 2);
    maxi_codeword[2] = ((postcode[3] & 0x03) << 4) | ((postcode[4] & 0x3c) >> 2);
    maxi_codeword[3] = ((postcode[2] & 0x03) << 4) | ((postcode[3] & 0x3c) >> 2);
    maxi_codeword[4] = ((postcode[1] & 0x03) << 4) | ((postcode[2] & 0x3c) >> 2);
    maxi_codeword[5] = ((postcode[0] & 0x03) << 4) | ((postcode[1] & 0x3c) >> 2);
    maxi_codeword[6] = ((postcode[0] & 0x3c) >> 2) | ((country & 0x3) << 4);
    maxi_codeword[7] = (country & 0xfc) >> 2;
    maxi_codeword[8] = ((country & 0x300) >> 8) | ((service & 0xf) << 2);
    maxi_codeword[9] = ((service & 0x3f0) >> 4);
}

INTERNAL int maxicode(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i, j, block, shift, mode, lp = 0;
    int error_number = 0, eclen;
    unsigned char maxi_codeword[144] = {0};
    int scm_vv = -1;

    mode = symbol->option_1;

    if (mode <= 0) { /* If mode is unspecified (-1) or to be auto-determined (0) between 2 and 3 */
        lp = (int) strlen(symbol->primary);
        if (lp == 0) {
            if (mode == 0) { /* Require primary message to auto-determine between 2 and 3 */
                strcpy(symbol->errtxt, "554: Primary Message empty");
                return ZINT_ERROR_INVALID_DATA;
            }
            mode = 4;
        } else {
            mode = 2;
            for (i = 0; i < lp - 6; i++) {
                if (((symbol->primary[i] < '0') || (symbol->primary[i] > '9')) && (symbol->primary[i] != ' ')) {
                    mode = 3;
                    break;
                }
            }
        }
    }

    if ((mode < 2) || (mode > 6)) { /* Only codes 2 to 6 supported */
        strcpy(symbol->errtxt, "550: Invalid MaxiCode Mode");
        return ZINT_ERROR_INVALID_OPTION;
    }

    if ((mode == 2) || (mode == 3)) { /* Modes 2 and 3 need data in symbol->primary */
        unsigned char postcode[10];
        int countrycode;
        int service;
        int postcode_len;
        if (lp == 0) { /* Mode set manually means lp doesn't get set */
            lp = (int) strlen(symbol->primary);
        }
        if (lp < 7 || lp > 15) { /* 1 to 9 character postcode + 3 digit country code + 3 digit service class */
            strcpy(symbol->errtxt, "551: Invalid length for Primary Message");
            return ZINT_ERROR_INVALID_DATA;
        }
        postcode_len = lp - 6;

        countrycode = to_int((const unsigned char *) (symbol->primary + postcode_len), 3);
        service = to_int((const unsigned char *) (symbol->primary + postcode_len + 3), 3);

        if (countrycode == -1 || service == -1) { /* check that country code and service are numeric */
            strcpy(symbol->errtxt, "552: Non-numeric country code or service class in Primary Message");
            return ZINT_ERROR_INVALID_DATA;
        }

        memcpy(postcode, symbol->primary, postcode_len);
        postcode[postcode_len] = '\0';

        if (mode == 2) {
            for (i = 0; i < postcode_len; i++) {
                if (postcode[i] == ' ') {
                    postcode[i] = '\0';
                    postcode_len = i;
                    break;
                } else if (postcode[i] < '0' || postcode[i] > '9') {
                    strcpy(symbol->errtxt, "555: Non-numeric postcode in Primary Message");
                    return ZINT_ERROR_INVALID_DATA;
                }
            }
            maxi_do_primary_2(maxi_codeword, postcode, postcode_len, countrycode, service);
        } else {
            /* Just truncate and space-pad */
            postcode[6] = '\0';
            for (i = postcode_len; i < 6; i++) {
                postcode[i] = ' ';
            }
            /* Upper-case and check for Code Set A characters only */
            to_upper(postcode);
            for (i = 0; i < 6; i++) {
                /* Don't allow Code Set A control characters CR, RS, GS and RS */
                if (postcode[i] < ' ' || maxiCodeSet[postcode[i]] > 1) {
                    strcpy(symbol->errtxt, "556: Invalid character in postcode in Primary Message");
                    return ZINT_ERROR_INVALID_DATA;
                }
            }
            maxi_do_primary_3(maxi_codeword, postcode, countrycode, service);
        }

        if (symbol->option_2) { /* Check for option_2 = vv + 1, where vv is version of SCM prefix "[)>\R01\Gvv" */
            if (symbol->option_2 < 0 || symbol->option_2 > 100) {
                strcpy(symbol->errtxt, "557: Invalid SCM prefix version");
                return ZINT_ERROR_INVALID_OPTION;
            }
            scm_vv = symbol->option_2 - 1;
        }

        if (symbol->debug & ZINT_DEBUG_PRINT) {
            printf("Postcode: %s, Country Code: %d, Service Class: %d\n", postcode, countrycode, service);
        }
    } else {
        maxi_codeword[0] = mode;
    }

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("Mode: %d\n", mode);
    }

    i = maxi_text_process(maxi_codeword, mode, source, length, symbol->eci, scm_vv, symbol->debug & ZINT_DEBUG_PRINT);
    if (i == ZINT_ERROR_TOO_LONG) {
        strcpy(symbol->errtxt, "553: Input data too long");
        return i;
    }

    /* All the data is sorted - now do error correction */
    maxi_do_primary_check(maxi_codeword); /* always EEC */

    if (mode == 5)
        eclen = 56; // 68 data codewords , 56 error corrections
    else
        eclen = 40; // 84 data codewords,  40 error corrections

    maxi_do_secondary_chk_even(maxi_codeword, eclen / 2); // do error correction of even
    maxi_do_secondary_chk_odd(maxi_codeword, eclen / 2); // do error correction of odd

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("Codewords:");
        for (i = 0; i < 144; i++) printf(" %d", maxi_codeword[i]);
        printf("\n");
    }
#ifdef ZINT_TEST
    if (symbol->debug & ZINT_DEBUG_TEST) {
        debug_test_codeword_dump(symbol, maxi_codeword, 144);
    }
#endif

    /* Copy data into symbol grid */
    for (i = 0; i < 33; i++) {
        for (j = 0; j < 30; j++) {
            block = (MaxiGrid[(i * 30) + j] + 5) / 6;

            if (block != 0) {
                shift = 5 - ((MaxiGrid[(i * 30) + j] + 5) % 6);

                if ((maxi_codeword[block - 1] >> shift) & 0x1) {
                    set_module(symbol, i, j);
                }
            }
        }
    }

    /* Add orientation markings */
    set_module(symbol, 0, 28); // Top right filler
    set_module(symbol, 0, 29);
    set_module(symbol, 9, 10); // Top left marker
    set_module(symbol, 9, 11);
    set_module(symbol, 10, 11);
    set_module(symbol, 15, 7); // Left hand marker
    set_module(symbol, 16, 8);
    set_module(symbol, 16, 20); // Right hand marker
    set_module(symbol, 17, 20);
    set_module(symbol, 22, 10); // Bottom left marker
    set_module(symbol, 23, 10);
    set_module(symbol, 22, 17); // Bottom right marker
    set_module(symbol, 23, 17);

    symbol->width = 30;
    symbol->rows = 33;

    /* Note MaxiCode fixed size so symbol height ignored but set anyway */
    (void) set_height(symbol, 5.0f, 0.0f, 0.0f, 1 /*no_errtxt*/);

    return error_number;
}
