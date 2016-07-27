/* dotcode.c - Handles DotCode */

/*
    libzint - the open source barcode library
    Copyright (C) 2016 Robin Stuart <rstuart114@gmail.com>

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

/* 
 * Attempts to encode DotCode according to AIMD013 Rev 1.34a, dated Feb 19, 2009
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifndef _MSC_VER
#include <stdint.h>
#else
#include <malloc.h>
#include "ms_stdint.h"
#endif
#include "common.h"
#include "gs1.h"

/*
static const char *C128Table[107] = {
    // Code 128 character encodation
    "212222", "222122", "222221", "121223", "121322", "131222", "122213",
    "122312", "132212", "221213", "221312", "231212", "112232", "122132", "122231", "113222",
    "123122", "123221", "223211", "221132", "221231", "213212", "223112", "312131", "311222",
    "321122", "321221", "312212", "322112", "322211", "212123", "212321", "232121", "111323",
    "131123", "131321", "112313", "132113", "132311", "211313", "231113", "231311", "112133",
    "112331", "132131", "113123", "113321", "133121", "313121", "211331", "231131", "213113",
    "213311", "213131", "311123", "311321", "331121", "312113", "312311", "332111", "314111",
    "221411", "431111", "111224", "111422", "121124", "121421", "141122", "141221", "112214",
    "112412", "122114", "122411", "142112", "142211", "241211", "221114", "413111", "241112",
    "134111", "111242", "121142", "121241", "114212", "124112", "124211", "411212", "421112",
    "421211", "212141", "214121", "412121", "111143", "111341", "131141", "114113", "114311",
    "411113", "411311", "113141", "114131", "311141", "411131", "211412", "211214", "211232",
    "2331112"
};
*/

#define GF 113
#define PM 3
//-------------------------------------------------------------------------
// "rsencode(nd,nc)" adds "nc" R-S check words to "nd" data words in wd[]
// employing Galois Field GF, where GF is prime, with a prime modulus of PM
//-------------------------------------------------------------------------
void rsencode (int nd, int nc, unsigned char *wd) {
    int i, j, k, nw, start, step, root[GF], c[GF];
    
    // Start by generating "nc" roots (antilogs):
    root[0] = 1;
    for (i=1; i<=nc; i++)
        root[i] = (PM * root[i-1]) % GF;
    
    // Here we compute how many interleaved R-S blocks will be needed
    nw = nd + nc; step = (nw + GF - 2)/(GF - 1);
    
    // ...& then for each such block:
    for (start=0; start<step; start++) {
        int ND = (nd-start+step-1)/step, NW = (nw-start+step-1)/step, NC = NW-ND;
        
        // first compute the generator polynomial "c" of order "NC":
        for (i=1; i<=NC; i++)
            c[i] = 0; c[0] = 1;
            
        for (i=1; i<=NC; i++) {
            for (j=NC; j>=1; j--) {
                c[j] = (GF + c[j] - (root[i] * c[j-1]) % GF) % GF;
            }
        }
            
        // & then compute the corresponding checkword values into wd[]
        // ... (a) starting at wd[start] & (b) stepping by step
        for (i=ND; i<NW; i++)
            wd[start+i*step] = 0;
        for (i=0; i<ND; i++) {
            k = (wd[start+i*step] + wd[start+ND*step]) % GF;
            for (j=0; j<NC-1; j++) {
                wd[start+(ND+j)*step] = (GF - ((c[j+1] * k) % GF) + wd[start+(ND+j+1)*step]) % GF;
            }
            wd[start+(ND+NC-1)*step] = (GF - ((c[NC] * k) % GF)) % GF;
        }
    for (i=ND; i<NW; i++)
        wd[start+i*step] = (GF - wd[start+i*step]) % GF;
    }
}

/* Check if the next character is directly encodable in code set A (Annex F.II.D) */
int datum_a(unsigned char source[], int position, int length) {
    int retval = 0;
    
    if (position < length) {
        if (source[position] <= 95) {
            retval = 1;
        }
    }
    
    return retval;
}

/* Check if the next character is directly encodable in code set B (Annex F.II.D) */
int datum_b(unsigned char source[], int position, int length) {
    int retval = 0;
    
    if (position < length) {
        if (source[position] >= 32) {
            retval = 1;
        }

        switch(source[position]) {
            case 9: // HT
            case 28: // FS
            case 29: // GS
            case 30: // RS
                retval = 1;
        }

        if (position != length - 2) {
            if ((source[position] == 13) && (source[position + 1] == 10)) { // CRLF
                retval = 1;
            }
        }
    }
    
    return retval;
}

