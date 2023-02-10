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
#include <locale.h>
#include <math.h>
#include <stdio.h>

#include "common.h"
#include "output.h"

static void pick_colour(int colour, char colour_code[]) {
    switch (colour) {
        case 1: /* Cyan */
            strcpy(colour_code, "00ffff");
            break;
        case 2: /* Blue */
            strcpy(colour_code, "0000ff");
            break;
        case 3: /* Magenta */
            strcpy(colour_code, "ff00ff");
            break;
        case 4: /* Red */
            strcpy(colour_code, "ff0000");
            break;
        case 5: /* Yellow */
            strcpy(colour_code, "ffff00");
            break;
        case 6: /* Green */
            strcpy(colour_code, "00ff00");
            break;
        case 8: /* White */
            strcpy(colour_code, "ffffff");
            break;
        default: /* Black */
            strcpy(colour_code, "000000");
            break;
    }
}

static void make_html_friendly(unsigned char *string, char *html_version) {
    /* Converts text to use HTML entity codes */

    int i, len, html_pos;

    html_pos = 0;
    html_version[html_pos] = '\0';
    len = (int) ustrlen(string);

    for (i = 0; i < len; i++) {
        switch (string[i]) {
            case '>':
                strcat(html_version, "&gt;");
                html_pos += 4;
                break;

            case '<':
                strcat(html_version, "&lt;");
                html_pos += 4;
                break;

            case '&':
                strcat(html_version, "&amp;");
                html_pos += 5;
                break;

            case '"':
                strcat(html_version, "&quot;");
                html_pos += 6;
                break;

            case '\'':
                strcat(html_version, "&apos;");
                html_pos += 6;
                break;

            default:
                html_version[html_pos] = string[i];
                html_pos++;
                html_version[html_pos] = '\0';
                break;
         }
    }
}

