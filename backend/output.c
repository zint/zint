/*  output.c - Common routines for raster/vector

    libzint - the open source barcode library
    Copyright (C) 2020 - 2021 Robin Stuart <rstuart114@gmail.com>

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

#include "common.h"
#include "output.h"

#define SSET "0123456789ABCDEF"

/* Check colour options are good. Note: using raster.c error nos 651-654 */
INTERNAL int output_check_colour_options(struct zint_symbol *symbol) {
    int error_number;

    if ((strlen(symbol->fgcolour) != 6) && (strlen(symbol->fgcolour) != 8)) {
        strcpy(symbol->errtxt, "651: Malformed foreground colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }
    if ((strlen(symbol->bgcolour) != 6) && (strlen(symbol->bgcolour) != 8)) {
        strcpy(symbol->errtxt, "652: Malformed background colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }

    to_upper((unsigned char *) symbol->fgcolour);
    to_upper((unsigned char *) symbol->bgcolour);

    error_number = is_sane(SSET, (unsigned char *) symbol->fgcolour, (int) strlen(symbol->fgcolour));
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "653: Malformed foreground colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }

    error_number = is_sane(SSET, (unsigned char *) symbol->bgcolour, (int) strlen(symbol->bgcolour));
    if (error_number == ZINT_ERROR_INVALID_DATA) {
        strcpy(symbol->errtxt, "654: Malformed background colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }

    return 0;
}

/* Return minimum quiet zones for each symbology */
static int quiet_zones(struct zint_symbol *symbol, int *left, int *right, int *top, int *bottom) {
    int done = 0;

    *left = *right = *top = *bottom = 0;

    /* These always have quiet zones set (previously used whitespace_width) */
    switch (symbol->symbology) {
        case BARCODE_CODE16K:
            /* BS EN 12323:2005 Section 4.5 (c) */
            *left = 10;
            *right = 1;
            done = 1;
            break;
        case BARCODE_CODE49:
            /* ANSI/AIM BC6-2000 Section 2.4 */
            *left = 10;
            *right = 1;
            done = 1;
            break;
        case BARCODE_CODABLOCKF:
        case BARCODE_HIBC_BLOCKF:
            /* AIM ISS-X-24 Section 4.6.1 */
            *left = 10;
            *right = 10;
            done = 1;
            break;
        case BARCODE_ITF14:
            /* GS1 General Specifications 21.0.1 Section 5.3.2.2 */
            *left = 10;
            *right = 10;
            done = 1;
            break;
        case BARCODE_EANX:
        case BARCODE_EANX_CHK:
        case BARCODE_EANX_CC:
        case BARCODE_ISBNX:
            /* GS1 General Specifications 21.0.1 Section 5.2.3.4 */
            switch (ustrlen(symbol->text)) {
                case 13: /* EAN-13 */
                    *left = 11;
                    *right = 7;
                    break;
                case 16: /* EAN-13/ISBN + 2 digit addon */
                case 19: /* EAN-13/ISBN + 5 digit addon */
                    *left = 11;
                    *right = 5;
                    break;
                case 5: /* EAN-5 addon */
                case 2: /* EAN-2 addon */
                    *left = 7;
                    *right = 5;
                    break;
                default: /* EAN-8 (+/- 2/5 digit addon) */
                    *left = 7;
                    *right = 7;
                    break;
            }
            done = 1;
            break;
        case BARCODE_UPCA:
        case BARCODE_UPCA_CHK:
        case BARCODE_UPCA_CC:
            /* GS1 General Specifications 21.0.1 Section 5.2.3.4 */
            *left = 9;
            if (ustrlen(symbol->text) > 12) { /* UPC-A + addon */
                *right = 5;
            } else {
                *right = 9;
            }
            done = 1;
            break;
        case BARCODE_UPCE:
        case BARCODE_UPCE_CHK:
        case BARCODE_UPCE_CC:
            /* GS1 General Specifications 21.0.1 Section 5.2.3.4 */
            *left = 9;
            if (ustrlen(symbol->text) > 8) { /* UPC-E + addon */
                *right = 5;
            } else {
                *right = 7;
            }
            done = 1;
            break;
    }

    if (done) {
        return done;
    }

    /* Only do others if flag set TODO: finish */
#if 0
    if (!(symbol->output_options & BARCODE_QUIET_ZONES)) {
        return done;
    }
#else
    return done;
#endif

    switch (symbol->symbology) {
        case BARCODE_CODE11:
            /* TODO */
            break;

        case BARCODE_C25STANDARD:
        case BARCODE_C25INTER:
        case BARCODE_C25IATA:
        case BARCODE_C25LOGIC:
        case BARCODE_C25IND:
            *left = *right = 10; // Probably this TODO: check
            done = 1;
            break;

        case BARCODE_CODE39:
        case BARCODE_EXCODE39:
        case BARCODE_LOGMARS:
        case BARCODE_PZN:
        case BARCODE_VIN:
        case BARCODE_HIBC_39:
        case BARCODE_CODE32:
            /* ISO/IEC 16388:2007 Section 4.4 (d) */
            *left = *right = 10;
            done = 1;
            break;
        case BARCODE_GS1_128: /* GS1-128 */
        case BARCODE_GS1_128_CC:
        case BARCODE_EAN14:
            /* GS1 General Specifications 21.0.1 Section 5.4.4.2 */
            *left = *right = 10;
            done = 1;
            break;
        case BARCODE_CODABAR:
            /* BS EN 798:1995 Section 4.4.1 (d) */
            *left = *right = 10;
            done = 1;
            break;
        case BARCODE_CODE128:
        case BARCODE_CODE128B:
        case BARCODE_HIBC_128:
        case BARCODE_NVE18:
            /* ISO/IEC 15417:2007 4.4.2 */
            *left = *right = 10;
            done = 1;
            break;

        case BARCODE_DPLEIT:
        case BARCODE_DPIDENT:
            /* TODO */
            break;

        case BARCODE_CODE93:
            /* TODO */
            break;

        case BARCODE_FLAT:
            /* TODO */
            break;

        case BARCODE_DBAR_OMN: /* GS1 Databar Omnidirectional */
        case BARCODE_DBAR_LTD: /* GS1 Databar Limited */
        case BARCODE_DBAR_EXP: /* GS1 Databar Expanded */
        case BARCODE_DBAR_STK: /* GS1 DataBar Stacked */
        case BARCODE_DBAR_OMNSTK: /* GS1 DataBar Stacked Omnidirectional */
        case BARCODE_DBAR_EXPSTK: /* GS1 Databar Expanded Stacked */
            /* GS1 General Specifications 21.0.1 Section 5.5.1.1 - Quiet Zones: None required */
            *left = *right = 0;
            done = 1;
            break;
        case BARCODE_DBAR_OMN_CC:
        case BARCODE_DBAR_LTD_CC:
        case BARCODE_DBAR_EXP_CC:
        case BARCODE_DBAR_STK_CC:
        case BARCODE_DBAR_OMNSTK_CC:
        case BARCODE_DBAR_EXPSTK_CC:
            /* GS1 General Specifications 21.0.1 Sections 5.11.2.1 (CC-A) & 5.11.2.2 (CC-B) */
            *left = *right = 1;
            done = 1;
            break;

        case BARCODE_TELEPEN:
        case BARCODE_TELEPEN_NUM:
            /* TODO */
            break;

        case BARCODE_POSTNET:
        case BARCODE_PLANET:
            /* Archived DMM C840 (Dec 09, 2004) left/right 0.125" ~ 6X, top/bottom 0.04" ~ 2X */
            *left = *right = 6; // TODO: Proper inch to X calc
            *top = *bottom = 2; // TODO: Proper inch to X calc
            done = 1;
            break;

        case BARCODE_MSI_PLESSEY:
            /* TODO */
            break;

        case BARCODE_FIM:
            /* TODO */
            break;

        case BARCODE_PHARMA:
        case BARCODE_PHARMA_TWO:
            /* TODO */
            break;

        case BARCODE_PDF417:
        case BARCODE_PDF417COMP:
        case BARCODE_HIBC_PDF:
            /* ISO/IEC 15438:2015 Section 5.8.3 */
            *left = *right = *top = *bottom = 2;
            done = 1;
            break;
        case BARCODE_MICROPDF417:
        case BARCODE_HIBC_MICPDF:
            /* ISO/IEC 24728:2006 Section 5.8.3 */
            *left = *right = *top = *bottom = 1;
            done = 1;
            break;
        case BARCODE_MAXICODE:
            /* ISO/IEC 16023:2000 Section 4.11.5 */
            *left = *right = *top = *bottom = 1;
            done = 1;
            break;
        case BARCODE_QRCODE:
        case BARCODE_UPNQR:
        case BARCODE_HIBC_QR:
            /* ISO/IEC 18004:2015 Section 9.1 */
            *left = *right = *top = *bottom = 4;
            done = 1;
            break;
        case BARCODE_MICROQR:
            /* ISO/IEC 18004:2015 Section 9.1 */
            *left = *right = *top = *bottom = 2;
            done = 1;
            break;
        case BARCODE_RMQR:
            /* ISO/IEC JTC1/SC31N000 Section 6.3.10 */
            *left = *right = *top = *bottom = 2;
            done = 1;
            break;
        case BARCODE_AUSPOST:
        case BARCODE_AUSREPLY:
        case BARCODE_AUSROUTE:
        case BARCODE_AUSREDIRECT:
            /* Customer Barcode Technical Specifications (2012) left/right 6mm ~ 6X, top/bottom 2mm ~ 2X */
            *left = *right = 6; // TODO: Proper mm to X calc
            *top = *bottom = 2; // TODO: Proper mm to X calc
            done = 1;
            break;

        case BARCODE_RM4SCC:
            /* TODO */
            break;

        case BARCODE_DATAMATRIX:
        case BARCODE_HIBC_DM:
            /* ISO/IEC 16022:2006 Section 7.1 */
            *left = *right = *top = *bottom = 1;
            done = 1;
            break;

        case BARCODE_JAPANPOST:
            /* TODO */
            break;

        case BARCODE_KOREAPOST:
            /* TODO */
            break;

        case BARCODE_USPS_IMAIL:
            /* USPS-B-3200 (2015) Section 2.3.2 left/right 0.125" ~ 6X, top/bottom 0.026" ~ 1X */
            *left = *right = 6; // TODO: Proper inch to X calc
            *top = *bottom = 1; // TODO: Proper inch to X calc
            done = 1;
            break;

        case BARCODE_PLESSEY:
            /* TODO */
            break;

        case BARCODE_KIX:
            /* Handleiding KIX code brochure left/right/top/bottom 2mm ~ 2X */
            *left = *right = *top = *bottom = 2; // TODO: Proper mm to X calc
            done = 1;
            break;
        case BARCODE_AZTEC:
        case BARCODE_HIBC_AZTEC:
        case BARCODE_AZRUNE:
            /* ISO/IEC 24778:2008 Section 4.1 (c) & Annex A.1 (Rune) - no quiet zone required */
            done = 1;
            break;

        case BARCODE_DAFT:
            /* TODO */
            break;

        case BARCODE_DOTCODE:
            /* ISS DotCode Rev. 4.0 Section 4.1 (3) (c) */
            *left = *right = *top = *bottom = 3;
            done = 1;
            break;
        case BARCODE_HANXIN:
            /* ISO/IEC DIS 20830:2019 Section 4.2.8 (also Section 6.2) */
            *left = *right = *top = *bottom = 3;
            done = 1;
            break;

        case BARCODE_MAILMARK:
            /* User Guide left/right/top/bottom 2mm ~ 2X */
            *left = *right = *top = *bottom = 2; // TODO: Proper mm to X calc
            done = 1;
            break;

        case BARCODE_CHANNEL:
            /* ANSI/AIM BC12-1998 Section 4.4 (c) */
            *left = 1;
            *right = 2;
            done = 1;
            break;

        case BARCODE_CODEONE:
            /* TODO */
            break;

        case BARCODE_GRIDMATRIX:
            /* AIMD014 (v 1.63) Section 7.1 */
            *left = *right = *top = *bottom = 6;
            done = 1;
            break;
        case BARCODE_ULTRA:
            /* AIMD/TSC15032-43 (v 0.99c) Section 9.2 */
            *left = *right = *top = *bottom = 1;
            done = 1;
            break;
    }

    return done; /* For self-checking */
}

/* Set left (x), top (y), right and bottom offsets for whitespace */
INTERNAL void output_set_whitespace_offsets(struct zint_symbol *symbol, int *xoffset, int *yoffset, int *roffset, int *boffset) {
    int qz_left, qz_right, qz_top, qz_bottom;

    quiet_zones(symbol, &qz_left, &qz_right, &qz_top, &qz_bottom);

    *xoffset = symbol->whitespace_width + qz_left;
    *roffset = symbol->whitespace_width + qz_right;
    if (symbol->output_options & BARCODE_BOX) {
        *xoffset += symbol->border_width;
        *roffset += symbol->border_width;
    }

    *yoffset = symbol->whitespace_height + qz_top;
    *boffset = symbol->whitespace_height + qz_bottom;
    if (symbol->output_options & (BARCODE_BOX | BARCODE_BIND)) {
        *yoffset += symbol->border_width;
        *boffset += symbol->border_width;
    }
}

/* Set composite offset and main width excluding addon (for start of addon calc) and addon text, returning UPC/EAN type */
INTERNAL int output_process_upcean(struct zint_symbol *symbol, int *p_main_width, int *p_comp_offset, unsigned char addon[6], int *p_addon_gap) {
    int main_width; /* Width of main linear symbol, excluding addon */
    int comp_offset; /* Whitespace offset (if any) of main linear symbol due to having composite */
    int upceanflag; /* UPC/EAN type flag */
    int i, j, latch;
    int text_length = (int) ustrlen(symbol->text);

    latch = 0;
    j = 0;
    /* Isolate add-on text */
    for (i = 6; i < text_length && j < 5; i++) {
        if (latch == 1) {
            addon[j] = symbol->show_hrt ? symbol->text[i] : ' '; /* Use dummy space-filled addon if no hrt */
            j++;
        } else if (symbol->text[i] == '+') {
            latch = 1;
        }
    }
    addon[j] = '\0';
    if (latch) {
        if (symbol->symbology == BARCODE_UPCA || symbol->symbology == BARCODE_UPCA_CHK || symbol->symbology == BARCODE_UPCA_CC) {
            *p_addon_gap = symbol->option_2 >= 9 && symbol->option_2 <= 12 ? symbol->option_2 : 9;
        } else {
            *p_addon_gap = symbol->option_2 >= 7 && symbol->option_2 <= 12 ? symbol->option_2 : 7;
        }
    }

    /* Calculate composite offset */
    comp_offset = 0;
    if (is_composite(symbol->symbology)) {
        while (!(module_is_set(symbol, symbol->rows - 1, comp_offset))) {
            comp_offset++;
        }
    }

    upceanflag = 0;
    main_width = symbol->width;
    if ((symbol->symbology == BARCODE_EANX) || (symbol->symbology == BARCODE_EANX_CHK)
            || (symbol->symbology == BARCODE_EANX_CC) || (symbol->symbology == BARCODE_ISBNX)) {
        switch (text_length) {
            case 13: /* EAN-13 */
            case 16: /* EAN-13 + EAN-2 */
            case 19: /* EAN-13 + EAN-5 */
                main_width = 95 + comp_offset; /* EAN-13 main symbol 95 modules wide */
                upceanflag = 13;
                break;
            case 2:
                /* EAN-2 can't have addon or be composite */
                upceanflag = 2;
                break;
            case 5:
                /* EAN-5 can't have addon or be composite */
                upceanflag = 5;
                break;
            default:
                main_width = 68 + comp_offset; /* EAN-8 main symbol 68 modules wide */
                upceanflag = 8;
                break;
        }
    } else if ((symbol->symbology == BARCODE_UPCA) || (symbol->symbology == BARCODE_UPCA_CHK)
            || (symbol->symbology == BARCODE_UPCA_CC)) {
        main_width = 95 + comp_offset; /* UPC-A main symbol 95 modules wide */
        upceanflag = 12;
    } else if ((symbol->symbology == BARCODE_UPCE) || (symbol->symbology == BARCODE_UPCE_CHK)
            || (symbol->symbology == BARCODE_UPCE_CC)) {
        main_width = 51 + comp_offset; /* UPC-E main symbol 51 modules wide */
        upceanflag = 6;
    }

    *p_comp_offset = comp_offset;
    *p_main_width = main_width;

    return upceanflag;
}

/* Calculate large bar height i.e. linear bars */
INTERNAL float output_large_bar_height(struct zint_symbol *symbol) {
    int i, large_bar_count, preset_height;
    float large_bar_height;

    large_bar_count = 0;
    preset_height = 0;
    for (i = 0; i < symbol->rows; i++) {
        preset_height += symbol->row_height[i];
        if (symbol->row_height[i] == 0) {
            large_bar_count++;
        }
    }

    if (large_bar_count) {
        large_bar_height = (float) (symbol->height - preset_height) / large_bar_count;
    } else {
        large_bar_height = 0.0f; /* Not used if large_bar_count zero */
    }

    return large_bar_height;
}

/* Split UPC/EAN add-on text into various constituents */
INTERNAL void output_upcean_split_text(int upceanflag, unsigned char text[],
                unsigned char textpart1[5], unsigned char textpart2[7], unsigned char textpart3[7], unsigned char textpart4[2]) {
    int i;

    if (upceanflag == 6) { /* UPC-E */
        textpart1[0] = text[0];
        textpart1[1] = '\0';

        for (i = 0; i < 6; i++) {
            textpart2[i] = text[i + 1];
        }
        textpart2[6] = '\0';

        textpart3[0] = text[7];
        textpart3[1] = '\0';

    } else if (upceanflag == 8) { /* EAN-8 */
        for (i = 0; i < 4; i++) {
            textpart1[i] = text[i];
        }
        textpart1[4] = '\0';

        for (i = 0; i < 4; i++) {
            textpart2[i] = text[i + 4];
        }
        textpart2[4] = '\0';

    } else if (upceanflag == 12) { /* UPC-A */
        textpart1[0] = text[0];
        textpart1[1] = '\0';

        for (i = 0; i < 5; i++) {
            textpart2[i] = text[i + 1];
        }
        textpart2[5] = '\0';

        for (i = 0; i < 5; i++) {
            textpart3[i] = text[i + 6];
        }
        textpart3[5] = '\0';

        textpart4[0] = text[11];
        textpart4[1] = '\0';

    } else if (upceanflag == 13) { /* EAN-13 */
        textpart1[0] = text[0];
        textpart1[1] = '\0';

        for (i = 0; i < 6; i++) {
            textpart2[i] = text[i + 1];
        }
        textpart2[6] = '\0';

        for (i = 0; i < 6; i++) {
            textpart3[i] = text[i + 7];
        }
        textpart3[6] = '\0';
    }
}