/* Check if the next characters are directly encodable in code set C (Annex F.II.D) */
int datum_c(unsigned char source[], int position, int length) {
    int retval = 0;
    
    if (position < length - 2) {
        if (((source[position] >= '0') && (source[position] <= '9'))
                && ((source[position + 1] >= '0') && (source[position + 1] <= '9')))
            retval = 1;
    }
    
    return retval;
}

/* Returns how many consecutive digits lie immediately ahead (Annex F.II.A) */
int n_digits(unsigned char source[], int position, int length) {
    int i;
    
    for(i = position; ((source[i] >= '0') && (source[i] <= '9')) && (i < length); i++);
    
    return i - position;
}

/* checks ahead for 10 or more digits starting "17xxxxxx10..." (annex F.II.B) */
int seventeen_ten(unsigned char source[], int position, int length) {
    int found = 0;
    
    if(n_digits(source, position, length) >= 10) {
        if(((source[position] == '1') && (source[position + 1] == '7'))
                && ((source[position + 8] == '1') && (source[position + 9] == '0'))) {
            found = 1;
        }
    }
    
    return found;
}

/*  checks how many characters ahead can be reached while datum_c is true,
 *  returning the resulting number of codewords (Annex F.II.E)
 */
int ahead_c(unsigned char source[], int position, int length) {
    int count = 0;
    
    for(int i = position; (i < length) && datum_c(source, i, length); i+= 2) {
        count++;
    }
    
    return count;
}

/* Annex F.II.F */
int try_c(unsigned char source[], int position, int length) {
    int retval = 0;
    
    if(n_digits(source, position, length) > 0) {
        if(ahead_c(source, position, length) > ahead_c(source, position + 1, length)) {
            retval = ahead_c(source, position, length);
        }
    }
    
    return retval;
}

/* Annex F.II.G */
int ahead_a(unsigned char source[], int position, int length) {
    int count = 0;
    
    for(int i = position; ((i < length) && datum_a(source, i, length)) 
            && (try_c(source, i, length) < 2); i++) {
        count++;
    }
    
    return count;
}

/* Annex F.II.H */
int ahead_b(unsigned char source[], int position, int length) {
    int count = 0;
    
    for(int i = position; ((i < length) && datum_b(source, i, length)) 
            && (try_c(source, i, length) < 2); i++) {
        count++;
    }
    
    return count;
}

/* checks if the next character is in the range 128 to 255  (Annex F.II.I) */
int binary(unsigned char source[], int position, int length) {
    int retval = 0;
    
    if(source[position] >= 128) {
        retval = 1;
    }
    
    return retval;
}

