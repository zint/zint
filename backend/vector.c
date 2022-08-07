/*  vector.c - Creates vector image objects */
/*
    libzint - the open source barcode library
    Copyright (C) 2018-2022 Robin Stuart <rstuart114@gmail.com>

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

#include "common.h"
#include "output.h"
#include "zfiletypes.h"

INTERNAL int ps_plot(struct zint_symbol *symbol);
INTERNAL int svg_plot(struct zint_symbol *symbol);
INTERNAL int emf_plot(struct zint_symbol *symbol, int rotate_angle);

static struct zint_vector_rect *vector_plot_create_rect(struct zint_symbol *symbol,
                                const float x, const float y, const float width, const float height) {
    struct zint_vector_rect *rect;

    rect = (struct zint_vector_rect *) malloc(sizeof(struct zint_vector_rect));
    if (!rect) {
        strcpy(symbol->errtxt, "691: Insufficient memory for vector rectangle");
        return NULL;
    }

    rect->next = NULL;
    rect->x = x;
    rect->y = y;
    rect->width = width;
    rect->height = height;
    rect->colour = -1; /* Default colour */

    return rect;
}

static void vector_plot_add_rect(struct zint_symbol *symbol, struct zint_vector_rect *rect,
            struct zint_vector_rect **last_rect) {
    if (*last_rect)
        (*last_rect)->next = rect;
    else
        symbol->vector->rectangles = rect; /* first rectangle */

    *last_rect = rect;
}

static struct zint_vector_hexagon *vector_plot_create_hexagon(struct zint_symbol *symbol,
                                    const float x, const float y, const float diameter) {
    struct zint_vector_hexagon *hexagon;

    hexagon = (struct zint_vector_hexagon *) malloc(sizeof(struct zint_vector_hexagon));
    if (!hexagon) {
        strcpy(symbol->errtxt, "692: Insufficient memory for vector hexagon");
        return NULL;
    }
    hexagon->next = NULL;
    hexagon->x = x;
    hexagon->y = y;
    hexagon->diameter = diameter;
    hexagon->rotation = 0;

    return hexagon;
}

static void vector_plot_add_hexagon(struct zint_symbol *symbol, struct zint_vector_hexagon *hexagon,
            struct zint_vector_hexagon **last_hexagon) {
    if (*last_hexagon)
        (*last_hexagon)->next = hexagon;
    else
        symbol->vector->hexagons = hexagon; /* first hexagon */

    *last_hexagon = hexagon;
}

static struct zint_vector_circle *vector_plot_create_circle(struct zint_symbol *symbol,
                                    const float x, const float y, const float diameter, const float width,
                                    const int colour) {
    struct zint_vector_circle *circle;

    circle = (struct zint_vector_circle *) malloc(sizeof(struct zint_vector_circle));
    if (!circle) {
        strcpy(symbol->errtxt, "693: Insufficient memory for vector circle");
        return NULL;
    }
    circle->next = NULL;
    circle->x = x;
    circle->y = y;
    circle->diameter = diameter;
    circle->width = width;
    circle->colour = colour;

    return circle;
}

static void vector_plot_add_circle(struct zint_symbol *symbol, struct zint_vector_circle *circle,
            struct zint_vector_circle **last_circle) {
    if (*last_circle)
        (*last_circle)->next = circle;
    else
        symbol->vector->circles = circle; /* first circle */

    *last_circle = circle;
}

static int vector_plot_add_string(struct zint_symbol *symbol, const unsigned char *text,
            const float x, const float y, const float fsize, const float width, const int halign,
            struct zint_vector_string **last_string) {
    struct zint_vector_string *string;

    string = (struct zint_vector_string *) malloc(sizeof(struct zint_vector_string));
    if (!string) {
        strcpy(symbol->errtxt, "694: Insufficient memory for vector string");
        return 0;
    }
    string->next = NULL;
    string->x = x;
    string->y = y;
    string->width = width;
    string->fsize = fsize;
    string->length = (int) ustrlen(text);
    string->rotation = 0;
    string->halign = halign;
    string->text = (unsigned char *) malloc(string->length + 1);
    if (!string->text) {
        free(string);
        strcpy(symbol->errtxt, "695: Insufficient memory for vector string text");
        return 0;
    }
    ustrcpy(string->text, text);

    if (*last_string)
        (*last_string)->next = string;
    else
        symbol->vector->strings = string; /* First text portion */
    *last_string = string;

    return 1;
}

