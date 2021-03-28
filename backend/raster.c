/* raster.c - Handles output to raster files */

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

#include <stdio.h>
#ifdef _MSC_VER
#include <malloc.h>
#include <fcntl.h>
#include <io.h>
/* For Visual C++ 6 suppress conversion from int to float warning */
#if _MSC_VER == 1200
#pragma warning(disable: 4244)
#endif
#endif
#include <math.h>
#include <assert.h>
#include "common.h"
#include "output.h"

#include "font.h" /* Font for human readable text */

#define SSET    "0123456789ABCDEF"

#define DEFAULT_INK '1'
#define DEFAULT_PAPER '0'

#define UPCEAN_TEXT 1

#ifndef NO_PNG
INTERNAL int png_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);
#endif /* NO_PNG */
INTERNAL int bmp_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);
INTERNAL int pcx_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);
INTERNAL int gif_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);
INTERNAL int tif_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);

static const char ultra_colour[] = "0CBMRYGKW";

static int buffer_plot(struct zint_symbol *symbol, unsigned char *pixelbuf) {
    /* Place pixelbuffer into symbol */
    int fgalpha, bgalpha;
    unsigned char fg[3], bg[3];
    unsigned char white[3] =   { 0xff, 0xff, 0xff };
    unsigned char cyan[3] =    {    0, 0xff, 0xff };
    unsigned char blue[3] =    {    0,    0, 0xff };
    unsigned char magenta[3] = { 0xff,    0, 0xff };
    unsigned char red[3] =     { 0xff,    0,    0 };
    unsigned char yellow[3] =  { 0xff, 0xff,    0 };
    unsigned char green[3] =   {    0, 0xff,    0 };
    unsigned char black[3] =   {    0,    0,    0 };
    unsigned char *map[91] = {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* 0x00-0F */
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* 0x10-1F */
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* 0x20-2F */
        bg, fg, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* 0-9 */
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* :;<=>?@ */
        NULL, blue, cyan, NULL, NULL, NULL, green, NULL, NULL, NULL, black, NULL, magenta, /* A-M */
        NULL, NULL, NULL, NULL, red, NULL, NULL, NULL, NULL, white, NULL, yellow, NULL /* N-Z */
    };
    int row, column;
    int plot_alpha = 0;
    unsigned char *bitmap;

    fg[0] = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
    fg[1] = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
    fg[2] = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
    bg[0] = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
    bg[1] = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
    bg[2] = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);

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

    symbol->bitmap = (unsigned char *) malloc((size_t) symbol->bitmap_width * symbol->bitmap_height * 3);
    if (symbol->bitmap == NULL) {
        strcpy(symbol->errtxt, "661: Insufficient memory for bitmap buffer");
        return ZINT_ERROR_MEMORY;
    }

    if (plot_alpha) {
        symbol->alphamap = (unsigned char *) malloc((size_t) symbol->bitmap_width * symbol->bitmap_height);
        if (symbol->alphamap == NULL) {
            strcpy(symbol->errtxt, "662: Insufficient memory for alphamap buffer");
            return ZINT_ERROR_MEMORY;
        }
        for (row = 0; row < symbol->bitmap_height; row++) {
            int p = row * symbol->bitmap_width;
            bitmap = symbol->bitmap + p * 3;
            for (column = 0; column < symbol->bitmap_width; column++, p++, bitmap += 3) {
                memcpy(bitmap, map[pixelbuf[p]], 3);
                symbol->alphamap[p] = pixelbuf[p] == DEFAULT_PAPER ? bgalpha : fgalpha;
            }
        }
    } else {
        for (row = 0; row < symbol->bitmap_height; row++) {
            int r = row * symbol->bitmap_width;
            unsigned char *pb = pixelbuf + r;
            bitmap = symbol->bitmap + r * 3;
            for (column = 0; column < symbol->bitmap_width; column++, pb++, bitmap += 3) {
                memcpy(bitmap, map[*pb], 3);
            }
        }
    }

    return 0;
}

