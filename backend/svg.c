/* svg.c - Scalable Vector Graphics */

/*
    libzint - the open source barcode library
    Copyright (C) 2009-2019 Robin Stuart <rstuart114@gmail.com>

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

#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif

#include "common.h"

void make_html_friendly(unsigned char * string, char * html_version) {
    /* Converts text to use HTML entity codes */

    int i, html_pos;

    html_pos = 0;
    html_version[html_pos] = '\0';

    for (i = 0; i < ustrlen(string); i++) {
        switch(string[i]) {
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

int svg_plot(struct zint_symbol *symbol) {
    FILE *fsvg;
    int error_number = 0;
    const char *locale = NULL;
    float ax, ay, bx, by, cx, cy, dx, dy, ex, ey, fx, fy;
    float radius;
    int i;

    struct zint_vector_rect *rect;
    struct zint_vector_hexagon *hex;
    struct zint_vector_circle *circle;
    struct zint_vector_string *string;

#ifdef _MSC_VER
    char* html_string;
#endif

    int html_len = strlen((char *)symbol->text) + 1;

    for (i = 0; i < strlen((char *)symbol->text); i++) {
        switch(symbol->text[i]) {
            case '>':
            case '<':
            case '"':
            case '&':
            case '\'':
                html_len += 6;
                break;
        }
    }

#ifndef _MSC_VER
    char html_string[html_len];
#else
    html_string = (char*) _alloca(html_len);
#endif

    /* Check for no created vector set */
    /* E-Mail Christian Schmitz 2019-09-10: reason unknown  Ticket #164*/
    if (symbol->vector == NULL) {
        return ZINT_ERROR_INVALID_DATA;
    }
    if (symbol->output_options & BARCODE_STDOUT) {
        fsvg = stdout;
    } else {
        fsvg = fopen(symbol->outfile, "w");
    }
    if (fsvg == NULL) {
        strcpy(symbol->errtxt, "660: Could not open output file");
        return ZINT_ERROR_FILE_ACCESS;
    }

    locale = setlocale(LC_ALL, "C");

    /* Start writing the header */
    fprintf(fsvg, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
    fprintf(fsvg, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n");
    fprintf(fsvg, "   \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
    fprintf(fsvg, "<svg width=\"%d\" height=\"%d\" version=\"1.1\"\n", (int) ceil(symbol->vector->width), (int) ceil(symbol->vector->height));
    fprintf(fsvg, "   xmlns=\"http://www.w3.org/2000/svg\">\n");
    fprintf(fsvg, "   <desc>Zint Generated Symbol\n");
    fprintf(fsvg, "   </desc>\n");
    fprintf(fsvg, "\n   <g id=\"barcode\" fill=\"#%s\">\n", symbol->fgcolour);

    fprintf(fsvg, "      <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" fill=\"#%s\" />\n", (int) ceil(symbol->vector->width), (int) ceil(symbol->vector->height), symbol->bgcolour);

    rect = symbol->vector->rectangles;
    while (rect) {
        fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", rect->x, rect->y, rect->width, rect->height);
        rect = rect->next;
    }

    hex = symbol->vector->hexagons;
    while (hex) {
        radius = hex->diameter / 2.0;
        ay = hex->y + (1.0 * radius);
        by = hex->y + (0.5 * radius);
        cy = hex->y - (0.5 * radius);
        dy = hex->y - (1.0 * radius);
        ey = hex->y - (0.5 * radius);
        fy = hex->y + (0.5 * radius);
        ax = hex->x;
        bx = hex->x + (0.86 * radius);
        cx = hex->x + (0.86 * radius);
        dx = hex->x;
        ex = hex->x - (0.86 * radius);
        fx = hex->x - (0.86 * radius);
        fprintf(fsvg, "      <path d=\"M %.2f %.2f L %.2f %.2f L %.2f %.2f L %.2f %.2f L %.2f %.2f L %.2f %.2f Z\" />\n", ax, ay, bx, by, cx, cy, dx, dy, ex, ey, fx, fy);
        hex = hex->next;
    }

    circle = symbol->vector->circles;
    while (circle) {
        if (circle->colour) {
            fprintf(fsvg, "      <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"#%s\" />\n", circle->x, circle->y, circle->diameter / 2.0, symbol->bgcolour);
        } else {
            fprintf(fsvg, "      <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"#%s\" />\n", circle->x, circle->y, circle->diameter / 2.0, symbol->fgcolour);
        }
        circle = circle->next;
    }

    string = symbol->vector->strings;
    while (string) {
        fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", string->x, string->y);
        fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", string->fsize, symbol->fgcolour);
        make_html_friendly(string->text, html_string);
        fprintf(fsvg, "         %s\n", html_string);
        fprintf(fsvg, "      </text>\n");
        string = string->next;
    }

    fprintf(fsvg, "   </g>\n");
    fprintf(fsvg, "</svg>\n");

    if (symbol->output_options & BARCODE_STDOUT) {
        fflush(fsvg);
    } else {
        fclose(fsvg);
    }

    if (locale)
        setlocale(LC_ALL, locale);

    return error_number;
}
