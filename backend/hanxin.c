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

/* Calculate the approximate length of the binary string */
int calculate_binlength(char mode[], const unsigned char source[], int length) {
    int i;
    char lastmode = ' ';
    int est_binlen = 0;
    int submode = 1;
    
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
                    submode = 1;
                }
                if (getsubmode((char) source[i]) != submode) {
                    est_binlen += 6;
                    submode = getsubmode((char) source[i]);
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

/* Finder pattern for top left of symbol */
void hx_place_finder_top_left(unsigned char* grid, int size) {
    int xp, yp;
    int x = 0, y = 0;
    
    int finder[] = {
        1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 0, 0, 0,
        1, 0, 1, 1, 1, 1, 1,
        1, 0, 1, 0, 0, 0, 0,
        1, 0, 1, 0, 1, 1, 1,
        1, 0, 1, 0, 1, 1, 1,
        1, 0, 1, 0, 1, 1, 1
    };
    
    for (xp = 0; xp < 7; xp++) {
        for (yp = 0; yp < 7; yp++) {
            if (finder[xp + (7 * yp)] == 1) {
                grid[((yp + y) * size) + (xp + x)] = 0x11;
            } else {
                grid[((yp + y) * size) + (xp + x)] = 0x10;
            }
        }
    }
}

/* Finder pattern for top right and bottom left of symbol */
void hx_place_finder(unsigned char* grid, int size, int x, int y) {
    int xp, yp;
    
    int finder[] = {
        1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 0, 1,
        0, 0, 0, 0, 1, 0, 1,
        1, 1, 1, 0, 1, 0, 1,
        1, 1, 1, 0, 1, 0, 1,
        1, 1, 1, 0, 1, 0, 1
    };
    
    for (xp = 0; xp < 7; xp++) {
        for (yp = 0; yp < 7; yp++) {
            if (finder[xp + (7 * yp)] == 1) {
                grid[((yp + y) * size) + (xp + x)] = 0x11;
            } else {
                grid[((yp + y) * size) + (xp + x)] = 0x10;
            }
        }
    }
}

/* Finder pattern for bottom right of symbol */
void hx_place_finder_bottom_right(unsigned char* grid, int size) {
    int xp, yp;
    int x = size - 7, y = size - 7;
    
    int finder[] = {
        1, 1, 1, 0, 1, 0, 1,
        1, 1, 1, 0, 1, 0, 1,
        1, 1, 1, 0, 1, 0, 1,
        0, 0, 0, 0, 1, 0, 1,
        1, 1, 1, 1, 1, 0, 1,
        0, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1
    };
    
    for (xp = 0; xp < 7; xp++) {
        for (yp = 0; yp < 7; yp++) {
            if (finder[xp + (7 * yp)] == 1) {
                grid[((yp + y) * size) + (xp + x)] = 0x11;
            } else {
                grid[((yp + y) * size) + (xp + x)] = 0x10;
            }
        }
    }
}

/* Avoid plotting outside symbol or over finder patterns */
void hx_safe_plot(unsigned char *grid, int size, int x, int y, int value) {
    if ((x >= 0) && (x < size)) {
        if ((y >= 0) && (y < size)) {
            if (grid[(y * size) + x] == 0) {
                grid[(y * size) + x] = value;
            }
        }
    }
}

/* Plot an alignment pattern around top and right of a module */
void hx_plot_alignment(unsigned char *grid, int size, int x, int y, int w, int h) {
    int i;
    hx_safe_plot(grid, size, x, y, 0x11);
    hx_safe_plot(grid, size, x - 1, y + 1, 0x10);
    
    for (i = 1; i <= w; i++) {
        /* Top */
        hx_safe_plot(grid, size, x - i, y, 0x11);
        hx_safe_plot(grid, size, x - i - 1, y + 1, 0x10);
    }
    
    for (i = 1; i < h; i++) {
        /* Right */
        hx_safe_plot(grid, size, x, y + i, 0x11);
        hx_safe_plot(grid, size, x - 1, y + i + 1, 0x10);
    }
}

/* Plot assistany alignment patterns */
void hx_plot_assistant(unsigned char *grid, int size, int x, int y) {
    hx_safe_plot(grid, size, x - 1, y - 1, 0x10);
    hx_safe_plot(grid, size, x, y - 1, 0x10);
    hx_safe_plot(grid, size, x + 1, y - 1, 0x10);
    hx_safe_plot(grid, size, x - 1, y, 0x10);
    hx_safe_plot(grid, size, x, y, 0x11);
    hx_safe_plot(grid, size, x + 1, y, 0x10);
    hx_safe_plot(grid, size, x - 1, y + 1, 0x10);
    hx_safe_plot(grid, size, x, y + 1, 0x10);
    hx_safe_plot(grid, size, x + 1, y + 1, 0x10);    
}

/* Put static elements in the grid */
void hx_setup_grid(unsigned char* grid, int size, int version) {
    int i, j;
    
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            grid[(i * size) + j] = 0;
        }
    }
    
    /* Add finder patterns */
    hx_place_finder_top_left(grid, size);
    hx_place_finder(grid, size, 0, size - 7);
    hx_place_finder(grid, size, size - 7, 0);
    hx_place_finder_bottom_right(grid, size);
    
    /* Add finder pattern separator region */
    for (i = 0; i < 8; i++) {
        /* Top left */
        grid[(7 * size) + i] = 0x10;
        grid[(i * size) + 7] = 0x10;
        
         /* Top right */
        grid[(7 * size) + (size - i - 1)] = 0x10;       
        grid[((size - i - 1) * size) + 7] = 0x10;
       
        /* Bottom left */
        grid[(i * size) + (size - 8)] = 0x10;
        grid[((size - 8) * size) + i] = 0x10;
        
        /* Bottom right */
        grid[((size - 8) * size) + (size - i - 1)] = 0x10;
        grid[((size - i - 1) * size) + (size - 8)] = 0x10;
    }
    
    /* Reserve function information region */
    for (i = 0; i < 9; i++) {
        /* Top left */
        grid[(8 * size) + i] = 0x10;
        grid[(i * size) + 8] = 0x10;
        
         /* Top right */
        grid[(8 * size) + (size - i - 1)] = 0x10;       
        grid[((size - i - 1) * size) + 8] = 0x10;
       
        /* Bottom left */
        grid[(i * size) + (size - 9)] = 0x10;
        grid[((size - 9) * size) + i] = 0x10;
        
        /* Bottom right */
        grid[((size - 9) * size) + (size - i - 1)] = 0x10;
        grid[((size - i - 1) * size) + (size - 9)] = 0x10;
    }
    
    if (version > 3) {
        int k = hx_module_k[version - 1];
        int r = hx_module_r[version - 1];
        int m = hx_module_m[version - 1];
        int x, y, row_switch, column_switch;
        int module_height, module_width;
        int mod_x, mod_y;
        
        /* Add assistant alignment patterns to left and right */
        y = 0;
        mod_y = 0;
        do {
            if (mod_y < m) {
                module_height = k;
            } else {
                module_height = r - 1;
            }
            
            if ((mod_y % 2) == 0) {
                if ((m % 2) == 1) {
                    hx_plot_assistant(grid, size, 0, y);
                }
            } else {
                if ((m % 2) == 0) {
                    hx_plot_assistant(grid, size, 0, y);
                }
                hx_plot_assistant(grid, size, size - 1, y);
            }
            
            mod_y++;
            y += module_height;
        } while (y < size);
        
        /* Add assistant alignment patterns to top and bottom */
        x = (size - 1);
        mod_x = 0;
        do {
            if (mod_x < m) {
                module_width = k;
            } else {
                module_width = r - 1;
            }
            
            if ((mod_x % 2) == 0) {
                if ((m % 2) == 1) {
                    hx_plot_assistant(grid, size, x, (size - 1));
                }
            } else {
                if ((m % 2) == 0) {
                    hx_plot_assistant(grid, size, x, (size - 1));
                }
                hx_plot_assistant(grid, size, x, 0);
            }
            
            mod_x++;
            x -= module_width;
        } while (x >= 0);
        
        /* Add alignment pattern */
        column_switch = 1;
        y = 0;
        mod_y = 0;        
        do {
            if (mod_y < m) {
                module_height = k;
            } else {
                module_height = r - 1;
            }
            
            if (column_switch == 1) {
                row_switch = 1;
                column_switch = 0;
            } else {
                row_switch = 0;
                column_switch = 1;
            }
            
            x = (size - 1);
            mod_x = 0;
            do {
                if (mod_x < m) {
                    module_width = k;
                } else {
                    module_width = r - 1;
                }
                
                if (row_switch == 1) {
                    if (!(y == 0 && x == (size - 1))) {
                        hx_plot_alignment(grid, size, x, y, module_width, module_height);
                    }
                    row_switch = 0;
                } else {
                    row_switch = 1;
                }
                mod_x++;
                x -= module_width;
            } while (x >= 0);
            
            mod_y++;
            y += module_height;
        } while (y < size);
    }
}

