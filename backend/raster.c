/* raster.c - Handles output to raster files */

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
#ifdef _MSC_VER
#include <malloc.h>
#include <fcntl.h>
#include <io.h>
#endif
#include <math.h>
#include <assert.h>
#include "common.h"
#include "output.h"

#include "font.h" /* Font for human readable text */

#define SSET    "0123456789ABCDEF"

#define DEFAULT_INK '1'
#define DEFAULT_PAPER '0'

#ifndef NO_PNG
INTERNAL int png_pixel_plot(struct zint_symbol *symbol, char *pixelbuf);
#endif /* NO_PNG */
INTERNAL int bmp_pixel_plot(struct zint_symbol *symbol, char *pixelbuf);
INTERNAL int pcx_pixel_plot(struct zint_symbol *symbol, char *pixelbuf);
INTERNAL int gif_pixel_plot(struct zint_symbol *symbol, char *pixelbuf);
INTERNAL int tif_pixel_plot(struct zint_symbol *symbol, char *pixelbuf);

static const char ultra_colour[] = "WCBMRYGK";

static int buffer_plot(struct zint_symbol *symbol, char *pixelbuf) {
    /* Place pixelbuffer into symbol */
    int fgred, fggrn, fgblu, bgred, bggrn, bgblu;
    int fgalpha, bgalpha;
    int row, column, i;
    int plot_alpha = 0;
    
    if (strlen(symbol->fgcolour) > 6) {
        fgalpha = (16 * ctoi(symbol->fgcolour[6])) + ctoi(symbol->fgcolour[7]);
        plot_alpha = 1;
    } else {
        fgalpha = 0xff;
    }
    
    if (strlen(symbol->bgcolour) > 6) {
        bgalpha = (16 * ctoi(symbol->bgcolour[6])) + ctoi(symbol->bgcolour[7]);
        plot_alpha = 1;
    } else {
        bgalpha = 0xff;
    }

    /* Free any previous bitmap */
    if (symbol->bitmap != NULL) {
        free(symbol->bitmap);
        symbol->bitmap = NULL;
    }
    if (symbol->alphamap != NULL) {
        free(symbol->alphamap);
        symbol->alphamap = NULL;
    }
    
    symbol->bitmap = (unsigned char *) malloc(symbol->bitmap_width * symbol->bitmap_height * 3);
    if (symbol->bitmap == NULL) {
        strcpy(symbol->errtxt, "661: Insufficient memory for bitmap buffer");
        return ZINT_ERROR_MEMORY;
    }
    if (plot_alpha) {
        symbol->alphamap = (unsigned char *) malloc(symbol->bitmap_width * symbol->bitmap_height);
        if (symbol->alphamap == NULL) {
            strcpy(symbol->errtxt, "662: Insufficient memory for alphamap buffer");
            return ZINT_ERROR_MEMORY;
        }
    }

    fgred = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
    fggrn = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
    fgblu = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
    bgred = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
    bggrn = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
    bgblu = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);

    for (row = 0; row < symbol->bitmap_height; row++) {
        for (column = 0; column < symbol->bitmap_width; column++) {
            i = ((row * symbol->bitmap_width) + column) * 3;
            switch (*(pixelbuf + (symbol->bitmap_width * row) + column)) {
                case 'W': // White
                    symbol->bitmap[i] = 255;
                    symbol->bitmap[i + 1] = 255;
                    symbol->bitmap[i + 2] = 255;
                    if (plot_alpha) symbol->alphamap[i / 3] = fgalpha;
                    break;
                case 'C': // Cyan
                    symbol->bitmap[i] = 0;
                    symbol->bitmap[i + 1] = 255;
                    symbol->bitmap[i + 2] = 255;
                    if (plot_alpha) symbol->alphamap[i / 3] = fgalpha;
                    break;
                case 'B': // Blue
                    symbol->bitmap[i] = 0;
                    symbol->bitmap[i + 1] = 0;
                    symbol->bitmap[i + 2] = 255;
                    if (plot_alpha) symbol->alphamap[i / 3] = fgalpha;
                    break;
                case 'M': // Magenta
                    symbol->bitmap[i] = 255;
                    symbol->bitmap[i + 1] = 0;
                    symbol->bitmap[i + 2] = 255;
                    if (plot_alpha) symbol->alphamap[i / 3] = fgalpha;
                    break;
                case 'R': // Red
                    symbol->bitmap[i] = 255;
                    symbol->bitmap[i + 1] = 0;
                    symbol->bitmap[i + 2] = 0;
                    if (plot_alpha) symbol->alphamap[i / 3] = fgalpha;
                    break;
                case 'Y': // Yellow
                    symbol->bitmap[i] = 255;
                    symbol->bitmap[i + 1] = 255;
                    symbol->bitmap[i + 2] = 0;
                    if (plot_alpha) symbol->alphamap[i / 3] = fgalpha;
                    break;
                case 'G': // Green
                    symbol->bitmap[i] = 0;
                    symbol->bitmap[i + 1] = 255;
                    symbol->bitmap[i + 2] = 0;
                    if (plot_alpha) symbol->alphamap[i / 3] = fgalpha;
                    break;
                case 'K': // Black
                    symbol->bitmap[i] = 0;
                    symbol->bitmap[i + 1] = 0;
                    symbol->bitmap[i + 2] = 0;
                    if (plot_alpha) symbol->alphamap[i / 3] = fgalpha;
                    break;
                case DEFAULT_INK:
                    symbol->bitmap[i] = fgred;
                    symbol->bitmap[i + 1] = fggrn;
                    symbol->bitmap[i + 2] = fgblu;
                    if (plot_alpha) symbol->alphamap[i / 3] = fgalpha;
                    break;
                default: // DEFAULT_PAPER
                    symbol->bitmap[i] = bgred;
                    symbol->bitmap[i + 1] = bggrn;
                    symbol->bitmap[i + 2] = bgblu;
                    if (plot_alpha) symbol->alphamap[i / 3] = bgalpha;
                    break;
            }
        }
    }

    return 0;
}

