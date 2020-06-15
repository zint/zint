/* bmp.c - Handles output to Windows Bitmap file */

/*
    libzint - the open source barcode library
    Copyright (C) 2009 - 2020 Robin Stuart <rstuart114@gmail.com>

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
#include "bmp.h"        /* Bitmap header structure */
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

INTERNAL int bmp_pixel_plot(struct zint_symbol *symbol, char *pixelbuf) {
    int i, row, column;
    unsigned int fgred, fggrn, fgblu, bgred, bggrn, bgblu;
    int row_size;
    unsigned int data_offset, data_size, file_size;
    unsigned char *bitmap_file_start, *bmp_posn;
    unsigned char *bitmap;
    FILE *bmp_file;
    bitmap_file_header_t file_header;
    bitmap_info_header_t info_header;

    row_size = 4 * ((24 * symbol->bitmap_width + 31) / 32);
    data_size = symbol->bitmap_height * row_size;
    data_offset = sizeof (bitmap_file_header_t) + sizeof (bitmap_info_header_t);
    file_size = data_offset + data_size;

    bitmap_file_start = (unsigned char *) malloc(file_size);
    if (bitmap_file_start == NULL) {
        strcpy(symbol->errtxt, "602: Out of memory");
        return ZINT_ERROR_MEMORY;
    }
    memset(bitmap_file_start, 0, file_size); /* Not required but keeps padding bytes consistent */

    bitmap = bitmap_file_start + data_offset;

    fgred = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
    fggrn = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
    fgblu = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
    bgred = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
    bggrn = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
    bgblu = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);

    /* Pixel Plotting */
    for (row = 0; row < symbol->bitmap_height; row++) {
        for (column = 0; column < symbol->bitmap_width; column++) {
            i = (3 * column) + (row * row_size);
            switch (*(pixelbuf + (symbol->bitmap_width * (symbol->bitmap_height - row - 1)) + column)) {
                case 'W': // White
                    bitmap[i] = 255;
                    bitmap[i + 1] = 255;
                    bitmap[i + 2] = 255;
                    break;
                case 'C': // Cyan
                    bitmap[i] = 255;
                    bitmap[i + 1] = 255;
                    bitmap[i + 2] = 0;
                    break;
                case 'B': // Blue
                    bitmap[i] = 255;
                    bitmap[i + 1] = 0;
                    bitmap[i + 2] = 0;
                    break;
                case 'M': // Magenta
                    bitmap[i] = 255;
                    bitmap[i + 1] = 0;
                    bitmap[i + 2] = 255;
                    break;
                case 'R': // Red
                    bitmap[i] = 0;
                    bitmap[i + 1] = 0;
                    bitmap[i + 2] = 255;
                    break;
                case 'Y': // Yellow
                    bitmap[i] = 0;
                    bitmap[i + 1] = 255;
                    bitmap[i + 2] = 255;
                    break;
                case 'G': // Green
                    bitmap[i] = 0;
                    bitmap[i + 1] = 255;
                    bitmap[i + 2] = 0;
                    break;
                case 'K': // Black
                    bitmap[i] = 0;
                    bitmap[i + 1] = 0;
                    bitmap[i + 2] = 0;
                    break;
                case '1':
                    bitmap[i] = fgblu;
                    bitmap[i + 1] = fggrn;
                    bitmap[i + 2] = fgred;
                    break;
                default:
                    bitmap[i] = bgblu;
                    bitmap[i + 1] = bggrn;
                    bitmap[i + 2] = bgred;
                    break;

            }
        }
    }

    symbol->bitmap_byte_length = data_size;

    file_header.header_field = 0x4d42; // "BM"
    file_header.file_size = file_size;
    file_header.reserved = 0;
    file_header.data_offset = data_offset;

    info_header.header_size = sizeof (bitmap_info_header_t);
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

    bmp_posn = bitmap_file_start;
    memcpy(bitmap_file_start, &file_header, sizeof (bitmap_file_header_t));
    bmp_posn += sizeof (bitmap_file_header_t);
    memcpy(bmp_posn, &info_header, sizeof (bitmap_info_header_t));

    /* Open output file in binary mode */
    if ((symbol->output_options & BARCODE_STDOUT) != 0) {
#ifdef _MSC_VER
        if (-1 == _setmode(_fileno(stdout), _O_BINARY)) {
            strcpy(symbol->errtxt, "600: Can't open output file");
            free(bitmap_file_start);
            return ZINT_ERROR_FILE_ACCESS;
        }
#endif
        bmp_file = stdout;
    } else {
        if (!(bmp_file = fopen(symbol->outfile, "wb"))) {
            free(bitmap_file_start);
            strcpy(symbol->errtxt, "601: Can't open output file");
            return ZINT_ERROR_FILE_ACCESS;
        }
    }

    fwrite(bitmap_file_start, file_header.file_size, 1, bmp_file);
    fclose(bmp_file);

    free(bitmap_file_start);
    return 0;
}
