/* dxfilmedge.c - Handles DX Film Edge symbology */
/*
    libzint - the open source barcode library
    Copyright (C) 2024-2025 Antoine Merino <antoine.merino.dev@gmail.com>

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

/* DX Film Edge Barcode is used on 35mm and APS films:
 * https://en.wikipedia.org/wiki/DX_encoding
 *
 * A little information about decoding this symbology can be found at
 * https://www.merinorus.com/blog/identifying-manufacturer-35-mm-films/
 *
 * Partial specification and history can be found on this Kodak patent:
 * https://patents.google.com/patent/US4965628A/en
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

#define DEBUG_STR_LEN 20
/* Max length of the DX info part. Include the \0. Eg: "018500\0", "150-10\0" */
#define MAX_DX_INFO_LENGTH 7 
/* Max length of the frame info part. Eg: "00A\0", "23A\0" */
#define MAX_FRAME_INFO_LENGTH 4 

void int_to_binary(int value, int width, char *output) {
    int i;
    for (i = 0; i < width; i++) {
        output[width - 1 - i] = (value & (1 << i)) ? '1' : '0';
    }
    output[width] = '\0';
}

void str_to_uppercase(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = toupper(str[i]);
    }
}

int count_char_occurrences(const char *str, char target) {
    int i, count = 0;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == target) {
            count++;
            if (count > 1) {
                return count;
            }
        }
    }
    return count;
}


