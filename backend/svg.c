/* svg.c - Scalable Vector Graphics */
/*
    libzint - the open source barcode library
    Copyright (C) 2009-2025 Robin Stuart <rstuart114@gmail.com>

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

#include "common.h"
#include "filemem.h"
#include "output.h"
#include "fonts/normal_woff2.h"
#include "fonts/upcean_woff2.h"

/* Convert Ultracode rectangle colour to RGB */
static void svg_pick_colour(const int colour, char colour_code[7]) {
    const int idx = colour >= 1 && colour <= 8 ? colour - 1 : 6 /*black*/;
    static const char rgbs[8][7] = {
        "00ffff", /* 0: Cyan (1) */
        "0000ff", /* 1: Blue (2) */
        "ff00ff", /* 2: Magenta (3) */
        "ff0000", /* 3: Red (4) */
        "ffff00", /* 4: Yellow (5) */
        "00ff00", /* 5: Green (6) */
        "000000", /* 6: Black (7) */
        "ffffff", /* 7: White (8) */
    };
    memcpy(colour_code, rgbs[idx], 7); /* Include terminating NUL */
}

/* Convert text to use HTML entity codes */
static void svg_make_html_friendly(const unsigned char *string, char *html_version) {

    /* `NOLINT` required due to disconnect between `symbol->text` and vector `string`s which are always <= */
    /* NOLINTBEGIN(clang-analyzer-security.ArrayBound) clang-tidy-21 false positive */
    for (; *string; string++) {
        switch (*string) {
            case '>':
                memcpy(html_version, "&gt;", 4);
                html_version += 4;
                break;

            case '<':
                memcpy(html_version, "&lt;", 4);
                html_version += 4;
                break;

            case '&':
                memcpy(html_version, "&amp;", 5);
                html_version += 5;
                break;

            case '"':
                memcpy(html_version, "&quot;", 6);
                html_version += 6;
                break;

            case '\'':
                memcpy(html_version, "&apos;", 6);
                html_version += 6;
                break;

            default:
                *html_version++ = *string;
                break;
         }
    }

    *html_version = '\0';
    /* NOLINTEND(clang-analyzer-security.ArrayBound) */
}

/* Helper to output floating point attribute */
static void svg_put_fattrib(const char *prefix, const int dp, const float val, struct filemem *fmp) {
    zint_fm_putsf(prefix, dp, val, fmp);
    zint_fm_putc('"', fmp);
}

/* Helper to output opacity attribute attribute and close tag (maybe) */
static void svg_put_opacity_close(const unsigned char alpha, const float val, const int close, struct filemem *fmp) {
    if (alpha != 0xFF) {
        svg_put_fattrib(" opacity=\"", 3, val, fmp);
    }
    if (close) {
        zint_fm_putc('/', fmp);
    }
    zint_fm_puts(">\n", fmp);
}

