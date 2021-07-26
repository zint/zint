/* ps.c - Post Script output */

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

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <math.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"

static void colour_to_pscolor(int option, int colour, char *output) {
    *output = '\0';
    if ((option & CMYK_COLOUR) == 0) {
        // Use RGB colour space
        switch (colour) {
            case 1: // Cyan
                strcat(output, "0.00 1.00 1.00");
                break;
            case 2: // Blue
                strcat(output, "0.00 0.00 1.00");
                break;
            case 3: // Magenta
                strcat(output, "1.00 0.00 1.00");
                break;
            case 4: // Red
                strcat(output, "1.00 0.00 0.00");
                break;
            case 5: // Yellow
                strcat(output, "1.00 1.00 0.00");
                break;
            case 6: // Green
                strcat(output, "0.00 1.00 0.00");
                break;
            case 8: // White
                strcat(output, "1.00 1.00 1.00");
                break;
            default: // Black
                strcat(output, "0.00 0.00 0.00");
                break;
        }
        strcat(output, " setrgbcolor");
    } else {
        // Use CMYK colour space
        switch (colour) {
            case 1: // Cyan
                strcat(output, "1.00 0.00 0.00 0.00");
                break;
            case 2: // Blue
                strcat(output, "1.00 1.00 0.00 0.00");
                break;
            case 3: // Magenta
                strcat(output, "0.00 1.00 0.00 0.00");
                break;
            case 4: // Red
                strcat(output, "0.00 1.00 1.00 0.00");
                break;
            case 5: // Yellow
                strcat(output, "0.00 0.00 1.00 0.00");
                break;
            case 6: // Green
                strcat(output, "1.00 0.00 1.00 0.00");
                break;
            case 8: // White
                strcat(output, "0.00 0.00 0.00 0.00");
                break;
            default: // Black
                strcat(output, "0.00 0.00 0.00 1.00");
                break;
        }
        strcat(output, " setcmykcolor");
    }
}

STATIC_UNLESS_ZINT_TEST void ps_convert(const unsigned char *string, unsigned char *ps_string) {
    const unsigned char *s;
    unsigned char *p = ps_string;

    for (s = string; *s; s++) {
        switch (*s) {
            case '(':
            case ')':
            case '\\':
                *p++ = '\\';
                *p++ = *s;
                break;
            case 0xC2: /* See `to_iso8859_1()` in raster.c */
                *p++ = *++s;
                break;
            case 0xC3:
                *p++ = *++s + 64;
                break;
            default:
                if (*s < 0x80) {
                    *p++ = *s;
                }
                break;

        }
    }
    *p = '\0';
}

