/* pcx.c - Handles output to ZSoft PCX file */
/*
    libzint - the open source barcode library
    Copyright (C) 2009-2023 Robin Stuart <rstuart114@gmail.com>

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

#include <errno.h>
#include <math.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif
#include "common.h"
#include "output.h"
#include "pcx.h"        /* PCX header structure */

/* ZSoft PCX File Format Technical Reference Manual http://bespin.org/~qz/pc-gpe/pcx.txt */
INTERNAL int pcx_pixel_plot(struct zint_symbol *symbol, const unsigned char *pixelbuf) {
    unsigned char fgred, fggrn, fgblu, fgalpha, bgred, bggrn, bgblu, bgalpha;
    int row, column, i, colour;
    int run_count;
    FILE *pcx_file;
    pcx_header_t header;
    int bytes_per_line = symbol->bitmap_width + (symbol->bitmap_width & 1); /* Must be even */
    unsigned char previous;
    const int output_to_stdout = symbol->output_options & BARCODE_STDOUT; /* Suppress gcc -fanalyzer warning */
    unsigned char *rle_row = (unsigned char *) z_alloca(bytes_per_line);

    rle_row[bytes_per_line - 1] = 0; /* Will remain zero if bitmap_width odd */

    (void) out_colour_get_rgb(symbol->fgcolour, &fgred, &fggrn, &fgblu, &fgalpha);
    (void) out_colour_get_rgb(symbol->bgcolour, &bgred, &bggrn, &bgblu, &bgalpha);

    header.manufacturer = 10; /* ZSoft */
    header.version = 5; /* Version 3.0 */
    header.encoding = 1; /* Run length encoding */
    header.bits_per_pixel = 8; /* TODO: 1-bit monochrome black/white */
    header.window_xmin = 0;
    header.window_ymin = 0;
    header.window_xmax = symbol->bitmap_width - 1;
    header.window_ymax = symbol->bitmap_height - 1;
    header.horiz_dpi = symbol->dpmm ? (uint16_t) roundf(stripf(symbol->dpmm * 25.4f)) : 300;
    header.vert_dpi = header.horiz_dpi;

    for (i = 0; i < 48; i++) {
        header.colourmap[i] = 0x00;
    }

    header.reserved = 0;
    header.number_of_planes = 3 + (fgalpha != 0xFF || bgalpha != 0xFF); /* TODO: 1-bit monochrome black/white */

    header.bytes_per_line = bytes_per_line;

    header.palette_info = 1; /* Colour */
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
        if (!(pcx_file = out_fopen(symbol->outfile, "wb"))) {
            sprintf(symbol->errtxt, "621: Could not open output file (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_ACCESS;
        }
    }

    fwrite(&header, sizeof(pcx_header_t), 1, pcx_file);

    for (row = 0; row < symbol->bitmap_height; row++) {
        const unsigned char *const pb = pixelbuf + row * symbol->bitmap_width;
        for (colour = 0; colour < header.number_of_planes; colour++) {
            for (column = 0; column < symbol->bitmap_width; column++) {
                const unsigned char ch = pb[column];
                switch (colour) {
                    case 0:
                        switch (ch) {
                            case 'W': /* White */
                            case 'M': /* Magenta */
                            case 'R': /* Red */
                            case 'Y': /* Yellow */
                                rle_row[column] = 255;
                                break;
                            case 'C': /* Cyan */
                            case 'B': /* Blue */
                            case 'G': /* Green */
                            case 'K': /* Black */
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
                        switch (ch) {
                            case 'W': /* White */
                            case 'C': /* Cyan */
                            case 'Y': /* Yellow */
                            case 'G': /* Green */
                                rle_row[column] = 255;
                                break;
                            case 'B': /* Blue */
                            case 'M': /* Magenta */
                            case 'R': /* Red */
                            case 'K': /* Black */
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
                        switch (ch) {
                            case 'W': /* White */
                            case 'C': /* Cyan */
                            case 'B': /* Blue */
                            case 'M': /* Magenta */
                                rle_row[column] = 255;
                                break;
                            case 'R': /* Red */
                            case 'Y': /* Yellow */
                            case 'G': /* Green */
                            case 'K': /* Black */
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
                    case 3:
                        rle_row[column] = ch != '0' ? fgalpha : bgalpha;
                        break;
                }
            }

            /* Based on ImageMagick/coders/pcx.c PCXWritePixels()
             * Copyright 1999-2020 ImageMagick Studio LLC */
            previous = rle_row[0];
            run_count = 1;
            for (column = 1; column < bytes_per_line; column++) { /* Note going up to bytes_per_line */
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

    if (ferror(pcx_file)) {
        sprintf(symbol->errtxt, "622: Incomplete write to output (%d: %.30s)", errno, strerror(errno));
        if (!output_to_stdout) {
            (void) fclose(pcx_file);
        }
        return ZINT_ERROR_FILE_WRITE;
    }

    if (output_to_stdout) {
        if (fflush(pcx_file) != 0) {
            sprintf(symbol->errtxt, "623: Incomplete flush to output (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_WRITE;
        }
    } else {
        if (fclose(pcx_file) != 0) {
            sprintf(symbol->errtxt, "624: Failure on closing output file (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_WRITE;
        }
    }

    return 0;
}

/* vim: set ts=4 sw=4 et : */
