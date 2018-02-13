/*  emf.c - Support for Microsoft Enhanced Metafile Format

    libzint - the open source barcode library
    Copyright (C) 2016-2017 Robin Stuart <rstuart114@gmail.com>

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

/* Developed according to [MS-EMF] - v20160714, Released July 14, 2016
 * and [MS-WMF] - v20160714, Released July 14, 2016 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"
#include "emf.h"

#define SSET	"0123456789ABCDEF"

int count_rectangles(struct zint_symbol *symbol) {
    int rectangles = 0;

    if ((symbol->symbology != BARCODE_MAXICODE) &&
            ((symbol->output_options & BARCODE_DOTTY_MODE) == 0)) {
        int this_row;
        for(this_row = 0; this_row < symbol->rows; this_row++) {
            int i, latch = 0;
            for(i = 0; i < symbol->width; i++) {
                if ((module_is_set(symbol, this_row, i)) && (latch == 0)) {
                    latch = 1;
                    rectangles++;
                }

                if ((!(module_is_set(symbol, this_row, i))) && (latch == 1)) {
                    latch = 0;
                }
            }
        }
    }

    return rectangles;
}

int count_circles(struct zint_symbol *symbol) {
    int circles = 0;

    if ((symbol->symbology != BARCODE_MAXICODE) &&
            ((symbol->output_options & BARCODE_DOTTY_MODE) != 0)) {
        int this_row;
        for(this_row = 0; this_row < symbol->rows; this_row++) {
            int i;
            for(i = 0; i < symbol->width; i++) {
                if (module_is_set(symbol, this_row, i)) {
                    circles++;
                }
            }
        }
    }

    return circles;
}

int count_hexagons(struct zint_symbol *symbol) {
    int hexagons = 0;

    if (symbol->symbology == BARCODE_MAXICODE) {
        int this_row;
        for(this_row = 0; this_row < symbol->rows; this_row++) {
            int i;
            for(i = 0; i < symbol->width; i++) {
                if (module_is_set(symbol, this_row, i)) {
                    hexagons++;
                }
            }
        }
    }

    return hexagons;
}

void utfle_copy(unsigned char *output, unsigned char *input, int length) {
    int i;
    int o;

    /* Convert UTF-8 to UTF-16LE - only needs to handle characters <= U+00FF */
    i = 0;
    o = 0;
    do {
        if(input[i] <= 0x7f) {
            /* 1 byte mode (7-bit ASCII) */
            output[o] = input[i];
            output[o + 1] = 0x00;
            o += 2;
            i++;
        } else {
            /* 2 byte mode */
            output[o] = ((input[i] & 0x1f) << 6) + (input[i + 1] & 0x3f);
            output[o + 1] = 0x00;
            o += 2;
            i += 2;
        }
    } while (i < length);
}

int bump_up(int input) {
    /* Strings length must be a multiple of 4 bytes */
    if ((input % 2) == 1) {
        input++;
    }
    return input;
}

