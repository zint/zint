/*  library.c - external functions of libzint

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

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"
#include "eci.h"
#include "gs1.h"
#include "zfiletypes.h"

#define TECHNETIUM  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%"

/* It's assumed that int is at least 32 bits, the following will compile-time fail if not
 * https://stackoverflow.com/a/1980056 */
typedef int static_assert_int_at_least_32bits[CHAR_BIT != 8 || sizeof(int) < 4 ? -1 : 1];

/* Create and initialize a symbol structure */
struct zint_symbol *ZBarcode_Create() {
    struct zint_symbol *symbol;

    symbol = (struct zint_symbol *) malloc(sizeof(*symbol));
    if (!symbol) return NULL;

    memset(symbol, 0, sizeof(*symbol));

    symbol->symbology = BARCODE_CODE128;
    symbol->scale = 1.0f;
    strcpy(symbol->fgcolour, "000000");
    symbol->fgcolor = &symbol->fgcolour[0];
    strcpy(symbol->bgcolour, "ffffff");
    symbol->bgcolor = &symbol->bgcolour[0];
#ifdef NO_PNG
    strcpy(symbol->outfile, "out.gif");
#else
    strcpy(symbol->outfile, "out.png");
#endif
    symbol->option_1 = -1;
    symbol->show_hrt = 1; // Show human readable text
    symbol->fontsize = 8;
    symbol->input_mode = DATA_MODE;
    symbol->eci = 0; // Default 0 uses ECI 3
    symbol->dot_size = 4.0f / 5.0f;
    symbol->guard_descent = 5.0f;
    symbol->warn_level = WARN_DEFAULT;
    symbol->bitmap = NULL;
    symbol->alphamap = NULL;
    symbol->vector = NULL;

    return symbol;
}

INTERNAL void vector_free(struct zint_symbol *symbol); /* Free vector structures */

/* Free any output buffers that may have been created and initialize output fields */
void ZBarcode_Clear(struct zint_symbol *symbol) {
    int i, j;

    if (!symbol) return;

    for (i = 0; i < symbol->rows; i++) {
        for (j = 0; j < symbol->width; j++) {
            unset_module(symbol, i, j);
        }
    }
    symbol->rows = 0;
    symbol->width = 0;
    memset(symbol->row_height, 0, sizeof(symbol->row_height));
    memset(symbol->text, 0, sizeof(symbol->text));
    symbol->errtxt[0] = '\0';
    if (symbol->bitmap != NULL) {
        free(symbol->bitmap);
        symbol->bitmap = NULL;
    }
    if (symbol->alphamap != NULL) {
        free(symbol->alphamap);
        symbol->alphamap = NULL;
    }
    symbol->bitmap_width = 0;
    symbol->bitmap_height = 0;
    symbol->bitmap_byte_length = 0;

    // If there is a rendered version, ensure its memory is released
    vector_free(symbol);
}

/* Free a symbol structure, including any output buffers */
void ZBarcode_Delete(struct zint_symbol *symbol) {
    if (!symbol) return;

    if (symbol->bitmap != NULL)
        free(symbol->bitmap);
    if (symbol->alphamap != NULL)
        free(symbol->alphamap);

    // If there is a rendered version, ensure its memory is released
    vector_free(symbol);

    free(symbol);
}

INTERNAL int eanx(struct zint_symbol *symbol, unsigned char source[], int length); /* EAN system barcodes */
INTERNAL int code39(struct zint_symbol *symbol, unsigned char source[], int length); /* Code 3 from 9 (or Code 39) */
INTERNAL int pzn(struct zint_symbol *symbol, unsigned char source[], int length); /* Pharmazentral Nummer (PZN) */
/* Extended Code 3 from 9 (or Code 39+) */
INTERNAL int excode39(struct zint_symbol *symbol, unsigned char source[], int length);
/* Codabar - a simple substitution cipher */
INTERNAL int codabar(struct zint_symbol *symbol, unsigned char source[], int length);
/* Code 2 of 5 Standard (& Matrix) */
INTERNAL int c25standard(struct zint_symbol *symbol, unsigned char source[], int length);
INTERNAL int c25ind(struct zint_symbol *symbol, unsigned char source[], int length); /* Code 2 of 5 Industrial */
INTERNAL int c25iata(struct zint_symbol *symbol, unsigned char source[], int length); /* Code 2 of 5 IATA */
INTERNAL int c25inter(struct zint_symbol *symbol, unsigned char source[], int length); /* Code 2 of 5 Interleaved */
INTERNAL int c25logic(struct zint_symbol *symbol, unsigned char source[], int length); /* Code 2 of 5 Data Logic */
INTERNAL int itf14(struct zint_symbol *symbol, unsigned char source[], int length); /* ITF-14 */
INTERNAL int dpleit(struct zint_symbol *symbol, unsigned char source[], int length); /* Deutsche Post Leitcode */
INTERNAL int dpident(struct zint_symbol *symbol, unsigned char source[], int length); /* Deutsche Post Identcode */
/* Code 93 - a re-working of Code 39+, generates 2 check digits */
INTERNAL int code93(struct zint_symbol *symbol, unsigned char source[], int length);
INTERNAL int code128(struct zint_symbol *symbol, unsigned char source[], int length); /* Code 128 and NVE-18 */
INTERNAL int gs1_128(struct zint_symbol *symbol, unsigned char source[], int length); /* EAN-128 (GS1-128) */
INTERNAL int code11(struct zint_symbol *symbol, unsigned char source[], int length); /* Code 11 */
INTERNAL int msi_plessey(struct zint_symbol *symbol, unsigned char source[], int length); /* MSI Plessey */
INTERNAL int telepen(struct zint_symbol *symbol, unsigned char source[], int length); /* Telepen ASCII */
INTERNAL int telepen_num(struct zint_symbol *symbol, unsigned char source[], int length); /* Telepen Numeric */
INTERNAL int plessey(struct zint_symbol *symbol, unsigned char source[], int length); /* Plessey Code */
INTERNAL int pharma(struct zint_symbol *symbol, unsigned char source[], int length); /* Pharmacode One Track */
INTERNAL int flat(struct zint_symbol *symbol, unsigned char source[], int length); /* Flattermarken */
INTERNAL int fim(struct zint_symbol *symbol, unsigned char source[], int length); /* Facing Identification Mark */
INTERNAL int pharma_two(struct zint_symbol *symbol, unsigned char source[], int length); /* Pharmacode Two Track */
INTERNAL int postnet(struct zint_symbol *symbol, unsigned char source[], int length); /* Postnet */
INTERNAL int planet(struct zint_symbol *symbol, unsigned char source[], int length); /* PLANET */
/* Intelligent Mail (aka USPS OneCode) */
INTERNAL int usps_imail(struct zint_symbol *symbol, unsigned char source[], int length);
INTERNAL int rm4scc(struct zint_symbol *symbol, unsigned char source[], int length); /* RM4SCC */
INTERNAL int auspost(struct zint_symbol *symbol, unsigned char source[], int length); /* Australia Post 4-state */
INTERNAL int code16k(struct zint_symbol *symbol, unsigned char source[], int length); /* Code 16k */
INTERNAL int pdf417(struct zint_symbol *symbol, unsigned char source[], int length); /* PDF417 */
INTERNAL int micropdf417(struct zint_symbol *symbol, unsigned char chaine[], int length); /* Micro PDF417 */
INTERNAL int maxicode(struct zint_symbol *symbol, unsigned char source[], int length); /* Maxicode */
INTERNAL int dbar_omn(struct zint_symbol *symbol, unsigned char source[], int length); /* RSS-14 */
INTERNAL int dbar_ltd(struct zint_symbol *symbol, unsigned char source[], int length); /* RSS Limited */
INTERNAL int dbar_exp(struct zint_symbol *symbol, unsigned char source[], int length); /* RSS Expanded */
INTERNAL int composite(struct zint_symbol *symbol, unsigned char source[], int length); /* Composite Symbology */
INTERNAL int kix(struct zint_symbol *symbol, unsigned char source[], int length); /* TNT KIX Code */
INTERNAL int aztec(struct zint_symbol *symbol, unsigned char source[], int length); /* Aztec Code */
INTERNAL int code32(struct zint_symbol *symbol, unsigned char source[], int length); /* Italian Pharmacode */
INTERNAL int daft(struct zint_symbol *symbol, unsigned char source[], int length); /* DAFT Code */
INTERNAL int ean14(struct zint_symbol *symbol, unsigned char source[], int length); /* EAN-14 */
INTERNAL int nve18(struct zint_symbol *symbol, unsigned char source[], int length); /* NVE-18 */
INTERNAL int microqr(struct zint_symbol *symbol, unsigned char source[], int length); /* Micro QR Code */
INTERNAL int azrune(struct zint_symbol *symbol, unsigned char source[], int length); /* Aztec Runes */
INTERNAL int koreapost(struct zint_symbol *symbol, unsigned char source[], int length); /* Korea Post */
INTERNAL int japanpost(struct zint_symbol *symbol, unsigned char source[], int length); /* Japanese Post */
INTERNAL int code49(struct zint_symbol *symbol, unsigned char source[], int length); /* Code 49 */
INTERNAL int channel(struct zint_symbol *symbol, unsigned char source[], int length); /* Channel Code */
INTERNAL int codeone(struct zint_symbol *symbol, unsigned char source[], int length); /* Code One */
INTERNAL int gridmatrix(struct zint_symbol *symbol, unsigned char source[], int length); /* Grid Matrix */
INTERNAL int hanxin(struct zint_symbol *symbol, unsigned char source[], int length); /* Han Xin */
INTERNAL int dotcode(struct zint_symbol *symbol, unsigned char source[], int length); /* DotCode */
INTERNAL int codablockf(struct zint_symbol *symbol, unsigned char source[], int length); /* Codablock */
INTERNAL int upnqr(struct zint_symbol *symbol, unsigned char source[], int length); /* UPNQR */
INTERNAL int qrcode(struct zint_symbol *symbol, unsigned char source[], int length); /* QR Code */
INTERNAL int datamatrix(struct zint_symbol *symbol, unsigned char source[], int length); /* Data Matrix (IEC16022) */
/* VIN Code (Vehicle Identification Number) */
INTERNAL int vin(struct zint_symbol *symbol, unsigned char source[], int length);
/* Royal Mail 4-state Mailmark */
INTERNAL int mailmark(struct zint_symbol *symbol, unsigned char source[], int length);
INTERNAL int ultra(struct zint_symbol *symbol, unsigned char source[], int length); /* Ultracode */
INTERNAL int rmqr(struct zint_symbol *symbol, unsigned char source[], int length); /* rMQR */
INTERNAL int dpd(struct zint_symbol *symbol, unsigned char source[], int length); /* DPD Code */