INTERNAL int zint_svg_plot(struct zint_symbol *symbol) {
    static const char normal_font_family[] = "Arimo";
    static const char upcean_font_family[] = "OCRB";
    struct filemem fm;
    struct filemem *const fmp = &fm;
    float previous_diameter;
    float radius, half_radius, half_sqrt3_radius;
    int i;
    char fgcolour_string[7];
    char bgcolour_string[7];
    unsigned char fgred, fggreen, fgblue, fg_alpha;
    unsigned char bgred, bggreen, bgblue, bg_alpha;
    float fg_alpha_opacity = 0.0f, bg_alpha_opacity = 0.0f; /* Suppress `-Wmaybe-uninitialized` */
    int bold;

    struct zint_vector_rect *rect;
    struct zint_vector_hexagon *hex;
    struct zint_vector_circle *circle;
    struct zint_vector_string *string;

    char colour_code[7];
    int html_len;

    const int is_upcean = z_is_upcean(symbol->symbology);
    char *html_string;

    (void) zint_out_colour_get_rgb(symbol->fgcolour, &fgred, &fggreen, &fgblue, &fg_alpha);
    if (fg_alpha != 0xFF) {
        fg_alpha_opacity = fg_alpha / 255.0f;
    }
    sprintf(fgcolour_string, "%02X%02X%02X", fgred, fggreen, fgblue);
    (void) zint_out_colour_get_rgb(symbol->bgcolour, &bgred, &bggreen, &bgblue, &bg_alpha);
    if (bg_alpha != 0xFF) {
        bg_alpha_opacity = bg_alpha / 255.0f;
    }
    sprintf(bgcolour_string, "%02X%02X%02X", bgred, bggreen, bgblue);

    html_len = symbol->text_length + 1;

    for (i = 0; i < symbol->text_length; i++) {
        switch (symbol->text[i]) {
            case '>':
            case '<':
            case '"':
            case '&':
            case '\'':
                html_len += 6;
                break;
        }
    }
    if (symbol->output_options & EANUPC_GUARD_WHITESPACE) {
        html_len += 12; /* Allow for "<" & ">" */
    }

    html_string = (char *) z_alloca(html_len);

    /* Check for no created vector set */
    /* E-Mail Christian Schmitz 2019-09-10: reason unknown  Ticket #164 */
    if (symbol->vector == NULL) {
        return z_errtxt(ZINT_ERROR_INVALID_DATA, symbol, 681, "Vector header NULL");
    }
    if (!zint_fm_open(fmp, symbol, "w")) {
        return ZEXT z_errtxtf(ZINT_ERROR_FILE_ACCESS, symbol, 680, "Could not open SVG output file (%1$d: %2$s)",
                                fmp->err, strerror(fmp->err));
    }

    /* Start writing the header */
    zint_fm_puts("<?xml version=\"1.0\" standalone=\"no\"?>\n"
          "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n",
          fmp);
    zint_fm_printf(fmp, "<svg width=\"%d\" height=\"%d\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n",
                    (int) ceilf(symbol->vector->width), (int) ceilf(symbol->vector->height));
    zint_fm_puts(" <desc>Zint Generated Symbol</desc>\n", fmp);
    if ((symbol->output_options & EMBED_VECTOR_FONT) && symbol->vector->strings) {
        /* Split into `puts()` rather than one very large `printf()` */
        zint_fm_printf(fmp, " <style>@font-face {font-family:\"%s\"; src:url(data:font/woff2;base64,",
                        is_upcean ? "OCRB" : "Arimo");
        zint_fm_puts(is_upcean ? upcean_woff2 : normal_woff2, fmp);
        zint_fm_puts(");}</style>\n", fmp);
    }
    zint_fm_printf(fmp, " <g id=\"barcode\" fill=\"#%s\">\n", fgcolour_string);

    if (bg_alpha != 0) {
        zint_fm_printf(fmp, "  <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" fill=\"#%s\"",
                        (int) ceilf(symbol->vector->width), (int) ceilf(symbol->vector->height), bgcolour_string);
        svg_put_opacity_close(bg_alpha, bg_alpha_opacity, 1 /*close*/, fmp);
    }

    if (symbol->vector->rectangles) {
        int current_colour = 0;
        rect = symbol->vector->rectangles;
        zint_fm_puts("  <path d=\"", fmp);
        while (rect) {
            if (current_colour && rect->colour != current_colour) {
                zint_fm_putc('"', fmp);
                if (current_colour != -1) {
                    svg_pick_colour(current_colour, colour_code);
                    zint_fm_printf(fmp, " fill=\"#%s\"", colour_code);
                }
                svg_put_opacity_close(fg_alpha, fg_alpha_opacity, 1 /*close*/, fmp);
                zint_fm_puts("  <path d=\"", fmp);
            }
            current_colour = rect->colour;
            zint_fm_putsf("M", 2, rect->x, fmp);
            zint_fm_putsf(" ", 2, rect->y, fmp);
            zint_fm_putsf("h", 2, rect->width, fmp);
            zint_fm_putsf("v", 2, rect->height, fmp);
            zint_fm_putsf("h-", 2, rect->width, fmp);
            zint_fm_puts("Z", fmp);
            rect = rect->next;
        }
        zint_fm_putc('"', fmp);
        if (current_colour != -1) {
            svg_pick_colour(current_colour, colour_code);
            zint_fm_printf(fmp, " fill=\"#%s\"", colour_code);
        }
        svg_put_opacity_close(fg_alpha, fg_alpha_opacity, 1 /*close*/, fmp);
    }

    if (symbol->vector->hexagons) {
        previous_diameter = radius = half_radius = half_sqrt3_radius = 0.0f;
        hex = symbol->vector->hexagons;
        zint_fm_puts("  <path d=\"", fmp);
        while (hex) {
            if (previous_diameter != hex->diameter) {
                previous_diameter = hex->diameter;
                radius = 0.5f * previous_diameter;
                half_radius = 0.25f * previous_diameter;
                half_sqrt3_radius = 0.43301270189221932338f * previous_diameter;
            }
            if (hex->rotation == 0 || hex->rotation == 180) {
                zint_fm_putsf("M", 2, hex->x, fmp);
                zint_fm_putsf(" ", 2, hex->y + radius, fmp);
                zint_fm_putsf("L", 2, hex->x + half_sqrt3_radius, fmp);
                zint_fm_putsf(" ", 2, hex->y + half_radius, fmp);
                zint_fm_putsf("L", 2, hex->x + half_sqrt3_radius, fmp);
                zint_fm_putsf(" ", 2, hex->y - half_radius, fmp);
                zint_fm_putsf("L", 2, hex->x, fmp);
                zint_fm_putsf(" ", 2, hex->y - radius, fmp);
                zint_fm_putsf("L", 2, hex->x - half_sqrt3_radius, fmp);
                zint_fm_putsf(" ", 2, hex->y - half_radius, fmp);
                zint_fm_putsf("L", 2, hex->x - half_sqrt3_radius, fmp);
                zint_fm_putsf(" ", 2, hex->y + half_radius, fmp);
            } else {
                zint_fm_putsf("M", 2, hex->x - radius, fmp);
                zint_fm_putsf(" ", 2, hex->y, fmp);
                zint_fm_putsf("L", 2, hex->x - half_radius, fmp);
                zint_fm_putsf(" ", 2, hex->y + half_sqrt3_radius, fmp);
                zint_fm_putsf("L", 2, hex->x + half_radius, fmp);
                zint_fm_putsf(" ", 2, hex->y + half_sqrt3_radius, fmp);
                zint_fm_putsf("L", 2, hex->x + radius, fmp);
                zint_fm_putsf(" ", 2, hex->y, fmp);
                zint_fm_putsf("L", 2, hex->x + half_radius, fmp);
                zint_fm_putsf(" ", 2, hex->y - half_sqrt3_radius, fmp);
                zint_fm_putsf("L", 2, hex->x - half_radius, fmp);
                zint_fm_putsf(" ", 2, hex->y - half_sqrt3_radius, fmp);
            }
            zint_fm_putc('Z', fmp);
            hex = hex->next;
        }
        zint_fm_putc('"', fmp);
        svg_put_opacity_close(fg_alpha, fg_alpha_opacity, 1 /*close*/, fmp);
    }

    previous_diameter = radius = 0.0f;
    circle = symbol->vector->circles;
    while (circle) {
        if (previous_diameter != circle->diameter) {
            previous_diameter = circle->diameter;
            radius = 0.5f * previous_diameter;
        }
        zint_fm_puts("  <circle", fmp);
        svg_put_fattrib(" cx=\"", 2, circle->x, fmp);
        svg_put_fattrib(" cy=\"", 2, circle->y, fmp);
        svg_put_fattrib(" r=\"", circle->width ? 3 : 2, radius, fmp);

        if (circle->colour) { /* Legacy - no longer used */
            if (circle->width) {
                zint_fm_printf(fmp, " stroke=\"#%s\"", bgcolour_string);
                svg_put_fattrib(" stroke-width=\"", 3, circle->width, fmp);
                zint_fm_puts(" fill=\"none\"", fmp);
            } else {
                zint_fm_printf(fmp, " fill=\"#%s\"", bgcolour_string);
            }
            /* This doesn't work how the user is likely to expect - more work needed! */
            svg_put_opacity_close(bg_alpha, bg_alpha_opacity, 1 /*close*/, fmp);
        } else {
            if (circle->width) {
                zint_fm_printf(fmp, " stroke=\"#%s\"", fgcolour_string);
                svg_put_fattrib(" stroke-width=\"", 3, circle->width, fmp);
                zint_fm_puts(" fill=\"none\"", fmp);
            }
            svg_put_opacity_close(fg_alpha, fg_alpha_opacity, 1 /*close*/, fmp);
        }
        circle = circle->next;
    }

    bold = (symbol->output_options & BOLD_TEXT) && !is_upcean;
    string = symbol->vector->strings;
    while (string) {
        const char *const halign = string->halign == 2 ? "end" : string->halign == 1 ? "start" : "middle";
        zint_fm_puts("  <text", fmp);
        svg_put_fattrib(" x=\"", 2, string->x, fmp);
        svg_put_fattrib(" y=\"", 2, string->y, fmp);
        zint_fm_printf(fmp, " text-anchor=\"%s\"", halign);
        if (is_upcean) {
            zint_fm_printf(fmp, " font-family=\"%s, monospace\"", upcean_font_family);
        } else {
            zint_fm_printf(fmp, " font-family=\"%s, Arial, sans-serif\"", normal_font_family);
        }
        svg_put_fattrib(" font-size=\"", 1, string->fsize, fmp);
        if (bold) {
            zint_fm_puts(" font-weight=\"bold\"", fmp);
        }
        if (string->rotation != 0) {
            zint_fm_printf(fmp, " transform=\"rotate(%d", string->rotation);
            zint_fm_putsf(",", 2, string->x, fmp);
            zint_fm_putsf(",", 2, string->y, fmp);
            zint_fm_puts(")\"", fmp);
        }
        svg_put_opacity_close(fg_alpha, fg_alpha_opacity, 0 /*close*/, fmp);
        svg_make_html_friendly(string->text, html_string);
        zint_fm_printf(fmp, "   %s\n", html_string);
        zint_fm_puts("  </text>\n", fmp);
        string = string->next;
    }

    zint_fm_puts(" </g>\n"
          "</svg>\n", fmp);

    if (zint_fm_error(fmp)) {
        ZEXT z_errtxtf(0, symbol, 682, "Incomplete write to SVG output (%1$d: %2$s)", fmp->err, strerror(fmp->err));
        (void) zint_fm_close(fmp, symbol);
        return ZINT_ERROR_FILE_WRITE;
    }

    if (!zint_fm_close(fmp, symbol)) {
        return ZEXT z_errtxtf(ZINT_ERROR_FILE_WRITE, symbol, 684, "Failure on closing SVG output file (%1$d: %2$s)",
                                fmp->err, strerror(fmp->err));
    }

    return 0;
}

/* vim: set ts=4 sw=4 et : */
