/* tif.c - Aldus Tagged Image File Format support */
/*
    libzint - the open source barcode library
    Copyright (C) 2016-2023 Robin Stuart <rstuart114@gmail.com>

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

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif
#include "common.h"
#include "output.h"
#include "tif.h"
#include "tif_lzw.h"

/* PhotometricInterpretation */
#define TIF_PMI_WHITEISZERO     0
#define TIF_PMI_BLACKISZERO     1
#define TIF_PMI_RGB             2
#define TIF_PMI_PALETTE_COLOR   3
#define TIF_PMI_SEPARATED       5 /* CMYK */

/* Compression */
#define TIF_NO_COMPRESSION      1
#define TIF_LZW                 5

static void to_color_map(const unsigned char rgb[4], tiff_color_t *color_map_entry) {
    color_map_entry->red = (rgb[0] << 8) | rgb[0];
    color_map_entry->green = (rgb[1] << 8) | rgb[1];
    color_map_entry->blue = (rgb[2] << 8) | rgb[2];
}

static void to_cmyk(const char *colour, unsigned char *cmyk) {
    int cyan, magenta, yellow, black;
    unsigned char alpha;

    (void) out_colour_get_cmyk(colour, &cyan, &magenta, &yellow, &black, &alpha);
    cmyk[0] = (unsigned char) roundf(cyan * 0xFF / 100.0f);
    cmyk[1] = (unsigned char) roundf(magenta * 0xFF / 100.0f);
    cmyk[2] = (unsigned char) roundf(yellow * 0xFF / 100.0f);
    cmyk[3] = (unsigned char) roundf(black * 0xFF / 100.0f);
    cmyk[4] = alpha;
}

static int is_big_endian(void) {
    return (*((const uint16_t *)"\x11\x22") == 0x1122);
}