int dotcode_encode_message(struct zint_symbol *symbol, unsigned char source[], int length, unsigned char *codeword_array) {
    int input_position, array_length, i;
    char encoding_mode;
    int inside_macro, done;
    int debug = 0;
    int binary_buffer_size = 0;
    int lawrencium[6]; // Reversed radix 103 values
    
#if defined(_MSC_VER) && _MSC_VER == 1200
    uint64_t binary_buffer = 0;
#else
    uint64_t binary_buffer = 0ULL;
#endif
    
    /* Analyse input data stream and encode using algorithm from Annex F */
    input_position = 0;
    array_length = 0;
    encoding_mode = 'C';
    inside_macro = 0;
    
    if (symbol->output_options & READER_INIT) {
        codeword_array[array_length] = 109; // FNC3
        array_length++;
    }
    
    if (symbol->input_mode != GS1_MODE) {
        codeword_array[array_length] = 107; // FNC1
        array_length++;
    }
    
    do {
        done = 0;
        
        /* Step A */
        if ((input_position == length - 2) && (inside_macro != 0) && (inside_macro != 100)) {
            // inside_macro only gets set to 97, 98 or 99 if the last two characters are RS/EOT
            input_position += 2;
            done = 1;
            if (debug) { printf("A "); }
        }
                
        if ((input_position == length - 1) && (inside_macro == 100)) {
            // inside_macro only gets set to 100 if the last character is EOT
            input_position++;
            done = 1;
            if (debug) { printf("A "); }
        }
        
        /* Step B1 */
        if ((!done) && (encoding_mode == 'C')) {
            if ((array_length == 0) && (length > 9)) {
                if((source[input_position] == '[') 
                        && (source[input_position + 1] == ')')
                        && (source[input_position + 2] == '>')
                        && (source[input_position + 3] == 30) // RS
                        && (source[length - 1] == 04)) {  // EOT
                    
                    codeword_array[array_length] = 106; // Latch B
                    array_length++;
                    encoding_mode = 'B';
                    
                    if ((source[input_position + 6] == 29) && (source[length - 2] == 30)) { // GS/RS
                        if ((source[input_position + 4] == '0') && (source[input_position + 5] == '5')) {
                            codeword_array[array_length] = 97; // Macro
                            array_length++;
                            input_position += 7;
                            inside_macro = 97;
                            done = 1;
                            if (debug) { printf("B1/1 "); }
                        }

                        if ((source[input_position + 4] == '0') && (source[input_position + 5] == '6')) {
                            codeword_array[array_length] = 98; // Macro
                            array_length++;
                            input_position += 7;
                            inside_macro = 98;
                            done = 1;
                            if (debug) { printf("B1/2 "); }
                        }

                        if ((source[input_position + 4] == '1') && (source[input_position + 5] == '2')) {
                            codeword_array[array_length] = 99; // Macro
                            array_length++;
                            input_position += 7;
                            inside_macro = 99;
                            done = 1;
                            if (debug) { printf("B1/3 "); }
                        }
                    }
                    
                    if (!done) {
                        codeword_array[array_length] = 100; // Macro
                        array_length++;
                        input_position += 4;
                        inside_macro = 100;
                        done = 1;
                        if (debug) { printf("B1/4 "); }
                    }
                }
            }
        }
        
        /* Step B2 */
        if ((!done) && (encoding_mode == 'C')) {
            if (seventeen_ten(source, input_position, length)) {
                codeword_array[array_length] = 100; // (17)...(10)
                array_length++;
                codeword_array[array_length] = ((source[input_position + 2] - '0') * 10) + (source[input_position + 3] - '0');
                array_length++;
                codeword_array[array_length] = ((source[input_position + 4] - '0') * 10) + (source[input_position + 5] - '0');
                array_length++;
                codeword_array[array_length] = ((source[input_position + 6] - '0') * 10) + (source[input_position + 7] - '0');
                array_length++;
                input_position += 10;
                done = 1;
                if (debug) { printf("B2/1 "); }
            }
        }
        
        if ((!done) && (encoding_mode == 'C')) {
            if (datum_c(source, input_position, length) || ((source[input_position] == '[') && (symbol->input_mode == GS1_MODE))) {
                if (source[input_position] == '[') { 
                    codeword_array[array_length] = 107; // FNC1
                    input_position++;
                } else {
                    codeword_array[array_length] = ((source[input_position] - '0') * 10) + (source[input_position + 1] - '0');
                    input_position += 2;
                }
                array_length++;
                done = 1;
                if (debug) { printf("B2/2 "); }
            }
        }
        
        /* Setp B3 */
        if ((!done) && (encoding_mode == 'C')) {
            if (binary(source, input_position, length)) {
                if (n_digits(source, input_position + 1, length) > 0) {
                    if ((source[input_position]  - 128) < 32) {
                        codeword_array[array_length] = 110; // Bin Shift A
                        array_length++;
                        codeword_array[array_length] = source[input_position] - 128 + 64;
                        array_length++;
                    } else {
                        codeword_array[array_length] = 111; // Bin Shift B
                        array_length++;
                        codeword_array[array_length] = source[input_position] - 128 - 32;
                        array_length++;
                    }
                    input_position++;
                } else {
                    codeword_array[array_length] = 112; // Bin Latch
                    array_length++;
                    encoding_mode = 'X';
                } 
                done = 1;
                if (debug) { printf("B3 "); }
            }
        }
        
        /* Step B4 */
        if ((!done) && (encoding_mode == 'C')) {
            int m = ahead_a(source, input_position, length);
            int n = ahead_b(source, input_position, length);
            if (m > n) {
                codeword_array[array_length] = 101; // Latch A
                array_length++;
                encoding_mode = 'A';
            } else {
                if (n <= 4) {
                    codeword_array[array_length] = 101 + n; // nx Shift B
                    array_length++;
                    
                    for(i = 0; i < n; i++) {
                        codeword_array[array_length] = source[input_position] - 32;
                        array_length++;
                        input_position++;
                    }
                } else {
                    codeword_array[array_length] = 106; // Latch B
                    array_length++;
                    encoding_mode = 'B';
                }
            }
            done = 1;
            if (debug) { printf("B4 "); }
        }
        
        /* Step C1 */
        if ((!done) && (encoding_mode == 'B')) {
            int n = try_c(source, input_position, length);
            
            if (n >= 2) {
                if (n <= 4) {
                    codeword_array[array_length] = 103 + (n - 2); // nx Shift C
                    array_length++;
                    for(i = 0; i < n; i++) {
                        codeword_array[array_length] = ((source[input_position] - '0') * 10) + (source[input_position + 1] - '0');
                        array_length++;
                        input_position += 2;
                    }
                } else {
                    codeword_array[array_length] = 106; // Latch C
                    array_length++;
                    encoding_mode = 'C';
                }
                done = 1;
                if (debug) { printf("C1 "); }
            }
        }
        
        /* Step C2 */
        if ((!done) && (encoding_mode == 'B')) {
            if ((source[input_position] == '[') && (symbol->input_mode == GS1_MODE)) {
                codeword_array[array_length] = 107; // FNC1
                array_length++;
                input_position++;
                done = 1;
                if (debug) { printf("C2/1 "); }
            } else {
                if (datum_b(source, input_position, length)) {
                    codeword_array[array_length] = source[input_position] - 32;
                    array_length++;
                    input_position++;
                    done = 1;
                    if (debug) { printf("C2/2 "); }
                }
            }
        }
        
        /* Step C3 */
        if ((!done) && (encoding_mode == 'B')) {
            if (binary(source, input_position, length)) {
                if (datum_b(source, input_position + 1, length)) {
                    if ((source[input_position]  - 128) < 32) {
                        codeword_array[array_length] = 110; // Bin Shift A
                        array_length++;
                        codeword_array[array_length] = source[input_position] - 128 + 64;
                        array_length++;
                    } else {
                        codeword_array[array_length] = 111; // Bin Shift B
                        array_length++;
                        codeword_array[array_length] = source[input_position] - 128 - 32;
                        array_length++;
                    }
                    input_position++;
                } else {
                    codeword_array[array_length] = 112; // Bin Latch
                    array_length++;
                    encoding_mode = 'X';
                }
                done = 1;
                if (debug) { printf("C3 "); }
            }
        }
        
        /* Step C4 */
        if ((!done) && (encoding_mode == 'B')) {
            if (ahead_a(source, input_position, length) == 1) {
                codeword_array[array_length] = 101; // Shift A
                array_length++;
                if (source[input_position] < 32) {
                    codeword_array[array_length] = source[input_position] + 64;
                } else {
                    codeword_array[array_length] = source[input_position] - 32;
                }
                array_length++;
                input_position++;
            } else {
                codeword_array[array_length] = 102; // Latch A
                array_length++;
                encoding_mode = 'A';
            }
            done = 1;
            if (debug) { printf("C4 "); }
        }
        
        /* Step D1 */
        if ((!done) && (encoding_mode == 'A')) {
            int n = try_c(source, input_position, length);
            if (n >= 2) {
                if (n <= 4) {
                    codeword_array[array_length] = 103 + (n - 2); // nx Shift C
                    array_length++;
                    for(i = 0; i < n; i++) {
                        codeword_array[array_length] = ((source[input_position] - '0') * 10) + (source[input_position + 1] - '0');
                        array_length++;
                        input_position += 2;
                    }
                } else {
                    codeword_array[array_length] = 106; // Latch C
                    array_length++;
                    encoding_mode = 'C';
                }
                done = 1;
                if (debug) { printf("D1 "); }
            }
        }
        
        /* Step D2 */
        if ((!done) && (encoding_mode == 'A')) {
            if ((source[input_position] == '[') && (symbol->input_mode == GS1_MODE)) {
                codeword_array[array_length] = 107; // FNC1
                array_length++;
                input_position++;
                done = 1;
                if (debug) { printf("D2/1 "); }
            } else {
                if (datum_a(source, input_position, length)) {
                    if (source[input_position] < 32) {
                        codeword_array[array_length] = source[input_position] +64;
                    } else {
                        codeword_array[array_length] = source[input_position] - 32;
                    }
                    array_length++;
                    input_position++;
                    done = 1;
                    if (debug) { printf("D2/2 "); }
                }
            }
        }
    
        /* Step D3 */
        if ((!done) && (encoding_mode == 'A')) {
            if (binary(source, input_position, length)) {
                if (datum_a(source, input_position + 1, length)) {
                    if ((source[input_position]  - 128) < 32) {
                        codeword_array[array_length] = 110; // Bin Shift A
                        array_length++;
                        codeword_array[array_length] = source[input_position] - 128 + 64;
                        array_length++;
                    } else {
                        codeword_array[array_length] = 111; // Bin Shift B
                        array_length++;
                        codeword_array[array_length] = source[input_position] - 128 - 32;
                        array_length++;
                    }
                    input_position++;
                } else {
                    codeword_array[array_length] = 112; // Bin Latch
                    array_length++;
                    encoding_mode = 'X';
                }
                done = 1;
                if (debug) { printf("D3 "); }
            }
        }
        
        /* Step D4 */
        if ((!done) && (encoding_mode == 'A')) {
            int n = ahead_b(source, input_position, length);
            
            if (n <= 6) {
                codeword_array[array_length] = 95 + n; // nx Shift B
                array_length++;
                for(i = 0; i < n; i++) {
                    codeword_array[array_length] = source[input_position] - 32;
                    array_length++;
                    input_position++;
                }
            } else {
                codeword_array[array_length] = 102; // Latch B
                array_length++;
                encoding_mode = 'B';
            }
            done = 1;
            if (debug) { printf("D4 "); }
        }
        
        /* Step E1 */
        if ((!done) && (encoding_mode == 'X')) {
            int n = try_c(source, input_position, length);
            
            if (n >= 2) {
                /* Empty binary buffer */
                for(i = 0; i < (binary_buffer_size + 1); i++) {
                    lawrencium[i] = binary_buffer % 103;
                    binary_buffer /= 103;
                }

                for(i = 0; i < (binary_buffer_size + 1); i++) {
                    codeword_array[array_length] = lawrencium[binary_buffer_size - i];
                    array_length++;
                }
                binary_buffer = 0;
                binary_buffer_size = 0;
                
                if (n <= 7) {
                    codeword_array[array_length] = 101 + n; // Interrupt for nx Shift C
                    array_length++;
                    for(i = 0; i < n; i++) {
                        codeword_array[array_length] = ((source[input_position] - '0') * 10) + (source[input_position + 1] - '0');
                        array_length++;
                        input_position += 2;
                    }
                } else {
                    codeword_array[array_length] = 111; // Terminate with Latch to C
                    array_length++;
                    encoding_mode = 'C';
                }
                done = 1;
                if (debug) { printf("E1 "); }
            }
        }
        
        /* Step E2 */
        /* Section 5.2.1.1 para D.2.i states:
         * "Groups of six codewords, each valued between 0 and 102, are radix converted from
         * base 103 into five base 259 values..."
         */
        if ((!done) && (encoding_mode == 'X')) {
            if(binary(source, input_position, length)
                    || binary(source, input_position + 1, length)
                    || binary(source, input_position + 2, length)
                    || binary(source, input_position + 3, length)) {
                binary_buffer *= 259;
                binary_buffer += source[input_position];
                binary_buffer_size++;
                
                if (binary_buffer_size == 5) {
                    for(i = 0; i < 6; i++) {
                        lawrencium[i] = binary_buffer % 103;
                        binary_buffer /= 103;
                    }
                    
                    for(i = 0; i < 6; i++) {
                        codeword_array[array_length] = lawrencium[5 - i];
                        array_length++;
                    }
                    binary_buffer = 0;
                    binary_buffer_size = 0;
                }
                input_position++;
                done = 1;
                if (debug) { printf("E2 "); }
            }
        }
        
        /* Step E3 */
        if ((!done) && (encoding_mode == 'X')) {
            /* Empty binary buffer */
            for(i = 0; i < (binary_buffer_size + 1); i++) {
                lawrencium[i] = binary_buffer % 103;
                binary_buffer /= 103;
            }

            for(i = 0; i < (binary_buffer_size + 1); i++) {
                codeword_array[array_length] = lawrencium[binary_buffer_size - i];
                array_length++;
            }
            binary_buffer = 0;
            binary_buffer_size = 0;
                    
            if (ahead_a(source, input_position, length) > ahead_b(source, input_position, length)) {
                codeword_array[array_length] = 109; // Terminate with Latch to A
                encoding_mode = 'A';
            } else {
                codeword_array[array_length] = 110; // Terminate with Latch to B
                encoding_mode = 'B';
            }
            array_length++;
            done = 1;
            if (debug) { printf("E3 "); }
        }
    } while (input_position < length);
    
    if (debug) { printf("\n\n"); }
    
    return array_length;
}

