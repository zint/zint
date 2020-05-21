/*  output.c - Common routines for raster/vector

    libzint - the open source barcode library
    Copyright (C) 2020 Robin Stuart <rstuart114@gmail.com>

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

#include <string.h>
#include "common.h"
#include "output.h"

#define SSET "0123456789ABCDEF"

/* Check colour options are good. Note: using raster.c error nos 651-654 */
INTERNAL int check_colour_options(struct zint_symbol *symbol) {
    int error_number;

    if (strlen(symbol->fgcolour) != 6) {
        strcpy(symbol->errtxt, "651: Malformed foreground colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }
    if (strlen(symbol->bgcolour) != 6) {
        strcpy(symbol->errtxt, "652: Malformed background colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }

    to_upper((unsigned char *) symbol->fgcolour);
    to_upper((unsigned char *) symbol->bgcolour);

    error_number = is_sane(SSET, (unsigned char *) symbol->fgcolour, strlen(symbol->fgcolour));
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "653: Malformed foreground colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }

    error_number = is_sane(SSET, (unsigned char *) symbol->bgcolour, strlen(symbol->fgcolour));
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "654: Malformed background colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }

    return 0;
}

/* Return minimum quiet zones for each symbology */
static void quiet_zones(struct zint_symbol *symbol, int *left, int *right, int *top, int *bottom) {
    *left = *right = *top = *bottom = 0;

    switch (symbol->symbology) {
        case BARCODE_CODE16K:
            /* BS EN 12323:2005 Section 4.5 c) */
            *left = 10;
            *right = 1;
            break;
        case BARCODE_CODE49:
            /* ANSI/AIM BC6-2000 Section 2.4 */
            *left = 10;
            *right = 1;
            break;
        case BARCODE_CODABLOCKF:
        case BARCODE_HIBC_BLOCKF:
            /* AIM ISS-X-24 Section 4.6.1 */
            *left = 10;
            *right = 10;
            break;
        /* TODO: others */
    }
}

/* Set left (x), top (y), right and bottom offsets for whitespace */
INTERNAL void set_whitespace_offsets(struct zint_symbol *symbol, int *xoffset, int *yoffset, int *roffset, int *boffset) {
    int qz_left, qz_right, qz_top, qz_bottom;

    quiet_zones(symbol, &qz_left, &qz_right, &qz_top, &qz_bottom);

    *xoffset = symbol->whitespace_width + qz_left;
    *roffset = symbol->whitespace_width + qz_right;
    if (symbol->output_options & BARCODE_BOX) {
        *xoffset += symbol->border_width;
        *roffset += symbol->border_width;
    }

    *yoffset = qz_top;
    *boffset = qz_bottom;
    if (symbol->output_options & (BARCODE_BOX | BARCODE_BIND)) {
        *yoffset += symbol->border_width;
        *boffset += symbol->border_width;
    }
}