INTERNAL int plot_raster(struct zint_symbol *symbol, int rotate_angle, int file_type); /* Plot to PNG/BMP/PCX */
INTERNAL int plot_vector(struct zint_symbol *symbol, int rotate_angle, int file_type); /* Plot to EPS/EMF/SVG */

/* Prefix error message with Error/Warning */
STATIC_UNLESS_ZINT_TEST int error_tag(struct zint_symbol *symbol, int error_number, const char *error_string) {

    if (error_number != 0) {
        static const char *error_fmt = "Error %.93s"; /* Truncate if too long */
        static const char *warn_fmt = "Warning %.91s"; /* Truncate if too long */
        const char *fmt = error_number >= ZINT_ERROR ? error_fmt : warn_fmt;
        char error_buffer[100];

        if (error_number < ZINT_ERROR && symbol->warn_level == WARN_FAIL_ALL) {
            /* Convert to error equivalent */
            if (error_number == ZINT_WARN_NONCOMPLIANT) {
                error_number = ZINT_ERROR_NONCOMPLIANT;
            } else if (error_number == ZINT_WARN_USES_ECI) {
                error_number = ZINT_ERROR_USES_ECI;
            } else { /* ZINT_WARN_INVALID_OPTION */
                error_number = ZINT_ERROR_INVALID_OPTION;
            }
            fmt = error_fmt;
        }
        sprintf(error_buffer, fmt, error_string ? error_string : symbol->errtxt);
        strcpy(symbol->errtxt, error_buffer);
    }

    return error_number;
}

/* Output a hexadecimal representation of the rendered symbol */
static int dump_plot(struct zint_symbol *symbol) {
    FILE *f;
    int i, r;
    char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8',
        '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    int space = 0;

    if (symbol->output_options & BARCODE_STDOUT) {
        f = stdout;
    } else {
        f = fopen(symbol->outfile, "w");
        if (!f) {
            strcpy(symbol->errtxt, "201: Could not open output file");
            return ZINT_ERROR_FILE_ACCESS;
        }
    }

    for (r = 0; r < symbol->rows; r++) {
        int byt = 0;
        for (i = 0; i < symbol->width; i++) {
            byt = byt << 1;
            if (symbol->symbology == BARCODE_ULTRA) {
                if (module_colour_is_set(symbol, r, i)) {
                    byt += 1;
                }
            } else {
                if (module_is_set(symbol, r, i)) {
                    byt += 1;
                }
            }
            if (((i + 1) % 4) == 0) {
                fputc(hex[byt], f);
                space++;
                byt = 0;
            }
            if (space == 2 && i + 1 < symbol->width) {
                fputc(' ', f);
                space = 0;
            }
        }

        if ((symbol->width % 4) != 0) {
            byt = byt << (4 - (symbol->width % 4));
            fputc(hex[byt], f);
        }
        fputs("\n", f);
        space = 0;
    }

    if (symbol->output_options & BARCODE_STDOUT) {
        fflush(f);
    } else {
        fclose(f);
    }

    return 0;
}

/* Process health industry bar code data */
static int hibc(struct zint_symbol *symbol, unsigned char source[], int length) {
    int i;
    int counter, error_number = 0;
    char to_process[113], check_digit;

    /* without "+" and check: max 110 characters in HIBC 2.6 */
    if (length > 110) {
        strcpy(symbol->errtxt, "202: Data too long for HIBC LIC (110 character maximum)");
        return ZINT_ERROR_TOO_LONG;
    }
    to_upper(source);
    if (is_sane(TECHNETIUM, source, length) != 0) {
        strcpy(symbol->errtxt, "203: Invalid character in data (alphanumerics, space and \"-.$/+%\" only)");
        return ZINT_ERROR_INVALID_DATA;
    }

    counter = 41;
    for (i = 0; i < length; i++) {
        counter += posn(TECHNETIUM, source[i]);
    }
    counter = counter % 43;

    if (counter < 10) {
        check_digit = itoc(counter);
    } else {
        if (counter < 36) {
            check_digit = (counter - 10) + 'A';
        } else {
            switch (counter) {
                case 36: check_digit = '-';
                    break;
                case 37: check_digit = '.';
                    break;
                case 38: check_digit = ' ';
                    break;
                case 39: check_digit = '$';
                    break;
                case 40: check_digit = '/';
                    break;
                case 41: check_digit = '+';
                    break;
                case 42: check_digit = '%';
                    break;
                default: check_digit = ' ';
                    break; /* Keep compiler happy */
            }
        }
    }

    to_process[0] = '+';
    memcpy(to_process + 1, source, length);
    to_process[length + 1] = check_digit;
    length += 2;
    to_process[length] = '\0';

    switch (symbol->symbology) {
        case BARCODE_HIBC_128:
            error_number = code128(symbol, (unsigned char *) to_process, length);
            ustrcpy(symbol->text, "*");
            ustrcat(symbol->text, to_process);
            ustrcat(symbol->text, "*");
            break;
        case BARCODE_HIBC_39:
            symbol->option_2 = 0;
            error_number = code39(symbol, (unsigned char *) to_process, length);
            ustrcpy(symbol->text, "*");
            ustrcat(symbol->text, to_process);
            ustrcat(symbol->text, "*");
            break;
        case BARCODE_HIBC_DM:
            error_number = datamatrix(symbol, (unsigned char *) to_process, length);
            break;
        case BARCODE_HIBC_QR:
            error_number = qrcode(symbol, (unsigned char *) to_process, length);
            break;
        case BARCODE_HIBC_PDF:
            error_number = pdf417(symbol, (unsigned char *) to_process, length);
            break;
        case BARCODE_HIBC_MICPDF:
            error_number = micropdf417(symbol, (unsigned char *) to_process, length);
            break;
        case BARCODE_HIBC_AZTEC:
            error_number = aztec(symbol, (unsigned char *) to_process, length);
            break;
        case BARCODE_HIBC_BLOCKF:
            error_number = codablockf(symbol, (unsigned char *) to_process, length);
            break;
    }

    return error_number;
}

