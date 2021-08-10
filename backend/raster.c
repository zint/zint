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

#include <assert.h>
#include <math.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <malloc.h>
#include <fcntl.h>
#include <io.h>
#endif /* _MSC_VER */

#include "common.h"
#include "output.h"
#include "zfiletypes.h"

#include "font.h" /* Font for human readable text */

#define DEFAULT_INK     '1'
#define DEFAULT_PAPER   '0'

#define UPCEAN_TEXT     1

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

static int save_raster_image_to_file(struct zint_symbol *symbol, const int image_height, const int image_width,
            unsigned char *pixelbuf, int rotate_angle, const int file_type) {
    int error_number;
    int row, column;

    unsigned char *rotated_pixbuf = pixelbuf;

    /* Suppress clang-analyzer-core.UndefinedBinaryOperatorResult warning */
    assert(rotate_angle == 0 || rotate_angle == 90 || rotate_angle == 180 || rotate_angle == 270);
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
            return ZINT_ERROR_MEMORY;
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

/* Helper to check point within bounds before setting */
static void draw_pt(unsigned char *buf, const int buf_width, const int buf_height,
            const int x, const int y, const int fill) {
    if (x >= 0 && x < buf_width && y >= 0 && y < buf_height) {
        buf[y * buf_width + x] = fill;
    }
}

/* Draw a rectangle */
static void draw_bar(unsigned char *pixelbuf, const int xpos, const int xlen, const int ypos, const int ylen,
            const int image_width, const int image_height, const char fill) {
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

/* Put a letter into a position */
static void draw_letter(unsigned char *pixelbuf, const unsigned char letter, int xposn, const int yposn,
            const int textflags, const int image_width, const int image_height, const int si) {
    int skip;

    skip = 0;

    if (letter < 33) {
        skip = 1;
    }

    if ((letter >= 127) && (letter < 161)) {
        skip = 1;
    }

    /* Following should never happen (ISBN check digit "X" not printed) */
    if ((textflags & UPCEAN_TEXT) && (letter < '0' || letter > '9')) {
        skip = 1; /* Not reached */
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
static void draw_string(unsigned char *pixbuf, const unsigned char input_string[], const int xposn, const int yposn,
            const int textflags, const int image_width, const int image_height, const int si) {
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

    string_length = (int) ustrlen(input_string);

    string_left_hand = xposn - ((letter_width * string_length - letter_gap) * half_si) / 2;
    if (odd_si) {
        string_left_hand -= (letter_width * string_length - letter_gap) / 4;
    }
    for (i = 0; i < string_length; i++) {
        x_incr = i * letter_width * half_si;
        if (odd_si) {
            x_incr += i * letter_width / 2;
        }
        draw_letter(pixbuf, input_string[i], string_left_hand + x_incr, yposn, textflags, image_width, image_height,
                    si);
    }
}

/* Draw disc using x² + y² <= r² */
static void draw_circle(unsigned char *pixelbuf, const int image_width, const int image_height,
            const int x0, const int y0, const float radius, const char fill) {
    int x, y;
    const int radius_i = (int) floorf(radius);
    const int radius_squared = radius_i * radius_i;

    for (y = -radius_i; y <= radius_i; y++) {
        const int y_squared = y * y;
        for (x = -radius_i; x <= radius_i; x++) {
            if ((x * x) + y_squared <= radius_squared) {
                draw_pt(pixelbuf, image_width, image_height, x0 + x, y0 + y, fill);
            }
        }
    }
}

/* Helper for `draw_wp_circle()` to draw horizontal filler lines within disc */
static void draw_mp_circle_lines(unsigned char *pixelbuf, const int image_width, const int image_height,
            const int x0, const int y0, const int x, const int y, const int fill) {
    int i;
    for (i = x0 - x; i <= x0 + x; i++) {
        draw_pt(pixelbuf, image_width, image_height, i, y0 + y, fill); /* (-x, y) to (x, y) */
        draw_pt(pixelbuf, image_width, image_height, i, y0 - y, fill); /* (-x, -y) to (x, -y) */
    }
    for (i = x0 - y; i <= x0 + y; i++) {
        draw_pt(pixelbuf, image_width, image_height, i, y0 + x, fill); /* (-y, x) to (y, x) */
        draw_pt(pixelbuf, image_width, image_height, i, y0 - x, fill); /* (-y, -x) to (y, -x) */
    }
}

/* Draw disc using Midpoint Circle Algorithm. Using this for MaxiCode rather than `draw_circle()` because it gives a
 * flatter circumference with no single pixel peaks, similar to Figures J3 and J6 in ISO/IEC 16023:2000.
 * Taken from https://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm#C
 * "Content is available under GNU Free Documentation License 1.2 unless otherwise noted."
 * https://www.gnu.org/licenses/old-licenses/fdl-1.2.html */
static void draw_mp_circle(unsigned char *pixelbuf, const int image_width, const int image_height,
            const int x0, const int y0, const int r, const int fill) {
    /* Using top RHS octant from (0, r) going clockwise, so fast direction is x (i.e. always incremented) */
    int f = 1 - r;
    int ddF_x = 0;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    draw_mp_circle_lines(pixelbuf, image_width, image_height, x0, y0, x, y, fill);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x + 1;
        draw_mp_circle_lines(pixelbuf, image_width, image_height, x0, y0, x, y, fill);
    }
}

/* Draw central bullseye finder in Maxicode symbols */
static void draw_bullseye(unsigned char *pixelbuf, const int image_width, const int image_height,
            const int hex_width, const int hex_height, const int hx_start, const int hx_end,
            const int hex_image_height, const int xoffset, const int yoffset, const float scaler) {

    /* ISO/IEC 16023:2000 4.11.4 and 4.2.1.1 */

    /* 14W right from leftmost centre = 14.5X */
    const int x = (int) floorf(14.5f * hex_width - hx_start + xoffset * scaler);
    /* 16Y above bottom-most centre = halfway */
    const int y = (int) ceilf(hex_image_height / 2 + yoffset * scaler);

    const int r1 = (int) ceilf(hex_height / 2.0f); /* Inner diameter is hex_height (V) */
    /* Total finder diameter is 9X, so radial increment for 5 radii r2 to r6 is ((9X - r1) / 5) / 2 */
    int r_incr = ((hex_width * 9 - r1) / 5) / 2;
    /* Fudge increment to lessen overlapping of finder with top/bottom of hexagons due to rounding errors */
    if (r_incr > hx_end) {
        r_incr -= hx_end;
    }

    draw_mp_circle(pixelbuf, image_width, image_height, x, y, r1 + r_incr * 5, DEFAULT_INK);
    draw_mp_circle(pixelbuf, image_width, image_height, x, y, r1 + r_incr * 4, DEFAULT_PAPER);
    draw_mp_circle(pixelbuf, image_width, image_height, x, y, r1 + r_incr * 3, DEFAULT_INK);
    draw_mp_circle(pixelbuf, image_width, image_height, x, y, r1 + r_incr * 2, DEFAULT_PAPER);
    draw_mp_circle(pixelbuf, image_width, image_height, x, y, r1 + r_incr, DEFAULT_INK);
    draw_mp_circle(pixelbuf, image_width, image_height, x, y, r1, DEFAULT_PAPER);
}

/* Put a hexagon into the pixel buffer */
static void draw_hexagon(unsigned char *pixelbuf, const int image_width, const int image_height,
            unsigned char *scaled_hexagon, const int hex_width, const int hex_height, const int xposn,
            const int yposn) {
    int i, j;

    for (i = 0; i < hex_height; i++) {
        for (j = 0; j < hex_width; j++) {
            if (scaled_hexagon[(i * hex_width) + j] == DEFAULT_INK) {
                draw_pt(pixelbuf, image_width, image_height, xposn + j, yposn + i, DEFAULT_INK);
            }
        }
    }
}

/* Bresenham's line algorithm https://en.wikipedia.org/wiki/Bresenham's_line_algorithm
 * Creative Commons Attribution-ShareAlike License
 * https://en.wikipedia.org/wiki/Wikipedia:Text_of_Creative_Commons_Attribution-ShareAlike_3.0_Unported_License */
static void plot_hexline(unsigned char *scaled_hexagon, const int hex_width, const int hex_height,
            int start_x, int start_y, const int end_x, const int end_y) {
    const int dx = abs(end_x - start_x);
    const int sx = start_x < end_x ? 1 : -1;
    const int dy = -abs(end_y - start_y);
    const int sy = start_y < end_y ? 1 : -1;
    int e_xy = dx + dy;

    for (;;) {
        int e2;
        draw_pt(scaled_hexagon, hex_width, hex_height, start_x, start_y, DEFAULT_INK);
        if (start_x == end_x && start_y == end_y) {
            break;
        }
        e2 = 2 * e_xy;
        if (e2 >= dy) { /* e_xy+e_x > 0 */
            e_xy += dy;
            start_x += sx;
        }
        if (e2 <= dx) { /* e_xy+e_y < 0 */
            e_xy += dx;
            start_y += sy;
        }
    }
}

/* Create a hexagon shape and fill it */
static void plot_hexagon(unsigned char *scaled_hexagon, const int hex_width, const int hex_height,
            const int hx_start, const int hy_start, const int hx_end, const int hy_end) {
    int line, i;
    int not_top;

    const int hx_width = hex_width - hx_start - hx_end;
    const int hy_height = hex_height - hx_start - hx_end;

    const int hx_width_odd = hx_width & 1;
    const int hy_height_odd = hy_height & 1;

    const int hx_radius = hx_width / 2;
    const int hy_radius = hy_height / 2;

    /* To ensure symmetry, draw top left quadrant first, then copy/flip to other quadrants */

    int start_y = hy_start + (hy_radius + 1) / 2;
    int end_x = hx_start + hx_radius;
    if (hx_radius > 2 && (hx_radius < 10 || !hx_width_odd)) {
        /* Line drawing matches examples in ISO/IEC 16023:2000 Annexe J if point just to the left of end midpoint */
        end_x--;
    }

    /* Plot line of top left quadrant */
    plot_hexline(scaled_hexagon, hex_width, hex_height, hx_start, start_y, end_x, hy_start);

    /* Fill to right */
    not_top = 0;
    for (line = hy_start; line < hy_start + hy_radius + hy_height_odd; line++) {
        int first = -1;
        for (i = hx_start; i < hx_start + hx_radius + hx_width_odd; i++) {
            if (first != -1) {
                scaled_hexagon[(hex_width * line) + i] = DEFAULT_INK;
                not_top = 1;
            } else if (scaled_hexagon[(hex_width * line) + i] == DEFAULT_INK) {
                first = i + 1;
            }
        }
        if (not_top && first == -1) { /* Fill empty lines at bottom */
            for (i = hx_start; i < hx_start + hx_radius + hx_width_odd; i++) {
                scaled_hexagon[(hex_width * line) + i] = DEFAULT_INK;
            }
        }
    }

    /* Copy left quadrant to right, flipping horizontally */
    for (line = hy_start; line < hy_start + hy_radius + hy_height_odd; line++) {
        for (i = hx_start; i < hx_start + hx_radius + hx_width_odd; i++) {
            if (scaled_hexagon[(hex_width * line) + i] == DEFAULT_INK) {
                scaled_hexagon[(hex_width * line) + hex_width - hx_end - (i - hx_start + 1)] = DEFAULT_INK;
            }
        }
    }

    /* Copy top to bottom, flipping vertically */
    for (line = hy_start; line < hy_start + hy_radius + hy_height_odd; line++) {
        for (i = hx_start; i < hex_width; i++) {
            if (scaled_hexagon[(hex_width * line) + i] == DEFAULT_INK) {
                scaled_hexagon[(hex_width * (hex_height - hy_end - (line - hy_start + 1))) + i] = DEFAULT_INK;
            }
        }
    }
}

/* Plot a MaxiCode symbol with hexagons and bullseye */
static int plot_raster_maxicode(struct zint_symbol *symbol, const int rotate_angle, const int file_type) {
    int row, column;
    int image_height, image_width;
    unsigned char *pixelbuf;
    int error_number;
    float xoffset, yoffset, roffset, boffset;
    float scaler = symbol->scale;
    int xoffset_scaled, yoffset_scaled;
    unsigned char *scaled_hexagon;
    int hex_width, hex_height;
    int hx_start, hy_start, hx_end, hy_end;
    int hex_image_width, hex_image_height;
    int yposn_offset;

    const float two_div_sqrt3 = 1.1547f; /* 2 / √3 */
    const float sqrt3_div_two = 0.866f; /* √3 / 2 == 1.5 / √3 */

    if (scaler < 0.2f) {
        scaler = 0.2f;
    }
    scaler *= 10.0f;

    output_set_whitespace_offsets(symbol, &xoffset, &yoffset, &roffset, &boffset);
    xoffset_scaled = (int) floorf(xoffset * scaler);
    yoffset_scaled = (int) floorf(yoffset * scaler);

    hex_width = (int) roundf(scaler); /* Short diameter, X in ISO/IEC 16023:2000 Figure 8 (same as W) */
    hex_height = (int) roundf(scaler * two_div_sqrt3); /* Long diameter, V in Figure 8 */

    /* Allow for whitespace around each hexagon (see ISO/IEC 16023:2000 Annexe J.4)
       TODO: replace following kludge with proper calc of whitespace as per J.4 Steps 8 to 11 */
    hx_start = (int) (scaler < 3.5f ? 0.0f : ceilf(hex_width * 0.05f));
    hy_start = (int) ceilf(hex_height * 0.05f);
    hx_end = (int) roundf((hex_width - hx_start) * 0.05f);
    hy_end = (int) roundf((hex_height - hy_start) * 0.05f);

    /* The hexagons will be drawn within box (hex_width - hx_start - hx_end) x (hex_height - hy_start - hy_end)
       and plotted starting at (-hx_start, -hy_start) */

    hex_image_width = 30 * hex_width - hx_start - hx_end;
    /* `yposn_offset` is vertical distance between rows, Y in Figure 8 */
    /* TODO: replace following kludge with proper calc of hex_width/hex_height/yposn_offset as per J.4 Steps 1 to 7 */
    yposn_offset = (int) (scaler > 10.0f ? floorf(sqrt3_div_two * hex_width) : roundf(sqrt3_div_two * hex_width));
    /* 32 rows drawn yposn_offset apart + final hexagon */
    hex_image_height = 32 * yposn_offset + hex_height - hy_start - hy_end;

    image_width = (int) ceilf(hex_image_width + (xoffset + roffset) * scaler);
    image_height = (int) ceilf(hex_image_height + (yoffset + boffset) * scaler);

    if (!(pixelbuf = (unsigned char *) malloc((size_t) image_width * image_height))) {
        strcpy(symbol->errtxt, "655: Insufficient memory for pixel buffer");
        return ZINT_ERROR_MEMORY;
    }
    memset(pixelbuf, DEFAULT_PAPER, (size_t) image_width * image_height);

    if (!(scaled_hexagon = (unsigned char *) malloc((size_t) hex_width * hex_height))) {
        strcpy(symbol->errtxt, "656: Insufficient memory for pixel buffer");
        free(pixelbuf);
        return ZINT_ERROR_MEMORY;
    }
    memset(scaled_hexagon, DEFAULT_PAPER, (size_t) hex_width * hex_height);

    plot_hexagon(scaled_hexagon, hex_width, hex_height, hx_start, hy_start, hx_end, hy_end);

    for (row = 0; row < symbol->rows; row++) {
        const int odd_row = row & 1; /* Odd (reduced) row, even (full) row */
        const int yposn = row * yposn_offset + yoffset_scaled - hy_start;
        const int xposn_offset = (odd_row ? hex_width / 2 : 0) + xoffset_scaled - hx_start;
        for (column = 0; column < symbol->width - odd_row; column++) {
            const int xposn = column * hex_width + xposn_offset;
            if (module_is_set(symbol, row, column)) {
                draw_hexagon(pixelbuf, image_width, image_height, scaled_hexagon, hex_width, hex_height, xposn,
                            yposn);
            }
        }
    }

    draw_bullseye(pixelbuf, image_width, image_height, hex_width, hex_height, hx_start, hx_end, hex_image_height,
        xoffset, yoffset, scaler);

    if (symbol->border_width > 0 && (symbol->output_options & (BARCODE_BOX | BARCODE_BIND))) {
        /* boundary bars */
        const int border_scaled = (int) floorf(symbol->border_width * scaler);
        const int vwhitesp_scaled = (int) floorf(symbol->whitespace_height * scaler);
        const int ybind_top = hex_image_height + border_scaled + vwhitesp_scaled;
        draw_bar(pixelbuf, 0, image_width, vwhitesp_scaled, border_scaled, image_width, image_height, DEFAULT_INK);
        draw_bar(pixelbuf, 0, image_width, ybind_top, border_scaled, image_width, image_height, DEFAULT_INK);

        if (symbol->output_options & BARCODE_BOX) {
            /* side bars */
            const int whitesp_scaled = (int) floorf(symbol->whitespace_width * scaler);
            const int xbox_right = hex_image_width + border_scaled + whitesp_scaled * 2;
            const int ybox_top = border_scaled + vwhitesp_scaled;
            const int ylen = image_height - (border_scaled + vwhitesp_scaled) * 2;
            draw_bar(pixelbuf, 0, border_scaled, ybox_top, ylen, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, xbox_right, border_scaled, ybox_top, ylen, image_width, image_height, DEFAULT_INK);
        }
    }

    error_number = save_raster_image_to_file(symbol, image_height, image_width, pixelbuf, rotate_angle, file_type);
    free(scaled_hexagon);
    if (rotate_angle || file_type != OUT_BUFFER || !(symbol->output_options & OUT_BUFFER_INTERMEDIATE)) {
        free(pixelbuf);
    }
    if (error_number == 0) {
        /* Check whether size is compliant */
        const float size_ratio = (float) hex_image_width / hex_image_height;
        if (size_ratio < 24.82f / 26.69f || size_ratio > 27.93f / 23.71f) {
            strcpy(symbol->errtxt, "663: Size not within the minimum/maximum ranges");
            error_number = ZINT_WARN_NONCOMPLIANT;
        }
    }
    return error_number;
}

/* Draw binding or box */
static void draw_bind_box(struct zint_symbol *symbol, unsigned char *pixelbuf, const int xoffset, const int roffset,
            const int textoffset, const int overspill_scaled, const int image_width, const int image_height,
            const int si) {
    if (symbol->border_width > 0 && (symbol->output_options & (BARCODE_BOX | BARCODE_BIND))) {
        const int bwidth = symbol->border_width * si;
        const int ybind_bottom = (textoffset + symbol->whitespace_height) * si;
        const int ybind_top = (textoffset + symbol->whitespace_height + symbol->height) * si + overspill_scaled
                                + bwidth;
        /* Horizontal boundary bars */
        if ((symbol->output_options & BARCODE_BOX)
                || (symbol->symbology != BARCODE_CODABLOCKF && symbol->symbology != BARCODE_HIBC_BLOCKF)) {
            /* Box or not CodaBlockF */
            const int xlen = (symbol->width + xoffset + roffset) * si + overspill_scaled;
            draw_bar(pixelbuf, 0, xlen, ybind_bottom, bwidth, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, 0, xlen, ybind_top, bwidth, image_width, image_height, DEFAULT_INK);
        } else {
            /* CodaBlockF bind - does not extend over horizontal whitespace */
            const int xlen = symbol->width * si;
            draw_bar(pixelbuf, xoffset * si, xlen, ybind_bottom, bwidth, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, xoffset * si, xlen, ybind_top, bwidth, image_width, image_height, DEFAULT_INK);
        }
        if (symbol->output_options & BARCODE_BOX) {
            /* Vertical side bars */
            const int ylen = (symbol->height + symbol->border_width * 2) * si;
            const int ybox = (textoffset + symbol->whitespace_height) * si;
            const int xbox_right = (symbol->width + xoffset + roffset - symbol->border_width) * si + overspill_scaled;
            draw_bar(pixelbuf, 0, bwidth, ybox, ylen, image_width, image_height, DEFAULT_INK);
            draw_bar(pixelbuf, xbox_right, bwidth, ybox, ylen, image_width, image_height, DEFAULT_INK);
        }
    }
}

static int plot_raster_dotty(struct zint_symbol *symbol, const int rotate_angle, const int file_type) {
    float scaler = 2 * symbol->scale;
    unsigned char *scaled_pixelbuf;
    int r, i;
    int scale_width, scale_height;
    int error_number = 0;
    float xoffset, yoffset, roffset, boffset;
    float dot_offset_scaled;
    float dot_radius_scaled;
    int dot_radius_scaled_i;
    int dot_overspill_scaled_i;

    if (scaler < 2.0f) {
        scaler = 2.0f;
    }
    dot_radius_scaled = (symbol->dot_size * scaler) / 2.0f;
    dot_radius_scaled_i = (int) floorf(dot_radius_scaled);

    output_set_whitespace_offsets(symbol, &xoffset, &yoffset, &roffset, &boffset);

    /* TODO: Revisit this overspill stuff, it's hacky */
    if (symbol->dot_size < 1.0f) {
        dot_overspill_scaled_i = 0;
        /* Offset (1 - dot_size) / 2 + dot_radius == (1 - dot_size + dot_size) / 2 == 1 / 2 */
        dot_offset_scaled = scaler / 2.0f;
    } else { /* Allow for exceeding 1X */
        dot_overspill_scaled_i = (int) ceilf((symbol->dot_size - 1.0f) * scaler);
        dot_offset_scaled = dot_radius_scaled;
    }
    if (dot_overspill_scaled_i == 0) {
        dot_overspill_scaled_i = 1;
    }

    scale_width = (int) floorf((symbol->width + xoffset + roffset) * scaler + dot_overspill_scaled_i);
    scale_height = (int) floorf((symbol->height + yoffset + boffset) * scaler + dot_overspill_scaled_i);

    /* Apply scale options by creating another pixel buffer */
    if (!(scaled_pixelbuf = (unsigned char *) malloc((size_t) scale_width * scale_height))) {
        strcpy(symbol->errtxt, "657: Insufficient memory for pixel buffer");
        return ZINT_ERROR_MEMORY;
    }
    memset(scaled_pixelbuf, DEFAULT_PAPER, (size_t) scale_width * scale_height);

    /* Plot the body of the symbol to the pixel buffer */
    for (r = 0; r < symbol->rows; r++) {
        int row_scaled = (int) floorf((r + yoffset) * scaler + dot_offset_scaled);
        for (i = 0; i < symbol->width; i++) {
            if (module_is_set(symbol, r, i)) {
                draw_circle(scaled_pixelbuf, scale_width, scale_height,
                            (int) floorf((i + xoffset) * scaler + dot_offset_scaled),
                            row_scaled, dot_radius_scaled_i, DEFAULT_INK);
            }
        }
    }

    draw_bind_box(symbol, scaled_pixelbuf, xoffset, roffset, 0 /*textoffset*/, dot_overspill_scaled_i,
                    scale_width, scale_height, (int) floorf(scaler));

    error_number = save_raster_image_to_file(symbol, scale_height, scale_width, scaled_pixelbuf, rotate_angle,
                                            file_type);
    if (rotate_angle || file_type != OUT_BUFFER || !(symbol->output_options & OUT_BUFFER_INTERMEDIATE)) {
        free(scaled_pixelbuf);
    }

    return error_number;
}

/* Convert UTF-8 to ISO 8859-1 for draw_string() human readable text */
static void to_iso8859_1(const unsigned char source[], unsigned char preprocessed[]) {
    int j, i, input_length;

    input_length = (int) ustrlen(source);

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

static int plot_raster_default(struct zint_symbol *symbol, const int rotate_angle, const int file_type) {
    int error_number;
    float large_bar_height;
    int textdone = 0;
    int main_width;
    int comp_offset = 0;
    unsigned char addon[6];
    int addon_gap = 0;
    float addon_text_posn = 0.0f;
    float xoffset, yoffset, roffset, boffset;
    float textoffset;
    int default_text_posn;
    float row_height, row_posn;
    int upceanflag = 0;
    int addon_latch = 0;
    unsigned char textpart1[5], textpart2[7], textpart3[7], textpart4[2];
    int textpos;
    int hide_text;
    int i, r;
    int text_height; /* Font pixel size (so whole integers) */
    float text_gap; /* Gap between barcode and text */

    int textflags = 0;
    int guardoffset = 0;
    int image_width, image_height;
    unsigned char *pixelbuf;
    float next_yposn;
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
    half_int_scaling = isfintf(scaler * 2.0f);
    if (half_int_scaling) {
        si = (int) (scaler * 2.0f);
    } else {
        si = 2;
    }

    large_bar_height = output_large_bar_height(symbol, si /*Round to scale*/);

    main_width = symbol->width;

    if (is_extendable(symbol->symbology)) {
        upceanflag = output_process_upcean(symbol, &main_width, &comp_offset, addon, &addon_gap);
    }

    output_set_whitespace_offsets(symbol, &xoffset, &yoffset, &roffset, &boffset);

    /* Note font sizes halved as in pixels */
    if (upceanflag) {
        textflags = UPCEAN_TEXT | (symbol->output_options & SMALL_TEXT); /* Bold not available for UPC/EAN */
        text_height = (UPCEAN_FONT_HEIGHT + 1) / 2;
        text_gap = 1.0f;
    } else {
        textflags = symbol->output_options & (SMALL_TEXT | BOLD_TEXT);
        text_height = textflags & SMALL_TEXT ? (SMALL_FONT_HEIGHT + 1) / 2 : (NORMAL_FONT_HEIGHT + 1) / 2;
        text_gap = 1.0f;
    }

    hide_text = ((!symbol->show_hrt) || (ustrlen(symbol->text) == 0));

    if (hide_text) {
        /* Need 5X from bottom for guard bars */
        textoffset = upceanflag && upceanflag != 2 && upceanflag != 5 ? 5 : 0;
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
        return ZINT_ERROR_MEMORY;
    }
    memset(pixelbuf, DEFAULT_PAPER, (size_t) image_width * image_height);

    default_text_posn = image_height - (textoffset - text_gap + symbol->whitespace_height) * si;

    row_height = 0.0f;
    row_posn = textoffset + boffset; /* Bottom up */
    next_yposn = textoffset + boffset;

    /* Plot the body of the symbol to the pixel buffer */
    for (r = 0; r < symbol->rows; r++) {
        float plot_yposn;
        float plot_height;
        int this_row = symbol->rows - r - 1; /* invert r otherwise plots upside down */
        row_posn += row_height;
        plot_yposn = next_yposn;
        row_height = symbol->row_height[this_row] ? symbol->row_height[this_row] : large_bar_height;
        next_yposn = row_posn + row_height;
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
                } while ((i + block_width < symbol->width)
                        && module_colour_is_set(symbol, this_row, i + block_width) == module_fill);

                if (module_fill) {
                    /* a colour block */
                    draw_bar(pixelbuf, (i + xoffset) * si, block_width * si, plot_yposn, plot_height, image_width,
                            image_height, ultra_colour[module_fill]);
                }
                i += block_width;

            } while (i < symbol->width);
        } else {
            do {
                int module_fill = module_is_set(symbol, this_row, i);
                int block_width = 0;
                do {
                    block_width++;
                } while ((i + block_width < symbol->width)
                        && module_is_set(symbol, this_row, i + block_width) == module_fill);

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
                    addon_text_posn = is_composite(symbol->symbology)
                                        ? image_height - (plot_yposn + plot_height + text_height + text_gap) * si
                                        : yoffset * si;
                    plot_yposn *= si;
                    plot_height *= si;
                    addon_latch = 1;
                }
                if (module_fill) {
                    /* a bar */
                    draw_bar(pixelbuf, (i + xoffset) * si, block_width * si, plot_yposn, plot_height, image_width,
                            image_height, DEFAULT_INK);
                }
                i += block_width;

            } while (i < symbol->width);
        }
    }

    xoffset += comp_offset;

    if (upceanflag) {
        /* Guard bar extension */

        if (upceanflag == 6) { /* UPC-E */
            draw_bar(pixelbuf, (0 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (2 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (46 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (48 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (50 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);

        } else if (upceanflag == 8) { /* EAN-8 */
            draw_bar(pixelbuf, (0 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (2 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (32 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (34 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (64 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (66 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);

        } else if (upceanflag == 12) { /* UPC-A */
            latch = 1;

            i = 0 + comp_offset;
            do {
                int module_fill = module_is_set(symbol, symbol->rows - 1, i);
                int block_width = 0;
                do {
                    block_width++;
                } while ((i + block_width < symbol->width)
                        && module_is_set(symbol, symbol->rows - 1, i + block_width) == module_fill);
                if (latch == 1) {
                    /* a bar */
                    draw_bar(pixelbuf, (i + xoffset - comp_offset) * si, block_width * si, guardoffset * si, 5 * si,
                            image_width, image_height, DEFAULT_INK);
                    latch = 0;
                } else {
                    /* a space */
                    latch = 1;
                }
                i += block_width;
            } while (i < 11 + comp_offset);
            draw_bar(pixelbuf, (46 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (48 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            latch = 1;
            i = 85 + comp_offset;
            do {
                int module_fill = module_is_set(symbol, symbol->rows - 1, i);
                int block_width = 0;
                do {
                    block_width++;
                } while ((i + block_width < symbol->width)
                        && module_is_set(symbol, symbol->rows - 1, i + block_width) == module_fill);
                if (latch == 1) {
                    /* a bar */
                    draw_bar(pixelbuf, (i + xoffset - comp_offset) * si, block_width * si, guardoffset * si, 5 * si,
                            image_width, image_height, DEFAULT_INK);
                    latch = 0;
                } else {
                    /* a space */
                    latch = 1;
                }
                i += block_width;
            } while (i < 96 + comp_offset);

        } else if (upceanflag == 13) { /* EAN-13 */
            draw_bar(pixelbuf, (0 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (2 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (46 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (48 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (92 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
            draw_bar(pixelbuf, (94 + xoffset) * si, 1 * si, guardoffset * si, 5 * si, image_width, image_height,
                    DEFAULT_INK);
        }
    }

    if (!hide_text) {

        if (upceanflag) {
            /* Note font sizes halved as in pixels */

            /* Halved again to get middle position that draw_string() expects */
            int upcea_width_adj = (UPCEAN_SMALL_FONT_WIDTH + 3) / 4;
            int upcea_height_adj = (UPCEAN_FONT_HEIGHT - UPCEAN_SMALL_FONT_HEIGHT) * si / 2;
            /* Halved again to get middle position that draw_string() expects */
            int ean_width_adj = (UPCEAN_FONT_WIDTH + 3) / 4;

            output_upcean_split_text(upceanflag, symbol->text, textpart1, textpart2, textpart3, textpart4);

            if (upceanflag == 6) { /* UPC-E */
                textpos = (-(5 + upcea_width_adj) + xoffset) * si;
                draw_string(pixelbuf, textpart1, textpos, default_text_posn + upcea_height_adj,
                            textflags | SMALL_TEXT, image_width, image_height, si);
                textpos = (24 + xoffset) * si;
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height,
                            si);
                textpos = (51 + 3 + upcea_width_adj + xoffset) * si;
                draw_string(pixelbuf, textpart3, textpos, default_text_posn + upcea_height_adj,
                            textflags | SMALL_TEXT, image_width, image_height, si);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = (61 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height,
                                    si);
                        break;
                    case 5:
                        textpos = (75 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height,
                                    si);
                        break;
                }

            } else if (upceanflag == 8) { /* EAN-8 */
                textpos = (17 + xoffset) * si;
                draw_string(pixelbuf, textpart1, textpos, default_text_posn, textflags, image_width, image_height,
                            si);
                textpos = (50 + xoffset) * si;
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height,
                            si);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = (77 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height,
                                    si);
                        break;
                    case 5:
                        textpos = (91 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height,
                                    si);
                        break;
                }

            } else if (upceanflag == 12) { /* UPC-A */
                textpos = (-(5 + upcea_width_adj) + xoffset) * si;
                draw_string(pixelbuf, textpart1, textpos, default_text_posn + upcea_height_adj,
                            textflags | SMALL_TEXT, image_width, image_height, si);
                textpos = (27 + xoffset) * si;
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height,
                            si);
                textpos = (67 + xoffset) * si;
                draw_string(pixelbuf, textpart3, textpos, default_text_posn, textflags, image_width, image_height,
                            si);
                textpos = (95 + 5 + upcea_width_adj + xoffset) * si;
                draw_string(pixelbuf, textpart4, textpos, default_text_posn + upcea_height_adj,
                            textflags | SMALL_TEXT, image_width, image_height, si);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = (105 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height,
                                    si);
                        break;
                    case 5:
                        textpos = (119 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height,
                                    si);
                        break;
                }

            } else if (upceanflag == 13) { /* EAN-13 */
                textpos = (-(5 + ean_width_adj) + xoffset) * si;
                draw_string(pixelbuf, textpart1, textpos, default_text_posn, textflags, image_width, image_height,
                            si);
                textpos = (24 + xoffset) * si;
                draw_string(pixelbuf, textpart2, textpos, default_text_posn, textflags, image_width, image_height,
                            si);
                textpos = (71 + xoffset) * si;
                draw_string(pixelbuf, textpart3, textpos, default_text_posn, textflags, image_width, image_height,
                            si);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = (105 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height,
                                    si);
                        break;
                    case 5:
                        textpos = (119 + xoffset + addon_gap) * si;
                        draw_string(pixelbuf, addon, textpos, addon_text_posn, textflags, image_width, image_height,
                                    si);
                        break;
                }
            }
        }

        if (!textdone) {
            /* Suppress clang-analyzer-core.CallAndMessage warning */
            unsigned char local_text[sizeof(symbol->text)] = {0};
            to_iso8859_1(symbol->text, local_text);
            /* Put the human readable text at the bottom */
            textpos = (main_width / 2 + xoffset) * si;
            draw_string(pixelbuf, local_text, textpos, default_text_posn, textflags, image_width, image_height,
                        si);
        }
    }

    xoffset -= comp_offset;

    // Binding and boxes
    if (symbol->output_options & BARCODE_BIND) {
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
                            ((r * row_height) + textoffset + yoffset - sep_height / 2) * si, sep_height * si,
                            image_width, image_height, DEFAULT_INK);
                }
            } else {
                for (r = 1; r < symbol->rows; r++) {
                    /* Avoid 11-module start and 13-module stop chars */
                    row_height = symbol->row_height[r - 1] ? symbol->row_height[r - 1] : large_bar_height;
                    draw_bar(pixelbuf, (xoffset + 11) * si, (symbol->width - 24) * si,
                            ((r * row_height) + textoffset + yoffset - sep_height / 2) * si, sep_height * si,
                            image_width, image_height, DEFAULT_INK);
                }
            }
        }
    }

    draw_bind_box(symbol, pixelbuf, xoffset, roffset, textoffset, 0 /*overspill*/, image_width, image_height, si);

    if (!half_int_scaling) {
        scale_width = image_width * scaler;
        scale_height = image_height * scaler;

        /* Apply scale options by creating another pixel buffer */
        if (!(scaled_pixelbuf = (unsigned char *) malloc((size_t) scale_width * scale_height))) {
            free(pixelbuf);
            strcpy(symbol->errtxt, "659: Insufficient memory for pixel buffer");
            return ZINT_ERROR_MEMORY;
        }
        memset(scaled_pixelbuf, DEFAULT_PAPER, (size_t) scale_width * scale_height);

        for (vert = 0; vert < scale_height; vert++) {
            int vert_row = vert * scale_width;
            int image_vert_row = ((int) (vert / scaler)) * image_width;
            for (horiz = 0; horiz < scale_width; horiz++) {
                *(scaled_pixelbuf + vert_row + horiz) = *(pixelbuf + image_vert_row + (int) (horiz / scaler));
            }
        }

        error_number = save_raster_image_to_file(symbol, scale_height, scale_width, scaled_pixelbuf, rotate_angle,
                                                file_type);
        if (rotate_angle || file_type != OUT_BUFFER || !(symbol->output_options & OUT_BUFFER_INTERMEDIATE)) {
            free(scaled_pixelbuf);
        }
        free(pixelbuf);
    } else {
        error_number = save_raster_image_to_file(symbol, image_height, image_width, pixelbuf, rotate_angle,
                                                file_type);
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
