/* png.c - Handles output to PNG file */

/*
    libzint - the open source barcode library
    Copyright (C) 2009-2021 Robin Stuart <rstuart114@gmail.com>

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

#ifndef NO_PNG

#include <errno.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#endif
#include "common.h"

#include <png.h>
#include <zlib.h>
#include <setjmp.h>

#define SSET	"0123456789ABCDEF"

/* Note if change this need to change "backend/tests/test_png.c" definition also */
struct wpng_error_type {
    struct zint_symbol *symbol;
    jmp_buf jmpbuf;
};

STATIC_UNLESS_ZINT_TEST void wpng_error_handler(png_structp png_ptr, png_const_charp msg) {
    struct wpng_error_type *wpng_error_ptr;

    wpng_error_ptr = (struct wpng_error_type *) png_get_error_ptr(png_ptr);
    if (wpng_error_ptr == NULL) {
        /* we are completely hosed now */
        fprintf(stderr, "Error 636: libpng error: %s\n", msg ? msg : "<NULL>");
        fprintf(stderr, "Error 637: jmpbuf not recoverable, terminating\n");
        fflush(stderr);
        return; /* libpng will call abort() */
    }
    sprintf(wpng_error_ptr->symbol->errtxt, "635: libpng error: %.60s", msg ? msg : "<NULL>");
    longjmp(wpng_error_ptr->jmpbuf, 1);
}

/* Guestimate best compression strategy */
static int guess_compression_strategy(struct zint_symbol *symbol, unsigned char *pixelbuf) {
    (void)pixelbuf;

    /* TODO: Do properly */

    /* It seems the best choice for typical barcode pngs is one of Z_DEFAULT_STRATEGY and Z_FILTERED */

    /* Some guesses */
    if (symbol->symbology == BARCODE_MAXICODE) {
        return Z_DEFAULT_STRATEGY;
    }
    if (symbol->symbology == BARCODE_AZTEC && symbol->bitmap_width <= 30) {
        return Z_DEFAULT_STRATEGY;
    }

    /* Z_FILTERED seems to work better for slightly more barcodes+data so default to that */
    return Z_FILTERED;
}