/* Han Xin Code - main */
int han_xin(struct zint_symbol *symbol, const unsigned char source[], int length) {
    char mode[length + 1];
    int est_binlen;
    int ecc_level = 1;
    int i, j, version;
    int target_binlen, size;

    hx_define_mode(mode, source, length);
    
    est_binlen = calculate_binlength(mode, source, length);
    
    char binary[est_binlen + 10];
    binary[0] = '\0';
    
    calculate_binary(binary, mode, source, length);
    
    version = 85;
    for (i = 84; i > 0; i--) {
        switch (ecc_level) {
            case 1:
                if ((hx_data_codewords_L1[i - 1] * 8) > est_binlen) {
                    version = i;
                    target_binlen = hx_data_codewords_L1[i - 1] * 8;
                }
                break;
            case 2:
                if ((hx_data_codewords_L2[i - 1] * 8) > est_binlen) {
                    version = i;
                    target_binlen = hx_data_codewords_L2[i - 1] * 8;
                }
                break;
            case 3:
                if ((hx_data_codewords_L3[i - 1] * 8) > est_binlen) {
                    version = i;
                    target_binlen = hx_data_codewords_L3[i - 1] * 8;
                }
                break;
            case 4:
                if ((hx_data_codewords_L4[i - 1] * 8) > est_binlen) {
                    version = i;
                    target_binlen = hx_data_codewords_L4[i - 1] * 8;
                }
                break;
        }
    }
    
    if (version == 85) {
        strcpy(symbol->errtxt, "Input too long for selected error correction level");
        return ZINT_ERROR_TOO_LONG;
    }
    
    size = (version * 2) + 21;
    
#ifndef _MSC_VER
    int datastream[target_binlen + 1];
    int fullstream[hx_total_codewords[version - 1] + 1];
    unsigned char grid[size * size];
#else
    datastream = (int *) _alloca((target_binlen + 1) * sizeof (int));
    fullstream = (int *) _alloca((hx_total_codewords[version - 1] + 1) * sizeof (int));
    grid = (unsigned char *) _alloca((size * size) * sizeof (unsigned char));
#endif
    
    hx_setup_grid(grid, size, version);
    
    printf("Binary: %s\n", binary);
    
    symbol->width = size;
    symbol->rows = size;

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            if (grid[(i * size) + j] & 0x01) {
                set_module(symbol, i, j);
            }
        }
        symbol->row_height[i] = 1;
    }
    
    return 0;
}