static int check_force_gs1(const int symbology) {
    /* Returns 1 if symbology MUST have GS1 data */

    switch (symbology) {
        case BARCODE_GS1_128:
        case BARCODE_EAN14:
        case BARCODE_NVE18:
        case BARCODE_DBAR_EXP:
        case BARCODE_DBAR_EXPSTK:
            return 1;
            break;
    }

    return is_composite(symbology);
}

static int gs1_compliant(const int symbology) {
    /* Returns 1 if symbology supports GS1 data */

    switch (symbology) {
        case BARCODE_CODE16K:
        case BARCODE_AZTEC:
        case BARCODE_DATAMATRIX:
        case BARCODE_CODE49:
        case BARCODE_QRCODE:
        case BARCODE_DOTCODE:
        case BARCODE_CODEONE:
        case BARCODE_ULTRA:
        case BARCODE_RMQR:
        // TODO: case BARCODE_CODABLOCKF:
        // TODO: case BARCODE_HANXIN:
        // TODO: case BARCODE_GRIDMATRIX:
            return 1;
            break;
    }

    return check_force_gs1(symbology);
}

static int is_dotty(const int symbology) {
    /* Returns 1 if symbology is a matrix design renderable as dots */

    switch (symbology) {
        /* Note MAXICODE and ULTRA absent */
        case BARCODE_QRCODE:
        case BARCODE_DATAMATRIX:
        case BARCODE_MICROQR:
        case BARCODE_HIBC_DM:
        case BARCODE_AZTEC:
        case BARCODE_HIBC_QR:
        case BARCODE_HIBC_AZTEC:
        case BARCODE_AZRUNE:
        case BARCODE_CODEONE:
        case BARCODE_GRIDMATRIX:
        case BARCODE_HANXIN:
        case BARCODE_DOTCODE:
        case BARCODE_UPNQR:
        case BARCODE_RMQR:
            return 1;
            break;
    }

    return 0;
}

static int is_fixed_ratio(const int symbology) {
    /* Returns 1 if symbology has fixed aspect ratio (matrix design) */

    if (is_dotty(symbology)) {
        return 1;
    }

    switch (symbology) {
        case BARCODE_MAXICODE:
        case BARCODE_ULTRA:
            return 1;
            break;
    }

    return 0;
}

static int supports_eci(const int symbology) {
    /* Returns 1 if symbology can encode the ECI character */

    switch (symbology) {
        case BARCODE_AZTEC:
        case BARCODE_DATAMATRIX:
        case BARCODE_MAXICODE:
        case BARCODE_MICROPDF417:
        case BARCODE_PDF417:
        case BARCODE_PDF417COMP:
        case BARCODE_QRCODE:
        case BARCODE_DOTCODE:
        case BARCODE_CODEONE:
        case BARCODE_GRIDMATRIX:
        case BARCODE_HANXIN:
        case BARCODE_ULTRA:
        case BARCODE_RMQR:
            return 1;
            break;
    }

    return 0;
}

static int has_hrt(const int symbology) {
    /* Returns 1 if symbology supports HRT */

    if (is_fixed_ratio(symbology)) {
        return 0;
    }

    switch (symbology) { /* These don't support HRT */
        case BARCODE_CODE16K:
        case BARCODE_CODE49:
        case BARCODE_FLAT:
        case BARCODE_POSTNET:
        case BARCODE_FIM:
        case BARCODE_PHARMA:
        case BARCODE_PHARMA_TWO:
        case BARCODE_PDF417:
        case BARCODE_PDF417COMP:
        case BARCODE_AUSPOST:
        case BARCODE_AUSREPLY:
        case BARCODE_AUSROUTE:
        case BARCODE_AUSREDIRECT:
        case BARCODE_RM4SCC:
        case BARCODE_CODABLOCKF:
        case BARCODE_JAPANPOST:
        case BARCODE_DBAR_STK:
        case BARCODE_DBAR_OMNSTK:
        case BARCODE_DBAR_EXPSTK:
        case BARCODE_PLANET:
        case BARCODE_MICROPDF417:
        case BARCODE_USPS_IMAIL:
        case BARCODE_KIX:
        case BARCODE_DAFT:
        case BARCODE_HIBC_PDF:
        case BARCODE_HIBC_MICPDF:
        case BARCODE_HIBC_BLOCKF:
        case BARCODE_MAILMARK:
        case BARCODE_DBAR_STK_CC:
        case BARCODE_DBAR_OMNSTK_CC:
        case BARCODE_DBAR_EXPSTK_CC:
            return 0;
            break;
    }

    return 1;
}

/* Used for dispatching barcodes and for whether symbol id valid */
typedef int (*barcode_func_t)(struct zint_symbol *, unsigned char *, int);
static const barcode_func_t barcode_funcs[146] = {
          NULL,      code11, c25standard,    c25inter,     c25iata, /*0-4*/
          NULL,    c25logic,      c25ind,      code39,    excode39, /*5-9*/
          NULL,        NULL,        NULL,        eanx,        eanx, /*10-14*/
          NULL,     gs1_128,        NULL,     codabar,        NULL, /*15-19*/
       code128,      dpleit,     dpident,     code16k,      code49, /*20-24*/
        code93,        NULL,        NULL,        flat,    dbar_omn, /*25-29*/
      dbar_ltd,    dbar_exp,     telepen,        NULL,        eanx, /*30-34*/
          eanx,        NULL,        eanx,        eanx,        NULL, /*35-39*/
       postnet,        NULL,        NULL,        NULL,        NULL, /*40-44*/
          NULL,        NULL, msi_plessey,        NULL,         fim, /*45-49*/
        code39,      pharma,         pzn,  pharma_two,        NULL, /*50-54*/
        pdf417,      pdf417,    maxicode,      qrcode,        NULL, /*55-59*/
       code128,        NULL,        NULL,     auspost,        NULL, /*60-64*/
          NULL,     auspost,     auspost,     auspost,        eanx, /*65-69*/
        rm4scc,  datamatrix,       ean14,         vin,  codablockf, /*70-74*/
         nve18,   japanpost,   koreapost,        NULL,    dbar_omn, /*75-79*/
      dbar_omn,    dbar_exp,      planet,        NULL, micropdf417, /*80-84*/
    usps_imail,     plessey, telepen_num,        NULL,       itf14, /*85-89*/
           kix,        NULL,       aztec,        daft,        NULL, /*90-94*/
          NULL,         dpd,     microqr,        hibc,        hibc, /*95-99*/
          NULL,        NULL,        hibc,        NULL,        hibc, /*100-104*/
          NULL,        hibc,        NULL,        hibc,        NULL, /*105-109*/
          hibc,        NULL,        hibc,        NULL,        NULL, /*110-114*/
       dotcode,      hanxin,        NULL,        NULL,        NULL, /*115-119*/
          NULL,    mailmark,        NULL,        NULL,        NULL, /*120-124*/
          NULL,        NULL,        NULL,      azrune,      code32, /*125-129*/
     composite,   composite,   composite,   composite,   composite, /*130-134*/
     composite,   composite,   composite,   composite,   composite, /*135-139*/
       channel,     codeone,  gridmatrix,       upnqr,       ultra, /*140-144*/
          rmqr,
};