static int save_raster_image_to_file(struct zint_symbol *symbol, int image_height, int image_width, char *pixelbuf, int rotate_angle, int image_type) {
    int error_number;
    int row, column;

    char *rotated_pixbuf = pixelbuf;

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

    if (rotate_angle) {
        if (!(rotated_pixbuf = (char *) malloc(image_width * image_height))) {
            strcpy(symbol->errtxt, "650: Insufficient memory for pixel buffer");
            return ZINT_ERROR_ENCODING_PROBLEM;
        }
    }

    /* Rotate image before plotting */
    switch (rotate_angle) {
        case 0: /* Plot the right way up */
            /* Nothing to do */
            break;
        case 90: /* Plot 90 degrees clockwise */
            for (row = 0; row < image_width; row++) {
                for (column = 0; column < image_height; column++) {
                    rotated_pixbuf[(row * image_height) + column] =
                            *(pixelbuf + (image_width * (image_height - column - 1)) + row);
                }
            }
            break;
        case 180: /* Plot upside down */
            for (row = 0; row < image_height; row++) {
                for (column = 0; column < image_width; column++) {
                    rotated_pixbuf[(row * image_width) + column] =
                            *(pixelbuf + (image_width * (image_height - row - 1)) + (image_width - column - 1));
                }
            }
            break;
        case 270: /* Plot 90 degrees anti-clockwise */
            for (row = 0; row < image_width; row++) {
                for (column = 0; column < image_height; column++) {
                    rotated_pixbuf[(row * image_height) + column] =
                            *(pixelbuf + (image_width * column) + (image_width - row - 1));
                }
            }
            break;
    }

    switch (image_type) {
        case OUT_BUFFER:
            error_number = buffer_plot(symbol, rotated_pixbuf);
            break;
        case OUT_PNG_FILE:
#ifndef NO_PNG
            error_number = png_pixel_plot(symbol, rotated_pixbuf);
#else
            if (rotate_angle) {
                free(rotated_pixbuf);
            }
            return ZINT_ERROR_INVALID_OPTION;
#endif
            break;
        case OUT_PCX_FILE:
            error_number = pcx_pixel_plot(symbol, rotated_pixbuf);
            break;
        case OUT_GIF_FILE:
            error_number = gif_pixel_plot(symbol, rotated_pixbuf);
            break;
        case OUT_TIF_FILE:
            error_number = tif_pixel_plot(symbol, rotated_pixbuf);
            break;
        default:
            error_number = bmp_pixel_plot(symbol, rotated_pixbuf);
            break;
    }

    if (rotate_angle) {
        free(rotated_pixbuf);
    }
    return error_number;
}

static void draw_bar(char *pixelbuf, int xpos, int xlen, int ypos, int ylen, int image_width, int image_height, char fill) {
    /* Draw a rectangle */
    int i, j, png_ypos;

    png_ypos = image_height - ypos - ylen;
    /* This fudge is needed because EPS measures height from the bottom up but
    PNG measures y position from the top down */

    for (i = (xpos); i < (xpos + xlen); i++) {
        for (j = (png_ypos); j < (png_ypos + ylen); j++) {
            *(pixelbuf + (image_width * j) + i) = fill;
        }
    }
}

static void draw_circle(char *pixelbuf, int image_width, int image_height, int x0, int y0, double radius, char fill) {
    int x, y;
    int radius_i = (int) radius;

    for (y = -radius_i; y <= radius_i; y++) {
        for (x = -radius_i; x <= radius_i; x++) {
            if ((x * x) + (y * y) <= (radius_i * radius_i)) {
                if ((y + y0 >= 0) && (y + y0 < image_height)
                        && (x + x0 >= 0) && (x + x0 < image_width)) {
                    *(pixelbuf + ((y + y0) * image_width) + (x + x0)) = fill;
                }
            }
        }
    }
}