static int save_raster_image_to_file(struct zint_symbol *symbol, int image_height, int image_width, unsigned char *pixelbuf, int rotate_angle, int file_type) {
    int error_number;
    int row, column;

    unsigned char *rotated_pixbuf = pixelbuf;

    assert(rotate_angle == 0 || rotate_angle == 90 || rotate_angle == 180 || rotate_angle == 270); /* Suppress clang-analyzer-core.UndefinedBinaryOperatorResult warning */
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
        if (!(rotated_pixbuf = (unsigned char *) malloc((size_t) image_width * image_height))) {
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

    switch (file_type) {
        case OUT_BUFFER:
            if (symbol->output_options & OUT_BUFFER_INTERMEDIATE) {
                if (symbol->bitmap != NULL) {
                    free(symbol->bitmap);
                    symbol->bitmap = NULL;
                }
                if (symbol->alphamap != NULL) {
                    free(symbol->alphamap);
                    symbol->alphamap = NULL;
                }
                symbol->bitmap = rotated_pixbuf;
                rotate_angle = 0; /* Suppress freeing buffer if rotated */
                error_number = 0;
            } else {
                error_number = buffer_plot(symbol, rotated_pixbuf);
            }
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

static void draw_bar(unsigned char *pixelbuf, int xpos, int xlen, int ypos, int ylen, int image_width, int image_height, char fill) {
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

static void draw_circle(unsigned char *pixelbuf, int image_width, int image_height, int x0, int y0, float radius, char fill) {
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

static void draw_bullseye(unsigned char *pixelbuf, int image_width, int image_height, int xoffset, int yoffset, int scaler) {
    /* Central bullseye in Maxicode symbols */
    float x = 14.5f * scaler;
    float y = 15.0f * scaler;

    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (4.571f * scaler) + 1.0f, DEFAULT_INK);
    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (3.779f * scaler) + 1.0f, DEFAULT_PAPER);
    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (2.988f * scaler) + 1.0f, DEFAULT_INK);
    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (2.196f * scaler) + 1.0f, DEFAULT_PAPER);
    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (1.394f * scaler) + 1.0f, DEFAULT_INK);
    draw_circle(pixelbuf, image_width, image_height, x + xoffset, y + yoffset, (0.602f * scaler) + 1.0f, DEFAULT_PAPER);
}

