/* pcx.c - Handles output to ZSoft PCX file */

/*
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "pcx.h"        /* PCX header structure */
#include <math.h>

#define SSET	"0123456789ABCDEF"

int pcx_pixel_plot(struct zint_symbol *symbol, int image_height, int image_width, char *pixelbuf, int rotate_angle) {
    int fgred, fggrn, fgblu, bgred, bggrn, bgblu;
    int errno;
    int row, column, i, colour;
    int run_count;
    FILE *pcx_file;
    
#ifndef _MSC_VER
    char rotated_bitmap[image_height * image_width];
#else
    char* rotated_bitmap = (char *) _alloca((image_height * image_width) * sizeof(char));
#endif /* _MSC_VER */

    switch (rotate_angle) {
        case 0:
        case 180:
            symbol->bitmap_width = image_width;
            symbol->bitmap_height = image_height;
            break;
        case 90:
        case 270:
            symbol->bitmap_width = image_height;
            symbol->bitmap_height = image_width;
            break;
    }

#ifndef _MSC_VER
    unsigned char rle_row[symbol->bitmap_width];
#else
    unsignd char* rle_row = (unsigned char *) _alloca((symbol->bitmap_width * 6) * sizeof(unsigned char));
#endif /* _MSC_VER */
    
    /* sort out colour options */
    to_upper((unsigned char*) symbol->fgcolour);
    to_upper((unsigned char*) symbol->bgcolour);

    if (strlen(symbol->fgcolour) != 6) {
        strcpy(symbol->errtxt, "Malformed foreground colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }
    if (strlen(symbol->bgcolour) != 6) {
        strcpy(symbol->errtxt, "Malformed background colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }
    errno = is_sane(SSET, (unsigned char*) symbol->fgcolour, strlen(symbol->fgcolour));
    if (errno == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "Malformed foreground colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }
    errno = is_sane(SSET, (unsigned char*) symbol->bgcolour, strlen(symbol->fgcolour));
    if (errno == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "Malformed background colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }

    fgred = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
    fggrn = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
    fgblu = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
    bgred = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
    bggrn = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
    bgblu = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);
    
    /* Rotate image before plotting */
    switch (rotate_angle) {
        case 0: /* Plot the right way up */
            for (row = 0; row < image_height; row++) {
                for (column = 0; column < image_width; column++) {
                    rotated_bitmap[(row * symbol->bitmap_width) + column] =
                            *(pixelbuf + (image_width * row) + column);
                }
            }
            break;
        case 90: /* Plot 90 degrees clockwise */
            for (row = 0; row < image_width; row++) {
                for (column = 0; column < image_height; column++) {
                    rotated_bitmap[(row * image_height) + column] =
                            *(pixelbuf + (image_width * (image_height - column - 1)) + row);
                }
            }
            break;
        case 180: /* Plot upside down */
            for (row = 0; row < image_height; row++) {
                for (column = 0; column < image_width; column++) {
                    rotated_bitmap[(row * image_width) + column] =
                            *(pixelbuf + (image_width * (image_height - row - 1)) + (image_width - column - 1));
                }
            }
            break;
        case 270: /* Plot 90 degrees anti-clockwise */
            for (row = 0; row < image_width; row++) {
                for (column = 0; column < image_height; column++) {
                    rotated_bitmap[(row * image_height) + column] =
                            *(pixelbuf + (image_width * column) + (image_width - row - 1));
                }
            }
            break;
    }
    
    pcx_header_t header;
    
    header.manufacturer = 10; // ZSoft
    header.version = 5; // Version 3.0
    header.encoding = 1; // Run length encoding
    header.bits_per_pixel = 8;
    header.window_xmin = 0;
    header.window_ymin = 0;
    header.window_xmax = symbol->bitmap_width - 1;
    header.window_ymax = symbol->bitmap_height - 1;
    header.horiz_dpi = 300;
    header.vert_dpi = 300;
    
    for(i = 0; i < 48; i++) {
        header.colourmap[i] = 0x00;
    }
    
    header.reserved = 0;
    header.number_of_planes = 3;
    
    if (symbol->bitmap_width % 2) {
        header.bytes_per_line = symbol->bitmap_width + 1;
    } else {
        header.bytes_per_line = symbol->bitmap_width;
    }
    
    header.palette_info = 1; // Colour
    header.horiz_screen_size = 0;
    header.vert_screen_size = 0;
    
    for(i = 0; i < 54; i++) {
        header.filler[i] = 0x00;
    }
    
    /* Open output file in binary mode */
    if ((symbol->output_options & BARCODE_STDOUT) != 0) {
#ifdef _MSC_VER
        if (-1 == _setmode(_fileno(stdout), _O_BINARY)) {
            strcpy(symbol->errtxt, "Can't open output file");
            return ZINT_ERROR_FILE_ACCESS;
        }
#endif
        pcx_file = stdout;
    } else {
        if (!(pcx_file = fopen(symbol->outfile, "wb"))) {
            strcpy(symbol->errtxt, "Can't open output file");
            return ZINT_ERROR_FILE_ACCESS;
        }
    }
    
    fwrite(&header, sizeof(pcx_header_t), 1, pcx_file);
    
    for(row = 0; row < symbol->bitmap_height; row++) {
        for (colour = 0; colour < 3; colour++) {
            for (column = 0; column < symbol->bitmap_width; column++) {
                switch(colour) {
                    case 0:
                        if (rotated_bitmap[(row * symbol->bitmap_width) + column] == '1') {
                            rle_row[column] = fgred;
                        } else {
                            rle_row[column] = bgred;
                        }
                        break;
                    case 1:
                        if (rotated_bitmap[(row * symbol->bitmap_width) + column] == '1') {
                            rle_row[column] = fggrn;
                        } else {
                            rle_row[column] = bggrn;
                        }
                        break;
                    case 2:
                        if (rotated_bitmap[(row * symbol->bitmap_width) + column] == '1') {
                            rle_row[column] = fgblu;
                        } else {
                            rle_row[column] = bgblu;
                        }
                        break;
                }
            }
            
            run_count = 1;
            for (column = 1; column < symbol->bitmap_width; column++) {
                if ((rle_row[column - 1] == rle_row[column]) && (run_count < 63)) {
                    run_count++;
                } else {
                    run_count += 0xc0;
                    fputc(run_count, pcx_file);
                    fputc(rle_row[column - 1], pcx_file);
                    run_count = 1;
                }
            }
            
            if (run_count > 1) {
                run_count += 0xc0;
                fputc(run_count, pcx_file);
                fputc(rle_row[column - 1], pcx_file);
            }
        }  
    }
    
    fclose(pcx_file);
    
    return 0;
}