INTERNAL int png_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf) {
    struct wpng_error_type wpng_error;
    FILE *outfile;
    png_structp png_ptr;
    png_infop info_ptr;
    int i;
    int row, column;
    png_color bg, fg;
    unsigned char bg_alpha, fg_alpha;
    unsigned char map[128];
    png_color palette[32];
    int num_palette;
    unsigned char trans_alpha[32];
    int num_trans;
    int bit_depth;
    int compression_strategy;
    unsigned char *pb;
    const int output_to_stdout = symbol->output_options & BARCODE_STDOUT;

#ifndef _MSC_VER
    unsigned char outdata[symbol->bitmap_width];
#else
    unsigned char *outdata = (unsigned char *) _alloca(symbol->bitmap_width);
#endif

    wpng_error.symbol = symbol;

    fg.red = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
    fg.green = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
    fg.blue = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
    bg.red = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
    bg.green = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
    bg.blue = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);

    if (strlen(symbol->fgcolour) > 6) {
        fg_alpha = (16 * ctoi(symbol->fgcolour[6])) + ctoi(symbol->fgcolour[7]);
    } else {
        fg_alpha = 0xff;
    }

    if (strlen(symbol->bgcolour) > 6) {
        bg_alpha = (16 * ctoi(symbol->bgcolour[6])) + ctoi(symbol->bgcolour[7]);
    } else {
        bg_alpha = 0xff;
    }

    num_trans = 0;
    if (symbol->symbology == BARCODE_ULTRA) {
        static const int ultra_chars[8] = { 'W', 'C', 'B', 'M', 'R', 'Y', 'G', 'K' };
        static const png_color ultra_colours[8] = {
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
            palette[i] = ultra_colours[i];
            if (fg_alpha != 0xff) {
                trans_alpha[i] = fg_alpha;
            }
        }
        num_palette = 8;
        if (fg_alpha != 0xff) {
            num_trans = 8;
        }

        /* For Ultracode, have foreground only if have bind/box */
        if (symbol->border_width > 0 && (symbol->output_options & (BARCODE_BIND | BARCODE_BOX))) {
            /* Check whether can re-use black */
            if (fg.red == 0 && fg.green == 0 && fg.blue == 0) {
                map['1'] = 7; /* Re-use black */
            } else {
                map['1'] = num_palette;
                palette[num_palette++] = fg;
                if (fg_alpha != 0xff) {
                    trans_alpha[num_trans++] = fg_alpha;
                }
            }
        }

        /* For Ultracode, have background only if have whitespace/quiet zones */
        if (symbol->whitespace_width > 0 || symbol->whitespace_height > 0) { /* TODO: BARCODE_QUIET_ZONES also */
            /* Check whether can re-use white */
            if (bg.red == 0xff && bg.green == 0xff && bg.blue == 0xff && bg_alpha == fg_alpha) {
                map['0'] = 0; /* Re-use white */
            } else {
                if (bg_alpha == 0xff || fg_alpha != 0xff) {
                    /* No alpha or have foreground alpha - add to end */
                    map['0'] = num_palette;
                    palette[num_palette++] = bg;
                } else {
                    /* Alpha and no foreground alpha - add to front & move white to end */
                    map['0'] = 0;
                    palette[0] = bg;
                    map['W'] = num_palette;
                    palette[num_palette++] = ultra_colours[0];
                }
                if (bg_alpha != 0xff) {
                    trans_alpha[num_trans++] = bg_alpha;
                }
            }
        }
    } else {
        int bg_idx = 0, fg_idx = 1;
        /* Do alphas first so can swop indexes if background not alpha */
        if (bg_alpha != 0xff) {
            trans_alpha[num_trans++] = bg_alpha;
        }
        if (fg_alpha != 0xff) {
            trans_alpha[num_trans++] = fg_alpha;
            if (num_trans == 1) {
                /* Only foreground has alpha so swop indexes - saves a byte! */
                bg_idx = 1;
                fg_idx = 0;
            }
        }

        map['0'] = bg_idx;
        palette[bg_idx] = bg;
        map['1'] = fg_idx;
        palette[fg_idx] = fg;
        num_palette = 2;
    }

    if (num_palette <= 2) {
        bit_depth = 1;
    } else {
        bit_depth = 4;
    }

    /* Open output file in binary mode */
    if (output_to_stdout) {
#ifdef _MSC_VER
        if (-1 == _setmode(_fileno(stdout), _O_BINARY)) {
            sprintf(symbol->errtxt, "631: Could not set stdout to binary (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_ACCESS;
        }
#endif
        outfile = stdout;
    } else {
        if (!(outfile = fopen(symbol->outfile, "wb"))) {
            sprintf(symbol->errtxt, "632: Could not open output file (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_ACCESS;
        }
    }

    /* Set up error handling routine as proc() above */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, &wpng_error, wpng_error_handler, NULL);
    if (!png_ptr) {
        strcpy(symbol->errtxt, "633: Insufficient memory for PNG write structure buffer");
        if (!output_to_stdout) {
            fclose(outfile);
        }
        return ZINT_ERROR_MEMORY;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        strcpy(symbol->errtxt, "634: Insufficient memory for PNG info structure buffer");
        if (!output_to_stdout) {
            fclose(outfile);
        }
        return ZINT_ERROR_MEMORY;
    }

    /* catch jumping here */
    if (setjmp(wpng_error.jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        if (!output_to_stdout) {
            fclose(outfile);
        }
        return ZINT_ERROR_MEMORY;
    }

    /* open output file with libpng */
    png_init_io(png_ptr, outfile);

    /* set compression */
    png_set_compression_level(png_ptr, 9);

    /* Compression strategy can make a difference */
    compression_strategy = guess_compression_strategy(symbol, pixelbuf);
    if (compression_strategy != Z_DEFAULT_STRATEGY) {
        png_set_compression_strategy(png_ptr, compression_strategy);
    }

    /* set Header block */
    png_set_IHDR(png_ptr, info_ptr, symbol->bitmap_width, symbol->bitmap_height,
            bit_depth, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_set_PLTE(png_ptr, info_ptr, palette, num_palette);
    if (num_trans) {
        png_set_tRNS(png_ptr, info_ptr, trans_alpha, num_trans, NULL);
    }

    /* write all chunks up to (but not including) first IDAT */
    png_write_info(png_ptr, info_ptr);

    /* Pixel Plotting */
    pb = pixelbuf;
    if (bit_depth == 1) {
        for (row = 0; row < symbol->bitmap_height; row++) {
            unsigned char *image_data = outdata;
            for (column = 0; column < symbol->bitmap_width; column += 8, image_data++) {
                unsigned char byte = 0;
                for (i = 0; i < 8 && column + i < symbol->bitmap_width; i++, pb++) {
                    byte |= map[*pb] << (7 - i);
                }
                *image_data = byte;
            }
            /* write row contents to file */
            png_write_row(png_ptr, outdata);
        }
    } else { /* Bit depth 4 */
        for (row = 0; row < symbol->bitmap_height; row++) {
            unsigned char *image_data = outdata;
            for (column = 0; column < symbol->bitmap_width; column += 2, image_data++) {
                unsigned char byte = map[*pb++] << 4;
                if (column + 1 < symbol->bitmap_width) {
                    byte |= map[*pb++];
                }
                *image_data = byte;
            }
            /* write row contents to file */
            png_write_row(png_ptr, outdata);
        }
    }

    /* End the file */
    png_write_end(png_ptr, NULL);

    /* make sure we have disengaged */
    png_destroy_write_struct(&png_ptr, &info_ptr);

    if (output_to_stdout) {
        fflush(outfile);
    } else {
        fclose(outfile);
    }

    return 0;
}
#else
/* https://stackoverflow.com/a/26541331/664741 Suppresses gcc warning ISO C forbids an empty translation unit */
typedef int make_iso_compilers_happy;
#endif /* NO_PNG */
