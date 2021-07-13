/* gs1.c - Verifies GS1 data */

/*
    libzint - the open source barcode library
    Copyright (C) 2009 - 2021 Robin Stuart <rstuart114@gmail.com>

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
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"
#include "gs1.h"

/* gs1_lint() validators and checkers */

/* Validate numeric */
static int numeric(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50]) {

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }

    if (data_len) {
        const unsigned char *d = data + offset;
        const unsigned char *de = d + (data_len > max ? max : data_len);

        for (; d < de; d++) {
            if (*d < '0' || *d > '9') {
                *p_err_no = 3;
                *p_err_posn = d - data + 1;
                sprintf(err_msg, "Non-numeric character '%c'", *d);
                return 0;
            }
        }
    }

    return 1;
}

/* GS1 General Specifications 21.0.1 Figure 7.9.5-1. GS1 AI encodable character reference values.
   Also used to determine if character in set 82 */
static const char c82[] = {
     0,  1, -1, -1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, /*!-0*/
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, -1, /*1-@*/
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, /*A-P*/
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54, -1, -1, -1, -1, 55, -1, /*Q-`*/
    56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, /*a-p*/
    72, 73, 74, 75, 76, 77, 78, 79, 80, 81, /*q-z*/
};

/* Validate of character set 82 (GS1 General Specifications Figure 7.11-1) */
static int cset82(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50]) {

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }

    if (data_len) {
        const unsigned char *d = data + offset;
        const unsigned char *de = d + (data_len > max ? max : data_len);

        for (; d < de; d++) {
            if (*d < '!' || *d > 'z' || c82[*d - '!'] == -1) {
                *p_err_no = 3;
                *p_err_posn = d - data + 1;
                sprintf(err_msg, "Invalid CSET 82 character '%c'", *d);
                return 0;
            }
        }
    }

    return 1;
}

/* Validate of character set 39 (GS1 General Specifications Figure 7.11-2) */
static int cset39(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50]) {

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }

    if (data_len) {
        const unsigned char *d = data + offset;
        const unsigned char *de = d + (data_len > max ? max : data_len);

        for (; d < de; d++) {
            /* 0-9, A-Z and "#", "-", "/" */
            if ((*d < '0' && *d != '#' && *d != '-' && *d != '/') || (*d > '9' && *d < 'A') || *d > 'Z') {
                *p_err_no = 3;
                *p_err_posn = d - data + 1;
                sprintf(err_msg, "Invalid CSET 39 character '%c'", *d);
                return 0;
            }
        }
    }

    return 1;
}

/* Check a check digit (GS1 General Specifications 7.9.1) */
static int csum(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }

    if (!length_only && data_len) {
        const unsigned char *d = data + offset;
        const unsigned char *de = d + (data_len > max ? max : data_len) - 1; /* Note less last character */
        int checksum = 0;
        int factor = (min & 1) ? 1 : 3;

        for (; d < de; d++) {
            checksum += (*d - '0') * factor;
            factor = factor == 3 ? 1 : 3;
        }
        checksum = 10 - checksum % 10;
        if (checksum == 10) {
            checksum = 0;
        }
        if (checksum != *d - '0') {
            *p_err_no = 3;
            *p_err_posn = d - data + 1;
            sprintf(err_msg, "Bad checksum '%c', expected '%c'", *d, checksum + '0');
            return 0;
        }
    }

    return 1;
}

/* Check alphanumeric check characters (GS1 General Specifications 7.9.5) */
static int csumalpha(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }
    if (data_len && data_len < 2) { /* Do this check separately for backward compatibility */
        *p_err_no = 4;
        return 0;
    }

    if (!length_only && data_len) {
        static const char c32[] = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ";
        static const char weights[] = {
            2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83
        };
        const unsigned char *d = data + offset;
        const unsigned char *de = d + (data_len > max ? max : data_len) - 2; /* Note less last 2 characters */
        int checksum = 0, c1, c2;

        for (; d < de; d++) {
            checksum += c82[*d - '!'] * weights[de - 1 - d];
        }
        checksum %= 1021;
        c1 = c32[checksum >> 5];
        c2 = c32[checksum & 0x1F];

        if (de[0] != c1 || de[1] != c2) {
            *p_err_no = 3;
            if (de[0] != c1) {
                *p_err_posn = (de - data) + 1;
                sprintf(err_msg, "Bad checksum '%c', expected '%c'", de[0], c1);
            } else {
                *p_err_posn = (de + 1 - data) + 1;
                sprintf(err_msg, "Bad checksum '%c', expected '%c'", de[1], c2);
            }
            return 0;
        }
    }

    return 1;
}