static int reduced_charset(struct zint_symbol *symbol, unsigned char *source, int length);

static int extended_or_reduced_charset(struct zint_symbol *symbol, unsigned char *source, const int length) {
    int error_number = 0;

    switch (symbol->symbology) {
        /* These are the "elite" standards which have support for specific character sets */
        case BARCODE_QRCODE:
        case BARCODE_MICROQR:
        case BARCODE_GRIDMATRIX:
        case BARCODE_HANXIN:
        case BARCODE_UPNQR:
        case BARCODE_RMQR:
            error_number = (*barcode_funcs[symbol->symbology])(symbol, source, length);
            break;
        default: error_number = reduced_charset(symbol, source, length);
            break;
    }

    return error_number;
}

/* These are the "norm" standards which only support Latin-1 at most, though a few support ECI */
static int reduced_charset(struct zint_symbol *symbol, unsigned char *source, int length) {
    int error_number = 0;
    unsigned char *preprocessed = source;

    int eci_length = get_eci_length(symbol->eci, source, length);
#ifndef _MSC_VER
    unsigned char preprocessed_buf[eci_length + 1];
#else
    unsigned char *preprocessed_buf = (unsigned char *) _alloca(eci_length + 1);
#endif

    if ((symbol->input_mode & 0x07) == UNICODE_MODE && is_eci_convertible(symbol->eci)) {
        /* Prior check ensures ECI only set for those that support it */
        preprocessed = preprocessed_buf;
        error_number = utf8_to_eci(symbol->eci, source, preprocessed, &length);
        if (error_number != 0) {
            if (symbol->eci) {
                sprintf(symbol->errtxt, "244: Invalid character in input data for ECI %d", symbol->eci);
            } else {
                strcpy(symbol->errtxt, "204: Invalid character in input data (ISO/IEC 8859-1 only)");
            }
            return error_number;
        }
    }

    error_number = (*barcode_funcs[symbol->symbology])(symbol, preprocessed, length);

    return error_number;
}

STATIC_UNLESS_ZINT_TEST void strip_bom(unsigned char *source, int *input_length) {
    int i;

    /* Note if BOM is only data then not stripped */
    if (*input_length > 3 && (source[0] == 0xef) && (source[1] == 0xbb) && (source[2] == 0xbf)) {
        /* BOM at start of input data, strip in accordance with RFC 3629 */
        for (i = 3; i <= *input_length; i++) { /* Include terminating NUL */
            source[i - 3] = source[i];
        }
        *input_length -= 3;
    }
}

static int escape_char_process(struct zint_symbol *symbol, unsigned char *input_string, int *length) {
    int in_posn, out_posn;
    int hex1, hex2;
    int i, unicode;

#ifndef _MSC_VER
    unsigned char escaped_string[*length + 1];
#else
    unsigned char *escaped_string = (unsigned char *) _alloca(*length + 1);
#endif

    in_posn = 0;
    out_posn = 0;

    do {
        if (input_string[in_posn] == '\\') {
            if (in_posn + 1 >= *length) {
                strcpy(symbol->errtxt, "236: Incomplete escape character in input data");
                return ZINT_ERROR_INVALID_DATA;
            }
            switch (input_string[in_posn + 1]) {
                case '0': escaped_string[out_posn] = 0x00; /* Null */
                    in_posn += 2;
                    break;
                case 'E': escaped_string[out_posn] = 0x04; /* End of Transmission */
                    in_posn += 2;
                    break;
                case 'a': escaped_string[out_posn] = 0x07; /* Bell */
                    in_posn += 2;
                    break;
                case 'b': escaped_string[out_posn] = 0x08; /* Backspace */
                    in_posn += 2;
                    break;
                case 't': escaped_string[out_posn] = 0x09; /* Horizontal tab */
                    in_posn += 2;
                    break;
                case 'n': escaped_string[out_posn] = 0x0a; /* Line feed */
                    in_posn += 2;
                    break;
                case 'v': escaped_string[out_posn] = 0x0b; /* Vertical tab */
                    in_posn += 2;
                    break;
                case 'f': escaped_string[out_posn] = 0x0c; /* Form feed */
                    in_posn += 2;
                    break;
                case 'r': escaped_string[out_posn] = 0x0d; /* Carriage return */
                    in_posn += 2;
                    break;
                case 'e': escaped_string[out_posn] = 0x1b; /* Escape */
                    in_posn += 2;
                    break;
                case 'G': escaped_string[out_posn] = 0x1d; /* Group Separator */
                    in_posn += 2;
                    break;
                case 'R': escaped_string[out_posn] = 0x1e; /* Record Separator */
                    in_posn += 2;
                    break;
                case 'x': if (in_posn + 4 > *length) {
                        strcpy(symbol->errtxt, "232: Incomplete escape character in input data");
                        return ZINT_ERROR_INVALID_DATA;
                    }
                    hex1 = ctoi(input_string[in_posn + 2]);
                    hex2 = ctoi(input_string[in_posn + 3]);
                    if ((hex1 >= 0) && (hex2 >= 0)) {
                        escaped_string[out_posn] = (hex1 << 4) + hex2;
                        in_posn += 4;
                    } else {
                        strcpy(symbol->errtxt, "233: Corrupt escape character in input data");
                        return ZINT_ERROR_INVALID_DATA;
                    }
                    break;
                case '\\': escaped_string[out_posn] = '\\';
                    in_posn += 2;
                    break;
                case 'u':
                    if (in_posn + 6 > *length) {
                        strcpy(symbol->errtxt, "209: Incomplete Unicode escape character in input data");
                        return ZINT_ERROR_INVALID_DATA;
                    }
                    unicode = 0;
                    for (i = 0; i < 4; i++) {
                        if (ctoi(input_string[in_posn + i + 2]) == -1) {
                            strcpy(symbol->errtxt, "211: Corrupt Unicode escape character in input data");
                            return ZINT_ERROR_INVALID_DATA;
                        }
                        unicode = unicode << 4;
                        unicode += ctoi(input_string[in_posn + i + 2]);
                    }
                    /* Exclude reversed BOM and surrogates */
                    if (unicode == 0xfffe || (unicode >= 0xd800 && unicode < 0xe000)) {
                        strcpy(symbol->errtxt, "246: Invalid Unicode BMP escape character in input data");
                        return ZINT_ERROR_INVALID_DATA;
                    }
                    if (unicode >= 0x800) {
                        escaped_string[out_posn] = 0xe0 + ((unicode & 0xf000) >> 12);
                        out_posn++;
                        escaped_string[out_posn] = 0x80 + ((unicode & 0x0fc0) >> 6);
                        out_posn++;
                        escaped_string[out_posn] = 0x80 + (unicode & 0x003f);
                    } else if (unicode >= 0x80) {
                        escaped_string[out_posn] = 0xc0 + ((unicode & 0x07c0) >> 6);
                        out_posn++;
                        escaped_string[out_posn] = 0x80 + (unicode & 0x003f);
                    } else {
                        escaped_string[out_posn] = unicode & 0x7f;
                    }
                    in_posn += 6;
                    break;
                default: strcpy(symbol->errtxt, "234: Unrecognised escape character in input data");
                    return ZINT_ERROR_INVALID_DATA;
                    break;
            }
        } else {
            escaped_string[out_posn] = input_string[in_posn];
            in_posn++;
        }
        out_posn++;
    } while (in_posn < *length);

    memcpy(input_string, escaped_string, out_posn);
    input_string[out_posn] = '\0';
    *length = out_posn;

    return 0;
}