INTERNAL int svg_plot(struct zint_symbol *symbol) {
    static const char font_family[] = "Helvetica, sans-serif";
    FILE *fsvg;
    int error_number = 0;
    const char *locale = NULL;
    float ax, ay, bx, by, cx, cy, dx, dy, ex, ey, fx, fy;
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

    html_string = (char *) z_alloca(html_len);

    /* Check for no created vector set */
    /* E-Mail Christian Schmitz 2019-09-10: reason unknown  Ticket #164*/
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

    locale = setlocale(LC_ALL, "C");

    /* Start writing the header */
    fputs("<?xml version=\"1.0\" standalone=\"no\"?>\n"
          "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
          "   \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n", fsvg);
    fprintf(fsvg, "<svg width=\"%d\" height=\"%d\" version=\"1.1\"\n",
            (int) ceilf(symbol->vector->width), (int) ceilf(symbol->vector->height));
    fputs("   xmlns=\"http://www.w3.org/2000/svg\">\n"
          "   <desc>Zint Generated Symbol\n"
          "   </desc>\n", fsvg);
    fprintf(fsvg, "\n   <g id=\"barcode\" fill=\"#%s\">\n", fgcolour_string);

    if (bg_alpha != 0) {
        fprintf(fsvg, "      <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" fill=\"#%s\"",
                (int) ceilf(symbol->vector->width), (int) ceilf(symbol->vector->height), bgcolour_string);
        if (bg_alpha != 0xff) {
            fprintf(fsvg, " opacity=\"%.3f\"", bg_alpha_opacity);
        }
        fputs(" />\n", fsvg);
    }

    rect = symbol->vector->rectangles;
    while (rect) {
        fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"",
                rect->x, rect->y, rect->width, rect->height);
        if (rect->colour != -1) {
            pick_colour(rect->colour, colour_code);
            fprintf(fsvg, " fill=\"#%s\"", colour_code);
        }
        if (fg_alpha != 0xff) {
            fprintf(fsvg, " opacity=\"%.3f\"", fg_alpha_opacity);
        }
        fputs(" />\n", fsvg);
        rect = rect->next;
    }

    previous_diameter = radius = half_radius = half_sqrt3_radius = 0.0f;
    hex = symbol->vector->hexagons;
    while (hex) {
        if (previous_diameter != hex->diameter) {
            previous_diameter = hex->diameter;
            radius = (float) (0.5 * previous_diameter);
            half_radius = (float) (0.25 * previous_diameter);
            half_sqrt3_radius = (float) (0.43301270189221932338 * previous_diameter);
        }
        if ((hex->rotation == 0) || (hex->rotation == 180)) {
            ay = hex->y + radius;
            by = hex->y + half_radius;
            cy = hex->y - half_radius;
            dy = hex->y - radius;
            ey = hex->y - half_radius;
            fy = hex->y + half_radius;
            ax = hex->x;
            bx = hex->x + half_sqrt3_radius;
            cx = hex->x + half_sqrt3_radius;
            dx = hex->x;
            ex = hex->x - half_sqrt3_radius;
            fx = hex->x - half_sqrt3_radius;
        } else {
            ay = hex->y;
            by = hex->y + half_sqrt3_radius;
            cy = hex->y + half_sqrt3_radius;
            dy = hex->y;
            ey = hex->y - half_sqrt3_radius;
            fy = hex->y - half_sqrt3_radius;
            ax = hex->x - radius;
            bx = hex->x - half_radius;
            cx = hex->x + half_radius;
            dx = hex->x + radius;
            ex = hex->x + half_radius;
            fx = hex->x - half_radius;
        }
        fprintf(fsvg, "      <path d=\"M %.2f %.2f L %.2f %.2f L %.2f %.2f L %.2f %.2f L %.2f %.2f L %.2f %.2f Z\"",
                ax, ay, bx, by, cx, cy, dx, dy, ex, ey, fx, fy);
        if (fg_alpha != 0xff) {
            fprintf(fsvg, " opacity=\"%.3f\"", fg_alpha_opacity);
        }
        fputs(" />\n", fsvg);
        hex = hex->next;
    }

    previous_diameter = radius = 0.0f;
    circle = symbol->vector->circles;
    while (circle) {
        if (previous_diameter != circle->diameter) {
            previous_diameter = circle->diameter;
            radius = (float) (0.5 * previous_diameter);
        }
        fprintf(fsvg, "      <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.*f\"",
                circle->x, circle->y, circle->width ? 3 : 2, radius);

        if (circle->colour) { /* Legacy - no longer used */
            if (circle->width) {
                fprintf(fsvg, " stroke=\"#%s\" stroke-width=\"%.3f\" fill=\"none\"", bgcolour_string, circle->width);
            } else {
                fprintf(fsvg, " fill=\"#%s\"", bgcolour_string);
            }
            if (bg_alpha != 0xff) {
                /* This doesn't work how the user is likely to expect - more work needed! */
                fprintf(fsvg, " opacity=\"%.3f\"", bg_alpha_opacity);
            }
        } else {
            if (circle->width) {
                fprintf(fsvg, " stroke=\"#%s\" stroke-width=\"%.3f\" fill=\"none\"", fgcolour_string, circle->width);
            }
            if (fg_alpha != 0xff) {
                fprintf(fsvg, " opacity=\"%.3f\"", fg_alpha_opacity);
            }
        }
        fputs(" />\n", fsvg);
        circle = circle->next;
    }

    bold = (symbol->output_options & BOLD_TEXT) && !is_extendable(symbol->symbology);
    string = symbol->vector->strings;
    while (string) {
        const char *const halign = string->halign == 2 ? "end" : string->halign == 1 ? "start" : "middle";
        fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"%s\"\n", string->x, string->y, halign);
        fprintf(fsvg, "         font-family=\"%s\" font-size=\"%.1f\"", font_family, string->fsize);
        if (bold) {
            fputs(" font-weight=\"bold\"", fsvg);
        }
        if (fg_alpha != 0xff) {
            fprintf(fsvg, " opacity=\"%.3f\"", fg_alpha_opacity);
        }
        if (string->rotation != 0) {
            fprintf(fsvg, " transform=\"rotate(%d,%.2f,%.2f)\"", string->rotation, string->x, string->y);
        }
        fputs(" >\n", fsvg);
        make_html_friendly(string->text, html_string);
        fprintf(fsvg, "         %s\n", html_string);
        fputs("      </text>\n", fsvg);
        string = string->next;
    }

    fputs("   </g>\n"
          "</svg>\n", fsvg);

    if (locale)
        setlocale(LC_ALL, locale);

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