static void draw_hexagon(unsigned char *pixelbuf, int image_width, unsigned char *scaled_hexagon, int hexagon_size, int xposn, int yposn) {
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

static void draw_letter(unsigned char *pixelbuf, unsigned char letter, int xposn, int yposn, int textflags, int image_width, int image_height, int si) {
    /* Put a letter into a position */
    int skip;

    skip = 0;

    if (letter < 33) {
        skip = 1;
    }

    if ((letter >= 127) && (letter < 161)) {
        skip = 1;
    }

    if ((textflags & UPCEAN_TEXT) && (letter < '0' || letter > '9')) {
        skip = 1;
    }

    if (yposn < 0) { /* Allow xposn < 0, dealt with below */
        skip = 1;
    }

    if (skip == 0) {
        int glyph_no;
        int x, y;
        int max_x, max_y;
        font_item *font_table;
        int bold = 0;
        unsigned glyph_mask;
        int font_y;
        int half_si = si / 2;
        int odd_si = si & 1;
        unsigned char *linePtr, *maxPtr;
        int x_start = 0;

        if (letter > 127) {
            glyph_no = letter - 67; /* 161 - (127 - 33) */
        } else {
            glyph_no = letter - 33;
        }

        if (textflags & UPCEAN_TEXT) { /* Needs to be before SMALL_TEXT check */
            /* No bold for UPCEAN */
            if (textflags & SMALL_TEXT) {
                font_table = upcean_small_font;
                max_x = UPCEAN_SMALL_FONT_WIDTH;
                max_y = UPCEAN_SMALL_FONT_HEIGHT;
            } else {
                font_table = upcean_font;
                max_x = UPCEAN_FONT_WIDTH;
                max_y = UPCEAN_FONT_HEIGHT;
            }
            glyph_no = letter - '0';
        } else if (textflags & SMALL_TEXT) { // small font 5x9
            /* No bold for small */
            max_x = SMALL_FONT_WIDTH;
            max_y = SMALL_FONT_HEIGHT;
            font_table = small_font;
        } else if (textflags & BOLD_TEXT) { // bold font -> regular font + 1
            max_x = NORMAL_FONT_WIDTH + 1;
            max_y = NORMAL_FONT_HEIGHT;
            font_table = ascii_font;
            bold = 1;
        } else { // regular font 7x14
            max_x = NORMAL_FONT_WIDTH;
            max_y = NORMAL_FONT_HEIGHT;
            font_table = ascii_font;
        }
        glyph_mask = ((unsigned) 1) << (max_x - 1);
        font_y = glyph_no * max_y;

        if (xposn < 0) {
            x_start = -xposn;
            xposn = 0;
        }

        if (yposn + max_y > image_height) {
            max_y = image_height - yposn;
        }

        linePtr = pixelbuf + (yposn * image_width) + xposn;
        for (y = 0; y < max_y; y++) {
            int x_si, y_si;
            unsigned char *pixelPtr = linePtr; /* Avoid warning */
            for (y_si = 0; y_si < half_si; y_si++) {
                int extra_dot = 0;
                pixelPtr = linePtr;
                maxPtr = linePtr + image_width - xposn;
                for (x = x_start; x < max_x && pixelPtr < maxPtr; x++) {
                    unsigned set = font_table[font_y + y] & (glyph_mask >> x);
                    for (x_si = 0; x_si < half_si && pixelPtr < maxPtr; x_si++) {
                        if (set) {
                            *pixelPtr = DEFAULT_INK;
                            extra_dot = bold;
                        } else if (extra_dot) {
                            *pixelPtr = DEFAULT_INK;
                            extra_dot = 0;
                        }
                        pixelPtr++;
                    }
                    if (pixelPtr < maxPtr && odd_si && (x & 1)) {
                        if (set) {
                            *pixelPtr = DEFAULT_INK;
                        }
                        pixelPtr++;
                    }
                }
                if (pixelPtr < maxPtr && extra_dot) {
                    *pixelPtr++ = DEFAULT_INK;
                }
                linePtr += image_width;
            }
            if (odd_si && (y & 1)) {
                memcpy(linePtr, linePtr - image_width, pixelPtr - (linePtr - image_width));
                linePtr += image_width;
            }
        }
    }
}

/* Plot a string into the pixel buffer */
static void draw_string(unsigned char *pixbuf, unsigned char input_string[], int xposn, int yposn, int textflags, int image_width, int image_height, int si) {
    int i, string_length, string_left_hand, letter_width, letter_gap;
    int half_si = si / 2, odd_si = si & 1, x_incr;

    if (textflags & UPCEAN_TEXT) { /* Needs to be before SMALL_TEXT check */
        /* No bold for UPCEAN */
        letter_width = textflags & SMALL_TEXT ? UPCEAN_SMALL_FONT_WIDTH : UPCEAN_FONT_WIDTH;
        letter_gap = 4;
    } else if (textflags & SMALL_TEXT) { // small font 5x9
        /* No bold for small */
        letter_width = SMALL_FONT_WIDTH;
        letter_gap = 0;
    } else if (textflags & BOLD_TEXT) { // bold font -> width of the regular font + 1 extra dot + 1 extra space
        letter_width = NORMAL_FONT_WIDTH + 1;
        letter_gap = 1;
    } else { // regular font 7x15
        letter_width = NORMAL_FONT_WIDTH;
        letter_gap = 0;
    }
    letter_width += letter_gap;

    string_length = ustrlen(input_string);

    string_left_hand = xposn - ((letter_width * string_length - letter_gap) * half_si) / 2;
    if (odd_si) {
        string_left_hand -= (letter_width * string_length - letter_gap) / 4;
    }
    for (i = 0; i < string_length; i++) {
        x_incr = i * letter_width * half_si;
        if (odd_si) {
            x_incr += i * letter_width / 2;
        }
        draw_letter(pixbuf, input_string[i], string_left_hand + x_incr, yposn, textflags, image_width, image_height, si);
    }
}

static void plot_hexline(unsigned char *scaled_hexagon, int hexagon_size, float start_x, float start_y, float end_x, float end_y) {
    /* Draw a straight line from start to end */
    int i;
    float inc_x, inc_y;

    inc_x = (end_x - start_x) / hexagon_size;
    inc_y = (end_y - start_y) / hexagon_size;

    for (i = 0; i < hexagon_size; i++) {
        float this_x = start_x + (i * inc_x);
        float this_y = start_y + (i * inc_y);
        if (((this_x >= 0) && (this_x < hexagon_size)) && ((this_y >= 0) && (this_y < hexagon_size))) {
                scaled_hexagon[(hexagon_size * (int)this_y) + (int)this_x] = DEFAULT_INK;
        }
    }
}

static void plot_hexagon(unsigned char *scaled_hexagon, int hexagon_size) {
    /* Create a hexagon shape and fill it */
    int line, i;

    float x_offset[6];
    float y_offset[6];
    float start_x, start_y;
    float end_x, end_y;

    x_offset[0] = 0.0f;
    x_offset[1] = 0.86f;
    x_offset[2] = 0.86f;
    x_offset[3] = 0.0f;
    x_offset[4] = -0.86f;
    x_offset[5] = -0.86f;

    y_offset[0] = 1.0f;
    y_offset[1] = 0.5f;
    y_offset[2] = -0.5f;
    y_offset[3] = -1.0f;
    y_offset[4] = -0.5f;
    y_offset[5] = 0.5f;

    /* Plot hexagon outline */
    for (line = 0; line < 5; line++) {
        start_x = (hexagon_size / 2.0f) + ((hexagon_size / 2.0f) * x_offset[line]);
        start_y = (hexagon_size / 2.0f) + ((hexagon_size / 2.0f) * y_offset[line]);
        end_x = (hexagon_size / 2.0f) + ((hexagon_size / 2.0f) * x_offset[line + 1]);
        end_y = (hexagon_size / 2.0f) + ((hexagon_size / 2.0f) * y_offset[line + 1]);
        plot_hexline(scaled_hexagon, hexagon_size, start_x, start_y, end_x, end_y);
    }
    start_x = (hexagon_size / 2.0f) + ((hexagon_size / 2.0f) * x_offset[line]);
    start_y = (hexagon_size / 2.0f) + ((hexagon_size / 2.0f) * y_offset[line]);
    end_x = (hexagon_size / 2.0f) + ((hexagon_size / 2.0f) * x_offset[0]);
    end_y = (hexagon_size / 2.0f) + ((hexagon_size / 2.0f) * y_offset[0]);
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

static int plot_raster_maxicode(struct zint_symbol *symbol, int rotate_angle, int file_type) {
    /* Plot a MaxiCode symbol with hexagons and bullseye */
    int row, column, xposn;
    int image_height, image_width;
    unsigned char *pixelbuf;
    int error_number;
    int xoffset, yoffset, roffset, boffset;
    float scaler = symbol->scale;
    unsigned char *scaled_hexagon;
    int hexagon_size;

    if (scaler < 0.5f) {
        scaler = 0.5f;
    }

    output_set_whitespace_offsets(symbol, &xoffset, &yoffset, &roffset, &boffset);

    image_width = ceil((double) (300 + 2 * (xoffset + roffset)) * scaler);
    image_height = ceil((double) (300 + 2 * (yoffset + boffset)) * scaler);

    if (!(pixelbuf = (unsigned char *) malloc((size_t) image_width * image_height))) {
        strcpy(symbol->errtxt, "655: Insufficient memory for pixel buffer");
        return ZINT_ERROR_ENCODING_PROBLEM;
    }
    memset(pixelbuf, DEFAULT_PAPER, (size_t) image_width * image_height);

    hexagon_size = ceil(scaler * 10);

    if (!(scaled_hexagon = (unsigned char *) malloc((size_t) hexagon_size * hexagon_size))) {
        strcpy(symbol->errtxt, "656: Insufficient memory for pixel buffer");
        free(pixelbuf);
        return ZINT_ERROR_ENCODING_PROBLEM;
    }
    memset(scaled_hexagon, DEFAULT_PAPER, (size_t) hexagon_size * hexagon_size);

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

    draw_bullseye(pixelbuf, image_width, image_height, (2 * xoffset), (2 * yoffset), scaler * 10);

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

    error_number = save_raster_image_to_file(symbol, image_height, image_width, pixelbuf, rotate_angle, file_type);
    free(scaled_hexagon);
    if (rotate_angle || file_type != OUT_BUFFER || !(symbol->output_options & OUT_BUFFER_INTERMEDIATE)) {
        free(pixelbuf);
    }
    return error_number;
}

static int plot_raster_dotty(struct zint_symbol *symbol, int rotate_angle, int file_type) {
    float scaler = 2 * symbol->scale;
    unsigned char *scaled_pixelbuf;
    int r, i;
    int scale_width, scale_height;
    int error_number = 0;
    int xoffset, yoffset, roffset, boffset;
    float dot_overspill;
    float dotoffset;
    float dotradius_scaled;
    int dot_overspill_scaled;

    if (scaler < 2.0f) {
        scaler = 2.0f;
    }

    symbol->height = symbol->rows; // This is true because only 2d matrix symbols are processed here

    output_set_whitespace_offsets(symbol, &xoffset, &yoffset, &roffset, &boffset);

    dot_overspill = symbol->dot_size - 1.0f; /* Allow for exceeding 1X */
    if (dot_overspill < 0.0f) {
        dotoffset = -dot_overspill / 2.0f;
        dot_overspill_scaled = 0;
    } else {
        dotoffset = 0.0f;
        dot_overspill_scaled = dot_overspill * scaler;
    }
    if (dot_overspill_scaled == 0) {
        dot_overspill_scaled = 1;
    }

    scale_width = (symbol->width + xoffset + roffset) * scaler + dot_overspill_scaled;
    scale_height = (symbol->height + yoffset + boffset) * scaler + dot_overspill_scaled;

    /* Apply scale options by creating another pixel buffer */
    if (!(scaled_pixelbuf = (unsigned char *) malloc((size_t) scale_width * scale_height))) {
        strcpy(symbol->errtxt, "657: Insufficient memory for pixel buffer");
        return ZINT_ERROR_ENCODING_PROBLEM;
    }
    memset(scaled_pixelbuf, DEFAULT_PAPER, (size_t) scale_width * scale_height);

    /* Plot the body of the symbol to the pixel buffer */
    dotradius_scaled = (symbol->dot_size * scaler) / 2.0f;
    for (r = 0; r < symbol->rows; r++) {
        float row_scaled = (r + dotoffset + yoffset) * scaler + dotradius_scaled;
        for (i = 0; i < symbol->width; i++) {
            if (module_is_set(symbol, r, i)) {
                draw_circle(scaled_pixelbuf, scale_width, scale_height,
                        (i + dotoffset + xoffset) * scaler + dotradius_scaled,
                        row_scaled,
                        dotradius_scaled,
                        DEFAULT_INK);
            }
        }
    }

    // TODO: bind/box

    error_number = save_raster_image_to_file(symbol, scale_height, scale_width, scaled_pixelbuf, rotate_angle, file_type);
    if (rotate_angle || file_type != OUT_BUFFER || !(symbol->output_options & OUT_BUFFER_INTERMEDIATE)) {
        free(scaled_pixelbuf);
    }

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

static int plot_raster_default(struct zint_symbol *symbol, int rotate_angle, int file_type) {
    int error_number;
    float large_bar_height;
    int textdone = 0;
    int main_width;
    int comp_offset = 0;
    unsigned char addon[6];
    int addon_gap = 0;
    float addon_text_posn = 0.0f;
    int xoffset, yoffset, roffset, boffset;
    int textoffset;
    int default_text_posn;
    float row_height, row_posn;
    int upceanflag = 0;
    int addon_latch = 0;
    unsigned char textpart1[5], textpart2[7], textpart3[7], textpart4[2];
    int textpos;
    int hide_text;
    int i, r;
    int text_height; /* Font pixel size (so whole integers) */
    int text_gap; /* Gap between barcode and text */

    int textflags = 0;
    int guardoffset = 0;
    int image_width, image_height;
    unsigned char *pixelbuf;
    int next_yposn;
    int latch;
    float scaler = symbol->scale;
    int si;
    int half_int_scaling;
    int scale_width, scale_height;
    unsigned char *scaled_pixelbuf;
    int horiz, vert;

    /* Ignore scaling < 0.5 for raster as would drop modules */
    if (scaler < 0.5f) {
        scaler = 0.5f;
    }
    /* If half-integer scaling, then set integer scaler `si` to avoid scaling at end */
    half_int_scaling = scaler * 2.0f == (int) (scaler * 2.0f);
    if (half_int_scaling) {
        si = (int) (scaler * 2.0f);
    } else {
        si = 2;
    }

    large_bar_height = output_large_bar_height(symbol);

    main_width = symbol->width;

    if (is_extendable(symbol->symbology)) {
        upceanflag = output_process_upcean(symbol, &main_width, &comp_offset, addon, &addon_gap);
    }

    output_set_whitespace_offsets(symbol, &xoffset, &yoffset, &roffset, &boffset);

    hide_text = ((!symbol->show_hrt) || (ustrlen(symbol->text) == 0));

    /* Note font sizes halved as in pixels */
    if (upceanflag) {
        textflags = UPCEAN_TEXT | (symbol->output_options & SMALL_TEXT); /* Bold not available for UPC/EAN */
        text_height = (UPCEAN_FONT_HEIGHT + 1) / 2;
        text_gap = 1;
    } else {
        textflags = symbol->output_options & (SMALL_TEXT | BOLD_TEXT);
        text_height = textflags & SMALL_TEXT ? (SMALL_FONT_HEIGHT + 1) / 2 : (NORMAL_FONT_HEIGHT + 1) / 2;
        text_gap = 1;
    }

    if (hide_text) {
        textoffset = upceanflag && upceanflag != 2 && upceanflag != 5 ? 5 : 0; /* Need 5X from bottom for guard bars */
    } else {
        if (upceanflag) {
            textoffset = (text_height > 5 ? text_height : 5) + text_gap; /* Need at least 5X for guard bars */
        } else {
            textoffset = text_height + text_gap;
        }
    }
    if (upceanflag) {
        guardoffset = textoffset - 5 + boffset;
    }

    image_width = (symbol->width + xoffset + roffset) * si;
    image_height = (symbol->height + textoffset + yoffset + boffset) * si;

    if (!(pixelbuf = (unsigned char *) malloc((size_t) image_width * image_height))) {
        strcpy(symbol->errtxt, "658: Insufficient memory for pixel buffer");
        return ZINT_ERROR_ENCODING_PROBLEM;
    }
    memset(pixelbuf, DEFAULT_PAPER, (size_t) image_width * image_height);

    default_text_posn = image_height - (textoffset - text_gap) * si;

    row_height = 0.0f;
    row_posn = textoffset + boffset; /* Bottom up */
    next_yposn = textoffset + boffset;

    /* Plot the body of the symbol to the pixel buffer */
    for (r = 0; r < symbol->rows; r++) {
        int plot_yposn;
        float plot_height;
        int this_row = symbol->rows - r - 1; /* invert r otherwise plots upside down */
        row_posn += row_height;
        plot_yposn = next_yposn;
        row_height = symbol->row_height[this_row] ? symbol->row_height[this_row] : large_bar_height;
        next_yposn = (int) (row_posn + row_height);
        plot_height = next_yposn - plot_yposn;

        plot_yposn *= si;
        plot_height *= si;

        i = 0;

        if (symbol->symbology == BARCODE_ULTRA) {
            do {
                int module_fill = module_colour_is_set(symbol, this_row, i);
                int block_width = 0;
                do {
                    block_width++;
                } while ((i + block_width < symbol->width) && module_colour_is_set(symbol, this_row, i + block_width) == module_fill);

                if (module_fill) {
                    /* a colour block */
                    draw_bar(pixelbuf, (i + xoffset) * si, block_width * si, plot_yposn, plot_height, image_width, image_height, ultra_colour[module_fill]);
                }
                i += block_width;

            } while (i < symbol->width);
        } else {
            do {
                int module_fill = module_is_set(symbol, this_row, i);
                int block_width = 0;
                do {
                    block_width++;
                } while ((i + block_width < symbol->width) && module_is_set(symbol, this_row, i + block_width) == module_fill);

                if (upceanflag && (addon_latch == 0) && (r == 0) && (i > main_width)) {
                    plot_height = row_height - (text_height + text_gap) + 5.0f;
                    plot_yposn = row_posn - 5.0f;
                    if (plot_yposn < 0.0f) {
                        plot_yposn = 0.0f;
                    }
                    if (upceanflag == 12 || upceanflag == 6) { /* UPC-A/E add-ons don't descend */
                        plot_height -= 5.0f;
                        plot_yposn += 5.0f;
                    }
                    if (plot_height < 0.5f) {
                        plot_height = 0.5f;
                    }
                    /* Need to invert composite position */
                    addon_text_posn = is_composite(symbol->symbology) ? image_height - (plot_yposn + plot_height + text_height + text_gap) * si : yoffset * si;
                    plot_yposn *= si;
                    plot_height *= si;
                    addon_latch = 1;
                }
                if (module_fill) {
                    /* a bar */
                    draw_bar(pixelbuf, (i + xoffset) * si, block_width * si, plot_yposn, plot_height, image_width, image_height, DEFAULT_INK);
                }
                i += block_width;

            } while (i < symbol->width);
        }
    }

    xoffset += comp_offset;

    if (upceanflag) {
        /* Guard bar extension */

        if (upceanflag == 6) { /* UPC-E */
            draw_bar(pixelbuf, (0 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (2 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (46 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (48 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (50 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);

        } else if (upceanflag == 8) { /* EAN-8 */
            draw_bar(pixelbuf, (0 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (2 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (32 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (34 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (64 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (66 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);

        } else if (upceanflag == 12) { /* UPC-A */
            latch = 1;

            i = 0 + comp_offset;
            do {
                int module_fill = module_is_set(symbol, symbol->rows - 1, i);
                int block_width = 0;
                do {
                    block_width++;
                } while ((i + block_width < symbol->width) && module_is_set(symbol, symbol->rows - 1, i + block_width) == module_fill);
                if (latch == 1) {
                    /* a bar */
                    draw_bar(pixelbuf, (i + xoffset - comp_offset) * si, block_width * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
                    latch = 0;
                } else {
                    /* a space */
                    latch = 1;
                }
                i += block_width;
            } while (i < 11 + comp_offset);
            draw_bar(pixelbuf, (46 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (48 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            latch = 1;
            i = 85 + comp_offset;
            do {
                int module_fill = module_is_set(symbol, symbol->rows - 1, i);
                int block_width = 0;
                do {
                    block_width++;
                } while ((i + block_width < symbol->width) && module_is_set(symbol, symbol->rows - 1, i + block_width) == module_fill);
                if (latch == 1) {
                    /* a bar */
                    draw_bar(pixelbuf, (i + xoffset - comp_offset) * si, block_width * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
                    latch = 0;
                } else {
                    /* a space */
                    latch = 1;
                }
                i += block_width;
            } while (i < 96 + comp_offset);

        } else if (upceanflag == 13) { /* EAN-13 */
            draw_bar(pixelbuf, (0 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (2 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (46 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (48 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (92 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (94 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height, DEFAULT_INK);
        }
    }

    if (!hide_text) {

        if (upceanflag) {
            /* Note font sizes halved as in pixels */
            int upcea_width_adj = (UPCEAN_SMALL_FONT_WIDTH + 3) / 4; /* Halved again to get middle position that draw_string() expects */
            int upcea_height_adj = (UPCEAN_FONT_HEIGHT - UPCEAN_SMALL_FONT_HEIGHT) * si / 2;
            int ean_width_adj = (UPCEAN_FONT_WIDTH + 3) / 4; /* Halved again to get middle position that draw_string() expects */

            output_upcean_split_text(upceanflag, symbol->text, textpart1, textpart2, textpart3, textpart4);

            if (upceanflag == 6) { /* UPC-E */
                textpos = (-(5 + upcea_width_adj) + xoffset) * si;
                draw_string(pixelbuf, textpart1, textpos, default_text_posn + upcea_height_adj, textflags | SMALL_TEXT, image_width, image_height, si);
                textpos = (24 + xoffset) * si;
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height, si);
                textpos = (51 + 3 + upcea_width_adj + xoffset) * si;
                draw_string(pixelbuf, textpart3, textpos, default_text_posn + upcea_height_adj, textflags | SMALL_TEXT, image_width, image_height, si);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = (61 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height, si);
                        break;
                    case 5:
                        textpos = (75 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height, si);
                        break;
                }

            } else if (upceanflag == 8) { /* EAN-8 */
                textpos = (17 + xoffset) * si;
                draw_string(pixelbuf, textpart1, textpos, default_text_posn, textflags, image_width, image_height, si);
                textpos = (50 + xoffset) * si;
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height, si);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = (77 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height, si);
                        break;
                    case 5:
                        textpos = (91 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height, si);
                        break;
                }

            } else if (upceanflag == 12) { /* UPC-A */
                textpos = (-(5 + upcea_width_adj) + xoffset) * si;
                draw_string(pixelbuf, textpart1, textpos, default_text_posn + upcea_height_adj, textflags | SMALL_TEXT, image_width, image_height, si);
                textpos = (27 + xoffset) * si;
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height, si);
                textpos = (67 + xoffset) * si;
                draw_string(pixelbuf, textpart3, textpos, default_text_posn, textflags, image_width, image_height, si);
                textpos = (95 + 5 + upcea_width_adj + xoffset) * si;
                draw_string(pixelbuf, textpart4, textpos, default_text_posn + upcea_height_adj, textflags | SMALL_TEXT, image_width, image_height, si);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = (105 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height, si);
                        break;
                    case 5:
                        textpos = (119 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height, si);
                        break;
                }

            } else if (upceanflag == 13) { /* EAN-13 */
                textpos = (-(5 + ean_width_adj) + xoffset) * si;
                draw_string(pixelbuf, textpart1, textpos, default_text_posn, textflags, image_width, image_height, si);
                textpos = (24 + xoffset) * si;
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height, si);
                textpos = (71 + xoffset) * si;
                draw_string(pixelbuf, textpart3, textpos, default_text_posn, textflags, image_width, image_height, si);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = (105 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height, si);
                        break;
                    case 5:
                        textpos = (119 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height, si);
                        break;
                }
            }
        }

        if (!textdone) {
            unsigned char local_text[sizeof(symbol->text)] = {0}; /* Suppress clang-analyzer-core.CallAndMessage warning */
            to_iso8859_1(symbol->text, local_text);
            /* Put the human readable text at the bottom */
            textpos = (main_width / 2 + xoffset) * si;
            draw_string(pixelbuf, local_text, textpos, default_text_posn, textflags, image_width, image_height, si);
        }
    }

    xoffset -= comp_offset;

    // Binding and boxes
    if ((symbol->output_options & BARCODE_BIND) != 0) {
        if ((symbol->rows > 1) && (is_stackable(symbol->symbology) == 1)) {
            float sep_height = 1.0f;
            if (symbol->option_3 > 0 && symbol->option_3 <= 4) {
                sep_height = symbol->option_3;
            }
            /* row binding */
            if (symbol->symbology != BARCODE_CODABLOCKF && symbol->symbology != BARCODE_HIBC_BLOCKF) {
                for (r = 1; r < symbol->rows; r++) {
                    row_height = symbol->row_height[r - 1] ? symbol->row_height[r - 1] : large_bar_height;
                    draw_bar(pixelbuf, xoffset * si, symbol->width * si,
                            ((r * row_height) + textoffset + yoffset - sep_height / 2) * si, sep_height * si, image_width, image_height, DEFAULT_INK);
                }
            } else {
                for (r = 1; r < symbol->rows; r++) {
                    /* Avoid 11-module start and 13-module stop chars */
                    row_height = symbol->row_height[r - 1] ? symbol->row_height[r - 1] : large_bar_height;
                    draw_bar(pixelbuf, (xoffset + 11) * si, (symbol->width - 24) * si,
                            ((r * row_height) + textoffset + yoffset - sep_height / 2) * si, sep_height * si, image_width, image_height, DEFAULT_INK);
                }
            }
        }
    }
    if (symbol->border_width > 0) {
        if ((symbol->output_options & BARCODE_BOX) || (symbol->output_options & BARCODE_BIND)) {
            /* boundary bars */
            if ((symbol->output_options & BARCODE_BOX) || (symbol->symbology != BARCODE_CODABLOCKF && symbol->symbology != BARCODE_HIBC_BLOCKF)) {
                draw_bar(pixelbuf, 0, (symbol->width + xoffset + roffset) * si,
                        textoffset * si, symbol->border_width * si, image_width, image_height, DEFAULT_INK);
                draw_bar(pixelbuf, 0, (symbol->width + xoffset + roffset) * si,
                        (textoffset + symbol->height + symbol->border_width) * si, symbol->border_width * si, image_width, image_height, DEFAULT_INK);
            } else {
                draw_bar(pixelbuf, xoffset * si, symbol->width * si,
                        textoffset * si, symbol->border_width * si, image_width, image_height, DEFAULT_INK);
                draw_bar(pixelbuf, xoffset * si, symbol->width * si,
                        (textoffset + symbol->height + symbol->border_width) * si, symbol->border_width * si, image_width, image_height, DEFAULT_INK);
            }
        }
        if ((symbol->output_options & BARCODE_BOX)) {
            /* side bars */
            draw_bar(pixelbuf, 0, symbol->border_width * si,
                    textoffset * si, (symbol->height + (2 * symbol->border_width)) * si, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, (symbol->width + xoffset + roffset - symbol->border_width) * si, symbol->border_width * si,
                    textoffset * si, (symbol->height + (2 * symbol->border_width)) * si, image_width, image_height, DEFAULT_INK);
        }
    }

    if (!half_int_scaling) {
        scale_width = image_width * scaler;
        scale_height = image_height * scaler;

        /* Apply scale options by creating another pixel buffer */
        if (!(scaled_pixelbuf = (unsigned char *) malloc((size_t) scale_width * scale_height))) {
            free(pixelbuf);
            strcpy(symbol->errtxt, "659: Insufficient memory for pixel buffer");
            return ZINT_ERROR_ENCODING_PROBLEM;
        }
        memset(scaled_pixelbuf, DEFAULT_PAPER, (size_t) scale_width * scale_height);

        for (vert = 0; vert < scale_height; vert++) {
            int vert_row = vert * scale_width;
            int image_vert_row = ((int) (vert / scaler)) * image_width;
            for (horiz = 0; horiz < scale_width; horiz++) {
                *(scaled_pixelbuf + vert_row + horiz) = *(pixelbuf + image_vert_row + (int) (horiz / scaler));
            }
        }

        error_number = save_raster_image_to_file(symbol, scale_height, scale_width, scaled_pixelbuf, rotate_angle, file_type);
        if (rotate_angle || file_type != OUT_BUFFER || !(symbol->output_options & OUT_BUFFER_INTERMEDIATE)) {
            free(scaled_pixelbuf);
        }
        free(pixelbuf);
    } else {
        error_number = save_raster_image_to_file(symbol, image_height, image_width, pixelbuf, rotate_angle, file_type);
        if (rotate_angle || file_type != OUT_BUFFER || !(symbol->output_options & OUT_BUFFER_INTERMEDIATE)) {
            free(pixelbuf);
        }
    }
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

    if (symbol->symbology == BARCODE_MAXICODE) {
        error = plot_raster_maxicode(symbol, rotate_angle, file_type);
    } else if (symbol->output_options & BARCODE_DOTTY_MODE) {
        error = plot_raster_dotty(symbol, rotate_angle, file_type);
    } else {
        error = plot_raster_default(symbol, rotate_angle, file_type);
    }

    return error;
}