/* Encode a barcode. If `length` is 0, `source` must be NUL-terminated. */
int ZBarcode_Encode(struct zint_symbol *symbol, const unsigned char *source, int length) {
    int error_number, warn_number;
#ifdef _MSC_VER
    unsigned char *local_source;
#endif

    if (!symbol) return ZINT_ERROR_INVALID_DATA;

    if (symbol->debug & ZINT_DEBUG_PRINT) {
        printf("ZBarcode_Encode: symbology: %d, input_mode: 0x%X, ECI: %d, option_1: %d, option_2: %d,"
                " option_3: %d, scale: %g\n    output_options: 0x%X, fg: %s, bg: %s,"
                " length: %d, First 10 source: \"%.*s\", First 10 primary: \"%.10s\"\n",
                symbol->symbology, symbol->input_mode, symbol->eci, symbol->option_1, symbol->option_2,
                symbol->option_3, symbol->scale, symbol->output_options, symbol->fgcolour, symbol->bgcolour,
                length, length < 10 ? length : 10, source ? (const char *) source : "<NULL>", symbol->primary);
    }

    warn_number = 0;

    if (source == NULL) {
        return error_tag(symbol, ZINT_ERROR_INVALID_DATA, "200: Input data NULL");
    }
    if (length <= 0) {
        length = (int) ustrlen(source);
    }
    if (length <= 0) {
        return error_tag(symbol, ZINT_ERROR_INVALID_DATA, "205: No input data");
    }
    if (length > ZINT_MAX_DATA_LEN) {
        return error_tag(symbol, ZINT_ERROR_TOO_LONG, "243: Input data too long");
    }

    /* First check the symbology field */
    if (!ZBarcode_ValidID(symbol->symbology)) {
        int orig_symbology = symbol->symbology; /* For self-check */
        if (symbol->symbology < 1) {
            warn_number = error_tag(symbol, ZINT_WARN_INVALID_OPTION, "206: Symbology out of range");
            if (warn_number >= ZINT_ERROR) {
                return warn_number;
            }
            symbol->symbology = BARCODE_CODE128;
        /* symbol->symbologys 1 to 126 are defined by tbarcode */
        } else if (symbol->symbology == 5) {
            symbol->symbology = BARCODE_C25STANDARD;
        } else if ((symbol->symbology >= 10) && (symbol->symbology <= 12)) {
            symbol->symbology = BARCODE_EANX;
        } else if (symbol->symbology == 15) {
            symbol->symbology = BARCODE_EANX;
        } else if (symbol->symbology == 17) {
            symbol->symbology = BARCODE_UPCA;
        } else if (symbol->symbology == 19) {
            warn_number = error_tag(symbol, ZINT_WARN_INVALID_OPTION, "207: Codabar 18 not supported");
            if (warn_number >= ZINT_ERROR) {
                return warn_number;
            }
            symbol->symbology = BARCODE_CODABAR;
        } else if (symbol->symbology == 26) { /* UPC-A up to tbarcode 9, ISSN for tbarcode 10+ */
            symbol->symbology = BARCODE_UPCA;
        } else if (symbol->symbology == 27) { /* UPCD1 up to tbarcode 9, ISSN + 2 digit add-on for tbarcode 10+ */
            return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "208: UPCD1 not supported");
        } else if (symbol->symbology == 33) {
            symbol->symbology = BARCODE_GS1_128;
        } else if (symbol->symbology == 36) {
            symbol->symbology = BARCODE_UPCA;
        } else if (symbol->symbology == 39) {
            symbol->symbology = BARCODE_UPCE;
        } else if ((symbol->symbology >= 41) && (symbol->symbology <= 45)) {
            symbol->symbology = BARCODE_POSTNET;
        } else if (symbol->symbology == 46) {
            symbol->symbology = BARCODE_PLESSEY;
        } else if (symbol->symbology == 48) {
            symbol->symbology = BARCODE_NVE18;
        } else if (symbol->symbology == 54) { /* General Parcel up to tbarcode 9, Brazilian CEPNet for tbarcode 10+ */
            warn_number = error_tag(symbol, ZINT_WARN_INVALID_OPTION, "210: General Parcel Code not supported");
            if (warn_number >= ZINT_ERROR) {
                return warn_number;
            }
            symbol->symbology = BARCODE_CODE128;
        } else if ((symbol->symbology == 59) || (symbol->symbology == 61)) {
            symbol->symbology = BARCODE_CODE128;
        } else if (symbol->symbology == 62) {
            symbol->symbology = BARCODE_CODE93;
        } else if ((symbol->symbology == 64) || (symbol->symbology == 65)) {
            symbol->symbology = BARCODE_AUSPOST;
        } else if (symbol->symbology == 78) {
            symbol->symbology = BARCODE_DBAR_OMN;
        } else if (symbol->symbology == 83) {
            symbol->symbology = BARCODE_PLANET;
        } else if (symbol->symbology == 88) {
            symbol->symbology = BARCODE_GS1_128;
        } else if (symbol->symbology == 91) { /* BC412 up to tbarcode 9, Code 32 for tbarcode 10+ */
            warn_number = error_tag(symbol, ZINT_WARN_INVALID_OPTION, "212: Symbology out of range");
            if (warn_number >= ZINT_ERROR) {
                return warn_number;
            }
            symbol->symbology = BARCODE_CODE128;
        } else if ((symbol->symbology >= 94) && (symbol->symbology <= 95)) {
            warn_number = error_tag(symbol, ZINT_WARN_INVALID_OPTION, "213: Symbology out of range");
            if (warn_number >= ZINT_ERROR) {
                return warn_number;
            }
            symbol->symbology = BARCODE_CODE128;
        } else if (symbol->symbology == 100) {
            symbol->symbology = BARCODE_HIBC_128;
        } else if (symbol->symbology == 101) {
            symbol->symbology = BARCODE_HIBC_39;
        } else if (symbol->symbology == 103) {
            symbol->symbology = BARCODE_HIBC_DM;
        } else if (symbol->symbology == 105) {
            symbol->symbology = BARCODE_HIBC_QR;
        } else if (symbol->symbology == 107) {
            symbol->symbology = BARCODE_HIBC_PDF;
        } else if (symbol->symbology == 109) {
            symbol->symbology = BARCODE_HIBC_MICPDF;
        } else if (symbol->symbology == 111) {
            symbol->symbology = BARCODE_HIBC_BLOCKF;
        } else if ((symbol->symbology == 113) || (symbol->symbology == 114)) {
            warn_number = error_tag(symbol, ZINT_WARN_INVALID_OPTION, "214: Symbology out of range");
            if (warn_number >= ZINT_ERROR) {
                return warn_number;
            }
            symbol->symbology = BARCODE_CODE128;
        } else if ((symbol->symbology >= 117) && (symbol->symbology <= 127)) {
            if (symbol->symbology != 121) { /* BARCODE_MAILMARK */
                warn_number = error_tag(symbol, ZINT_WARN_INVALID_OPTION, "215: Symbology out of range");
                if (warn_number >= ZINT_ERROR) {
                    return warn_number;
                }
                symbol->symbology = BARCODE_CODE128;
            }
        /* Everything from 128 up is Zint-specific */
        } else if (symbol->symbology > 145) {
            warn_number = error_tag(symbol, ZINT_WARN_INVALID_OPTION, "216: Symbology out of range");
            if (warn_number >= ZINT_ERROR) {
                return warn_number;
            }
            symbol->symbology = BARCODE_CODE128;
        }
        if (symbol->symbology == orig_symbology) { /* Should never happen */
            return error_tag(symbol, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error"); /* Not reached */
        }
    }

    if (symbol->eci != 0) {
        if (!(supports_eci(symbol->symbology))) {
            return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "217: Symbology does not support ECI switching");
        }
        if ((symbol->eci < 0) || (symbol->eci == 1) || (symbol->eci == 2) || (symbol->eci > 999999)) {
            return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "218: Invalid ECI mode");
        }
    }

    if ((symbol->scale < 0.01f) || (symbol->scale > 100.0f)) {
        return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "227: Scale out of range (0.01 to 100)");
    }
    if ((symbol->dot_size < 0.01f) || (symbol->dot_size > 20.0f)) {
        return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "221: Dot size out of range (0.01 to 20)");
    }

    if ((symbol->height < 0.0f) || (symbol->height > 2000.0f)) { /* Allow for 44 row CODABLOCKF at 45X each */
        return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "765: Height out of range (0 to 2000)");
    }
    if ((symbol->guard_descent < 0.0f) || (symbol->guard_descent > 50.0f)) {
        return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "769: Guard bar descent out of range (0 to 50)");
    }
    if ((symbol->whitespace_width < 0) || (symbol->whitespace_width > 100)) {
        return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "766: Whitespace width out of range (0 to 100)");
    }
    if ((symbol->whitespace_height < 0) || (symbol->whitespace_height > 100)) {
        return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "767: Whitespace height out of range (0 to 100)");
    }
    if ((symbol->border_width < 0) || (symbol->border_width > 100)) {
        return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "768: Border width out of range (0 to 100)");
    }

    if ((symbol->input_mode & 0x07) > 2) {
        symbol->input_mode = DATA_MODE; /* Reset completely TODO: in future, warn/error */
    }

    if ((symbol->input_mode & 0x07) == UNICODE_MODE && !is_valid_utf8(source, length)) {
        return error_tag(symbol, ZINT_ERROR_INVALID_DATA, "245: Invalid UTF-8 in input data");
    }