INTERNAL void vector_free(struct zint_symbol *symbol) {
    if (symbol->vector != NULL) {
        struct zint_vector_rect *rect;
        struct zint_vector_hexagon *hex;
        struct zint_vector_circle *circle;
        struct zint_vector_string *string;

        /* Free Rectangles */
        rect = symbol->vector->rectangles;
        while (rect) {
            struct zint_vector_rect *r = rect;
            rect = rect->next;
            free(r);
        }

        /* Free Hexagons */
        hex = symbol->vector->hexagons;
        while (hex) {
            struct zint_vector_hexagon *h = hex;
            hex = hex->next;
            free(h);
        }

        /* Free Circles */
        circle = symbol->vector->circles;
        while (circle) {
            struct zint_vector_circle *c = circle;
            circle = circle->next;
            free(c);
        }

        /* Free Strings */
        string = symbol->vector->strings;
        while (string) {
            struct zint_vector_string *s = string;
            string = string->next;
            free(s->text);
            free(s);
        }

        /* Free vector */
        free(symbol->vector);
        symbol->vector = NULL;
    }
}

static void vector_scale(struct zint_symbol *symbol, const int file_type) {
    struct zint_vector_rect *rect;
    struct zint_vector_hexagon *hex;
    struct zint_vector_circle *circle;
    struct zint_vector_string *string;
    float scale = symbol->scale * 2.0f;

    if (scale < 0.2f) { /* Minimum vector scale 0.1 */
        scale = 0.2f;
    }

    if ((file_type == OUT_EMF_FILE) && (symbol->symbology == BARCODE_MAXICODE)) {
        /* Increase size to overcome limitations in EMF file format */
        scale *= 20;
    }

    symbol->vector->width = stripf(symbol->vector->width * scale);
    symbol->vector->height = stripf(symbol->vector->height * scale);

    rect = symbol->vector->rectangles;
    while (rect) {
        rect->x = stripf(rect->x * scale);
        rect->y = stripf(rect->y * scale);
        rect->height = stripf(rect->height * scale);
        rect->width = stripf(rect->width * scale);
        rect = rect->next;
    }

    hex = symbol->vector->hexagons;
    while (hex) {
        hex->x = stripf(hex->x * scale);
        hex->y = stripf(hex->y * scale);
        hex->diameter = stripf(hex->diameter * scale);
        hex = hex->next;
    }

    circle = symbol->vector->circles;
    while (circle) {
        circle->x = stripf(circle->x * scale);
        circle->y = stripf(circle->y * scale);
        circle->diameter = stripf(circle->diameter * scale);
        circle->width = stripf(circle->width * scale);
        circle = circle->next;
    }

    string = symbol->vector->strings;
    while (string) {
        string->x = stripf(string->x * scale);
        string->y = stripf(string->y * scale);
        string->width = stripf(string->width * scale);
        string->fsize = stripf(string->fsize * scale);
        string = string->next;
    }
}