static void draw_bullseye(char *pixelbuf, int image_width, int image_height, int xoffset, int yoffset, int scaler) {
    /* Central bullseye in Maxicode symbols */
    double x = 14.5 * scaler;
    double y = 15.0 * scaler;
    if(scaler < 10) {
        x = 16.0 * scaler;
        y = 16.5 * scaler;
    }

    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (4.571 * scaler) + 1, DEFAULT_INK);
    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (3.779 * scaler) + 1, DEFAULT_PAPER);
    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (2.988 * scaler) + 1, DEFAULT_INK);
    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (2.196 * scaler) + 1, DEFAULT_PAPER);
    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (1.394 * scaler) + 1, DEFAULT_INK);
    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (0.602 * scaler) + 1, DEFAULT_PAPER);
}

static void draw_hexagon(char *pixelbuf, int image_width, char *scaled_hexagon, int hexagon_size, int xposn, int yposn) {
    /* Put a hexagon into the pixel buffer */
    int i, j;

    for (i = 0; i < hexagon_size; i++) {
        for (j = 0; j < hexagon_size; j++) {
            if (scaled_hexagon[(i * hexagon_size) + j] == DEFAULT_INK) {
                *(pixelbuf + (image_width * i) + (image_width * yposn) + xposn + j) = DEFAULT_INK;
            }
        }
    }
}

static void draw_letter(char *pixelbuf, unsigned char letter, int xposn, int yposn, int textflags, int image_width, int image_height) {
    /* Put a letter into a position */
    int skip;

    skip = 0;

    if (letter < 33) {
        skip = 1;
    }

    if ((letter > 127) && (letter < 161)) {
        skip = 1;
    }

    if (xposn < 0 || yposn < 0) {
        skip = 1;
    }

    if (skip == 0) {
        int glyph_no;
        int x, y;
        if (letter > 128) {
            glyph_no = letter - 66;
        } else {
            glyph_no = letter - 33;
        }


        switch (textflags) {
            int max_x, max_y;
            case 1: // small font 5x9
                max_x = 5;
                max_y = 9;

                if (xposn + max_x >= image_width) {
                    max_x = image_width - xposn - 1;
                }

                if (yposn + max_y >= image_height) {
                    max_y = image_height - yposn - 1;
                }

                for (y = 0; y < max_y; y++) {
                    for (x = 0; x < max_x; x++) {
                        if (small_font[(glyph_no * 9) + y] & (0x10 >> x)) {
                            *(pixelbuf + (y * image_width) + (yposn * image_width) + xposn + x) = DEFAULT_INK;
                        }
                    }
                }
                break;

            case 2: // bold font -> twice the regular font
            {
                char * linePtr;
                max_x = 7;
                max_y = 14;

                if (xposn + max_x + 1 >= image_width) {
                    max_x = image_width - xposn - 2;
                }

                if (yposn + max_y >= image_height) {
                    max_y = image_height - yposn - 1;
                }

                linePtr = pixelbuf + (yposn * image_width) + xposn + 1;
                for (y = 0; y < max_y; y++) {
                    char * pixelPtr = linePtr;
                    int extra_dot = 0;
                    for (x = 0; x < max_x; x++) {
                        if (ascii_font[(glyph_no * 14) + y] & (0x40 >> x)) {
                            *pixelPtr = DEFAULT_INK;
                            extra_dot = 1;
                        } else {
                            if (extra_dot) {
                                *pixelPtr = DEFAULT_INK;
                            }

                            extra_dot = 0;
                        }

                        ++pixelPtr;
                    }

                    if (extra_dot) {
                        *pixelPtr = DEFAULT_INK;
                    }

                    linePtr += image_width;
                }
            }
                break;

            default: // regular font 7x14
                max_x = 7;
                max_y = 14;

                if (xposn + max_x >= image_width) {
                    max_x = image_width - xposn - 1;
                }

                if (yposn + max_y >= image_height) {
                    max_y = image_height - yposn - 1;
                }

                for (y = 0; y < max_y; y++) {
                    for (x = 0; x < max_x; x++) {
                        if (ascii_font[(glyph_no * 14) + y] & (0x40 >> x)) {
                            *(pixelbuf + (y * image_width) + (yposn * image_width) + xposn + x) = DEFAULT_INK;
                        }
                    }
                }
                break;
        }
    }
}

/* Plot a string into the pixel buffer */
static void draw_string(char *pixbuf, unsigned char input_string[], int xposn, int yposn, int textflags, int image_width, int image_height) {
    int i, string_length, string_left_hand, letter_width = 7;

    switch (textflags) {
        case 1: // small font 5x9
            letter_width = 5;
            break;

        case 2: // bold font -> width of the regular font + 1 extra dot + 1 extra space
            letter_width = 9;
            break;

        default: // regular font 7x15
            letter_width = 7;
            break;
    }

    string_length = ustrlen(input_string);
    string_left_hand = xposn - ((letter_width * string_length) / 2);

    for (i = 0; i < string_length; i++) {
        // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage) suppress (probable) false positive about 2nd arg input_string[i] being uninitialized
        draw_letter(pixbuf, input_string[i], string_left_hand + (i * letter_width), yposn, textflags, image_width, image_height);
    }
}