#ifndef _MSC_VER
    unsigned char local_source[length + 1];
#else
    local_source = (unsigned char *) _alloca(length + 1);
#endif

    memcpy(local_source, source, length);
    local_source[length] = '\0';

    /* Start acting on input mode */
    if (symbol->input_mode & ESCAPE_MODE) {
        error_number = escape_char_process(symbol, local_source, &length); /* Only returns errors, not warnings */
        if (error_number != 0) {
            return error_tag(symbol, error_number, NULL);
        }
    }

    if ((symbol->input_mode & 0x07) == UNICODE_MODE) {
        strip_bom(local_source, &length);
    }

    if (((symbol->input_mode & 0x07) == GS1_MODE) || (check_force_gs1(symbol->symbology))) {
        if (gs1_compliant(symbol->symbology) == 1) {
            // Reduce input for composite and non-forced symbologies, others (EAN128 and RSS_EXP based) will
            // handle it themselves
            if (is_composite(symbol->symbology) || !check_force_gs1(symbol->symbology)) {
#ifndef _MSC_VER
                unsigned char reduced[length + 1];
#else
                unsigned char *reduced = (unsigned char *) _alloca(length + 1);
#endif
                error_number = gs1_verify(symbol, local_source, length, reduced);
                if (error_number) {
                    static const char in_2d_comp[] = " in 2D component";
                    if (is_composite(symbol->symbology)
                            && strlen(symbol->errtxt) + strlen(in_2d_comp) < sizeof(symbol->errtxt)) {
                        strcat(symbol->errtxt, in_2d_comp);
                    }
                    error_number = error_tag(symbol, error_number, NULL);
                    if (error_number >= ZINT_ERROR) {
                        return error_number;
                    }
                    warn_number = error_number; /* Override any previous warning (errtxt has been overwritten) */
                }
                ustrcpy(local_source, reduced); // Cannot contain NUL char
                length = (int) ustrlen(local_source);
            }
        } else {
            return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "220: Selected symbology does not support GS1 mode");
        }
    }

    error_number = extended_or_reduced_charset(symbol, local_source, length);

    if ((error_number == ZINT_ERROR_INVALID_DATA) && symbol->eci == 0 && supports_eci(symbol->symbology)
            && (symbol->input_mode & 0x07) == UNICODE_MODE) {
        /* Try another ECI mode */
        symbol->eci = get_best_eci(local_source, length);
        if (symbol->eci != 0) {
            error_number = extended_or_reduced_charset(symbol, local_source, length);
            /* Inclusion of ECI more noteworthy than other warnings, so overwrite (if any) */
            if (error_number < ZINT_ERROR) {
                error_number = ZINT_WARN_USES_ECI;
                if (!(symbol->debug & ZINT_DEBUG_TEST)) {
                    sprintf(symbol->errtxt, "222: Encoded data includes ECI %d", symbol->eci);
                }
                if (symbol->debug & ZINT_DEBUG_PRINT) printf("Added ECI %d\n", symbol->eci);
            }
        }
    }

    if (error_number == 0) {
        error_number = warn_number; /* Already tagged */
    } else {
        error_number = error_tag(symbol, error_number, NULL);
    }

    if (error_number < ZINT_ERROR) {
        if (symbol->height < 0.5f) { /* Absolute minimum */
            (void) set_height(symbol, 0.0f, 50.0f, 0.0f, 1 /*no_errtxt*/);
        }
    }

    return error_number;
}

/* Output a previously encoded symbol to file `symbol->outfile` */
int ZBarcode_Print(struct zint_symbol *symbol, int rotate_angle) {
    int error_number;
    int len;

    if (!symbol) return ZINT_ERROR_INVALID_DATA;

    switch (rotate_angle) {
        case 0:
        case 90:
        case 180:
        case 270:
            break;
        default:
            return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "223: Invalid rotation angle");
            break;
    }

    if (symbol->output_options & BARCODE_DOTTY_MODE) {
        if (!(is_dotty(symbol->symbology))) {
            return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "224: Selected symbology cannot be rendered as dots");
        }
    }

    len = (int) strlen(symbol->outfile);
    if (len > 3) {
        char output[4];
        output[0] = symbol->outfile[len - 3];
        output[1] = symbol->outfile[len - 2];
        output[2] = symbol->outfile[len - 1];
        output[3] = '\0';
        to_upper((unsigned char *) output);

        if (!(strcmp(output, "PNG"))) {
            error_number = plot_raster(symbol, rotate_angle, OUT_PNG_FILE);

        } else if (!(strcmp(output, "BMP"))) {
            error_number = plot_raster(symbol, rotate_angle, OUT_BMP_FILE);

        } else if (!(strcmp(output, "PCX"))) {
            error_number = plot_raster(symbol, rotate_angle, OUT_PCX_FILE);

        } else if (!(strcmp(output, "GIF"))) {
            error_number = plot_raster(symbol, rotate_angle, OUT_GIF_FILE);

        } else if (!(strcmp(output, "TIF"))) {
            error_number = plot_raster(symbol, rotate_angle, OUT_TIF_FILE);

        } else if (!(strcmp(output, "TXT"))) {
            error_number = dump_plot(symbol);

        } else if (!(strcmp(output, "EPS"))) {
            error_number = plot_vector(symbol, rotate_angle, OUT_EPS_FILE);

        } else if (!(strcmp(output, "SVG"))) {
            error_number = plot_vector(symbol, rotate_angle, OUT_SVG_FILE);

        } else if (!(strcmp(output, "EMF"))) {
            error_number = plot_vector(symbol, rotate_angle, OUT_EMF_FILE);

        } else {
            return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "225: Unknown output format");
        }
    } else {
        return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "226: Unknown output format");
    }

    return error_tag(symbol, error_number, NULL);
}