int dotcode(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i, j;
    int data_length, ecc_length;
    int min_dots, n_dots;
    int height, width, pad_chars;
    int mask_score[4];
    int weight;
    
    /* Test data */
/*
    symbol->input_mode = GS1_MODE;
    length = 15;
    source[0] = '0';
    source[1] = '2';
    source[2] = '[';
    source[3] = 0x80;
    source[4] = 0xd0;
    source[5] = 0x20;
    source[6] = 0xd2;
    source[7] = 0x00;
    source[8] = 0x00;
    source[9] = 0x00;
    source[10] = 0x00;
    source[11] = 48;
    source[12] = 0xcc;
    source[13] = 49;
    source[14] = 0x1f;
*/
    
#ifndef _MSC_VER
    unsigned char codeword_array[length * 3];
    unsigned char masked_codeword_array[length * 3];
#else
    unsigned char* codeword_array = (unsigned char *) _alloca(length * 3 * sizeof(unsigned char));
    unsigned char* masked_codeword_array = (unsigned char *) _alloca(length * 3 * sizeof(unsigned char));
#endif /* _MSC_VER */
    
    data_length = dotcode_encode_message(symbol, source, length, codeword_array);
    
    ecc_length = 3 + (data_length / 2);
    
    printf("Codeword length = %d, ECC length = %d\n", data_length, ecc_length);
    
    min_dots = 9 * (data_length + 3 + (data_length / 2)) + 2;
    printf("Min Dots %d\n", min_dots);
    
    //FIXME: Listen to user preferences here
    height = sqrt(2 * min_dots);
    if ((height % 2) == 1) {
        height++;
    }
    
    width = (2 * min_dots) / height;
    
    if ((width % 2) != 1) {
        width++;
    }
    
    n_dots = (height * width) / 2;
    
    /* Add pad characters */
    for(pad_chars = 0; 9 * ((data_length + pad_chars + 3 + ((data_length + pad_chars) / 2)) + 2) < n_dots; pad_chars++);
    
    printf("Pad characters %d\n", pad_chars);
    
    if (pad_chars > 0) {
        codeword_array[data_length] = 109; // Latch to Code Set A
        data_length++;
        pad_chars--;
    }
    
    for (i = 0; i < pad_chars; i++) {
        codeword_array[data_length] = 106; // Pad
        data_length++;
    }
    
    ecc_length = 3 + (data_length / 2);
    
    /* Evaluate data mask options */
    for (i = 0; i < 4; i++) {
        switch(i) {
            case 0:
                masked_codeword_array[0] = 0;
                for(j = 0; j < data_length; j++) {
                    masked_codeword_array[j + 1] = codeword_array[j];
                }
                printf("Masked Data codewords: ");
                for (j = 0; j <= data_length; j++) {
                    printf(" %d ", (int) masked_codeword_array[j]);
                }
                printf("\n");
                break;
            case 1:
                weight = 0;
                masked_codeword_array[0] = 1;
                for(j = 0; j < data_length; j++) {
                    masked_codeword_array[j + 1] = (weight + codeword_array[j]) % 113;
                    weight += 3;
                }
                printf("Masked Data codewords: ");
                for (j = 0; j <= data_length; j++) {
                    printf(" %d ", (int) masked_codeword_array[j]);
                }
                printf("\n");
                break;
            case 2:
                weight = 0;
                masked_codeword_array[0] = 2;
                for(j = 0; j < data_length; j++) {
                    masked_codeword_array[j + 1] = (weight + codeword_array[j]) % 113;
                    weight += 7;
                }
                printf("Masked Data codewords: ");
                for (j = 0; j <= data_length; j++) {
                    printf(" %d ", (int) masked_codeword_array[j]);
                }
                printf("\n");
                break;
            case 3:
                weight = 0;
                masked_codeword_array[0] = 3;
                for(j = 0; j < data_length; j++) {
                    masked_codeword_array[j + 1] = (weight + codeword_array[j]) % 113;
                    weight += 17;
                }                
                printf("Masked Data codewords: ");
                for (j = 0; j <= data_length; j++) {
                    printf(" %d ", (int) masked_codeword_array[j]);
                }
                printf("\n");
                break;
        }
        
        rsencode(data_length + 1, ecc_length, masked_codeword_array);
        
        printf("Full code stream: ");
        for (j = 0; j < (data_length + ecc_length + 1); j++) {
            printf("%d ", (int) masked_codeword_array[j]);
        }
        printf("\n");
        
    }
    
    printf("Proposed size = height %d, width %d, (total usable dots %d)\n", height, width, n_dots);
    
    return ZINT_ERROR_INVALID_OPTION;
}