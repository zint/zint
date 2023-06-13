/* svg.c - Scalable Vector Graphics */
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

#include "common.h"
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
    strcpy(colour_code, rgbs[idx]);
}

/* Convert text to use HTML entity codes */
static void svg_make_html_friendly(const unsigned char *string, char *html_version) {

    for (; *string; string++) {
        switch (*string) {
            case '>':
                strcpy(html_version, "&gt;");
                html_version += 4;
                break;

            case '<':
                strcpy(html_version, "&lt;");
                html_version += 4;
                break;

            case '&':
                strcpy(html_version, "&amp;");
                html_version += 5;
                break;

            case '"':
                strcpy(html_version, "&quot;");
                html_version += 6;
                break;

            case '\'':
                strcpy(html_version, "&apos;");
                html_version += 6;
                break;

            default:
                *html_version++ = *string;
                break;
         }
    }

    *html_version = '\0';
}

/* Helper to output floating point attribute */
static void svg_put_fattrib(const char *prefix, const int dp, const float val, FILE *fsvg) {
    out_putsf(prefix, dp, val, fsvg);
    fputc('"', fsvg);
}

/* Helper to output opacity attribute attribute and close tag (maybe) */
static void svg_put_opacity_close(const unsigned char alpha, const float val, const int close, FILE *fsvg) {
    if (alpha != 0xff) {
        svg_put_fattrib(" opacity=\"", 3, val, fsvg);
    }
    if (close) {
        fputc('/', fsvg);
    }
    fputs(">\n", fsvg);
}