static void vector_rotate(struct zint_symbol *symbol, const int rotate_angle) {
    /* Rotates the image */
    struct zint_vector_rect *rect;
    struct zint_vector_hexagon *hex;
    struct zint_vector_circle *circle;
    struct zint_vector_string *string;
    float temp;

    if (rotate_angle == 0) {
        /* No rotation needed */
        return;
    }

    rect = symbol->vector->rectangles;
    while (rect) {
        if (rotate_angle == 90) {
            temp = rect->x;
            rect->x = stripf(symbol->vector->height - (rect->y + rect->height));
            rect->y = temp;
            temp = rect->width;
            rect->width = rect->height;
            rect->height = temp;
        } else if (rotate_angle == 180) {
            rect->x = stripf(symbol->vector->width - (rect->x + rect->width));
            rect->y = stripf(symbol->vector->height - (rect->y + rect->height));
        } else if (rotate_angle == 270) {
            temp = rect->x;
            rect->x = rect->y;
            rect->y = stripf(symbol->vector->width - (temp + rect->width));
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
            hex->x = stripf(symbol->vector->height - hex->y);
            hex->y = temp;
            hex->rotation = 90;
        } else if (rotate_angle == 180) {
            hex->x = stripf(symbol->vector->width - hex->x);
            hex->y = stripf(symbol->vector->height - hex->y);
            hex->rotation = 180;
        } else if (rotate_angle == 270) {
            temp = hex->x;
            hex->x = hex->y;
            hex->y = stripf(symbol->vector->width - temp);
            hex->rotation = 270;
        }
        hex = hex->next;
    }

    circle = symbol->vector->circles;
    while (circle) {
        if (rotate_angle == 90) {
            temp = circle->x;
            circle->x = stripf(symbol->vector->height - circle->y);
            circle->y = temp;
        } else if (rotate_angle == 180) {
            circle->x = stripf(symbol->vector->width - circle->x);
            circle->y = stripf(symbol->vector->height - circle->y);
        } else if (rotate_angle == 270) {
            temp = circle->x;
            circle->x = circle->y;
            circle->y = stripf(symbol->vector->width - temp);
        }
        circle = circle->next;
    }

    string = symbol->vector->strings;
    while (string) {
        if (rotate_angle == 90) {
            temp = string->x;
            string->x = stripf(symbol->vector->height - string->y);
            string->y = temp;
            string->rotation = 90;
        } else if (rotate_angle == 180) {
            string->x = stripf(symbol->vector->width - string->x);
            string->y = stripf(symbol->vector->height - string->y);
            string->rotation = 180;
        } else if (rotate_angle == 270) {
            temp = string->x;
            string->x = string->y;
            string->y = stripf(symbol->vector->width - temp);
            string->rotation = 270;
        }
        string = string->next;
    }

    if ((rotate_angle == 90) || (rotate_angle == 270)) {
        temp = symbol->vector->height;
        symbol->vector->height = symbol->vector->width;
        symbol->vector->width = temp;
    }
}