/* Check for a GS1 Prefix (GS1 General Specifications GS1 1.4.2) */
static int key(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }
    if (data_len && data_len < 2) { /* Do this check separately for backward compatibility */
        *p_err_no = 4;
        return 0;
    }

    if (!length_only && data_len) {
        data += offset;

        if (data[0] < '0' || data[0] > '9' || data[1] < '0' || data[1] > '9') {
            *p_err_no = 3;
            *p_err_posn = offset + (data[0] < '0' || data[0] > '9' ? 0 : 1) + 1;
            sprintf(err_msg, "Non-numeric company prefix '%c'", data[0] < '0' || data[0] > '9' ? data[0] : data[1]);
            return 0;
        }
    }

    return 1;
}

/* Check for a date YYMMDD with zero day allowed */
static int yymmd0(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {

    static char days_in_month[] = { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    (void)max;

    data_len -= offset;

    if (data_len < min || (data_len && data_len < 6)) {
        return 0;
    }

    if (!length_only && data_len) {
        int month, day;

        month = to_int(data + offset + 2, 2);
        if (month == 0 || month > 12) {
            *p_err_no = 3;
            *p_err_posn = offset + 2 + 1;
            sprintf(err_msg, "Invalid month '%.2s'", data + offset + 2);
            return 0;
        }

        day = to_int(data + offset + 4, 2);
        if (day && day > days_in_month[month]) {
            *p_err_no = 3;
            *p_err_posn = offset + 4 + 1;
            sprintf(err_msg, "Invalid day '%.2s'", data + offset + 4);
            return 0;
        }
        if (month == 2 && day == 29) { /* Leap year check */
            int year = to_int(data + offset, 2);
            if (year & 3) { /* Good until 2050 when 00 will mean 2100 (GS1 General Specifications 7.12) */
                *p_err_no = 3;
                *p_err_posn = offset + 4 + 1;
                sprintf(err_msg, "Invalid day '%.2s'", data + offset + 4);
                return 0;
            }
        }
    }

    return 1;
}

/* Check for a date YYMMDD. Zero day NOT allowed */
static int yymmdd(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {

    if (!yymmd0(data, data_len, offset, min, max, p_err_no, p_err_posn, err_msg, length_only)) {
        return 0;
    }

    data_len -= offset;

    if (!length_only && data_len) {
        int day = to_int(data + offset + 4, 2);
        if (day == 0) {
            *p_err_no = 3;
            *p_err_posn = offset + 4 + 1;
            sprintf(err_msg, "Invalid day '%.2s'", data + offset + 4);
            return 0;
        }
    }

    return 1;
}

/* Check for a date and hours YYMMDDHH */
static int yymmddhh(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {

    if (data_len < min || (data_len && data_len < 8)) {
        return 0;
    }

    if (!yymmdd(data, data_len, offset, min, max, p_err_no, p_err_posn, err_msg, length_only)) {
        return 0;
    }

    data_len -= offset;

    if (!length_only && data_len) {
        int hour = to_int(data + offset + 6, 2);
        if (hour > 23) {
            *p_err_no = 3;
            *p_err_posn = offset + 6 + 1;
            sprintf(err_msg, "Invalid hour of day '%.2s'", data + offset + 6);
            return 0;
        }
    }

    return 1;
}

/* Check for a time HHMM */
static int hhmm(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min || (data_len && data_len < 4)) {
        return 0;
    }

    if (!length_only && data_len) {
        int hour, mins;

        hour = to_int(data + offset, 2);
        if (hour > 23) {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            sprintf(err_msg, "Invalid hour of day '%.2s'", data + offset);
            return 0;
        }
        mins = to_int(data + offset + 2, 2);
        if (mins > 59) {
            *p_err_no = 3;
            *p_err_posn = offset + 2 + 1;
            sprintf(err_msg, "Invalid minutes in the hour '%.2s'", data + offset + 2);
            return 0;
        }
    }

    return 1;
}

/* Check for a time MMSS with seconds optional */
static int mmoptss(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min || (data_len && data_len < 2)
            || (data_len > 2 && data_len < 4)) {
        return 0;
    }

    if (!length_only && data_len) {
        int mins = to_int(data + offset, 2);
        if (mins > 59) {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            sprintf(err_msg, "Invalid minutes in the hour '%.2s'", data + offset);
            return 0;
        }
        if (data_len > 2) {
            int secs = to_int(data + offset + 2, 2);
            if (secs > 59) {
                *p_err_no = 3;
                *p_err_posn = offset + 2 + 1;
                sprintf(err_msg, "Invalid seconds in the minute '%.2s'", data + offset + 2);
                return 0;
            }
        }
    }

    return 1;
}

/* Generated by "php backend/tools/gen_iso3166_h.php > backend/iso3166.h" */
#include "iso3166.h"

/* Check for an ISO 3166-1 numeric country code */
static int iso3166(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min || (data_len && data_len < 3)) {
        return 0;
    }

    if (!length_only && data_len) {
        if (!iso3166_numeric(to_int(data + offset, 3))) {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            sprintf(err_msg, "Unknown country code '%.3s'", data + offset);
            return 0;
        }
    }

    return 1;
}

/* Check for a list of ISO 3166-1 numeric country codes */
static int iso3166list(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {

    int data_len_max;

    data_len -= offset;
    data_len_max = data_len > max ? max : data_len;

    if (data_len < min || (data_len && data_len < 3)) {
        return 0;
    }
    if (data_len && data_len_max % 3) { /* Do this check separately for backward compatibility */
        *p_err_no = 4;
        return 0;
    }

    if (!length_only && data_len) {
        int i;
        for (i = 0; i < data_len_max; i += 3) {
            if (!iso3166(data, offset + data_len, offset + i, 3, 3, p_err_no, p_err_posn, err_msg, length_only)) {
                return 0;
            }
        }
    }

    return 1;
}

/* Check for an ISO 3166-1 numeric country code allowing "999" */
static int iso3166999(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min || (data_len && data_len < 3)) {
        return 0;
    }

    if (!length_only && data_len) {
        int cc = to_int(data + offset, 3);
        if (cc != 999 && !iso3166_numeric(cc)) {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            sprintf(err_msg, "Unknown country code '%.3s'", data + offset);
            return 0;
        }
    }

    return 1;
}

/* Check for an ISO 3166-1 alpha2 country code */
static int iso3166alpha2(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min || (data_len && data_len < 2)) {
        return 0;
    }

    if (!length_only && data_len) {
        if (!iso3166_alpha2((const char *) (data + offset))) {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            sprintf(err_msg, "Unknown country code '%.2s'", data + offset);
            return 0;
        }
    }

    return 1;
}

