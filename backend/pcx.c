/* pcx.c - Handles output to ZSoft PCX file */
/* ZSoft PCX File Format Technical Reference Manual http://bespin.org/~qz/pc-gpe/pcx.txt */

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

#include <errno.h>
#include <stdio.h>
#include "common.h"
#include "pcx.h"        /* PCX header structure */
#include <math.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#include <malloc.h>
#endif

INTERNAL int pcx_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf) {
    int fgred, fggrn, fgblu, bgred, bggrn, bgblu;
    int row, column, i, colour;
    int run_count;
    FILE *pcx_file;
    pcx_header_t header;
    int bytes_per_line = symbol->bitmap_width + (symbol->bitmap_width & 1); // Must be even
    unsigned char previous;
    const int output_to_stdout = symbol->output_options & BARCODE_STDOUT; /* Suppress gcc -fanalyzer warning */
#ifdef _MSC_VER
    unsigned char *rle_row;
#endif

#ifndef _MSC_VER
    unsigned char rle_row[bytes_per_line];
#else
    rle_row = (unsigned char *) _alloca(bytes_per_line);
#endif /* _MSC_VER */

    rle_row[bytes_per_line - 1] = 0; // Will remain zero if bitmap_width odd

    fgred = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
    fggrn = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
    fgblu = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
    bgred = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
    bggrn = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
    bgblu = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);

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

    for (i = 0; i < 48; i++) {
        header.colourmap[i] = 0x00;
    }

    header.reserved = 0;
    header.number_of_planes = 3;

    header.bytes_per_line = bytes_per_line;

    header.palette_info = 1; // Colour
    header.horiz_screen_size = 0;
    header.vert_screen_size = 0;

    for (i = 0; i < 54; i++) {
        header.filler[i] = 0x00;
    }

    /* Open output file in binary mode */
    if (output_to_stdout) {
#ifdef _MSC_VER
        if (-1 == _setmode(_fileno(stdout), _O_BINARY)) {
            sprintf(symbol->errtxt, "620: Could not set stdout to binary (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_ACCESS;
        }
#endif
        pcx_file = stdout;
    } else {
        if (!(pcx_file = fopen(symbol->outfile, "wb"))) {
            sprintf(symbol->errtxt, "621: Could not open output file (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_ACCESS;
        }
    }

    fwrite(&header, sizeof(pcx_header_t), 1, pcx_file);

    for (row = 0; row < symbol->bitmap_height; row++) {
        for (colour = 0; colour < 3; colour++) {
            for (column = 0; column < symbol->bitmap_width; column++) {
                switch (colour) {
                    case 0:
                        switch (pixelbuf[(row * symbol->bitmap_width) + column]) {
                            case 'W': // White
                            case 'M': // Magenta
                            case 'R': // Red
                            case 'Y': // Yellow
                                rle_row[column] = 255;
                                break;
                            case 'C': // Cyan
                            case 'B': // Blue
                            case 'G': // Green
                            case 'K': // Black
                                rle_row[column] = 0;
                                break;
                            case '1':
                                rle_row[column] = fgred;
                                break;
                            default:
                                rle_row[column] = bgred;
                                break;
                        }
                        break;
                    case 1:
                        switch (pixelbuf[(row * symbol->bitmap_width) + column]) {
                            case 'W': // White
                            case 'C': // Cyan
                            case 'Y': // Yellow
                            case 'G': // Green
                                rle_row[column] = 255;
                                break;
                            case 'B': // Blue
                            case 'M': // Magenta
                            case 'R': // Red
                            case 'K': // Black
                                rle_row[column] = 0;
                                break;
                            case '1':
                                rle_row[column] = fggrn;
                                break;
                            default:
                                rle_row[column] = bggrn;
                                break;
                        }
                        break;
                    case 2:
                        switch (pixelbuf[(row * symbol->bitmap_width) + column]) {
                            case 'W': // White
                            case 'C': // Cyan
                            case 'B': // Blue
                            case 'M': // Magenta
                                rle_row[column] = 255;
                                break;
                            case 'R': // Red
                            case 'Y': // Yellow
                            case 'G': // Green
                            case 'K': // Black
                                rle_row[column] = 0;
                                break;
                            case '1':
                                rle_row[column] = fgblu;
                                break;
                            default:
                                rle_row[column] = bgblu;
                                break;
                        }
                        break;
                }
            }

            /* Based on ImageMagick/coders/pcx.c PCXWritePixels()
             * Copyright 1999-2020 ImageMagick Studio LLC */
            previous = rle_row[0];
            run_count = 1;
            for (column = 1; column < bytes_per_line; column++) { // Note going up to bytes_per_line
                if ((previous == rle_row[column]) && (run_count < 63)) {
                    run_count++;
                } else {
                    if (run_count > 1 || (previous & 0xc0) == 0xc0) {
                        run_count += 0xc0;
                        fputc(run_count, pcx_file);
                    }
                    fputc(previous, pcx_file);
                    previous = rle_row[column];
                    run_count = 1;
                }
            }

            if (run_count > 1 || (previous & 0xc0) == 0xc0) {
                run_count += 0xc0;
                fputc(run_count, pcx_file);
            }
            fputc(previous, pcx_file);
        }
    }

    if (output_to_stdout) {
        fflush(pcx_file);
    } else {
        fclose(pcx_file);
    }

    return 0;
}