/* TIFF Revision 6.0 https://www.adobe.io/content/dam/udp/en/open/standards/tiff/TIFF6.pdf */
INTERNAL int tif_pixel_plot(struct zint_symbol *symbol, const unsigned char *pixelbuf) {
    unsigned char fg[4], bg[4];
    int i;
    int pmi; /* PhotometricInterpretation */
    int rows_per_strip, strip_count;
    int rows_last_strip;
    int bytes_per_strip;
    uint16_t bits_per_sample;
    int samples_per_pixel;
    int pixels_per_sample;
    unsigned char map[128];
    tiff_color_t color_map[256] = {{0}};
    unsigned char palette[32][5];
    int color_map_size = 0;
    int extra_samples = 0;
    uint32_t free_memory;
    int row, column, strip;
    int strip_row;
    unsigned int bytes_put;
    long total_bytes_put;
    FILE *tif_file;
    const unsigned char *pb;
    int compression = TIF_NO_COMPRESSION;
    tif_lzw_state lzw_state;
    long file_pos;
    const int output_to_stdout = symbol->output_options & BARCODE_STDOUT;
    uint32_t *strip_offset;
    uint32_t *strip_bytes;
    unsigned char *strip_buf;

    tiff_header_t header;
    uint16_t entries = 0;
    tiff_tag_t tags[20];
    uint32_t offset = 0;
    int update_offsets[20];
    int offsets = 0;
    int ifd_size;
    uint32_t temp32;

    (void) out_colour_get_rgb(symbol->fgcolour, &fg[0], &fg[1], &fg[2], &fg[3]);
    (void) out_colour_get_rgb(symbol->bgcolour, &bg[0], &bg[1], &bg[2], &bg[3]);

    if (symbol->symbology == BARCODE_ULTRA) {
        static const int ultra_chars[8] = { 'W', 'C', 'B', 'M', 'R', 'Y', 'G', 'K' };

        if (symbol->output_options & CMYK_COLOUR) {
            static const unsigned char ultra_cmyks[8][4] = {
                {    0,    0,    0,    0 }, /* White */
                { 0xFF,    0,    0,    0 }, /* Cyan */
                { 0xFF, 0xFF,    0,    0 }, /* Blue */
                {    0, 0xFF,    0,    0 }, /* Magenta */
                {    0, 0xFF, 0xFF,    0 }, /* Red */
                {    0,    0, 0xFF,    0 }, /* Yellow */
                { 0xFF,    0, 0xFF,    0 }, /* Green */
                {    0,    0,    0, 0xFF }, /* Black */
            };
            for (i = 0; i < 8; i++) {
                map[ultra_chars[i]] = i;
                memcpy(palette[i], ultra_cmyks[i], 4);
                palette[i][4] = fg[3];
            }
            map['0'] = 8;
            to_cmyk(symbol->bgcolour, palette[8]);
            map['1'] = 9;
            to_cmyk(symbol->fgcolour, palette[9]);

            pmi = TIF_PMI_SEPARATED;
            bits_per_sample = 8;
            if (fg[3] == 0xff && bg[3] == 0xff) { /* If no alpha */
                samples_per_pixel = 4;
            } else {
                samples_per_pixel = 5;
                extra_samples = 1; /* Associated alpha */
            }
            pixels_per_sample = 1;
        } else {
            static const unsigned char ultra_rgbs[8][3] = {
                { 0xff, 0xff, 0xff, }, /* White */
                {    0, 0xff, 0xff, }, /* Cyan */
                {    0,    0, 0xff, }, /* Blue */
                { 0xff,    0, 0xff, }, /* Magenta */
                { 0xff,    0,    0, }, /* Red */
                { 0xff, 0xff,    0, }, /* Yellow */
                {    0, 0xff,    0, }, /* Green */
                {    0,    0,    0, }, /* Black */
            };
            for (i = 0; i < 8; i++) {
                map[ultra_chars[i]] = i;
                memcpy(palette[i], ultra_rgbs[i], 3);
                palette[i][3] = fg[3];
            }
            map['0'] = 8;
            memcpy(palette[8], bg, 4);
            map['1'] = 9;
            memcpy(palette[9], fg, 4);

            if (fg[3] == 0xff && bg[3] == 0xff) { /* If no alpha */
                pmi = TIF_PMI_PALETTE_COLOR;
                for (i = 0; i < 10; i++) {
                    to_color_map(palette[i], &color_map[i]);
                }
                bits_per_sample = 4;
                samples_per_pixel = 1;
                pixels_per_sample = 2;
                color_map_size = 16; /* 2**BitsPerSample */
            } else {
                pmi = TIF_PMI_RGB;
                bits_per_sample = 8;
                samples_per_pixel = 4;
                pixels_per_sample = 1;
                extra_samples = 1; /* Associated alpha */
            }
        }
    } else { /* fg/bg only */
        if (symbol->output_options & CMYK_COLOUR) {
            map['0'] = 0;
            to_cmyk(symbol->bgcolour, palette[0]);
            map['1'] = 1;
            to_cmyk(symbol->fgcolour, palette[1]);

            pmi = TIF_PMI_SEPARATED;
            bits_per_sample = 8;
            if (fg[3] == 0xff && bg[3] == 0xff) { /* If no alpha */
                samples_per_pixel = 4;
            } else {
                samples_per_pixel = 5;
                extra_samples = 1; /* Associated alpha */
            }
            pixels_per_sample = 1;
        } else if (bg[0] == 0xff && bg[1] == 0xff && bg[2] == 0xff && bg[3] == 0xff
                    && fg[0] == 0 && fg[1] == 0 && fg[2] == 0 && fg[3] == 0xff) {
            map['0'] = 0;
            map['1'] = 1;

            pmi = TIF_PMI_WHITEISZERO;
            bits_per_sample = 1;
            samples_per_pixel = 1;
            pixels_per_sample = 8;
        } else if (bg[0] == 0 && bg[1] == 0 && bg[2] == 0 && bg[3] == 0xff
                && fg[0] == 0xff && fg[1] == 0xff && fg[2] == 0xff && fg[3] == 0xff) {
            map['0'] = 0;
            map['1'] = 1;

            pmi = TIF_PMI_BLACKISZERO;
            bits_per_sample = 1;
            samples_per_pixel = 1;
            pixels_per_sample = 8;
        } else {
            map['0'] = 0;
            memcpy(palette[0], bg, 4);
            map['1'] = 1;
            memcpy(palette[1], fg, 4);

            pmi = TIF_PMI_PALETTE_COLOR;
            for (i = 0; i < 2; i++) {
                to_color_map(palette[i], &color_map[i]);
            }
            if (fg[3] == 0xff && bg[3] == 0xff) { /* If no alpha */
                bits_per_sample = 4;
                samples_per_pixel = 1;
                pixels_per_sample = 2;
                color_map_size = 16; /* 2**BitsPerSample */
            } else {
                bits_per_sample = 8;
                samples_per_pixel = 2;
                pixels_per_sample = 1;
                color_map_size = 256; /* 2**BitsPerSample */
                extra_samples = 1; /* Associated alpha */
            }
        }
    }

    /* TIFF Rev 6 Section 7 p.27 "Set RowsPerStrip such that the size of each strip is about 8K bytes...
     * Note that extremely wide high resolution images may have rows larger than 8K bytes; in this case,
     * RowsPerStrip should be 1, and the strip will be larger than 8K." */
    rows_per_strip = (8192 * pixels_per_sample) / (symbol->bitmap_width * samples_per_pixel);
    if (rows_per_strip == 0) {
        rows_per_strip = 1;
    }

    /* Suppresses clang-tidy clang-analyzer-core.VLASize warning */
    assert(symbol->bitmap_height > 0);

    if (rows_per_strip >= symbol->bitmap_height) {
        strip_count = 1;
        rows_per_strip = rows_last_strip = symbol->bitmap_height;
    } else {
        strip_count = symbol->bitmap_height / rows_per_strip;
        rows_last_strip = symbol->bitmap_height % rows_per_strip;
        if (rows_last_strip != 0) {
            strip_count++;
        }
        if (rows_per_strip > symbol->bitmap_height) {
            rows_per_strip = rows_last_strip = symbol->bitmap_height;
        }
    }
    assert(strip_count > 0); /* Suppress clang-analyzer-core.UndefinedBinaryOperatorResult */

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("TIFF (%dx%d) Strip Count %d, Rows Per Strip %d, Pixels Per Sample %d, Samples Per Pixel %d, PMI %d\n",
            symbol->bitmap_width, symbol->bitmap_height, strip_count, rows_per_strip, pixels_per_sample,
            samples_per_pixel, pmi);
    }

    bytes_per_strip = rows_per_strip * ((symbol->bitmap_width + pixels_per_sample - 1) / pixels_per_sample)
                        * samples_per_pixel;

    strip_offset = (uint32_t *) z_alloca(sizeof(uint32_t) * strip_count);
    strip_bytes = (uint32_t *) z_alloca(sizeof(uint32_t) * strip_count);
    strip_buf = (unsigned char *) z_alloca(bytes_per_strip + 1);

    free_memory = sizeof(tiff_header_t);

    for (i = 0; i < strip_count; i++) {
        strip_offset[i] = free_memory;
        if (i != (strip_count - 1)) {
            strip_bytes[i] = bytes_per_strip;
        } else {
            if (rows_last_strip) {
                strip_bytes[i] = rows_last_strip
                                    * ((symbol->bitmap_width + pixels_per_sample - 1) / pixels_per_sample)
                                    * samples_per_pixel;
            } else {
                strip_bytes[i] = bytes_per_strip;
            }
        }
        free_memory += strip_bytes[i];
    }
    if (free_memory & 1) {
        free_memory++; /* IFD must be on word boundary */
    }

    if (free_memory > 0xffff0000) {
        strcpy(symbol->errtxt, "670: Output file size too big");
        return ZINT_ERROR_MEMORY;
    }

    /* Open output file in binary mode */
    if (output_to_stdout) {
#ifdef _MSC_VER
        if (-1 == _setmode(_fileno(stdout), _O_BINARY)) {
            sprintf(symbol->errtxt, "671: Could not set stdout to binary (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_ACCESS;
        }
#endif
        tif_file = stdout;
    } else {
        if (!(tif_file = out_fopen(symbol->outfile, "wb+"))) { /* '+' as use fseek/ftell() */
            sprintf(symbol->errtxt, "672: Could not open output file (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_ACCESS;
        }
        compression = TIF_LZW;
        tif_lzw_init(&lzw_state);
    }

    /* Header */
    if (is_big_endian()) {
        header.byte_order = 0x4D4D; /* "MM" big-endian */
    } else {
        header.byte_order = 0x4949; /* "II" little-endian */
    }
    header.identity = 42;
    header.offset = free_memory;

    fwrite(&header, sizeof(tiff_header_t), 1, tif_file);
    total_bytes_put = sizeof(tiff_header_t);

    /* Pixel data */
    pb = pixelbuf;
    strip = 0;
    strip_row = 0;
    bytes_put = 0;
    for (row = 0; row < symbol->bitmap_height; row++) {
        if (samples_per_pixel == 1) {
            if (bits_per_sample == 1) { /* WHITEISZERO or BLACKISZERO */
                for (column = 0; column < symbol->bitmap_width; column += 8) {
                    unsigned char byte = 0;
                    for (i = 0; i < 8 && column + i < symbol->bitmap_width; i++, pb++) {
                        byte |= map[*pb] << (7 - i);
                    }
                    strip_buf[bytes_put++] = byte;
                }
            } else { /* bits_per_sample == 4, PALETTE_COLOR with no alpha */
                for (column = 0; column < symbol->bitmap_width; column += 2) {
                    unsigned char byte = map[*pb++] << 4;
                    if (column + 1 < symbol->bitmap_width) {
                        byte |= map[*pb++];
                    }
                    strip_buf[bytes_put++] = byte;
                }
            }
        } else if (samples_per_pixel == 2) { /* PALETTE_COLOR with alpha */
            for (column = 0; column < symbol->bitmap_width; column++) {
                const int idx = map[*pb++];
                strip_buf[bytes_put++] = idx;
                strip_buf[bytes_put++] = palette[idx][3];
            }
        } else { /* samples_per_pixel >= 4, RGB with alpha (4) or CMYK with (5) or without (4) alpha */
            for (column = 0; column < symbol->bitmap_width; column++) {
                const int idx = map[*pb++];
                memcpy(&strip_buf[bytes_put], &palette[idx], samples_per_pixel);
                bytes_put += samples_per_pixel;
            }
        }

        strip_row++;

        if (strip_row == rows_per_strip || (strip == strip_count - 1 && strip_row == rows_last_strip)) {
            /* End of strip */
            if (compression == TIF_LZW) {
                file_pos = ftell(tif_file);
                if (!tif_lzw_encode(&lzw_state, tif_file, strip_buf, bytes_put)) { /* Only fails if can't malloc */
                    tif_lzw_cleanup(&lzw_state);
                    (void) fclose(tif_file); /* Only use LZW if not STDOUT, so ok to close */
                    strcpy(symbol->errtxt, "673: Failed to malloc LZW hash table");
                    return ZINT_ERROR_MEMORY;
                }
                bytes_put = ftell(tif_file) - file_pos;
                if (bytes_put != strip_bytes[strip]) {
                    const int diff = bytes_put - strip_bytes[strip];
                    strip_bytes[strip] = bytes_put;
                    for (i = strip + 1; i < strip_count; i++) {
                        strip_offset[i] += diff;
                    }
                }
            } else {
                fwrite(strip_buf, 1, bytes_put, tif_file);
            }
            strip++;
            total_bytes_put += bytes_put;
            bytes_put = 0;
            strip_row = 0;
            /* Suppress clang-analyzer-core.UndefinedBinaryOperatorResult */
            assert(strip < strip_count || row + 1 == symbol->bitmap_height);
        }
    }

    if (total_bytes_put & 1) {
        putc(0, tif_file); /* IFD must be on word boundary */
        total_bytes_put++;
    }

    if (compression == TIF_LZW) {
        tif_lzw_cleanup(&lzw_state);

        file_pos = ftell(tif_file);
        fseek(tif_file, 4, SEEK_SET);
        free_memory = file_pos;
        fwrite(&free_memory, 4, 1, tif_file);
        fseek(tif_file, file_pos, SEEK_SET);
    }

    /* Image File Directory */
    tags[entries].tag = 0x0100; /* ImageWidth */
    tags[entries].type = 3; /* SHORT */
    tags[entries].count = 1;
    tags[entries++].offset = symbol->bitmap_width;

    tags[entries].tag = 0x0101; /* ImageLength - number of rows */
    tags[entries].type = 3; /* SHORT */
    tags[entries].count = 1;
    tags[entries++].offset = symbol->bitmap_height;

    if (samples_per_pixel != 1 || bits_per_sample != 1) {
        tags[entries].tag = 0x0102; /* BitsPerSample */
        tags[entries].type = 3; /* SHORT */
        tags[entries].count = samples_per_pixel;
        if (samples_per_pixel == 1) {
            tags[entries++].offset = bits_per_sample;
        } else if (samples_per_pixel == 2) { /* 2 SHORTS fit into LONG offset so packed into offset */
            tags[entries++].offset = (bits_per_sample << 16) | bits_per_sample;
        } else {
            update_offsets[offsets++] = entries;
            tags[entries++].offset = free_memory;
            free_memory += samples_per_pixel * 2;
        }
    }

    tags[entries].tag = 0x0103; /* Compression */
    tags[entries].type = 3; /* SHORT */
    tags[entries].count = 1;
    tags[entries++].offset = compression;

    tags[entries].tag = 0x0106; /* PhotometricInterpretation */
    tags[entries].type = 3; /* SHORT */
    tags[entries].count = 1;
    tags[entries++].offset = pmi;

    tags[entries].tag = 0x0111; /* StripOffsets */
    tags[entries].type = 4; /* LONG */
    tags[entries].count = strip_count;
    if (strip_count == 1) {
        tags[entries++].offset = strip_offset[0];
    } else {
        update_offsets[offsets++] = entries;
        tags[entries++].offset = free_memory;
        free_memory += strip_count * 4;
    }

    if (samples_per_pixel > 1) {
        tags[entries].tag = 0x0115; /* SamplesPerPixel */
        tags[entries].type = 3; /* SHORT */
        tags[entries].count = 1;
        tags[entries++].offset = samples_per_pixel;
    }

    tags[entries].tag = 0x0116; /* RowsPerStrip */
    tags[entries].type = 4; /* LONG */
    tags[entries].count = 1;
    tags[entries++].offset = rows_per_strip;

    tags[entries].tag = 0x0117; /* StripByteCounts */
    tags[entries].type = 4; /* LONG */
    tags[entries].count = strip_count;
    if (strip_count == 1) {
        tags[entries++].offset = strip_bytes[0];
    } else {
        update_offsets[offsets++] = entries;
        tags[entries++].offset = free_memory;
        free_memory += strip_count * 4;
    }

    tags[entries].tag = 0x011a; /* XResolution */
    tags[entries].type = 5; /* RATIONAL */
    tags[entries].count = 1;
    update_offsets[offsets++] = entries;
    tags[entries++].offset = free_memory;
    free_memory += 8;

    tags[entries].tag = 0x011b; /* YResolution */
    tags[entries].type = 5; /* RATIONAL */
    tags[entries].count = 1;
    update_offsets[offsets++] = entries;
    tags[entries++].offset = free_memory;
    free_memory += 8;

    tags[entries].tag = 0x0128; /* ResolutionUnit */
    tags[entries].type = 3; /* SHORT */
    tags[entries].count = 1;
    if (symbol->dpmm) {
        tags[entries++].offset = 3; /* Centimetres */
    } else {
        tags[entries++].offset = 2; /* Inches */
    }

    if (color_map_size) {
        tags[entries].tag = 0x0140; /* ColorMap */
        tags[entries].type = 3; /* SHORT */
        tags[entries].count = color_map_size * 3;
        update_offsets[offsets++] = entries;
        tags[entries++].offset = free_memory;
        /* free_memory += color_map_size * 3 * 2; Unnecessary as long as last use */
    }

    if (extra_samples) {
        tags[entries].tag = 0x0152; /* ExtraSamples */
        tags[entries].type = 3; /* SHORT */
        tags[entries].count = 1;
        tags[entries++].offset = extra_samples;
    }

    ifd_size = sizeof(entries) + sizeof(tiff_tag_t) * entries + sizeof(offset);
    for (i = 0; i < offsets; i++) {
        tags[update_offsets[i]].offset += ifd_size;
    }

    fwrite(&entries, sizeof(entries), 1, tif_file);
    fwrite(&tags, sizeof(tiff_tag_t), entries, tif_file);
    fwrite(&offset, sizeof(offset), 1, tif_file);
    total_bytes_put += ifd_size;

    if (samples_per_pixel > 2) {
        for (i = 0; i < samples_per_pixel; i++) {
            fwrite(&bits_per_sample, sizeof(bits_per_sample), 1, tif_file);
        }
        total_bytes_put += sizeof(bits_per_sample) * samples_per_pixel;
    }

    if (strip_count != 1) {
        /* Strip offsets */
        for (i = 0; i < strip_count; i++) {
            fwrite(&strip_offset[i], 4, 1, tif_file);
        }

        /* Strip byte lengths */
        for (i = 0; i < strip_count; i++) {
            fwrite(&strip_bytes[i], 4, 1, tif_file);
        }
        total_bytes_put += strip_count * 8;
    }

    /* XResolution */
    temp32 = symbol->dpmm ? symbol->dpmm : 72;
    fwrite(&temp32, 4, 1, tif_file);
    temp32 = symbol->dpmm ? 10 /*cm*/ : 1;
    fwrite(&temp32, 4, 1, tif_file);
    total_bytes_put += 8;

    /* YResolution */
    temp32 = symbol->dpmm ? symbol->dpmm : 72;
    fwrite(&temp32, 4, 1, tif_file);
    temp32 = symbol->dpmm ? 10 /*cm*/ : 1;
    fwrite(&temp32, 4, 1, tif_file);
    total_bytes_put += 8;

    if (color_map_size) {
        for (i = 0; i < color_map_size; i++) {
            fwrite(&color_map[i].red, 2, 1, tif_file);
        }
        for (i = 0; i < color_map_size; i++) {
            fwrite(&color_map[i].green, 2, 1, tif_file);
        }
        for (i = 0; i < color_map_size; i++) {
            fwrite(&color_map[i].blue, 2, 1, tif_file);
        }
        total_bytes_put += 6 * color_map_size;
    }

    if (ferror(tif_file)) {
        sprintf(symbol->errtxt, "679: Incomplete write to output (%d: %.30s)", errno, strerror(errno));
        if (!output_to_stdout) {
            (void) fclose(tif_file);
        }
        return ZINT_ERROR_FILE_WRITE;
    }

    if (output_to_stdout) {
        if (fflush(tif_file) != 0) {
            sprintf(symbol->errtxt, "980: Incomplete flush to output (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_WRITE;
        }
    } else {
        if (ftell(tif_file) != total_bytes_put) {
            (void) fclose(tif_file);
            strcpy(symbol->errtxt, "674: Failed to write all output");
            return ZINT_ERROR_FILE_WRITE;
        }
        if (fclose(tif_file) != 0) {
            sprintf(symbol->errtxt, "981: Failure on closing output file (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_WRITE;
        }
    }

    return 0;
}

/* vim: set ts=4 sw=4 et : */