/* Generated by "php backend/tools/gen_iso4217_h.php > backend/iso4217.h" */
#include "iso4217.h"

/* Check for an ISO 4217 currency code */
static int iso4217(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min || (data_len && data_len < 3)) {
        return 0;
    }

    if (!length_only && data_len) {
        if (!iso4217_numeric(to_int(data + offset, 3))) {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            sprintf(err_msg, "Unknown currency code '%.3s'", data + offset);
            return 0;
        }
    }

    return 1;
}

/* Check for percent encoded */
static int pcenc(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {

    static const char hex_chars[] = "0123456789ABCDEFabcdef";

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }

    if (!length_only && data_len) {
        const unsigned char *d = data + offset;
        const unsigned char *de = d + (data_len > max ? max : data_len);

        for (; d < de; d++) {
            if (*d == '%') {
                if (de - d < 3) {
                    *p_err_no = 3;
                    *p_err_posn = d - data + 1;
                    strcpy(err_msg, "Invalid % escape");
                    return 0;
                }
                if (strchr(hex_chars, *(++d)) == NULL || strchr(hex_chars, *(++d)) == NULL) {
                    *p_err_no = 3;
                    *p_err_posn = d - data + 1;
                    strcpy(err_msg, "Invalid character for percent encoding");
                    return 0;
                }
            }
        }
    }

    return 1;
}

/* Check for yes/no (1/0) indicator */
static int yesno(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }

    if (!length_only && data_len) {
        if (data[offset] != '0' && data[offset] != '1') {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            strcpy(err_msg, "Neither 0 nor 1 for yes or no");
            return 0;
        }
    }

    return 1;
}

/* Check for importer index (GS1 General Specifications 3.8.17) */
static int importeridx(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }

    if (!length_only && data_len) {
        const unsigned char *d = data + offset;

        /* 0-9, A-Z, a-z and "-", "_" */
        if ((*d < '0' && *d != '-') || (*d > '9' && *d < 'A') || (*d > 'Z' && *d < 'a' && *d != '_') || *d > 'z') {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            sprintf(err_msg, "Invalid importer index '%c'", *d);
            return 0;
        }
    }

    return 1;
}

/* Check non-zero */
static int nonzero(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }

    if (!length_only && data_len) {
        int val = to_int(data + offset, data_len > max ? max : data_len);

        if (val == 0) {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            strcpy(err_msg, "Zero not permitted");
            return 0;
        }
    }

    return 1;
}

/* Check winding direction (0/1/9) (GS1 General Specifications 3.9.1) */
static int winding(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }

    if (!length_only && data_len) {
        if (data[offset] != '0' && data[offset] != '1' && data[offset] != '9') {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            sprintf(err_msg, "Invalid winding direction '%c'", data[offset]);
            return 0;
        }
    }

    return 1;
}

/* Check zero */
static int zero(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }

    if (!length_only && data_len) {
        if (data[offset] != '0') {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            strcpy(err_msg, "Zero is required");
            return 0;
        }
    }

    return 1;
}

