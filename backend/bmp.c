/* bmp.c - Handles output to Windows Bitmap file */

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
#include "bmp.h"        /* Bitmap header structure */
#include <math.h>

#define SSET	"0123456789ABCDEF"

int bmp_pixel_plot(struct zint_symbol *symbol, int image_height, int image_width, char *pixelbuf, int rotate_angle) {
    int i, row, column, errno;
    int fgred, fggrn, fgblu, bgred, bggrn, bgblu;
    int row_size;
    unsigned int data_size;
    unsigned char *bitmap_file_start, *bmp_posn;
    FILE *bmp_file;

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

    if (symbol->bitmap != NULL)
        free(symbol->bitmap);
    
    row_size = 4 * floor((24 * symbol->bitmap_width + 31) / 32);
    symbol->bitmap = (char *) malloc(row_size * symbol->bitmap_height);

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

    /* Pixel Plotting */
    i = 0;
    switch (rotate_angle) {
        case 0: /* Plot the right way up */
            for (row = 0; row < image_height; row++) {
                for (column = 0; column < image_width; column++) {
                    i = (3 * column) + (row * row_size);
                    switch (*(pixelbuf + (image_width * (image_height - row - 1)) + column)) {
                        case '1':
                            symbol->bitmap[i] = fgblu;
                            symbol->bitmap[i + 1] = fggrn;
                            symbol->bitmap[i + 2] = fgred;
                            break;
                        default:
                            symbol->bitmap[i] = bgblu;
                            symbol->bitmap[i + 1] = bggrn;
                            symbol->bitmap[i + 2] = bgred;
                            break;

                    }
                }
            }
            break;
        case 90: /* Plot 90 degrees clockwise */
            for (row = 0; row < image_width; row++) {
                for (column = 0; column < image_height; column++) {
                    i = (3 * column) + (row * row_size);
                    switch (*(pixelbuf + (image_width * (image_height - column - 1)) + (image_width - row - 1))) {
                        case '1':
                            symbol->bitmap[i] = fgblu;
                            symbol->bitmap[i + 1] = fggrn;
                            symbol->bitmap[i + 2] = fgred;
                            break;
                        default:
                            symbol->bitmap[i] = bgblu;
                            symbol->bitmap[i + 1] = bggrn;
                            symbol->bitmap[i + 2] = bgred;
                            break;

                    }
                }
            }
            break;
        case 180: /* Plot upside down */
            for (row = 0; row < image_height; row++) {
                for (column = 0; column < image_width; column++) {
                    i = (3 * column) + (row * row_size);
                    switch (*(pixelbuf + (image_width * row) + (image_width - column - 1))) {
                        case '1':
                            symbol->bitmap[i] = fgblu;
                            symbol->bitmap[i + 1] = fggrn;
                            symbol->bitmap[i + 2] = fgred;
                            break;
                        default:
                            symbol->bitmap[i] = bgblu;
                            symbol->bitmap[i + 1] = bggrn;
                            symbol->bitmap[i + 2] = bgred;
                            break;

                    }
                }
            }
            break;
        case 270: /* Plot 90 degrees anti-clockwise */
            for (row = 0; row < image_width; row++) {
                for (column = 0; column < image_height; column++) {
                    i = (3 * column) + (row * row_size);
                    switch (*(pixelbuf + (image_width * column) + row)) {
                        case '1':
                            symbol->bitmap[i] = fgblu;
                            symbol->bitmap[i + 1] = fggrn;
                            symbol->bitmap[i + 2] = fgred;
                            break;
                        default:
                            symbol->bitmap[i] = bgblu;
                            symbol->bitmap[i + 1] = bggrn;
                            symbol->bitmap[i + 2] = bgred;
                            break;

                    }
                }
            }
            break;
    }

    data_size = symbol->bitmap_height * row_size;
    symbol->bitmap_byte_length = data_size;
    
    bitmap_file_header_t file_header;
    bitmap_info_header_t info_header;
    
    file_header.header_field = 0x4d42;  // "BM"
    file_header.file_size = sizeof(bitmap_file_header_t) + sizeof(bitmap_info_header_t) + data_size;
    file_header.reserved = 0;
    file_header.data_offset = sizeof(bitmap_file_header_t) + sizeof(bitmap_info_header_t);

    info_header.header_size = sizeof(bitmap_info_header_t);
    info_header.width = symbol->bitmap_width;
    info_header.height = symbol->bitmap_height;
    info_header.colour_planes = 1;
    info_header.bits_per_pixel = 24;
    info_header.compression_method = 0; // BI_RGB
    info_header.image_size = 0;
    info_header.horiz_res = 0;
    info_header.vert_res = 0;
    info_header.colours = 0;
    info_header.important_colours = 0;
    
    bitmap_file_start = (unsigned char*)malloc(file_header.file_size);
    memset(bitmap_file_start, 0xff, file_header.file_size);
    
    bmp_posn = bitmap_file_start;
    memcpy( bitmap_file_start, &file_header, sizeof(bitmap_file_header_t));
    bmp_posn += sizeof(bitmap_file_header_t);
    memcpy(bmp_posn, &info_header, sizeof(bitmap_info_header_t) );
    bmp_posn += sizeof(bitmap_info_header_t);
    memcpy(bmp_posn, symbol->bitmap, data_size);
    
    /* Open output file in binary mode */
    if ((symbol->output_options & BARCODE_STDOUT) != 0) {
#ifdef _MSC_VER
        if (-1 == _setmode(_fileno(stdout), _O_BINARY)) {
            strcpy(symbol->errtxt, "Can't open output file");
            return ZINT_ERROR_FILE_ACCESS;
        }
#endif
        bmp_file = stdout;
    } else {
        if (!(bmp_file = fopen(symbol->outfile, "wb"))) {
            strcpy(symbol->errtxt, "Can't open output file");
            return ZINT_ERROR_FILE_ACCESS;
        }
    }
    
    fwrite(bitmap_file_start, file_header.file_size, 1, bmp_file);
    fclose(bmp_file);
    
    return 0;
}