static void plot_hexline(char *scaled_hexagon, int hexagon_size, double start_x, double start_y, double end_x, double end_y) {
    /* Draw a straight line from start to end */
    int i;
    double inc_x, inc_y;

    inc_x = (end_x - start_x) / hexagon_size;
    inc_y = (end_y - start_y) / hexagon_size;

    for (i = 0; i < hexagon_size; i++) {
        double this_x = start_x + (i * inc_x);
        double this_y = start_y + (i * inc_y);
        if (((this_x >= 0) && (this_x < hexagon_size)) && ((this_y >= 0) && (this_y < hexagon_size))) {
                scaled_hexagon[(hexagon_size * (int)this_y) + (int)this_x] = DEFAULT_INK;
        }
    }
}

static void plot_hexagon(char *scaled_hexagon, int hexagon_size) {
    /* Create a hexagon shape and fill it */
    int line, i;

    double x_offset[6];
    double y_offset[6];
    double start_x, start_y;
    double end_x, end_y;

    x_offset[0] = 0.0;
    x_offset[1] = 0.86;
    x_offset[2] = 0.86;
    x_offset[3] = 0.0;
    x_offset[4] = -0.86;
    x_offset[5] = -0.86;

    y_offset[0] = 1.0;
    y_offset[1] = 0.5;
    y_offset[2] = -0.5;
    y_offset[3] = -1.0;
    y_offset[4] = -0.5;
    y_offset[5] = 0.5;

    /* Plot hexagon outline */
    for (line = 0; line < 5; line++) {
        start_x = (hexagon_size / 2.0) + ((hexagon_size / 2.0) * x_offset[line]);
        start_y = (hexagon_size / 2.0) + ((hexagon_size / 2.0) * y_offset[line]);
        end_x = (hexagon_size / 2.0) + ((hexagon_size / 2.0) * x_offset[line + 1]);
        end_y = (hexagon_size / 2.0) + ((hexagon_size / 2.0) * y_offset[line + 1]);
        plot_hexline(scaled_hexagon, hexagon_size, start_x, start_y, end_x, end_y);
    }
    start_x = (hexagon_size / 2.0) + ((hexagon_size / 2.0) * x_offset[line]);
    start_y = (hexagon_size / 2.0) + ((hexagon_size / 2.0) * y_offset[line]);
    end_x = (hexagon_size / 2.0) + ((hexagon_size / 2.0) * x_offset[0]);
    end_y = (hexagon_size / 2.0) + ((hexagon_size / 2.0) * y_offset[0]);
    plot_hexline(scaled_hexagon, hexagon_size, start_x, start_y, end_x, end_y);

    /* Fill hexagon */
    for (line = 0; line < hexagon_size; line++) {
        char ink = DEFAULT_PAPER;
        for (i = 0; i < hexagon_size; i++) {
            if (scaled_hexagon[(hexagon_size * line) + i] == DEFAULT_INK) {
                if (i < (hexagon_size / 2)) {
                    ink = DEFAULT_INK;
                } else {
                    ink = DEFAULT_PAPER;
                }
            }

            if (ink == DEFAULT_INK) {
                scaled_hexagon[(hexagon_size * line) + i] = ink;
            }
        }
    }
}

