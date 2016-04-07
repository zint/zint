/*  hanxin.c - Han Xin Code

    libzint - the open source barcode library
    Copyright (C) 2009-2016 Robin Stuart <rstuart114@gmail.com>

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

/* This code attempts to implement Han Xin Code according to AIMD-015:2010 (Rev 0.8) */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"
#include "reedsol.h"
#include "hanxin.h"

/* Calculate the approximate length of the binary string */
int calculate_binlength(char mode[], int length) {
    int i;
    char lastmode = ' ';
    int est_binlen = 0;
    
    for (i = 0; i < length; i++) {
        switch (mode[i]) {
            case 'n':
                if (lastmode != 'n') {
                    est_binlen += 14;
                    lastmode = 'n';
                }
                est_binlen += 4;
                break;
            case 't':
                if (lastmode != 't') {
                    est_binlen += 10;
                    lastmode = 't';
                }
                est_binlen += 6;
            case 'b':
                if (lastmode != 'b') {
                    est_binlen += 17;
                    lastmode = 'b';
                }
        }
    }
    
    return est_binlen;
}

/* Calculate mode switching */
void hx_define_mode(char mode[], const unsigned char source[], int length) {
    int i;
    char lastmode = 't';
    
    for(i = 0; i < length; i++) {
        if ((source[i] >= '0') && (source[i] <= '9')) {
            mode[i] = 'n';
            if (lastmode != 'n') {
                lastmode = 'n';
            }
        } else {
            if ((source[i] <= 127) && ((source[i] <= 27) || (source[i] >= 32))) {
                mode[i] = 't';
                if (lastmode != 't') {
                    lastmode = 't';
                }
            } else {
                mode[i] = 'b';
                if (lastmode != 'b') {
                    lastmode = 'b';
                }
            }
        }
    }
    mode[length] = '\0';
}

/* Find which submode to use for a text character */
int getsubmode(char input) {
    int submode = 2;
    
    if ((input >= '0') && (input <= '9')) {
        submode = 1;
    }
    
    if ((input >= 'A') && (input <= 'Z')) {
        submode = 1;
    }
    
    if ((input >= 'a') && (input <= 'z')) {
        submode = 1;
    }
    
    return submode;
}

/* Convert Text 1 sub-mode character to encoding value, as given in table 3 */
int lookup_text1(char input) {
    int encoding_value = 0;
    
    if ((input >= '0') && (input <= '9')) {
        encoding_value = input - '0';
    }
    
    if ((input >= 'A') && (input <= 'Z')) {
        encoding_value = input - 'A' + 10;
    }
    
    if ((input >= 'a') && (input <= 'z')) {
        encoding_value = input - 'a' + 36;
    }
    
    return encoding_value;
}

/* Convert Text 2 sub-mode character to encoding value, as given in table 4 */
int lookup_text2(char input) {
    int encoding_value = 0;
    
    if ((input >= 0) && (input <= 27)) {
        encoding_value = input;
    }
    
    if ((input >= ' ') && (input <= '/')) {
        encoding_value = input - ' ' + 28;
    }
    
    if ((input >= '[') && (input <= 96)) {
        encoding_value = input - '[' + 51;
    }
    
    if ((input >= '{') && (input <= 127)) {
        encoding_value = input - '{' + 57;
    }
    
    return encoding_value;
}