/* Output a previously encoded symbol to memory as raster (`symbol->bitmap`) */
int ZBarcode_Buffer(struct zint_symbol *symbol, int rotate_angle) {
    int error_number;

    if (!symbol) return ZINT_ERROR_INVALID_DATA;

    switch (rotate_angle) {
        case 0:
        case 90:
        case 180:
        case 270:
            break;
        default:
            return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "228: Invalid rotation angle");
            break;
    }

    if (symbol->output_options & BARCODE_DOTTY_MODE) {
        if (!(is_dotty(symbol->symbology))) {
            return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "237: Selected symbology cannot be rendered as dots");
        }
    }

    error_number = plot_raster(symbol, rotate_angle, OUT_BUFFER);
    return error_tag(symbol, error_number, NULL);
}

/* Output a previously encoded symbol to memory as vector (`symbol->vector`) */
int ZBarcode_Buffer_Vector(struct zint_symbol *symbol, int rotate_angle) {
    int error_number;

    if (!symbol) return ZINT_ERROR_INVALID_DATA;

    switch (rotate_angle) {
        case 0:
        case 90:
        case 180:
        case 270:
            break;
        default:
            return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "219: Invalid rotation angle");
            break;
    }

    if (symbol->output_options & BARCODE_DOTTY_MODE) {
        if (!(is_dotty(symbol->symbology))) {
            return error_tag(symbol, ZINT_ERROR_INVALID_OPTION, "238: Selected symbology cannot be rendered as dots");
        }
    }

    error_number = plot_vector(symbol, rotate_angle, OUT_BUFFER);
    return error_tag(symbol, error_number, NULL);
}

/* Encode and output a symbol to file `symbol->outfile` */
int ZBarcode_Encode_and_Print(struct zint_symbol *symbol, const unsigned char *source, int length, int rotate_angle) {
    int error_number;
    int first_err;

    error_number = ZBarcode_Encode(symbol, source, length);
    if (error_number >= ZINT_ERROR) {
        return error_number;
    }

    first_err = error_number;
    error_number = ZBarcode_Print(symbol, rotate_angle);
    if (error_number == 0) {
        error_number = first_err;
    }
    return error_number;
}

/* Encode and output a symbol to memory as raster (`symbol->bitmap`) */
int ZBarcode_Encode_and_Buffer(struct zint_symbol *symbol, const unsigned char *source, int length,
            int rotate_angle) {
    int error_number;
    int first_err;

    error_number = ZBarcode_Encode(symbol, source, length);
    if (error_number >= ZINT_ERROR) {
        return error_number;
    }

    first_err = error_number;
    error_number = ZBarcode_Buffer(symbol, rotate_angle);
    if (error_number == 0) {
        error_number = first_err;
    }

    return error_number;
}

/* Encode and output a symbol to memory as vector (`symbol->vector`) */
int ZBarcode_Encode_and_Buffer_Vector(struct zint_symbol *symbol, const unsigned char *source, int length,
            int rotate_angle) {
    int error_number;
    int first_err;

    error_number = ZBarcode_Encode(symbol, source, length);
    if (error_number >= ZINT_ERROR) {
        return error_number;
    }

    first_err = error_number;
    error_number = ZBarcode_Buffer_Vector(symbol, rotate_angle);
    if (error_number == 0) {
        error_number = first_err;
    }

    return error_number;
}

/* Encode a barcode using input data from file `filename` */
int ZBarcode_Encode_File(struct zint_symbol *symbol, const char *filename) {
    FILE *file;
    int file_opened = 0;
    unsigned char *buffer;
    long fileLen;
    size_t n;
    size_t nRead = 0;
    int ret;

    if (!symbol) return ZINT_ERROR_INVALID_DATA;

    if (!filename) {
        return error_tag(symbol, ZINT_ERROR_INVALID_DATA, "239: Filename NULL");
    }

    if (!strcmp(filename, "-")) {
        file = stdin;
        fileLen = ZINT_MAX_DATA_LEN;
    } else {
        file = fopen(filename, "rb");
        if (!file) {
            sprintf(symbol->errtxt, "229: Unable to read input file (%d: %.30s)", errno, strerror(errno));
            return error_tag(symbol, ZINT_ERROR_INVALID_DATA, NULL);
        }
        file_opened = 1;

        /* Get file length */
        fseek(file, 0, SEEK_END);
        fileLen = ftell(file);
        fseek(file, 0, SEEK_SET);

        /* On many Linux distros ftell() returns LONG_MAX not -1 on error */
        if (fileLen <= 0 || fileLen == LONG_MAX) {
            fclose(file);
            return error_tag(symbol, ZINT_ERROR_INVALID_DATA, "235: Input file empty or unseekable");
        }
        if (fileLen > ZINT_MAX_DATA_LEN) {
            fclose(file);
            return error_tag(symbol, ZINT_ERROR_TOO_LONG, "230: Input file too long");
        }
    }

    /* Allocate memory */
    buffer = (unsigned char *) malloc(fileLen);
    if (!buffer) {
        if (file_opened) {
            fclose(file);
        }
        return error_tag(symbol, ZINT_ERROR_MEMORY, "231: Insufficient memory for file read buffer");
    }

    /* Read file contents into buffer */

    do {
        n = fread(buffer + nRead, 1, fileLen - nRead, file);
        if (ferror(file)) {
            sprintf(symbol->errtxt, "241: Input file read error (%d: %.30s)", errno, strerror(errno));
            if (file_opened) {
                fclose(file);
            }
            free(buffer);
            return error_tag(symbol, ZINT_ERROR_INVALID_DATA, NULL);
        }
        nRead += n;
    } while (!feof(file) && (0 < n) && ((long) nRead < fileLen));

    if (file_opened) {
        fclose(file);
    }
    ret = ZBarcode_Encode(symbol, buffer, (int) nRead);
    free(buffer);
    return ret;
}

/* Encode a symbol using input data from file `filename` and output to file `symbol->outfile` */
int ZBarcode_Encode_File_and_Print(struct zint_symbol *symbol, const char *filename, int rotate_angle) {
    int error_number;
    int first_err;

    error_number = ZBarcode_Encode_File(symbol, filename);
    if (error_number >= ZINT_ERROR) {
        return error_number;
    }

    first_err = error_number;
    error_number = ZBarcode_Print(symbol, rotate_angle);
    if (error_number == 0) {
        error_number = first_err;
    }

    return error_number;
}

/* Encode a symbol using input data from file `filename` and output to memory as raster (`symbol->bitmap`) */
int ZBarcode_Encode_File_and_Buffer(struct zint_symbol *symbol, char const *filename, int rotate_angle) {
    int error_number;
    int first_err;

    error_number = ZBarcode_Encode_File(symbol, filename);
    if (error_number >= ZINT_ERROR) {
        return error_number;
    }

    first_err = error_number;
    error_number = ZBarcode_Buffer(symbol, rotate_angle);
    if (error_number == 0) {
        error_number = first_err;
    }

    return error_number;
}