/* Check piece of a trade item (GS1 General Specifications 3.9.6 and 3.9.17) */
static int pieceoftotal(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min || (data_len && data_len < 4)) {
        return 0;
    }

    if (!length_only && data_len) {
        int pieces, total;

        pieces = to_int(data + offset, 2);
        if (pieces == 0) {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            strcpy(err_msg, "Piece number cannot be zero");
            return 0;
        }
        total = to_int(data + offset + 2, 2);
        if (total == 0) {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            strcpy(err_msg, "Total number cannot be zero");
            return 0;
        }
        if (pieces > total) {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            sprintf(err_msg, "Piece number '%.2s' exceeds total '%.2s'", data + offset, data + offset + 2);
            return 0;
        }
    }

    return 1;
}

/* Check IBAN (ISO 13616) */
static int iban(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }
    if (data_len && data_len < 5) { /* Do this check separately for backward compatibility */
        *p_err_no = 4;
        return 0;
    }

    if (!length_only && data_len) {
        const unsigned char *d = data + offset;
        const unsigned char *de = d + (data_len > max ? max : data_len);
        int checksum = 0;
        int given_checksum;

        if (d[0] < 'A' || d[0] > 'Z' || d[1] < 'A' || d[1] > 'Z') { /* 1st 2 chars alphabetic country code */
            *p_err_no = 3;
            *p_err_posn = d - data + 1;
            sprintf(err_msg, "Non-alphabetic IBAN country code '%.2s'", d);
            return 0;
        }
        if (!iso3166_alpha2((const char *) d)) {
            *p_err_no = 3;
            *p_err_posn = d - data + 1;
            sprintf(err_msg, "Invalid IBAN country code '%.2s'", d);
            return 0;
        }
        d += 2;
        if (d[0] < '0' || d[0] > '9' || d[1] < '0' || d[1] > '9') { /* 2nd 2 chars numeric checksum */
            *p_err_no = 3;
            *p_err_posn = d - data + 1;
            sprintf(err_msg, "Non-numeric IBAN checksum '%.2s'", d);
            return 0;
        }
        given_checksum = to_int(d, 2);
        d += 2;
        for (; d < de; d++) {
            /* 0-9, A-Z */
            if (*d < '0' || (*d > '9' && *d < 'A') || *d > 'Z') {
                *p_err_no = 3;
                *p_err_posn = d - data + 1;
                sprintf(err_msg, "Invalid IBAN character '%c'", *d);
                return 0;
            }
            if (*d >= 'A') {
                checksum = checksum * 100 + *d - 'A' + 10;
            } else {
                checksum = checksum * 10 + *d - '0';
            }
            checksum %= 97;
        }

        /* Add in country code */
        checksum = (((checksum * 100) % 97) + (data[offset] - 'A' + 10)) * 100 + data[offset + 1] - 'A' + 10;
        checksum %= 97;

        checksum *= 100; /* Allow for checksum "00" */
        checksum %= 97;

        checksum = 98 - checksum;

        if (checksum != given_checksum) {
            *p_err_no = 3;
            *p_err_posn = offset + 2 + 1;
            sprintf(err_msg, "Bad IBAN checksum '%.2s', expected '%02d'", data + offset + 2, checksum);
            return 0;
        }
    }

    return 1;
}

/* Check CPID does not begin with zero */
static int nozeroprefix(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {
    (void)max;

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }

    if (!length_only && data_len) {
        /* GS1 General Specifications 3.9.11 "The C/P serial number SHALL NOT begin with a "0" digit, unless the
           entire serial number consists of the single digit '0'." */
        if (data[0] == '0' && data_len != 1) {
            *p_err_no = 3;
            *p_err_posn = offset + 1;
            strcpy(err_msg, "Zero prefix is not permitted");
            return 0;
        }
    }

    return 1;
}

/* Helper to parse coupon Variable Length Indicator (VLI) and associated field. If `vli_nine` set
 * then a VLI of '9' means no field present */
static const unsigned char *coupon_vli(const unsigned char *data, const int data_len, const unsigned char *d,
            const char *name, const int vli_offset, const int vli_min, const int vli_max, const int vli_nine,
            int *p_err_no, int *p_err_posn, char err_msg[50]) {
    const unsigned char *de;
    int vli;

    if (d - data + 1 > data_len) {
        *p_err_no = 3;
        *p_err_posn = d - data + 1;
        sprintf(err_msg, "%s VLI missing", name);
        return NULL;
    }
    vli = to_int(d, 1);
    if ((vli < vli_min || vli > vli_max) && (vli != 9 || !vli_nine)) {
        *p_err_no = 3;
        *p_err_posn = d - data + 1;
        sprintf(err_msg, vli < 0 ? "Non-numeric %s VLI '%c'" : "Invalid %s VLI '%c'", name, *d);
        return NULL;
    }
    d++;
    if (vli != 9 || !vli_nine) {
        if (d - data + vli + vli_offset > data_len) {
            *p_err_no = 3;
            *p_err_posn = d - data + 1;
            sprintf(err_msg, "%s incomplete", name);
            return NULL;
        }
        de = d + vli + vli_offset;
        for (; d < de; d++) {
            if (*d < '0' || *d > '9') {
                *p_err_no = 3;
                *p_err_posn = d - data + 1;
                sprintf(err_msg, "Non-numeric %s '%c'", name, *d);
                return NULL;
            }
        }
    }

    return d;
}

