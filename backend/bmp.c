/* bmp.c - Handles output to Windows Bitmap file */
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
#include "bmp.h"        /* Bitmap header structure */

INTERNAL int bmp_pixel_plot(struct zint_symbol *symbol, const unsigned char *pixelbuf) {
    int i, row, column;
    int row_size;
    int bits_per_pixel;
    int colour_count;
    int resolution;
    unsigned int data_offset, data_size, file_size;
    unsigned char *bitmap_file_start, *bmp_posn;
    unsigned char *bitmap;
    FILE *bmp_file;
    bitmap_file_header_t file_header;
    bitmap_info_header_t info_header;
    color_ref_t bg_color_ref;
    color_ref_t fg_color_ref;
    color_ref_t ultra_color_ref[8];
    int ultra_fg_index = 9;
    unsigned char map[128];
    const int output_to_stdout = symbol->output_options & BARCODE_STDOUT; /* Suppress gcc -fanalyzer warning */

    (void) out_colour_get_rgb(symbol->fgcolour, &fg_color_ref.red, &fg_color_ref.green, &fg_color_ref.blue,
                NULL /*alpha*/);
    fg_color_ref.reserved = 0x00;

    (void) out_colour_get_rgb(symbol->bgcolour, &bg_color_ref.red, &bg_color_ref.green, &bg_color_ref.blue,
                NULL /*alpha*/);
    bg_color_ref.reserved = 0x00;

    if (symbol->symbology == BARCODE_ULTRA) {
        static const int ultra_chars[8] = { 'C', 'B', 'M', 'R', 'Y', 'G', 'K', 'W' };
        for (i = 0; i < 8; i++) {
            ultra_color_ref[i].red = colour_to_red(i + 1);
            ultra_color_ref[i].green = colour_to_green(i + 1);
            ultra_color_ref[i].blue = colour_to_blue(i + 1);
            ultra_color_ref[i].reserved = 0x00;
            if (memcmp(&ultra_color_ref[i], &fg_color_ref, sizeof(fg_color_ref)) == 0) {
                ultra_fg_index = i + 1;
            }
            map[ultra_chars[i]] = i + 1;
        }
        bits_per_pixel = 4;
        colour_count = ultra_fg_index == 9 ? 10 : 9;
        map['0'] = 0;
        map['1'] = ultra_fg_index;
    } else {
        bits_per_pixel = 1;
        colour_count = 2;
        map['0'] = 0;
        map['1'] = 0x80;
    }
    row_size = 4 * ((bits_per_pixel * symbol->bitmap_width + 31) / 32);
    data_size = symbol->bitmap_height * row_size;
    data_offset = sizeof(bitmap_file_header_t) + sizeof(bitmap_info_header_t);
    data_offset += colour_count * sizeof(color_ref_t);
    file_size = data_offset + data_size;

    bitmap_file_start = (unsigned char *) malloc(file_size);
    if (bitmap_file_start == NULL) {
        strcpy(symbol->errtxt, "602: Insufficient memory for BMP file buffer");
        return ZINT_ERROR_MEMORY;
    }
    memset(bitmap_file_start, 0, file_size); /* Not required but keeps padding bytes consistent */

    bitmap = bitmap_file_start + data_offset;

    /* Pixel Plotting */
    if (symbol->symbology == BARCODE_ULTRA) {
        for (row = 0; row < symbol->bitmap_height; row++) {
            const unsigned char *pb = pixelbuf + (symbol->bitmap_width * (symbol->bitmap_height - row - 1));
            for (column = 0; column < symbol->bitmap_width; column++) {
                bitmap[(column >> 1) + (row * row_size)] |= map[pb[column]] << (!(column & 1) << 2);
            }
        }
    } else {
        for (row = 0; row < symbol->bitmap_height; row++) {
            const unsigned char *pb = pixelbuf + (symbol->bitmap_width * (symbol->bitmap_height - row - 1));
            for (column = 0; column < symbol->bitmap_width; column++) {
                bitmap[(column >> 3) + (row * row_size)] |= map[pb[column]] >> (column & 7);
            }
        }
    }

    file_header.header_field = 0x4d42; /* "BM" */
    file_header.file_size = file_size;
    file_header.reserved = 0;
    file_header.data_offset = data_offset;

    info_header.header_size = sizeof(bitmap_info_header_t);
    info_header.width = symbol->bitmap_width;
    info_header.height = symbol->bitmap_height;
    info_header.colour_planes = 1;
    info_header.bits_per_pixel = bits_per_pixel;
    info_header.compression_method = 0; /* BI_RGB */
    info_header.image_size = 0;
    resolution = symbol->dpmm ? (int) roundf(stripf(symbol->dpmm * 1000.0f)) : 0; /* pixels per metre */
    info_header.horiz_res = resolution;
    info_header.vert_res = resolution;
    info_header.colours = colour_count;
    info_header.important_colours = colour_count;

    bmp_posn = bitmap_file_start;
    memcpy(bitmap_file_start, &file_header, sizeof(bitmap_file_header_t));
    bmp_posn += sizeof(bitmap_file_header_t);
    memcpy(bmp_posn, &info_header, sizeof(bitmap_info_header_t));

    bmp_posn += sizeof(bitmap_info_header_t);
    memcpy(bmp_posn, &bg_color_ref, sizeof(color_ref_t));
    if (symbol->symbology == BARCODE_ULTRA) {
        for (i = 0; i < 8; i++) {
            bmp_posn += sizeof(color_ref_t);
            memcpy(bmp_posn, &ultra_color_ref[i], sizeof(color_ref_t));
        }
        if (ultra_fg_index == 9) {
            bmp_posn += sizeof(color_ref_t);
            memcpy(bmp_posn, &fg_color_ref, sizeof(color_ref_t));
        }
    } else {
        bmp_posn += sizeof(color_ref_t);
        memcpy(bmp_posn, &fg_color_ref, sizeof(color_ref_t));
    }

    /* Open output file in binary mode */
    if (output_to_stdout) {
#ifdef _MSC_VER
        if (-1 == _setmode(_fileno(stdout), _O_BINARY)) {
            sprintf(symbol->errtxt, "600: Could not set stdout to binary (%d: %.30s)", errno, strerror(errno));
            free(bitmap_file_start);
            return ZINT_ERROR_FILE_ACCESS;
        }
#endif
        bmp_file = stdout;
    } else {
        if (!(bmp_file = out_fopen(symbol->outfile, "wb"))) {
            sprintf(symbol->errtxt, "601: Could not open output file (%d: %.30s)", errno, strerror(errno));
            free(bitmap_file_start);
            return ZINT_ERROR_FILE_ACCESS;
        }
    }

    fwrite(bitmap_file_start, file_header.file_size, 1, bmp_file);

    if (ferror(bmp_file)) {
        sprintf(symbol->errtxt, "603: Incomplete write to output (%d: %.30s)", errno, strerror(errno));
        free(bitmap_file_start);
        if (!output_to_stdout) {
            (void) fclose(bmp_file);
        }
        return ZINT_ERROR_FILE_WRITE;
    }

    if (output_to_stdout) {
        if (fflush(bmp_file) != 0) {
            sprintf(symbol->errtxt, "604: Incomplete flush to output (%d: %.30s)", errno, strerror(errno));
            free(bitmap_file_start);
            return ZINT_ERROR_FILE_WRITE;
        }
    } else {
        if (fclose(bmp_file) != 0) {
            sprintf(symbol->errtxt, "605: Failure on closing output file (%d: %.30s)", errno, strerror(errno));
            free(bitmap_file_start);
            return ZINT_ERROR_FILE_WRITE;
        }
    }

    free(bitmap_file_start);
    return 0;
}

/* vim: set ts=4 sw=4 et : */