static int plot_raster_maxicode(struct zint_symbol *symbol, int rotate_angle, int data_type) {
    /* Plot a MaxiCode symbol with hexagons and bullseye */
    int row, column, xposn;
    int image_height, image_width;
    char *pixelbuf;
    int error_number;
    int xoffset, yoffset;
    int roffset, boffset;
    double scaler = symbol->scale;
    char *scaled_hexagon;
    int hexagon_size;

    if (scaler <= 0) {
        scaler = 0.5;
    }

    output_set_whitespace_offsets(symbol, &xoffset, &yoffset, &roffset, &boffset);

    image_width = ceil((300 + 2 * (xoffset + roffset)) * scaler);
    image_height = ceil((300 + 2 * (yoffset + boffset)) * scaler);

    if (!(pixelbuf = (char *) malloc(image_width * image_height))) {
        strcpy(symbol->errtxt, "655: Insufficient memory for pixel buffer");
        return ZINT_ERROR_ENCODING_PROBLEM;
    }
    memset(pixelbuf, DEFAULT_PAPER, image_width * image_height);

    hexagon_size = ceil(scaler * 10);

    if (!(scaled_hexagon = (char *) malloc(hexagon_size * hexagon_size))) {
        strcpy(symbol->errtxt, "656: Insufficient memory for pixel buffer");
        free(pixelbuf);
        return ZINT_ERROR_ENCODING_PROBLEM;
    }
    memset(scaled_hexagon, DEFAULT_PAPER, hexagon_size * hexagon_size);

    plot_hexagon(scaled_hexagon, hexagon_size);

    for (row = 0; row < symbol->rows; row++) {
        int yposn = row * 9;
        for (column = 0; column < symbol->width; column++) {
            xposn = column * 10;
            if (module_is_set(symbol, row, column)) {
                if (row & 1) {
                    /* Odd (reduced) row */
                    xposn += 5;
                    draw_hexagon(pixelbuf, image_width, scaled_hexagon, hexagon_size, (xposn + (2 * xoffset)) * scaler, (yposn + (2 * yoffset)) * scaler);
                } else {
                    /* Even (full) row */
                    draw_hexagon(pixelbuf, image_width, scaled_hexagon, hexagon_size, (xposn + (2 * xoffset)) * scaler, (yposn + (2 * yoffset)) * scaler);
                }
            }
        }
    }

    draw_bullseye(pixelbuf, image_width, image_height, (2.0 * xoffset), (2.0 * yoffset), scaler * 10);

    // Virtual hexagon
    //draw_hexagon(pixelbuf, image_width, scaled_hexagon, hexagon_size, ((14 * 10) + (2 * xoffset)) * scaler, ((16 * 9) + (2 * yoffset)) * scaler);

    if (symbol->border_width > 0) {
        if ((symbol->output_options & BARCODE_BOX) || (symbol->output_options & BARCODE_BIND)) {
            /* boundary bars */
            draw_bar(pixelbuf, 0, image_width, 0, symbol->border_width * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, 0, image_width, 300 + (symbol->border_width * 2), symbol->border_width * 2, image_width, image_height, DEFAULT_INK);
        }

        if (symbol->output_options & BARCODE_BOX) {
            /* side bars */
            draw_bar(pixelbuf, 0, symbol->border_width * 2, 0, image_height, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, 300 + ((symbol->border_width + symbol->whitespace_width + symbol->whitespace_width) * 2), symbol->border_width * 2, 0, image_height, image_width, image_height, DEFAULT_INK);
        }
    }

    error_number = save_raster_image_to_file(symbol, image_height, image_width, pixelbuf, rotate_angle, data_type);
    free(scaled_hexagon);
    free(pixelbuf);
    return error_number;
}

static int plot_raster_dotty(struct zint_symbol *symbol, int rotate_angle, int data_type) {
    double scaler = 2 * symbol->scale;
    double half_scaler, dot_size_scaled;
    char *scaled_pixelbuf;
    int r, i;
    int scale_width, scale_height;
    int error_number = 0;
    int xoffset, yoffset, image_width, image_height;
    int roffset, boffset;

    symbol->height = symbol->rows; // This is true because only 2d matrix symbols are processed here

    output_set_whitespace_offsets(symbol, &xoffset, &yoffset, &roffset, &boffset);

    image_width = symbol->width + xoffset + roffset;
    image_height = symbol->height + yoffset + boffset;

    if (scaler < 2.0) {
        scaler = 2.0;
    }
    scale_width = (image_width * scaler) + 1;
    scale_height = (image_height * scaler) + 1;

    /* Apply scale options by creating another pixel buffer */
    if (!(scaled_pixelbuf = (char *) malloc(scale_width * scale_height))) {
        strcpy(symbol->errtxt, "657: Insufficient memory for pixel buffer");
        return ZINT_ERROR_ENCODING_PROBLEM;
    }
    memset(scaled_pixelbuf, DEFAULT_PAPER, scale_width * scale_height);

    /* Plot the body of the symbol to the pixel buffer */
    half_scaler = scaler / 2.0;
    dot_size_scaled = (symbol->dot_size * scaler) / 2.0;
    for (r = 0; r < symbol->rows; r++) {
        double row_scaled = (r + yoffset) * scaler + half_scaler;
        for (i = 0; i < symbol->width; i++) {
            if (module_is_set(symbol, r, i)) {
                draw_circle(scaled_pixelbuf, scale_width, scale_height,
                        (i + xoffset) * scaler + half_scaler,
                        row_scaled,
                        dot_size_scaled,
                        DEFAULT_INK);
            }
        }
    }

    error_number = save_raster_image_to_file(symbol, scale_height, scale_width, scaled_pixelbuf, rotate_angle, data_type);
    free(scaled_pixelbuf);

    return error_number;
}

/* Convert UTF-8 to ISO 8859-1 for draw_string() human readable text */
static void to_iso8859_1(const unsigned char source[], unsigned char preprocessed[]) {
    int j, i, input_length;

    input_length = ustrlen(source);

    j = 0;
    i = 0;
    while (i < input_length) {
        switch (source[i]) {
            case 0xC2:
                /* UTF-8 C2xxh */
                /* Character range: C280h (latin: 80h) to C2BFh (latin: BFh) */
                assert(i + 1 < input_length);
                i++;
                preprocessed[j] = source[i];
                j++;
                break;
            case 0xC3:
                /* UTF-8 C3xx */
                /* Character range: C380h (latin: C0h) to C3BFh (latin: FFh) */
                assert(i + 1 < input_length);
                i++;
                preprocessed[j] = source[i] + 64;
                j++;
                break;
            default:
                /* Process ASCII (< 80h), all other unicode points are ignored */
                if (source[i] < 128) {
                    preprocessed[j] = source[i];
                    j++;
                }
                break;
        }
        i++;
    }
    preprocessed[j] = '\0';

    return;
}