static void vector_reduce_rectangles(struct zint_symbol *symbol) {
    /* Looks for vertically aligned rectangles and merges them together */
    struct zint_vector_rect *rect, *target, *prev;

    rect = symbol->vector->rectangles;
    while (rect) {
        prev = rect;
        target = prev->next;

        while (target) {
            if ((rect->x == target->x) && (rect->width == target->width)
                    && (stripf(rect->y + rect->height) == target->y) && (rect->colour == target->colour)) {
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
}

INTERNAL int plot_vector(struct zint_symbol *symbol, int rotate_angle, int file_type) {
    int error_number;
    int main_width;
    int comp_xoffset = 0;
    unsigned char addon[6];
    int addon_gap = 0;
    float addon_text_yposn = 0.0f;
    float xoffset, yoffset, roffset, boffset;
    float textoffset;
    int upceanflag = 0;
    int addon_latch = 0;
    unsigned char textpart1[5], textpart2[7], textpart3[7], textpart4[2];
    int hide_text;
    int i, r;
    int block_width = 0;
    int text_height; /* Font pixel size (so whole integers) */
    float text_gap; /* Gap between barcode and text */
    float guard_descent;
    const int is_codablockf = symbol->symbology == BARCODE_CODABLOCKF || symbol->symbology == BARCODE_HIBC_BLOCKF;

    float addon_row_height;
    float large_bar_height;
    int upcae_outside_text_height = 0; /* UPC-A/E outside digits font size */
    float digit_ascent_factor = 0.25f; /* Assuming digit ascent roughly 25% less than font size */
    float dot_overspill = 0.0f;
    float dot_offset = 0.0f;
    int rect_count = 0, last_row_start = 0; /* For UPC/EAN guard bars */
    float yposn;

    struct zint_vector *vector;
    struct zint_vector_rect *rect, *last_rectangle = NULL;
    struct zint_vector_hexagon *hexagon, *last_hexagon = NULL;
    struct zint_vector_string *last_string = NULL;
    struct zint_vector_circle *circle, *last_circle = NULL;
    struct zint_vector_rect **first_row_rects = z_alloca(sizeof(struct zint_vector_rect *) * (symbol->rows + 1));

    memset(first_row_rects, 0, sizeof(struct zint_vector_rect *) * (symbol->rows + 1));

    /* Free any previous rendering structures */
    vector_free(symbol);

    /* Sanity check colours */
    error_number = out_check_colour_options(symbol);
    if (error_number != 0) {
        return error_number;
    }

    /* Allocate memory */
    vector = symbol->vector = (struct zint_vector *) malloc(sizeof(struct zint_vector));
    if (!vector) {
        strcpy(symbol->errtxt, "696: Insufficient memory for vector header");
        return ZINT_ERROR_MEMORY;
    }
    vector->rectangles = NULL;
    vector->hexagons = NULL;
    vector->circles = NULL;
    vector->strings = NULL;

    large_bar_height = out_large_bar_height(symbol, 0 /*si (scale and round)*/, NULL /*row_heights_si*/,
                        NULL /*symbol_height_si*/);

    main_width = symbol->width;

    if (is_extendable(symbol->symbology)) {
        upceanflag = out_process_upcean(symbol, &main_width, &comp_xoffset, addon, &addon_gap);
    }

    hide_text = ((!symbol->show_hrt) || (ustrlen(symbol->text) == 0));

    out_set_whitespace_offsets(symbol, hide_text, &xoffset, &yoffset, &roffset, &boffset, 0 /*scaler*/,
        NULL, NULL, NULL, NULL);

    /* Note font sizes scaled by 2 so really twice these values */
    if (upceanflag) {
        /* Note BOLD_TEXT ignored for UPCEAN by svg/emf/ps/qzint */
        text_height = symbol->output_options & SMALL_TEXT ? 7 : 10;
        upcae_outside_text_height = symbol->output_options & SMALL_TEXT ? 6 : 7;
        /* Negative to move close to barcode (less digit ascent, then add 0.5X) */
        text_gap = -text_height * digit_ascent_factor + 0.5f;
        /* Guard bar height (none for EAN-2 and EAN-5) */
        guard_descent = upceanflag != 2 && upceanflag != 5 ? symbol->guard_descent : 0.0f;
    } else {
        text_height = symbol->output_options & SMALL_TEXT ? 6 : 7;
        text_gap = text_height * 0.1f;
        guard_descent = 0.0f;
    }

    if (hide_text) {
        textoffset = guard_descent;
    } else {
        if (upceanflag) {
            /* Add fudge for anti-aliasing of digits */
            if (text_height + 0.2f + text_gap > guard_descent) {
                textoffset = text_height + 0.2f + text_gap;
            } else {
                textoffset = guard_descent;
            }
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

    /* Plot Maxicode symbols */
    if (symbol->symbology == BARCODE_MAXICODE) {
        float bull_x, bull_y, bull_d_incr, bull_width;
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

        /* Bullseye (ISO/IEC 16023:2000 4.2.1.1 and 4.11.4) */
        bull_x = 14.5f * hex_diameter + xoffset; /* 14W right from leftmost centre = 14.5X */
        bull_y = vector->height / 2.0f; /* 16Y above bottom-most centre = halfway */
        /* Total finder diameter is 9X, so diametric increment for 5 diameters d2 to d6 is (9X - d1) / 5 */
        bull_d_incr = (hex_diameter * 9 - hex_ydiameter) / 5.0f;
        bull_width = bull_d_incr / 2.0f;

        circle = vector_plot_create_circle(symbol, bull_x, bull_y,
                    hex_ydiameter + bull_d_incr * 5 - bull_width, bull_width, 0);
        if (!circle) return ZINT_ERROR_MEMORY;
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(symbol, bull_x, bull_y,
                    hex_ydiameter + bull_d_incr * 3 - bull_width, bull_width, 0);
        if (!circle) return ZINT_ERROR_MEMORY;
        vector_plot_add_circle(symbol, circle, &last_circle);
        circle = vector_plot_create_circle(symbol, bull_x, bull_y,
                    hex_ydiameter + bull_d_incr - bull_width, bull_width, 0);
        if (!circle) return ZINT_ERROR_MEMORY;
        vector_plot_add_circle(symbol, circle, &last_circle);

        /* Hexagons */
        for (r = 0; r < symbol->rows; r++) {
            const int odd_row = r & 1; /* Odd (reduced) row, even (full) row */
            const float hex_yposn = r * yposn_offset + hex_yradius + yoffset;
            const float xposn_offset = (odd_row ? hex_diameter : hex_radius) + xoffset;
            for (i = 0; i < symbol->width - odd_row; i++) {
                if (module_is_set(symbol, r, i)) {
                    const float hex_xposn = i * hex_diameter + xposn_offset;
                    hexagon = vector_plot_create_hexagon(symbol, hex_xposn, hex_yposn, hex_diameter);
                    if (!hexagon) return ZINT_ERROR_MEMORY;
                    vector_plot_add_hexagon(symbol, hexagon, &last_hexagon);
                }
            }
        }
    /* Dotty mode */
    } else if (symbol->output_options & BARCODE_DOTTY_MODE) {
        for (r = 0; r < symbol->rows; r++) {
            for (i = 0; i < symbol->width; i++) {
                if (module_is_set(symbol, r, i)) {
                    circle = vector_plot_create_circle(symbol, i + dot_offset + xoffset, r + dot_offset + yoffset,
                                symbol->dot_size, 0, 0);
                    if (!circle) return ZINT_ERROR_MEMORY;
                    vector_plot_add_circle(symbol, circle, &last_circle);
                }
            }
        }
    /* Plot rectangles - most symbols created here */
    } else if (symbol->symbology == BARCODE_ULTRA) {
        yposn = yoffset;
        for (r = 0; r < symbol->rows; r++) {
            const float row_height = symbol->row_height[r];

            for (i = 0; i < symbol->width; i += block_width) {
                const int fill = module_colour_is_set(symbol, r, i);
                for (block_width = 1; (i + block_width < symbol->width)
                                        && module_colour_is_set(symbol, r, i + block_width) == fill; block_width++);
                if (fill) {
                    /* a colour block */
                    rect = vector_plot_create_rect(symbol, i + xoffset, yposn, block_width, row_height);
                    if (!rect) return ZINT_ERROR_MEMORY;
                    rect->colour = module_colour_is_set(symbol, r, i);
                    vector_plot_add_rect(symbol, rect, &last_rectangle);
                }
            }
            yposn += row_height;
        }

    } else if (upceanflag >= 6) { /* UPC-E, EAN-8, UPC-A, EAN-13 */
        yposn = yoffset;
        for (r = 0; r < symbol->rows; r++) {
            const float row_height = symbol->row_height[r] ? symbol->row_height[r] : large_bar_height;
            last_row_start = rect_count;

            for (i = 0; i < symbol->width; i += block_width) {
                const int fill = module_is_set(symbol, r, i);
                for (block_width = 1; (i + block_width < symbol->width)
                                        && module_is_set(symbol, r, i + block_width) == fill; block_width++);

                if ((r == (symbol->rows - 1)) && (i > main_width) && (addon_latch == 0)) {
                    addon_text_yposn = yposn + text_height - text_height * digit_ascent_factor;
                    if (addon_text_yposn < 0.0f) {
                        addon_text_yposn = 0.0f;
                    }
                    addon_row_height = row_height - (addon_text_yposn - yposn) + text_gap;
                    if (upceanflag != 12 && upceanflag != 6) { /* UPC-A/E add-ons don't descend */
                        addon_row_height += guard_descent;
                    }
                    if (addon_row_height < 0.5f) {
                        addon_row_height = 0.5f;
                    }
                    addon_latch = 1;
                }
                if (fill) {
                    /* a bar */
                    if (addon_latch) {
                        rect = vector_plot_create_rect(symbol, i + xoffset, addon_text_yposn - text_gap,
                                                        block_width, addon_row_height);
                    } else {
                        rect = vector_plot_create_rect(symbol, i + xoffset, yposn, block_width, row_height);
                    }
                    if (!rect) return ZINT_ERROR_MEMORY;
                    vector_plot_add_rect(symbol, rect, &last_rectangle);
                    rect_count++;
                }
            }
            yposn += row_height;
        }

    } else {
        yposn = yoffset;
        for (r = 0; r < symbol->rows; r++) {
            const float row_height = symbol->row_height[r] ? symbol->row_height[r] : large_bar_height;

            for (i = 0; i < symbol->width; i += block_width) {
                const int fill = module_is_set(symbol, r, i);
                for (block_width = 1; (i + block_width < symbol->width)
                                        && module_is_set(symbol, r, i + block_width) == fill; block_width++);
                if (fill) {
                    /* a bar */
                    rect = vector_plot_create_rect(symbol, i + xoffset, yposn, block_width, row_height);
                    if (!rect) return ZINT_ERROR_MEMORY;
                    vector_plot_add_rect(symbol, rect, &last_rectangle);
                    if (i == 0) {
                        first_row_rects[r] = rect;
                    }
                }
            }
            yposn += row_height;
        }
    }

    if (guard_descent && upceanflag >= 6) { /* UPC-E, EAN-8, UPC-A, EAN-13 */
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
                        rect->height += guard_descent;
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
                        rect->height += guard_descent;
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
                        rect->height += guard_descent;
                        break;
                }
                i++;
            }
        } else { /* EAN-13 */
            i = 0;
            for (rect = symbol->vector->rectangles; rect != NULL; rect = rect->next) {
                switch (i - last_row_start) {
                    case 0:
                    case 1:
                    case 14:
                    case 15:
                    case 28:
                    case 29:
                        rect->height += guard_descent;
                        break;
                }
                i++;
            }
        }
    }

    /* Add the text */

    if (!hide_text) {
        float text_xposn;
        float text_yposn;

        xoffset += comp_xoffset;

        text_yposn = yoffset + symbol->height + text_height + text_gap; /* Calculated to bottom of text */
        if (symbol->border_width > 0 && (symbol->output_options & (BARCODE_BOX | BARCODE_BIND))) {
            text_yposn += symbol->border_width;
        }

        if (upceanflag >= 6) { /* UPC-E, EAN-8, UPC-A, EAN-13 */
            float textwidth;

            out_upcean_split_text(upceanflag, symbol->text, textpart1, textpart2, textpart3, textpart4);

            if (upceanflag == 6) { /* UPC-E */
                text_xposn = -5.0f + xoffset;
                textwidth = 6.2f;
                if (!vector_plot_add_string(symbol, textpart1, text_xposn, text_yposn, upcae_outside_text_height,
                        textwidth, 2 /*right align*/, &last_string)) return ZINT_ERROR_MEMORY;
                text_xposn = 24.0f + xoffset;
                textwidth = 6.0f * 8.5f;
                if (!vector_plot_add_string(symbol, textpart2, text_xposn, text_yposn, text_height,
                        textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                text_xposn = 51.0f + 3.0f + xoffset;
                textwidth = 6.2f;
                if (!vector_plot_add_string(symbol, textpart3, text_xposn, text_yposn, upcae_outside_text_height,
                        textwidth, 1 /*left align*/, &last_string)) return ZINT_ERROR_MEMORY;
                switch (ustrlen(addon)) {
                    case 2:
                        text_xposn = 61.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;
                        if (!vector_plot_add_string(symbol, addon, text_xposn, addon_text_yposn,
                                text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                        break;
                    case 5:
                        text_xposn = 75.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        if (!vector_plot_add_string(symbol, addon, text_xposn, addon_text_yposn,
                                text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                        break;
                }

            } else if (upceanflag == 8) { /* EAN-8 */
                text_xposn = 17.0f + xoffset;
                textwidth = 4.0f * 8.5f;
                if (!vector_plot_add_string(symbol, textpart1, text_xposn, text_yposn,
                        text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                text_xposn = 50.0f + xoffset;
                if (!vector_plot_add_string(symbol, textpart2, text_xposn, text_yposn,
                        text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                switch (ustrlen(addon)) {
                    case 2:
                        text_xposn = 77.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;
                        if (!vector_plot_add_string(symbol, addon, text_xposn, addon_text_yposn,
                                text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                        break;
                    case 5:
                        text_xposn = 91.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        if (!vector_plot_add_string(symbol, addon, text_xposn, addon_text_yposn,
                                text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                        break;
                }

            } else if (upceanflag == 12) { /* UPC-A */
                text_xposn = -5.0f + xoffset;
                textwidth = 6.2f;
                if (!vector_plot_add_string(symbol, textpart1, text_xposn, text_yposn, upcae_outside_text_height,
                        textwidth, 2 /*right align*/, &last_string)) return ZINT_ERROR_MEMORY;
                text_xposn = 27.0f + xoffset;
                textwidth = 5.0f * 8.5f;
                if (!vector_plot_add_string(symbol, textpart2, text_xposn, text_yposn, text_height,
                        textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                text_xposn = 67.0f + xoffset;
                if (!vector_plot_add_string(symbol, textpart3, text_xposn, text_yposn, text_height,
                        textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                text_xposn = 95.0f + 5.0f + xoffset;
                textwidth = 6.2f;
                if (!vector_plot_add_string(symbol, textpart4, text_xposn, text_yposn, upcae_outside_text_height,
                        textwidth, 1 /*left align*/, &last_string)) return ZINT_ERROR_MEMORY;
                switch (ustrlen(addon)) {
                    case 2:
                        text_xposn = 105.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;
                        if (!vector_plot_add_string(symbol, addon, text_xposn, addon_text_yposn,
                                text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                        break;
                    case 5:
                        text_xposn = 119.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        if (!vector_plot_add_string(symbol, addon, text_xposn, addon_text_yposn,
                                text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                        break;
                }

            } else { /* EAN-13 */
                text_xposn = -5.0f + xoffset;
                textwidth = 8.5f;
                if (!vector_plot_add_string(symbol, textpart1, text_xposn, text_yposn,
                        text_height, textwidth, 2 /*right align*/, &last_string)) return ZINT_ERROR_MEMORY;
                text_xposn = 24.0f + xoffset;
                textwidth = 6.0f * 8.5f;
                if (!vector_plot_add_string(symbol, textpart2, text_xposn, text_yposn,
                        text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                text_xposn = 71.0f + xoffset;
                if (!vector_plot_add_string(symbol, textpart3, text_xposn, text_yposn,
                        text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                switch (ustrlen(addon)) {
                    case 2:
                        text_xposn = 105.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;
                        if (!vector_plot_add_string(symbol, addon, text_xposn, addon_text_yposn,
                                text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                        break;
                    case 5:
                        text_xposn = 119.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        if (!vector_plot_add_string(symbol, addon, text_xposn, addon_text_yposn,
                                text_height, textwidth, 0, &last_string)) return ZINT_ERROR_MEMORY;
                        break;
                }
            }
        } else {
            /* Put normal human readable text at the bottom (and centered) */
            /* calculate start xoffset to center text */
            text_xposn = main_width / 2.0f + xoffset;
            if (!vector_plot_add_string(symbol, symbol->text, text_xposn, text_yposn,
                    text_height, symbol->width, 0, &last_string)) return ZINT_ERROR_MEMORY;
        }

        xoffset -= comp_xoffset; /* Restore xoffset */
    }

    /* Separator binding for stacked barcodes */
    if ((symbol->output_options & BARCODE_BIND) && (symbol->rows > 1) && is_stackable(symbol->symbology)) {
        float sep_xoffset = xoffset;
        float sep_width = symbol->width;
        float sep_height = 1.0f, sep_yoffset, sep_half_height;
        if (symbol->option_3 > 0 && symbol->option_3 <= 4) {
            sep_height = symbol->option_3;
        }
        sep_half_height = sep_height / 2.0f;
        sep_yoffset = yoffset - sep_half_height;
        if (is_codablockf) {
            /* Avoid 11-module start and 13-module stop chars */
            sep_xoffset += 11;
            sep_width -= 11 + 13;
        }
        /* Adjust original rectangles so don't overlap with separator(s) (important for RGBA) */
        for (r = 0; r < symbol->rows; r++) {
            for (rect = first_row_rects[r], i = 0; rect && rect != first_row_rects[r + 1]; rect = rect->next, i++) {
                if (is_codablockf) { /* Skip start and stop chars */
                    if (i < 3) {
                        continue;
                    }
                    if ((i / 3) * 11 + 13 >= symbol->width) { /* 3 bars and 11 modules per char */
                        break;
                    }
                }
                if (r != 0) {
                    rect->y += sep_height - sep_half_height;
                    rect->height -= r + 1 == symbol->rows ? sep_half_height : sep_height;
                } else {
                    rect->height -= sep_half_height;
                }
                if (rect->height < 0) {
                    rect->height = 0.0f;
                    /* TODO: warn? */
                }
            }
        }
        for (r = 1; r < symbol->rows; r++) {
            const float row_height = symbol->row_height[r - 1] ? symbol->row_height[r - 1] : large_bar_height;
            rect = vector_plot_create_rect(symbol, sep_xoffset, (r * row_height) + sep_yoffset,
                                            sep_width, sep_height);
            if (!rect) return ZINT_ERROR_MEMORY;
            vector_plot_add_rect(symbol, rect, &last_rectangle);
        }
    }

    /* Bind/box */
    if (symbol->border_width > 0 && (symbol->output_options & (BARCODE_BOX | BARCODE_BIND))) {
        const int horz_outside = is_fixed_ratio(symbol->symbology);
        float ybind_top = yoffset - symbol->border_width;
        /* Following equivalent to yoffset + symbol->height + dot_overspill except for BARCODE_MAXICODE */
        float ybind_bot = vector->height - textoffset - boffset;
        if (horz_outside) {
            ybind_top = 0;
            ybind_bot = vector->height - symbol->border_width;
        }
        /* Top */
        rect = vector_plot_create_rect(symbol, 0.0f, ybind_top, vector->width, symbol->border_width);
        if (!rect) return ZINT_ERROR_MEMORY;
        if (!(symbol->output_options & BARCODE_BOX) && is_codablockf) {
            /* CodaBlockF bind - does not extend over horizontal whitespace */
            rect->x = xoffset;
            rect->width -= xoffset + roffset;
        }
        vector_plot_add_rect(symbol, rect, &last_rectangle);
        /* Bottom */
        rect = vector_plot_create_rect(symbol, 0.0f, ybind_bot, vector->width, symbol->border_width);
        if (!rect) return ZINT_ERROR_MEMORY;
        if (!(symbol->output_options & BARCODE_BOX) && is_codablockf) {
            /* CodaBlockF bind - does not extend over horizontal whitespace */
            rect->x = xoffset;
            rect->width -= xoffset + roffset;
        }
        vector_plot_add_rect(symbol, rect, &last_rectangle);
        if (symbol->output_options & BARCODE_BOX) {
            const float xbox_right = vector->width - symbol->border_width;
            float box_top = yoffset;
            /* Following equivalent to symbol->height except for BARCODE_MAXICODE */
            float box_height = vector->height - textoffset - dot_overspill - yoffset - boffset;
            if (horz_outside) {
                box_top = symbol->border_width;
                box_height = vector->height - symbol->border_width * 2;
            }
            /* Left */
            rect = vector_plot_create_rect(symbol, 0.0f, box_top, symbol->border_width, box_height);
            if (!rect) return ZINT_ERROR_MEMORY;
            vector_plot_add_rect(symbol, rect, &last_rectangle);
            /* Right */
            rect = vector_plot_create_rect(symbol, xbox_right, box_top, symbol->border_width, box_height);
            if (!rect) return ZINT_ERROR_MEMORY;
            vector_plot_add_rect(symbol, rect, &last_rectangle);
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

/* vim: set ts=4 sw=4 et : */