int parse_dx_code(struct zint_symbol *symbol, const char *source, char *binary_output, int *output_length, bool *has_frame_info) {

    int i;
    int parity_bit = 0;
    int dx_extract = -1, dx_code_1 = -1, dx_code_2 = -1, frame_number = -1;
    char binary_dx_code_1[8], binary_dx_code_2[5], binary_frame_number[7];
    char half_frame_flag = '\0';
    char dx_info[MAX_DX_INFO_LENGTH] = "\0";
    char frame_info[MAX_FRAME_INFO_LENGTH] = "\0";
    char *detected_char = strchr((const char *)(source), ' ');
    const char *frame_start;

    *has_frame_info = false;

    /* All codes should start with a digit*/
    if (not_sane(IS_NUM_F, (unsigned char *)source, 1)){
        return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1018, "Invalid character \"%c\", DX code should start with a number", source[0]);
    }

    /* Check if there is the '/' separator, which indicates the frame number is present. */
    detected_char = strchr((const char *)(source), '/');
    if (detected_char){
        /* Split the DX information from the frame number */
        size_t dx_length = detected_char - (char *)source;
        if (dx_length >= MAX_DX_INFO_LENGTH){
            return errtxt(ZINT_ERROR_INVALID_DATA, symbol, 1014, "DX information is too long");
        }
        strncat(dx_info, source, dx_length);
        dx_info[dx_length] = '\0';
        frame_start = detected_char + 1;
        if (strlen(frame_start) >= MAX_FRAME_INFO_LENGTH) {
            return errtxt(ZINT_ERROR_INVALID_DATA, symbol, 1002, "Frame number part is too long");
        }
        strncat(frame_info, frame_start, sizeof(frame_info) - 1);
        *has_frame_info = true;
        str_to_uppercase(frame_info);
        if ((i = not_sane(IS_UPR_F | IS_NUM_F | IS_MNS_F, (unsigned char *)(frame_info), strlen(frame_info)))){
            return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1012, "Frame number \"%s\" is invalid (expected digits, eventually followed by \'A\')", frame_info);
        }
    }
    else{
        /* No "/" found, store the entire input in dx_info */
        if (strlen(source) >= MAX_DX_INFO_LENGTH) {
            return errtxt(ZINT_ERROR_INVALID_DATA, symbol, 1003, "DX information is too long");
        }
        strncat(dx_info, source, sizeof(dx_info) - 1);
    }

    if ((i = not_sane(IS_NUM_F | IS_MNS_F, (unsigned char *)dx_info, strlen(dx_info)))){
        return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1016, "Invalid character at position %d in DX info (digits and \'-\' character only)", i);
    }

    if (ZINT_DEBUG_PRINT) printf("\nDX info part: \"%s\", Frame info part: \"%s\"\n", dx_info, frame_info);
    /* Parse the DX information */
    if (strchr(dx_info, '-')){
        /* DX code parts 1 and 2 are given directly, separated by a '-'. Eg: "79-7" */
        if (ZINT_DEBUG_PRINT) printf("DX code 1 and 2 are separated by a dash \'-\'\n");
        if (count_char_occurrences(dx_info, '-') > 1){
            return errtxt(ZINT_ERROR_INVALID_DATA, symbol, 1009, "The \'-\' is used to separate DX parts 1 and 2, and should be used no more than once");
        }
        if (sscanf(dx_info, "%d-%d", &dx_code_1, &dx_code_2) < 2){
            return errtxt(ZINT_ERROR_INVALID_DATA, symbol, 1004, "Wrong format for DX parts 1 and 2 (expected format: XXX-XX, digits)");
        }
        if (dx_code_1 <= 0 || dx_code_1 > 127){
            return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1006, "DX part 1 \"%d\" must be between 1 and 127", dx_code_1);
        }
        if (dx_code_2 < 0 || dx_code_2 > 15){
            return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1007, "DX part 2 \"%d\" must be between 0 and 15", dx_code_2);
        }
    }
    else{
        /* DX format is either 4 digits (DX Extract, eg: 1271) or 6 digits (DX Full, eg: 012710) */
        if (ZINT_DEBUG_PRINT) printf("No \'-\' separator, computing from DX Extract (4 digits) or DX Full (6 digits)\n");
        if (strlen(dx_info) == 5 || strlen(dx_info) > 6){
            return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1005, "DX number \"%s\" is incorrect; expected 4 digits (DX extract) or 6 digits (DX full)", dx_info);
        }
        if (strlen(dx_info) == 6){
            if (ZINT_DEBUG_PRINT) printf("DX full format detected: %s. Removing the first and the last characters.\n", dx_info);
            /* Convert DX Full to DX Extract (remove first and last character) */
            for (i=0; i <= 3; ++i){
                dx_info[i] = dx_info[i+1];
            }
            dx_info[4] = '\0';
        }
        /* Compute the DX parts 1 and 2 from the DX extract */
        if (sscanf(dx_info, "%d", &dx_extract) < 1){
            /* Should not happen (DX info format has been checked above), but better safe than sorry */
            return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1005, "DX number \"%s\" is incorrect; expected 4 digits (DX extract) or 6 digits (DX full)", dx_info);
        }
        if (dx_extract < 16 || dx_extract > 2047){
            return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1015, "DX extract \"%d\" must be between 16 and 2047", dx_extract);
        }
        if (ZINT_DEBUG_PRINT) printf("Computed DX extract: %04d\n", dx_extract);
        dx_code_1 = dx_extract / 16;
        dx_code_2 = dx_extract % 16;
    }

    /* Convert components to binary strings */
    int_to_binary(dx_code_1, 7, binary_dx_code_1);
    int_to_binary(dx_code_2, 4, binary_dx_code_2);

    if (ZINT_DEBUG_PRINT) printf("%-*s%d\t-> %s\n", DEBUG_STR_LEN, "DX code 1:", dx_code_1, binary_dx_code_1);
    if (ZINT_DEBUG_PRINT) printf("%-*s%d\t-> %s\n", DEBUG_STR_LEN, "DX code 2:", dx_code_2, binary_dx_code_2);

    if (*has_frame_info) {
        if (strlen(frame_info) < 1){
            return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1017, "Frame number indicator \"/\" at position %d, but frame number is empty", (int)(detected_char - (char *)source + 1));
        }
        /* Some frame numbers are special values, convert them their equivalent number */
        if (strcmp(frame_info, "S") == 0 || strcmp(frame_info, "X") == 0) {
            strcpy(frame_info, "62");
        } else if (strcmp(frame_info, "SA") == 0 || strcmp(frame_info, "XA") == 0) {
            strcpy(frame_info, "62A");
        } else if (strcmp(frame_info, "K") == 0 || strcmp(frame_info, "00") == 0) {
            strcpy(frame_info, "63");
        } else if (strcmp(frame_info, "KA") == 0 || strcmp(frame_info, "00A") == 0) {
            strcpy(frame_info, "63A");
        } else if (strcmp(frame_info, "F") == 0) {
            strcpy(frame_info, "0");
        } else if (strcmp(frame_info, "FA") == 0) {
            strcpy(frame_info, "0A");
        }

        if (sscanf(frame_info, "%d%c", &frame_number, &half_frame_flag) < 1){
            return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1012, "Frame number \"%s\" is invalid (expected digits, eventually followed by \'A\')", frame_info);
        }
        if (frame_number < 0 || frame_number > 63){
            return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1008, "Frame number \"%d\"should be between 0 and 63", frame_number);
        }
        int_to_binary(frame_number, 6, binary_frame_number);
        if (ZINT_DEBUG_PRINT) printf("%-*s%d\t-> %s\n", DEBUG_STR_LEN, "Frame number:", frame_number, binary_frame_number);
    }

    /* Build the binary output */
    strcpy(binary_output, "101010"); /* Start pattern */
    strcat(binary_output, binary_dx_code_1);
    strcat(binary_output, "0"); /* Separator between DX part 1 and DX part 2 */
    strcat(binary_output, binary_dx_code_2);
    if (*has_frame_info) {
        strcat(binary_output, binary_frame_number);
        if (toupper(half_frame_flag) == 'A') {
            if (ZINT_DEBUG_PRINT) printf("%-*s\'%c\'\t-> 1\n", DEBUG_STR_LEN, "Half frame flag:", half_frame_flag);
            strcat(binary_output, "1"); /* Half-frame is set */
        } else {
            if (half_frame_flag){
                return errtxtf(ZINT_ERROR_INVALID_DATA, symbol, 1012, "Frame number \"%s\" is invalid (expected digits, eventually followed by \'A\')", frame_info);
            }
            if (ZINT_DEBUG_PRINT) printf("%-*s\'%c\'\t-> 0\n", DEBUG_STR_LEN, "Half frame flag:", half_frame_flag);
            strcat(binary_output, "0"); /* Half-frame is NOT set */
        }
        strcat(binary_output, "0"); /* Separator between half frame flag and parity bit*/
    }
    
    /* Parity bit */
    for (i = 6; binary_output[i] != '\0'; i++) {
        if (binary_output[i] == '1') {
            parity_bit++;
        }
    }
    parity_bit %= 2;
    if (ZINT_DEBUG_PRINT) printf("%-*s%s\t-> %d\n", DEBUG_STR_LEN, "Parity bit:", parity_bit?"yes":"no", parity_bit);
    if (parity_bit){
        strcat(binary_output, "1");
    }
    else{
        strcat(binary_output, "0");
    }
    
    strcat(binary_output, "0101"); /* Stop pattern */

    *output_length = strlen(binary_output);
    return 0;
}