/* Encode a symbol using input data from file `filename` and output to memory as vector (`symbol->vector`) */
int ZBarcode_Encode_File_and_Buffer_Vector(struct zint_symbol *symbol, const char *filename, int rotate_angle) {
    int error_number;
    int first_err;

    error_number = ZBarcode_Encode_File(symbol, filename);
    if (error_number >= ZINT_ERROR) {
        return error_number;
    }

    first_err = error_number;
    error_number = ZBarcode_Buffer_Vector(symbol, rotate_angle);
    if (error_number == 0) {
        error_number = first_err;
    }

    return error_number;
}

/* Checks whether a symbology is supported */
int ZBarcode_ValidID(int symbol_id) {

    if (symbol_id <= 0 || symbol_id > 145) {
        return 0;
    }

    return barcode_funcs[symbol_id] != NULL;
}

/* Return the capability flags for symbology `symbol_id` that match `cap_flag` */
unsigned int ZBarcode_Cap(int symbol_id, unsigned int cap_flag) {
    unsigned int result = 0;

    if (!ZBarcode_ValidID(symbol_id)) {
        return 0;
    }

    if ((cap_flag & ZINT_CAP_HRT) && has_hrt(symbol_id)) {
        result |= ZINT_CAP_HRT;
    }
    if ((cap_flag & ZINT_CAP_STACKABLE) && is_stackable(symbol_id)) {
        result |= ZINT_CAP_STACKABLE;
    }
    if ((cap_flag & ZINT_CAP_EXTENDABLE) && is_extendable(symbol_id)) {
        result |= ZINT_CAP_EXTENDABLE;
    }
    if ((cap_flag & ZINT_CAP_COMPOSITE) && is_composite(symbol_id)) {
        result |= ZINT_CAP_COMPOSITE;
    }
    if ((cap_flag & ZINT_CAP_ECI) && supports_eci(symbol_id)) {
        result |= ZINT_CAP_ECI;
    }
    if ((cap_flag & ZINT_CAP_GS1) && gs1_compliant(symbol_id)) {
        result |= ZINT_CAP_GS1;
    }
    if ((cap_flag & ZINT_CAP_DOTTY) && is_dotty(symbol_id)) {
        result |= ZINT_CAP_DOTTY;
    }
    if (cap_flag & ZINT_CAP_QUIET_ZONES) {
        switch (symbol_id) { /* See `quiet_zones()` in "output.c" */
            case BARCODE_CODE16K:
            case BARCODE_CODE49:
            case BARCODE_CODABLOCKF:
            case BARCODE_HIBC_BLOCKF:
            case BARCODE_ITF14:
            case BARCODE_EANX:
            case BARCODE_EANX_CHK:
            case BARCODE_EANX_CC:
            case BARCODE_ISBNX:
            case BARCODE_UPCA:
            case BARCODE_UPCA_CHK:
            case BARCODE_UPCA_CC:
            case BARCODE_UPCE:
            case BARCODE_UPCE_CHK:
            case BARCODE_UPCE_CC:
                result |= ZINT_CAP_QUIET_ZONES;
                break;
        }
    }
    if ((cap_flag & ZINT_CAP_FIXED_RATIO) && is_fixed_ratio(symbol_id)) {
        result |= ZINT_CAP_FIXED_RATIO;
    }
    if (cap_flag & ZINT_CAP_READER_INIT) {
        /* Note does not include HIBC versions */
        switch (symbol_id) {
            case BARCODE_CODE128: /* Note does not include GS1_128 or NVE18 */
            case BARCODE_CODE128B:
            case BARCODE_CODE16K:
            case BARCODE_CODABLOCKF:
            case BARCODE_PDF417:
            case BARCODE_PDF417COMP:
            case BARCODE_DATAMATRIX:
            case BARCODE_MICROPDF417:
            case BARCODE_AZTEC:
            case BARCODE_DOTCODE:
            case BARCODE_GRIDMATRIX:
            case BARCODE_ULTRA:
                result |= ZINT_CAP_READER_INIT;
                break;
        }
    }
    if (cap_flag & ZINT_CAP_FULL_MULTIBYTE) {
        switch (symbol_id) {
            case BARCODE_QRCODE:
            case BARCODE_MICROQR:
            //case BARCODE_HIBC_QR: Note character set restricted to ASCII subset
            //case BARCODE_UPNQR: Note does not use Kanji mode
            case BARCODE_RMQR:
            case BARCODE_HANXIN:
            case BARCODE_GRIDMATRIX:
                result |= ZINT_CAP_FULL_MULTIBYTE;
                break;
        }
    }
    if (cap_flag & ZINT_CAP_MASK) {
        switch (symbol_id) {
            case BARCODE_QRCODE:
            case BARCODE_MICROQR:
            case BARCODE_HANXIN:
            case BARCODE_DOTCODE:
                result |= ZINT_CAP_MASK;
                break;
        }
    }
    if (cap_flag & ZINT_CAP_STRUCTAPP) {
        switch (symbol_id) {
            case BARCODE_PDF417:
            case BARCODE_PDF417COMP:
            case BARCODE_MAXICODE:
            case BARCODE_QRCODE: /* Note does not include MICROQR, UPNQR or rMQR */
            case BARCODE_DATAMATRIX:
            case BARCODE_MICROPDF417:
            case BARCODE_AZTEC:
            case BARCODE_HIBC_DM:
            case BARCODE_HIBC_QR:
            case BARCODE_HIBC_PDF:
            case BARCODE_HIBC_MICPDF:
            case BARCODE_HIBC_AZTEC:
            case BARCODE_DOTCODE:
            case BARCODE_CODEONE:
            case BARCODE_GRIDMATRIX:
            case BARCODE_ULTRA:
                result |= ZINT_CAP_STRUCTAPP;
                break;
        }
    }
    if ((cap_flag & ZINT_CAP_COMPLIANT_HEIGHT) && !is_fixed_ratio(symbol_id)) {
        switch (symbol_id) {
            /* These don't have a compliant height defined */
            case BARCODE_CODE11: /* TODO: Find doc */
            case BARCODE_C25STANDARD: /* For C25 only have doc for C25INTER */
            case BARCODE_C25IATA:
            case BARCODE_C25LOGIC:
            case BARCODE_C25IND:
            case BARCODE_CODE128: /* Left to application */
            case BARCODE_CODE128B:
            case BARCODE_DPLEIT: /* TODO: Find doc */
            case BARCODE_DPIDENT: /* TODO: Find doc */
            case BARCODE_FLAT: /* TODO: Find doc */
            case BARCODE_MSI_PLESSEY: /* TODO: Find doc */
            case BARCODE_PDF417: /* Has compliant height but already warns & uses for default */
            case BARCODE_PDF417COMP:
            case BARCODE_VIN: /* Spec unlikely */
            case BARCODE_KOREAPOST: /* TODO: Find doc */
            case BARCODE_MICROPDF417: /* See PDF417 */
            case BARCODE_PLESSEY: /* TODO: Find doc */
            case BARCODE_DAFT: /* Generic */
            case BARCODE_HIBC_128: /* See CODE128 */
            case BARCODE_HIBC_PDF: /* See PDF417 */
            case BARCODE_HIBC_MICPDF: /* See PDF417 */
                break;
            default:
                result |= ZINT_CAP_COMPLIANT_HEIGHT;
                break;
        }
    }

    return result;
}

/* Return the version of Zint linked to */
int ZBarcode_Version() {
    if (ZINT_VERSION_BUILD) {
        return (ZINT_VERSION_MAJOR * 10000) + (ZINT_VERSION_MINOR * 100) + ZINT_VERSION_RELEASE * 10
                + ZINT_VERSION_BUILD;
    }
    return (ZINT_VERSION_MAJOR * 10000) + (ZINT_VERSION_MINOR * 100) + ZINT_VERSION_RELEASE;
}