INTERNAL int ps_plot(struct zint_symbol *symbol) {
    FILE *feps;
    int fgred, fggrn, fgblu, bgred, bggrn, bgblu;
    float red_ink, green_ink, blue_ink, red_paper, green_paper, blue_paper;
    float cyan_ink, magenta_ink, yellow_ink, black_ink;
    float cyan_paper, magenta_paper, yellow_paper, black_paper;
    int error_number = 0;
    float ax, ay, bx, by, cx, cy, dx, dy, ex, ey, fx, fy;
    float previous_diameter;
    float radius, half_radius, half_sqrt3_radius;
    int colour_index, colour_rect_flag;
    char ps_color[33]; /* max "1.00 0.00 0.00 0.00 setcmykcolor" = 32 + 1 */
    int draw_background = 1;
    struct zint_vector_rect *rect;
    struct zint_vector_hexagon *hex;
    struct zint_vector_circle *circle;
    struct zint_vector_string *string;
    const char *locale = NULL;
    const char *font;
    int i, len;
    int ps_len = 0;
    int iso_latin1 = 0;
    const int output_to_stdout = symbol->output_options & BARCODE_STDOUT;
#ifdef _MSC_VER
    unsigned char *ps_string;
#endif

    if (symbol->vector == NULL) {
        strcpy(symbol->errtxt, "646: Vector header NULL");
        return ZINT_ERROR_INVALID_DATA;
    }

    if (strlen(symbol->bgcolour) > 6) {
        if ((ctoi(symbol->bgcolour[6]) == 0) && (ctoi(symbol->bgcolour[7]) == 0)) {
            draw_background = 0;
        }
    }

    if (output_to_stdout) {
        feps = stdout;
    } else {
        if (!(feps = fopen(symbol->outfile, "w"))) {
            sprintf(symbol->errtxt, "645: Could not open output file (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_ACCESS;
        }
    }

    locale = setlocale(LC_ALL, "C");

    fgred = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
    fggrn = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
    fgblu = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
    bgred = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
    bggrn = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
    bgblu = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);
    red_ink = (float) (fgred / 256.0);
    green_ink = (float) (fggrn / 256.0);
    blue_ink = (float) (fgblu / 256.0);
    red_paper = (float) (bgred / 256.0);
    green_paper = (float) (bggrn / 256.0);
    blue_paper = (float) (bgblu / 256.0);

    /* Convert RGB to CMYK */
    if (red_ink > green_ink) {
        if (blue_ink > red_ink) {
            black_ink = 1.0f - blue_ink;
        } else {
            black_ink = 1.0f - red_ink;
        }
    } else {
        if (blue_ink > red_ink) {
            black_ink = 1.0f - blue_ink;
        } else {
            black_ink = 1.0f - green_ink;
        }
    }
    if (black_ink < 1.0f) {
        cyan_ink = (1.0f - red_ink - black_ink) / (1.0f - black_ink);
        magenta_ink = (1.0f - green_ink - black_ink) / (1.0f - black_ink);
        yellow_ink = (1.0f - blue_ink - black_ink) / (1.0f - black_ink);
    } else {
        cyan_ink = 0.0f;
        magenta_ink = 0.0f;
        yellow_ink = 0.0f;
    }

    if (red_paper > green_paper) {
        if (blue_paper > red_paper) {
            black_paper = 1.0f - blue_paper;
        } else {
            black_paper = 1.0f - red_paper;
        }
    } else {
        if (blue_paper > red_paper) {
            black_paper = 1.0f - blue_paper;
        } else {
            black_paper = 1.0f - green_paper;
        }
    }
    if (black_paper < 1.0f) {
        cyan_paper = (1.0f - red_paper - black_paper) / (1.0f - black_paper);
        magenta_paper = (1.0f - green_paper - black_paper) / (1.0f - black_paper);
        yellow_paper = (1.0f - blue_paper - black_paper) / (1.0f - black_paper);
    } else {
        cyan_paper = 0.0f;
        magenta_paper = 0.0f;
        yellow_paper = 0.0f;
    }

    for (i = 0, len = (int) ustrlen(symbol->text); i < len; i++) {
        switch (symbol->text[i]) {
            case '(':
            case ')':
            case '\\':
                ps_len += 2;
                break;
            default:
                if (!iso_latin1 && symbol->text[i] >= 0x80) {
                    iso_latin1 = 1;
                }
                ps_len++; /* Will overcount 2 byte UTF-8 chars */
                break;
        }
    }

#ifndef _MSC_VER
    unsigned char ps_string[ps_len + 1];
#else
    ps_string = (unsigned char *) _alloca(ps_len + 1);
#endif

    /* Start writing the header */
    fprintf(feps, "%%!PS-Adobe-3.0 EPSF-3.0\n");
    if (ZINT_VERSION_BUILD) {
        fprintf(feps, "%%%%Creator: Zint %d.%d.%d.%d\n",
                ZINT_VERSION_MAJOR, ZINT_VERSION_MINOR, ZINT_VERSION_RELEASE, ZINT_VERSION_BUILD);
    } else {
        fprintf(feps, "%%%%Creator: Zint %d.%d.%d\n", ZINT_VERSION_MAJOR, ZINT_VERSION_MINOR, ZINT_VERSION_RELEASE);
    }
    fprintf(feps, "%%%%Title: Zint Generated Symbol\n");
    fprintf(feps, "%%%%Pages: 0\n");
    fprintf(feps, "%%%%BoundingBox: 0 0 %d %d\n",
            (int) ceilf(symbol->vector->width), (int) ceilf(symbol->vector->height));
    fprintf(feps, "%%%%EndComments\n");

    /* Definitions */
    fprintf(feps, "/TL { setlinewidth moveto lineto stroke } bind def\n");
    fprintf(feps, "/TD { newpath 0 360 arc fill } bind def\n");
    fprintf(feps, "/TH { 0 setlinewidth moveto lineto lineto lineto lineto lineto closepath fill } bind def\n");
    fprintf(feps, "/TB { 2 copy } bind def\n");
    fprintf(feps, "/TR { newpath 4 1 roll exch moveto 1 index 0 rlineto 0 exch rlineto neg 0 rlineto closepath fill } bind def\n");
    fprintf(feps, "/TE { pop pop } bind def\n");

    fprintf(feps, "newpath\n");

    /* Now the actual representation */

    // Background
    if (draw_background) {
        if ((symbol->output_options & CMYK_COLOUR) == 0) {
            fprintf(feps, "%.2f %.2f %.2f setrgbcolor\n", red_paper, green_paper, blue_paper);
        } else {
            fprintf(feps, "%.2f %.2f %.2f %.2f setcmykcolor\n", cyan_paper, magenta_paper, yellow_paper, black_paper);
        }

        fprintf(feps, "%.2f 0.00 TB 0.00 %.2f TR\n", symbol->vector->height, symbol->vector->width);
        fprintf(feps, "TE\n");
    }

    if (symbol->symbology != BARCODE_ULTRA) {
        if ((symbol->output_options & CMYK_COLOUR) == 0) {
            fprintf(feps, "%.2f %.2f %.2f setrgbcolor\n", red_ink, green_ink, blue_ink);
        } else {
            fprintf(feps, "%.2f %.2f %.2f %.2f setcmykcolor\n", cyan_ink, magenta_ink, yellow_ink, black_ink);
        }
    }

    // Rectangles
    if (symbol->symbology == BARCODE_ULTRA) {
        colour_rect_flag = 0;
        rect = symbol->vector->rectangles;
        while (rect) {
            if (rect->colour == -1) { // Foreground
                if (colour_rect_flag == 0) {
                    // Set foreground colour
                    if ((symbol->output_options & CMYK_COLOUR) == 0) {
                        fprintf(feps, "%.2f %.2f %.2f setrgbcolor\n", red_ink, green_ink, blue_ink);
                    } else {
                        fprintf(feps, "%.2f %.2f %.2f %.2f setcmykcolor\n",
                                cyan_ink, magenta_ink, yellow_ink, black_ink);
                    }
                    colour_rect_flag = 1;
                }
                fprintf(feps, "%.2f %.2f TB %.2f %.2f TR\n",
                        rect->height, (symbol->vector->height - rect->y) - rect->height, rect->x, rect->width);
                fprintf(feps, "TE\n");
            }
            rect = rect->next;
        }
        for (colour_index = 1; colour_index <= 8; colour_index++) {
            colour_rect_flag = 0;
            rect = symbol->vector->rectangles;
            while (rect) {
                if (rect->colour == colour_index) {
                    if (colour_rect_flag == 0) {
                        // Set new colour
                        colour_to_pscolor(symbol->output_options, colour_index, ps_color);
                        fprintf(feps, "%s\n", ps_color);
                        colour_rect_flag = 1;
                    }
                    fprintf(feps, "%.2f %.2f TB %.2f %.2f TR\n",
                            rect->height, (symbol->vector->height - rect->y) - rect->height, rect->x, rect->width);
                    fprintf(feps, "TE\n");
                }
                rect = rect->next;
            }
        }
    } else {
        rect = symbol->vector->rectangles;
        while (rect) {
            fprintf(feps, "%.2f %.2f TB %.2f %.2f TR\n",
                    rect->height, (symbol->vector->height - rect->y) - rect->height, rect->x, rect->width);
            fprintf(feps, "TE\n");
            rect = rect->next;
        }
    }

    // Hexagons
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
            ay = (symbol->vector->height - hex->y) + radius;
            by = (symbol->vector->height - hex->y) + half_radius;
            cy = (symbol->vector->height - hex->y) - half_radius;
            dy = (symbol->vector->height - hex->y) - radius;
            ey = (symbol->vector->height - hex->y) - half_radius;
            fy = (symbol->vector->height - hex->y) + half_radius;
            ax = hex->x;
            bx = hex->x + half_sqrt3_radius;
            cx = hex->x + half_sqrt3_radius;
            dx = hex->x;
            ex = hex->x - half_sqrt3_radius;
            fx = hex->x - half_sqrt3_radius;
        } else {
            ay = (symbol->vector->height - hex->y);
            by = (symbol->vector->height - hex->y) + half_sqrt3_radius;
            cy = (symbol->vector->height - hex->y) + half_sqrt3_radius;
            dy = (symbol->vector->height - hex->y);
            ey = (symbol->vector->height - hex->y) - half_sqrt3_radius;
            fy = (symbol->vector->height - hex->y) - half_sqrt3_radius;
            ax = hex->x - radius;
            bx = hex->x - half_radius;
            cx = hex->x + half_radius;
            dx = hex->x + radius;
            ex = hex->x + half_radius;
            fx = hex->x - half_radius;
        }
        fprintf(feps, "%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f TH\n",
                ax, ay, bx, by, cx, cy, dx, dy, ex, ey, fx, fy);
        hex = hex->next;
    }

    // Circles
    previous_diameter = radius = 0.0f;
    circle = symbol->vector->circles;
    while (circle) {
        if (previous_diameter != circle->diameter) {
            previous_diameter = circle->diameter;
            radius = (float) (0.5 * previous_diameter);
        }
        if (circle->colour) {
            // A 'white' circle
            if ((symbol->output_options & CMYK_COLOUR) == 0) {
                fprintf(feps, "%.2f %.2f %.2f setrgbcolor\n", red_paper, green_paper, blue_paper);
            } else {
                fprintf(feps, "%.2f %.2f %.2f %.2f setcmykcolor\n",
                        cyan_paper, magenta_paper, yellow_paper, black_paper);
            }
            fprintf(feps, "%.2f %.2f %.2f TD\n", circle->x, (symbol->vector->height - circle->y), radius);
            if (circle->next) {
                if ((symbol->output_options & CMYK_COLOUR) == 0) {
                    fprintf(feps, "%.2f %.2f %.2f setrgbcolor\n", red_ink, green_ink, blue_ink);
                } else {
                    fprintf(feps, "%.2f %.2f %.2f %.2f setcmykcolor\n", cyan_ink, magenta_ink, yellow_ink, black_ink);
                }
            }
        } else {
            // A 'black' circle
            fprintf(feps, "%.2f %.2f %.2f TD\n", circle->x, (symbol->vector->height - circle->y), radius);
        }
        circle = circle->next;
    }

    // Text

    string = symbol->vector->strings;

    if (string) {
        if ((symbol->output_options & BOLD_TEXT)
                && (!is_extendable(symbol->symbology) || (symbol->output_options & SMALL_TEXT))) {
            font = "Helvetica-Bold";
        } else {
            font = "Helvetica";
        }
        if (iso_latin1) {
            /* Change encoding to ISO 8859-1, see Postscript Language Reference Manual 2nd Edition Example 5.6 */
            fprintf(feps, "/%s findfont\n", font);
            fprintf(feps, "dup length dict begin\n");
            fprintf(feps, "{1 index /FID ne {def} {pop pop} ifelse} forall\n");
            fprintf(feps, "/Encoding ISOLatin1Encoding def\n");
            fprintf(feps, "currentdict\n");
            fprintf(feps, "end\n");
            fprintf(feps, "/Helvetica-ISOLatin1 exch definefont pop\n");
            font = "Helvetica-ISOLatin1";
        }
        do {
            ps_convert(string->text, ps_string);
            fprintf(feps, "matrix currentmatrix\n");
            fprintf(feps, "/%s findfont\n", font);
            fprintf(feps, "%.2f scalefont setfont\n", string->fsize);
            fprintf(feps, " 0 0 moveto %.2f %.2f translate 0.00 rotate 0 0 moveto\n",
                    string->x, (symbol->vector->height - string->y));
            if (string->halign == 0 || string->halign == 2) { /* Need width for middle or right align */
                fprintf(feps, " (%s) stringwidth\n", ps_string);
            }
            if (string->rotation != 0) {
                fprintf(feps, "gsave\n");
                fprintf(feps, "%d rotate\n", 360 - string->rotation);
            }
            if (string->halign == 0 || string->halign == 2) {
                fprintf(feps, "pop\n");
                fprintf(feps, "%s 0 rmoveto\n", string->halign == 2 ? "neg" : "-2 div");
            }
            fprintf(feps, " (%s) show\n", ps_string);
            if (string->rotation != 0) {
                fprintf(feps, "grestore\n");
            }
            fprintf(feps, "setmatrix\n");
            string = string->next;
        } while (string);
    }

    if (output_to_stdout) {
        fflush(feps);
    } else {
        fclose(feps);
    }

    if (locale)
        setlocale(LC_ALL, locale);

    return error_number;
}