static int plot_raster_default(struct zint_symbol *symbol, int rotate_angle, int data_type) {
    int error_number;
    double large_bar_height;
    int textdone;
    int main_width, comp_offset, addon_gap;
    unsigned char addon[6];
    int xoffset, yoffset, roffset, boffset;
    double addon_text_posn;
    int textoffset;
    int default_text_posn;
    double row_height, row_posn;
    int upceanflag = 0;
    int addon_latch = 0;
    unsigned char textpart1[5], textpart2[7], textpart3[7], textpart4[2];
    int textpos;
    int hide_text = 0;
    int i, r;

    int textflags = 0;
    int image_width, image_height;
    char *pixelbuf;
    int next_yposn;
    int latch;
    int block_width;
    double scaler = symbol->scale;
    int scale_width, scale_height;
    char *scaled_pixelbuf;
    int horiz, vert;

    large_bar_height = output_large_bar_height(symbol);
    textdone = 0;

    main_width = symbol->width;
    comp_offset = 0;

    if (is_extendable(symbol->symbology)) {
        upceanflag = output_process_upcean(symbol, &main_width, &comp_offset, addon, &addon_gap);
    }

    output_set_whitespace_offsets(symbol, &xoffset, &yoffset, &roffset, &boffset);

    addon_text_posn = 0.0;
    hide_text = ((!symbol->show_hrt) || (ustrlen(symbol->text) == 0));

    if (symbol->output_options & SMALL_TEXT) {
        textflags = 1;
    } else if (symbol->output_options & BOLD_TEXT) {
        textflags = 2;
    }

    if (ustrlen(symbol->text) != 0) {
        textoffset = 9;
    } else {
        textoffset = 0;
    }

    image_width = 2 * (symbol->width + xoffset + roffset);
    image_height = 2 * (symbol->height + textoffset + yoffset + boffset);

    if (!(pixelbuf = (char *) malloc(image_width * image_height))) {
        strcpy(symbol->errtxt, "658: Insufficient memory for pixel buffer");
        return ZINT_ERROR_ENCODING_PROBLEM;
    }
    memset(pixelbuf, DEFAULT_PAPER, image_width * image_height);

    default_text_posn = image_height - 17;

    row_height = 0.0;
    row_posn = textoffset + yoffset;
    next_yposn = textoffset + yoffset;

    /* Plot the body of the symbol to the pixel buffer */
    for (r = 0; r < symbol->rows; r++) {
        int plot_yposn;
        int plot_height;
        int this_row = symbol->rows - r - 1; /* invert r otherwise plots upside down */
        int module_fill;
        row_posn += row_height;
        plot_yposn = next_yposn;
        if (symbol->row_height[this_row] == 0) {
            row_height = large_bar_height;
        } else {
            row_height = symbol->row_height[this_row];
        }
        next_yposn = (int) (row_posn + row_height);
        plot_height = next_yposn - plot_yposn;

        i = 0;

        do {
            module_fill = module_is_set(symbol, this_row, i);
            block_width = 0;
            do {
                block_width++;
            } while ((i + block_width < symbol->width) && module_is_set(symbol, this_row, i + block_width) == module_is_set(symbol, this_row, i));

            if ((addon_latch == 0) && (r == 0) && (i > main_width)) {
                if (upceanflag == 12 || upceanflag == 6) { /* UPC-A/E add-ons don't descend */
                    plot_height = row_height > 8.0 ? row_height - 8.0 : 1;
                    plot_yposn = row_posn;
                } else {
                    plot_height = row_height > 3.0 ? row_height - 3.0 : 1;
                    plot_yposn = row_posn - 5;
                }
                addon_latch = 1;
            }
            if (module_fill) {
                /* a bar */
                if (symbol->symbology == BARCODE_ULTRA) {
                    draw_bar(pixelbuf, (i + xoffset) * 2, block_width * 2, plot_yposn * 2, plot_height * 2, image_width, image_height, ultra_colour[module_fill]);
                } else {
                    draw_bar(pixelbuf, (i + xoffset) * 2, block_width * 2, plot_yposn * 2, plot_height * 2, image_width, image_height, DEFAULT_INK);
                }
            }
            i += block_width;

        } while (i < symbol->width);
    }

    xoffset += comp_offset;

    if (upceanflag) {
        /* Guard bar extension */

        if (upceanflag == 6) { /* UPC-E */
            draw_bar(pixelbuf, (0 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (2 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (46 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (48 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (50 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);

        } else if (upceanflag == 8) { /* EAN-8 */
            draw_bar(pixelbuf, (0 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (2 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (32 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (34 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (64 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (66 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);

        } else if (upceanflag == 12) { /* UPC-A */
            latch = 1;

            i = 0 + comp_offset;
            do {
                block_width = 0;
                do {
                    block_width++;
                } while ((i + block_width < symbol->width) && module_is_set(symbol, symbol->rows - 1, i + block_width) == module_is_set(symbol, symbol->rows - 1, i));
                if (latch == 1) {
                    /* a bar */
                    draw_bar(pixelbuf, (i + xoffset - comp_offset) * 2, block_width * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
                    latch = 0;
                } else {
                    /* a space */
                    latch = 1;
                }
                i += block_width;
            } while (i < 11 + comp_offset);
            draw_bar(pixelbuf, (46 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (48 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            latch = 1;
            i = 85 + comp_offset;
            do {
                block_width = 0;
                do {
                    block_width++;
                } while ((i + block_width < symbol->width) && module_is_set(symbol, symbol->rows - 1, i + block_width) == module_is_set(symbol, symbol->rows - 1, i));
                if (latch == 1) {
                    /* a bar */
                    draw_bar(pixelbuf, (i + xoffset - comp_offset) * 2, block_width * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
                    latch = 0;
                } else {
                    /* a space */
                    latch = 1;
                }
                i += block_width;
            } while (i < 96 + comp_offset);

        } else if (upceanflag == 13) { /* EAN-13 */
            draw_bar(pixelbuf, (0 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (2 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (46 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (48 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (92 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (94 + xoffset) * 2, 1 * 2, (4 + yoffset) * 2, 5 * 2, image_width, image_height, DEFAULT_INK);
        }
    }

    if (!hide_text) {

        if (upceanflag) {
            output_upcean_split_text(upceanflag, symbol->text, textpart1, textpart2, textpart3, textpart4);

            if (upceanflag == 6) { /* UPC-E */
                textpos = 2 * (-5 + xoffset);
                draw_string(pixelbuf, textpart1, textpos, default_text_posn, textflags, image_width, image_height);
                textpos = 2 * (24 + xoffset);
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height);
                textpos = 2 * (55 + xoffset);
                draw_string(pixelbuf, textpart3, textpos, default_text_posn, textflags, image_width, image_height);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 2 * (61 + xoffset + addon_gap);
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height);
                        break;
                    case 5:
                        textpos = 2 * (75 + xoffset + addon_gap);
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height);
                        break;
                }

            } else if (upceanflag == 8) { /* EAN-8 */
                textpos = 2 * (17 + xoffset);
                draw_string(pixelbuf, textpart1, textpos, default_text_posn, textflags, image_width, image_height);
                textpos = 2 * (50 + xoffset);
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 2 * (77 + xoffset + addon_gap);
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height);
                        break;
                    case 5:
                        textpos = 2 * (91 + xoffset + addon_gap);
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height);
                        break;
                }

            } else if (upceanflag == 12) { /* UPC-A */
                textpos = 2 * (-5 + xoffset);
                draw_string(pixelbuf, textpart1, textpos, default_text_posn, textflags, image_width, image_height);
                textpos = 2 * (27 + xoffset);
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height);
                textpos = 2 * (68 + xoffset);
                draw_string(pixelbuf, textpart3, textpos, default_text_posn, textflags, image_width, image_height);
                textpos = 2 * (100 + xoffset);
                draw_string(pixelbuf, textpart4, textpos, default_text_posn, textflags, image_width, image_height);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 2 * (107 + xoffset + addon_gap);
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height);
                        break;
                    case 5:
                        textpos = 2 * (121 + xoffset + addon_gap);
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height);
                        break;
                }

            } else if (upceanflag == 13) { /* EAN-13 */
                textpos = 2 * (-7 + xoffset);
                draw_string(pixelbuf, textpart1, textpos, default_text_posn, textflags, image_width, image_height);
                textpos = 2 * (24 + xoffset);
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height);
                textpos = 2 * (71 + xoffset);
                draw_string(pixelbuf, textpart3, textpos, default_text_posn, textflags, image_width, image_height);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 2 * (105 + xoffset + addon_gap);
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height);
                        break;
                    case 5:
                        textpos = 2 * (119 + xoffset + addon_gap);
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height);
                        break;
                }
            }
        }

        if (!textdone) {
            unsigned char local_text[sizeof(symbol->text)];
            to_iso8859_1(symbol->text, local_text);
            /* Put the human readable text at the bottom */
            textpos = 2 * (main_width / 2 + xoffset);
            draw_string(pixelbuf, local_text, textpos, default_text_posn, textflags, image_width, image_height);
        }
    }

    xoffset -= comp_offset;

    // Binding and boxes
    if ((symbol->output_options & BARCODE_BIND) != 0) {
        if ((symbol->rows > 1) && (is_stackable(symbol->symbology) == 1)) {
            double sep_height = 1;
            if (symbol->option_3 > 0 && symbol->option_3 <= 4) {
                sep_height = symbol->option_3;
            }
            /* row binding */
            if (symbol->symbology != BARCODE_CODABLOCKF && symbol->symbology != BARCODE_HIBC_BLOCKF) {
                for (r = 1; r < symbol->rows; r++) {
                    draw_bar(pixelbuf, xoffset * 2, symbol->width * 2, ((r * row_height) + textoffset + yoffset - sep_height / 2) * 2, sep_height * 2, image_width, image_height, DEFAULT_INK);
                }
            } else {
                for (r = 1; r < symbol->rows; r++) {
                    /* Avoid 11-module start and 13-module stop chars */
                    draw_bar(pixelbuf, (xoffset + 11) * 2, (symbol->width - 24) * 2, ((r * row_height) + textoffset + yoffset - sep_height / 2) * 2, sep_height * 2, image_width, image_height, DEFAULT_INK);
                }
            }
        }
    }
    if (symbol->border_width > 0) {
        if ((symbol->output_options & BARCODE_BOX) || (symbol->output_options & BARCODE_BIND)) {
            /* boundary bars */
            if ((symbol->output_options & BARCODE_BOX) || (symbol->symbology != BARCODE_CODABLOCKF && symbol->symbology != BARCODE_HIBC_BLOCKF)) {
                draw_bar(pixelbuf, 0, (symbol->width + xoffset + roffset) * 2, textoffset * 2, symbol->border_width * 2, image_width, image_height, DEFAULT_INK);
                draw_bar(pixelbuf, 0, (symbol->width + xoffset + roffset) * 2, (textoffset + symbol->height + symbol->border_width) * 2, symbol->border_width * 2, image_width, image_height, DEFAULT_INK);
            } else {
                draw_bar(pixelbuf, xoffset * 2, symbol->width * 2, textoffset * 2, symbol->border_width * 2, image_width, image_height, DEFAULT_INK);
                draw_bar(pixelbuf, xoffset * 2, symbol->width * 2, (textoffset + symbol->height + symbol->border_width) * 2, symbol->border_width * 2, image_width, image_height, DEFAULT_INK);
            }
        }
        if ((symbol->output_options & BARCODE_BOX)) {
            /* side bars */
            draw_bar(pixelbuf, 0, symbol->border_width * 2, textoffset * 2, (symbol->height + (2 * symbol->border_width)) * 2, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (symbol->width + xoffset + roffset - symbol->border_width) * 2, symbol->border_width * 2, textoffset * 2, (symbol->height + (2 * symbol->border_width)) * 2, image_width, image_height, DEFAULT_INK);
        }
    }

    if (scaler <= 0) {
        scaler = 0.5;
    }

    if (scaler != 1.0) {
        scale_width = image_width * scaler;
        scale_height = image_height * scaler;

        /* Apply scale options by creating another pixel buffer */
        if (!(scaled_pixelbuf = (char *) malloc(scale_width * scale_height))) {
            free(pixelbuf);
            strcpy(symbol->errtxt, "659: Insufficient memory for pixel buffer");
            return ZINT_ERROR_ENCODING_PROBLEM;
        }
        memset(scaled_pixelbuf, DEFAULT_PAPER, scale_width * scale_height);

        for (vert = 0; vert < scale_height; vert++) {
            int vert_row = vert * scale_width;
            int image_vert_row = ((int) (vert / scaler)) * image_width;
            for (horiz = 0; horiz < scale_width; horiz++) {
                *(scaled_pixelbuf + vert_row + horiz) = *(pixelbuf + image_vert_row + (int) (horiz / scaler));
            }
        }

        error_number = save_raster_image_to_file(symbol, scale_height, scale_width, scaled_pixelbuf, rotate_angle, data_type);
        free(scaled_pixelbuf);
    } else {
        error_number = save_raster_image_to_file(symbol, image_height, image_width, pixelbuf, rotate_angle, data_type);
    }
    free(pixelbuf);
    return error_number;
}

INTERNAL int plot_raster(struct zint_symbol *symbol, int rotate_angle, int file_type) {
    int error;

#ifdef NO_PNG
    if (file_type == OUT_PNG_FILE) {
        strcpy(symbol->errtxt, "660: PNG format disabled at compile time");
        return ZINT_ERROR_INVALID_OPTION;
    }
#endif /* NO_PNG */

    error = output_check_colour_options(symbol);
    if (error != 0) {
        return error;
    }

    if (symbol->output_options & BARCODE_DOTTY_MODE) {
        error = plot_raster_dotty(symbol, rotate_angle, file_type);
    } else {
        if (symbol->symbology == BARCODE_MAXICODE) {
            error = plot_raster_maxicode(symbol, rotate_angle, file_type);
        } else {
            error = plot_raster_default(symbol, rotate_angle, file_type);
        }
    }

    return error;
}