/* Convert input data to binary stream */
void calculate_binary(char binary[], char mode[], const unsigned char source[], int length) {
    int block_length;
    int position = 0;
    int i, p, count, encoding_value;
    int debug = 1;
    
    do {
        block_length = 0;
        do {
            block_length++;
        } while (mode[position + block_length] == mode[position]);
        
        switch(mode[position]) {
            case 'n':
                /* Numeric mode */
                /* Mode indicator */
                strcat(binary, "0001");
                
                if (debug) {
                    printf("Numeric\n");
                }
                
                i = 0;
                
                while (i < block_length) {
                    int first = 0, second = 0, third = 0;
                    
                    first = posn(NEON, (char) source[position + i]);
                    count = 1;
                    encoding_value = first;
                    
                    if (i + 1 < block_length && mode[position + i + 1] == 'n') {
                        second = posn(NEON, (char) source[position + i + 1]);
                        count = 2;
                        encoding_value = (encoding_value * 10) + second;
                        
                        if (i + 2 < block_length && mode[position + i + 2] == 'n') {
                            third = posn(NEON, (char) source[position + i + 2]);
                            count = 3;
                            encoding_value = (encoding_value * 10) + third;
                        }
                    }
                    
                    for (p = 0; p < 10; p++) {
                        if (encoding_value & (0x200 >> p)) {
                            strcat(binary, "1");
                        } else {
                            strcat(binary, "0");
                        }
                    }
                    
                    if (debug) {
                        printf("0x%4X (%d)", encoding_value, encoding_value);
                    }
                    
                    i += count;
                }
                
                /* Mode terminator depends on number of characters in last group (Table 2) */
                switch (count) {
                    case 1:
                        strcat(binary, "1111111101");
                        break;
                    case 2:
                        strcat(binary, "1111111110");
                        break;
                    case 3:
                        strcat(binary, "1111111111");
                        break;
                }
                
                if (debug) {
                    printf(" (TERM %d)\n", count);
                }
                
                break;
            case 't':
                /* Text mode */
                if (position != 0) {
                    /* Mode indicator */
                    strcat(binary, "0010");
                    
                    if (debug) {
                        printf("Text\n");
                    }
                }
                
                int submode = 1;
                    
                i = 0;
                    
                while (i < block_length) {
                        
                    if (getsubmode(source[i + position]) != submode) {
                        /* Change submode */
                        strcat(binary, "111110");
                        submode = getsubmode(source[i + position]);
                        if (debug) {
                            printf("SWITCH ");
                        }
                    }
                    
                    if (submode == 1) {
                        encoding_value = lookup_text1((char) source[i + position]);
                    } else {
                        encoding_value = lookup_text2((char) source[i + position]);
                    }
                    
                    for (p = 0; p < 6; p++) {
                        if (encoding_value & (0x20 >> p)) {
                            strcat(binary, "1");
                        } else {
                            strcat(binary, "0");
                        }
                    }
                        
                    if (debug) {
                        printf("%c (%d) ", (char) source[i], encoding_value);
                    }
                    i++;
                }
                
                /* Terminator */
                strcat(binary, "111111");
                
                if (debug) {
                    printf("\n");
                }
                break;
            case 'b':
                /* Binary Mode */
                /* Mode indicator */
                strcat(binary, "0011");
                
                /* Count indicator */
                for (p = 0; p < 13; p++) {
                    if (block_length & (0x1000 >> p)) {
                        strcat(binary, "1");
                    } else {
                        strcat(binary, "0");
                    }
                }
                
                if (debug) {
                    printf("Binary (length %d)\n", block_length);
                }
                
                i = 0;
                
                while (i < block_length) {
                    
                    /* 8-bit bytes with no conversion */
                    for (p = 0; p < 8; p++) {
                        if (source[i + position] & (0x80 >> p)) {
                            strcat(binary, "1");
                        } else {
                            strcat(binary, "0");
                        }
                    }
                    
                    if (debug) {
                        printf("%d ", source[i + position]);
                    }
                    
                    i++;
                }
                
                if (debug) {
                    printf("\n");
                }
                break;
        }
        
        position += block_length;
        
    } while (position < length);
}

/* Han Xin Code - main */
int han_xin(struct zint_symbol *symbol, const unsigned char source[], int length) {
    char mode[length + 1];
    int est_binlen;

    hx_define_mode(mode, source, length);
    
    est_binlen = calculate_binlength(mode, length);
    
    char binary[est_binlen + 10];
    binary[0] = '\0';
    
    calculate_binary(binary, mode, source, length);
    
    printf("Binary: %s\n", binary);
    
    strcpy(symbol->errtxt, "Under Construction!");
    return ZINT_ERROR_INVALID_OPTION;
}