/*  vector.c - Creates vector image objects

    libzint - the open source barcode library
    Copyright (C) 2018 - 2021 Robin Stuart <rstuart114@gmail.com>

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
#include "zfiletypes.h"

INTERNAL int ps_plot(struct zint_symbol *symbol);
INTERNAL int svg_plot(struct zint_symbol *symbol);
INTERNAL int emf_plot(struct zint_symbol *symbol, int rotate_angle);

static struct zint_vector_rect *vector_plot_create_rect(float x, float y, float width, float height) {
    struct zint_vector_rect *rect;

    rect = (struct zint_vector_rect *) malloc(sizeof(struct zint_vector_rect));
    if (!rect) return NULL;

    rect->next = NULL;
    rect->x = x;
    rect->y = y;
    rect->width = width;
    rect->height = height;
    rect->colour = -1; // Default colour

    return rect;
}

static int vector_plot_add_rect(struct zint_symbol *symbol, struct zint_vector_rect *rect,
            struct zint_vector_rect **last_rect) {
    if (!rect) return 0;
    if (*last_rect)
        (*last_rect)->next = rect;
    else
        symbol->vector->rectangles = rect; // first rectangle

    *last_rect = rect;
    return 1;
}

static struct zint_vector_hexagon *vector_plot_create_hexagon(float x, float y, float diameter) {
    struct zint_vector_hexagon *hexagon;

    hexagon = (struct zint_vector_hexagon *) malloc(sizeof(struct zint_vector_hexagon));
    if (!hexagon) return NULL;
    hexagon->next = NULL;
    hexagon->x = x;
    hexagon->y = y;
    hexagon->diameter = diameter;
    hexagon->rotation = 0;

    return hexagon;
}

static int vector_plot_add_hexagon(struct zint_symbol *symbol, struct zint_vector_hexagon *hexagon,
            struct zint_vector_hexagon **last_hexagon) {
    if (!hexagon) return 0;
    if (*last_hexagon)
        (*last_hexagon)->next = hexagon;
    else
        symbol->vector->hexagons = hexagon; // first hexagon

    *last_hexagon = hexagon;
    return 1;
}

static struct zint_vector_circle *vector_plot_create_circle(float x, float y, float diameter, int colour) {
    struct zint_vector_circle *circle;

    circle = (struct zint_vector_circle *) malloc(sizeof(struct zint_vector_circle));
    if (!circle) return NULL;
    circle->next = NULL;
    circle->x = x;
    circle->y = y;
    circle->diameter = diameter;
    circle->colour = colour;

    return circle;
}

static int vector_plot_add_circle(struct zint_symbol *symbol, struct zint_vector_circle *circle,
            struct zint_vector_circle **last_circle) {
    if (!circle) return 0;
    if (*last_circle)
        (*last_circle)->next = circle;
    else
        symbol->vector->circles = circle; // first circle

    *last_circle = circle;
    return 1;
}

static int vector_plot_add_string(struct zint_symbol *symbol,
        unsigned char *text, float x, float y, float fsize, float width, int halign,
        struct zint_vector_string **last_string) {
    struct zint_vector_string *string;

    string = (struct zint_vector_string *) malloc(sizeof(struct zint_vector_string));
    if (!string) return 0;
    string->next = NULL;
    string->x = x;
    string->y = y;
    string->width = width;
    string->fsize = fsize;
    string->length = (int) ustrlen(text);
    string->rotation = 0;
    string->halign = halign;
    string->text = (unsigned char *) malloc(ustrlen(text) + 1);
    if (!string->text) { free(string); return 0; }
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
    float scale = symbol->scale * 2.0f;

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

static void vector_rotate(struct zint_symbol *symbol, int rotate_angle) {
    // Rotates the image
    struct zint_vector_rect *rect;
    struct zint_vector_hexagon *hex;
    struct zint_vector_circle *circle;
    struct zint_vector_string *string;
    float temp;
    
    if (rotate_angle == 0) {
        // No rotation needed
        return;
    }
    
    rect = symbol->vector->rectangles;
    while (rect) {
        if (rotate_angle == 90) {
            temp = rect->x;
            rect->x = symbol->vector->height - (rect->y + rect->height);
            rect->y = temp;
            temp = rect->width;
            rect->width = rect->height;
            rect->height = temp;
        } else if (rotate_angle == 180) {
            rect->x = symbol->vector->width - (rect->x + rect->width);
            rect->y = symbol->vector->height - (rect->y + rect->height);
        } else if (rotate_angle == 270) {
            temp = rect->x;
            rect->x = rect->y;
            rect->y = symbol->vector->width - (temp + rect->width);
            temp = rect->width;
            rect->width = rect->height;
            rect->height = temp;
        }
        rect = rect->next;
    }
    
    hex = symbol->vector->hexagons;
    while (hex) {
        if (rotate_angle == 90) {
            temp = hex->x;
            hex->x = symbol->vector->height - hex->y;
            hex->y = temp;
            hex->rotation = 90;
        } else if (rotate_angle == 180) {
            hex->x = symbol->vector->width - hex->x;
            hex->y = symbol->vector->height - hex->y;
            hex->rotation = 180;
        } else if (rotate_angle == 270) {
            temp = hex->x;
            hex->x = hex->y;
            hex->y = symbol->vector->width - temp;
            hex->rotation = 270;
        }
        hex = hex->next;
    }
    
    circle = symbol->vector->circles;
    while (circle) {
        if (rotate_angle == 90) {
            temp = circle->x;
            circle->x = symbol->vector->height - circle->y;
            circle->y = temp;
        } else if (rotate_angle == 180) {
            circle->x = symbol->vector->width - circle->x;
            circle->y = symbol->vector->height - circle->y;
        } else if (rotate_angle == 270) {
            temp = circle->x;
            circle->x = circle->y;
            circle->y = symbol->vector->width - temp;
        }
        circle = circle->next;
    }
    
    string = symbol->vector->strings;
    while (string) {
        if (rotate_angle == 90) {
            temp = string->x;
            string->x = symbol->vector->height - string->y;
            string->y = temp;
            string->rotation = 90;
        } else if (rotate_angle == 180) {
            string->x = symbol->vector->width - string->x;
            string->y = symbol->vector->height - string->y;
            string->rotation = 180;
        } else if (rotate_angle == 270) {
            temp = string->x;
            string->x = string->y;
            string->y = symbol->vector->width - temp;
            string->rotation = 270;
        }
        string = string->next;
    }
    
    if ((rotate_angle == 90) || (rotate_angle == 270)) {
        temp = symbol->vector->height;
        symbol->vector->height = symbol->vector->width;
        symbol->vector->width = temp;
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
            if ((rect->x == target->x) && (rect->width == target->width) && ((rect->y + rect->height) == target->y)
                    && (rect->colour == target->colour)) {
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
    float large_bar_height;
    int textdone = 0;
    int main_width;
    int comp_offset = 0;
    unsigned char addon[6];
    int addon_gap = 0;
    float addon_text_posn = 0.0f;
    float addon_bar_height = 0.0f;
    float xoffset, yoffset, roffset, boffset;
    float textoffset;
    float default_text_posn;
    float row_height, row_posn;
    int upceanflag = 0;
    int addon_latch = 0;
    unsigned char textpart1[5], textpart2[7], textpart3[7], textpart4[2];
    float textpos;
    int hide_text;
    int i, r;
    int text_height; /* Font pixel size (so whole integers) */

    int upcae_outside_text_height = 0; /* UPC-A/E outside digits font size */
    float digit_ascent_factor = 0.25f; /* Assuming digit ascent roughly 25% less than font size */
    float text_gap; /* Gap between barcode and text */
    float dot_overspill = 0.0f;
    float dot_offset = 0.0f;
    int rect_count, last_row_start = 0;
    int this_row;

    struct zint_vector *vector;
    struct zint_vector_rect *rectangle, *rect, *last_rectangle = NULL;
    struct zint_vector_hexagon *last_hexagon = NULL;
    struct zint_vector_string *last_string = NULL;
    struct zint_vector_circle *last_circle = NULL;

    // Free any previous rendering structures
    vector_free(symbol);

    // Sanity check colours
    error_number = output_check_colour_options(symbol);
    if (error_number != 0) {
        return error_number;
    }

    // Allocate memory
    vector = symbol->vector = (struct zint_vector *) malloc(sizeof(struct zint_vector));
    if (!vector) return ZINT_ERROR_MEMORY;
    vector->rectangles = NULL;
    vector->hexagons = NULL;
    vector->circles = NULL;
    vector->strings = NULL;

    large_bar_height = output_large_bar_height(symbol, 0 /*No rounding to scale*/);

    main_width = symbol->width;

    if (is_extendable(symbol->symbology)) {
        upceanflag = output_process_upcean(symbol, &main_width, &comp_offset, addon, &addon_gap);
    }

    output_set_whitespace_offsets(symbol, &xoffset, &yoffset, &roffset, &boffset);

    /* Note font sizes scaled by 2 so really twice these values */
    if (upceanflag) {
        /* Note BOLD_TEXT ignored for UPCEAN by svg/emf/ps/qzint */
        text_height = symbol->output_options & SMALL_TEXT ? 7 : 10;
        upcae_outside_text_height = symbol->output_options & SMALL_TEXT ? 6 : 7;
        /* Negative to move close to barcode (less digit ascent, then add 0.5X) */
        text_gap = -text_height * digit_ascent_factor + 0.5f;
    } else {
        text_height = symbol->output_options & SMALL_TEXT ? 6 : 7;
        text_gap = text_height * 0.1f;
    }

    hide_text = ((!symbol->show_hrt) || (ustrlen(symbol->text) == 0));

    if (hide_text) {
        textoffset = upceanflag && upceanflag != 2 && upceanflag != 5 ? 5.0f : 0.0f; /* Allow for guard bars */
    } else {
        if (upceanflag) {
            textoffset = text_height + 0.2f + text_gap; /* Add fudge for anti-aliasing of digits */
        } else {
            textoffset = text_height * 1.25f + text_gap; /* Allow +25% for characters descending below baseline */
        }
    }

    if ((symbol->symbology != BARCODE_MAXICODE) && (symbol->output_options & BARCODE_DOTTY_MODE)) {
        if (symbol->dot_size < 1.0f) {
            dot_overspill = 0.0f;
            /* Offset (1 - dot_size) / 2 + dot_radius == (1 - dot_size + dot_size) / 2 == 1 / 2 */
            dot_offset = 0.5f;
        } else { /* Allow for exceeding 1X */
            dot_overspill = symbol->dot_size - 1.0f + 0.1f; /* Fudge for anti-aliasing */
            dot_offset = symbol->dot_size / 2.0f + 0.05f; /* Fudge for anti-aliasing */
        }
    }

    vector->width = symbol->width + dot_overspill + (xoffset + roffset);
    vector->height = symbol->height + textoffset + dot_overspill + (yoffset + boffset);

    if (symbol->border_width > 0 && (symbol->output_options & (BARCODE_BOX | BARCODE_BIND))) {
        default_text_posn = symbol->height + text_height + text_gap + symbol->whitespace_height
                            + symbol->border_width * 2;
    } else {
        default_text_posn = symbol->height + text_height + text_gap + symbol->whitespace_height;
    }

    // Plot Maxicode symbols
    if (symbol->symbology == BARCODE_MAXICODE) {
        struct zint_vector_circle *circle;
        float bull_x, bull_y, bull_d_incr;
        const float two_div_sqrt3 = 1.1547f; /* 2 / √3 */
        const float sqrt3_div_two = 0.866f; /* √3 / 2 == 1.5 / √3 */

        /* `hex_diameter` is short diameter, X in ISO/IEC 16023:2000 Figure 8 (same as W) */
        const float hex_diameter = 1.0f;
        const float hex_radius = hex_diameter / 2.0f;
        const float hex_ydiameter = two_div_sqrt3 * hex_diameter; /* Long diameter, V in Figure 8 */
        const float hex_yradius = hex_ydiameter / 2.0f;
        const float yposn_offset = sqrt3_div_two * hex_diameter; /* Vertical distance between rows, Y in Figure 8 */

        vector->width = 30 * hex_diameter + (xoffset + roffset);
        /* 32 rows drawn yposn_offset apart + final hexagon */
        vector->height = 32 * yposn_offset + hex_ydiameter + (yoffset + boffset);

        // Bullseye (ISO/IEC 16023:2000 4.2.1.1 and 4.11.4)
        bull_x = 14.5f * hex_diameter + xoffset; /* 14W right from leftmost centre = 14.5X */
        bull_y = vector->height / 2.0f; /* 16Y above bottom-most centre = halfway */
        /* Total finder diameter is 9X, so diametric increment for 5 diameters d2 to d6 is (9X - d1) / 5 */
        bull_d_incr = (hex_diameter * 9 - hex_ydiameter) / 5.0f;

        // TODO: Add width to circle so can draw rings instead of overlaying circles
        circle = vector_plot_create_circle(bull_x, bull_y, hex_ydiameter + bull_d_incr * 5, 0);
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(bull_x, bull_y, hex_ydiameter + bull_d_incr * 4, 1);
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(bull_x, bull_y, hex_ydiameter + bull_d_incr * 3, 0);
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(bull_x, bull_y, hex_ydiameter + bull_d_incr * 2, 1);
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(bull_x, bull_y, hex_ydiameter + bull_d_incr, 0);
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(bull_x, bull_y, hex_ydiameter, 1);
        vector_plot_add_circle(symbol, circle, &last_circle);

        /* Hexagons */
        for (r = 0; r < symbol->rows; r++) {
            const int odd_row = r & 1; /* Odd (reduced) row, even (full) row */
            const float yposn = r * yposn_offset + hex_yradius + yoffset;
            const float xposn_offset = (odd_row ? hex_diameter : hex_radius) + xoffset;
            for (i = 0; i < symbol->width - odd_row; i++) {
                if (module_is_set(symbol, r, i)) {
                    const float xposn = i * hex_diameter + xposn_offset;
                    struct zint_vector_hexagon *hexagon = vector_plot_create_hexagon(xposn, yposn, hex_diameter);
                    vector_plot_add_hexagon(symbol, hexagon, &last_hexagon);
                }
            }
        }
    // Dotty mode
    } else if (symbol->output_options & BARCODE_DOTTY_MODE) {
        for (r = 0; r < symbol->rows; r++) {
            for (i = 0; i < symbol->width; i++) {
                if (module_is_set(symbol, r, i)) {
                    struct zint_vector_circle *circle = vector_plot_create_circle(
                                                            i + dot_offset + xoffset,
                                                            r + dot_offset + yoffset,
                                                            symbol->dot_size, 0);
                    vector_plot_add_circle(symbol, circle, &last_circle);
                }
            }
        }
    // Plot rectangles - most symbols created here
    } else {
        rect_count = 0;
        row_posn = yoffset;
        for (r = 0; r < symbol->rows; r++) {
            this_row = r;
            last_row_start = rect_count;
            row_height = symbol->row_height[this_row] ? symbol->row_height[this_row] : large_bar_height;

            i = 0;

            if (symbol->symbology == BARCODE_ULTRA) {
                do {
                    int module_fill = module_colour_is_set(symbol, this_row, i);
                    int block_width = 0;
                    do {
                        block_width++;
                    } while (i + block_width < symbol->width
                            && module_colour_is_set(symbol, this_row, i + block_width) == module_fill);
                    if (module_fill) {
                        /* a colour block */
                        rectangle = vector_plot_create_rect(i + xoffset, row_posn, block_width, row_height);
                        rectangle->colour = module_colour_is_set(symbol, this_row, i);
                        vector_plot_add_rect(symbol, rectangle, &last_rectangle);
                        rect_count++;
                    }
                    i += block_width;

                } while (i < symbol->width);
            } else {
                do {
                    int module_fill = module_is_set(symbol, this_row, i);
                    int block_width = 0;
                    do {
                        block_width++;
                    } while (i + block_width < symbol->width
                            && module_is_set(symbol, this_row, i + block_width) == module_fill);
                    if (upceanflag && (addon_latch == 0) && (r == (symbol->rows - 1)) && (i > main_width)) {
                        addon_text_posn = row_posn + text_height - text_height * digit_ascent_factor;
                        if (addon_text_posn < 0.0f) {
                            addon_text_posn = 0.0f;
                        }
                        addon_bar_height = row_height - (addon_text_posn - row_posn) + text_gap;
                        if (upceanflag != 12 && upceanflag != 6) { /* UPC-A/E don't descend */
                            addon_bar_height += 5.0f;
                        }
                        if (addon_bar_height < 0.5f) {
                            addon_bar_height = 0.5f;
                        }
                        addon_latch = 1;
                    }
                    if (module_fill) {
                        /* a bar */
                        if (addon_latch == 0) {
                            rectangle = vector_plot_create_rect(i + xoffset, row_posn, block_width, row_height);
                        } else {
                            rectangle = vector_plot_create_rect(i + xoffset, addon_text_posn - text_gap, block_width,
                                                                addon_bar_height);
                        }
                        vector_plot_add_rect(symbol, rectangle, &last_rectangle);
                        rect_count++;
                    }
                    i += block_width;

                } while (i < symbol->width);
            }

            row_posn += row_height;
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
                        rect->height += 5.0f;
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
                        rect->height += 5.0f;
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
                        rect->height += 5.0f;
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
                        rect->height += 5.0f;
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
            float textwidth;

            output_upcean_split_text(upceanflag, symbol->text, textpart1, textpart2, textpart3, textpart4);

            if (upceanflag == 6) { /* UPC-E */
                textpos = -5.0f + xoffset;
                textwidth = 6.2f;
                vector_plot_add_string(symbol, textpart1, textpos, default_text_posn, upcae_outside_text_height,
                                        textwidth, 2 /*right align*/, &last_string);
                textpos = 24.0f + xoffset;
                textwidth = 6.0f * 8.5f;
                vector_plot_add_string(symbol, textpart2, textpos, default_text_posn, text_height, textwidth, 0,
                                        &last_string);
                textpos = 51.0f + 3.0f + xoffset;
                textwidth = 6.2f;
                vector_plot_add_string(symbol, textpart3, textpos, default_text_posn, upcae_outside_text_height,
                                        textwidth, 1 /*left align*/, &last_string);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 61.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, 0,
                                                &last_string);
                        break;
                    case 5:
                        textpos = 75.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, 0,
                                                &last_string);
                        break;
                }

            } else if (upceanflag == 8) { /* EAN-8 */
                textpos = 17.0f + xoffset;
                textwidth = 4.0f * 8.5f;
                vector_plot_add_string(symbol, textpart1, textpos, default_text_posn, text_height, textwidth, 0,
                                        &last_string);
                textpos = 50.0f + xoffset;
                vector_plot_add_string(symbol, textpart2, textpos, default_text_posn, text_height, textwidth, 0,
                                        &last_string);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 77.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, 0,
                                                &last_string);
                        break;
                    case 5:
                        textpos = 91.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, 0,
                                                &last_string);
                        break;
                }

            } else if (upceanflag == 12) { /* UPC-A */
                textpos = -5.0f + xoffset;
                textwidth = 6.2f;
                vector_plot_add_string(symbol, textpart1, textpos, default_text_posn, upcae_outside_text_height,
                                        textwidth, 2 /*right align*/, &last_string);
                textpos = 27.0f + xoffset;
                textwidth = 5.0f * 8.5f;
                vector_plot_add_string(symbol, textpart2, textpos, default_text_posn, text_height, textwidth, 0,
                                        &last_string);
                textpos = 67.0f + xoffset;
                vector_plot_add_string(symbol, textpart3, textpos, default_text_posn, text_height, textwidth, 0,
                                        &last_string);
                textpos = 95.0f + 5.0f + xoffset;
                textwidth = 6.2f;
                vector_plot_add_string(symbol, textpart4, textpos, default_text_posn, upcae_outside_text_height,
                                        textwidth, 1 /*left align*/, &last_string);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 105.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, 0,
                                                &last_string);
                        break;
                    case 5:
                        textpos = 119.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, 0,
                                                &last_string);
                        break;
                }

            } else if (upceanflag == 13) { /* EAN-13 */
                textpos = -5.0f + xoffset;
                textwidth = 8.5f;
                vector_plot_add_string(symbol, textpart1, textpos, default_text_posn, text_height, textwidth,
                                        2 /*right align*/, &last_string);
                textpos = 24.0f + xoffset;
                textwidth = 6.0f * 8.5f;
                vector_plot_add_string(symbol, textpart2, textpos, default_text_posn, text_height, textwidth, 0,
                                        &last_string);
                textpos = 71.0f + xoffset;
                vector_plot_add_string(symbol, textpart3, textpos, default_text_posn, text_height, textwidth, 0,
                                        &last_string);
                textdone = 1;
                switch (ustrlen(addon)) {
                    case 2:
                        textpos = 105.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, 0,
                                                &last_string);
                        break;
                    case 5:
                        textpos = 119.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        vector_plot_add_string(symbol, addon, textpos, addon_text_posn, text_height, textwidth, 0,
                                                &last_string);
                        break;
                }
            }
        }

        if (!textdone) {
            /* Put normal human readable text at the bottom (and centered) */
            // calculate start xoffset to center text
            vector_plot_add_string(symbol, symbol->text, main_width / 2.0f + xoffset, default_text_posn, text_height,
                                    symbol->width, 0, &last_string);
        }

        xoffset -= comp_offset; // Restore xoffset
    }

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
                    rectangle = vector_plot_create_rect(xoffset, (r * row_height) + yoffset - sep_height / 2,
                                                        symbol->width, sep_height);
                    vector_plot_add_rect(symbol, rectangle, &last_rectangle);
                }
            } else {
                for (r = 1; r < symbol->rows; r++) {
                    /* Avoid 11-module start and 13-module stop chars */
                    row_height = symbol->row_height[r - 1] ? symbol->row_height[r - 1] : large_bar_height;
                    rectangle = vector_plot_create_rect(xoffset + 11, (r * row_height) + yoffset - sep_height / 2,
                                                        symbol->width - 24, sep_height);
                    vector_plot_add_rect(symbol, rectangle, &last_rectangle);
                }
            }
        }
    }
    if (symbol->border_width > 0) {
        if (symbol->output_options & (BARCODE_BOX | BARCODE_BIND)) {
            float ybind_bottom = vector->height - symbol->border_width - textoffset - symbol->whitespace_height;
            // Top
            rectangle = vector_plot_create_rect(0.0f, symbol->whitespace_height, vector->width, symbol->border_width);
            if (!(symbol->output_options & BARCODE_BOX)
                    && (symbol->symbology == BARCODE_CODABLOCKF || symbol->symbology == BARCODE_HIBC_BLOCKF)) {
                /* CodaBlockF bind - does not extend over horizontal whitespace */
                rectangle->x = xoffset;
                rectangle->width -= (2.0f * xoffset);
            }
            vector_plot_add_rect(symbol, rectangle, &last_rectangle);
            // Bottom
            rectangle = vector_plot_create_rect(0.0f, ybind_bottom, vector->width, symbol->border_width);
            if (!(symbol->output_options & BARCODE_BOX)
                    && (symbol->symbology == BARCODE_CODABLOCKF || symbol->symbology == BARCODE_HIBC_BLOCKF)) {
                /* CodaBlockF bind - does not extend over horizontal whitespace */
                rectangle->x = xoffset;
                rectangle->width -= (2.0f * xoffset);
            }
            vector_plot_add_rect(symbol, rectangle, &last_rectangle);
        }
        if (symbol->output_options & BARCODE_BOX) {
            float xbox_right = vector->width - symbol->border_width;
            float box_height = vector->height - textoffset - (symbol->whitespace_height + symbol->border_width) * 2;
            // Left
            rectangle = vector_plot_create_rect(0.0f, yoffset, symbol->border_width, box_height);
            vector_plot_add_rect(symbol, rectangle, &last_rectangle);
            // Right
            rectangle = vector_plot_create_rect(xbox_right, yoffset, symbol->border_width, box_height);
            vector_plot_add_rect(symbol, rectangle, &last_rectangle);
        }
    }

    vector_reduce_rectangles(symbol);

    vector_scale(symbol, file_type);
    
    if (file_type != OUT_EMF_FILE) {
        /* EMF does its own rotation (with mixed results in various apps) */
        vector_rotate(symbol, rotate_angle);
    }

    switch (file_type) {
        case OUT_EPS_FILE:
            error_number = ps_plot(symbol);
            break;
        case OUT_SVG_FILE:
            error_number = svg_plot(symbol);
            break;
        case OUT_EMF_FILE:
            error_number = emf_plot(symbol, rotate_angle);
            break;
        /* case OUT_BUFFER: No more work needed */
    }

    return error_number;
}
