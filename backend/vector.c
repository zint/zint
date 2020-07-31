/*  vector.c - Creates vector image objects

    libzint - the open source barcode library
    Copyright (C) 2018 - 2020 Robin Stuart <rstuart114@gmail.com>

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
#include <math.h>

#ifdef _MSC_VER
#include <malloc.h>
#endif

#include "common.h"
#include "output.h"

INTERNAL int ps_plot(struct zint_symbol *symbol);
INTERNAL int svg_plot(struct zint_symbol *symbol);
INTERNAL int emf_plot(struct zint_symbol *symbol);

static struct zint_vector_rect *vector_plot_create_rect(double x, double y, double width, double height) {
    struct zint_vector_rect *rect;

    rect = (struct zint_vector_rect*) malloc(sizeof (struct zint_vector_rect));
    if (!rect) return NULL;

    rect->next = NULL;
    rect->x = x;
    rect->y = y;
    rect->width = width;
    rect->height = height;
    rect->colour = -1; // Default colour

    return rect;
}

static int vector_plot_add_rect(struct zint_symbol *symbol, struct zint_vector_rect *rect, struct zint_vector_rect **last_rect) {
    if (!rect) return ZINT_ERROR_MEMORY;
    if (*last_rect)
        (*last_rect)->next = rect;
    else
        symbol->vector->rectangles = rect; // first rectangle

    *last_rect = rect;
    return 1;
}

static struct zint_vector_hexagon *vector_plot_create_hexagon(double x, double y, double diameter) {
    struct zint_vector_hexagon *hexagon;

    hexagon = (struct zint_vector_hexagon*) malloc(sizeof (struct zint_vector_hexagon));
    if (!hexagon) return NULL;
    hexagon->next = NULL;
    hexagon->x = x;
    hexagon->y = y;
    hexagon->diameter = (diameter * 5.0) / 4.0; // Ugly kludge for legacy support

    return hexagon;
}

static int vector_plot_add_hexagon(struct zint_symbol *symbol, struct zint_vector_hexagon *hexagon, struct zint_vector_hexagon **last_hexagon) {
    if (!hexagon) return ZINT_ERROR_MEMORY;
    if (*last_hexagon)
        (*last_hexagon)->next = hexagon;
    else
        symbol->vector->hexagons = hexagon; // first hexagon

    *last_hexagon = hexagon;
    return 1;
}

static struct zint_vector_circle *vector_plot_create_circle(double x, double y, double diameter, int colour) {
    struct zint_vector_circle *circle;

    circle = (struct zint_vector_circle *) malloc(sizeof (struct zint_vector_circle));
    if (!circle) return NULL;
    circle->next = NULL;
    circle->x = x;
    circle->y = y;
    circle->diameter = diameter;
    circle->colour = colour;

    return circle;
}

static int vector_plot_add_circle(struct zint_symbol *symbol, struct zint_vector_circle *circle, struct zint_vector_circle **last_circle) {
    if (!circle) return ZINT_ERROR_MEMORY;
    if (*last_circle)
        (*last_circle)->next = circle;
    else
        symbol->vector->circles = circle; // first circle

    *last_circle = circle;
    return 1;
}

static int vector_plot_add_string(struct zint_symbol *symbol,
        unsigned char *text, double x, double y, double fsize, double width,
        struct zint_vector_string **last_string) {
    struct zint_vector_string *string;

    string = (struct zint_vector_string*) malloc(sizeof (struct zint_vector_string));
    string->next = NULL;
    string->x = x;
    string->y = y;
    string->width = width;
    string->fsize = fsize;
    string->length = ustrlen(text);
    string->text = (unsigned char*) malloc(sizeof (unsigned char) * (ustrlen(text) + 1));
    ustrcpy(string->text, text);

    if (*last_string)
        (*last_string)->next = string;
    else
        symbol->vector->strings = string; // First text portion
    *last_string = string;

    return 1;
}

INTERNAL void vector_free(struct zint_symbol *symbol) {
    if (symbol->vector != NULL) {
        struct zint_vector_rect *rect;
        struct zint_vector_hexagon *hex;
        struct zint_vector_circle *circle;
        struct zint_vector_string *string;

        // Free Rectangles
        rect = symbol->vector->rectangles;
        while (rect) {
            struct zint_vector_rect *r = rect;
            rect = rect->next;
            free(r);
        }

        // Free Hexagons
        hex = symbol->vector->hexagons;
        while (hex) {
            struct zint_vector_hexagon *h = hex;
            hex = hex->next;
            free(h);
        }

        // Free Circles
        circle = symbol->vector->circles;
        while (circle) {
            struct zint_vector_circle *c = circle;
            circle = circle->next;
            free(c);
        }

        // Free Strings
        string = symbol->vector->strings;
        while (string) {
            struct zint_vector_string *s = string;
            string = string->next;
            free(s->text);
            free(s);
        }

        // Free vector
        free(symbol->vector);
        symbol->vector = NULL;
    }
}

static void vector_scale(struct zint_symbol *symbol, int file_type) {
    struct zint_vector_rect *rect;
    struct zint_vector_hexagon *hex;
    struct zint_vector_circle *circle;
    struct zint_vector_string *string;
    double scale = symbol->scale * 2.0;

    if ((file_type == OUT_EMF_FILE) && (symbol->symbology == BARCODE_MAXICODE)) {
        // Increase size to overcome limitations in EMF file format
        scale *= 20;
    }
    
    symbol->vector->width *= scale;
    symbol->vector->height *= scale;

    rect = symbol->vector->rectangles;
    while (rect) {
        rect->x *= scale;
        rect->y *= scale;
        rect->height *= scale;
        rect->width *= scale;
        rect = rect->next;
    }

    hex = symbol->vector->hexagons;
    while (hex) {
        hex->x *= scale;
        hex->y *= scale;
        hex->diameter *= scale;
        hex = hex->next;
    }

    circle = symbol->vector->circles;
    while (circle) {
        circle->x *= scale;
        circle->y *= scale;
        circle->diameter *= scale;
        circle = circle->next;
    }

    string = symbol->vector->strings;
    while (string) {
        string->x *= scale;
        string->y *= scale;
        string->width *= scale;
        string->fsize *= scale;
        string = string->next;
    }
    return;
}

static void vector_reduce_rectangles(struct zint_symbol *symbol) {
    // Looks for vertically aligned rectangles and merges them together
    struct zint_vector_rect *rect, *target, *prev;

    rect = symbol->vector->rectangles;
    while (rect) {
        prev = rect;
        target = prev->next;

        while (target) {
            if ((rect->x == target->x) && (rect->width == target->width) && ((rect->y + rect->height) == target->y) && (rect->colour == target->colour)) {
                rect->height += target->height;
                prev->next = target->next;
                free(target);
            } else {
                prev = target;
            }
            target = prev->next;
        }

        rect = rect->next;
    }

    return;
}

INTERNAL int plot_vector(struct zint_symbol *symbol, int rotate_angle, int file_type) {
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

    double text_height;
    int rect_count, last_row_start;
    int this_row;

    struct zint_vector *vector;
    struct zint_vector_rect *rectangle, *rect, *last_rectangle = NULL;
    struct zint_vector_hexagon *last_hexagon = NULL;
    struct zint_vector_string *last_string = NULL;
    struct zint_vector_circle *last_circle = NULL;

    (void)rotate_angle; /* Not currently implemented */

    // Free any previous rendering structures
    vector_free(symbol);

    // Sanity check colours
    error_number = output_check_colour_options(symbol);
    if (error_number != 0) {
        return error_number;
    }

    // Allocate memory
    vector = symbol->vector = (struct zint_vector *) malloc(sizeof (struct zint_vector));
    if (!vector) return ZINT_ERROR_MEMORY;
    vector->rectangles = NULL;
    vector->hexagons = NULL;
    vector->circles = NULL;
    vector->strings = NULL;

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

    if (hide_text) {
        text_height = 0.0;
        textoffset = upceanflag ? 9.0 : 0.0;
    } else {
        text_height = upceanflag ? 11.0 : 9.0;
        textoffset = 9.0;
    }
    if (symbol->output_options & SMALL_TEXT)
        text_height *= 0.8;

    vector->width = ceil(symbol->width + (xoffset + roffset));
    vector->height = ceil(symbol->height + textoffset + (yoffset + boffset));

    if (symbol->border_width > 0 && ((symbol->output_options & BARCODE_BOX) || (symbol->output_options & BARCODE_BIND))) {
        default_text_posn = symbol->height + textoffset + symbol->border_width + symbol->border_width;
    } else {
        default_text_posn = symbol->height + textoffset;
    }

    row_height = 0.0;
    rect_count = 0;
    last_row_start = 0;

    // Plot rectangles - most symbols created here
    if ((symbol->symbology != BARCODE_MAXICODE) && ((symbol->output_options & BARCODE_DOTTY_MODE) == 0)) {
        for (r = 0; r < symbol->rows; r++) {
            this_row = r;
            last_row_start = rect_count;
            if (symbol->row_height[this_row] == 0) {
                row_height = large_bar_height;
            } else {
                row_height = symbol->row_height[this_row];
            }
            row_posn = 0;
            for (i = 0; i < r; i++) {
                if (symbol->row_height[i] == 0) {
                    row_posn += large_bar_height;
                } else {
                    row_posn += symbol->row_height[i];
                }
            }
            row_posn += yoffset;

            i = 0;

            do {
                int block_width = 0;
                do {
                    block_width++;
                } while (i + block_width < symbol->width && module_is_set(symbol, this_row, i + block_width) == module_is_set(symbol, this_row, i));
                if ((addon_latch == 0) && (r == (symbol->rows - 1)) && (i > main_width)) {
                    addon_text_posn = row_posn + 8.0;
                    addon_latch = 1;
                }
                if (module_is_set(symbol, this_row, i)) {
                    /* a bar or colour block */
                    if (addon_latch == 0) {
                        rectangle = vector_plot_create_rect(i + xoffset, row_posn, block_width, row_height);
                        if (symbol->symbology == BARCODE_ULTRA) {
                            rectangle->colour = module_is_set(symbol, this_row, i);
                        }
                    } else {
                        if (upceanflag == 12 || upceanflag == 6) { /* UPC-A/E don't descend */
                            rectangle = vector_plot_create_rect(i + xoffset, row_posn + 10.0, block_width, row_height > 10.0 ? row_height - 10.0 : 1.0);
                        } else {
                            rectangle = vector_plot_create_rect(i + xoffset, row_posn + 10.0, block_width, row_height > 5.0 ? row_height - 5.0 : 1.0);
                        }
                    }
                    vector_plot_add_rect(symbol, rectangle, &last_rectangle);
                    rect_count++;
                }
                i += block_width;

            } while (i < symbol->width);
        }
    }

    // Plot Maxicode symbols
    if (symbol->symbology == BARCODE_MAXICODE) {
        struct zint_vector_circle *circle;
        vector->width = 37.0 + (xoffset + roffset);
        vector->height = 36.0 + (yoffset + boffset);

        // Bullseye
        circle = vector_plot_create_circle(17.88 + xoffset, 17.8 + yoffset, 10.85, 0);
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(17.88 + xoffset, 17.8 + yoffset, 8.97, 1);
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(17.88 + xoffset, 17.8 + yoffset, 7.10, 0);
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(17.88 + xoffset, 17.8 + yoffset, 5.22, 1);
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(17.88 + xoffset, 17.8 + yoffset, 3.31, 0);
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(17.88 + xoffset, 17.8 + yoffset, 1.43, 1);
        vector_plot_add_circle(symbol, circle, &last_circle);

        /* Hexagons */
        for (r = 0; r < symbol->rows; r++) {
            for (i = 0; i < symbol->width; i++) {
                if (module_is_set(symbol, r, i)) {
                    //struct zint_vector_hexagon *hexagon = vector_plot_create_hexagon(((i * 0.88) + ((r & 1) ? 1.76 : 1.32)), ((r * 0.76) + 0.76), symbol->dot_size);
                    struct zint_vector_hexagon *hexagon = vector_plot_create_hexagon(((i * 1.23) + 0.615 + ((r & 1) ? 0.615 : 0.0)) + xoffset,
                                                                                     ((r * 1.067) + 0.715) + yoffset, symbol->dot_size);
                    vector_plot_add_hexagon(symbol, hexagon, &last_hexagon);
                }
            }
        }
    }

    // Dotty mode
    if ((symbol->symbology != BARCODE_MAXICODE) && (symbol->output_options & BARCODE_DOTTY_MODE)) {
        for (r = 0; r < symbol->rows; r++) {
            for (i = 0; i < symbol->width; i++) {
                if (module_is_set(symbol, r, i)) {
                    struct zint_vector_circle *circle = vector_plot_create_circle(i + 0.5 + xoffset, r + 0.5 + yoffset, 1.0, 0);
                    vector_plot_add_circle(symbol, circle, &last_circle);
                }
            }
        }
    }

    if (upceanflag) {
        /* Guard bar extension */
        if (upceanflag == 6) { /* UPC-E */
            i = 0;
            for (rect = symbol->vector->rectangles; rect != NULL; rect = rect->next) {
                switch (i - last_row_start) {
                    case 0:
                    case 1:
                    case 14:
                    case 15:
                    case 16:
                        rect->height += 5.0;
                        break;
                }
                i++;
            }
        } else if (upceanflag == 8) { /* EAN-8 */
            i = 0;
            for (rect = symbol->vector->rectangles; rect != NULL; rect = rect->next) {
                switch (i - last_row_start) {
                    case 0:
                    case 1:
                    case 10:
                    case 11:
                    case 20:
                    case 21:
                        rect->height += 5.0;
                        break;
                }
                i++;
            }
        } else if (upceanflag == 12) { /* UPC-A */
            i = 0;
            for (rect = symbol->vector->rectangles; rect != NULL; rect = rect->next) {
                switch (i - last_row_start) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 14:
                    case 15:
                    case 26:
                    case 27:
                    case 28:
                    case 29:
                        rect->height += 5.0;
                        break;
                }
                i++;
            }
        } else if (upceanflag == 13) { /* EAN-13 */
            i = 0;
            for (rect = symbol->vector->rectangles; rect != NULL; rect = rect->next) {
                switch (i - last_row_start) {
                    case 0:
                    case 1:
                    case 14:
                    case 15:
                    case 28:
                    case 29:
                        rect->height += 5.0;
                        break;
                }
                i++;
            }
        }
    }

    /* Add the text */

    if (!hide_text) {

        xoffset += comp_offset;

        if (upceanflag) {
            double textwidth;
            output_upcean_split_text(upceanflag, symbol->text, textpart1, textpart2, textpart3, textpart4);

            if (upceanflag == 6) { /* UPC-E */
                textpos = -5 + xoffset;
                textwidth = 6.2;
                vector_plot_add_string(symbol, textpart1, textpos, default_text_posn - 2.0, text_height * (8.0 / 11.0), textwidth, &last_string);
                textpos = 24 + xoffset;
                textwidth = 6.0 * 8.5;
                vector_plot_add_string(symbol, textpart2, textpos, default_text_posn, text_height, textwidth, &last_string);
                textpos = 55 + xoffset;
                textwidth = 6.2;
                vector_plot_add_string(symbol, textpart3, textpos, default_text_posn - 2.0, text_height * (8.0 / 11.0), textwidth, &last_string);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 61 + xoffset + addon_gap;
                        textwidth = 2.0 * 8.5;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, &last_string);
                        break;
                    case 5:
                        textpos = 75 + xoffset + addon_gap;
                        textwidth = 5.0 * 8.5;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, &last_string);
                        break;
                }

            } else if (upceanflag == 8) { /* EAN-8 */
                textpos = 17 + xoffset;
                textwidth = 4.0 * 8.5;
                vector_plot_add_string(symbol, textpart1, textpos, default_text_posn, text_height, textwidth, &last_string);
                textpos = 50 + xoffset;
                vector_plot_add_string(symbol, textpart2, textpos, default_text_posn, text_height, textwidth, &last_string);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 77 + xoffset + addon_gap;
                        textwidth = 2.0 * 8.5;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, &last_string);
                        break;
                    case 5:
                        textpos = 91 + xoffset + addon_gap;
                        textwidth = 5.0 * 8.5;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, &last_string);
                        break;
                }

            } else if (upceanflag == 12) { /* UPC-A */
                textpos = -5 + xoffset;
                textwidth = 6.2;
                vector_plot_add_string(symbol, textpart1, textpos, default_text_posn - 2.0, text_height * (8.0 / 11.0), textwidth, &last_string);
                textpos = 27 + xoffset;
                textwidth = 5.0 * 8.5;
                vector_plot_add_string(symbol, textpart2, textpos, default_text_posn, text_height, textwidth, &last_string);
                textpos = 68 + xoffset;
                vector_plot_add_string(symbol, textpart3, textpos, default_text_posn, text_height, textwidth, &last_string);
                textpos = 100 + xoffset;
                textwidth = 6.2;
                vector_plot_add_string(symbol, textpart4, textpos, default_text_posn - 2.0, text_height * (8.0 / 11.0), textwidth, &last_string);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 107 + xoffset + addon_gap;
                        textwidth = 2.0 * 8.5;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, &last_string);
                        break;
                    case 5:
                        textpos = 121 + xoffset + addon_gap;
                        textwidth = 5.0 * 8.5;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, &last_string);
                        break;
                }

            } else if (upceanflag == 13) { /* EAN-13 */
                textpos = -7 + xoffset;
                textwidth = 8.5;
                vector_plot_add_string(symbol, textpart1, textpos, default_text_posn, text_height, textwidth, &last_string);
                textpos = 24 + xoffset;
                textwidth = 6.0 * 8.5;
                vector_plot_add_string(symbol, textpart2, textpos, default_text_posn, text_height, textwidth, &last_string);
                textpos = 71 + xoffset;
                vector_plot_add_string(symbol, textpart3, textpos, default_text_posn, text_height, textwidth, &last_string);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 105 + xoffset + addon_gap;
                        textwidth = 2.0 * 8.5;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, &last_string);
                        break;
                    case 5:
                        textpos = 119 + xoffset + addon_gap;
                        textwidth = 5.0 * 8.5;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, &last_string);
                        break;
                }
            }
        }

        if (!textdone) {
            /* Put normal human readable text at the bottom (and centered) */
            // calculate start xoffset to center text
            vector_plot_add_string(symbol, symbol->text, main_width / 2.0 + xoffset, default_text_posn, text_height, symbol->width, &last_string);
        }

        xoffset -= comp_offset; // Restore xoffset
    }

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
                    rectangle = vector_plot_create_rect(xoffset, (r * row_height) + yoffset - sep_height / 2, symbol->width, sep_height);
                    vector_plot_add_rect(symbol, rectangle, &last_rectangle);
                }
            } else {
                for (r = 1; r < symbol->rows; r++) {
                    /* Avoid 11-module start and 13-module stop chars */
                    rectangle = vector_plot_create_rect(xoffset + 11, (r * row_height) + yoffset - sep_height / 2, symbol->width - 24, sep_height);
                    vector_plot_add_rect(symbol, rectangle, &last_rectangle);
                }
            }
        }
    }
    if (symbol->border_width > 0) {
        if ((symbol->output_options & BARCODE_BOX) || (symbol->output_options & BARCODE_BIND)) {
            // Top
            rectangle = vector_plot_create_rect(0.0, 0.0, vector->width, symbol->border_width);
            if (!(symbol->output_options & BARCODE_BOX) && (symbol->symbology == BARCODE_CODABLOCKF || symbol->symbology == BARCODE_HIBC_BLOCKF)) {
                rectangle->x = xoffset;
                rectangle->width -= (2.0 * xoffset);
            }
            vector_plot_add_rect(symbol, rectangle, &last_rectangle);
            // Bottom
            rectangle = vector_plot_create_rect(0.0, vector->height - symbol->border_width - textoffset, vector->width, symbol->border_width);
            if (!(symbol->output_options & BARCODE_BOX) && (symbol->symbology == BARCODE_CODABLOCKF || symbol->symbology == BARCODE_HIBC_BLOCKF)) {
                rectangle->x = xoffset;
                rectangle->width -= (2.0 * xoffset);
            }
            vector_plot_add_rect(symbol, rectangle, &last_rectangle);
        }
        if (symbol->output_options & BARCODE_BOX) {
            // Left
            rectangle = vector_plot_create_rect(0.0, 0.0, symbol->border_width, vector->height - textoffset);
            vector_plot_add_rect(symbol, rectangle, &last_rectangle);
            // Right
            rectangle = vector_plot_create_rect(vector->width - symbol->border_width, 0.0, symbol->border_width, vector->height - textoffset);
            vector_plot_add_rect(symbol, rectangle, &last_rectangle);
        }
    }

    vector_reduce_rectangles(symbol);

    vector_scale(symbol, file_type);

    switch (file_type) {
        case OUT_EPS_FILE:
            error_number = ps_plot(symbol);
            break;
        case OUT_SVG_FILE:
            error_number = svg_plot(symbol);
            break;
        case OUT_EMF_FILE:
            error_number = emf_plot(symbol);
            break;
        /* case OUT_BUFFER: No more work needed */
    }

    return error_number;
}