int emf_plot(struct zint_symbol *symbol) {
    int i, block_width, latch, this_row;
    float large_bar_height, preset_height, row_height;
    FILE *emf_file;
    int fgred, fggrn, fgblu, bgred, bggrn, bgblu;
    int error_number = 0;
    int textoffset, xoffset, yoffset, textdone;
    int large_bar_count, comp_offset;
    float scaler = symbol->scale * 10;
    int rectangle_count, this_rectangle;
    int circle_count, this_circle;
    int hexagon_count, this_hexagon;
    int bytecount, recordcount;
    int upcean = 0;
    unsigned char regw[7];
    unsigned char regx[7];
    unsigned char regy[7];
    unsigned char regz[7];
    unsigned char output_buffer[12];
    uint32_t dx;

    emr_header_t emr_header;
    emr_eof_t emr_eof;
    emr_createbrushindirect_t emr_createbrushindirect_fg;
    emr_createbrushindirect_t emr_createbrushindirect_bg;
    emr_selectobject_t emr_selectobject_fgbrush;
    emr_selectobject_t emr_selectobject_bgbrush;
    emr_createpen_t emr_createpen;
    emr_selectobject_t emr_selectobject_pen;
    emr_rectangle_t background;
    emr_ellipse_t bullseye[6];
    emr_extcreatefontindirectw_t emr_extcreatefontindirectw;
    emr_selectobject_t emr_selectobject_font;
    emr_exttextoutw_t emr_exttextoutw[6];
    emr_extcreatefontindirectw_t emr_extcreatefontindirectw_big;
    emr_selectobject_t emr_selectobject_font_big;

    box_t box;

#ifndef _MSC_VER
    unsigned char local_text[bump_up(ustrlen(symbol->text) + 1)];
    unsigned char string_buffer[2 * bump_up(ustrlen(symbol->text) + 1)];
#else
	unsigned char* local_text;
	unsigned char* string_buffer;
	emr_rectangle_t *rectangle, *row_binding;
	emr_ellipse_t* circle;
    emr_polygon_t* hexagon;
    local_text = (unsigned char*) _alloca(bump_up(ustrlen(symbol->text) + 1) * sizeof (unsigned char));
    string_buffer = (unsigned char*) _alloca(2 * bump_up(ustrlen(symbol->text) + 1) * sizeof (unsigned char));
#endif

    row_height = 0;
    textdone = 0;
    comp_offset = 0;
    this_rectangle = 0;
    this_circle = 0;
    this_hexagon = 0;
    dx = 0;
    latch = 0;

    for(i = 0; i < 6; i++) {
        regw[i] = '\0';
        regx[i] = '\0';
        regy[i] = '\0';
        regz[i] = '\0';
    }

    if (symbol->show_hrt != 0) {
        /* Copy text from symbol */
        ustrcpy(local_text, symbol->text);
    } else {
        /* No text needed */
        switch (symbol->symbology) {
            case BARCODE_EANX:
            case BARCODE_EANX_CC:
            case BARCODE_ISBNX:
            case BARCODE_UPCA:
            case BARCODE_UPCE:
            case BARCODE_UPCA_CC:
            case BARCODE_UPCE_CC:
                /* For these symbols use dummy text to ensure formatting is done
                 * properly even if no text is required */
                for (i = 0; i < ustrlen(symbol->text); i++) {
                    if (symbol->text[i] == '+') {
                        local_text[i] = '+';
                    } else {
                        local_text[i] = ' ';
                    }
                    local_text[ustrlen(symbol->text)] = '\0';
                }
                break;
            default:
                /* For everything else, just remove the text */
                local_text[0] = '\0';
                break;
        }
    }

        /* sort out colour options */
    to_upper((unsigned char*) symbol->fgcolour);
    to_upper((unsigned char*) symbol->bgcolour);

    if (strlen(symbol->fgcolour) != 6) {
        strcpy(symbol->errtxt, "641: Malformed foreground colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }

    if (strlen(symbol->bgcolour) != 6) {
        strcpy(symbol->errtxt, "642: Malformed background colour target");
        return ZINT_ERROR_INVALID_OPTION;
    }

    fgred = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
    fggrn = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
    fgblu = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
    bgred = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
    bggrn = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
    bgblu = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);

    if (symbol->height == 0) {
        symbol->height = 50;
    }

    large_bar_count = 0;
    preset_height = 0.0;
    for (i = 0; i < symbol->rows; i++) {
        preset_height += symbol->row_height[i];
        if (symbol->row_height[i] == 0) {
            large_bar_count++;
        }
    }
    large_bar_height = (symbol->height - preset_height) / large_bar_count;

    if (large_bar_count == 0) {
        symbol->height = preset_height;
    }

    while (!(module_is_set(symbol, symbol->rows - 1, comp_offset))) {
        comp_offset++;
    }

    /* Certain symbols need whitespace otherwise characters get chopped off the sides */
    if ((((symbol->symbology == BARCODE_EANX) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_EANX_CC))
            || (symbol->symbology == BARCODE_ISBNX)) {
        switch (ustrlen(local_text)) {
            case 13: /* EAN 13 */
            case 16:
            case 19:
                if (symbol->whitespace_width == 0) {
                    symbol->whitespace_width = 10;
                }
                break;
        }
        upcean = 1;
    }

    if (((symbol->symbology == BARCODE_UPCA) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCA_CC)) {
        if (symbol->whitespace_width == 0) {
            symbol->whitespace_width = 10;
        }
        upcean = 1;
    }

    if (((symbol->symbology == BARCODE_UPCE) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCE_CC)) {
        if (symbol->whitespace_width == 0) {
            symbol->whitespace_width = 10;
        }
        upcean = 1;
    }

    if (ustrlen(local_text) != 0) {
        textoffset = 9;
    } else {
        textoffset = 0;
    }
    xoffset = symbol->border_width + symbol->whitespace_width;
    yoffset = symbol->border_width;

    rectangle_count = count_rectangles(symbol);
    circle_count = count_circles(symbol);
    hexagon_count = count_hexagons(symbol);

#ifndef _MSC_VER
    emr_rectangle_t rectangle[rectangle_count];
    emr_rectangle_t row_binding[symbol->rows - 1];
    emr_ellipse_t circle[circle_count];
    emr_polygon_t hexagon[hexagon_count];
#else
    rectangle = (emr_rectangle_t*) _alloca(rectangle_count*sizeof(emr_rectangle_t));
    row_binding = (emr_rectangle_t*) _alloca((symbol->rows - 1)*sizeof(emr_rectangle_t));
    circle = (emr_ellipse_t*) _alloca(circle_count*sizeof(emr_ellipse_t));
    hexagon = (emr_polygon_t*) _alloca(hexagon_count*sizeof(emr_polygon_t));
#endif

    /* Header */
    emr_header.type = 0x00000001; // EMR_HEADER
    emr_header.size = 88; // Assuming no additional data in header
    emr_header.emf_header.bounds.left = 0;
    if (symbol->symbology != BARCODE_MAXICODE) {
        emr_header.emf_header.bounds.right = ceil((symbol->width + xoffset + xoffset) * scaler);
        emr_header.emf_header.bounds.bottom = ceil((symbol->height + textoffset + yoffset + yoffset) * scaler);
    } else {
        emr_header.emf_header.bounds.right = ceil((74.0F + xoffset + xoffset) * scaler);
        emr_header.emf_header.bounds.bottom = ceil((72.0F + yoffset + yoffset) * scaler);
    }
    emr_header.emf_header.bounds.top = 0;
    emr_header.emf_header.frame.left = 0;
    emr_header.emf_header.frame.right = emr_header.emf_header.bounds.right * 30;
    emr_header.emf_header.frame.top = 0;
    emr_header.emf_header.frame.bottom = emr_header.emf_header.bounds.bottom * 30;
    emr_header.emf_header.record_signature = 0x464d4520; // ENHMETA_SIGNATURE
    emr_header.emf_header.version = 0x00010000;;
    emr_header.emf_header.handles = 6; // Number of graphics objects
    emr_header.emf_header.reserved = 0x0000;
    emr_header.emf_header.n_description = 0;
    emr_header.emf_header.off_description = 0;
    emr_header.emf_header.n_pal_entries = 0;
    emr_header.emf_header.device.cx = 1000;
    emr_header.emf_header.device.cy = 1000;
    emr_header.emf_header.millimeters.cx = 300;
    emr_header.emf_header.millimeters.cy = 300;
    bytecount = 88;
    recordcount = 1;

    /* Create Brushes */
    emr_createbrushindirect_fg.type = 0x00000027; // EMR_CREATEBRUSHINDIRECT
    emr_createbrushindirect_fg.size = 24;
    emr_createbrushindirect_fg.ih_brush = 1;
    emr_createbrushindirect_fg.log_brush.brush_style = 0x0000; // BS_SOLID
    emr_createbrushindirect_fg.log_brush.color.red = fgred;
    emr_createbrushindirect_fg.log_brush.color.green = fggrn;
    emr_createbrushindirect_fg.log_brush.color.blue = fgblu;
    emr_createbrushindirect_fg.log_brush.color.reserved = 0;
    emr_createbrushindirect_fg.log_brush.brush_hatch = 0; // ignored
    bytecount += 24;
    recordcount++;

    emr_createbrushindirect_bg.type = 0x00000027; // EMR_CREATEBRUSHINDIRECT
    emr_createbrushindirect_bg.size = 24;
    emr_createbrushindirect_bg.ih_brush = 2;
    emr_createbrushindirect_bg.log_brush.brush_style = 0x0000; // BS_SOLID
    emr_createbrushindirect_bg.log_brush.color.red = bgred;
    emr_createbrushindirect_bg.log_brush.color.green = bggrn;
    emr_createbrushindirect_bg.log_brush.color.blue = bgblu;
    emr_createbrushindirect_bg.log_brush.color.reserved = 0;
    emr_createbrushindirect_bg.log_brush.brush_hatch = 0; // ignored
    bytecount += 24;
    recordcount++;

    emr_selectobject_fgbrush.type = 0x00000025; // EMR_SELECTOBJECT
    emr_selectobject_fgbrush.size = 12;
    emr_selectobject_fgbrush.ih_object = 1;
    bytecount += 12;
    recordcount++;

    emr_selectobject_bgbrush.type = 0x00000025; // EMR_SELECTOBJECT
    emr_selectobject_bgbrush.size = 12;
    emr_selectobject_bgbrush.ih_object = 2;
    bytecount += 12;
    recordcount++;

    /* Create Pens */
    emr_createpen.type = 0x00000026; // EMR_CREATEPEN
    emr_createpen.size = 28;
    emr_createpen.ih_pen = 3;
    emr_createpen.log_pen.pen_style = 0x00000005; // PS_NULL
    emr_createpen.log_pen.width.x = 1;
    emr_createpen.log_pen.width.y = 0; // ignored
    emr_createpen.log_pen.color_ref.red = 0;
    emr_createpen.log_pen.color_ref.green = 0;
    emr_createpen.log_pen.color_ref.blue = 0;
    emr_createpen.log_pen.color_ref.reserved = 0;
    bytecount += 28;
    recordcount++;

    emr_selectobject_pen.type = 0x00000025; // EMR_SELECTOBJECT
    emr_selectobject_pen.size = 12;
    emr_selectobject_pen.ih_object = 3;
    bytecount += 12;
    recordcount++;

    /* Create font records */
    if ((symbol->show_hrt != 0) && (ustrlen(local_text) != 0)) {
        emr_extcreatefontindirectw.type = 0x00000052; // EMR_EXTCREATEFONTINDIRECTW
        emr_extcreatefontindirectw.size = 104;
        emr_extcreatefontindirectw.ih_fonts = 4;
        emr_extcreatefontindirectw.elw.height = (8 * scaler);
        emr_extcreatefontindirectw.elw.width = 0; // automatic
        emr_extcreatefontindirectw.elw.escapement = 0;
        emr_extcreatefontindirectw.elw.orientation = 0;
        emr_extcreatefontindirectw.elw.weight = 400;
        emr_extcreatefontindirectw.elw.italic = 0x00;
        emr_extcreatefontindirectw.elw.underline = 0x00;
        emr_extcreatefontindirectw.elw.strike_out = 0x00;
        emr_extcreatefontindirectw.elw.char_set = 0x01;
        emr_extcreatefontindirectw.elw.out_precision = 0x00; // OUT_DEFAULT_PRECIS
        emr_extcreatefontindirectw.elw.clip_precision = 0x00; // CLIP_DEFAULT_PRECIS
        emr_extcreatefontindirectw.elw.quality = 0x00;
        emr_extcreatefontindirectw.elw.pitch_and_family = 0x00;
        for(i = 0; i < 64; i++) {
            emr_extcreatefontindirectw.elw.facename[i] = '\0';
        }
        utfle_copy(emr_extcreatefontindirectw.elw.facename, (unsigned char*) "sans-serif", 10);

        emr_selectobject_font.type = 0x00000025; // EMR_SELECTOBJECT
        emr_selectobject_font.size = 12;
        emr_selectobject_font.ih_object = 4;

        if (!((symbol->symbology == BARCODE_EANX) || (symbol->symbology == BARCODE_EANX_CC) || (symbol->symbology == BARCODE_ISBNX))) {
            bytecount += 104;
            recordcount++;
            bytecount += 12;
            recordcount++;
        }

        if (upcean) {
            emr_extcreatefontindirectw_big.type = 0x00000052; // EMR_EXTCREATEFONTINDIRECTW
            emr_extcreatefontindirectw_big.size = 104;
            if (!((symbol->symbology == BARCODE_EANX) || (symbol->symbology == BARCODE_EANX_CC) || (symbol->symbology == BARCODE_ISBNX))) {
                emr_extcreatefontindirectw_big.ih_fonts = 4;
            } else {
                emr_extcreatefontindirectw_big.ih_fonts = 5;
            }
            emr_extcreatefontindirectw_big.elw.height = (11 * scaler);
            emr_extcreatefontindirectw_big.elw.width = 0; // automatic
            emr_extcreatefontindirectw_big.elw.escapement = 0;
            emr_extcreatefontindirectw_big.elw.orientation = 0;
            emr_extcreatefontindirectw_big.elw.weight = 400;
            emr_extcreatefontindirectw_big.elw.italic = 0x00;
            emr_extcreatefontindirectw_big.elw.underline = 0x00;
            emr_extcreatefontindirectw_big.elw.strike_out = 0x00;
            emr_extcreatefontindirectw_big.elw.char_set = 0x01;
            emr_extcreatefontindirectw_big.elw.out_precision = 0x00; // OUT_DEFAULT_PRECIS
            emr_extcreatefontindirectw_big.elw.clip_precision = 0x00; // CLIP_DEFAULT_PRECIS
            emr_extcreatefontindirectw_big.elw.quality = 0x00;
            emr_extcreatefontindirectw_big.elw.pitch_and_family = 0x00;
            for(i = 0; i < 64; i++) {
                emr_extcreatefontindirectw_big.elw.facename[i] = '\0';
            }
            utfle_copy(emr_extcreatefontindirectw_big.elw.facename, (unsigned char*) "sans-serif", 10);
            bytecount += 104;
            recordcount++;

            emr_selectobject_font_big.type = 0x00000025; // EMR_SELECTOBJECT
            emr_selectobject_font_big.size = 12;
            emr_selectobject_font_big.ih_object = 5;
            bytecount += 12;
            recordcount++;
        }
    }

    /* Text */
    if ((symbol->show_hrt != 0) && (ustrlen(local_text) != 0)) {

        if ((symbol->symbology == BARCODE_EANX) || (symbol->symbology == BARCODE_EANX_CC) || (symbol->symbology == BARCODE_ISBNX)) {
            latch = ustrlen(local_text);
            for(i = 0; i < ustrlen(local_text); i++) {
                if (local_text[i] == '+') {
                    latch = i;
                }
            }
            if (latch > 8) {
                // EAN-13
                for(i = 1; i <= 6; i++) {
                    regw[i - 1] = local_text[i];
                    regx[i - 1] = local_text[i + 6];
                }
                if (ustrlen(local_text) > latch) {
                    // With add-on
                    for (i = (latch + 1); i <= ustrlen(local_text); i++) {
                        regz[i - (latch + 1)] = local_text[i];
                    }
                }
                local_text[1] = '\0';
            } else if (latch > 5) {
                // EAN-8
                for(i = 0; i <= 3; i++) {
                    regw[i] = local_text[i + 4];
                }
                if (ustrlen(local_text) > latch) {
                    // With add-on
                    for (i = (latch + 1); i <= ustrlen(local_text); i++) {
                        regz[i - (latch + 1)] = local_text[i];
                    }
                }
                local_text[4] = '\0';
            }

        }

        if ((symbol->symbology == BARCODE_UPCA) || (symbol->symbology == BARCODE_UPCA_CC)) {
            latch = ustrlen(local_text);
            for(i = 0; i < ustrlen(local_text); i++) {
                if (local_text[i] == '+') {
                    latch = i;
                }
            }
            if (ustrlen(local_text) > latch) {
                // With add-on
                for (i = (latch + 1); i <= ustrlen(local_text); i++) {
                    regz[i - (latch + 1)] = local_text[i];
                }
            }
            for(i = 1; i <= 5; i++) {
                regw[i - 1] = local_text[i];
                regx[i - 1] = local_text[i + 6];
            }
            regy[0] = local_text[11];
            local_text[1] = '\0';
        }

        if ((symbol->symbology == BARCODE_UPCE) || (symbol->symbology == BARCODE_UPCE_CC)) {
            latch = ustrlen(local_text);
            for(i = 0; i < ustrlen(local_text); i++) {
                if (local_text[i] == '+') {
                    latch = i;
                }
            }
            if (ustrlen(local_text) > latch) {
                // With add-on
                for (i = (latch + 1); i <= ustrlen(local_text); i++) {
                    regz[i - (latch + 1)] = local_text[i];
                }
            }
            for(i = 1; i <= 6; i++) {
                regw[i - 1] = local_text[i];
            }
            regx[0] = local_text[7];
            local_text[1] = '\0';
        }

        for(i = 0; i <= 5; i++) {
            emr_exttextoutw[i].type = 0x00000054; // EMR_EXTTEXTOUTW
            emr_exttextoutw[i].bounds.top = 0; // ignored
            emr_exttextoutw[i].bounds.left = 0; // ignoredemr_header.emf_header.bytes +=
            emr_exttextoutw[i].bounds.right = 0xffffffff; // ignored
            emr_exttextoutw[i].bounds.bottom = 0xffffffff; // ignored
            emr_exttextoutw[i].i_graphics_mode = 0x00000001; // GM_COMPATIBLE
            emr_exttextoutw[i].ex_scale = 1.0;
            emr_exttextoutw[i].ey_scale = 1.0;
            emr_exttextoutw[i].w_emr_text.off_string = 76;
            emr_exttextoutw[i].w_emr_text.options = 0;
            emr_exttextoutw[i].w_emr_text.rectangle.top = 0;
            emr_exttextoutw[i].w_emr_text.rectangle.left = 0;
            emr_exttextoutw[i].w_emr_text.rectangle.right = 0xffffffff;
            emr_exttextoutw[i].w_emr_text.rectangle.bottom = 0xffffffff;
            if (i > 0) {
                emr_exttextoutw[i].size = 76 + (6 * 6);
                emr_exttextoutw[i].w_emr_text.off_dx = 76 + (2 * 6);
            }
        }

        emr_exttextoutw[0].w_emr_text.chars = ustrlen(local_text);
        emr_exttextoutw[0].size = 76 + (6 * bump_up(ustrlen(local_text) + 1));
        emr_exttextoutw[0].w_emr_text.reference.x = (emr_header.emf_header.bounds.right - (ustrlen(local_text) * 5.3 * scaler)) / 2; // text left
        emr_exttextoutw[0].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler); // text top
        emr_exttextoutw[0].w_emr_text.off_dx = 76 + (2 * bump_up(ustrlen(local_text) + 1));
        for (i = 0; i < bump_up(ustrlen(local_text) + 1) * 2; i++) {
            string_buffer[i] = '\0';
        }
        utfle_copy(string_buffer, local_text, ustrlen(local_text));
        bytecount += 76 + (6 * bump_up(ustrlen(local_text) + 1));
        recordcount++;

        emr_exttextoutw[1].w_emr_text.chars = ustrlen(regw);
        emr_exttextoutw[2].w_emr_text.chars = ustrlen(regx);
        emr_exttextoutw[3].w_emr_text.chars = ustrlen(regy);
        emr_exttextoutw[4].w_emr_text.chars = ustrlen(regz);

        if ((symbol->symbology == BARCODE_EANX) || (symbol->symbology == BARCODE_EANX_CC) || (symbol->symbology == BARCODE_ISBNX)) {
            if (latch > 8) {
                /* EAN-13 */
                emr_exttextoutw[0].w_emr_text.reference.x = (xoffset - 9) * scaler;
                emr_exttextoutw[0].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
                emr_exttextoutw[1].w_emr_text.reference.x = (8 + xoffset) * scaler;
                emr_exttextoutw[1].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
                emr_exttextoutw[2].w_emr_text.reference.x = (55 + xoffset) * scaler;
                emr_exttextoutw[2].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
                if (ustrlen(regz) > 2) {
                    emr_exttextoutw[4].w_emr_text.reference.x = (115 + xoffset) * scaler;
                    bytecount += 112;
                    recordcount++;
                } else if (ustrlen(regz) != 0) {
                    emr_exttextoutw[4].w_emr_text.reference.x = (109 + xoffset) * scaler;
                    bytecount += 112;
                    recordcount++;
                }
                emr_exttextoutw[4].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - ((large_bar_height + 9) * scaler);
                bytecount += 2 * 112;
                recordcount += 2;
            } else if (latch > 5) {
                /* EAN-8 */
                emr_exttextoutw[0].w_emr_text.reference.x = (7 + xoffset) * scaler;
                emr_exttextoutw[0].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
                emr_exttextoutw[1].w_emr_text.reference.x = (40 + xoffset) * scaler;
                emr_exttextoutw[1].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
                if (ustrlen(regz) > 2) {
                    emr_exttextoutw[4].w_emr_text.reference.x = (87 + xoffset) * scaler;
                    bytecount += 112;
                    recordcount++;
                } else if (ustrlen(regz) != 0) {
                    emr_exttextoutw[4].w_emr_text.reference.x = (81 + xoffset) * scaler;
                    bytecount += 112;
                    recordcount++;
                }
                emr_exttextoutw[4].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - ((large_bar_height + 9) * scaler);
                bytecount += 112;
                recordcount++;
            }
        }

        if ((symbol->symbology == BARCODE_UPCA) || (symbol->symbology == BARCODE_UPCA_CC)) {
            emr_exttextoutw[0].w_emr_text.reference.x = (xoffset - 7) * scaler;
            emr_exttextoutw[0].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
            emr_exttextoutw[1].w_emr_text.reference.x = (14 + xoffset) * scaler;
            emr_exttextoutw[1].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
            emr_exttextoutw[2].w_emr_text.reference.x = (55 + xoffset) * scaler;
            emr_exttextoutw[2].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
            emr_exttextoutw[3].w_emr_text.reference.x = (98 + xoffset) * scaler;
            emr_exttextoutw[3].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
            if (ustrlen(regz) > 2) {
                emr_exttextoutw[4].w_emr_text.reference.x = (117 + xoffset) * scaler;
                bytecount += 112;
                recordcount++;
            } else if (ustrlen(regz) != 0) {
                emr_exttextoutw[4].w_emr_text.reference.x = (111 + xoffset) * scaler;
                bytecount += 112;
                recordcount++;
            }
            emr_exttextoutw[4].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - ((large_bar_height + 9) * scaler);
            bytecount += (3 * 112) + 12;
            recordcount += 4;
        }

        if ((symbol->symbology == BARCODE_UPCE) || (symbol->symbology == BARCODE_UPCE_CC)) {
            emr_exttextoutw[0].w_emr_text.reference.x = (xoffset - 7) * scaler;
            emr_exttextoutw[0].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
            emr_exttextoutw[1].w_emr_text.reference.x = (8 + xoffset) * scaler;
            emr_exttextoutw[1].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
            emr_exttextoutw[2].w_emr_text.reference.x = (53 + xoffset) * scaler;
            emr_exttextoutw[2].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - (9 * scaler);;
            if (ustrlen(regz) > 2) {
                emr_exttextoutw[4].w_emr_text.reference.x = (71 + xoffset) * scaler;
                bytecount += 112;
                recordcount++;
            } else if (ustrlen(regz) != 0) {
                emr_exttextoutw[4].w_emr_text.reference.x = (65 + xoffset) * scaler;
                bytecount += 112;
                recordcount++;
            }
            emr_exttextoutw[4].w_emr_text.reference.y = emr_header.emf_header.bounds.bottom - ((large_bar_height + 9) * scaler);
            bytecount += (2 * 112) + 12;
            recordcount += 3;
        }
    }

    /* Make background from a rectangle */
    background.type = 0x0000002b; // EMR_RECTANGLE;
    background.size = 24;
    background.box.top = 0;
    background.box.left = 0;
    background.box.right = emr_header.emf_header.bounds.right;
    background.box.bottom = emr_header.emf_header.bounds.bottom;
    bytecount += 24;
    recordcount++;

    /* Make bind and box rectangles if needed */
    if ((symbol->output_options & BARCODE_BIND) || (symbol->output_options & BARCODE_BOX)) {
        box.top.type = 0x0000002b; // EMR_RECTANGLE;
        box.top.size = 24;
        box.top.box.top = 0;
        box.top.box.bottom = symbol->border_width * scaler;
        box.top.box.left = symbol->border_width * scaler;
        box.top.box.right = emr_header.emf_header.bounds.right - (symbol->border_width * scaler);
        bytecount += 24;
        recordcount++;

        box.bottom.type = 0x0000002b; // EMR_RECTANGLE;
        box.bottom.size = 24;
        box.bottom.box.top = emr_header.emf_header.bounds.bottom - ((symbol->border_width + textoffset) * scaler);
        box.bottom.box.bottom = emr_header.emf_header.bounds.bottom - (textoffset * scaler);
        box.bottom.box.left = symbol->border_width * scaler;
        box.bottom.box.right = emr_header.emf_header.bounds.right - (symbol->border_width * scaler);
        bytecount += 24;
        recordcount++;

        if (symbol->output_options & BARCODE_BOX) {
            box.left.type = 0x0000002b; // EMR_RECTANGLE;
            box.left.size = 24;
            box.left.box.top = 0;
            box.left.box.bottom = emr_header.emf_header.bounds.bottom - (textoffset * scaler);
            box.left.box.left = 0;
            box.left.box.right = symbol->border_width * scaler;
            bytecount += 24;
            recordcount++;

            box.right.type = 0x0000002b; // EMR_RECTANGLE;
            box.right.size = 24;
            box.right.box.top = 0;
            box.right.box.bottom = emr_header.emf_header.bounds.bottom - (textoffset * scaler);
            box.right.box.left = emr_header.emf_header.bounds.right - (symbol->border_width * scaler);
            box.right.box.right = emr_header.emf_header.bounds.right;
            bytecount += 24;
            recordcount++;
        }
    }

    /* Make image rectangles, circles, hexagons */
    for (this_row = 0; this_row < symbol->rows; this_row++) {
        float row_posn;

        if (symbol->row_height[this_row] == 0) {
            row_height = large_bar_height;
        } else {
            row_height = symbol->row_height[this_row];
        }
        row_posn = 0;
        for (i = 0; i < this_row; i++) {
            if (symbol->row_height[i] == 0) {
                row_posn += large_bar_height;
            } else {
                row_posn += symbol->row_height[i];
            }
        }
        row_posn += yoffset;

        if (symbol->symbology != BARCODE_MAXICODE) {
            if ((symbol->output_options & BARCODE_DOTTY_MODE) != 0) {
                // Use dots (circles)
                for(i = 0; i < symbol->width; i++) {
                    if(module_is_set(symbol, this_row, i)) {
                        circle[this_circle].type = 0x0000002a; // EMR_ELLIPSE
                        circle[this_circle].size = 24;
                        circle[this_circle].box.top = this_row * scaler;
                        circle[this_circle].box.bottom = (this_row + 1) * scaler;
                        circle[this_circle].box.left = (i + xoffset) * scaler;
                        circle[this_circle].box.right = (i + xoffset + 1) * scaler;
                        this_circle++;
                        bytecount += 24;
                        recordcount++;
                    }
                }
            } else {
                // Normal mode, with rectangles
                i = 0;
                if (module_is_set(symbol, this_row, 0)) {
                    latch = 1;
                } else {
                    latch = 0;
                }

                do {
                    block_width = 0;
                    do {
                        block_width++;
                    } while (module_is_set(symbol, this_row, i + block_width) == module_is_set(symbol, this_row, i));

                    if (latch == 1) {
                        /* a bar */
                        rectangle[this_rectangle].type = 0x0000002b; // EMR_RECTANGLE;
                        rectangle[this_rectangle].size = 24;
                        rectangle[this_rectangle].box.top = row_posn * scaler;
                        rectangle[this_rectangle].box.bottom = (row_posn + row_height) * scaler;
                        rectangle[this_rectangle].box.left = (i + xoffset) * scaler;
                        rectangle[this_rectangle].box.right = (i + xoffset + block_width) * scaler;
                        bytecount += 24;
                        recordcount++;

                        if (this_row == symbol->rows - 1) {
                            /* Last row, extend bars if needed */
                            if ((((symbol->symbology == BARCODE_EANX) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_EANX_CC)) ||
                                    (symbol->symbology == BARCODE_ISBNX)) {
                                /* guard bar extensions for EAN8 and EAN13 */
                                if (ustrlen(regx) != 0) {
                                    /* EAN-13 */
                                    switch (i) {
                                        case 0:
                                        case 2:
                                        case 46:
                                        case 48:
                                        case 92:
                                        case 94:
                                            rectangle[this_rectangle].box.bottom += (5 * scaler);
                                            break;
                                    }
                                    if (i > 94) {
                                        /* Add-on */
                                        rectangle[this_rectangle].box.top += (10 * scaler);
                                        rectangle[this_rectangle].box.bottom += (5 * scaler);
                                    }
                                } else if (ustrlen(regw) != 0) {
                                        /* EAN-8 */
                                        switch (i) {
                                            case 0:
                                            case 2:
                                            case 32:
                                            case 34:
                                            case 64:
                                            case 66:
                                                rectangle[this_rectangle].box.bottom += (5 * scaler);
                                                break;
                                        }
                                        if (i > 66) {
                                            /* Add-on */
                                            rectangle[this_rectangle].box.top += (10 * scaler);
                                            rectangle[this_rectangle].box.bottom += (5 * scaler);
                                        }
                                }
                            }
                            if (((symbol->symbology == BARCODE_UPCA) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCA_CC)) {
                                /* guard bar extensions for UPCA */
                                if (((i >= 0) && (i <= 11)) || ((i >= 85) && (i <= 96))) {
                                    rectangle[this_rectangle].box.bottom += (5 * scaler);
                                }
                                if ((i == 46) || (i == 48)) {
                                    rectangle[this_rectangle].box.bottom += (5 * scaler);
                                }
                                if (i > 96) {
                                    /* Add-on */
                                    rectangle[this_rectangle].box.top += (10 * scaler);
                                    rectangle[this_rectangle].box.bottom += (5 * scaler);
                                }
                            }

                            if (((symbol->symbology == BARCODE_UPCE) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCE_CC)) {
                                /* guard bar extensions for UPCE */
                                switch (i) {
                                    case 0:
                                    case 2:
                                    case 46:
                                    case 48:
                                    case 50:
                                        rectangle[this_rectangle].box.bottom += (5 * scaler);
                                        break;
                                }
                                if (i > 50) {
                                    /* Add-on */
                                    rectangle[this_rectangle].box.top += (10 * scaler);
                                    rectangle[this_rectangle].box.bottom += (5 * scaler);
                                }
                            }
                        }

                        this_rectangle++;
                        latch = 0;
                    } else {
                        /* a space */
                        latch = 1;
                    }


                    i += block_width;
                } while (i < symbol->width);
            }
        } else {
            float ax, ay, bx, by, cx, cy, dx, dy, ex, ey, fx, fy, mx, my;
            /* Maxicode, use hexagons */

            /* Calculate bullseye */
            for(i = 0; i < 6; i++) {
                bullseye[i].type = 0x0000002a; // EMR_ELLIPSE
                bullseye[i].size = 24;
            }
            bullseye[0].box.top = (35.60 - 10.85) * scaler;
            bullseye[0].box.bottom = (35.60 + 10.85) * scaler;
            bullseye[0].box.left = (35.76 - 10.85) * scaler;
            bullseye[0].box.right = (35.76 + 10.85) * scaler;
            bullseye[1].box.top = (35.60 - 8.97) * scaler;
            bullseye[1].box.bottom = (35.60 + 8.97) * scaler;
            bullseye[1].box.left = (35.76 - 8.97) * scaler;
            bullseye[1].box.right = (35.76 + 8.97) * scaler;
            bullseye[2].box.top = (35.60 - 7.10) * scaler;
            bullseye[2].box.bottom = (35.60 + 7.10) * scaler;
            bullseye[2].box.left = (35.76 - 7.10) * scaler;
            bullseye[2].box.right = (35.76 + 7.10) * scaler;
            bullseye[3].box.top = (35.60 - 5.22) * scaler;
            bullseye[3].box.bottom = (35.60 + 5.22) * scaler;
            bullseye[3].box.left = (35.76 - 5.22) * scaler;
            bullseye[3].box.right = (35.76 + 5.22) * scaler;
            bullseye[4].box.top = (35.60 - 3.31) * scaler;
            bullseye[4].box.bottom = (35.60 + 3.31) * scaler;
            bullseye[4].box.left = (35.76 - 3.31) * scaler;
            bullseye[4].box.right = (35.76 + 3.31) * scaler;
            bullseye[5].box.top = (35.60 - 1.43) * scaler;
            bullseye[5].box.bottom = (35.60 + 1.43) * scaler;
            bullseye[5].box.left = (35.76 - 1.43) * scaler;
            bullseye[5].box.right = (35.76 + 1.43) * scaler;

            /* Plot hexagons */
            for(i = 0; i < symbol->width; i++) {
                if(module_is_set(symbol, this_row, i)) {
                    hexagon[this_hexagon].type = 0x00000003; // EMR_POLYGON
                    hexagon[this_hexagon].size = 76;
                    hexagon[this_hexagon].count = 6;

                    my = this_row * 2.135 + 1.43;
                    ay = my + 1.0 + yoffset;
                    by = my + 0.5 + yoffset;
                    cy = my - 0.5 + yoffset;
                    dy = my - 1.0 + yoffset;
                    ey = my - 0.5 + yoffset;
                    fy = my + 0.5 + yoffset;
                    if (this_row & 1) {
                        mx = (2.46 * i) + 1.23 + 1.23;
                    } else {
                        mx = (2.46 * i) + 1.23;
                    }
                    ax = mx + xoffset;
                    bx = mx + 0.86 + xoffset;
                    cx = mx + 0.86 + xoffset;
                    dx = mx + xoffset;
                    ex = mx - 0.86 + xoffset;
                    fx = mx - 0.86 + xoffset;

                    hexagon[this_hexagon].a_points_a.x = ax * scaler;
                    hexagon[this_hexagon].a_points_a.y = ay * scaler;
                    hexagon[this_hexagon].a_points_b.x = bx * scaler;
                    hexagon[this_hexagon].a_points_b.y = by * scaler;
                    hexagon[this_hexagon].a_points_c.x = cx * scaler;
                    hexagon[this_hexagon].a_points_c.y = cy * scaler;
                    hexagon[this_hexagon].a_points_d.x = dx * scaler;
                    hexagon[this_hexagon].a_points_d.y = dy * scaler;
                    hexagon[this_hexagon].a_points_e.x = ex * scaler;
                    hexagon[this_hexagon].a_points_e.y = ey * scaler;
                    hexagon[this_hexagon].a_points_f.x = fx * scaler;
                    hexagon[this_hexagon].a_points_f.y = fy * scaler;

                    hexagon[this_hexagon].bounds.top = hexagon[this_hexagon].a_points_d.y;
                    hexagon[this_hexagon].bounds.bottom = hexagon[this_hexagon].a_points_a.y;
                    hexagon[this_hexagon].bounds.left = hexagon[this_hexagon].a_points_e.x;
                    hexagon[this_hexagon].bounds.right = hexagon[this_hexagon].a_points_c.x;
                    this_hexagon++;
                    bytecount += 76;
                    recordcount++;
                }
            }
        }
    }

    if (symbol->output_options & BARCODE_BIND) {
        if ((symbol->rows > 1) && (is_stackable(symbol->symbology) == 1)) {
            /* row binding */
            for (i = 1; i < symbol->rows; i++) {
                    row_binding[i - 1].type = 0x0000002b; // EMR_RECTANGLE;
                    row_binding[i - 1].size = 24;
                    row_binding[i - 1].box.top = ((i * row_height) + yoffset - 1) * scaler;
                    row_binding[i - 1].box.bottom = row_binding[i - 1].box.top + (2 * scaler);

                    if (symbol->symbology != BARCODE_CODABLOCKF) {
                        row_binding[i - 1].box.left = xoffset * scaler;
                        row_binding[i - 1].box.right = emr_header.emf_header.bounds.right - (xoffset * scaler);
                    } else {
                        row_binding[i - 1].box.left = (xoffset + 11) * scaler;
                        row_binding[i - 1].box.right = emr_header.emf_header.bounds.right - ((xoffset + 14) * scaler);
                    }
                    bytecount += 24;
                    recordcount++;
            }
        }
    }

    /* Create EOF record */
    emr_eof.type = 0x0000000e; // EMR_EOF
    emr_eof.size = 18; // Assuming no palette entries
    emr_eof.n_pal_entries = 0;
    emr_eof.off_pal_entries = 0;
    emr_eof.size_last = emr_eof.size;
    bytecount += 18;
    recordcount++;

    /* Put final counts in header */
    emr_header.emf_header.bytes = bytecount;
    emr_header.emf_header.records = recordcount;

    /* Send EMF data to file */
    if (symbol->output_options & BARCODE_STDOUT) {
        emf_file = stdout;
    } else {
        emf_file = fopen(symbol->outfile, "wb");
    }
    if (emf_file == NULL) {
        strcpy(symbol->errtxt, "640: Could not open output file");
        return ZINT_ERROR_FILE_ACCESS;
    }

    fwrite(&emr_header, sizeof(emr_header_t), 1, emf_file);

    fwrite(&emr_createbrushindirect_fg, sizeof(emr_createbrushindirect_t), 1, emf_file);
    fwrite(&emr_createbrushindirect_bg, sizeof(emr_createbrushindirect_t), 1, emf_file);
    fwrite(&emr_createpen, sizeof(emr_createpen_t), 1, emf_file);

    if ((symbol->show_hrt != 0) && (ustrlen(local_text) != 0)) {
       fwrite(&emr_extcreatefontindirectw, sizeof(emr_extcreatefontindirectw_t), 1, emf_file);
    }

    fwrite(&emr_selectobject_bgbrush, sizeof(emr_selectobject_t), 1, emf_file);
    fwrite(&emr_selectobject_pen, sizeof(emr_selectobject_t), 1, emf_file);
    fwrite(&background, sizeof(emr_rectangle_t), 1, emf_file);

    fwrite(&emr_selectobject_fgbrush, sizeof(emr_selectobject_t), 1, emf_file);

    for (i = 0; i < rectangle_count; i++) {
        fwrite(&rectangle[i], sizeof(emr_rectangle_t), 1, emf_file);
    }
    for (i = 0; i < circle_count; i++) {
        fwrite(&circle[i], sizeof(emr_ellipse_t), 1, emf_file);
    }
    for (i = 0; i < hexagon_count; i++) {
        fwrite(&hexagon[i], sizeof(emr_polygon_t), 1, emf_file);
    }

    if ((symbol->output_options & BARCODE_BIND) || (symbol->output_options & BARCODE_BOX)) {
        fwrite(&box.top, sizeof(emr_rectangle_t), 1,  emf_file);
        fwrite(&box.bottom, sizeof(emr_rectangle_t), 1, emf_file);
        if (symbol->output_options & BARCODE_BOX) {
            fwrite(&box.left, sizeof(emr_rectangle_t), 1, emf_file);
            fwrite(&box.right, sizeof(emr_rectangle_t), 1, emf_file);
        }
    }

    if (symbol->output_options & BARCODE_BIND) {
        if ((symbol->rows > 1) && (is_stackable(symbol->symbology) == 1)) {
            for(i = 0; i < symbol->rows - 1; i++) {
                fwrite(&row_binding[i], sizeof(emr_rectangle_t), 1, emf_file);
            }
        }
    }

    if(symbol->symbology == BARCODE_MAXICODE) {
        fwrite(&bullseye[0], sizeof(emr_ellipse_t), 1, emf_file);
        fwrite(&emr_selectobject_bgbrush, sizeof(emr_selectobject_t), 1, emf_file);
        fwrite(&bullseye[1], sizeof(emr_ellipse_t), 1, emf_file);
        fwrite(&emr_selectobject_fgbrush, sizeof(emr_selectobject_t), 1, emf_file);
        fwrite(&bullseye[2], sizeof(emr_ellipse_t), 1, emf_file);
        fwrite(&emr_selectobject_bgbrush, sizeof(emr_selectobject_t), 1, emf_file);
        fwrite(&bullseye[3], sizeof(emr_ellipse_t), 1, emf_file);
        fwrite(&emr_selectobject_fgbrush, sizeof(emr_selectobject_t), 1, emf_file);
        fwrite(&bullseye[4], sizeof(emr_ellipse_t), 1, emf_file);
        fwrite(&emr_selectobject_bgbrush, sizeof(emr_selectobject_t), 1, emf_file);
        fwrite(&bullseye[5], sizeof(emr_ellipse_t), 1, emf_file);
    }

    if ((symbol->show_hrt != 0) && (ustrlen(local_text) != 0)) {
        if ((symbol->symbology == BARCODE_EANX) || (symbol->symbology == BARCODE_EANX_CC) || (symbol->symbology == BARCODE_ISBNX)) {
            if (ustrlen(regx) != 0) {
                /* EAN-13 */
                fwrite(&emr_selectobject_font, sizeof(emr_selectobject_t), 1, emf_file);
                fwrite(&emr_exttextoutw[0], sizeof(emr_exttextoutw_t), 1, emf_file);
                fwrite(&string_buffer, 2 * bump_up(ustrlen(local_text) + 1), 1, emf_file);
                for (i = 0; i < bump_up(ustrlen(local_text) + 1); i++) {
                    fwrite(&dx, 4, 1, emf_file);
                }
                fwrite(&emr_exttextoutw[1], sizeof(emr_exttextoutw_t), 1, emf_file);
                utfle_copy(output_buffer, regw, 6);
                fwrite(&output_buffer, 12, 1, emf_file);
                for (i = 0; i < 6; i++) {
                    fwrite(&dx, 4, 1, emf_file);
                }
                fwrite(&emr_exttextoutw[2], sizeof(emr_exttextoutw_t), 1, emf_file);
                utfle_copy(output_buffer, regx, 6);
                fwrite(&output_buffer, 12, 1, emf_file);
                for (i = 0; i < 6; i++) {
                    fwrite(&dx, 4, 1, emf_file);
                }
                if (ustrlen(regz) != 0) {
                    fwrite(&emr_exttextoutw[4], sizeof(emr_exttextoutw_t), 1, emf_file);
                    utfle_copy(output_buffer, regz, 6);
                    fwrite(&output_buffer, 12, 1, emf_file);
                    for (i = 0; i < 6; i++) {
                        fwrite(&dx, 4, 1, emf_file);
                    }
                }
                textdone = 1;
            } else if (ustrlen(regw) != 0) {
                /* EAN-8 */
                fwrite(&emr_selectobject_font, sizeof(emr_selectobject_t), 1, emf_file);
                fwrite(&emr_exttextoutw[0], sizeof(emr_exttextoutw_t), 1, emf_file);
                fwrite(&string_buffer, 2 * bump_up(ustrlen(local_text) + 1), 1, emf_file);
                for (i = 0; i < bump_up(ustrlen(local_text) + 1); i++) {
                    fwrite(&dx, 4, 1, emf_file);
                }
                fwrite(&emr_exttextoutw[1], sizeof(emr_exttextoutw_t), 1, emf_file);
                utfle_copy(output_buffer, regw, 6);
                fwrite(&output_buffer, 12, 1, emf_file);
                for (i = 0; i < 6; i++) {
                    fwrite(&dx, 4, 1, emf_file);
                }
                if (ustrlen(regz) != 0) {
                    fwrite(&emr_exttextoutw[4], sizeof(emr_exttextoutw_t), 1, emf_file);
                    utfle_copy(output_buffer, regz, 6);
                    fwrite(&output_buffer, 12, 1, emf_file);
                    for (i = 0; i < 6; i++) {
                        fwrite(&dx, 4, 1, emf_file);
                    }
                }
                textdone = 1;
            }
        }
        if ((symbol->symbology == BARCODE_UPCA) || (symbol->symbology == BARCODE_UPCA_CC)) {
            fwrite(&emr_selectobject_font, sizeof(emr_selectobject_t), 1, emf_file);
            fwrite(&emr_exttextoutw[0], sizeof(emr_exttextoutw_t), 1, emf_file);
            fwrite(&string_buffer, 2 * bump_up(ustrlen(local_text) + 1), 1, emf_file);
            for (i = 0; i < bump_up(ustrlen(local_text) + 1); i++) {
                fwrite(&dx, 4, 1, emf_file);
            }
            fwrite(&emr_exttextoutw[3], sizeof(emr_exttextoutw_t), 1, emf_file);
            utfle_copy(output_buffer, regy, 6);
            fwrite(&output_buffer, 12, 1, emf_file);
            for (i = 0; i < 6; i++) {
                fwrite(&dx, 4, 1, emf_file);
            }
            fwrite(&emr_selectobject_font_big, sizeof(emr_selectobject_t), 1, emf_file);
            fwrite(&emr_exttextoutw[1], sizeof(emr_exttextoutw_t), 1, emf_file);
            utfle_copy(output_buffer, regw, 6);
            fwrite(&output_buffer, 12, 1, emf_file);
            for (i = 0; i < 6; i++) {
                fwrite(&dx, 4, 1, emf_file);
            }
            fwrite(&emr_exttextoutw[2], sizeof(emr_exttextoutw_t), 1, emf_file);
            utfle_copy(output_buffer, regx, 6);
            fwrite(&output_buffer, 12, 1, emf_file);
            for (i = 0; i < 6; i++) {
                fwrite(&dx, 4, 1, emf_file);
            }
            if (ustrlen(regz) != 0) {
                fwrite(&emr_exttextoutw[4], sizeof(emr_exttextoutw_t), 1, emf_file);
                utfle_copy(output_buffer, regz, 6);
                fwrite(&output_buffer, 12, 1, emf_file);
                for (i = 0; i < 6; i++) {
                    fwrite(&dx, 4, 1, emf_file);
                }
            }
            textdone = 1;
        }

        if (((symbol->symbology == BARCODE_UPCE) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCE_CC)) {
            fwrite(&emr_selectobject_font, sizeof(emr_selectobject_t), 1, emf_file);
            fwrite(&emr_exttextoutw[0], sizeof(emr_exttextoutw_t), 1, emf_file);
            fwrite(&string_buffer, 2 * bump_up(ustrlen(local_text) + 1), 1, emf_file);
            for (i = 0; i < bump_up(ustrlen(local_text) + 1); i++) {
                fwrite(&dx, 4, 1, emf_file);
            }
            fwrite(&emr_exttextoutw[2], sizeof(emr_exttextoutw_t), 1, emf_file);
            utfle_copy(output_buffer, regx, 6);
            fwrite(&output_buffer, 12, 1, emf_file);
            for (i = 0; i < 6; i++) {
                fwrite(&dx, 4, 1, emf_file);
            }
            fwrite(&emr_selectobject_font_big, sizeof(emr_selectobject_t), 1, emf_file);
            fwrite(&emr_exttextoutw[1], sizeof(emr_exttextoutw_t), 1, emf_file);
            utfle_copy(output_buffer, regw, 6);
            fwrite(&output_buffer, 12, 1, emf_file);
            for (i = 0; i < 6; i++) {
                fwrite(&dx, 4, 1, emf_file);
            }
            if (ustrlen(regz) != 0) {
                fwrite(&emr_exttextoutw[4], sizeof(emr_exttextoutw_t), 1, emf_file);
                utfle_copy(output_buffer, regz, 6);
                fwrite(&output_buffer, 12, 1, emf_file);
                for (i = 0; i < 6; i++) {
                    fwrite(&dx, 4, 1, emf_file);
                }
            }
            textdone = 1;
        }

        if (textdone == 0) {
            fwrite(&emr_selectobject_font, sizeof(emr_selectobject_t), 1, emf_file);
            fwrite(&emr_exttextoutw[0], sizeof(emr_exttextoutw_t), 1, emf_file);
            fwrite(&string_buffer, 2 * bump_up(ustrlen(local_text) + 1), 1, emf_file);
            for (i = 0; i < bump_up(ustrlen(local_text) + 1); i++) {
                fwrite(&dx, 4, 1, emf_file);
            }
        }
    }

    fwrite(&emr_eof, sizeof(emr_eof_t), 1, emf_file);

    if (symbol->output_options & BARCODE_STDOUT) {
        fflush(emf_file);
    } else {
        fclose(emf_file);
    }
    return error_number;
}