/* Helper to parse coupon value field (numeric) */
static const unsigned char *coupon_val(const unsigned char *data, const int data_len, const unsigned char *d,
            const char *name, const int val_len, int *p_val, int *p_err_no, int *p_err_posn, char err_msg[50]) {
    int val;

    if (d - data + val_len > data_len) {
        *p_err_no = 3;
        *p_err_posn = d - data + 1;
        sprintf(err_msg, "%s incomplete", name);
        return NULL;
    }
    val = to_int(d, val_len);
    if (val < 0) {
        *p_err_no = 3;
        *p_err_posn = d - data + 1;
        sprintf(err_msg, "Non-numeric %s", name);
        return NULL;
    }
    d += val_len;

    if (p_val) {
        *p_val = val;
    }
    return d;
}

/* Check North American Coupon Code */
/* Note all fields including optional must be numeric so type could be N..70 */
static int couponcode(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {

    /* Minimum possible required fields length = 21
     * (from "North American Coupon Application Guideline Using GS1 DataBar Expanded Symbols R2.0 (Feb 13 2015)")
     * VLI - Variable Length Indicator; GCP - GS1 Company Prefix; OC - Offer Code; SV - Save Value;
     * PPR - Primary Purchase Requirement; PPFC - Primary Purchase Family Code */
    const int min_req_len = 1 /*GCP VLI*/ + 6 /*GCP*/ + 6 /*OC*/ + 1 /*SV VLI*/ + 1 /*SV*/
                            + 1 /*PPR VLI*/ + 1 /*PPR*/ + 1 /*PPR Code*/ + 3 /*PPFC*/;

    (void)max;

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }
    if (data_len && data_len < min_req_len) { /* Do separately for backward compatibility */
        *p_err_no = 4;
        return 0;
    }

    if (!length_only && data_len) {
        const unsigned char *d = data + offset;
        int val;

        data_len += offset;

        /* Required fields */
        d = coupon_vli(data, data_len, d, "Primary GS1 Co. Prefix", 6, 0, 6, 0, p_err_no, p_err_posn, err_msg);
        if (d == NULL) {
            return 0;
        }
        d = coupon_val(data, data_len, d, "Offer Code", 6, NULL, p_err_no, p_err_posn, err_msg);
        if (d == NULL) {
            return 0;
        }
        d = coupon_vli(data, data_len, d, "Save Value", 0, 1, 5, 0, p_err_no, p_err_posn, err_msg);
        if (d == NULL) {
            return 0;
        }
        d = coupon_vli(data, data_len, d, "Primary Purch. Req.", 0, 1, 5, 0, p_err_no, p_err_posn, err_msg);
        if (d == NULL) {
            return 0;
        }
        d = coupon_val(data, data_len, d, "Primary Purch. Req. Code", 1, &val, p_err_no, p_err_posn, err_msg);
        if (d == NULL) {
            return 0;
        }
        if (val > 5 && val < 9) {
            *p_err_no = 3;
            *p_err_posn = d - 1 - data + 1;
            sprintf(err_msg, "Invalid Primary Purch. Req. Code '%c'", *(d - 1));
            return 0;
        }
        d = coupon_val(data, data_len, d, "Primary Purch. Family Code", 3, NULL, p_err_no, p_err_posn, err_msg);
        if (d == NULL) {
            return 0;
        }

        /* Optional fields */
        while (d - data < data_len) {
            int data_field = to_int(d, 1);
            d++;

            if (data_field == 1) {

                d = coupon_val(data, data_len, d, "Add. Purch. Rules Code", 1, &val, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                if (val > 3) {
                    *p_err_no = 3;
                    *p_err_posn = d - 1 - data + 1;
                    sprintf(err_msg, "Invalid Add. Purch. Rules Code '%c'", *(d - 1));
                    return 0;
                }
                d = coupon_vli(data, data_len, d, "2nd Purch. Req.", 0, 1, 5, 0, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                d = coupon_val(data, data_len, d, "2nd Purch. Req. Code", 1, &val, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                if (val > 4 && val < 9) {
                    *p_err_no = 3;
                    *p_err_posn = d - 1 - data + 1;
                    sprintf(err_msg, "Invalid 2nd Purch. Req. Code '%c'", *(d - 1));
                    return 0;
                }
                d = coupon_val(data, data_len, d, "2nd Purch. Family Code", 3, NULL, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                d = coupon_vli(data, data_len, d, "2nd Purch. GS1 Co. Prefix", 6, 0, 6, 1, p_err_no, p_err_posn,
                        err_msg);
                if (d == NULL) {
                    return 0;
                }

            } else if (data_field == 2) {

                d = coupon_vli(data, data_len, d, "3rd Purch. Req.", 0, 1, 5, 0, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                d = coupon_val(data, data_len, d, "3rd Purch. Req. Code", 1, &val, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                if (val > 4 && val < 9) {
                    *p_err_no = 3;
                    *p_err_posn = d - 1 - data + 1;
                    sprintf(err_msg, "Invalid 3rd Purch. Req. Code '%c'", *(d - 1));
                    return 0;
                }
                d = coupon_val(data, data_len, d, "3rd Purch. Family Code", 3, NULL, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                d = coupon_vli(data, data_len, d, "3rd Purch. GS1 Co. Prefix", 6, 0, 6, 1, p_err_no, p_err_posn,
                        err_msg);
                if (d == NULL) {
                    return 0;
                }

            } else if (data_field == 3) {

                d = coupon_val(data, data_len, d, "Expiration Date", 6, NULL, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                if (!yymmd0(data, data_len, d - 6 - data, 6, 6, p_err_no, p_err_posn, err_msg, 0)) {
                    return 0;
                }

            } else if (data_field == 4) {

                d = coupon_val(data, data_len, d, "Start Date", 6, NULL, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                if (!yymmd0(data, data_len, d - 6 - data, 6, 6, p_err_no, p_err_posn, err_msg, 0)) {
                    return 0;
                }

            } else if (data_field == 5) {

                d = coupon_vli(data, data_len, d, "Serial Number", 6, 0, 9, 0, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }

            } else if (data_field == 6) {

                d = coupon_vli(data, data_len, d, "Retailer ID", 6, 1, 7, 0, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }

            } else if (data_field == 9) {

                d = coupon_val(data, data_len, d, "Save Value Code", 1, &val, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                if ((val > 2 && val < 5) || val > 6) {
                    *p_err_no = 3;
                    *p_err_posn = d - 1 - data + 1;
                    sprintf(err_msg, "Invalid Save Value Code '%c'", *(d - 1));
                    return 0;
                }
                d = coupon_val(data, data_len, d, "Save Value Applies To", 1, &val, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                if (val > 2) {
                    *p_err_no = 3;
                    *p_err_posn = d - 1 - data + 1;
                    sprintf(err_msg, "Invalid Save Value Applies To '%c'", *(d - 1));
                    return 0;
                }
                d = coupon_val(data, data_len, d, "Store Coupon Flag", 1, NULL, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                d = coupon_val(data, data_len, d, "Don't Multiply Flag", 1, &val, p_err_no, p_err_posn, err_msg);
                if (d == NULL) {
                    return 0;
                }
                if (val > 1) {
                    *p_err_no = 3;
                    *p_err_posn = d - 1 - data + 1;
                    sprintf(err_msg, "Invalid Don't Multiply Flag '%c'", *(d - 1));
                    return 0;
                }

            } else {

                *p_err_no = 3;
                *p_err_posn = d - 1 - data + 1;
                sprintf(err_msg, data_field < 0 ? "Non-numeric Data Field '%c'" : "Invalid Data Field '%c'",
                    *(d - 1));
                return 0;
            }
        }
    }

    return 1;
}

/* Check North American Positive Offer File */
/* Note max is currently set at 36 numeric digits with remaining 34 characters reserved */
static int couponposoffer(const unsigned char *data, int data_len, int offset, int min, int max, int *p_err_no,
            int *p_err_posn, char err_msg[50], const int length_only) {

    /* Minimum possible length = 21
     * (from "GS1 AI (8112) Coupon Data Specifications Release 1.0 (March 2020)")
     * CFMT - Coupon Format; CFID - Coupon Funder ID; VLI - Variable Length Indicator;
     * OC - Offer Code; SN - Serial Number */
    const int min_len = 1 /*CFMT*/ + 1 /*CFID VLI*/ + 6 /*CFID*/ + 6 /*OC*/ + 1 /*SN VLI*/ + 6 /*SN*/;
    const int max_len = 36;

    (void)max;

    data_len -= offset;

    if (data_len < min) {
        return 0;
    }
    if (data_len && (data_len < min_len || data_len > max_len)) { /* Do separately for backward compatibility */
        *p_err_no = 4;
        return 0;
    }

    if (!length_only && data_len) {
        const unsigned char *d = data + offset;
        int val;

        d = coupon_val(data, data_len, d, "Coupon Format", 1, &val, p_err_no, p_err_posn, err_msg);
        if (d == NULL) {
            return 0;
        }
        if (val != 0 && val != 1) {
            *p_err_no = 3;
            *p_err_posn = d - 1 - data + 1;
            strcpy(err_msg, "Coupon Format must be 0 or 1");
            return 0;
        }
        d = coupon_vli(data, data_len, d, "Coupon Funder ID", 6, 0, 6, 0, p_err_no, p_err_posn, err_msg);
        if (d == NULL) {
            return 0;
        }
        d = coupon_val(data, data_len, d, "Offer Code", 6, NULL, p_err_no, p_err_posn, err_msg);
        if (d == NULL) {
            return 0;
        }
        d = coupon_vli(data, data_len, d, "Serial Number", 6, 0, 9, 0, p_err_no, p_err_posn, err_msg);
        if (d == NULL) {
            return 0;
        }
        if (d - data != data_len) {
            *p_err_no = 3;
            *p_err_posn = d - data + 1;
            strcpy(err_msg, "Reserved trailing characters");
            return 0;
        }
    }

    return 1;
}

/* Generated by "php backend/tools/gen_gs1_linter.php > backend/gs1_lint.h" */
#include "gs1_lint.h"

/* Verify a GS1 input string */
INTERNAL int gs1_verify(struct zint_symbol *symbol, const unsigned char source[], const int src_len,
                unsigned char reduced[]) {
    int i, j, last_ai, ai_latch;
    char ai_string[5]; /* 4 char max "NNNN" */
    int bracket_level, max_bracket_level, ai_length, max_ai_length, min_ai_length;
    int ai_count;
    int error_value = 0;
    char obracket = symbol->input_mode & GS1PARENS_MODE ? '(' : '[';
    char cbracket = symbol->input_mode & GS1PARENS_MODE ? ')' : ']';
    int ai_max = chr_cnt(source, src_len, obracket) + 1; /* Plus 1 so non-zero */
#ifndef _MSC_VER
    int ai_value[ai_max], ai_location[ai_max], data_location[ai_max], data_length[ai_max];
#else
    int *ai_value = (int *) _alloca(ai_max * sizeof(int));
    int *ai_location = (int *) _alloca(ai_max * sizeof(int));
    int *data_location = (int *) _alloca(ai_max * sizeof(int));
    int *data_length = (int *) _alloca(ai_max * sizeof(int));
#endif

    /* Detect extended ASCII characters */
    for (i = 0; i < src_len; i++) {
        if (source[i] >= 128) {
            strcpy(symbol->errtxt, "250: Extended ASCII characters are not supported by GS1");
            return ZINT_ERROR_INVALID_DATA;
        }
        if (source[i] == '\0') {
            strcpy(symbol->errtxt, "262: NUL characters not permitted in GS1 mode");
            return ZINT_ERROR_INVALID_DATA;
        }
        if (source[i] < 32) {
            strcpy(symbol->errtxt, "251: Control characters are not supported by GS1");
            return ZINT_ERROR_INVALID_DATA;
        }
        if (source[i] == 127) {
            strcpy(symbol->errtxt, "263: DEL characters are not supported by GS1");
            return ZINT_ERROR_INVALID_DATA;
        }
    }

    if (source[0] != obracket) {
        strcpy(symbol->errtxt, "252: Data does not start with an AI");
        return ZINT_ERROR_INVALID_DATA;
    }

    /* Check the position of the brackets */
    bracket_level = 0;
    max_bracket_level = 0;
    ai_length = 0;
    max_ai_length = 0;
    min_ai_length = 5;
    j = 0;
    ai_latch = 0;
    for (i = 0; i < src_len; i++) {
        ai_length += j;
        if (((j == 1) && (source[i] != cbracket)) && ((source[i] < '0') || (source[i] > '9'))) {
            ai_latch = 1;
        }
        if (source[i] == obracket) {
            bracket_level++;
            j = 1;
        }
        if (source[i] == cbracket) {
            bracket_level--;
            if (ai_length < min_ai_length) {
                min_ai_length = ai_length;
            }
            j = 0;
            ai_length = 0;
        }
        if (bracket_level > max_bracket_level) {
            max_bracket_level = bracket_level;
        }
        if (ai_length > max_ai_length) {
            max_ai_length = ai_length;
        }
    }
    min_ai_length--;

    if (bracket_level != 0) {
        /* Not all brackets are closed */
        strcpy(symbol->errtxt, "253: Malformed AI in input data (brackets don\'t match)");
        return ZINT_ERROR_INVALID_DATA;
    }

    if (max_bracket_level > 1) {
        /* Nested brackets */
        strcpy(symbol->errtxt, "254: Found nested brackets in input data");
        return ZINT_ERROR_INVALID_DATA;
    }

    if (max_ai_length > 4) {
        /* AI is too long */
        strcpy(symbol->errtxt, "255: Invalid AI in input data (AI too long)");
        return ZINT_ERROR_INVALID_DATA;
    }

    if (min_ai_length <= 1) {
        /* AI is too short */
        strcpy(symbol->errtxt, "256: Invalid AI in input data (AI too short)");
        return ZINT_ERROR_INVALID_DATA;
    }

    if (ai_latch == 1) {
        /* Non-numeric data in AI */
        strcpy(symbol->errtxt, "257: Invalid AI in input data (non-numeric characters in AI)");
        return ZINT_ERROR_INVALID_DATA;
    }

    ai_count = 0;
    for (i = 1; i < src_len; i++) {
        if (source[i - 1] == obracket) {
            ai_location[ai_count] = i;
            j = 0;
            do {
                ai_string[j] = source[i + j];
                j++;
            } while (ai_string[j - 1] != cbracket);
            ai_string[j - 1] = '\0';
            ai_value[ai_count] = atoi(ai_string);
            ai_count++;
        }
    }

    for (i = 0; i < ai_count; i++) {
        data_location[i] = ai_location[i] + 3;
        if (ai_value[i] >= 100) {
            data_location[i]++;
            if (ai_value[i] >= 1000) {
                data_location[i]++;
            }
        }
        data_length[i] = 0;
        while ((data_location[i] + data_length[i] < src_len)
                    && (source[data_location[i] + data_length[i]] != obracket)) {
            data_length[i]++;
        }
        if (data_length[i] == 0) {
            /* No data for given AI */
            strcpy(symbol->errtxt, "258: Empty data field in input data");
            return ZINT_ERROR_INVALID_DATA;
        }
    }

    strcpy(ai_string, "");

    if (!(symbol->input_mode & GS1NOCHECK_MODE)) {
        // Check for valid AI values and data lengths according to GS1 General
        // Specifications Release 21.0.1, January 2021
        for (i = 0; i < ai_count; i++) {
            int err_no, err_posn;
            char err_msg[50];
            if (!gs1_lint(ai_value[i], source + data_location[i], data_length[i], &err_no, &err_posn, err_msg)) {
                if (err_no == 1) {
                    sprintf(symbol->errtxt, "260: Invalid AI (%02d)", ai_value[i]);
                } else if (err_no == 2 || err_no == 4) { /* 4 is backward-incompatible bad length */
                    sprintf(symbol->errtxt, "259: Invalid data length for AI (%02d)", ai_value[i]);
                } else {
                    sprintf(symbol->errtxt, "261: AI (%02d) position %d: %s", ai_value[i], err_posn, err_msg);
                }
                /* For backward compatibility only error on unknown AI or bad length */
                if (err_no == 1 || err_no == 2) {
                    return ZINT_ERROR_INVALID_DATA;
                }
                error_value = ZINT_WARN_NONCOMPLIANT;
            }
        }
    }

    /* Resolve AI data - put resulting string in 'reduced' */
    j = 0;
    ai_latch = 1;
    for (i = 0; i < src_len; i++) {
        if ((source[i] != obracket) && (source[i] != cbracket)) {
            reduced[j++] = source[i];
        }
        if (source[i] == obracket) {
            /* Start of an AI string */
            if (ai_latch == 0) {
                reduced[j++] = '[';
            }
            ai_string[0] = source[i + 1];
            ai_string[1] = source[i + 2];
            ai_string[2] = '\0';
            last_ai = atoi(ai_string);
            ai_latch = 0;
            /* The following values from "GS1 General Specifications Release 21.0.1"
               Figure 7.8.4-2 "Element strings with predefined length using GS1 Application Identifiers" */
            if (
                    ((last_ai >= 0) && (last_ai <= 4))
                    || ((last_ai >= 11) && (last_ai <= 20))
                    || (last_ai == 23) /* legacy support */
                    || ((last_ai >= 31) && (last_ai <= 36))
                    || (last_ai == 41)
                    ) {
                ai_latch = 1;
            }
        }
        /* The ']' character is simply dropped from the input */
    }
    reduced[j] = '\0';

    /* the character '[' in the reduced string refers to the FNC1 character */
    return error_value;
}

/* Helper to return standard GS1 check digit (GS1 General Specifications 7.9.1) */
INTERNAL char gs1_check_digit(const unsigned char source[], const int length) {
    int i;
    int count = 0;
    int factor = length & 1 ? 3 : 1;

    for (i = 0; i < length; i++) {
        count += factor * ctoi(source[i]);
        factor = factor == 1 ? 3 : 1;
    }

    return itoc((10 - (count % 10)) % 10);
}