INTERNAL int svg_plot(struct zint_symbol *symbol) {
    static const char normal_font_family[] = "Arimo, Arial, sans-serif";
    static const char upcean_font_family[] = "OCRB, monospace";
    FILE *fsvg;
    int error_number = 0;
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
    int len, html_len;

    const int extendable = is_extendable(symbol->symbology);
    const int output_to_stdout = symbol->output_options & BARCODE_STDOUT;
    char *html_string;

    (void) out_colour_get_rgb(symbol->fgcolour, &fgred, &fggreen, &fgblue, &fg_alpha);
    if (fg_alpha != 0xff) {
        fg_alpha_opacity = fg_alpha / 255.0f;
    }
    sprintf(fgcolour_string, "%02X%02X%02X", fgred, fggreen, fgblue);
    (void) out_colour_get_rgb(symbol->bgcolour, &bgred, &bggreen, &bgblue, &bg_alpha);
    if (bg_alpha != 0xff) {
        bg_alpha_opacity = bg_alpha / 255.0f;
    }
    sprintf(bgcolour_string, "%02X%02X%02X", bgred, bggreen, bgblue);

    len = (int) ustrlen(symbol->text);
    html_len = len + 1;

    for (i = 0; i < len; i++) {
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
        strcpy(symbol->errtxt, "681: Vector header NULL");
        return ZINT_ERROR_INVALID_DATA;
    }
    if (output_to_stdout) {
        fsvg = stdout;
    } else {
        if (!(fsvg = out_fopen(symbol->outfile, "w"))) {
            sprintf(symbol->errtxt, "680: Could not open output file (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_ACCESS;
        }
    }

    /* Start writing the header */
    fputs("<?xml version=\"1.0\" standalone=\"no\"?>\n"
          "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n",
          fsvg);
    fprintf(fsvg, "<svg width=\"%d\" height=\"%d\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n",
            (int) ceilf(symbol->vector->width), (int) ceilf(symbol->vector->height));
    fputs(" <desc>Zint Generated Symbol</desc>\n", fsvg);
    if ((symbol->output_options & EMBED_VECTOR_FONT) && symbol->vector->strings) {
        fprintf(fsvg, " <style>@font-face {font-family:\"%s\"; src:url(data:font/woff2;base64,%s);}</style>\n",
                extendable ? "OCRB" : "Arimo", extendable ? upcean_woff2 : normal_woff2);
    }
    fprintf(fsvg, " <g id=\"barcode\" fill=\"#%s\">\n", fgcolour_string);

    if (bg_alpha != 0) {
        fprintf(fsvg, "  <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" fill=\"#%s\"",
                (int) ceilf(symbol->vector->width), (int) ceilf(symbol->vector->height), bgcolour_string);
        svg_put_opacity_close(bg_alpha, bg_alpha_opacity, 1 /*close*/, fsvg);
    }

    if (symbol->vector->rectangles) {
        int current_colour = 0;
        rect = symbol->vector->rectangles;
        fputs("  <path d=\"", fsvg);
        while (rect) {
            if (current_colour && rect->colour != current_colour) {
                fputc('"', fsvg);
                if (current_colour != -1) {
                    svg_pick_colour(current_colour, colour_code);
                    fprintf(fsvg, " fill=\"#%s\"", colour_code);
                }
                svg_put_opacity_close(fg_alpha, fg_alpha_opacity, 1 /*close*/, fsvg);
                fputs("  <path d=\"", fsvg);
            }
            current_colour = rect->colour;
            out_putsf("M", 2, rect->x, fsvg);
            out_putsf(" ", 2, rect->y, fsvg);
            out_putsf("h", 2, rect->width, fsvg);
            out_putsf("v", 2, rect->height, fsvg);
            out_putsf("h-", 2, rect->width, fsvg);
            fputs("Z", fsvg);
            rect = rect->next;
        }
        fputc('"', fsvg);
        if (current_colour != -1) {
            svg_pick_colour(current_colour, colour_code);
            fprintf(fsvg, " fill=\"#%s\"", colour_code);
        }
        svg_put_opacity_close(fg_alpha, fg_alpha_opacity, 1 /*close*/, fsvg);
    }

    if (symbol->vector->hexagons) {
        previous_diameter = radius = half_radius = half_sqrt3_radius = 0.0f;
        hex = symbol->vector->hexagons;
        fputs("  <path d=\"", fsvg);
        while (hex) {
            if (previous_diameter != hex->diameter) {
                previous_diameter = hex->diameter;
                radius = (float) (0.5 * previous_diameter);
                half_radius = (float) (0.25 * previous_diameter);
                half_sqrt3_radius = (float) (0.43301270189221932338 * previous_diameter);
            }
            if ((hex->rotation == 0) || (hex->rotation == 180)) {
                out_putsf("M", 2, hex->x, fsvg);
                out_putsf(" ", 2, hex->y + radius, fsvg);
                out_putsf("L", 2, hex->x + half_sqrt3_radius, fsvg);
                out_putsf(" ", 2, hex->y + half_radius, fsvg);
                out_putsf("L", 2, hex->x + half_sqrt3_radius, fsvg);
                out_putsf(" ", 2, hex->y - half_radius, fsvg);
                out_putsf("L", 2, hex->x, fsvg);
                out_putsf(" ", 2, hex->y - radius, fsvg);
                out_putsf("L", 2, hex->x - half_sqrt3_radius, fsvg);
                out_putsf(" ", 2, hex->y - half_radius, fsvg);
                out_putsf("L", 2, hex->x - half_sqrt3_radius, fsvg);
                out_putsf(" ", 2, hex->y + half_radius, fsvg);
            } else {
                out_putsf("M", 2, hex->x - radius, fsvg);
                out_putsf(" ", 2, hex->y, fsvg);
                out_putsf("L", 2, hex->x - half_radius, fsvg);
                out_putsf(" ", 2, hex->y + half_sqrt3_radius, fsvg);
                out_putsf("L", 2, hex->x + half_radius, fsvg);
                out_putsf(" ", 2, hex->y + half_sqrt3_radius, fsvg);
                out_putsf("L", 2, hex->x + radius, fsvg);
                out_putsf(" ", 2, hex->y, fsvg);
                out_putsf("L", 2, hex->x + half_radius, fsvg);
                out_putsf(" ", 2, hex->y - half_sqrt3_radius, fsvg);
                out_putsf("L", 2, hex->x - half_radius, fsvg);
                out_putsf(" ", 2, hex->y - half_sqrt3_radius, fsvg);
            }
            fputc('Z', fsvg);
            hex = hex->next;
        }
        fputc('"', fsvg);
        svg_put_opacity_close(fg_alpha, fg_alpha_opacity, 1 /*close*/, fsvg);
    }

    previous_diameter = radius = 0.0f;
    circle = symbol->vector->circles;
    while (circle) {
        if (previous_diameter != circle->diameter) {
            previous_diameter = circle->diameter;
            radius = (float) (0.5 * previous_diameter);
        }
        fputs("  <circle", fsvg);
        svg_put_fattrib(" cx=\"", 2, circle->x, fsvg);
        svg_put_fattrib(" cy=\"", 2, circle->y, fsvg);
        svg_put_fattrib(" r=\"", circle->width ? 3 : 2, radius, fsvg);

        if (circle->colour) { /* Legacy - no longer used */
            if (circle->width) {
                fprintf(fsvg, " stroke=\"#%s\"", bgcolour_string);
                svg_put_fattrib(" stroke-width=\"", 3, circle->width, fsvg);
                fputs(" fill=\"none\"", fsvg);
            } else {
                fprintf(fsvg, " fill=\"#%s\"", bgcolour_string);
            }
            /* This doesn't work how the user is likely to expect - more work needed! */
            svg_put_opacity_close(bg_alpha, bg_alpha_opacity, 1 /*close*/, fsvg);
        } else {
            if (circle->width) {
                fprintf(fsvg, " stroke=\"#%s\"", fgcolour_string);
                svg_put_fattrib(" stroke-width=\"", 3, circle->width, fsvg);
                fputs(" fill=\"none\"", fsvg);
            }
            svg_put_opacity_close(fg_alpha, fg_alpha_opacity, 1 /*close*/, fsvg);
        }
        circle = circle->next;
    }

    bold = (symbol->output_options & BOLD_TEXT) && !extendable;
    string = symbol->vector->strings;
    while (string) {
        const char *const halign = string->halign == 2 ? "end" : string->halign == 1 ? "start" : "middle";
        fputs("  <text", fsvg);
        svg_put_fattrib(" x=\"", 2, string->x, fsvg);
        svg_put_fattrib(" y=\"", 2, string->y, fsvg);
        fprintf(fsvg, " text-anchor=\"%s\"", halign);
        fprintf(fsvg, " font-family=\"%s\"", extendable ? upcean_font_family : normal_font_family);
        svg_put_fattrib(" font-size=\"", 1, string->fsize, fsvg);
        if (bold) {
            fputs(" font-weight=\"bold\"", fsvg);
        }
        if (string->rotation != 0) {
            fprintf(fsvg, " transform=\"rotate(%d", string->rotation);
            out_putsf(",", 2, string->x, fsvg);
            out_putsf(",", 2, string->y, fsvg);
            fputs(")\"", fsvg);
        }
        svg_put_opacity_close(fg_alpha, fg_alpha_opacity, 0 /*close*/, fsvg);
        svg_make_html_friendly(string->text, html_string);
        fprintf(fsvg, "   %s\n", html_string);
        fputs("  </text>\n", fsvg);
        string = string->next;
    }

    fputs(" </g>\n"
          "</svg>\n", fsvg);

    if (ferror(fsvg)) {
        sprintf(symbol->errtxt, "682: Incomplete write to output (%d: %.30s)", errno, strerror(errno));
        if (!output_to_stdout) {
            (void) fclose(fsvg);
        }
        return ZINT_ERROR_FILE_WRITE;
    }

    if (output_to_stdout) {
        if (fflush(fsvg) != 0) {
            sprintf(symbol->errtxt, "683: Incomplete flush to output (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_WRITE;
        }
    } else {
        if (fclose(fsvg) != 0) {
            sprintf(symbol->errtxt, "684: Failure on closing output file (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_WRITE;
        }
    }

    return error_number;
}

/* vim: set ts=4 sw=4 et : */