INTERNAL int dxfilmedge(struct zint_symbol *symbol, char source[], int length) {
    int i;
    int writer = 0;
    int error_number = 0;

    char char_data[32];
    int data_length;
    bool has_frame_info;

    const char long_clock_pattern[] = "1111101010101010101010101010111";
    const char short_clock_pattern[] = "11111010101010101010111";
    const char *clock_pattern;
    int clock_length;
    int parse_result = -1;

    if (length > 10) {
        return errtxtf(ZINT_ERROR_TOO_LONG, symbol, 1013, "Input length %d too long (maximum 10)", length);
    }

    parse_result = parse_dx_code(symbol, source, char_data, &data_length, &has_frame_info);
    if (parse_result != 0){
        if (ZINT_DEBUG_PRINT) printf("Error %s\n\n", symbol->errtxt);
        return parse_result;
    }

    /* Clock signal is longer if the frame number is provided */
    if (has_frame_info){
        clock_pattern = long_clock_pattern;
        clock_length = sizeof(long_clock_pattern) -1;
    }
    else{
        clock_pattern = short_clock_pattern;
        clock_length = sizeof(short_clock_pattern) -1;
    }
  
    /* First row: clock pattern */
    for (i = 0; i < clock_length; i++) {
        if (clock_pattern[i] == '1') {
            set_module(symbol, 0, writer);
        } else if (clock_pattern[i] == '0') {
            unset_module(symbol, 0, writer);
        }
        writer++;
    }

    /* Reset writer X position for the second row */
    writer = 0;

    /* Second row: data signal */
    for (i = 0; i < clock_length; i++) {
        if (char_data[i] == '1') {
            set_module(symbol, 1, writer);
        } else if (char_data[i] == '0') {
            unset_module(symbol, 1, writer);
        }
        writer++;
    }
    symbol->rows = 2; 
    symbol->width = clock_length;

    if (symbol->output_options & COMPLIANT_HEIGHT) {
        /* Measured ratio on 35mm films. Depending on the brands, one symbol height is about 3 * the X-dim.*/
        const float default_height = 6.0f;

        /* AFAIK There is no standard on minimum and maximum height, so we stay close to the measurements */
        const float min_row_height = 2.2f;
        const float max_height = 7.5f;
        error_number = set_height(symbol, min_row_height, default_height, max_height, 0 /*no_errtxt*/);
    }

    return error_number;
}
