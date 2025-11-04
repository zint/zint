/*
    libzint - the open source barcode library
    Copyright (C) 2019-2025 Robin Stuart <rstuart114@gmail.com>

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

#include <math.h>
#include "testcommon.h"

/* Round to 3 decimal places (avoids rounding differences on various platforms) */
#define rnd3dpf(m) z_stripf(roundf((m) * 1000) / 1000)

static struct zint_vector_rect *find_rect(struct zint_symbol *symbol, float x, float y, float width, float height) {
    struct zint_vector_rect *rect;

    if (symbol->vector == NULL) {
        return NULL;
    }
    x = rnd3dpf(x);
    y = rnd3dpf(y);
    width = rnd3dpf(width);
    height = rnd3dpf(height);
    for (rect = symbol->vector->rectangles; rect != NULL; rect = rect->next) {
        /* printf("x %.9g, y %.9g, width %.9g, height %.9g\n", rect->x, rect->y, rect->width, rect->height); */
        if (rnd3dpf(rect->x) == x && rnd3dpf(rect->y) == y) {
            if (height && width) {
                if (rnd3dpf(rect->height) == height && rnd3dpf(rect->width) == width) {
                    break;
                }
            } else if (height) {
                if (rnd3dpf(rect->height) == height) {
                    break;
                }
            } else if (width) {
                if (rnd3dpf(rect->width) == width) {
                    break;
                }
            } else {
                break;
            }
        }
    }

    return rect;
}

static struct zint_vector_circle *find_circle(struct zint_symbol *symbol, float x, float y, float diameter) {
    struct zint_vector_circle *circle;

    if (symbol->vector == NULL) {
        return NULL;
    }
    x = rnd3dpf(x);
    y = rnd3dpf(y);
    diameter = rnd3dpf(diameter);
    for (circle = symbol->vector->circles; circle != NULL; circle = circle->next) {
        /* printf("x %.9g, y %.9g, diameter %.9g\n", circle->x, circle->y, circle->diameter); */
        if (rnd3dpf(circle->x) == x && rnd3dpf(circle->y) == y) {
            if (diameter) {
                if (rnd3dpf(circle->diameter) == diameter) {
                    break;
                }
            } else {
                break;
            }
        }
    }

    return circle;
}

static struct zint_vector_string *find_string(struct zint_symbol *symbol, float x, float y) {
    struct zint_vector_string *string;

    if (symbol->vector == NULL) {
        return NULL;
    }
    x = rnd3dpf(x);
    y = rnd3dpf(y);
    for (string = symbol->vector->strings; string != NULL; string = string->next) {
        /* printf("string->x %.9g, string->y %.9g\n", string->x, string->y); */
        if (rnd3dpf(string->x) == x && rnd3dpf(string->y) == y) {
            break;
        }
    }

    return string;
}

/* Helper to check string vectors */
static int check_vector_strings(const struct zint_symbol *symbol, char errmsg[128]) {
    const int has_hrt = ZBarcode_Cap(symbol->symbology, ZINT_CAP_HRT) == ZINT_CAP_HRT;
    const struct zint_vector *vector = symbol->vector;
    const struct zint_vector_string *string;
    int i, length;

    if (!vector) {
        strcpy(errmsg, "vector NULL");
        return 0;
    }
    if (symbol->show_hrt && has_hrt) {
        if (!symbol->vector->strings) {
            strcpy(errmsg, "vector->strings NULL");
            return 0;
        }
        for (string = symbol->vector->strings, i = 0; string; string = string->next, i++) {
            if (string->x < 0.0f) {
                sprintf(errmsg, "string[%d]->x %g negative", i, string->x);
                return 0;
            }
            if (string->x >= vector->width) {
                sprintf(errmsg, "string[%d]->x %g >= vector->width %g", i, string->x, vector->width);
                return 0;
            }
            if (string->y < 0.0f) {
                sprintf(errmsg, "string[%d]->y %g negative", i, string->y);
                return 0;
            }
            if (string->y >= vector->height) {
                sprintf(errmsg, "string[%d]->y %g >= vector->height %g", i, string->y, vector->height);
                return 0;
            }
            if (string->fsize < 1.0f) {
                sprintf(errmsg, "string[%d]->fsize %g < 1", i, string->fsize);
                return 0;
            }
            if (string->width <= 0.0f) {
                sprintf(errmsg, "string[%d]->width %g zero or negative", i, string->width);
                return 0;
            }
            if (string->rotation != 0 && string->rotation != 90 && string->rotation != 180 && string->rotation != 270) {
                sprintf(errmsg, "string[%d]->rotation %d not 0, 90, 180, 270", i, string->rotation);
                return 0;
            }
            if (!string->text) {
                sprintf(errmsg, "string[%d]->text NULL", i);
                return 0;
            }
            length = (int) z_ustrlen(string->text);
            if (string->length != length) {
                sprintf(errmsg, "string[%d]->length != %d", string->length, length);
                return 0;
            }
        }
        if ((ZBarcode_Cap(symbol->symbology, ZINT_CAP_EANUPC) & ZINT_CAP_EANUPC) == ZINT_CAP_EANUPC) {
            const unsigned char *addon = (const unsigned char *) strchr((const char *) symbol->text, '+');
            const int has_addon = addon != NULL;
            const int text_len = has_addon ? (int) (addon - symbol->text) : (int) z_ustrlen(symbol->text);
            int num = -1;
            switch (symbol->symbology) {
                case BARCODE_EANX:
                case BARCODE_EANX_CHK:
                case BARCODE_ISBNX:
                case BARCODE_EANX_CC:
                    num = text_len <= 8 ? text_len <= 5 ? 1 : 2 : 3;
                    break;
                case BARCODE_EAN8:
                case BARCODE_EAN8_CC:
                    num = 2;
                    break;
                case BARCODE_EAN_2ADDON:
                case BARCODE_EAN_5ADDON:
                    num = 1;
                    break;
                case BARCODE_EAN13:
                case BARCODE_EAN13_CC:
                    num = 3;
                    break;
                case BARCODE_UPCA:
                case BARCODE_UPCA_CHK:
                case BARCODE_UPCA_CC:
                    num = 4;
                    break;
                case BARCODE_UPCE:
                case BARCODE_UPCE_CHK:
                case BARCODE_UPCE_CC:
                    num = 3;
                    break;
                default:
                    strcpy(errmsg, "unhandled EAN/UPC");
                    return 0;
                    break;
            }
            if (i != num + has_addon) {
                printf("num %d, text_len %d, has_addon %d\n", num, text_len, has_addon);
                sprintf(errmsg, "no. of strings %d != %d", i, num + has_addon);
                return 0;
            }
        } else {
            if (i > 1) {
                sprintf(errmsg, "no. of strings %d > 1", i);
                return 0;
            }
        }
    } else {
        if (symbol->vector->strings) {
            strcpy(errmsg, "vector->strings not NULL");
            return 0;
        }
    }
    return 1;
}

/* Helper to check rectangle vectors */
static int check_vector_rectangles(const struct zint_symbol *symbol, char errmsg[128]) {
    const struct zint_vector *vector = symbol->vector;
    const struct zint_vector_rect *rect;
    const int have_border = symbol->border_width && (symbol->output_options & (BARCODE_BIND | BARCODE_BOX | BARCODE_BIND_TOP));
    const int dotty = z_is_dotty(symbol->symbology) && (symbol->output_options & BARCODE_DOTTY_MODE);
    int i;

    if (!vector) {
        strcpy(errmsg, "vector NULL");
        return 0;
    }
    if (have_border) {
        if (!vector->rectangles) {
            strcpy(errmsg, "rectangles NULL");
            return 0;
        }
    } else {
        if (symbol->symbology == BARCODE_DOTCODE || symbol->symbology == BARCODE_MAXICODE) {
            if (vector->rectangles) {
                strcpy(errmsg, "rectangles not NULL");
                return 0;
            }
        } else {
            if (!dotty) {
                if (!vector->rectangles) {
                    strcpy(errmsg, "rectangles NULL");
                    return 0;
                }
            }
        }
    }
    for (rect = vector->rectangles, i = 0; rect; rect = rect->next, i++) {
        if (rect->x < 0.0f) {
            sprintf(errmsg, "rect[%d]->x %g negative", i, rect->x);
            return 0;
        }
        if (rect->x >= vector->width) {
            sprintf(errmsg, "rect[%d]->x %g >= vector->width %g", i, rect->x, vector->width);
            return 0;
        }
        if (rect->y < 0.0f) {
            sprintf(errmsg, "rect[%d]->y %g negative", i, rect->y);
            return 0;
        }
        if (rect->y >= vector->height) {
            sprintf(errmsg, "rect[%d]->y %g >= vector->height %g", i, rect->y, vector->height);
            return 0;
        }
        if (rect->width <= 0.0f) {
            sprintf(errmsg, "rect[%d]->width %g <= 0", i, rect->width);
            return 0;
        }
        if (rect->width > vector->width) {
            sprintf(errmsg, "rect[%d]->width %g > vector->width %g", i, rect->width, vector->width);
            return 0;
        }
        if (z_stripf(rect->x + rect->width) > vector->width) {
            sprintf(errmsg, "rect[%d]->x + width %g > vector->width %g", i, rect->x + rect->width, vector->width);
            return 0;
        }
        if (rect->height <= 0.0f) {
            sprintf(errmsg, "rect[%d]->height %g <= 0", i, rect->height);
            return 0;
        }
        if (rect->height > vector->height) {
            sprintf(errmsg, "rect[%d]->height %g > vector->height %g", i, rect->height, vector->height);
            return 0;
        }
        if (z_stripf(rect->y + rect->height) > vector->height) {
            sprintf(errmsg, "rect[%d]->y + height %g > vector->height %g", i, rect->y + rect->height, vector->height);
            return 0;
        }

        if (symbol->symbology == BARCODE_ULTRA) {
            if (rect->colour != -1 && (rect->colour < 1 || rect->colour > 8)) {
                sprintf(errmsg, "rect[%d]->colour %d != -1 and not 1-8", i, rect->colour);
                return 0;
            }
        } else {
            if (rect->colour != -1) {
                sprintf(errmsg, "rect[%d]->colour %d != -1", i, rect->colour);
                return 0;
            }
        }
    }
    return 1;
}

/* Helper to check circle vectors */
static int check_vector_circles(const struct zint_symbol *symbol, char errmsg[128]) {
    const struct zint_vector *vector = symbol->vector;
    const struct zint_vector_circle *circle;
    const int dotty = z_is_dotty(symbol->symbology) && (symbol->output_options & BARCODE_DOTTY_MODE);
    int i;

    if (!vector) {
        strcpy(errmsg, "vector NULL");
        return 0;
    }
    if (symbol->symbology == BARCODE_DOTCODE || symbol->symbology == BARCODE_MAXICODE) {
        if (!vector->circles) {
            strcpy(errmsg, "circles NULL");
            return 0;
        }
    } else {
        if (vector->circles) {
            if (!dotty) {
                strcpy(errmsg, "circles not NULL");
                return 0;
            }
        } else {
            if (dotty) {
                strcpy(errmsg, "circles NULL");
                return 0;
            }
        }
    }
    for (circle = vector->circles, i = 0; circle; circle = circle->next, i++) {
        if (circle->x < 0.0f) {
            sprintf(errmsg, "circle[%d]->x %g negative", i, circle->x);
            return 0;
        }
        if (circle->x >= vector->width) {
            sprintf(errmsg, "circle[%d]->x %g >= vector->width %g", i, circle->x, vector->width);
            return 0;
        }
        if (circle->y < 0.0f) {
            sprintf(errmsg, "circle[%d]->y %g negative", i, circle->y);
            return 0;
        }
        if (circle->y >= vector->height) {
            sprintf(errmsg, "circle[%d]->y %g >= vector->height %g", i, circle->y, vector->height);
            return 0;
        }
        if (circle->diameter <= 0.0f) {
            sprintf(errmsg, "circle[%d]->diameter %g <= 0", i, circle->diameter);
            return 0;
        }
        if (circle->diameter > vector->width) {
            sprintf(errmsg, "circle[%d]->diameter %g > vector->diameter %g", i, circle->diameter, vector->width);
            return 0;
        }
        if (circle->diameter > vector->height) {
            sprintf(errmsg, "circle[%d]->diameter %g > vector->height %g", i, circle->diameter, vector->height);
            return 0;
        }
        if (z_stripf(circle->x + circle->diameter / 2) > vector->width) {
            sprintf(errmsg, "circle[%d]->x + diameter / 2 %g > vector->width %g", i, circle->x + circle->diameter / 2, vector->width);
            return 0;
        }
        if (z_stripf(circle->y + circle->diameter / 2) > vector->height) {
            sprintf(errmsg, "circle[%d]->y + diameter / 2 %g > vector->height %g", i, circle->y + circle->diameter / 2, vector->height);
            return 0;
        }

        if (circle->colour != 0) {
            sprintf(errmsg, "circle[%d]->colour %d != 0", i, circle->colour);
            return 0;
        }
    }
    return 1;
}

/* Helper to check hexagon vectors */
static int check_vector_hexagons(const struct zint_symbol *symbol, char errmsg[128]) {
    const struct zint_vector *vector = symbol->vector;
    const struct zint_vector_hexagon *hex;
    int i;

    if (!vector) {
        strcpy(errmsg, "vector NULL");
        return 0;
    }
    if (symbol->symbology == BARCODE_MAXICODE) {
        if (!vector->hexagons) {
            strcpy(errmsg, "hexagons NULL");
            return 0;
        }
    } else {
        if (vector->hexagons) {
            strcpy(errmsg, "hexagons not NULL");
            return 0;
        }
    }
    for (hex = vector->hexagons, i = 0; hex; hex = hex->next, i++) {
        if (hex->x < 0.0f) {
            sprintf(errmsg, "hex[%d]->x %g negative", i, hex->x);
            return 0;
        }
        if (hex->x >= vector->width) {
            sprintf(errmsg, "hex[%d]->x %g >= vector->width %g", i, hex->x, vector->width);
            return 0;
        }
        if (hex->y < 0.0f) {
            sprintf(errmsg, "hex[%d]->y %g negative", i, hex->y);
            return 0;
        }
        if (hex->y >= vector->height) {
            sprintf(errmsg, "hex[%d]->y %g >= vector->height %g", i, hex->y, vector->height);
            return 0;
        }
        if (hex->diameter <= 0.0f) {
            sprintf(errmsg, "hex[%d]->diameter %g <= 0", i, hex->diameter);
            return 0;
        }
        if (hex->diameter > vector->width) {
            sprintf(errmsg, "hex[%d]->diameter %g > vector->diameter %g", i, hex->diameter, vector->width);
            return 0;
        }
        if (hex->diameter > vector->height) {
            sprintf(errmsg, "hex[%d]->diameter %g > vector->height %g", i, hex->diameter, vector->height);
            return 0;
        }
        if (z_stripf(hex->x + hex->diameter / 2) > vector->width) {
            sprintf(errmsg, "hex[%d]->x + diameter / 2 %g > vector->width %g", i, hex->x + hex->diameter / 2, vector->width);
            return 0;
        }
        if (z_stripf(hex->y + hex->diameter / 2) > vector->height) {
            sprintf(errmsg, "hex[%d]->y + diameter / 2 %g > vector->height %g", i, hex->y + hex->diameter / 2, vector->height);
            return 0;
        }

        if (hex->rotation != 0 && hex->rotation != 90 && hex->rotation != 180 && hex->rotation != 270) {
            sprintf(errmsg, "hex[%d]->rotation %d not 0, 90, 180, 270", i, hex->rotation);
            return 0;
        }
    }
    return 1;
}

/* Helper to check vectors */
static int check_vectors(const struct zint_symbol *symbol, char errmsg[128]) {
    if (!check_vector_strings(symbol, errmsg)) {
        return 0;
    }
    if (!check_vector_rectangles(symbol, errmsg)) {
        return 0;
    }
    if (!check_vector_circles(symbol, errmsg)) {
        return 0;
    }
    if (!check_vector_hexagons(symbol, errmsg)) {
        return 0;
    }
    return 1;
}

/* Return count of strings */
static int cnt_strings(const struct zint_symbol *symbol) {
    const struct zint_vector_string *string;
    int cnt = 0;

    if (symbol->vector) {
        for (string = symbol->vector->strings; string; string = string->next, cnt++);
    }
    return cnt;
}

static void test_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *fgcolour;
        const char *bgcolour;
        int rotate_angle;
        const char *data;
        int ret;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "123456", "7890ab", 0, "A", 0, 1, 46, 92, 116.28 },
        /*  1*/ { BARCODE_CODE128, "12345", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  2*/ { BARCODE_CODE128, NULL, "1234567", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  3*/ { BARCODE_CODE128, "12345 ", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  4*/ { BARCODE_CODE128, NULL, "EEFFGG", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODE128, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        if (data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }

        ret = ZBarcode_Encode_and_Buffer_Vector(symbol, (unsigned char *) data[i].data, length, data[i].rotate_angle);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_buffer_vector(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *data;
        const char *composite;
        int ret;
        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, "1234567890", "", 0, 50, 1, 108, 216, 116.28 },
        /*  1*/ { BARCODE_C25STANDARD, "1234567890", "", 0, 50, 1, 117, 234, 116.28 },
        /*  2*/ { BARCODE_C25INTER, "1234567890", "", 0, 50, 1, 99, 198, 116.28 },
        /*  3*/ { BARCODE_C25IATA, "1234567890", "", 0, 50, 1, 149, 298, 116.28 },
        /*  4*/ { BARCODE_C25LOGIC, "1234567890", "", 0, 50, 1, 109, 218, 116.28 },
        /*  5*/ { BARCODE_C25IND, "1234567890", "", 0, 50, 1, 159, 318, 116.28 },
        /*  6*/ { BARCODE_CODE39, "1234567890", "", 0, 50, 1, 155, 310, 116.28 },
        /*  7*/ { BARCODE_EXCODE39, "1234567890", "", 0, 50, 1, 155, 310, 116.28 },
        /*  8*/ { BARCODE_EAN13, "123456789012", "", 0, 50, 1, 95, 226, 118 },
        /*  9*/ { BARCODE_EANX, "123456789012", "", 0, 50, 1, 95, 226, 118 },
        /* 10*/ { BARCODE_EAN13, "1234567890128", "", 0, 50, 1, 95, 226, 118 },
        /* 11*/ { BARCODE_EANX_CHK, "1234567890128", "", 0, 50, 1, 95, 226, 118 },
        /* 12*/ { BARCODE_EAN13, "123456789012+12", "", 0, 50, 1, 122, 276, 118 },
        /* 13*/ { BARCODE_EANX, "123456789012+12", "", 0, 50, 1, 122, 276, 118 },
        /* 14*/ { BARCODE_EAN13, "1234567890128+12", "", 0, 50, 1, 122, 276, 118 },
        /* 15*/ { BARCODE_EANX_CHK, "1234567890128+12", "", 0, 50, 1, 122, 276, 118 },
        /* 16*/ { BARCODE_EAN13, "123456789012+12345", "", 0, 50, 1, 149, 330, 118 },
        /* 17*/ { BARCODE_EANX, "123456789012+12345", "", 0, 50, 1, 149, 330, 118 },
        /* 18*/ { BARCODE_EAN13, "1234567890128+12345", "", 0, 50, 1, 149, 330, 118 },
        /* 19*/ { BARCODE_EANX_CHK, "1234567890128+12345", "", 0, 50, 1, 149, 330, 118 },
        /* 20*/ { BARCODE_EAN8, "1234567", "", 0, 50, 1, 67, 162, 118 },
        /* 21*/ { BARCODE_EANX, "1234567", "", 0, 50, 1, 67, 162, 118 },
        /* 22*/ { BARCODE_EAN8, "12345670", "", 0, 50, 1, 67, 162, 118 },
        /* 23*/ { BARCODE_EANX_CHK, "12345670", "", 0, 50, 1, 67, 162, 118 },
        /* 24*/ { BARCODE_EAN8, "1234567+12", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 118 },
        /* 25*/ { BARCODE_EANX, "1234567+12", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 118 },
        /* 26*/ { BARCODE_EAN8, "12345670+12", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 118 },
        /* 27*/ { BARCODE_EANX_CHK, "12345670+12", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 118 },
        /* 28*/ { BARCODE_EAN8, "1234567+12345", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 118 },
        /* 29*/ { BARCODE_EANX, "1234567+12345", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 118 },
        /* 30*/ { BARCODE_EAN8, "12345670+12345", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 118 },
        /* 31*/ { BARCODE_EANX_CHK, "12345670+12345", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 118 },
        /* 32*/ { BARCODE_EAN_5ADDON, "1234", "", 0, 50, 1, 47, 104, 118 },
        /* 33*/ { BARCODE_EANX, "1234", "", 0, 50, 1, 47, 104, 118 },
        /* 34*/ { BARCODE_EANX_CHK, "1234", "", 0, 50, 1, 47, 104, 118 },
        /* 35*/ { BARCODE_EAN_2ADDON, "12", "", 0, 50, 1, 20, 50, 118 },
        /* 36*/ { BARCODE_EANX, "12", "", 0, 50, 1, 20, 50, 118 },
        /* 37*/ { BARCODE_EANX_CHK, "12", "", 0, 50, 1, 20, 50, 118 },
        /* 38*/ { BARCODE_GS1_128, "[01]12345678901231", "", 0, 50, 1, 134, 268, 116.28 },
        /* 39*/ { BARCODE_CODABAR, "A00000000B", "", 0, 50, 1, 102, 204, 116.28 },
        /* 40*/ { BARCODE_CODE128, "1234567890", "", 0, 50, 1, 90, 180, 116.28 },
        /* 41*/ { BARCODE_DPLEIT, "1234567890123", "", 0, 72, 1, 135, 270, 160.28 },
        /* 42*/ { BARCODE_DPIDENT, "12345678901", "", 0, 72, 1, 117, 234, 160.28 },
        /* 43*/ { BARCODE_CODE16K, "1234567890", "", 0, 20, 2, 70, 162, 44 },
        /* 44*/ { BARCODE_CODE49, "1234567890", "", 0, 20, 2, 70, 162, 44 },
        /* 45*/ { BARCODE_CODE93, "1234567890", "", 0, 50, 1, 127, 254, 116.28 },
        /* 46*/ { BARCODE_FLAT, "1234567890", "", 0, 50, 1, 90, 180, 100 },
        /* 47*/ { BARCODE_DBAR_OMN, "1234567890123", "", 0, 50, 1, 96, 192, 116.28 },
        /* 48*/ { BARCODE_DBAR_LTD, "1234567890123", "", 0, 50, 1, 79, 158, 116.28 },
        /* 49*/ { BARCODE_DBAR_EXP, "[01]12345678901231", "", 0, 34, 1, 134, 268, 84.279999 },
        /* 50*/ { BARCODE_TELEPEN, "1234567890", "", 0, 50, 1, 208, 416, 116.28 },
        /* 51*/ { BARCODE_UPCA, "12345678901", "", 0, 50, 1, 95, 226, 118 },
        /* 52*/ { BARCODE_UPCA_CHK, "123456789012", "", 0, 50, 1, 95, 226, 118 },
        /* 53*/ { BARCODE_UPCA, "12345678901+12", "", 0, 50, 1, 124, 276, 118 },
        /* 54*/ { BARCODE_UPCA_CHK, "123456789012+12", "", 0, 50, 1, 124, 276, 118 },
        /* 55*/ { BARCODE_UPCA, "12345678901+12345", "", 0, 50, 1, 151, 330, 118 },
        /* 56*/ { BARCODE_UPCA_CHK, "123456789012+12345", "", 0, 50, 1, 151, 330, 118 },
        /* 57*/ { BARCODE_UPCE, "1234567", "", 0, 50, 1, 51, 134, 118 },
        /* 58*/ { BARCODE_UPCE_CHK, "12345670", "", 0, 50, 1, 51, 134, 118 },
        /* 59*/ { BARCODE_UPCE, "1234567+12", "", 0, 50, 1, 78, 184, 118 },
        /* 60*/ { BARCODE_UPCE_CHK, "12345670+12", "", 0, 50, 1, 78, 184, 118 },
        /* 61*/ { BARCODE_UPCE, "1234567+12345", "", 0, 50, 1, 105, 238, 118 },
        /* 62*/ { BARCODE_UPCE_CHK, "12345670+12345", "", 0, 50, 1, 105, 238, 118 },
        /* 63*/ { BARCODE_POSTNET, "12345678901", "", 0, 12, 2, 123, 246, 24 },
        /* 64*/ { BARCODE_MSI_PLESSEY, "1234567890", "", 0, 50, 1, 127, 254, 116.28 },
        /* 65*/ { BARCODE_FIM, "A", "", 0, 50, 1, 17, 34, 100 },
        /* 66*/ { BARCODE_LOGMARS, "1234567890", "", 0, 50, 1, 191, 382, 116.28 },
        /* 67*/ { BARCODE_PHARMA, "123456", "", 0, 50, 1, 58, 116, 100 },
        /* 68*/ { BARCODE_PZN, "123456", "", 0, 50, 1, 142, 284, 116.28 },
        /* 69*/ { BARCODE_PHARMA_TWO, "12345678", "", 0, 10, 2, 29, 58, 20 },
        /* 70*/ { BARCODE_CEPNET, "12345678", "", 0, 5.375, 2, 93, 186, 10.75 },
        /* 71*/ { BARCODE_PDF417, "1234567890", "", 0, 21, 7, 103, 206, 42 },
        /* 72*/ { BARCODE_PDF417COMP, "1234567890", "", 0, 21, 7, 69, 138, 42 },
        /* 73*/ { BARCODE_MAXICODE, "1234567890", "", 0, 165, 33, 30, 60, 57.7333984 },
        /* 74*/ { BARCODE_QRCODE, "1234567890AB", "", 0, 21, 21, 21, 42, 42 },
        /* 75*/ { BARCODE_CODE128AB, "1234567890", "", 0, 50, 1, 145, 290, 116.28 },
        /* 76*/ { BARCODE_AUSPOST, "12345678901234567890123", "", 0, 8, 3, 133, 266, 16 },
        /* 77*/ { BARCODE_AUSREPLY, "12345678", "", 0, 8, 3, 73, 146, 16 },
        /* 78*/ { BARCODE_AUSROUTE, "12345678", "", 0, 8, 3, 73, 146, 16 },
        /* 79*/ { BARCODE_AUSREDIRECT, "12345678", "", 0, 8, 3, 73, 146, 16 },
        /* 80*/ { BARCODE_ISBNX, "123456789", "", 0, 50, 1, 95, 226, 118 },
        /* 81*/ { BARCODE_ISBNX, "123456789+12", "", 0, 50, 1, 122, 276, 118 },
        /* 82*/ { BARCODE_ISBNX, "123456789+12345", "", 0, 50, 1, 149, 330, 118 },
        /* 83*/ { BARCODE_RM4SCC, "1234567890", "", 0, 8, 3, 91, 182, 16 },
        /* 84*/ { BARCODE_DATAMATRIX, "ABC", "", 0, 10, 10, 10, 20, 20 },
        /* 85*/ { BARCODE_EAN14, "1234567890123", "", 0, 50, 1, 134, 268, 116.28 },
        /* 86*/ { BARCODE_VIN, "12345678701234567", "", 0, 50, 1, 246, 492, 116.28 },
        /* 87*/ { BARCODE_CODABLOCKF, "1234567890", "", 0, 20, 2, 101, 242, 44 },
        /* 88*/ { BARCODE_NVE18, "12345678901234567", "", 0, 50, 1, 156, 312, 116.28 },
        /* 89*/ { BARCODE_JAPANPOST, "1234567890", "", 0, 8, 3, 133, 266, 16 },
        /* 90*/ { BARCODE_KOREAPOST, "123456", "", 0, 50, 1, 167, 334, 116.28 },
        /* 91*/ { BARCODE_DBAR_STK, "1234567890123", "", 0, 13, 3, 50, 100, 26 },
        /* 92*/ { BARCODE_DBAR_OMNSTK, "1234567890123", "", 0, 69, 5, 50, 100, 138 },
        /* 93*/ { BARCODE_DBAR_EXPSTK, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142 },
        /* 94*/ { BARCODE_PLANET, "12345678901", "", 0, 12, 2, 123, 246, 24 },
        /* 95*/ { BARCODE_MICROPDF417, "1234567890", "", 0, 12, 6, 82, 164, 24 },
        /* 96*/ { BARCODE_USPS_IMAIL, "12345678901234567890", "", 0, 8, 3, 129, 258, 16 },
        /* 97*/ { BARCODE_PLESSEY, "1234567890", "", 0, 50, 1, 227, 454, 116.28 },
        /* 98*/ { BARCODE_TELEPEN_NUM, "1234567890", "", 0, 50, 1, 128, 256, 116.28 },
        /* 99*/ { BARCODE_ITF14, "1234567890", "", 0, 50, 1, 135, 330, 136.28 },
        /*100*/ { BARCODE_KIX, "123456ABCDE", "", 0, 8, 3, 87, 174, 16 },
        /*101*/ { BARCODE_AZTEC, "1234567890AB", "", 0, 15, 15, 15, 30, 30 },
        /*102*/ { BARCODE_DAFT, "DAFTDAFTDAFTDAFT", "", 0, 8, 3, 31, 62, 16 },
        /*103*/ { BARCODE_DPD, "0123456789012345678901234567", "", 0, 50, 1, 189, 378, 122.28 },
        /*104*/ { BARCODE_MICROQR, "12345", "", 0, 11, 11, 11, 22, 22 },
        /*105*/ { BARCODE_HIBC_128, "1234567890", "", 0, 50, 1, 123, 246, 116.28 },
        /*106*/ { BARCODE_HIBC_39, "1234567890", "", 0, 50, 1, 223, 446, 116.28 },
        /*107*/ { BARCODE_HIBC_DM, "ABC", "", 0, 12, 12, 12, 24, 24 },
        /*108*/ { BARCODE_HIBC_QR, "1234567890AB", "", 0, 21, 21, 21, 42, 42 },
        /*109*/ { BARCODE_HIBC_PDF, "1234567890", "", 0, 24, 8, 103, 206, 48 },
        /*110*/ { BARCODE_HIBC_MICPDF, "1234567890", "", 0, 28, 14, 38, 76, 56 },
        /*111*/ { BARCODE_HIBC_BLOCKF, "1234567890", "", 0, 30, 3, 101, 242, 64 },
        /*112*/ { BARCODE_HIBC_AZTEC, "1234567890AB", "", 0, 19, 19, 19, 38, 38 },
        /*113*/ { BARCODE_DOTCODE, "ABC", "", 0, 11, 11, 16, 32, 22 },
        /*114*/ { BARCODE_HANXIN, "1234567890AB", "", 0, 23, 23, 23, 46, 46 },
        /*115*/ { BARCODE_MAILMARK_2D, "012100123412345678AB19XY1A 0", "", 0, 24, 24, 24, 48, 48 },
        /*116*/ { BARCODE_UPU_S10, "EE876543216CA", "", 0, 50, 1, 156, 312, 116.28 },
        /*117*/ { BARCODE_MAILMARK_4S, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20 },
        /*118*/ { BARCODE_AZRUNE, "255", "", 0, 11, 11, 11, 22, 22 },
        /*119*/ { BARCODE_CODE32, "12345678", "", 0, 50, 1, 103, 206, 116.28 },
        /*120*/ { BARCODE_EAN13_CC, "123456789012", "[20]01", 0, 50, 7, 99, 226, 118 },
        /*121*/ { BARCODE_EANX_CC, "123456789012", "[20]01", 0, 50, 7, 99, 226, 118 },
        /*122*/ { BARCODE_EAN13_CC, "123456789012+12", "[20]01", 0, 50, 7, 125, 276, 118 },
        /*123*/ { BARCODE_EANX_CC, "123456789012+12", "[20]01", 0, 50, 7, 125, 276, 118 },
        /*124*/ { BARCODE_EAN13_CC, "123456789012+12345", "[20]01", 0, 50, 7, 152, 330, 118 },
        /*125*/ { BARCODE_EANX_CC, "123456789012+12345", "[20]01", 0, 50, 7, 152, 330, 118 },
        /*126*/ { BARCODE_EAN8_CC, "1234567", "[20]01", 0, 50, 8, 72, 162, 118 },
        /*127*/ { BARCODE_EANX_CC, "1234567", "[20]01", 0, 50, 8, 72, 162, 118 },
        /*128*/ { BARCODE_EAN8_CC, "1234567+12", "[20]01", ZINT_WARN_NONCOMPLIANT, 50, 8, 98, 212, 118 },
        /*129*/ { BARCODE_EANX_CC, "1234567+12", "[20]01", ZINT_WARN_NONCOMPLIANT, 50, 8, 98, 212, 118 },
        /*130*/ { BARCODE_EAN8_CC, "1234567+12345", "[20]01", ZINT_WARN_NONCOMPLIANT, 50, 8, 125, 266, 118 },
        /*131*/ { BARCODE_EANX_CC, "1234567+12345", "[20]01", ZINT_WARN_NONCOMPLIANT, 50, 8, 125, 266, 118 },
        /*132*/ { BARCODE_GS1_128_CC, "[01]12345678901231", "[20]01", 0, 50, 5, 145, 290, 116.28 },
        /*133*/ { BARCODE_DBAR_OMN_CC, "1234567890123", "[20]01", 0, 21, 5, 100, 200, 58.279999 },
        /*134*/ { BARCODE_DBAR_LTD_CC, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 54.279999 },
        /*135*/ { BARCODE_DBAR_EXP_CC, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 98.279999 },
        /*136*/ { BARCODE_UPCA_CC, "12345678901", "[20]01", 0, 50, 7, 99, 226, 118 },
        /*137*/ { BARCODE_UPCA_CC, "12345678901+12", "[20]01", 0, 50, 7, 127, 276, 118 },
        /*138*/ { BARCODE_UPCA_CC, "12345678901+12345", "[20]01", 0, 50, 7, 154, 330, 118 },
        /*139*/ { BARCODE_UPCE_CC, "1234567", "[20]01", 0, 50, 9, 55, 134, 118 },
        /*140*/ { BARCODE_UPCE_CC, "1234567+12", "[20]01", 0, 50, 9, 81, 184, 118 },
        /*141*/ { BARCODE_UPCE_CC, "1234567+12345", "[20]01", 0, 50, 9, 108, 238, 118 },
        /*142*/ { BARCODE_DBAR_STK_CC, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48 },
        /*143*/ { BARCODE_DBAR_OMNSTK_CC, "1234567890123", "[20]01", 0, 80, 11, 56, 112, 160 },
        /*144*/ { BARCODE_DBAR_EXPSTK_CC, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156 },
        /*145*/ { BARCODE_CHANNEL, "01", "", 0, 50, 1, 19, 38, 116.28 },
        /*146*/ { BARCODE_CODEONE, "12345678901234567890", "", 0, 16, 16, 18, 36, 32 },
        /*147*/ { BARCODE_GRIDMATRIX, "ABC", "", 0, 18, 18, 18, 36, 36 },
        /*148*/ { BARCODE_UPNQR, "1234567890AB", "", 0, 77, 77, 77, 154, 154 },
        /*149*/ { BARCODE_ULTRA, "1234567890", "", 0, 13, 13, 18, 36, 26 },
        /*150*/ { BARCODE_RMQR, "12345", "", 0, 11, 11, 27, 54, 22 },
        /*151*/ { BARCODE_BC412, "1234567", "", 0, 16.666666, 1, 102, 204, 49.613335 },
        /*152*/ { BARCODE_DXFILMEDGE, "120476", "", 0, 6, 2, 23, 46, 12 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;
    int ret_buf;
    char errmsg[128] = {0}; /* Suppress clang -fsanitize=memory false positive */

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;
        symbol->debug |= debug;

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = (int) strlen(text);

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%s) ret %d != %d (%s)\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        ret_buf = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret_buf, "i:%d ZBarcode_Buffer_Vector(%d) ret_buf %d != 0\n", i, data[i].symbology, ret_buf);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, \"%s\", \"%s\", %s, %.9g, %d, %d, %.9g, %.9g },\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].data, data[i].composite,
                    testUtilErrorName(ret),
                    symbol->height, symbol->rows, symbol->width, symbol->vector->width, symbol->vector->height);
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.9g != %.9g\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->vector->width, data[i].expected_vector_width,
                        "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->vector->width,
                        data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height,
                        "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->vector->height,
                        data[i].expected_vector_height);

            assert_nonzero(check_vectors(symbol, errmsg), "i:%d (%s) %s\n",
                i, testUtilBarcodeName(data[i].symbology), errmsg);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_has_hrt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *data;
        const char *composite;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, "1234567890", "" },
        /*  1*/ { BARCODE_C25STANDARD, "1234567890", "" },
        /*  2*/ { BARCODE_C25INTER, "1234567890", "" },
        /*  3*/ { BARCODE_C25IATA, "1234567890", "" },
        /*  4*/ { BARCODE_C25LOGIC, "1234567890", "" },
        /*  5*/ { BARCODE_C25IND, "1234567890", "" },
        /*  6*/ { BARCODE_CODE39, "1234567890", "" },
        /*  7*/ { BARCODE_EXCODE39, "1234567890", "" },
        /*  8*/ { BARCODE_EAN8, "12345670", "" },
        /*  9*/ { BARCODE_EAN_2ADDON, "12", "" },
        /* 10*/ { BARCODE_EAN_5ADDON, "12345", "" },
        /* 11*/ { BARCODE_EANX, "123456789012", "" },
        /* 12*/ { BARCODE_EANX_CHK, "1234567890128", "" },
        /* 13*/ { BARCODE_EAN13, "1234567890128", "" },
        /* 14*/ { BARCODE_GS1_128, "[01]12345678901231", "" },
        /* 15*/ { BARCODE_CODABAR, "A00000000B", "" },
        /* 16*/ { BARCODE_CODE128, "1234567890", "" },
        /* 17*/ { BARCODE_DPLEIT, "1234567890123", "" },
        /* 18*/ { BARCODE_DPIDENT, "12345678901", "" },
        /* 19*/ { BARCODE_CODE16K, "1234567890", "" },
        /* 20*/ { BARCODE_CODE49, "1234567890", "" },
        /* 21*/ { BARCODE_CODE93, "1234567890", "" },
        /* 22*/ { BARCODE_FLAT, "1234567890", "" },
        /* 23*/ { BARCODE_DBAR_OMN, "1234567890123", "" },
        /* 24*/ { BARCODE_DBAR_LTD, "1234567890123", "" },
        /* 25*/ { BARCODE_DBAR_EXP, "[01]12345678901231", "" },
        /* 26*/ { BARCODE_TELEPEN, "1234567890", "" },
        /* 27*/ { BARCODE_UPCA, "12345678901", "" },
        /* 28*/ { BARCODE_UPCA_CHK, "123456789012", "" },
        /* 29*/ { BARCODE_UPCE, "1234567", "" },
        /* 30*/ { BARCODE_UPCE_CHK, "12345670", "" },
        /* 31*/ { BARCODE_POSTNET, "12345678901", "" },
        /* 32*/ { BARCODE_MSI_PLESSEY, "1234567890", "" },
        /* 33*/ { BARCODE_FIM, "A", "" },
        /* 34*/ { BARCODE_LOGMARS, "1234567890", "" },
        /* 35*/ { BARCODE_PHARMA, "123456", "" },
        /* 36*/ { BARCODE_PZN, "123456", "" },
        /* 37*/ { BARCODE_PHARMA_TWO, "12345678", "" },
        /* 38*/ { BARCODE_CEPNET, "12345678", "" },
        /* 39*/ { BARCODE_PDF417, "1234567890", "" },
        /* 40*/ { BARCODE_PDF417COMP, "1234567890", "" },
        /* 41*/ { BARCODE_MAXICODE, "1234567890", "" },
        /* 42*/ { BARCODE_QRCODE, "1234567890AB", "" },
        /* 43*/ { BARCODE_CODE128AB, "1234567890", "" },
        /* 44*/ { BARCODE_AUSPOST, "12345678901234567890123", "" },
        /* 45*/ { BARCODE_AUSREPLY, "12345678", "" },
        /* 46*/ { BARCODE_AUSROUTE, "12345678", "" },
        /* 47*/ { BARCODE_AUSREDIRECT, "12345678", "" },
        /* 48*/ { BARCODE_ISBNX, "123456789", "" },
        /* 49*/ { BARCODE_RM4SCC, "1234567890", "" },
        /* 50*/ { BARCODE_DATAMATRIX, "ABC", "" },
        /* 51*/ { BARCODE_EAN14, "1234567890123", "" },
        /* 52*/ { BARCODE_VIN, "12345678701234567", "" },
        /* 53*/ { BARCODE_CODABLOCKF, "1234567890", "" },
        /* 54*/ { BARCODE_NVE18, "12345678901234567", "" },
        /* 55*/ { BARCODE_JAPANPOST, "1234567890", "" },
        /* 56*/ { BARCODE_KOREAPOST, "123456", "" },
        /* 57*/ { BARCODE_DBAR_STK, "1234567890123", "" },
        /* 58*/ { BARCODE_DBAR_OMNSTK, "1234567890123", "" },
        /* 59*/ { BARCODE_DBAR_EXPSTK, "[01]12345678901231", "" },
        /* 60*/ { BARCODE_PLANET, "12345678901", "" },
        /* 61*/ { BARCODE_MICROPDF417, "1234567890", "" },
        /* 62*/ { BARCODE_USPS_IMAIL, "12345678901234567890", "" },
        /* 63*/ { BARCODE_PLESSEY, "1234567890", "" },
        /* 64*/ { BARCODE_TELEPEN_NUM, "1234567890", "" },
        /* 65*/ { BARCODE_ITF14, "1234567890", "" },
        /* 66*/ { BARCODE_KIX, "123456ABCDE", "" },
        /* 67*/ { BARCODE_AZTEC, "1234567890AB", "" },
        /* 68*/ { BARCODE_DAFT, "DAFTDAFTDAFTDAFT", "" },
        /* 69*/ { BARCODE_DPD, "0123456789012345678901234567", "" },
        /* 70*/ { BARCODE_MICROQR, "12345", "" },
        /* 71*/ { BARCODE_HIBC_128, "1234567890", "" },
        /* 72*/ { BARCODE_HIBC_39, "1234567890", "" },
        /* 73*/ { BARCODE_HIBC_DM, "ABC", "" },
        /* 74*/ { BARCODE_HIBC_QR, "1234567890AB", "" },
        /* 75*/ { BARCODE_HIBC_PDF, "1234567890", "" },
        /* 76*/ { BARCODE_HIBC_MICPDF, "1234567890", "" },
        /* 77*/ { BARCODE_HIBC_BLOCKF, "1234567890", "" },
        /* 78*/ { BARCODE_HIBC_AZTEC, "1234567890AB", "" },
        /* 79*/ { BARCODE_DOTCODE, "ABC", "" },
        /* 80*/ { BARCODE_HANXIN, "1234567890AB", "" },
        /* 81*/ { BARCODE_MAILMARK_2D, "012100123412345678AB19XY1A 0", "" },
        /* 82*/ { BARCODE_UPU_S10, "EE876543216CA", "" },
        /* 83*/ { BARCODE_MAILMARK_4S, "01000000000000000AA00AA0A", "" },
        /* 84*/ { BARCODE_AZRUNE, "255", "" },
        /* 85*/ { BARCODE_CODE32, "12345678", "" },
        /* 86*/ { BARCODE_EANX_CC, "123456789012", "[20]01" },
        /* 87*/ { BARCODE_GS1_128_CC, "[01]12345678901231", "[20]01" },
        /* 88*/ { BARCODE_DBAR_OMN_CC, "1234567890123", "[20]01" },
        /* 89*/ { BARCODE_DBAR_LTD_CC, "1234567890123", "[20]01" },
        /* 90*/ { BARCODE_DBAR_EXP_CC, "[01]12345678901231", "[20]01" },
        /* 91*/ { BARCODE_UPCA_CC, "12345678901", "[20]01" },
        /* 92*/ { BARCODE_DBAR_STK_CC, "1234567890123", "[20]01" },
        /* 93*/ { BARCODE_DBAR_OMNSTK_CC, "1234567890123", "[20]01" },
        /* 94*/ { BARCODE_DBAR_EXPSTK_CC, "[01]12345678901231", "[20]01" },
        /* 95*/ { BARCODE_CHANNEL, "01", "" },
        /* 96*/ { BARCODE_CODEONE, "12345678901234567890", "" },
        /* 97*/ { BARCODE_GRIDMATRIX, "ABC", "" },
        /* 98*/ { BARCODE_UPNQR, "1234567890AB", "" },
        /* 99*/ { BARCODE_ULTRA, "1234567890", "" },
        /*100*/ { BARCODE_RMQR, "12345", "" },
        /*101*/ { BARCODE_BC412, "1234567", "" },
        /*102*/ { BARCODE_DXFILMEDGE, "16", "" },
        /*103*/ { BARCODE_EAN8_CC, "12345670", "[20]01" },
        /*104*/ { BARCODE_EAN13_CC, "123456789012", "[20]01" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;
    char errmsg[128] = {0}; /* Suppress clang -fsanitize=memory false positive */

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;
        symbol->debug |= debug;

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = (int) strlen(text);

        ret = ZBarcode_Encode_and_Buffer_Vector(symbol, (const unsigned char *) text, length, 0);
        assert_zero(ret, "i:%d ZBarcode_Encode_and_Buffer_Vector(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Encode_and_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        assert_nonzero(check_vectors(symbol, errmsg), "i:%d ZBarcode_Encode_and_Buffer_Vector(%d) %s\n",
            i, data[i].symbology, errmsg);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_upcean_hrt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int show_hrt;
        int output_options;
        const char *data;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
        float expected_string_x;
        float expected_string_y;
        float expected_string2_x;
        float expected_string2_y;
        int expected_string_cnt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_EAN13, -1, -1, "123456789012", 0, 50, 1, 95, 226, 118, 12.2, 117.2, -1, -1, 3 }, /* EAN-13 */
        /*  1*/ { BARCODE_EANX, -1, -1, "123456789012", 0, 50, 1, 95, 226, 118, 12.2, 117.2, -1, -1, 3 }, /* EAN-13 */
        /*  2*/ { BARCODE_EAN13, -1, BARCODE_CONTENT_SEGS, "123456789012", 0, 50, 1, 95, 226, 118, 12.2, 117.2, -1, -1, 3 }, /* EAN-13 */
        /*  3*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "123456789012", 0, 50, 1, 95, 226, 118, 12.2, 117.2, -1, -1, 3 }, /* EAN-13 */
        /*  4*/ { BARCODE_EAN13, 0, -1, "123456789012", 0, 50, 1, 95, 226, 110, -1, -1, -1, -1, 0 }, /* EAN-13 */
        /*  5*/ { BARCODE_EANX, 0, -1, "123456789012", 0, 50, 1, 95, 226, 110, -1, -1, -1, -1, 0 }, /* EAN-13 */
        /*  6*/ { BARCODE_EAN13, -1, EANUPC_GUARD_WHITESPACE, "123456789012", 0, 50, 1, 95, 226, 118, 12.2, 117.2, 227, 117.2, 4 }, /* EAN-13 */
        /*  7*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "123456789012", 0, 50, 1, 95, 226, 118, 12.2, 117.2, 227, 117.2, 4 }, /* EAN-13 */
        /*  8*/ { BARCODE_EANX_CHK, -1, -1, "1234567890128", 0, 50, 1, 95, 226, 118, 12.2, 117.2, -1, -1, 3 }, /* EAN-13 */
        /*  9*/ { BARCODE_EANX_CHK, 0, -1, "1234567890128", 0, 50, 1, 95, 226, 110, -1, -1, -1, -1, 0 }, /* EAN-13 */
        /* 10*/ { BARCODE_EANX_CHK, -1, EANUPC_GUARD_WHITESPACE, "1234567890128", 0, 50, 1, 95, 226, 118, 12.2, 117.2, 227, 117.2, 4 }, /* EAN-13 */
        /* 11*/ { BARCODE_ISBNX, -1, -1, "9784567890120", 0, 50, 1, 95, 226, 118, 12.2, 117.2, -1, -1, 3 },
        /* 12*/ { BARCODE_ISBNX, -1, BARCODE_CONTENT_SEGS, "9784567890120", 0, 50, 1, 95, 226, 118, 12.2, 117.2, -1, -1, 3 },
        /* 13*/ { BARCODE_ISBNX, 0, -1, "9784567890120", 0, 50, 1, 95, 226, 110, -1, -1, -1, -1, 0 },
        /* 14*/ { BARCODE_ISBNX, -1, EANUPC_GUARD_WHITESPACE, "9784567890120", 0, 50, 1, 95, 226, 118, 12.2, 117.2, 227, 117.2, 4 },
        /* 15*/ { BARCODE_EAN8, -1, -1, "1234567", 0, 50, 1, 67, 162, 118, 49, 117.2, -1, -1, 2 }, /* EAN-8 */
        /* 16*/ { BARCODE_EANX, -1, -1, "1234567", 0, 50, 1, 67, 162, 118, 49, 117.2, -1, -1, 2 }, /* EAN-8 */
        /* 17*/ { BARCODE_EAN8, -1, BARCODE_CONTENT_SEGS, "1234567", 0, 50, 1, 67, 162, 118, 49, 117.2, -1, -1, 2 }, /* EAN-8 */
        /* 18*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "1234567", 0, 50, 1, 67, 162, 118, 49, 117.2, -1, -1, 2 }, /* EAN-8 */
        /* 19*/ { BARCODE_EAN8, 0, -1, "1234567", 0, 50, 1, 67, 162, 110, -1, -1, -1, -1, 0 }, /* EAN-8 */
        /* 20*/ { BARCODE_EANX, 0, -1, "1234567", 0, 50, 1, 67, 162, 110, -1, -1, -1, -1, 0 }, /* EAN-8 */
        /* 21*/ { BARCODE_EAN8, -1, EANUPC_GUARD_WHITESPACE, "1234567", 0, 50, 1, 67, 162, 118, 49, 117.2, -1, 117.2, 4 }, /* EAN-8 */
        /* 22*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "1234567", 0, 50, 1, 67, 162, 118, 49, 117.2, -1, 117.2, 4 }, /* EAN-8 */
        /* 23*/ { BARCODE_EAN_5ADDON, -1, -1, "1234", 0, 50, 1, 47, 104, 118, 47, 15.6, -1, -1, 1 }, /* EAN-5 */
        /* 24*/ { BARCODE_EANX, -1, -1, "1234", 0, 50, 1, 47, 104, 118, 47, 15.6, -1, -1, 1 }, /* EAN-5 */
        /* 25*/ { BARCODE_EAN_5ADDON, -1, BARCODE_CONTENT_SEGS, "1234", 0, 50, 1, 47, 104, 118, 47, 15.6, -1, -1, 1 }, /* EAN-5 */
        /* 26*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "1234", 0, 50, 1, 47, 104, 118, 47, 15.6, -1, -1, 1 }, /* EAN-5 */
        /* 27*/ { BARCODE_EAN_5ADDON, 0, -1, "1234", 0, 50, 1, 47, 104, 100, -1, -1, -1, -1, 0 }, /* EAN-5 */
        /* 28*/ { BARCODE_EANX, 0, -1, "1234", 0, 50, 1, 47, 104, 100, -1, -1, -1, -1, 0 }, /* EAN-5 */
        /* 29*/ { BARCODE_EAN_5ADDON, -1, EANUPC_GUARD_WHITESPACE, "1234", 0, 50, 1, 47, 104, 118, 47, 15.6, 105, 15.6, 2 }, /* EAN-5 */
        /* 30*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "1234", 0, 50, 1, 47, 104, 118, 47, 15.6, 105, 15.6, 2 }, /* EAN-5 */
        /* 31*/ { BARCODE_EAN_2ADDON, -1, -1, "12", 0, 50, 1, 20, 50, 118, 20, 15.6, -1, -1, 1 }, /* EAN-2 */
        /* 32*/ { BARCODE_EANX, -1, -1, "12", 0, 50, 1, 20, 50, 118, 20, 15.6, -1, -1, 1 }, /* EAN-2 */
        /* 33*/ { BARCODE_EAN_2ADDON, -1, BARCODE_CONTENT_SEGS, "12", 0, 50, 1, 20, 50, 118, 20, 15.6, -1, -1, 1 }, /* EAN-2 */
        /* 34*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "12", 0, 50, 1, 20, 50, 118, 20, 15.6, -1, -1, 1 }, /* EAN-2 */
        /* 35*/ { BARCODE_EAN_2ADDON, 0, -1, "12", 0, 50, 1, 20, 50, 100, -1, -1, -1, -1, 0 }, /* EAN-2 */
        /* 36*/ { BARCODE_EANX, 0, -1, "12", 0, 50, 1, 20, 50, 100, -1, -1, -1, -1, 0 }, /* EAN-2 */
        /* 37*/ { BARCODE_EAN_2ADDON, -1, EANUPC_GUARD_WHITESPACE, "12", 0, 50, 1, 20, 50, 118, 20, 15.6, 51, 15.6, 2 }, /* EAN-2 */
        /* 38*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "12", 0, 50, 1, 20, 50, 118, 20, 15.6, 51, 15.6, 2 }, /* EAN-2 */
        /* 39*/ { BARCODE_UPCA, -1, -1, "12345678901", 0, 50, 1, 95, 226, 118, 8.7, 117.2, -1, -1, 4 },
        /* 40*/ { BARCODE_UPCA, -1, BARCODE_CONTENT_SEGS, "12345678901", 0, 50, 1, 95, 226, 118, 8.7, 117.2, -1, -1, 4 },
        /* 41*/ { BARCODE_UPCA, 0, -1, "12345678901", 0, 50, 1, 95, 226, 110, -1, -1, -1, -1, 0 },
        /* 42*/ { BARCODE_UPCA, -1, EANUPC_GUARD_WHITESPACE, "12345678901", 0, 50, 1, 95, 226, 118, 8.7, 117.2, -1, -1, 4 },
        /* 43*/ { BARCODE_UPCA_CHK, -1, -1, "123456789012", 0, 50, 1, 95, 226, 118, 8.7, 117.2, -1, -1, 4 },
        /* 44*/ { BARCODE_UPCA_CHK, 0, -1, "123456789012", 0, 50, 1, 95, 226, 110, -1, -1, -1, -1, 0 },
        /* 45*/ { BARCODE_UPCA_CHK, -1, EANUPC_GUARD_WHITESPACE, "123456789012", 0, 50, 1, 95, 226, 118, 8.7, 117.2, -1, -1, 4 },
        /* 46*/ { BARCODE_UPCE, -1, -1, "1234567", 0, 50, 1, 51, 134, 118, 8.7, 117.2, -1, -1, 3 },
        /* 47*/ { BARCODE_UPCE, -1, BARCODE_CONTENT_SEGS, "1234567", 0, 50, 1, 51, 134, 118, 8.7, 117.2, -1, -1, 3 },
        /* 48*/ { BARCODE_UPCE, 0, -1, "1234567", 0, 50, 1, 51, 134, 110, -1, -1, -1, -1, 0 },
        /* 49*/ { BARCODE_UPCE, -1, EANUPC_GUARD_WHITESPACE, "1234567", 0, 50, 1, 51, 134, 118, 8.7, 117.2, -1, -1, 3 },
        /* 50*/ { BARCODE_UPCE_CHK, -1, -1, "12345670", 0, 50, 1, 51, 134, 118, 8.7, 117.2, -1, -1, 3 },
        /* 51*/ { BARCODE_UPCE_CHK, 0, -1, "12345670", 0, 50, 1, 51, 134, 110, -1, -1, -1, -1, 0 },
        /* 52*/ { BARCODE_UPCE_CHK, -1, EANUPC_GUARD_WHITESPACE, "12345670", 0, 50, 1, 51, 134, 118, 8.7, 117.2, -1, -1, 3 },
        /* 53*/ { BARCODE_EAN13, -1, -1, "123456789012+12", 0, 50, 1, 122, 276.0, 118, 12.2, 117.2, 246, 15.6, 4 }, /* EAN-13 + EAN-2 */
        /* 54*/ { BARCODE_EANX, -1, -1, "123456789012+12", 0, 50, 1, 122, 276.0, 118, 12.2, 117.2, 246, 15.6, 4 }, /* EAN-13 + EAN-2 */
        /* 55*/ { BARCODE_EAN13, -1, BARCODE_CONTENT_SEGS, "123456789012+12", 0, 50, 1, 122, 276.0, 118, 12.2, 117.2, 246, 15.6, 4 }, /* EAN-13 + EAN-2 */
        /* 56*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "123456789012+12", 0, 50, 1, 122, 276.0, 118, 12.2, 117.2, 246, 15.6, 4 }, /* EAN-13 + EAN-2 */
        /* 57*/ { BARCODE_EAN13, 0, -1, "123456789012+12", 0, 50, 1, 122, 276.0, 110, -1, -1, -1, -1, 0 }, /* EAN-13 + EAN-2 */
        /* 58*/ { BARCODE_EANX, 0, -1, "123456789012+12", 0, 50, 1, 122, 276.0, 110, -1, -1, -1, -1, 0 }, /* EAN-13 + EAN-2 */
        /* 59*/ { BARCODE_EAN13, -1, EANUPC_GUARD_WHITESPACE, "123456789012+12", 0, 50, 1, 122, 276.0, 118, 12.2, 117.2, 246, 15.6, 5 }, /* EAN-13 + EAN-2 */
        /* 60*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "123456789012+12", 0, 50, 1, 122, 276.0, 118, 12.2, 117.2, 246, 15.6, 5 }, /* EAN-13 + EAN-2 */
        /* 61*/ { BARCODE_ISBNX, -1, -1, "9784567890120+12", 0, 50, 1, 122, 276.0, 118, 12.2, 117.2, 246, 15.6, 4 }, /* ISBN + EAN-2 */
        /* 62*/ { BARCODE_ISBNX, 0, -1, "9784567890120+12", 0, 50, 1, 122, 276.0, 110, -1, -1, -1, -1, 0 }, /* ISBN + EAN-2 */
        /* 63*/ { BARCODE_ISBNX, -1, EANUPC_GUARD_WHITESPACE, "9784567890120+12", 0, 50, 1, 122, 276.0, 118, 246, 15.6, 277, 15.6, 5 }, /* ISBN + EAN-2 */
        /* 64*/ { BARCODE_EAN13, -1, -1, "123456789012+12345", 0, 50, 1, 149, 330.0, 118, 12.2, 117.2, 274, 15.6, 4 }, /* EAN-13 + EAN-5 */
        /* 65*/ { BARCODE_EANX, -1, -1, "123456789012+12345", 0, 50, 1, 149, 330.0, 118, 12.2, 117.2, 274, 15.6, 4 }, /* EAN-13 + EAN-5 */
        /* 66*/ { BARCODE_EAN13, -1, BARCODE_CONTENT_SEGS, "123456789012+12345", 0, 50, 1, 149, 330.0, 118, 12.2, 117.2, 274, 15.6, 4 }, /* EAN-13 + EAN-5 */
        /* 67*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "123456789012+12345", 0, 50, 1, 149, 330.0, 118, 12.2, 117.2, 274, 15.6, 4 }, /* EAN-13 + EAN-5 */
        /* 68*/ { BARCODE_EAN13, 0, -1, "123456789012+12345", 0, 50, 1, 149, 330.0, 110, -1, -1, -1, -1, 0 }, /* EAN-13 + EAN-5 */
        /* 69*/ { BARCODE_EANX, 0, -1, "123456789012+12345", 0, 50, 1, 149, 330.0, 110, -1, -1, -1, -1, 0 }, /* EAN-13 + EAN-5 */
        /* 70*/ { BARCODE_EAN13, -1, EANUPC_GUARD_WHITESPACE, "123456789012+12345", 0, 50, 1, 149, 330.0, 118, 12.2, 117.2, 274, 15.6, 5 }, /* EAN-13 + EAN-5 */
        /* 71*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "123456789012+12345", 0, 50, 1, 149, 330.0, 118, 12.2, 117.2, 274, 15.6, 5 }, /* EAN-13 + EAN-5 */
        /* 72*/ { BARCODE_ISBNX, -1, -1, "9784567890120+12345", 0, 50, 1, 149, 330.0, 118, 12.2, 117.2, 274, 15.6, 4 }, /* ISBN + EAN-5 */
        /* 73*/ { BARCODE_ISBNX, 0, -1, "9784567890120+12345", 0, 50, 1, 149, 330.0, 110, -1, -1, -1, -1, 0 }, /* ISBN + EAN-5 */
        /* 74*/ { BARCODE_ISBNX, -1, EANUPC_GUARD_WHITESPACE, "9784567890120+12345", 0, 50, 1, 149, 330.0, 118, 274, 15.6, 331, 15.6, 5 }, /* ISBN + EAN-5 */
        /* 75*/ { BARCODE_EAN8, -1, -1, "1234567+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 118, 49, 117.2, 182, 15.6, 3 }, /* EAN-8 + EAN-2 */
        /* 76*/ { BARCODE_EANX, -1, -1, "1234567+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 118, 49, 117.2, 182, 15.6, 3 }, /* EAN-8 + EAN-2 */
        /* 77*/ { BARCODE_EAN8, -1, BARCODE_CONTENT_SEGS, "1234567+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 118, 49, 117.2, 182, 15.6, 3 }, /* EAN-8 + EAN-2 */
        /* 78*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "1234567+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 118, 49, 117.2, 182, 15.6, 3 }, /* EAN-8 + EAN-2 */
        /* 79*/ { BARCODE_EAN8, 0, -1, "1234567+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 110, -1, -1, -1, -1, 0 }, /* EAN-8 + EAN-2 */
        /* 80*/ { BARCODE_EANX, 0, -1, "1234567+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 110, -1, -1, -1, -1, 0 }, /* EAN-8 + EAN-2 */
        /* 81*/ { BARCODE_EAN8, -1, EANUPC_GUARD_WHITESPACE, "1234567+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 118, 182, 15.6, 213, 15.6, 5 }, /* EAN-8 + EAN-2 */
        /* 82*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "1234567+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 118, 182, 15.6, 213, 15.6, 5 }, /* EAN-8 + EAN-2 */
        /* 83*/ { BARCODE_EAN8, -1, -1, "1234567+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 118, 49, 117.2, 210, 15.6, 3 }, /* EAN-8 + EAN-5 */
        /* 84*/ { BARCODE_EANX, -1, -1, "1234567+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 118, 49, 117.2, 210, 15.6, 3 }, /* EAN-8 + EAN-5 */
        /* 85*/ { BARCODE_EAN8, -1, BARCODE_CONTENT_SEGS, "1234567+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 118, 49, 117.2, 210, 15.6, 3 }, /* EAN-8 + EAN-5 */
        /* 86*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "1234567+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 118, 49, 117.2, 210, 15.6, 3 }, /* EAN-8 + EAN-5 */
        /* 87*/ { BARCODE_EAN8, 0, -1, "1234567+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 110, -1, -1, -1, -1, 0 }, /* EAN-8 + EAN-5 */
        /* 88*/ { BARCODE_EANX, 0, -1, "1234567+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 110, -1, -1, -1, -1, 0 }, /* EAN-8 + EAN-5 */
        /* 89*/ { BARCODE_EAN8, -1, EANUPC_GUARD_WHITESPACE, "1234567+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 118, 210, 15.6, 210, 15.6, 5 }, /* EAN-8 + EAN-5 */
        /* 90*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "1234567+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 118, 210, 15.6, 210, 15.6, 5 }, /* EAN-8 + EAN-5 */
        /* 91*/ { BARCODE_UPCA, -1, -1, "12345678901+12", 0, 50, 1, 124, 276, 118, 8.7, 117.2, 246, 15.6, 5 },
        /* 92*/ { BARCODE_UPCA, -1, BARCODE_CONTENT_SEGS, "12345678901+12", 0, 50, 1, 124, 276, 118, 8.7, 117.2, 246, 15.6, 5 },
        /* 93*/ { BARCODE_UPCA, 0, -1, "12345678901+12", 0, 50, 1, 124, 276, 110, -1, -1, -1, -1, 0 },
        /* 94*/ { BARCODE_UPCA, -1, EANUPC_GUARD_WHITESPACE, "12345678901+12", 0, 50, 1, 124, 276, 118, 8.7, 117.2, 277, 15.6, 6 },
        /* 95*/ { BARCODE_UPCA, -1, -1, "12345678901+12345", 0, 50, 1, 151, 330, 118, 8.7, 117.2, 274, 15.6, 5 },
        /* 96*/ { BARCODE_UPCA, -1, BARCODE_CONTENT_SEGS, "12345678901+12345", 0, 50, 1, 151, 330, 118, 8.7, 117.2, 274, 15.6, 5 },
        /* 97*/ { BARCODE_UPCA, 0, -1, "12345678901+12345", 0, 50, 1, 151, 330, 110, -1, -1, -1, -1, 0 },
        /* 98*/ { BARCODE_UPCA, -1, EANUPC_GUARD_WHITESPACE, "12345678901+12345", 0, 50, 1, 151, 330, 118, 274, 15.6, 331, 15.6, 6 },
        /* 99*/ { BARCODE_UPCE, -1, -1, "1234567+12", 0, 50, 1, 78, 184.0, 118, 8.7, 117.2, 154, 15.6, 4 },
        /*100*/ { BARCODE_UPCE, -1, BARCODE_CONTENT_SEGS, "1234567+12", 0, 50, 1, 78, 184.0, 118, 8.7, 117.2, 154, 15.6, 4 },
        /*101*/ { BARCODE_UPCE, 0, -1, "1234567+12", 0, 50, 1, 78, 184.0, 110, -1, -1, -1, -1, 0 },
        /*102*/ { BARCODE_UPCE, -1, EANUPC_GUARD_WHITESPACE, "1234567+12", 0, 50, 1, 78, 184.0, 118, 8.7, 117.2, 185, 15.6, 5 },
        /*103*/ { BARCODE_UPCE, -1, -1, "1234567+12345", 0, 50, 1, 105, 238.0, 118, 8.7, 117.2, 182, 15.6, 4 },
        /*104*/ { BARCODE_UPCE, -1, BARCODE_CONTENT_SEGS, "1234567+12345", 0, 50, 1, 105, 238.0, 118, 8.7, 117.2, 182, 15.6, 4 },
        /*105*/ { BARCODE_UPCE, 0, -1, "1234567+12345", 0, 50, 1, 105, 238.0, 110, -1, -1, -1, -1, 0 },
        /*106*/ { BARCODE_UPCE, -1, EANUPC_GUARD_WHITESPACE, "1234567+12345", 0, 50, 1, 105, 238.0, 118, 182, 15.6, 239, 15.6, 5 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    struct zint_vector_string *string;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        int string_cnt;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }
        if (data[i].output_options != -1) {
            symbol->output_options = data[i].output_options;
        }
        symbol->debug |= debug;

        length = (int) strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%d) ret %d != %d (%s)\n",
                    i, data[i].symbology, ret, data[i].ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0 (%s)\n",
                    i, data[i].symbology, ret, symbol->errtxt);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.9g != %.9g\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);

        assert_equal(symbol->vector->width, data[i].expected_vector_width,
                    "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
        assert_equal(symbol->vector->height, data[i].expected_vector_height,
                    "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->vector->height,
                    data[i].expected_vector_height);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            testUtilVectorPrint(symbol);
        }

        if (data[i].show_hrt) {
            assert_nonnull(symbol->vector->strings, "i:%d (%s) vector->strings NULL\n",
                        i, testUtilBarcodeName(data[i].symbology));
            string = find_string(symbol, data[i].expected_string_x, data[i].expected_string_y);
            assert_nonnull(string, "i:%d (%s) find_string(%.9g, %.9g) NULL\n",
                        i, testUtilBarcodeName(data[i].symbology), data[i].expected_string_x,
                        data[i].expected_string_y);

            if (data[i].expected_string2_x != -1) {
                assert_nonnull(symbol->vector->strings->next, "i:%d (%s) vector->strings->next NULL\n",
                            i, testUtilBarcodeName(data[i].symbology));
                string = find_string(symbol, data[i].expected_string2_x, data[i].expected_string2_y);
                assert_nonnull(string, "i:%d (%s) find_string(%.9g, %.9g) NULL\n",
                            i, testUtilBarcodeName(data[i].symbology), data[i].expected_string2_x,
                            data[i].expected_string2_y);
            }
        } else {
            assert_null(symbol->vector->strings, "i:%d (%s) vector->strings NULL\n",
                        i, testUtilBarcodeName(data[i].symbology));
        }
        string_cnt = cnt_strings(symbol);
        assert_equal(string_cnt, data[i].expected_string_cnt, "i:%d (%s) cnt_strings(symbol) %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), string_cnt, data[i].expected_string_cnt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_row_separator(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int border_width;
        int option_1;
        int option_3;
        const char *data;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_separator_x;
        int expected_separator_y;
        int expected_separator_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, -1, -1, -1, "A", 0, 20, 2, 101, 42, 21, 2 },
        /*  1*/ { BARCODE_CODABLOCKF, -1, -1, 0, "A", 0, 20, 2, 101, 42, 21, 2 }, /* Same as default */
        /*  2*/ { BARCODE_CODABLOCKF, -1, -1, 1, "A", 0, 20, 2, 101, 42, 21, 2 }, /* Same as default */
        /*  3*/ { BARCODE_CODABLOCKF, -1, -1, 2, "A", 0, 20, 2, 101, 42, 20, 4 },
        /*  4*/ { BARCODE_CODABLOCKF, -1, -1, 3, "A", 0, 20, 2, 101, 42, 19, 6 },
        /*  5*/ { BARCODE_CODABLOCKF, -1, -1, 4, "A", 0, 20, 2, 101, 42, 18, 8 },
        /*  6*/ { BARCODE_CODABLOCKF, -1, -1, 5, "A", 0, 20, 2, 101, 42, 21, 2 }, /* > 4 ignored, same as default */
        /*  7*/ { BARCODE_CODABLOCKF, -1, 1, -1, "A", 0, 5, 1, 46, 20, 0, 2 }, /* CODE128 top separator */
        /*  8*/ { BARCODE_CODABLOCKF, 0, -1, -1, "A", 0, 20, 2, 101, 42, 21, 2 }, /* Border width zero, same as default */
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    struct zint_vector_rect *rect;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.9g != %.9g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

        rect = find_rect(symbol, data[i].expected_separator_x, data[i].expected_separator_y, 0, data[i].expected_separator_height);
        assert_nonnull(rect, "i:%d (%d) find_rect(%d, %d, 0, %d) NULL\n", i, data[i].symbology, data[i].expected_separator_x, data[i].expected_separator_y, data[i].expected_separator_height);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_stacking(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        int option_1;
        int option_3;
        const char *data;
        const char *data2;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_separator_x;
        int expected_separator_y;
        int expected_separator_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, "A", "B", 50, 2, 46, 92, 116, -1, -1, -1 },
        /*  1*/ { BARCODE_CODE128, BARCODE_BIND, -1, -1, "A", "B", 50, 2, 46, 92, 116, 0, 49, 2 },
        /*  2*/ { BARCODE_CODE128, BARCODE_BIND, -1, 2, "A", "B", 50, 2, 46, 92, 116, 0, 48, 4 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    struct zint_vector_rect *rect;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        int length2;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, data[i].output_options, data[i].data, -1, debug);
        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);

        length2 = (int) strlen(data[i].data2);
        ret = ZBarcode_Encode(symbol, TCU(data[i].data2), length2);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.9g != %.9g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

        if (data[i].expected_separator_y != -1) {
            if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
                testUtilVectorPrint(symbol);
            }

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
            assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.9g != %.9g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

            rect = find_rect(symbol, data[i].expected_separator_x, data[i].expected_separator_y, 0, data[i].expected_separator_height);
            assert_nonnull(rect, "i:%d (%d) find_rect(%d, %d, 0, %d) NULL\n", i, data[i].symbology, data[i].expected_separator_x, data[i].expected_separator_y, data[i].expected_separator_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_output_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int whitespace_width;
        int whitespace_height;
        int border_width;
        int output_options;
        const char *data;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
        int expected_set;
        float expected_set_x;
        float expected_set_y;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, -1, "A123", 0, 50, 1, 79, 158, 116.28, 0, 0, 4 },
        /*  1*/ { BARCODE_CODE128, -1, -1, 2, -1, "A123", 0, 50, 1, 79, 158, 116.28, 0, 0, 4 },
        /*  2*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BIND, "A123", 0, 50, 1, 79, 158, 124.28, 1, 0, 4 },
        /*  3*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BIND, "A123", 0, 50, 1, 79, 158, 124.28, 0, 4, 4 },
        /*  4*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BOX, "A123", 0, 50, 1, 79, 166, 124.28, 1, 4, 4 },
        /*  5*/ { BARCODE_CODE128, -1, -1, 0, BARCODE_BIND, "A123", 0, 50, 1, 79, 158, 116.28, 0, 0, 4 },
        /*  6*/ { BARCODE_CODE128, -1, -1, 0, BARCODE_BOX, "A123", 0, 50, 1, 79, 158, 116.28, 0, 4, 4 },
        /*  7*/ { BARCODE_CODE128, -1, -1, -1, -1, "A123", 0, 50, 1, 79, 158, 116.28, 0, 2, 0 },
        /*  8*/ { BARCODE_CODE128, 1, -1, -1, -1, "A123", 0, 50, 1, 79, 162, 116.28, 1, 2, 0 },
        /*  9*/ { BARCODE_CODE128, 1, 2, -1, -1, "A123", 0, 50, 1, 79, 162, 124.28, 0, 2, 0 },
        /* 10*/ { BARCODE_CODE128, 1, 2, -1, -1, "A123", 0, 50, 1, 79, 162, 124.28, 1, 2, 4 },
        /* 11*/ { BARCODE_CODE128, -1, -1, -1, -1, "A123", 0, 50, 1, 79, 158, 116.28, 0, 6, 8 },
        /* 12*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BIND, "A123", 0, 50, 1, 79, 170, 132.28, 1, 6, 8 },
        /* 13*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BIND, "A123", 0, 50, 1, 79, 170, 132.28, 0, 14, 8 },
        /* 14*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BOX, "A123", 0, 50, 1, 79, 186, 132.28, 1, 14, 8 },
        /* 15*/ { BARCODE_CODE128, -1, -1, -1, BARCODE_DOTTY_MODE, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 16*/ { BARCODE_QRCODE, -1, -1, -1, -1, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 6 },
        /* 17*/ { BARCODE_QRCODE, -1, -1, 3, -1, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 6 },
        /* 18*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND, "A123", 0, 21, 21, 21, 42, 54, 1, 0, 6 },
        /* 19*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND, "A123", 0, 21, 21, 21, 42, 54, 0, 22, 8 },
        /* 20*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BOX, "A123", 0, 21, 21, 21, 54, 54, 1, 22, 8 },
        /* 21*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 58, 70, 0, 0, 6 },
        /* 22*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 58, 70, 1, 0, 0 },
        /* 23*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BOX | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 70, 70, 0, 22, 8 },
        /* 24*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BOX | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 70, 70, 1, 30, 16 },
        /* 25*/ { BARCODE_QRCODE, -1, -1, -1, -1, "A123", 0, 21, 21, 21, 42, 42, 0, 10, 12 },
        /* 26*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND, "A123", 0, 21, 21, 21, 62, 66, 1, 10, 12 },
        /* 27*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND, "A123", 0, 21, 21, 21, 62, 66, 0, 22, 12 },
        /* 28*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BOX, "A123", 0, 21, 21, 21, 86, 66, 1, 22, 12 },
        /* 29*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 78, 82, 0, 10, 12 },
        /* 30*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 78, 82, 1, 18, 20 },
        /* 31*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 50 },
        /* 32*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 50 },
        /* 33*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_BIND | BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 58, 1, 0, 50 },
        /* 34*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_BIND | BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 58, 0, 54, 8 },
        /* 35*/ { BARCODE_QRCODE, 1, -1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 62, 58, 1, 54, 8 },
        /* 36*/ { BARCODE_MAXICODE, -1, -1, -1, -1, "A123", 0, 165, 33, 30, 60, 57.733398, 0, 0, 67.7334 },
        /* 37*/ { BARCODE_MAXICODE, -1, -1, 5, -1, "A123", 0, 165, 33, 30, 60, 57.733398, 0, 0, 67.7334 },
        /* 38*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 60, 77.733398, 1, 0, 67.7334 },
        /* 39*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 60, 77.733398, 0, 70, 10 },
        /* 40*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BOX, "A123", 0, 165, 33, 30, 80, 77.733398, 1, 70, 10 },
        /* 41*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 165, 33, 30, 64, 81.733398, 0, 0, 67.7334 },
        /* 42*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 165, 33, 30, 64, 81.733398, 1, 0, 71.7334 },
        /* 43*/ { BARCODE_MAXICODE, -1, -1, -1, -1, "A123", 0, 165, 33, 30, 60, 57.733398, 0, 0, 67.7334 },
        /* 44*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 84, 77.733398, 1, 0, 67.7334 },
        /* 45*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 84, 77.733398, 0, 94, 10 },
        /* 46*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BOX, "A123", 0, 165, 33, 30, 104, 77.733398, 1, 94, 10 },
        /* 47*/ { BARCODE_MAXICODE, -1, -1, -1, BARCODE_DOTTY_MODE, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 48*/ { BARCODE_ITF14, -1, -1, -1, -1, "123", 0, 50, 1, 135, 330, 136.28, 1, 320, 10 },
        /* 49*/ { BARCODE_ITF14, -1, -1, 0, -1, "123", 0, 50, 1, 135, 330, 136.28, 1, 320, 10 },
        /* 50*/ { BARCODE_ITF14, -1, -1, 0, BARCODE_BOX, "123", 0, 50, 1, 135, 310, 116.28, 0, 300, 0 }, /* No zero-width/height rectangles */
        /* 51*/ { BARCODE_CODABLOCKF, -1, -1, -1, -1, "A123", 0, 20, 2, 101, 242, 44, 1, 20, 42 },
        /* 52*/ { BARCODE_CODABLOCKF, -1, -1, -1, BARCODE_BIND_TOP, "A123", 0, 20, 2, 101, 242, 42, 0, 20, 42 },
        /* 53*/ { BARCODE_CODE16K, -1, -1, -1, -1, "A123", 0, 20, 2, 70, 162, 44, 1, 0, 42 },
        /* 54*/ { BARCODE_CODE16K, -1, -1, -1, BARCODE_BIND_TOP, "A123", 0, 20, 2, 70, 162, 42, 0, 0, 42 },
        /* 55*/ { BARCODE_CODE49, -1, -1, -1, -1, "A123", 0, 20, 2, 70, 162, 44, 1, 0, 42 },
        /* 56*/ { BARCODE_CODE49, -1, -1, -1, BARCODE_BIND_TOP, "A123", 0, 20, 2, 70, 162, 42, 0, 0, 42 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    struct zint_vector_rect *rect;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].whitespace_width != -1) {
            symbol->whitespace_width = data[i].whitespace_width;
        }
        if (data[i].whitespace_height != -1) {
            symbol->whitespace_height = data[i].whitespace_height;
        }
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != %d\n", i, data[i].symbology, ret, data[i].ret);

        if (ret < ZINT_ERROR) {
            assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

            if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
                testUtilVectorPrint(symbol);
            }

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.9g != %.9g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

            if (data[i].expected_set != -1) {
                rect = find_rect(symbol, data[i].expected_set_x, data[i].expected_set_y, 0, 0);
                if (data[i].expected_set) {
                    assert_nonnull(rect, "i:%d (%d) find_rect(%g, %g, 0, 0) NULL\n", i, data[i].symbology, data[i].expected_set_x, data[i].expected_set_y);
                } else {
                    assert_null(rect, "i:%d (%d) find_rect(%g, %g, 0, 0) not NULL\n", i, data[i].symbology, data[i].expected_set_x, data[i].expected_set_y);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* Checks that symbol lead-in (composite offset) isn't used to calc string position for non-composite barcodes */
static void test_noncomposite_string_x(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *data;

        int expected_width;
        float expected_string_x;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /* 0*/ { BARCODE_DBAR_OMN, "1234567890123", 96, 96 },
        /* 1*/ { BARCODE_DBAR_LTD, "1234567890123", 79, 79 },
        /* 2*/ { BARCODE_DBAR_EXP, "[01]12345678901231", 134, 134 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;
        symbol->debug |= debug;

        length = (int) strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_nonnull(symbol->vector->strings, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings NULL\n", i, data[i].symbology);
        assert_equal(symbol->vector->strings->x, data[i].expected_string_x,
            "i:%d (%s) symbol->vector->strings->x %.9g != %.9g\n", i, testUtilBarcodeName(data[i].symbology), symbol->vector->strings->x, data[i].expected_string_x);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* Checks UPCA/UPCE main_symbol_width_x (used for addon formatting) set whether whitespace width set or not */
static void test_upcean_whitespace_width(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *data;
        int whitespace_width;

        int expected_width;
        float expected_vector_width;
        int expected_string_cnt;
        float expected_string_y;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /* 0*/ { BARCODE_UPCA, "12345678904+12345", 0, 151, 330.0, 5, 15.6 },
        /* 1*/ { BARCODE_UPCA, "12345678904+12345", 11, 151, 330.0 + 4 * 11, 5, 15.6 },
        /* 2*/ { BARCODE_UPCE, "1234567+12", 0, 78, 184.0, 4, 15.6 },
        /* 3*/ { BARCODE_UPCE, "1234567+12", 8, 78, 184.0 + 4 * 8, 4, 15.6 }, /* Note: change from previous behaviour where if whitespace < 10 then set to 10 */
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    struct zint_vector_string *string;
    int string_cnt;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;
        symbol->whitespace_width = data[i].whitespace_width;
        symbol->debug |= debug;

        length = (int) strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            testUtilVectorPrint(symbol);
        }

        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);

        assert_nonnull(symbol->vector->strings, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings NULL\n", i, data[i].symbology);
        /* Get add-on string (last) */
        for (string = symbol->vector->strings, string_cnt = 1; string->next; string_cnt++) {
            string = string->next;
        }
        assert_equal(string_cnt, data[i].expected_string_cnt, "i:%d (%s) string_cnt %d != %d\n", i, testUtilBarcodeName(data[i].symbology), string_cnt, data[i].expected_string_cnt);
        assert_equal(string->y, data[i].expected_string_y, "i:%d (%s) string->y %.9g != %.9g\n", i, testUtilBarcodeName(data[i].symbology), string->y, data[i].expected_string_y);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_scale(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_2;
        int border_width;
        int output_options;
        float height;
        float scale;
        const char *data;
        const char *composite;

        int ret_vector;
        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
        int expected_set;
        float expected_set_x;
        float expected_set_y;
        float expected_set_width;
        float expected_set_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, 0, 0, "1", "", 0, 15, 5, 103, 206, 30, 1, 52, 0, 8, 6 }, /* With no scaling */
        /*  1*/ { BARCODE_PDF417, -1, -1, -1, 0, 0.1, "1", "", 0, 15, 5, 103, 206 * 0.1, 3, 1, 5.2000003, 0, 8 * 0.1, 6 * 0.1 },
        /*  2*/ { BARCODE_PDF417, -1, -1, -1, 0, 0.3, "1", "", 0, 15, 5, 103, 61.8000031, 30 * 0.3, 1, 52 * 0.3, 0, 2.4000001, 1.8000001 },
        /*  3*/ { BARCODE_PDF417, -1, -1, -1, 0, 0.6, "1", "", 0, 15, 5, 103, 123.600006, 30 * 0.6, 1, 52 * 0.6, 0, 4.8000002, 3.6000001 },
        /*  4*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, 0, "1234567", "[17]010615[10]A123456\"", 0, 50, 10, 55, 134, 118, 1, 28, 36, 2, 64 }, /* With no scaling */
        /*  5*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, 0.1, "1234567", "[17]010615[10]A123456\"", 0, 50, 10, 55, 13.4000006, 118 * 0.1, 1, 28 * 0.1, 3.6000001, 2 * 0.1, 64 * 0.1 },
        /*  6*/ { BARCODE_UPCE_CC, -1, -1, -1, 0.1, 0.1, "1234567", "[17]010615[10]A123456\"", 0, 18.5, 10, 55, 13.4000006, 5.5, 1, 28 * 0.1, 3.6000001, 2 * 0.1, 0.1 }, /* Height specified */
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;
    struct zint_vector_rect *rect;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }
        if (data[i].height) {
            symbol->height = data[i].height;
        }
        if (data[i].scale) {
            symbol->scale = data[i].scale;
        }
        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = (int) strlen(text);

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_nonzero(ret < ZINT_ERROR, "i:%d ZBarcode_Encode(%d) ret %d >= ZINT_ERROR (%s)\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret_vector, symbol->errtxt);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            testUtilVectorPrint(symbol);
        }

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.9g != %.9g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

        assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
        assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

        if (data[i].expected_set != -1) {
            rect = find_rect(symbol, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
            if (data[i].expected_set) {
                assert_nonnull(rect, "i:%d (%d) find_rect(%g, %g, %g, %g) NULL\n", i, data[i].symbology,
                    data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
            } else {
                assert_null(rect, "i:%d (%d) find_rect(%g, %g, %g, %g) not NULL\n", i, data[i].symbology,
                    data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_guard_descent(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        float guard_descent;
        const char *data;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
        float expected_set_x;
        float expected_set_y;
        float expected_set_width;
        float expected_set_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_UPCE, -1, "1234567", 0, 50, 1, 51, 134, 118, 18, 0, 2, 110 },
        /*  1*/ { BARCODE_UPCE, 0, "1234567", 0, 50, 1, 51, 134, 118, 18, 0, 2, 100 },
        /*  2*/ { BARCODE_UPCE, 3, "1234567", 0, 50, 1, 51, 134, 118, 18, 0, 2, 106 },
        /*  3*/ { BARCODE_UPCE, 8, "1234567", 0, 50, 1, 51, 134, 118, 18, 0, 2, 116 },
        /*  4*/ { BARCODE_UPCE, 8.2, "1234567", 0, 50, 1, 51, 134, 118, 18, 0, 2, 116.4 },
        /*  5*/ { BARCODE_UPCE, 8.25, "1234567", 0, 50, 1, 51, 134, 118, 18, 0, 2, 116.5 },
        /*  6*/ { BARCODE_UPCE, 8.3, "1234567", 0, 50, 1, 51, 134, 118, 18, 0, 2, 116.6 },
        /*  7*/ { BARCODE_UPCE, 19.6, "1234567", 0, 50, 1, 51, 134, 139.2, 18, 0, 2, 139.2 },
        /*  8*/ { BARCODE_UPCE, -1, "1234567+12345", 0, 50, 1, 105, 238, 118, 118, 0, 2, 110 },
        /*  9*/ { BARCODE_UPCE, -1, "1234567+12345", 0, 50, 1, 105, 238, 118, 134, 18, 2, 92 },
        /* 10*/ { BARCODE_UPCE, 0, "1234567+12345", 0, 50, 1, 105, 238, 118, 118, 0, 2, 100 },
        /* 11*/ { BARCODE_UPCE, 0, "1234567+12345", 0, 50, 1, 105, 238, 118, 134, 18, 2, 82 },
        /* 12*/ { BARCODE_UPCE, 4, "1234567+12345", 0, 50, 1, 105, 238, 118, 118, 0, 2, 108 },
        /* 13*/ { BARCODE_UPCE, 4, "1234567+12345", 0, 50, 1, 105, 238, 118, 134, 18, 2, 90 },
        /* 14*/ { BARCODE_UPCA, -1, "12345678901", 0, 50, 1, 95, 226, 118, 188, 0, 4, 110 },
        /* 15*/ { BARCODE_UPCA, 0, "12345678901", 0, 50, 1, 95, 226, 118, 188, 0, 4, 100 },
        /* 16*/ { BARCODE_UPCA, 6, "12345678901", 0, 50, 1, 95, 226, 118, 188, 0, 4, 112 },
        /* 17*/ { BARCODE_UPCA, -1, "12345678901+12", 0, 50, 1, 124, 276, 118, 188, 0, 4, 110 },
        /* 18*/ { BARCODE_UPCA, -1, "12345678901+12", 0, 50, 1, 124, 276, 118, 262, 18, 4, 92 },
        /* 19*/ { BARCODE_UPCA, 0, "12345678901+12", 0, 50, 1, 124, 276, 118, 188, 0, 4, 100 },
        /* 20*/ { BARCODE_UPCA, 0, "12345678901+12", 0, 50, 1, 124, 276, 118, 262, 18, 4, 82 },
        /* 21*/ { BARCODE_UPCA, 9, "12345678901+12", 0, 50, 1, 124, 276, 118, 188, 0, 4, 118 },
        /* 22*/ { BARCODE_UPCA, 9, "12345678901+12", 0, 50, 1, 124, 276, 118, 262, 18, 4, 100 },
        /* 23*/ { BARCODE_EAN13, -1, "123456789012", 0, 50, 1, 95, 226, 118, 22, 0, 2, 110 },
        /* 24*/ { BARCODE_EANX, -1, "123456789012", 0, 50, 1, 95, 226, 118, 22, 0, 2, 110 },
        /* 25*/ { BARCODE_EAN13, 0, "123456789012", 0, 50, 1, 95, 226, 118, 22, 0, 2, 100 },
        /* 26*/ { BARCODE_EANX, 0, "123456789012", 0, 50, 1, 95, 226, 118, 22, 0, 2, 100 },
        /* 27*/ { BARCODE_EAN13, 7, "123456789012", 0, 50, 1, 95, 226, 118, 22, 0, 2, 114 },
        /* 28*/ { BARCODE_EANX, 7, "123456789012", 0, 50, 1, 95, 226, 118, 22, 0, 2, 114 },
        /* 29*/ { BARCODE_EAN13, -1, "123456789012+12", 0, 50, 1, 122, 276, 118, 22, 0, 2, 110 },
        /* 30*/ { BARCODE_EANX, -1, "123456789012+12", 0, 50, 1, 122, 276, 118, 22, 0, 2, 110 },
        /* 31*/ { BARCODE_EAN13, -1, "123456789012+12", 0, 50, 1, 122, 276, 118, 262, 18, 4, 92 },
        /* 32*/ { BARCODE_EANX, -1, "123456789012+12", 0, 50, 1, 122, 276, 118, 262, 18, 4, 92 },
        /* 33*/ { BARCODE_EAN13, 0, "123456789012+12", 0, 50, 1, 122, 276, 118, 22, 0, 2, 100 },
        /* 34*/ { BARCODE_EANX, 0, "123456789012+12", 0, 50, 1, 122, 276, 118, 22, 0, 2, 100 },
        /* 35*/ { BARCODE_EAN13, 0, "123456789012+12", 0, 50, 1, 122, 276, 118, 262, 18, 4, 82 },
        /* 36*/ { BARCODE_EANX, 0, "123456789012+12", 0, 50, 1, 122, 276, 118, 262, 18, 4, 82 },
        /* 37*/ { BARCODE_EAN13, 8.21, "123456789012+12", 0, 50, 1, 122, 276, 118, 22, 0, 2, 116.42 },
        /* 38*/ { BARCODE_EANX, 8.21, "123456789012+12", 0, 50, 1, 122, 276, 118, 22, 0, 2, 116.42 },
        /* 39*/ { BARCODE_EAN13, 8.21, "123456789012+12", 0, 50, 1, 122, 276, 118, 262, 18, 4, 98.42 },
        /* 40*/ { BARCODE_EANX, 8.21, "123456789012+12", 0, 50, 1, 122, 276, 118, 262, 18, 4, 98.42 },
        /* 41*/ { BARCODE_ISBNX, -1, "123456789", 0, 50, 1, 95, 226, 118, 22, 0, 2, 110 },
        /* 42*/ { BARCODE_ISBNX, 0, "123456789", 0, 50, 1, 95, 226, 118, 22, 0, 2, 100 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    struct zint_vector_rect *rect;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].guard_descent != -1.0f) {
            symbol->guard_descent = data[i].guard_descent;
        }

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != %d\n", i, data[i].symbology, ret, data[i].ret);

        if (ret < ZINT_ERROR) {
            assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

            if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) { /* ZINT_DEBUG_TEST_PRINT 16 */
                testUtilVectorPrint(symbol);
            }

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.9g != %.9g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

            rect = find_rect(symbol, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
            assert_nonnull(rect, "i:%d (%s) find_rect(%.9g, %.9g, %.9g, %.9g) NULL\n",
                                    i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_x,
                                    data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_quiet_zones(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        int option_1;
        int option_2;
        int show_hrt;
        const char *data;
        const char *composite;

        int ret;
        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
        float expected_set_x;
        float expected_set_y;
        float expected_set_width;
        float expected_set_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, -1, -1, -1, "1234", "", 0, 50, 1, 62, 124, 116.28, 0, 0, 2, 100 },
        /*  1*/ { BARCODE_CODE11, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 62, 164, 116.28, 20, 0, 2, 100 },
        /*  2*/ { BARCODE_CODE11, BARCODE_QUIET_ZONES | BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 62, 124, 116.28, 0, 0, 2, 100 }, /* BARCODE_NO_QUIET_ZONES trumps BARCODE_QUIET_ZONES */
        /*  3*/ { BARCODE_C25STANDARD, -1, -1, -1, -1, "1234", "", 0, 50, 1, 57, 114, 116.28, 0, 0, 8, 100 },
        /*  4*/ { BARCODE_C25STANDARD, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 57, 154, 116.28, 20, 0, 8, 100 },
        /*  5*/ { BARCODE_C25INTER, -1, -1, -1, -1, "1234", "", 0, 50, 1, 45, 90, 116.28, 0, 0, 2, 100 },
        /*  6*/ { BARCODE_C25INTER, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 45, 130, 116.28, 20, 0, 2, 100 },
        /*  7*/ { BARCODE_C25IATA, -1, -1, -1, -1, "1234", "", 0, 50, 1, 65, 130, 116.28, 0, 0, 2, 100 },
        /*  8*/ { BARCODE_C25IATA, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 65, 170, 116.28, 20, 0, 2, 100 },
        /*  9*/ { BARCODE_C25LOGIC, -1, -1, -1, -1, "1234", "", 0, 50, 1, 49, 98, 116.28, 0, 0, 2, 100 },
        /* 10*/ { BARCODE_C25LOGIC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 49, 138, 116.28, 20, 0, 2, 100 },
        /* 11*/ { BARCODE_C25IND, -1, -1, -1, -1, "1234", "", 0, 50, 1, 75, 150, 116.28, 0, 0, 6, 100 },
        /* 12*/ { BARCODE_C25IND, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 75, 190, 116.28, 20, 0, 6, 100 },
        /* 13*/ { BARCODE_CODE39, -1, -1, -1, -1, "1234", "", 0, 50, 1, 77, 154, 116.28, 0, 0, 2, 100 },
        /* 14*/ { BARCODE_CODE39, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 77, 194, 116.28, 20, 0, 2, 100 },
        /* 15*/ { BARCODE_EXCODE39, -1, -1, -1, -1, "1234", "", 0, 50, 1, 77, 154, 116.28, 0, 0, 2, 100 },
        /* 16*/ { BARCODE_EXCODE39, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 77, 194, 116.28, 20, 0, 2, 100 },
        /* 17*/ { BARCODE_EAN13, -1, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 226, 118, 22, 0, 2, 110 },
        /* 18*/ { BARCODE_EANX, -1, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 226, 118, 22, 0, 2, 110 },
        /* 19*/ { BARCODE_EAN13, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 226, 118, 22, 0, 2, 110 },
        /* 20*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 226, 118, 22, 0, 2, 110 },
        /* 21*/ { BARCODE_EAN13, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 212, 118, 22, 0, 2, 110 },
        /* 22*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 212, 118, 22, 0, 2, 110 },
        /* 23*/ { BARCODE_EAN13, -1, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 226, 110, 22, 0, 2, 110 }, /* Hide text */
        /* 24*/ { BARCODE_EANX, -1, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 226, 110, 22, 0, 2, 110 }, /* Hide text */
        /* 25*/ { BARCODE_EAN13, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 226, 110, 22, 0, 2, 110 }, /* Hide text */
        /* 26*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 226, 110, 22, 0, 2, 110 }, /* Hide text */
        /* 27*/ { BARCODE_EAN13, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 190, 110, 0, 0, 2, 110 }, /* Hide text */
        /* 28*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 190, 110, 0, 0, 2, 110 }, /* Hide text */
        /* 29*/ { BARCODE_EAN13, -1, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 276, 118, 262, 18, 4, 92 },
        /* 30*/ { BARCODE_EANX, -1, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 276, 118, 262, 18, 4, 92 },
        /* 31*/ { BARCODE_EAN13, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 276, 118, 262, 18, 4, 92 },
        /* 32*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 276, 118, 262, 18, 4, 92 },
        /* 33*/ { BARCODE_EAN13, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 266, 118, 262, 18, 4, 92 },
        /* 34*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 266, 118, 262, 18, 4, 92 },
        /* 35*/ { BARCODE_EAN13, -1, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 276, 110, 262, 18, 4, 92 }, /* Hide text */
        /* 36*/ { BARCODE_EANX, -1, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 276, 110, 262, 18, 4, 92 }, /* Hide text */
        /* 37*/ { BARCODE_EAN13, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 276, 110, 262, 18, 4, 92 }, /* Hide text */
        /* 38*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 276, 110, 262, 18, 4, 92 }, /* Hide text */
        /* 39*/ { BARCODE_EAN13, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 244, 110, 240, 18, 4, 92 }, /* Hide text */
        /* 40*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 244, 110, 240, 18, 4, 92 }, /* Hide text */
        /* 41*/ { BARCODE_EANX_CHK, -1, -1, -1, -1, "0234567890129+12345", "", 0, 50, 1, 149, 330, 118, 318, 18, 2, 92 },
        /* 42*/ { BARCODE_EANX_CHK, BARCODE_QUIET_ZONES, -1, -1, -1, "0234567890129+12345", "", 0, 50, 1, 149, 330, 118, 318, 18, 2, 92 },
        /* 43*/ { BARCODE_EANX_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "0234567890129+12345", "", 0, 50, 1, 149, 320, 118, 318, 18, 2, 92 },
        /* 44*/ { BARCODE_EAN8, -1, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 162, 118, 14, 0, 2, 110 }, /* EAN-8 */
        /* 45*/ { BARCODE_EANX, -1, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 162, 118, 14, 0, 2, 110 }, /* EAN-8 */
        /* 46*/ { BARCODE_EAN8, BARCODE_QUIET_ZONES, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 162, 118, 14, 0, 2, 110 }, /* EAN-8 */
        /* 47*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 162, 118, 14, 0, 2, 110 }, /* EAN-8 */
        /* 48*/ { BARCODE_EAN8, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 134, 118, 0, 0, 2, 110 }, /* EAN-8 */
        /* 49*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 134, 118, 0, 0, 2, 110 }, /* EAN-8 */
        /* 50*/ { BARCODE_EAN_5ADDON, -1, -1, -1, -1, "02345", "", 0, 50, 1, 47, 104, 118, 92, 18, 2, 100 }, /* EAN-5 */
        /* 51*/ { BARCODE_EANX, -1, -1, -1, -1, "02345", "", 0, 50, 1, 47, 104, 118, 92, 18, 2, 100 }, /* EAN-5 */
        /* 52*/ { BARCODE_EAN_5ADDON, BARCODE_QUIET_ZONES, -1, -1, -1, "02345", "", 0, 50, 1, 47, 104, 118, 92, 18, 2, 100 }, /* EAN-5 */
        /* 53*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, -1, "02345", "", 0, 50, 1, 47, 104, 118, 92, 18, 2, 100 }, /* EAN-5 */
        /* 54*/ { BARCODE_EAN_5ADDON, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "02345", "", 0, 50, 1, 47, 94, 118, 92, 18, 2, 100 }, /* EAN-5 - only width changes */
        /* 55*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "02345", "", 0, 50, 1, 47, 94, 118, 92, 18, 2, 100 }, /* EAN-5 - only width changes */
        /* 56*/ { BARCODE_EAN_2ADDON, -1, -1, -1, -1, "02", "", 0, 50, 1, 20, 50, 118, 36, 18, 4, 100 }, /* EAN-2 */
        /* 57*/ { BARCODE_EANX, -1, -1, -1, -1, "02", "", 0, 50, 1, 20, 50, 118, 36, 18, 4, 100 }, /* EAN-2 */
        /* 58*/ { BARCODE_EAN_2ADDON, BARCODE_QUIET_ZONES, -1, -1, -1, "02", "", 0, 50, 1, 20, 50, 118, 36, 18, 4, 100 }, /* EAN-2 */
        /* 59*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, -1, "02", "", 0, 50, 1, 20, 50, 118, 36, 18, 4, 100 }, /* EAN-2 */
        /* 60*/ { BARCODE_EAN_2ADDON, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "02", "", 0, 50, 1, 20, 40, 118, 36, 18, 4, 100 }, /* EAN-2 - only width changes */
        /* 61*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "02", "", 0, 50, 1, 20, 40, 118, 36, 18, 4, 100 }, /* EAN-2 - only width changes */
        /* 62*/ { BARCODE_GS1_128, -1, -1, -1, -1, "[20]02", "", 0, 50, 1, 68, 136, 116.28, 0, 0, 4, 100 },
        /* 63*/ { BARCODE_GS1_128, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]02", "", 0, 50, 1, 68, 176, 116.28, 20, 0, 4, 100 },
        /* 64*/ { BARCODE_CODABAR, -1, -1, -1, -1, "A0B", "", 0, 50, 1, 32, 64, 116.28, 0, 0, 2, 100 },
        /* 65*/ { BARCODE_CODABAR, BARCODE_QUIET_ZONES, -1, -1, -1, "A0B", "", 0, 50, 1, 32, 104, 116.28, 20, 0, 2, 100 },
        /* 66*/ { BARCODE_CODE128, -1, -1, -1, -1, "1234", "", 0, 50, 1, 57, 114, 116.28, 0, 0, 4, 100 },
        /* 67*/ { BARCODE_CODE128, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 57, 154, 116.28, 20, 0, 4, 100 },
        /* 68*/ { BARCODE_DPLEIT, -1, -1, -1, -1, "1234", "", 0, 72, 1, 135, 270, 160.28, 0, 0, 2, 144 },
        /* 69*/ { BARCODE_DPLEIT, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 72, 1, 135, 310, 160.28, 20, 0, 2, 144 },
        /* 70*/ { BARCODE_DPIDENT, -1, -1, -1, -1, "1234", "", 0, 72, 1, 117, 234, 160.28, 0, 0, 2, 144 },
        /* 71*/ { BARCODE_DPIDENT, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 72, 1, 117, 274, 160.28, 20, 0, 2, 144 },
        /* 72*/ { BARCODE_CODE16K, -1, -1, -1, -1, "1234", "", 0, 20, 2, 70, 162, 44, 20, 2, 6, 19 },
        /* 73*/ { BARCODE_CODE16K, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 70, 162, 44, 20, 2, 6, 19 },
        /* 74*/ { BARCODE_CODE16K, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 70, 140, 44, 0, 2, 6, 19 },
        /* 75*/ { BARCODE_CODE49, -1, -1, -1, -1, "1234", "", 0, 20, 2, 70, 162, 44, 20, 2, 2, 19 },
        /* 76*/ { BARCODE_CODE49, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 70, 162, 44, 20, 2, 2, 19 },
        /* 77*/ { BARCODE_CODE49, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 70, 140, 44, 0, 2, 2, 19 },
        /* 78*/ { BARCODE_CODE93, -1, -1, -1, -1, "1234", "", 0, 50, 1, 73, 146, 116.28, 0, 0, 2, 100 },
        /* 79*/ { BARCODE_CODE93, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 73, 186, 116.28, 20, 0, 2, 100 },
        /* 80*/ { BARCODE_FLAT, -1, -1, -1, -1, "1234", "", 0, 50, 1, 36, 72, 100, 0, 0, 2, 100 },
        /* 81*/ { BARCODE_FLAT, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 36, 72, 100, 0, 0, 2, 100 },
        /* 82*/ { BARCODE_FLAT, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 36, 72, 100, 0, 0, 2, 100 },
        /* 83*/ { BARCODE_DBAR_OMN, -1, -1, -1, -1, "1234", "", 0, 50, 1, 96, 192, 116.28, 2, 0, 2, 100 },
        /* 84*/ { BARCODE_DBAR_OMN, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 96, 192, 116.28, 2, 0, 2, 100 },
        /* 85*/ { BARCODE_DBAR_OMN, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 96, 192, 116.28, 2, 0, 2, 100 },
        /* 86*/ { BARCODE_DBAR_LTD, -1, -1, -1, -1, "1234", "", 0, 50, 1, 79, 158, 116.28, 2, 0, 2, 100 },
        /* 87*/ { BARCODE_DBAR_LTD, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 79, 158, 116.28, 2, 0, 2, 100 },
        /* 88*/ { BARCODE_DBAR_LTD, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 79, 158, 116.28, 2, 0, 2, 100 },
        /* 89*/ { BARCODE_DBAR_EXP, -1, -1, -1, -1, "[20]02", "", 0, 34, 1, 102, 204, 84.279999, 2, 0, 2, 68 },
        /* 90*/ { BARCODE_DBAR_EXP, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]02", "", 0, 34, 1, 102, 204, 84.279999, 2, 0, 2, 68 },
        /* 91*/ { BARCODE_DBAR_EXP, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "[20]02", "", 0, 34, 1, 102, 204, 84.279999, 2, 0, 2, 68 },
        /* 92*/ { BARCODE_TELEPEN, -1, -1, -1, -1, "1234", "", 0, 50, 1, 112, 224, 116.28, 0, 0, 2, 100 },
        /* 93*/ { BARCODE_TELEPEN, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 112, 264, 116.28, 20, 0, 2, 100 },
        /* 94*/ { BARCODE_UPCA, -1, -1, -1, -1, "01457137763", "", 0, 50, 1, 95, 226, 118, 18, 0, 2, 110 },
        /* 95*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, -1, "01457137763", "", 0, 50, 1, 95, 226, 118, 18, 0, 2, 110 },
        /* 96*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "01457137763", "", 0, 50, 1, 95, 226, 118, 18, 0, 2, 110 },
        /* 97*/ { BARCODE_UPCA, -1, -1, -1, 0, "01457137763", "", 0, 50, 1, 95, 226, 110, 18, 0, 2, 110 }, /* Hide text */
        /* 98*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, 0, "01457137763", "", 0, 50, 1, 95, 226, 110, 18, 0, 2, 110 }, /* Hide text */
        /* 99*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "01457137763", "", 0, 50, 1, 95, 190, 110, 0, 0, 2, 110 }, /* Hide text */
        /*100*/ { BARCODE_UPCA, -1, -1, -1, -1, "01457137763+12", "", 0, 50, 1, 124, 276, 118, 18, 0, 2, 110 },
        /*101*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, -1, "01457137763+12", "", 0, 50, 1, 124, 276, 118, 18, 0, 2, 110 },
        /*102*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "01457137763+12", "", 0, 50, 1, 124, 266, 118, 18, 0, 2, 110 },
        /*103*/ { BARCODE_UPCA, -1, -1, -1, 0, "01457137763+12", "", 0, 50, 1, 124, 276, 110, 18, 0, 2, 110 }, /* Hide text */
        /*104*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, 0, "01457137763+12", "", 0, 50, 1, 124, 276, 110, 18, 0, 2, 110 }, /* Hide text */
        /*105*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "01457137763+12", "", 0, 50, 1, 124, 248, 110, 0, 0, 2, 110 }, /* Hide text */
        /*106*/ { BARCODE_UPCA_CHK, -1, -1, -1, -1, "014571377638+12345", "", 0, 50, 1, 151, 330, 118, 18, 0, 2, 110 },
        /*107*/ { BARCODE_UPCA_CHK, BARCODE_QUIET_ZONES, -1, -1, -1, "014571377638+12345", "", 0, 50, 1, 151, 330, 118, 18, 0, 2, 110 },
        /*108*/ { BARCODE_UPCA_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "014571377638+12345", "", 0, 50, 1, 151, 320, 118, 18, 0, 2, 110 },
        /*109*/ { BARCODE_UPCA_CHK, -1, -1, -1, 0, "014571377638+12345", "", 0, 50, 1, 151, 330, 110, 18, 0, 2, 110 }, /* Hide text */
        /*110*/ { BARCODE_UPCA_CHK, BARCODE_QUIET_ZONES, -1, -1, 0, "014571377638+12345", "", 0, 50, 1, 151, 330, 110, 18, 0, 2, 110 }, /* Hide text */
        /*111*/ { BARCODE_UPCA_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "014571377638+12345", "", 0, 50, 1, 151, 302, 110, 0, 0, 2, 110 }, /* Hide text */
        /*112*/ { BARCODE_UPCE, -1, -1, -1, -1, "145713", "", 0, 50, 1, 51, 134, 118, 18, 0, 2, 110 },
        /*113*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, -1, "145713", "", 0, 50, 1, 51, 134, 118, 18, 0, 2, 110 },
        /*114*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "145713", "", 0, 50, 1, 51, 134, 118, 18, 0, 2, 110 },
        /*115*/ { BARCODE_UPCE, -1, -1, -1, 0, "145713", "", 0, 50, 1, 51, 134, 110, 18, 0, 2, 110 }, /* Hide text */
        /*116*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, 0, "145713", "", 0, 50, 1, 51, 134, 110, 18, 0, 2, 110 }, /* Hide text */
        /*117*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "145713", "", 0, 50, 1, 51, 102, 110, 0, 0, 2, 110 }, /* Hide text */
        /*118*/ { BARCODE_UPCE_CHK, -1, -1, -1, -1, "1457132+12", "", 0, 50, 1, 78, 184, 118, 170, 18, 4, 92 },
        /*119*/ { BARCODE_UPCE_CHK, BARCODE_QUIET_ZONES, -1, -1, -1, "1457132+12", "", 0, 50, 1, 78, 184, 118, 170, 18, 4, 92 },
        /*120*/ { BARCODE_UPCE_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1457132+12", "", 0, 50, 1, 78, 174, 118, 170, 18, 4, 92 },
        /*121*/ { BARCODE_UPCE_CHK, -1, -1, -1, 0, "1457132+12", "", 0, 50, 1, 78, 184, 110, 170, 18, 4, 92 }, /* Hide text */
        /*122*/ { BARCODE_UPCE_CHK, BARCODE_QUIET_ZONES, -1, -1, 0, "1457132+12", "", 0, 50, 1, 78, 184, 110, 170, 18, 4, 92 }, /* Hide text */
        /*123*/ { BARCODE_UPCE_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "1457132+12", "", 0, 50, 1, 78, 156, 110, 152, 18, 4, 92 }, /* Hide text */
        /*124*/ { BARCODE_UPCE, -1, -1, -1, -1, "145713+12345", "", 0, 50, 1, 105, 238, 118, 226, 18, 2, 92 },
        /*125*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, -1, "145713+12345", "", 0, 50, 1, 105, 238, 118, 226, 18, 2, 92 },
        /*126*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "145713+12345", "", 0, 50, 1, 105, 228, 118, 226, 18, 2, 92 },
        /*127*/ { BARCODE_UPCE, -1, -1, -1, 0, "145713+12345", "", 0, 50, 1, 105, 238, 110, 226, 18, 2, 92 }, /* Hide text */
        /*128*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, 0, "145713+12345", "", 0, 50, 1, 105, 238, 110, 226, 18, 2, 92 }, /* Hide text */
        /*129*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "145713+12345", "", 0, 50, 1, 105, 210, 110, 208, 18, 2, 92 }, /* Hide text */
        /*130*/ { BARCODE_POSTNET, -1, -1, -1, -1, "12345", "", 0, 12, 2, 63, 126, 24, 0, 0, 2, 24 },
        /*131*/ { BARCODE_POSTNET, BARCODE_QUIET_ZONES, -1, -1, -1, "12345", "", 0, 12, 2, 63, 146, 30.4, 10, 3.2, 2, 24 },
        /*132*/ { BARCODE_MSI_PLESSEY, -1, -1, -1, -1, "1234", "", 0, 50, 1, 55, 110, 116.28, 0, 0, 4, 100 },
        /*133*/ { BARCODE_MSI_PLESSEY, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 55, 158, 116.28, 24, 0, 4, 100 },
        /*134*/ { BARCODE_FIM, -1, -1, -1, -1, "A", "", 0, 50, 1, 17, 34, 100, 0, 0, 2, 100 },
        /*135*/ { BARCODE_FIM, BARCODE_QUIET_ZONES, -1, -1, -1, "A", "", 0, 50, 1, 17, 50.955414, 100, 10.585987, 0, 2, 100 },
        /*136*/ { BARCODE_LOGMARS, -1, -1, -1, -1, "1234", "", 0, 50, 1, 95, 190, 116.28, 0, 0, 2, 100 },
        /*137*/ { BARCODE_LOGMARS, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 95, 230, 116.28, 20, 0, 2, 100 },
        /*138*/ { BARCODE_PHARMA, -1, -1, -1, -1, "1234", "", 0, 50, 1, 38, 76, 100, 0, 0, 2, 100 },
        /*139*/ { BARCODE_PHARMA, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 38, 100, 100, 12, 0, 2, 100 },
        /*140*/ { BARCODE_PZN, -1, -1, -1, -1, "1234", "", 0, 50, 1, 142, 284, 116.28, 0, 0, 2, 100 },
        /*141*/ { BARCODE_PZN, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 142, 324, 116.28, 20, 0, 2, 100 },
        /*142*/ { BARCODE_PHARMA_TWO, -1, -1, -1, -1, "1234", "", 0, 10, 2, 13, 26, 20, 8, 0, 2, 10 },
        /*143*/ { BARCODE_PHARMA_TWO, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 10, 2, 13, 50, 20, 20, 0, 2, 10 },
        /*144*/ { BARCODE_CEPNET, -1, -1, -1, -1, "12345678", "", 0, 5.375, 2, 93, 186, 10.75, 0, 0, 2, 10.75 },
        /*145*/ { BARCODE_CEPNET, BARCODE_QUIET_ZONES, -1, -1, -1, "12345678", "", 0, 5.375, 2, 93, 226, 17.15, 20, 3.2, 2, 10.75 },
        /*146*/ { BARCODE_PDF417, -1, -1, -1, -1, "1234", "", 0, 18, 6, 103, 206, 36, 0, 0, 16, 36 },
        /*147*/ { BARCODE_PDF417, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 18, 6, 103, 214, 44, 4, 4, 16, 36 },
        /*148*/ { BARCODE_PDF417COMP, -1, -1, -1, -1, "1234", "", 0, 18, 6, 69, 138, 36, 0, 0, 16, 36 },
        /*149*/ { BARCODE_PDF417COMP, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 18, 6, 69, 146, 44, 4, 4, 16, 36 },
        /*150*/ { BARCODE_MAXICODE, -1, -1, -1, -1, "1234", "", 0, 165, 33, 30, 60, 57.733398, 29, 28.866699, 16.430941, 0 },
        /*151*/ { BARCODE_MAXICODE, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 165, 33, 30, 64, 61.733398, 31, 30.866699, 16.430941, 0 },
        /*152*/ { BARCODE_QRCODE, -1, -1, -1, -1, "1234", "", 0, 21, 21, 21, 42, 42, 0, 0, 14, 2 },
        /*153*/ { BARCODE_QRCODE, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 21, 21, 21, 58, 58, 8, 8, 14, 2 },
        /*154*/ { BARCODE_CODE128AB, -1, -1, -1, -1, "1234", "", 0, 50, 1, 79, 158, 116.28, 0, 0, 4, 100 },
        /*155*/ { BARCODE_CODE128AB, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 79, 198, 116.28, 20, 0, 4, 100 },
        /*156*/ { BARCODE_AUSPOST, -1, -1, -1, -1, "12345678", "", 0, 8, 3, 73, 146, 16, 0, 0, 2, 10 },
        /*157*/ { BARCODE_AUSPOST, BARCODE_QUIET_ZONES, -1, -1, -1, "12345678", "", 0, 8, 3, 73, 186, 29.333332, 20, 6.6666665, 2, 10 },
        /*158*/ { BARCODE_AUSREPLY, -1, -1, -1, -1, "1234", "", 0, 8, 3, 73, 146, 16, 0, 0, 2, 10 },
        /*159*/ { BARCODE_AUSREPLY, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 73, 186, 29.333332, 20, 6.6666665, 2, 10 },
        /*160*/ { BARCODE_AUSROUTE, -1, -1, -1, -1, "1234", "", 0, 8, 3, 73, 146, 16, 0, 0, 2, 10 },
        /*161*/ { BARCODE_AUSROUTE, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 73, 186, 29.333332, 20, 6.6666665, 2, 10 },
        /*162*/ { BARCODE_AUSREDIRECT, -1, -1, -1, -1, "1234", "", 0, 8, 3, 73, 146, 16, 0, 0, 2, 10 },
        /*163*/ { BARCODE_AUSREDIRECT, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 73, 186, 29.333332, 20, 6.6666665, 2, 10 },
        /*164*/ { BARCODE_ISBNX, -1, -1, -1, -1, "123456789X", "", 0, 50, 1, 95, 226, 118, 22, 0, 2, 110 },
        /*165*/ { BARCODE_ISBNX, BARCODE_QUIET_ZONES, -1, -1, -1, "123456789X", "", 0, 50, 1, 95, 226, 118, 22, 0, 2, 110 },
        /*166*/ { BARCODE_ISBNX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "123456789X", "", 0, 50, 1, 95, 212, 118, 22, 0, 2, 110 },
        /*167*/ { BARCODE_RM4SCC, -1, -1, -1, -1, "1234", "", 0, 8, 3, 43, 86, 16, 0, 0, 2, 10 },
        /*168*/ { BARCODE_RM4SCC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 43, 98.283463, 28.283464, 6.1417322, 6.1417322, 2, 10 },
        /*169*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, "1234", "", 0, 10, 10, 10, 20, 20, 0, 0, 2, 2 },
        /*170*/ { BARCODE_DATAMATRIX, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 10, 10, 10, 24, 24, 2, 2, 2, 2 },
        /*171*/ { BARCODE_EAN14, -1, -1, -1, -1, "1234", "", 0, 50, 1, 134, 268, 116.28, 0, 0, 4, 100 },
        /*172*/ { BARCODE_EAN14, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 134, 308, 116.28, 20, 0, 4, 100 },
        /*173*/ { BARCODE_VIN, -1, -1, -1, -1, "12345678701234567", "", 0, 50, 1, 246, 492, 116.28, 0, 0, 2, 100 },
        /*174*/ { BARCODE_VIN, BARCODE_QUIET_ZONES, -1, -1, -1, "12345678701234567", "", 0, 50, 1, 246, 532, 116.28, 20, 0, 2, 100 },
        /*175*/ { BARCODE_CODABLOCKF, -1, -1, -1, -1, "1234", "", 0, 20, 2, 101, 242, 44, 20, 2, 4, 40 },
        /*176*/ { BARCODE_CODABLOCKF, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 101, 242, 44, 20, 2, 4, 40 },
        /*177*/ { BARCODE_CODABLOCKF, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 101, 202, 44, 0, 2, 4, 40 },
        /*178*/ { BARCODE_NVE18, -1, -1, -1, -1, "1234", "", 0, 50, 1, 156, 312, 116.28, 0, 0, 4, 100 },
        /*179*/ { BARCODE_NVE18, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 156, 352, 116.28, 20, 0, 4, 100 },
        /*180*/ { BARCODE_JAPANPOST, -1, -1, -1, -1, "1234", "", 0, 8, 3, 133, 266, 16, 0, 0, 2, 16 },
        /*181*/ { BARCODE_JAPANPOST, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 133, 279.33334, 29.333332, 6.6666665, 6.6666665, 2, 16 },
        /*182*/ { BARCODE_KOREAPOST, -1, -1, -1, -1, "1234", "", 0, 50, 1, 167, 334, 116.28, 8, 0, 2, 100 },
        /*183*/ { BARCODE_KOREAPOST, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 167, 374, 116.28, 28, 0, 2, 100 },
        /*184*/ { BARCODE_DBAR_STK, -1, -1, -1, -1, "1234", "", 0, 13, 3, 50, 100, 26, 2, 0, 2, 10 },
        /*185*/ { BARCODE_DBAR_STK, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 13, 3, 50, 100, 26, 2, 0, 2, 10 },
        /*186*/ { BARCODE_DBAR_STK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 13, 3, 50, 100, 26, 2, 0, 2, 10 },
        /*187*/ { BARCODE_DBAR_OMNSTK, -1, -1, -1, -1, "1234", "", 0, 69, 5, 50, 100, 138, 2, 0, 2, 66 },
        /*188*/ { BARCODE_DBAR_OMNSTK, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 69, 5, 50, 100, 138, 2, 0, 2, 66 },
        /*189*/ { BARCODE_DBAR_OMNSTK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 69, 5, 50, 100, 138, 2, 0, 2, 66 },
        /*190*/ { BARCODE_DBAR_EXPSTK, -1, -1, -1, -1, "[20]12", "", 0, 34, 1, 102, 204, 68, 2, 0, 2, 68 },
        /*191*/ { BARCODE_DBAR_EXPSTK, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]12", "", 0, 34, 1, 102, 204, 68, 2, 0, 2, 68 },
        /*192*/ { BARCODE_DBAR_EXPSTK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "[20]12", "", 0, 34, 1, 102, 204, 68, 2, 0, 2, 68 },
        /*193*/ { BARCODE_PLANET, -1, -1, -1, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, 0, 0, 2, 24 },
        /*194*/ { BARCODE_PLANET, BARCODE_QUIET_ZONES, -1, -1, -1, "12345678901", "", 0, 12, 2, 123, 266, 30.4, 10, 3.2, 2, 24 },
        /*195*/ { BARCODE_MICROPDF417, -1, -1, -1, -1, "1234", "", 0, 22, 11, 38, 76, 44, 0, 0, 4, 4 },
        /*196*/ { BARCODE_MICROPDF417, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 22, 11, 38, 80, 48, 2, 2, 4, 4 },
        /*197*/ { BARCODE_USPS_IMAIL, -1, -1, -1, -1, "12345678901234567890", "", 0, 8, 3, 129, 258, 16, 0, 0, 2, 10 },
        /*198*/ { BARCODE_USPS_IMAIL, BARCODE_QUIET_ZONES, -1, -1, -1, "12345678901234567890", "", 0, 8, 3, 129, 277.5, 20.056, 9.75, 2.0280001, 2, 10 },
        /*199*/ { BARCODE_PLESSEY, -1, -1, -1, -1, "1234", "", 0, 50, 1, 131, 262, 116.28, 0, 0, 6, 100 },
        /*200*/ { BARCODE_PLESSEY, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 131, 310, 116.28, 24, 0, 6, 100 },
        /*201*/ { BARCODE_TELEPEN_NUM, -1, -1, -1, -1, "1234", "", 0, 50, 1, 80, 160, 116.28, 0, 0, 2, 100 },
        /*202*/ { BARCODE_TELEPEN_NUM, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 80, 200, 116.28, 20, 0, 2, 100 },
        /*203*/ { BARCODE_ITF14, -1, -1, -1, -1, "1234", "", 0, 50, 1, 135, 330, 136.28, 30, 10, 2, 100 },
        /*204*/ { BARCODE_ITF14, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 135, 330, 136.28, 30, 10, 2, 100 },
        /*205*/ { BARCODE_ITF14, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 135, 290, 136.28, 10, 10, 2, 100 },
        /*206*/ { BARCODE_KIX, -1, -1, -1, -1, "1234", "", 0, 8, 3, 31, 62, 16, 8, 0, 2, 10 },
        /*207*/ { BARCODE_KIX, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 31, 74.283463, 28.283464, 14.141732, 6.1417322, 2, 10 },
        /*208*/ { BARCODE_AZTEC, -1, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 6, 0, 6, 2 },
        /*209*/ { BARCODE_AZTEC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 6, 0, 6, 2 },
        /*210*/ { BARCODE_AZTEC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 6, 0, 6, 2 },
        /*211*/ { BARCODE_DAFT, -1, -1, -1, -1, "FADT", "", 0, 8, 3, 7, 14, 16, 0, 0, 2, 16 },
        /*212*/ { BARCODE_DAFT, BARCODE_QUIET_ZONES, -1, -1, -1, "FADT", "", 0, 8, 3, 7, 14, 16, 0, 0, 2, 16 },
        /*213*/ { BARCODE_DAFT, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "FADT", "", 0, 8, 3, 7, 14, 16, 0, 0, 2, 16 },
        /*214*/ { BARCODE_DPD, -1, -1, -1, -1, "1234567890123456789012345678", "", 0, 50, 1, 189, 378, 122.28, 0, 6, 4, 100 },
        /*215*/ { BARCODE_DPD, BARCODE_QUIET_ZONES, -1, -1, -1, "1234567890123456789012345678", "", 0, 50, 1, 189, 428, 122.28, 25, 6, 4, 100 },
        /*216*/ { BARCODE_MICROQR, -1, -1, -1, -1, "1234", "", 0, 11, 11, 11, 22, 22, 0, 0, 14, 2 },
        /*217*/ { BARCODE_MICROQR, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 11, 11, 11, 30, 30, 4, 4, 14, 2 },
        /*218*/ { BARCODE_HIBC_128, -1, -1, -1, -1, "1234", "", 0, 50, 1, 90, 180, 116.28, 0, 0, 4, 100 },
        /*219*/ { BARCODE_HIBC_128, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 90, 220, 116.28, 20, 0, 4, 100 },
        /*220*/ { BARCODE_HIBC_39, -1, -1, -1, -1, "1234", "", 0, 50, 1, 127, 254, 116.28, 0, 0, 2, 100 },
        /*221*/ { BARCODE_HIBC_39, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 127, 294, 116.28, 20, 0, 2, 100 },
        /*222*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, "1234", "", 0, 12, 12, 12, 24, 24, 0, 0, 2, 2 },
        /*223*/ { BARCODE_HIBC_DM, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 12, 12, 12, 28, 28, 2, 2, 2, 2 },
        /*224*/ { BARCODE_HIBC_QR, -1, -1, -1, -1, "1234", "", 0, 21, 21, 21, 42, 42, 0, 0, 14, 2 },
        /*225*/ { BARCODE_HIBC_QR, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 21, 21, 21, 58, 58, 8, 8, 14, 2 },
        /*226*/ { BARCODE_HIBC_PDF, -1, -1, -1, -1, "1234", "", 0, 21, 7, 103, 206, 42, 0, 0, 16, 42 },
        /*227*/ { BARCODE_HIBC_PDF, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 21, 7, 103, 214, 50, 4, 4, 16, 42 },
        /*228*/ { BARCODE_HIBC_MICPDF, -1, -1, -1, -1, "1234", "", 0, 12, 6, 82, 164, 24, 0, 0, 4, 4 },
        /*229*/ { BARCODE_HIBC_MICPDF, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 12, 6, 82, 168, 28, 2, 2, 4, 4 },
        /*230*/ { BARCODE_HIBC_BLOCKF, -1, -1, -1, -1, "1234", "", 0, 20, 2, 101, 242, 44, 20, 2, 4, 40 },
        /*231*/ { BARCODE_HIBC_BLOCKF, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 101, 242, 44, 20, 2, 4, 40 },
        /*232*/ { BARCODE_HIBC_BLOCKF, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 101, 202, 44, 0, 2, 4, 40 },
        /*233*/ { BARCODE_HIBC_AZTEC, -1, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 22, 0, 2, 2 },
        /*234*/ { BARCODE_HIBC_AZTEC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 22, 0, 2, 2 },
        /*235*/ { BARCODE_HIBC_AZTEC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 22, 0, 2, 2 },
        /*236*/ { BARCODE_DOTCODE, -1, -1, -1, -1, "1234", "", 0, 10, 10, 13, 26, 20, 5, 1, 1.6, 0 },
        /*237*/ { BARCODE_DOTCODE, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 10, 10, 13, 38, 32, 11, 7, 1.6, 0 },
        /*238*/ { BARCODE_HANXIN, -1, -1, -1, -1, "1234", "", 0, 23, 23, 23, 46, 46, 0, 0, 14, 2 },
        /*239*/ { BARCODE_HANXIN, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 23, 23, 23, 58, 58, 6, 6, 14, 2 },
        /*240*/ { BARCODE_MAILMARK_2D, -1, -1, -1, -1, "012100123412345678AB19XY1A 0", "", 0, 24, 24, 24, 48, 48, 0, 0, 2, 2 },
        /*241*/ { BARCODE_MAILMARK_2D, BARCODE_QUIET_ZONES, -1, -1, -1, "012100123412345678AB19XY1A 0", "", 0, 24, 24, 24, 64, 64, 8, 8, 2, 2 },
        /*242*/ { BARCODE_UPU_S10, -1, -1, -1, -1, "EE876543216CA", "", 0, 50, 1, 156, 312, 116.28, 0, 0, 4, 100 },
        /*243*/ { BARCODE_UPU_S10, BARCODE_QUIET_ZONES, -1, -1, -1, "EE876543216CA", "", 0, 50, 1, 156, 352, 116.28, 20, 0, 4, 100 },
        /*244*/ { BARCODE_MAILMARK_4S, -1, -1, -1, -1, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, 0, 0, 2, 20 },
        /*245*/ { BARCODE_MAILMARK_4S, BARCODE_QUIET_ZONES, -1, -1, -1, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 322.28348, 32.283463, 6.1417322, 6.1417322, 2, 20 },
        /*246*/ { BARCODE_AZRUNE, -1, -1, -1, -1, "123", "", 0, 11, 11, 11, 22, 22, 0, 0, 8, 2 },
        /*247*/ { BARCODE_AZRUNE, BARCODE_QUIET_ZONES, -1, -1, -1, "123", "", 0, 11, 11, 11, 22, 22, 0, 0, 8, 2 },
        /*248*/ { BARCODE_AZRUNE, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "123", "", 0, 11, 11, 11, 22, 22, 0, 0, 8, 2 },
        /*249*/ { BARCODE_CODE32, -1, -1, -1, -1, "1234", "", 0, 50, 1, 103, 206, 116.28, 0, 0, 2, 100 },
        /*250*/ { BARCODE_CODE32, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 103, 246, 116.28, 20, 0, 2, 100 },
        /*251*/ { BARCODE_EAN13_CC, -1, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 226, 118, 26, 24, 2, 86 },
        /*252*/ { BARCODE_EANX_CC, -1, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 226, 118, 26, 24, 2, 86 },
        /*253*/ { BARCODE_EAN13_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 226, 118, 26, 24, 2, 86 },
        /*254*/ { BARCODE_EANX_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 226, 118, 26, 24, 2, 86 },
        /*255*/ { BARCODE_EAN13_CC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 214, 118, 26, 24, 2, 86 },
        /*256*/ { BARCODE_EANX_CC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 214, 118, 26, 24, 2, 86 },
        /*257*/ { BARCODE_EAN13_CC, -1, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 226, 110, 26, 24, 2, 86 }, /* Hide text */
        /*258*/ { BARCODE_EANX_CC, -1, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 226, 110, 26, 24, 2, 86 }, /* Hide text */
        /*259*/ { BARCODE_EAN13_CC, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 226, 110, 26, 24, 2, 86 }, /* Hide text */
        /*260*/ { BARCODE_EANX_CC, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 226, 110, 26, 24, 2, 86 }, /* Hide text */
        /*261*/ { BARCODE_EAN13_CC, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 198, 110, 10, 24, 2, 86 }, /* Hide text */
        /*262*/ { BARCODE_EANX_CC, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 198, 110, 10, 24, 2, 86 }, /* Hide text */
        /*263*/ { BARCODE_GS1_128_CC, -1, -1, -1, -1, "[20]02", "", 0, 50, 5, 99, 198, 116.28, 24, 14, 4, 86 }, /* CC-A */
        /*264*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]02", "", 0, 50, 5, 99, 204, 116.28, 26, 14, 4, 86 },
        /*265*/ { BARCODE_GS1_128_CC, -1, -1, -1, -1, "[91]1", "", 0, 50, 5, 100, 200, 116.28, 20, 14, 4, 86 }, /* CC-A */
        /*266*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "[91]1", "", 0, 50, 5, 100, 222, 116.28, 22, 14, 4, 86 },
        /*267*/ { BARCODE_GS1_128_CC, -1, 2, -1, -1, "[91]1", "", 0, 50, 6, 100, 200, 116.28, 20, 18, 4, 82 }, /* CC-B */
        /*268*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "[91]1", "", 0, 50, 6, 100, 222, 116.28, 22, 18, 4, 82 },
        /*269*/ { BARCODE_GS1_128_CC, -1, 3, -1, -1, "[20]02", "", 0, 50, 15, 86, 172, 116.28, 14, 80, 4, 20 }, /* CC-C */
        /*270*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, 3, -1, -1, "[20]02", "", 0, 50, 15, 86, 198, 116.28, 20, 80, 4, 20 },
        /*271*/ { BARCODE_DBAR_OMN_CC, -1, -1, -1, -1, "1234", "", 0, 21, 5, 100, 200, 58.279999, 10, 14, 2, 28 }, /* CC-A */
        /*272*/ { BARCODE_DBAR_OMN_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 21, 5, 100, 202, 58.279999, 12, 14, 2, 28 },
        /*273*/ { BARCODE_DBAR_OMN_CC, -1, 2, -1, -1, "1234", "", 0, 23, 6, 100, 200, 62.279999, 10, 18, 2, 28 }, /* CC-B */
        /*274*/ { BARCODE_DBAR_OMN_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "1234", "", 0, 23, 6, 100, 202, 62.279999, 12, 18, 2, 28 },
        /*275*/ { BARCODE_DBAR_LTD_CC, -1, -1, -1, -1, "1234", "", 0, 19, 6, 79, 158, 54.279999, 2, 18, 2, 20 }, /* CC-A */
        /*276*/ { BARCODE_DBAR_LTD_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 19, 6, 79, 158, 54.279999, 2, 18, 2, 20 }, /* Same */
        /*277*/ { BARCODE_DBAR_LTD_CC, -1, 2, -1, -1, "1234", "", 0, 23, 8, 88, 176, 62.279999, 20, 26, 2, 20 }, /* CC-B */
        /*278*/ { BARCODE_DBAR_LTD_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "1234", "", 0, 23, 8, 88, 178, 62.279999, 22, 26, 2, 20 },
        /*279*/ { BARCODE_DBAR_EXP_CC, -1, -1, -1, -1, "[20]12", "", 0, 41, 5, 102, 204, 98.279999, 2, 14, 2, 68 }, /* CC-A */
        /*280*/ { BARCODE_DBAR_EXP_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]12", "", 0, 41, 5, 102, 204, 98.279999, 2, 14, 2, 68 }, /* Same */
        /*281*/ { BARCODE_DBAR_EXP_CC, -1, 2, -1, -1, "[20]12", "", 0, 43, 6, 102, 204, 102.28, 2, 18, 2, 68 }, /* CC-B */
        /*282*/ { BARCODE_DBAR_EXP_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "[20]12", "", 0, 43, 6, 102, 204, 102.28, 2, 18, 2, 68 }, /* Same */
        /*283*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, "01457137763", "", 0, 50, 7, 99, 226, 118, 18, 20, 2, 90 },
        /*284*/ { BARCODE_UPCA_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "01457137763", "", 0, 50, 7, 99, 226, 118, 18, 20, 2, 90 },
        /*285*/ { BARCODE_UPCA_CC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "01457137763", "", 0, 50, 7, 99, 226, 118, 18, 20, 2, 90 },
        /*286*/ { BARCODE_UPCA_CC, -1, -1, -1, 0, "01457137763", "", 0, 50, 7, 99, 226, 110, 18, 20, 2, 90 }, /* Hide text */
        /*287*/ { BARCODE_UPCA_CC, BARCODE_QUIET_ZONES, -1, -1, 0, "01457137763", "", 0, 50, 7, 99, 226, 110, 18, 20, 2, 90 }, /* Hide text */
        /*288*/ { BARCODE_UPCA_CC, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "01457137763", "", 0, 50, 7, 99, 198, 110, 6, 20, 2, 90 }, /* Hide text */
        /*289*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, "145713", "", 0, 50, 9, 55, 134, 118, 18, 28, 2, 82 },
        /*290*/ { BARCODE_UPCE_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "145713", "", 0, 50, 9, 55, 134, 118, 18, 28, 2, 82 },
        /*291*/ { BARCODE_UPCE_CC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "145713", "", 0, 50, 9, 55, 134, 118, 18, 28, 2, 82 },
        /*292*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, "145713", "", 0, 50, 9, 55, 134, 110, 18, 28, 2, 82 }, /* Hide text */
        /*293*/ { BARCODE_UPCE_CC, BARCODE_QUIET_ZONES, -1, -1, 0, "145713", "", 0, 50, 9, 55, 134, 110, 18, 28, 2, 82 }, /* Hide text */
        /*294*/ { BARCODE_UPCE_CC, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "145713", "", 0, 50, 9, 55, 110, 110, 6, 28, 2, 82 }, /* Hide text */
        /*295*/ { BARCODE_DBAR_STK_CC, -1, -1, -1, -1, "1234", "", 0, 24, 9, 56, 112, 48, 0, 34, 2, 14 }, /* CC-A */
        /*296*/ { BARCODE_DBAR_STK_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 24, 9, 56, 114, 48, 0, 34, 2, 14 },
        /*297*/ { BARCODE_DBAR_STK_CC, -1, 2, -1, -1, "1234", "", 0, 30, 12, 56, 112, 60, 0, 46, 2, 14 }, /* CC-B */
        /*298*/ { BARCODE_DBAR_STK_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "1234", "", 0, 30, 12, 56, 114, 60, 0, 46, 2, 14 },
        /*299*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, -1, -1, "1234", "", 0, 80, 11, 56, 112, 160, 0, 94, 2, 66 }, /* CC-A */
        /*300*/ { BARCODE_DBAR_OMNSTK_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 80, 11, 56, 114, 160, 0, 94, 2, 66 },
        /*301*/ { BARCODE_DBAR_OMNSTK_CC, -1, 2, -1, -1, "1234", "", 0, 86, 14, 56, 112, 172, 0, 106, 2, 66 }, /* CC-B */
        /*302*/ { BARCODE_DBAR_OMNSTK_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "1234", "", 0, 86, 14, 56, 114, 172, 0, 106, 2, 66 },
        /*303*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, -1, -1, "[20]12", "", 0, 41, 5, 102, 204, 82, 2, 14, 2, 68 }, /* CC-A */
        /*304*/ { BARCODE_DBAR_EXPSTK_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]12", "", 0, 41, 5, 102, 204, 82, 2, 14, 2, 68 }, /* Same */
        /*305*/ { BARCODE_DBAR_EXPSTK_CC, -1, 2, -1, -1, "[20]12", "", 0, 43, 6, 102, 204, 86, 2, 18, 2, 68 }, /* CC-B */
        /*306*/ { BARCODE_DBAR_EXPSTK_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "[20]12", "", 0, 43, 6, 102, 204, 86, 2, 18, 2, 68 }, /* Same */
        /*307*/ { BARCODE_CHANNEL, -1, -1, -1, -1, "1234", "", 0, 50, 1, 27, 54, 116.28, 0, 0, 2, 100 },
        /*308*/ { BARCODE_CHANNEL, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 27, 60, 116.28, 2, 0, 2, 100 },
        /*309*/ { BARCODE_CODEONE, -1, -1, -1, -1, "1234", "", 0, 16, 16, 18, 36, 32, 0, 0, 2, 2 }, /* Versions A to H - no quiet zone */
        /*310*/ { BARCODE_CODEONE, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 16, 16, 18, 36, 32, 0, 0, 2, 2 },
        /*311*/ { BARCODE_CODEONE, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 16, 16, 18, 36, 32, 0, 0, 2, 2 },
        /*312*/ { BARCODE_CODEONE, -1, -1, 9, -1, "1234", "", 0, 8, 8, 11, 22, 16, 10, 0, 2, 2 }, /* Version S (& T) have quiet zones */
        /*313*/ { BARCODE_CODEONE, BARCODE_QUIET_ZONES, -1, 9, -1, "1234", "", 0, 8, 8, 11, 26, 18, 12, 0, 2, 2 },
        /*314*/ { BARCODE_GRIDMATRIX, -1, -1, -1, -1, "123", "", 0, 18, 18, 18, 36, 36, 0, 0, 12, 2 },
        /*315*/ { BARCODE_GRIDMATRIX, BARCODE_QUIET_ZONES, -1, -1, -1, "123", "", 0, 18, 18, 18, 60, 60, 12, 12, 12, 2 },
        /*316*/ { BARCODE_UPNQR, -1, -1, -1, -1, "1234", "", 0, 77, 77, 77, 154, 154, 0, 0, 14, 2 },
        /*317*/ { BARCODE_UPNQR, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 77, 77, 77, 170, 170, 8, 8, 14, 2 },
        /*318*/ { BARCODE_ULTRA, -1, -1, -1, -1, "1234", "", 0, 13, 13, 15, 30, 26, 0, 0, 30, 2 },
        /*319*/ { BARCODE_ULTRA, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 13, 13, 15, 34, 30, 2, 2, 30, 2 },
        /*320*/ { BARCODE_RMQR, -1, -1, -1, -1, "1234", "", 0, 11, 11, 27, 54, 22, 0, 0, 14, 2 },
        /*321*/ { BARCODE_RMQR, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 11, 11, 27, 62, 30, 4, 4, 14, 2 },
        /*322*/ { BARCODE_BC412, -1, -1, -1, -1, "1234567", "", 0, 16.666666, 1, 102, 204, 49.613335, 0, 0, 2, 33.333336 },
        /*323*/ { BARCODE_BC412, BARCODE_QUIET_ZONES, -1, -1, -1, "1234567", "", 0, 16.666666, 1, 102, 244, 49.613335, 20, 0, 2, 33.333336 },
        /*324*/ { BARCODE_DXFILMEDGE, -1, -1, -1, -1, "120476", "", 0, 6, 2, 23, 46, 12, 0, 0, 10, 6 },
        /*325*/ { BARCODE_DXFILMEDGE, BARCODE_QUIET_ZONES, -1, -1, -1, "120476", "", 0, 6, 2, 23, 53.2000008, 12, 3.6, 0, 10, 6 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;
    static const char composite[] = "[20]12";

    struct zint_vector_rect *rect;
    struct zint_vector_circle *circle;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }

        if (z_is_composite(symbol->symbology)) {
            text = *(data[i].composite) ? data[i].composite : composite;
            length = (int) strlen(text);
            assert_nonzero(strlen(data[i].data) < 128, "i:%d linear data length %d >= 128\n", i, (int) strlen(data[i].data));
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != %d\n", i, data[i].symbology, ret, data[i].ret);

        if (ret < ZINT_ERROR) {
            assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

            if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) { /* ZINT_DEBUG_TEST_PRINT 16 */
                testUtilVectorPrint(symbol);
            }

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.9g != %.9g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

            if (symbol->symbology == BARCODE_MAXICODE || symbol->symbology == BARCODE_DOTCODE) {
                circle = find_circle(symbol, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width);
                assert_nonnull(circle, "i:%d (%s) find_circle(%.9g, %.9g, %.9g) NULL\n",
                                        i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_x,
                                        data[i].expected_set_y, data[i].expected_set_width);
            } else {
                rect = find_rect(symbol, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
                assert_nonnull(rect, "i:%d (%s) find_rect(%.9g, %.9g, %.9g, %.9g) NULL\n",
                                        i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_x,
                                        data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_text_gap(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        int option_2;
        int show_hrt;
        float text_gap;
        float scale;
        const char *data;
        const char *composite;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
        float expected_set_x;
        float expected_set_y;
        float expected_set_width;
        float expected_set_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, -1, -1, 1, 0, "1234", "", 0, 50, 1, 62, 124, 116.28, 62.0, 113.3444, -1, -1 }, /* Default */
        /*  1*/ { BARCODE_CODE11, -1, -1, -1, 0, 0, "1234", "", 0, 50, 1, 62, 124, 114.28, 62.0, 111.3444, -1, -1 },
        /*  2*/ { BARCODE_CODE11, -1, -1, -1, 0.1, 0, "1234", "", 0, 50, 1, 62, 124, 114.479996, 62.0, 111.5444, -1, -1 },
        /*  3*/ { BARCODE_CODE11, -1, -1, -1, 0.2, 0, "1234", "", 0, 50, 1, 62, 124, 114.68, 62.0, 111.7444, -1, -1 },
        /*  4*/ { BARCODE_CODE11, -1, -1, -1, 0.3, 0, "1234", "", 0, 50, 1, 62, 124, 114.88, 62.0, 111.9444, -1, -1 },
        /*  5*/ { BARCODE_CODE11, -1, -1, -1, 0.4, 0, "1234", "", 0, 50, 1, 62, 124, 115.08, 62.0, 112.1444, -1, -1 },
        /*  6*/ { BARCODE_CODE11, -1, -1, -1, 0.5, 0, "1234", "", 0, 50, 1, 62, 124, 115.28, 62.0, 112.3444, -1, -1 },
        /*  7*/ { BARCODE_CODE11, -1, -1, -1, 0.6, 0, "1234", "", 0, 50, 1, 62, 124, 115.479996, 62.0, 112.5444, -1, -1 },
        /*  8*/ { BARCODE_CODE11, -1, -1, -1, 0.7, 0, "1234", "", 0, 50, 1, 62, 124, 115.68, 62.0, 112.7444, -1, -1 },
        /*  9*/ { BARCODE_CODE11, -1, -1, -1, 0.75, 0, "1234", "", 0, 50, 1, 62, 124, 115.78, 62.0, 112.8444, -1, -1 },
        /* 10*/ { BARCODE_CODE11, -1, -1, -1, 0.8, 0, "1234", "", 0, 50, 1, 62, 124, 115.88, 62.0, 112.9444, -1, -1 },
        /* 11*/ { BARCODE_CODE11, -1, -1, -1, 0.9, 0, "1234", "", 0, 50, 1, 62, 124, 116.08, 62.0, 113.1444, -1, -1 },
        /* 12*/ { BARCODE_CODE11, -1, -1, -1, 1.1, 0, "1234", "", 0, 50, 1, 62, 124, 116.479996, 62.0, 113.5444, -1, -1 },
        /* 13*/ { BARCODE_CODE11, -1, -1, -1, 1.2, 0, "1234", "", 0, 50, 1, 62, 124, 116.68, 62.0, 113.7444, -1, -1 },
        /* 14*/ { BARCODE_CODE11, -1, -1, -1, 1.3, 0, "1234", "", 0, 50, 1, 62, 124, 116.88, 62.0, 113.9444, -1, -1 },
        /* 15*/ { BARCODE_CODE11, -1, -1, -1, 1.4, 0, "1234", "", 0, 50, 1, 62, 124, 117.08, 62.0, 114.1444, -1, -1 },
        /* 16*/ { BARCODE_CODE11, -1, -1, -1, 1.5, 0, "1234", "", 0, 50, 1, 62, 124, 117.28, 62.0, 114.3444, -1, -1 },
        /* 17*/ { BARCODE_CODE11, -1, -1, -1, 2.0, 0, "1234", "", 0, 50, 1, 62, 124, 118.28, 62.0, 115.3444, -1, -1 },
        /* 18*/ { BARCODE_CODE11, -1, -1, -1, 2.1, 0, "1234", "", 0, 50, 1, 62, 124, 118.479996, 62.0, 115.5444, -1, -1 },
        /* 19*/ { BARCODE_CODE11, -1, -1, -1, 2.6, 0, "1234", "", 0, 50, 1, 62, 124, 119.479996, 62.0, 116.5444, -1, -1 },
        /* 20*/ { BARCODE_CODE11, -1, -1, -1, 3.0, 0, "1234", "", 0, 50, 1, 62, 124, 120.28, 62.0, 117.3444, -1, -1 },
        /* 21*/ { BARCODE_CODE11, -1, -1, -1, 4.0, 0, "1234", "", 0, 50, 1, 62, 124, 122.28, 62.0, 119.3444, -1, -1 },
        /* 22*/ { BARCODE_CODE11, -1, -1, -1, 5.0, 0, "1234", "", 0, 50, 1, 62, 124, 124.28, 62.0, 121.3444, -1, -1 },
        /* 23*/ { BARCODE_CODE11, -1, -1, -1, 10.0, 0, "1234", "", 0, 50, 1, 62, 124, 134.28, 62.0, 131.3444, -1, -1 },
        /* 24*/ { BARCODE_CODE11, -1, -1, -1, -1.0, 0, "1234", "", 0, 50, 1, 62, 124, 112.28, 62.0, 109.344406, -1, -1 },
        /* 25*/ { BARCODE_CODE11, -1, -1, -1, -0.5, 0, "1234", "", 0, 50, 1, 62, 124, 113.28, 62.0, 110.344406, -1, -1 },
        /* 26*/ { BARCODE_CODE11, -1, -1, -1, 1, 3.0, "1234", "", 0, 50, 1, 62, 372, 348.84, 186.0, 340.0332, -1, -1 }, /* Scale default */
        /* 27*/ { BARCODE_CODE11, -1, -1, -1, 0, 3.0, "1234", "", 0, 50, 1, 62, 372, 342.84, 186.0, 334.0332, -1, -1 }, /* Scale */
        /* 28*/ { BARCODE_CODE11, -1, -1, -1, 0.1, 3.0, "1234", "", 0, 50, 1, 62, 372, 343.44, 186.0, 334.6332, -1, -1 }, /* Scale */
        /* 29*/ { BARCODE_UPCA, -1, -1, -1, 1, 0, "01457130763", "", 0, 50, 1, 95, 226, 118, 74.0, 117.2, -1, -1 }, /* Default */
        /* 30*/ { BARCODE_UPCA, -1, -1, -1, 0, 0, "01457130763", "", 0, 50, 1, 95, 226, 116, 74.0, 115.2, -1, -1 },
        /* 31*/ { BARCODE_UPCA, -1, -1, -1, 0.1, 0, "01457130763", "", 0, 50, 1, 95, 226, 116.2, 74.0, 115.4, -1, -1 },
        /* 32*/ { BARCODE_UPCA, -1, -1, -1, 0.6, 0, "01457130763", "", 0, 50, 1, 95, 226, 117.2, 74.0, 116.4, -1, -1 },
        /* 33*/ { BARCODE_UPCA, -1, -1, -1, 0.7, 0, "01457130763", "", 0, 50, 1, 95, 226, 117.4, 74.0, 116.6, -1, -1 },
        /* 34*/ { BARCODE_UPCA, -1, -1, -1, 0.75, 0, "01457130763", "", 0, 50, 1, 95, 226, 117.5, 74.0, 116.7, -1, -1 },
        /* 35*/ { BARCODE_UPCA, -1, -1, -1, 0.8, 0, "01457130763", "", 0, 50, 1, 95, 226, 117.6, 74.0, 116.8, -1, -1 },
        /* 36*/ { BARCODE_UPCA, -1, -1, -1, 1.1, 0, "01457130763", "", 0, 50, 1, 95, 226, 118.2, 74.0, 117.4, -1, -1 },
        /* 37*/ { BARCODE_UPCA, -1, -1, -1, 1.6, 0, "01457130763", "", 0, 50, 1, 95, 226, 119.2, 74.0, 118.4, -1, -1 },
        /* 38*/ { BARCODE_UPCA, -1, -1, -1, 1.6, 2.5, "01457130763", "", 0, 50, 1, 95, 565, 298, 185.0, 296.0, -1, -1 }, /* Scale */
        /* 39*/ { BARCODE_UPCA, -1, -1, -1, 1, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 118, 74.0, 117.2, -1, -1 }, /* Default */
        /* 40*/ { BARCODE_UPCA, -1, -1, -1, 1, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 118, 230.0, 18, 4.0, 92 }, /* Default */
        /* 41*/ { BARCODE_UPCA, -1, -1, -1, 0, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 116, 230.0, 16, 4.0, 94 },
        /* 42*/ { BARCODE_UPCA, -1, -1, -1, 0.1, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 116.2, 230.0, 16.2, 4.0, 93.8 },
        /* 43*/ { BARCODE_UPCA, -1, -1, -1, 0.6, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 117.2, 230.0, 17.2, 4.0, 92.8 },
        /* 44*/ { BARCODE_UPCA, -1, -1, -1, 0.75, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 117.5, 230.0, 17.5, 4.0, 92.5 },
        /* 45*/ { BARCODE_UPCA, -1, -1, -1, 0.9, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 117.8, 230.0, 17.8, 4.0, 92.2 },
        /* 46*/ { BARCODE_UPCA, -1, -1, -1, 1.1, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 118.2, 74.0, 117.4, -1, -1 },
        /* 47*/ { BARCODE_UPCA, -1, -1, -1, 4.2, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 124.4, 230.0, 24.4, 4.0, 85.6 },
        /* 48*/ { BARCODE_UPCA_CC, -1, -1, -1, 1, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 118, 74.0, 117.2, -1, -1 }, /* Default */
        /* 49*/ { BARCODE_UPCA_CC, -1, -1, -1, 0, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 116, 74.0, 115.2, -1, -1 },
        /* 50*/ { BARCODE_UPCA_CC, -1, -1, -1, 0.1, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 116.2, 74.0, 115.4, -1, -1 },
        /* 51*/ { BARCODE_UPCA_CC, -1, -1, -1, 1, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 118, 236.0, 42, 4.0, 68 }, /* Default */
        /* 52*/ { BARCODE_UPCA_CC, -1, -1, -1, 0, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 116, 236.0, 40, 4.0, 70 },
        /* 53*/ { BARCODE_UPCA_CC, -1, -1, -1, 0.1, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 116.2, 236.0, 40.2, 4.0, 69.8 },
        /* 54*/ { BARCODE_UPCA_CC, -1, -1, -1, 0.6, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 117.2, 236.0, 41.2, 4.0, 68.8 },
        /* 55*/ { BARCODE_UPCA_CC, -1, -1, -1, 0.75, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 117.5, 236.0, 41.5, 4.0, 68.5 },
        /* 56*/ { BARCODE_UPCA_CC, -1, -1, -1, 1.1, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 118.2, 236.0, 42.2, 4.0, 67.8 },
        /* 57*/ { BARCODE_UPCA_CC, -1, -1, -1, 1.5, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 119.0, 236.0, 43.0, 4.0, 67.0 },
        /* 58*/ { BARCODE_UPCA_CC, -1, -1, 0, 1, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 110, 236.0, 42, 4.0, 68 }, /* Hide text default */
        /* 59*/ { BARCODE_UPCA_CC, -1, -1, 0, 0, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 110, 236.0, 40, 4.0, 70 }, /* Hide text */
        /* 60*/ { BARCODE_UPCA_CC, -1, -1, 0, 1.5, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 110, 236.0, 43.0, 4.0, 67.0 }, /* Hide text */
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;

    struct zint_vector_string *string;
    struct zint_vector_rect *rect;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }
        symbol->text_gap = data[i].text_gap;
        if (data[i].scale != 0.0f) {
            symbol->scale = data[i].scale;
        }

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, data[i].output_options, text, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != %d\n", i, data[i].symbology, ret, data[i].ret);

        if (ret < ZINT_ERROR) {
            assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

            if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) { /* ZINT_DEBUG_TEST_PRINT 16 */
                testUtilVectorPrint(symbol);
            }

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.9g != %.9g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

            if (data[i].expected_set_width == -1.0f) {
                string = find_string(symbol, data[i].expected_set_x, data[i].expected_set_y);
                assert_nonnull(string, "i:%d (%s) find_string(%.9g, %.9g) NULL\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_x, data[i].expected_set_y);
            } else {
                rect = find_rect(symbol, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
                assert_nonnull(rect, "i:%d (%s) find_rect(%.9g, %.9g, %.9g, %.9g) NULL\n",
                                        i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_x,
                                        data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_height(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        float height;
        const char *data;
        const char *composite;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;

        const char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, 1, "1234567890", "", 0, 1, 1, 108, 216, 2, "" },
        /*  1*/ { BARCODE_CODE11, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 108, 216, 2, "TODO: Find doc" },
        /*  2*/ { BARCODE_CODE11, -1, 4, "1234567890", "", 0, 4, 1, 108, 216, 8, "" },
        /*  3*/ { BARCODE_CODE11, -1, 10, "1234567890", "", 0, 10, 1, 108, 216, 20, "" },
        /*  4*/ { BARCODE_C25STANDARD, -1, 1, "1234567890", "", 0, 1, 1, 117, 234, 2, "" },
        /*  5*/ { BARCODE_C25STANDARD, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 117, 234, 2, "No doc for C25 except C25INTER" },
        /*  6*/ { BARCODE_C25STANDARD, -1, 4, "1234567890", "", 0, 4, 1, 117, 234, 8, "" },
        /*  7*/ { BARCODE_C25STANDARD, -1, 11, "1234567890", "", 0, 11, 1, 117, 234, 22, "" },
        /*  8*/ { BARCODE_C25INTER, -1, 1, "1234567890", "", 0, 1, 1, 99, 198, 2, "" },
        /*  9*/ { BARCODE_C25INTER, -1, 15, "1234567890", "", 0, 15, 1, 99, 198, 30, "" },
        /* 10*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 15, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 15, 1, 99, 198, 30, "" },
        /* 11*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 15.5, "1234567890", "", 0, 15.5, 1, 99, 198, 31, "" },
        /* 12*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 17.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 17.5, 1, 117, 234, 35, "" },
        /* 13*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 17.75, "12345678901", "", 0, 17.75, 1, 117, 234, 35.5, "" },
        /* 14*/ { BARCODE_C25IATA, -1, 1, "1234567890", "", 0, 1, 1, 149, 298, 2, "" },
        /* 15*/ { BARCODE_C25IATA, -1, 4, "1234567890", "", 0, 4, 1, 149, 298, 8, "" },
        /* 16*/ { BARCODE_C25IATA, -1, 30, "1234567890", "", 0, 30, 1, 149, 298, 60, "" },
        /* 17*/ { BARCODE_C25LOGIC, -1, 1, "1234567890", "", 0, 1, 1, 109, 218, 2, "" },
        /* 18*/ { BARCODE_C25LOGIC, -1, 4, "1234567890", "", 0, 4, 1, 109, 218, 8, "" },
        /* 19*/ { BARCODE_C25LOGIC, -1, 41, "1234567890", "", 0, 41, 1, 109, 218, 82, "" },
        /* 20*/ { BARCODE_C25IND, -1, 1, "1234567890", "", 0, 1, 1, 159, 318, 2, "" },
        /* 21*/ { BARCODE_C25IND, -1, 4, "1234567890", "", 0, 4, 1, 159, 318, 8, "" },
        /* 22*/ { BARCODE_C25IND, -1, 21, "1234567890", "", 0, 21, 1, 159, 318, 42, "" },
        /* 23*/ { BARCODE_CODE39, -1, 1, "1234567890", "", 0, 1, 1, 155, 310, 2, "" },
        /* 24*/ { BARCODE_CODE39, -1, 4, "1", "", 0, 4, 1, 38, 76, 8, "" },
        /* 25*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 4, "1", "", ZINT_WARN_NONCOMPLIANT, 4, 1, 38, 76, 8, "Min height data-length dependent" },
        /* 26*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 4.4, "1", "", 0, 4.4000001, 1, 38, 76, 8.8000002, "" },
        /* 27*/ { BARCODE_CODE39, -1, 17, "1234567890", "", 0, 17, 1, 155, 310, 34, "" },
        /* 28*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 17, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17, 1, 155, 310, 34, "Min height data-length dependent" },
        /* 29*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 17.85, "1234567890", "", 0, 17.85, 1, 155, 310, 35.700001, "" },
        /* 30*/ { BARCODE_EXCODE39, -1, 1, "1234567890", "", 0, 1, 1, 155, 310, 2, "" },
        /* 31*/ { BARCODE_EXCODE39, -1, 17.8, "1234567890", "", 0, 17.799999, 1, 155, 310, 35.599998, "" },
        /* 32*/ { BARCODE_EXCODE39, COMPLIANT_HEIGHT, 17.8, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17.799999, 1, 155, 310, 35.599998, "" },
        /* 33*/ { BARCODE_EXCODE39, COMPLIANT_HEIGHT, 17.9, "1234567890", "", 0, 17.9, 1, 155, 310, 35.799999, "" },
        /* 34*/ { BARCODE_EAN13, -1, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "EAN-13" },
        /* 35*/ { BARCODE_EANX, -1, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "EAN-13" },
        /* 36*/ { BARCODE_EAN13, -1, 69, "123456789012", "", 0, 69, 1, 95, 226, 148, "" },
        /* 37*/ { BARCODE_EANX, -1, 69, "123456789012", "", 0, 69, 1, 95, 226, 148, "" },
        /* 38*/ { BARCODE_EAN13, COMPLIANT_HEIGHT, 69, "123456789012", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /* 39*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 69, "123456789012", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /* 40*/ { BARCODE_EAN13, COMPLIANT_HEIGHT, 69.25, "123456789012", "", 0, 69.25, 1, 95, 226, 148.5, "" },
        /* 41*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 69.25, "123456789012", "", 0, 69.25, 1, 95, 226, 148.5, "" },
        /* 42*/ { BARCODE_EAN8, -1, 55, "1234567", "", 0, 55, 1, 67, 162, 120, "EAN-8" },
        /* 43*/ { BARCODE_EANX, -1, 55, "1234567", "", 0, 55, 1, 67, 162, 120, "EAN-8" },
        /* 44*/ { BARCODE_EAN8, COMPLIANT_HEIGHT, 55, "1234567", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 67, 162, 120, "EAN-8" },
        /* 45*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 55, "1234567", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 67, 162, 120, "EAN-8" },
        /* 46*/ { BARCODE_EAN8, COMPLIANT_HEIGHT, 55.25, "1234567", "", 0, 55.25, 1, 67, 162, 120.5, "EAN-8" },
        /* 47*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 55.25, "1234567", "", 0, 55.25, 1, 67, 162, 120.5, "EAN-8" },
        /* 48*/ { BARCODE_EANX_CHK, -1, 1, "1234567890128", "", 0, 1, 1, 95, 226, 12, "EAN-13" },
        /* 49*/ { BARCODE_EANX_CHK, -1, 69, "1234567890128", "", 0, 69, 1, 95, 226, 148, "" },
        /* 50*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, 69, "1234567890128", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /* 51*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, 69.25, "1234567890128", "", 0, 69.25, 1, 95, 226, 148.5, "" },
        /* 52*/ { BARCODE_GS1_128, -1, 1, "[01]12345678901231", "", 0, 1, 1, 134, 268, 2, "" },
        /* 53*/ { BARCODE_GS1_128, -1, 5.7, "[01]12345678901231", "", 0, 5.6999998, 1, 134, 268, 11.4, "" },
        /* 54*/ { BARCODE_GS1_128, COMPLIANT_HEIGHT, 5.7, "[01]12345678901231", "", ZINT_WARN_NONCOMPLIANT, 5.6999998, 1, 134, 268, 11.4, "" },
        /* 55*/ { BARCODE_GS1_128, COMPLIANT_HEIGHT, 5.725, "[01]12345678901231", "", 0, 5.7249999, 1, 134, 268, 11.45, "Note considered compliant even though rendered height same as before" },
        /* 56*/ { BARCODE_GS1_128, -1, 50, "[01]12345678901231", "", 0, 50, 1, 134, 268, 100, "" },
        /* 57*/ { BARCODE_CODABAR, -1, 1, "A0B", "", 0, 1, 1, 32, 64, 2, "" },
        /* 58*/ { BARCODE_CODABAR, -1, 4, "A0B", "", 0, 4, 1, 32, 64, 8, "" },
        /* 59*/ { BARCODE_CODABAR, -1, 26, "A0B", "", 0, 26, 1, 32, 64, 52, "" },
        /* 60*/ { BARCODE_CODABAR, COMPLIANT_HEIGHT, 11, "A0B", "", ZINT_WARN_NONCOMPLIANT, 11, 1, 32, 64, 22, "" },
        /* 61*/ { BARCODE_CODABAR, COMPLIANT_HEIGHT, 12, "A0B", "", 0, 12, 1, 32, 64, 24, "" },
        /* 62*/ { BARCODE_CODE128, -1, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "" },
        /* 63*/ { BARCODE_CODE128, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "" },
        /* 64*/ { BARCODE_CODE128, -1, 4, "1234567890", "", 0, 4, 1, 90, 180, 8, "" },
        /* 65*/ { BARCODE_CODE128, -1, 7, "1234567890", "", 0, 7, 1, 90, 180, 14, "" },
        /* 66*/ { BARCODE_DPLEIT, -1, 1, "1234567890123", "", 0, 1, 1, 135, 270, 2, "" },
        /* 67*/ { BARCODE_DPLEIT, COMPLIANT_HEIGHT, 1, "1234567890123", "", 0, 1, 1, 135, 270, 2, "TODO: Find doc" },
        /* 68*/ { BARCODE_DPLEIT, -1, 4, "1234567890123", "", 0, 4, 1, 135, 270, 8, "" },
        /* 69*/ { BARCODE_DPIDENT, -1, 1, "12345678901", "", 0, 1, 1, 117, 234, 2, "" },
        /* 70*/ { BARCODE_DPIDENT, COMPLIANT_HEIGHT, 1, "12345678901", "", 0, 1, 1, 117, 234, 2, "TODO: Find doc" },
        /* 71*/ { BARCODE_DPIDENT, -1, 4, "12345678901", "", 0, 4, 1, 117, 234, 8, "" },
        /* 72*/ { BARCODE_CODE16K, -1, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "2 rows" },
        /* 73*/ { BARCODE_CODE16K, -1, 1, "1234567890", "", 0, 1, 2, 70, 162, 6, "" },
        /* 74*/ { BARCODE_CODE16K, -1, 4, "1234567890", "", 0, 4, 2, 70, 162, 12, "" },
        /* 75*/ { BARCODE_CODE16K, -1, 16.75, "1234567890", "", 0, 16.75, 2, 70, 162, 37.5, "" },
        /* 76*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 16.75, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16.75, 2, 70, 162, 37.5, "" },
        /* 77*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 17, "1234567890", "", 0, 17, 2, 70, 162, 38, "" },
        /* 78*/ { BARCODE_CODE16K, -1, -1, "12345678901234567890123456789012345678901234567890", "", 0, 60, 6, 70, 162, 124, "6 rows" },
        /* 79*/ { BARCODE_CODE16K, -1, 52.5, "12345678901234567890123456789012345678901234567890", "", 0, 52.5, 6, 70, 162, 109, "" },
        /* 80*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 52.5, "12345678901234567890123456789012345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 52.5, 6, 70, 162, 109, "" },
        /* 81*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 53, "12345678901234567890123456789012345678901234567890", "", 0, 53, 6, 70, 162, 110, "" },
        /* 82*/ { BARCODE_CODE49, -1, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "2 rows" },
        /* 83*/ { BARCODE_CODE49, -1, 1, "1234567890", "", 0, 1, 2, 70, 162, 6, "" },
        /* 84*/ { BARCODE_CODE49, -1, 16.75, "1234567890", "", 0, 16.75, 2, 70, 162, 37.5, "" },
        /* 85*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 16.75, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16.75, 2, 70, 162, 37.5, "" },
        /* 86*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 17, "1234567890", "", 0, 17, 2, 70, 162, 38, "" },
        /* 87*/ { BARCODE_CODE49, -1, -1, "12345678901234567890", "", 0, 30, 3, 70, 162, 64, "3 rows" },
        /* 88*/ { BARCODE_CODE49, -1, 25.75, "12345678901234567890", "", 0, 25.75, 3, 70, 162, 55.5, "" },
        /* 89*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 25.75, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 25.75, 3, 70, 162, 55.5, "" },
        /* 90*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 26, "12345678901234567890", "", 0, 26, 3, 70, 162, 56, "" },
        /* 91*/ { BARCODE_CODE93, -1, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "" },
        /* 92*/ { BARCODE_CODE93, -1, 9.9, "1", "", 0, 9.8999996, 1, 46, 92, 19.799999, "" },
        /* 93*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 9.89, "1", "", ZINT_WARN_NONCOMPLIANT, 9.89000034, 1, 46, 92, 19.7800007, "Min height data-length dependent" },
        /* 94*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 10, "1", "", 0, 10, 1, 46, 92, 20, "" },
        /* 95*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 22, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 22, 1, 127, 254, 44, "Min height data-length dependent" },
        /* 96*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 22.1, "1234567890", "", 0, 22.1, 1, 127, 254, 44.200001, "" },
        /* 97*/ { BARCODE_FLAT, -1, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "" },
        /* 98*/ { BARCODE_FLAT, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "TODO: Find doc" },
        /* 99*/ { BARCODE_FLAT, -1, 4, "1234567890", "", 0, 4, 1, 90, 180, 8, "" },
        /*100*/ { BARCODE_DBAR_OMN, -1, 1, "1234567890123", "", 0, 1, 1, 96, 192, 2, "" },
        /*101*/ { BARCODE_DBAR_OMN, -1, 12.9, "1234567890123", "", 0, 12.9, 1, 96, 192, 25.799999, "" },
        /*102*/ { BARCODE_DBAR_OMN, COMPLIANT_HEIGHT, 12.9, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 12.9, 1, 96, 192, 25.799999, "" },
        /*103*/ { BARCODE_DBAR_OMN, COMPLIANT_HEIGHT, 13, "1234567890123", "", 0, 13, 1, 96, 192, 26, "" },
        /*104*/ { BARCODE_DBAR_LTD, -1, 1, "1234567890123", "", 0, 1, 1, 79, 158, 2, "" },
        /*105*/ { BARCODE_DBAR_LTD, -1, 9, "1234567890123", "", 0, 9, 1, 79, 158, 18, "" },
        /*106*/ { BARCODE_DBAR_LTD, COMPLIANT_HEIGHT, 9, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 9, 1, 79, 158, 18, "" },
        /*107*/ { BARCODE_DBAR_LTD, COMPLIANT_HEIGHT, 10, "1234567890123", "", 0, 10, 1, 79, 158, 20, "" },
        /*108*/ { BARCODE_DBAR_EXP, -1, 1, "[01]12345678901231", "", 0, 1, 1, 134, 268, 2, "" },
        /*109*/ { BARCODE_DBAR_EXP, -1, 33, "[01]12345678901231", "", 0, 33, 1, 134, 268, 66, "" },
        /*110*/ { BARCODE_DBAR_EXP, COMPLIANT_HEIGHT, 33, "[01]12345678901231", "", ZINT_WARN_NONCOMPLIANT, 33, 1, 134, 268, 66, "" },
        /*111*/ { BARCODE_DBAR_EXP, COMPLIANT_HEIGHT, 34, "[01]12345678901231", "", 0, 34, 1, 134, 268, 68, "" },
        /*112*/ { BARCODE_TELEPEN, -1, 1, "1234567890", "", 0, 1, 1, 208, 416, 2, "" },
        /*113*/ { BARCODE_TELEPEN, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 208, 416, 2, "No known min" },
        /*114*/ { BARCODE_TELEPEN, -1, 4, "1234567890", "", 0, 4, 1, 208, 416, 8, "" },
        /*115*/ { BARCODE_UPCA, -1, 1, "12345678901", "", 0, 1, 1, 95, 226, 12, "" },
        /*116*/ { BARCODE_UPCA, -1, 69, "12345678901", "", 0, 69, 1, 95, 226, 148, "" },
        /*117*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, 69, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /*118*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, 69.25, "12345678901", "", 0, 69.25, 1, 95, 226, 148.5, "" },
        /*119*/ { BARCODE_UPCA_CHK, -1, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "" },
        /*120*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, 69, "123456789012", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /*121*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, 69.5, "123456789012", "", 0, 69.5, 1, 95, 226, 149, "" },
        /*122*/ { BARCODE_UPCE, -1, 1, "1234567", "", 0, 1, 1, 51, 134, 12, "" },
        /*123*/ { BARCODE_UPCE, -1, 69, "1234567", "", 0, 69, 1, 51, 134, 148, "" },
        /*124*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, 69, "1234567", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 51, 134, 148, "" },
        /*125*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, 69.25, "1234567", "", 0, 69.25, 1, 51, 134, 148.5, "" },
        /*126*/ { BARCODE_UPCE_CHK, -1, 1, "12345670", "", 0, 1, 1, 51, 134, 12, "" },
        /*127*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, 69.24, "12345670", "", ZINT_WARN_NONCOMPLIANT, 69.239998, 1, 51, 134, 148.48, "" },
        /*128*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, 69.25, "12345670", "", 0, 69.25, 1, 51, 134, 148.5, "" },
        /*129*/ { BARCODE_POSTNET, -1, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, "" },
        /*130*/ { BARCODE_POSTNET, -1, 1, "12345678901", "", 0, 1, 2, 123, 246, 2, "" },
        /*131*/ { BARCODE_POSTNET, -1, 4.5, "12345678901", "", 0, 4.5, 2, 123, 246, 9, "" },
        /*132*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 4.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 4.5, 2, 123, 246, 9, "" },
        /*133*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 4.6, "12345678901", "", 0, 4.5999999, 2, 123, 246, 9.1999998, "" },
        /*134*/ { BARCODE_POSTNET, -1, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*135*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*136*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 9.1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 9.1000004, 2, 123, 246, 18.200001, "" },
        /*137*/ { BARCODE_POSTNET, -1, 20, "12345678901", "", 0, 20, 2, 123, 246, 40, "" },
        /*138*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 20, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 20, 2, 123, 246, 40, "" },
        /*139*/ { BARCODE_MSI_PLESSEY, -1, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "" },
        /*140*/ { BARCODE_MSI_PLESSEY, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "TODO: Find doc" },
        /*141*/ { BARCODE_MSI_PLESSEY, -1, 4, "1234567890", "", 0, 4, 1, 127, 254, 8, "" },
        /*142*/ { BARCODE_FIM, -1, 1, "A", "", 0, 1, 1, 17, 34, 2, "" },
        /*143*/ { BARCODE_FIM, -1, 12.7, "A", "", 0, 12.7, 1, 17, 34, 25.4, "" },
        /*144*/ { BARCODE_FIM, COMPLIANT_HEIGHT, 12.7, "A", "", ZINT_WARN_NONCOMPLIANT, 12.7, 1, 17, 34, 25.4, "" },
        /*145*/ { BARCODE_FIM, COMPLIANT_HEIGHT, 12.8, "A", "", 0, 12.8, 1, 17, 34, 25.6, "" },
        /*146*/ { BARCODE_LOGMARS, -1, 1, "1234567890", "", 0, 1, 1, 191, 382, 2, "" },
        /*147*/ { BARCODE_LOGMARS, -1, 6, "1234567890", "", 0, 6, 1, 191, 382, 12, "" },
        /*148*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 6, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 6, 1, 191, 382, 12, "" },
        /*149*/ { BARCODE_LOGMARS, -1, 6.25, "1234567890", "", 0, 6.25, 1, 191, 382, 12.5, "" },
        /*150*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 6.25, "1234567890", "", 0, 6.25, 1, 191, 382, 12.5, "" },
        /*151*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 116, "1234567890", "", 0, 116, 1, 191, 382, 232, "" },
        /*152*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 117, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 117, 1, 191, 382, 234, "" },
        /*153*/ { BARCODE_PHARMA, -1, 1, "123456", "", 0, 1, 1, 58, 116, 2, "" },
        /*154*/ { BARCODE_PHARMA, -1, 15, "123456", "", 0, 15, 1, 58, 116, 30, "" },
        /*155*/ { BARCODE_PHARMA, COMPLIANT_HEIGHT, 15, "123456", "", ZINT_WARN_NONCOMPLIANT, 15, 1, 58, 116, 30, "" },
        /*156*/ { BARCODE_PHARMA, COMPLIANT_HEIGHT, 16, "123456", "", 0, 16, 1, 58, 116, 32, "" },
        /*157*/ { BARCODE_PZN, -1, 1, "123456", "", 0, 1, 1, 142, 284, 2, "" },
        /*158*/ { BARCODE_PZN, -1, 17.7, "123456", "", 0, 17.700001, 1, 142, 284, 35.400002, "" },
        /*159*/ { BARCODE_PZN, COMPLIANT_HEIGHT, 17.7, "123456", "", ZINT_WARN_NONCOMPLIANT, 17.700001, 1, 142, 284, 35.400002, "" },
        /*160*/ { BARCODE_PZN, COMPLIANT_HEIGHT, 17.8, "123456", "", 0, 17.799999, 1, 142, 284, 35.599998, "" },
        /*161*/ { BARCODE_PHARMA_TWO, -1, -1, "12345678", "", 0, 10, 2, 29, 58, 20, "" },
        /*162*/ { BARCODE_PHARMA_TWO, -1, 1, "12345678", "", 0, 1, 2, 29, 58, 2, "" },
        /*163*/ { BARCODE_PHARMA_TWO, -1, 3.9, "12345678", "", 0, 3.9000001, 2, 29, 58, 7.8000002, "" },
        /*164*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 3.9, "12345678", "", ZINT_WARN_NONCOMPLIANT, 3.9000001, 2, 29, 58, 7.8000002, "" },
        /*165*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 4, "12345678", "", 0, 4, 2, 29, 58, 8, "" },
        /*166*/ { BARCODE_PHARMA_TWO, -1, 15, "12345678", "", 0, 15, 2, 29, 58, 30, "" },
        /*167*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 15, "12345678", "", 0, 15, 2, 29, 58, 30, "" },
        /*168*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 15.1, "12345678", "", ZINT_WARN_NONCOMPLIANT, 15.1, 2, 29, 58, 30.200001, "" },
        /*169*/ { BARCODE_CEPNET, -1, -1, "12345678", "", 0, 5.375, 2, 93, 186, 10.75, "" },
        /*170*/ { BARCODE_CEPNET, -1, 1, "12345678", "", 0, 1.25, 2, 93, 186, 2.5, "" },
        /*171*/ { BARCODE_CEPNET, -1, 4.5, "12345678", "", 0, 4.5, 2, 93, 186, 9, "" },
        /*172*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 4.5, "12345678", "", ZINT_WARN_NONCOMPLIANT, 4.5, 2, 93, 186, 9, "" },
        /*173*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 4.6, "12345678", "", 0, 4.5999999, 2, 93, 186, 9.1999998, "" },
        /*174*/ { BARCODE_CEPNET, -1, 9, "12345678", "", 0, 9, 2, 93, 186, 18, "" },
        /*175*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 9, "12345678", "", 0, 9, 2, 93, 186, 18, "" },
        /*176*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 9.1, "12345678", "", ZINT_WARN_NONCOMPLIANT, 9.1000004, 2, 93, 186, 18.200001, "" },
        /*177*/ { BARCODE_CEPNET, -1, 20, "12345678", "", 0, 20, 2, 93, 186, 40, "" },
        /*178*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 20, "12345678", "", ZINT_WARN_NONCOMPLIANT, 20, 2, 93, 186, 40, "" },
        /*179*/ { BARCODE_PDF417, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 4, 8, 120, 240, 8, "8 rows" },
        /*180*/ { BARCODE_PDF417, -1, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 4, 8, 120, 240, 8, "" },
        /*181*/ { BARCODE_PDF417, -1, 9, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 9, 8, 120, 240, 18, "" },
        /*182*/ { BARCODE_PDF417, -1, 10, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 10, 8, 120, 240, 20, "" },
        /*183*/ { BARCODE_PDF417, -1, 11, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 11, 8, 120, 240, 22, "" },
        /*184*/ { BARCODE_PDF417, -1, 12, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 12, 8, 120, 240, 24, "" },
        /*185*/ { BARCODE_PDF417, -1, 13, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 13, 8, 120, 240, 26, "" },
        /*186*/ { BARCODE_PDF417, -1, 14, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 14, 8, 120, 240, 28, "" },
        /*187*/ { BARCODE_PDF417, -1, 15, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 15, 8, 120, 240, 30, "" },
        /*188*/ { BARCODE_PDF417, -1, 16, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 16, 8, 120, 240, 32, "" },
        /*189*/ { BARCODE_PDF417, -1, 17, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 17, 8, 120, 240, 34, "" },
        /*190*/ { BARCODE_PDF417, -1, 18, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 18, 8, 120, 240, 36, "" },
        /*191*/ { BARCODE_PDF417, -1, 19, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 19, 8, 120, 240, 38, "" },
        /*192*/ { BARCODE_PDF417, -1, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 8, 120, 240, 40, "" },
        /*193*/ { BARCODE_PDF417, -1, 21, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 21, 8, 120, 240, 42, "" },
        /*194*/ { BARCODE_PDF417, -1, 22, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 22, 8, 120, 240, 44, "" },
        /*195*/ { BARCODE_PDF417, -1, 23, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 23, 8, 120, 240, 46, "" },
        /*196*/ { BARCODE_PDF417, -1, 24, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 24, 8, 120, 240, 48, "" },
        /*197*/ { BARCODE_PDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 36, 12, 120, 240, 72, "12 rows" },
        /*198*/ { BARCODE_PDF417, -1, 12, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 12, 12, 120, 240, 24, "" },
        /*199*/ { BARCODE_PDF417COMP, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJ", "", ZINT_WARN_NONCOMPLIANT, 4.5, 9, 86, 172, 9, "9 rows" },
        /*200*/ { BARCODE_PDF417COMP, -1, 24, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJ", "", ZINT_WARN_NONCOMPLIANT, 24, 9, 86, 172, 48, "" },
        /*201*/ { BARCODE_MAXICODE, -1, 1, "1234567890", "", 0, 16.5, 33, 30, 60, 57.733398, "Fixed size, symbol->height ignored" },
        /*202*/ { BARCODE_QRCODE, -1, 1, "ABCD", "", 0, 21, 21, 21, 42, 42, "Fixed width-to-height ratio, symbol->height ignored" },
        /*203*/ { BARCODE_CODE128AB, -1, 1, "1234567890", "", 0, 1, 1, 145, 290, 2, "" },
        /*204*/ { BARCODE_CODE128AB, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 145, 290, 2, "" },
        /*205*/ { BARCODE_CODE128AB, -1, 4, "1234567890", "", 0, 4, 1, 145, 290, 8, "" },
        /*206*/ { BARCODE_AUSPOST, -1, -1, "12345678901234567890123", "", 0, 8, 3, 133, 266, 16, "" },
        /*207*/ { BARCODE_AUSPOST, -1, 1, "12345678901234567890123", "", 0, 2, 3, 133, 266, 4, "" },
        /*208*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 1, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 1.9230771, 3, 133, 266, 3.8461542, "" },
        /*209*/ { BARCODE_AUSPOST, -1, 6.9, "12345678901234567890123", "", 0, 6.9000001, 3, 133, 266, 13.8, "" },
        /*210*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 6.9, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 6.9000001, 3, 133, 266, 13.8, "" },
        /*211*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 7, "12345678901234567890123", "", 0, 7, 3, 133, 266, 14, "" },
        /*212*/ { BARCODE_AUSPOST, -1, 14, "12345678901234567890123", "", 0, 14, 3, 133, 266, 28, "" },
        /*213*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 14, "12345678901234567890123", "", 0, 14, 3, 133, 266, 28, "" },
        /*214*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 14.1, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 14.099999, 3, 133, 266, 28.199999, "" },
        /*215*/ { BARCODE_AUSREPLY, -1, 14, "12345678", "", 0, 14, 3, 73, 146, 28, "" },
        /*216*/ { BARCODE_AUSREPLY, COMPLIANT_HEIGHT, 14, "12345678", "", 0, 14, 3, 73, 146, 28, "" },
        /*217*/ { BARCODE_AUSREPLY, COMPLIANT_HEIGHT, 14.25, "12345678", "", ZINT_WARN_NONCOMPLIANT, 14.25, 3, 73, 146, 28.5, "" },
        /*218*/ { BARCODE_AUSROUTE, -1, 7, "12345678", "", 0, 7, 3, 73, 146, 14, "" },
        /*219*/ { BARCODE_AUSROUTE, COMPLIANT_HEIGHT, 7, "12345678", "", 0, 7, 3, 73, 146, 14, "" },
        /*220*/ { BARCODE_AUSREDIRECT, -1, 14, "12345678", "", 0, 14, 3, 73, 146, 28, "" },
        /*221*/ { BARCODE_AUSREDIRECT, COMPLIANT_HEIGHT, 14, "12345678", "", 0, 14, 3, 73, 146, 28, "" },
        /*222*/ { BARCODE_ISBNX, -1, 1, "123456789", "", 0, 1, 1, 95, 226, 12, "" },
        /*223*/ { BARCODE_ISBNX, -1, 69, "123456789", "", 0, 69, 1, 95, 226, 148, "" },
        /*224*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, 69, "123456789", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /*225*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, 69.5, "123456789", "", 0, 69.5, 1, 95, 226, 149, "" },
        /*226*/ { BARCODE_RM4SCC, -1, -1, "1234567890", "", 0, 8, 3, 91, 182, 16, "" },
        /*227*/ { BARCODE_RM4SCC, -1, 1, "1234567890", "", 0, 2, 3, 91, 182, 4, "" },
        /*228*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.9615386, 3, 91, 182, 3.9230771, "" },
        /*229*/ { BARCODE_RM4SCC, -1, 4, "1234567890", "", 0, 4, 3, 91, 182, 8, "" },
        /*230*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4, 3, 91, 182, 8, "" },
        /*231*/ { BARCODE_RM4SCC, -1, 6, "1234567890", "", 0, 6, 3, 91, 182, 12, "" },
        /*232*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 6, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 6, 3, 91, 182, 12, "" },
        /*233*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 6.5, "1234567890", "", 0, 6.5, 3, 91, 182, 13, "" },
        /*234*/ { BARCODE_RM4SCC, -1, 10.8, "1234567890", "", 0, 10.8, 3, 91, 182, 21.6, "" },
        /*235*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 10.8, "1234567890", "", 0, 10.8, 3, 91, 182, 21.6, "" },
        /*236*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 11, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 11, 3, 91, 182, 22, "" },
        /*237*/ { BARCODE_RM4SCC, -1, 16, "1234567890", "", 0, 16, 3, 91, 182, 32, "" },
        /*238*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16, 3, 91, 182, 32, "" },
        /*239*/ { BARCODE_DATAMATRIX, -1, 1, "ABCD", "", 0, 12, 12, 12, 24, 24, "Fixed width-to-height ratio, symbol->height ignored" },
        /*240*/ { BARCODE_EAN14, -1, 1, "1234567890123", "", 0, 1, 1, 134, 268, 2, "" },
        /*241*/ { BARCODE_EAN14, -1, 5.7, "1234567890123", "", 0, 5.6999998, 1, 134, 268, 11.4, "" },
        /*242*/ { BARCODE_EAN14, COMPLIANT_HEIGHT, 5.7, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 5.6999998, 1, 134, 268, 11.4, "" },
        /*243*/ { BARCODE_EAN14, COMPLIANT_HEIGHT, 5.8, "1234567890123", "", 0, 5.8000002, 1, 134, 268, 11.6, "" },
        /*244*/ { BARCODE_VIN, -1, 1, "12345678701234567", "", 0, 1, 1, 246, 492, 2, "" },
        /*245*/ { BARCODE_VIN, COMPLIANT_HEIGHT, 1, "12345678701234567", "", 0, 1, 1, 246, 492, 2, "Doc unlikely" },
        /*246*/ { BARCODE_VIN, -1, 4, "12345678701234567", "", 0, 4, 1, 246, 492, 8, "" },
        /*247*/ { BARCODE_CODABLOCKF, -1, -1, "1234567890", "", 0, 20, 2, 101, 242, 44, "2 rows" },
        /*248*/ { BARCODE_CODABLOCKF, -1, 1, "1234567890", "", 0, 1, 2, 101, 242, 6, "" },
        /*249*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1, 2, 101, 242, 6, "" },
        /*250*/ { BARCODE_CODABLOCKF, -1, 15.5, "1234567890", "", 0, 15.5, 2, 101, 242, 35, "" },
        /*251*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 15.5, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 15.5, 2, 101, 242, 35, "" },
        /*252*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 16, "1234567890", "", 0, 16, 2, 101, 242, 36, "" },
        /*253*/ { BARCODE_CODABLOCKF, -1, -1, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 110, 11, 156, 352, 224, "11 rows, 14 cols" },
        /*254*/ { BARCODE_CODABLOCKF, -1, 87, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 87, 11, 156, 352, 178, "" },
        /*255*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 87, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", ZINT_WARN_NONCOMPLIANT, 87, 11, 156, 352, 178, "" },
        /*256*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 88, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 88, 11, 156, 352, 180, "" },
        /*257*/ { BARCODE_CODABLOCKF, -1, 220, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 220, 11, 156, 352, 444, "" },
        /*258*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 220, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 220, 11, 156, 352, 444, "" },
        /*259*/ { BARCODE_CODABLOCKF, -1, -1, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 110, 11, 167, 374, 224, "11 rows, 15 cols" },
        /*260*/ { BARCODE_CODABLOCKF, -1, 93, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 92.999992, 11, 167, 374, 189.99998, "" },
        /*261*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 93, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", ZINT_WARN_NONCOMPLIANT, 92.999992, 11, 167, 374, 189.99998, "" },
        /*262*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 93.5, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 93.5, 11, 167, 374, 191, "" },
        /*263*/ { BARCODE_NVE18, -1, 1, "12345678901234567", "", 0, 1, 1, 156, 312, 2, "" },
        /*264*/ { BARCODE_NVE18, -1, 5.7, "12345678901234567", "", 0, 5.6999998, 1, 156, 312, 11.4, "" },
        /*265*/ { BARCODE_NVE18, COMPLIANT_HEIGHT, 5.7, "12345678901234567", "", ZINT_WARN_NONCOMPLIANT, 5.6999998, 1, 156, 312, 11.4, "" },
        /*266*/ { BARCODE_NVE18, COMPLIANT_HEIGHT, 5.8, "12345678901234567", "", 0, 5.8000002, 1, 156, 312, 11.6, "" },
        /*267*/ { BARCODE_JAPANPOST, -1, -1, "1234567890", "", 0, 8, 3, 133, 266, 16, "" },
        /*268*/ { BARCODE_JAPANPOST, -1, 1, "1234567890", "", 0, 2, 3, 133, 266, 4, "" },
        /*269*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 133, 266, 3, "" },
        /*270*/ { BARCODE_JAPANPOST, -1, 4.8, "1234567890", "", 0, 4.8000002, 3, 133, 266, 9.6000004, "" },
        /*271*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 4.8, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4.8000002, 3, 133, 266, 9.6000004, "" },
        /*272*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 4.9, "1234567890", "", 0, 4.9000001, 3, 133, 266, 9.8000002, "" },
        /*273*/ { BARCODE_JAPANPOST, -1, 7, "1234567890", "", 0, 7, 3, 133, 266, 14, "" },
        /*274*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 7, "1234567890", "", 0, 7, 3, 133, 266, 14, "" },
        /*275*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 7.5, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 7.5, 3, 133, 266, 15, "" },
        /*276*/ { BARCODE_JAPANPOST, -1, 16, "1234567890", "", 0, 16, 3, 133, 266, 32, "" },
        /*277*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 15.999999, 3, 133, 266, 31.999998, "" },
        /*278*/ { BARCODE_KOREAPOST, -1, 1, "123456", "", 0, 1, 1, 167, 334, 2, "" },
        /*279*/ { BARCODE_KOREAPOST, COMPLIANT_HEIGHT, 1, "123456", "", 0, 1, 1, 167, 334, 2, "TODO: Find doc" },
        /*280*/ { BARCODE_KOREAPOST, -1, 4, "123456", "", 0, 4, 1, 167, 334, 8, "" },
        /*281*/ { BARCODE_DBAR_STK, -1, -1, "1234567890123", "", 0, 13, 3, 50, 100, 26, "" },
        /*282*/ { BARCODE_DBAR_STK, -1, 1, "1234567890123", "", 0, 2.2, 3, 50, 100, 4.4000001, "" },
        /*283*/ { BARCODE_DBAR_STK, -1, 12, "1234567890123", "", 0, 12, 3, 50, 100, 24, "" },
        /*284*/ { BARCODE_DBAR_STK, COMPLIANT_HEIGHT, 12, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 12, 3, 50, 100, 24, "" },
        /*285*/ { BARCODE_DBAR_STK, COMPLIANT_HEIGHT, 13, "1234567890123", "", 0, 13, 3, 50, 100, 26, "" },
        /*286*/ { BARCODE_DBAR_OMNSTK, -1, -1, "1234567890123", "", 0, 69, 5, 50, 100, 138, "" },
        /*287*/ { BARCODE_DBAR_OMNSTK, -1, 1, "1234567890123", "", 0, 4, 5, 50, 100, 8, "" },
        /*288*/ { BARCODE_DBAR_OMNSTK, -1, 68, "1234567890123", "", 0, 68, 5, 50, 100, 136, "" },
        /*289*/ { BARCODE_DBAR_OMNSTK, COMPLIANT_HEIGHT, 68, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 68, 5, 50, 100, 136, "" },
        /*290*/ { BARCODE_DBAR_OMNSTK, COMPLIANT_HEIGHT, 69, "1234567890123", "", 0, 69, 5, 50, 100, 138, "" },
        /*291*/ { BARCODE_DBAR_EXPSTK, -1, -1, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142, "2 rows" },
        /*292*/ { BARCODE_DBAR_EXPSTK, -1, 1, "[01]12345678901231", "", 0, 4, 5, 102, 204, 8, "" },
        /*293*/ { BARCODE_DBAR_EXPSTK, -1, 70, "[01]12345678901231", "", 0, 70, 5, 102, 204, 140, "" },
        /*294*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 70, "[01]12345678901231", "", ZINT_WARN_NONCOMPLIANT, 70, 5, 102, 204, 140, "" },
        /*295*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 71, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142, "" },
        /*296*/ { BARCODE_DBAR_EXPSTK, -1, -1, "[01]09501101530003[3920]123456789012345", "", 0, 108, 9, 102, 204, 216, "3 rows" },
        /*297*/ { BARCODE_DBAR_EXPSTK, -1, 1, "[01]09501101530003[3920]123456789012345", "", 0, 7.5, 9, 102, 204, 15, "" },
        /*298*/ { BARCODE_DBAR_EXPSTK, -1, 107.5, "[01]09501101530003[3920]123456789012345", "", 0, 107.5, 9, 102, 204, 215, "" },
        /*299*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 107.5, "[01]09501101530003[3920]123456789012345", "", ZINT_WARN_NONCOMPLIANT, 107.5, 9, 102, 204, 215, "" },
        /*300*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 108, "[01]09501101530003[3920]123456789012345", "", 0, 108, 9, 102, 204, 216, "" },
        /*301*/ { BARCODE_PLANET, -1, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, "" },
        /*302*/ { BARCODE_PLANET, -1, 1, "12345678901", "", 0, 1, 2, 123, 246, 2, "" },
        /*303*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 1.25, 2, 123, 246, 2.5, "" },
        /*304*/ { BARCODE_PLANET, -1, 4.5, "12345678901", "", 0, 4.5, 2, 123, 246, 9, "" },
        /*305*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 4.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 4.5, 2, 123, 246, 9, "" },
        /*306*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 4.6, "12345678901", "", 0, 4.5999999, 2, 123, 246, 9.1999998, "" },
        /*307*/ { BARCODE_PLANET, -1, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*308*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*309*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 9.1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 9.1000004, 2, 123, 246, 18.200001, "" },
        /*310*/ { BARCODE_PLANET, -1, 24, "12345678901", "", 0, 24, 2, 123, 246, 48, "" },
        /*311*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 24, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 24, 2, 123, 246, 48, "" },
        /*312*/ { BARCODE_MICROPDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 20, 10, 82, 164, 40, "10 rows" },
        /*313*/ { BARCODE_MICROPDF417, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 5, 10, 82, 164, 10, "" },
        /*314*/ { BARCODE_MICROPDF417, -1, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 5, 10, 82, 164, 10, "" },
        /*315*/ { BARCODE_MICROPDF417, -1, 19, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 19, 10, 82, 164, 38, "" },
        /*316*/ { BARCODE_MICROPDF417, -1, 30, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 30, 10, 82, 164, 60, "" },
        /*317*/ { BARCODE_MICROPDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 40, 20, 55, 110, 80, "20 rows" },
        /*318*/ { BARCODE_MICROPDF417, -1, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 20, 55, 110, 40, "" },
        /*319*/ { BARCODE_USPS_IMAIL, -1, -1, "12345678901234567890", "", 0, 8, 3, 129, 258, 16, "" },
        /*320*/ { BARCODE_USPS_IMAIL, -1, 1, "12345678901234567890", "", 0, 2, 3, 129, 258, 4, "" },
        /*321*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 1, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5104165, 3, 129, 258, 3.020833, "" },
        /*322*/ { BARCODE_USPS_IMAIL, -1, 4.8, "12345678901234567890", "", 0, 4.8000002, 3, 129, 258, 9.6000004, "" },
        /*323*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 4.8, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 4.8000002, 3, 129, 258, 9.6000004, "" },
        /*324*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 4.9, "12345678901234567890", "", 0, 4.9000001, 3, 129, 258, 9.8000002, "" },
        /*325*/ { BARCODE_USPS_IMAIL, -1, 7.7, "12345678901234567890", "", 0, 7.6999998, 3, 129, 258, 15.4, "" },
        /*326*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 7.7, "12345678901234567890", "", 0, 7.69999981, 3, 129, 258, 15.3999996, "" },
        /*327*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 7.8, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 7.80000019, 3, 129, 258, 15.6000004, "" },
        /*328*/ { BARCODE_PLESSEY, -1, 1, "1234567890", "", 0, 1, 1, 227, 454, 2, "" },
        /*329*/ { BARCODE_PLESSEY, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 227, 454, 2, "TODO: Find doc" },
        /*330*/ { BARCODE_PLESSEY, -1, 4, "1234567890", "", 0, 4, 1, 227, 454, 8, "" },
        /*331*/ { BARCODE_TELEPEN_NUM, -1, 1, "1234567890", "", 0, 1, 1, 128, 256, 2, "" },
        /*332*/ { BARCODE_TELEPEN_NUM, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 128, 256, 2, "No known min" },
        /*333*/ { BARCODE_TELEPEN_NUM, -1, 4, "1234567890", "", 0, 4, 1, 128, 256, 8, "" },
        /*334*/ { BARCODE_ITF14, -1, 1, "1234567890", "", 0, 1, 1, 135, 330, 22, "" },
        /*335*/ { BARCODE_ITF14, -1, 5.7, "1234567890", "", 0, 5.6999998, 1, 135, 330, 31.4, "" },
        /*336*/ { BARCODE_ITF14, COMPLIANT_HEIGHT, 5.7, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 5.6999998, 1, 135, 330, 31.4, "" },
        /*337*/ { BARCODE_ITF14, COMPLIANT_HEIGHT, 5.8, "1234567890", "", 0, 5.8000002, 1, 135, 330, 31.6, "" },
        /*338*/ { BARCODE_KIX, -1, -1, "1234567890", "", 0, 8, 3, 79, 158, 16, "" },
        /*339*/ { BARCODE_KIX, -1, 1, "1234567890", "", 0, 2, 3, 79, 158, 4, "" },
        /*340*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.9615386, 3, 79, 158, 3.9230771, "" },
        /*341*/ { BARCODE_KIX, -1, 6.4, "1234567890", "", 0, 6.4000001, 3, 79, 158, 12.8, "" },
        /*342*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 6.4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 6.3999996, 3, 79, 158, 12.799999, "" },
        /*343*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 6.5, "1234567890", "", 0, 6.5, 3, 79, 158, 13, "" },
        /*344*/ { BARCODE_KIX, -1, 10.8, "1234567890", "", 0, 10.8, 3, 79, 158, 21.6, "" },
        /*345*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 10.8, "1234567890", "", 0, 10.8, 3, 79, 158, 21.6, "" },
        /*346*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 10.9, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 10.9, 3, 79, 158, 21.799999, "" },
        /*347*/ { BARCODE_KIX, -1, 16, "1234567890", "", 0, 16, 3, 79, 158, 32, "" },
        /*348*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16, 3, 79, 158, 32, "" },
        /*349*/ { BARCODE_AZTEC, -1, 1, "1234567890AB", "", 0, 15, 15, 15, 30, 30, "Fixed width-to-height ratio, symbol->height ignored" },
        /*350*/ { BARCODE_DAFT, -1, -1, "DAFTDAFTDAFTDAFT", "", 0, 8, 3, 31, 62, 16, "" },
        /*351*/ { BARCODE_DAFT, -1, 1, "DAFTDAFTDAFTDAFT", "", 0, 2, 3, 31, 62, 4, "" },
        /*352*/ { BARCODE_DAFT, COMPLIANT_HEIGHT, 1, "DAFTDAFTDAFTDAFT", "", 0, 2, 3, 31, 62, 4, "" },
        /*353*/ { BARCODE_DAFT, -1, 4, "DAFTDAFTDAFTDAFT", "", 0, 4, 3, 31, 62, 8, "" },
        /*354*/ { BARCODE_DAFT, -1, 6, "DAFTDAFTDAFTDAFT", "", 0, 6, 3, 31, 62, 12, "" },
        /*355*/ { BARCODE_DAFT, -1, 12, "DAFTDAFTDAFTDAFT", "", 0, 12, 3, 31, 62, 24, "" },
        /*356*/ { BARCODE_DAFT, -1, 16, "DAFTDAFTDAFTDAFT", "", 0, 16, 3, 31, 62, 32, "" },
        /*357*/ { BARCODE_DAFT, COMPLIANT_HEIGHT, 16, "DAFTDAFTDAFTDAFT", "", 0, 16, 3, 31, 62, 32, "" },
        /*358*/ { BARCODE_DPD, -1, 1, "0123456789012345678901234567", "", 0, 1, 1, 189, 378, 8, "" },
        /*359*/ { BARCODE_DPD, -1, 62, "0123456789012345678901234567", "", 0, 62, 1, 189, 378, 130, "" },
        /*360*/ { BARCODE_DPD, COMPLIANT_HEIGHT, 62, "0123456789012345678901234567", "", ZINT_WARN_NONCOMPLIANT, 62, 1, 189, 378, 130, "" },
        /*361*/ { BARCODE_DPD, COMPLIANT_HEIGHT, 62.5, "0123456789012345678901234567", "", 0, 62.5, 1, 189, 378, 131, "" },
        /*362*/ { BARCODE_MICROQR, -1, 1, "12345", "", 0, 11, 11, 11, 22, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*363*/ { BARCODE_HIBC_128, -1, 1, "1234567890", "", 0, 1, 1, 123, 246, 2, "" },
        /*364*/ { BARCODE_HIBC_128, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 123, 246, 2, "" },
        /*365*/ { BARCODE_HIBC_128, -1, 4, "1234567890", "", 0, 4, 1, 123, 246, 8, "" },
        /*366*/ { BARCODE_HIBC_39, -1, 1, "1234567890", "", 0, 1, 1, 223, 446, 2, "" },
        /*367*/ { BARCODE_HIBC_39, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1, 1, 223, 446, 2, "" },
        /*368*/ { BARCODE_HIBC_39, -1, 4, "1234567890", "", 0, 4, 1, 223, 446, 8, "" },
        /*369*/ { BARCODE_HIBC_DM, -1, 1, "ABC", "", 0, 12, 12, 12, 24, 24, "Fixed width-to-height ratio, symbol->height ignored" },
        /*370*/ { BARCODE_HIBC_QR, -1, 1, "1234567890AB", "", 0, 21, 21, 21, 42, 42, "Fixed width-to-height ratio, symbol->height ignored" },
        /*371*/ { BARCODE_HIBC_PDF, -1, -1, "ABCDEF", "", 0, 21, 7, 103, 206, 42, "7 rows" },
        /*372*/ { BARCODE_HIBC_PDF, -1, 1, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3.5, 7, 103, 206, 7, "" },
        /*373*/ { BARCODE_HIBC_PDF, -1, 4, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 4, 7, 103, 206, 8, "" },
        /*374*/ { BARCODE_HIBC_PDF, -1, 36, "ABCDEF", "", 0, 36, 7, 103, 206, 72, "" },
        /*375*/ { BARCODE_HIBC_MICPDF, -1, -1, "ABCDEF", "", 0, 12, 6, 82, 164, 24, "6 rows" },
        /*376*/ { BARCODE_HIBC_MICPDF, -1, 1, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3, 6, 82, 164, 6, "" },
        /*377*/ { BARCODE_HIBC_MICPDF, -1, 4, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 4, 6, 82, 164, 8, "" },
        /*378*/ { BARCODE_HIBC_MICPDF, -1, 47, "ABCDEF", "", 0, 47, 6, 82, 164, 94, "" },
        /*379*/ { BARCODE_HIBC_BLOCKF, -1, -1, "1234567890", "", 0, 30, 3, 101, 242, 64, "3 rows" },
        /*380*/ { BARCODE_HIBC_BLOCKF, -1, 1, "1234567890", "", 0, 1.5, 3, 101, 242, 7, "" },
        /*381*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 101, 242, 7, "" },
        /*382*/ { BARCODE_HIBC_BLOCKF, -1, 4, "1234567890", "", 0, 4, 3, 101, 242, 12, "" },
        /*383*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4, 3, 101, 242, 12, "" },
        /*384*/ { BARCODE_HIBC_BLOCKF, -1, 23, "1234567890", "", 0, 23, 3, 101, 242, 50, "" },
        /*385*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 23, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 23, 3, 101, 242, 50, "" },
        /*386*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 24, "1234567890", "", 0, 24, 3, 101, 242, 52, "" },
        /*387*/ { BARCODE_HIBC_BLOCKF, -1, 60, "1234567890", "", 0, 60, 3, 101, 242, 124, "" },
        /*388*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 60, "1234567890", "", 0, 60, 3, 101, 242, 124, "" },
        /*389*/ { BARCODE_HIBC_AZTEC, -1, 1, "1234567890AB", "", 0, 19, 19, 19, 38, 38, "Fixed width-to-height ratio, symbol->height ignored" },
        /*390*/ { BARCODE_DOTCODE, -1, 1, "ABC", "", 0, 11, 11, 16, 32, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*391*/ { BARCODE_HANXIN, -1, 1, "1234567890AB", "", 0, 23, 23, 23, 46, 46, "Fixed width-to-height ratio, symbol->height ignored" },
        /*392*/ { BARCODE_MAILMARK_2D, -1, 1, "012100123412345678AB19XY1A 0", "", 0, 24, 24, 24, 48, 48, "Fixed width-to-height ratio, symbol->height ignored" },
        /*393*/ { BARCODE_UPU_S10, -1, 1, "EE876543216CA", "", 0, 1, 1, 156, 312, 2, "" },
        /*394*/ { BARCODE_UPU_S10, COMPLIANT_HEIGHT, 1, "EE876543216CA", "", ZINT_WARN_NONCOMPLIANT, 1, 1, 156, 312, 2, "" },
        /*395*/ { BARCODE_UPU_S10, -1, 24.5f, "EE876543216CA", "", 0, 24.5f, 1, 156, 312, 49, "" },
        /*396*/ { BARCODE_UPU_S10, COMPLIANT_HEIGHT, 24.5f, "EE876543216CA", "", ZINT_WARN_NONCOMPLIANT, 24.5f, 1, 156, 312, 49, "" },
        /*397*/ { BARCODE_UPU_S10, -1, 25, "EE876543216CA", "", 0, 25, 1, 156, 312, 50, "" },
        /*398*/ { BARCODE_UPU_S10, COMPLIANT_HEIGHT, 25, "EE876543216CA", "", 0, 25, 1, 156, 312, 50, "" },
        /*399*/ { BARCODE_MAILMARK_4S, -1, -1, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "" },
        /*400*/ { BARCODE_MAILMARK_4S, -1, 1, "01000000000000000AA00AA0A", "", 0, 2.5, 3, 155, 310, 5, "" },
        /*401*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 1, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 1.9615386, 3, 155, 310, 3.9230771, "" },
        /*402*/ { BARCODE_MAILMARK_4S, -1, 6.4, "01000000000000000AA00AA0A", "", 0, 6.4000001, 3, 155, 310, 12.8, "" },
        /*403*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 6.4, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 6.3999996, 3, 155, 310, 12.799999, "" },
        /*404*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 6.5, "01000000000000000AA00AA0A", "", 0, 6.5, 3, 155, 310, 13, "" },
        /*405*/ { BARCODE_MAILMARK_4S, -1, 10, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "" },
        /*406*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 10, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "" },
        /*407*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 11, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 11, 3, 155, 310, 22, "" },
        /*408*/ { BARCODE_MAILMARK_4S, -1, 15, "01000000000000000AA00AA0A", "", 0, 15, 3, 155, 310, 30, "" },
        /*409*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 15, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 15, 3, 155, 310, 30, "" },
        /*410*/ { BARCODE_MAILMARK_4S, -1, 20, "01000000000000000AA00AA0A", "", 0, 20, 3, 155, 310, 40, "" },
        /*411*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 20, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 20, 3, 155, 310, 40, "" },
        /*412*/ { BARCODE_AZRUNE, -1, 1, "1", "", 0, 11, 11, 11, 22, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*413*/ { BARCODE_CODE32, -1, 1, "12345678", "", 0, 1, 1, 103, 206, 2, "" },
        /*414*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 1, "12345678", "", ZINT_WARN_NONCOMPLIANT, 1, 1, 103, 206, 2, "" },
        /*415*/ { BARCODE_CODE32, -1, 19, "12345678", "", 0, 19, 1, 103, 206, 38, "" },
        /*416*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 19, "12345678", "", ZINT_WARN_NONCOMPLIANT, 19, 1, 103, 206, 38, "" },
        /*417*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 20, "12345678", "", 0, 20, 1, 103, 206, 40, "" },
        /*418*/ { BARCODE_EAN13_CC, -1, -1, "123456789012", "[20]01", 0, 50, 7, 99, 226, 110, "EAN-13, CC-A 3 rows" },
        /*419*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01", 0, 50, 7, 99, 226, 110, "EAN-13, CC-A 3 rows" },
        /*420*/ { BARCODE_EAN13_CC, -1, 1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "" },
        /*421*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "" },
        /*422*/ { BARCODE_EAN13_CC, -1, 81, "123456789012", "[20]01", 0, 81, 7, 99, 226, 172, "" },
        /*423*/ { BARCODE_EANX_CC, -1, 81, "123456789012", "[20]01", 0, 81, 7, 99, 226, 172, "" },
        /*424*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 81, "123456789012", "[20]01", ZINT_WARN_NONCOMPLIANT, 81, 7, 99, 226, 172, "" },
        /*425*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 81, "123456789012", "[20]01", ZINT_WARN_NONCOMPLIANT, 81, 7, 99, 226, 172, "" },
        /*426*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 81.25, "123456789012", "[20]01", 0, 81.25, 7, 99, 226, 172.5, "" },
        /*427*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 81.25, "123456789012", "[20]01", 0, 81.25, 7, 99, 226, 172.5, "" },
        /*428*/ { BARCODE_EAN13_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 50, 9, 99, 226, 110, "EAN-13, CC-A 5 rows" },
        /*429*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 50, 9, 99, 226, 110, "EAN-13, CC-A 5 rows" },
        /*430*/ { BARCODE_EAN13_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 16.5, 9, 99, 226, 43, "" },
        /*431*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 16.5, 9, 99, 226, 43, "" },
        /*432*/ { BARCODE_EAN13_CC, -1, 85, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85, 9, 99, 226, 180, "" },
        /*433*/ { BARCODE_EANX_CC, -1, 85, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85, 9, 99, 226, 180, "" },
        /*434*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 85, "123456789012", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 85, 9, 99, 226, 180, "" },
        /*435*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 85, "123456789012", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 85, 9, 99, 226, 180, "" },
        /*436*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 85.25, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85.25, 9, 99, 226, 180.5, "" },
        /*437*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 85.25, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85.25, 9, 99, 226, 180.5, "" },
        /*438*/ { BARCODE_EAN13_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 11, 99, 226, 110, "EAN-13, CC-A 7 rows" },
        /*439*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 11, 99, 226, 110, "EAN-13, CC-A 7 rows" },
        /*440*/ { BARCODE_EAN13_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.5, 11, 99, 226, 51, "" },
        /*441*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.5, 11, 99, 226, 51, "" },
        /*442*/ { BARCODE_EAN13_CC, -1, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89, 11, 99, 226, 188, "" },
        /*443*/ { BARCODE_EANX_CC, -1, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89, 11, 99, 226, 188, "" },
        /*444*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 89, 11, 99, 226, 188, "" },
        /*445*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 89, 11, 99, 226, 188, "" },
        /*446*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 89.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89.25, 11, 99, 226, 188.5, "" },
        /*447*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 89.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89.25, 11, 99, 226, 188.5, "" },
        /*448*/ { BARCODE_EAN13_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 14, 99, 226, 110, "EAN-13, CC-B 10 rows" },
        /*449*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 14, 99, 226, 110, "EAN-13, CC-B 10 rows" },
        /*450*/ { BARCODE_EAN13_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.5, 14, 99, 226, 63, "" },
        /*451*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.5, 14, 99, 226, 63, "" },
        /*452*/ { BARCODE_EAN13_CC, -1, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95, 14, 99, 226, 200, "" },
        /*453*/ { BARCODE_EANX_CC, -1, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95, 14, 99, 226, 200, "" },
        /*454*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 95, 14, 99, 226, 200, "" },
        /*455*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 95, 14, 99, 226, 200, "" },
        /*456*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 95.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95.25, 14, 99, 226, 200.5, "" },
        /*457*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 95.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95.25, 14, 99, 226, 200.5, "" },
        /*458*/ { BARCODE_EAN8_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234", 0, 50, 10, 72, 162, 110, "EAN-8, CC-A 4 rows" },
        /*459*/ { BARCODE_EANX_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234", 0, 50, 10, 72, 162, 110, "EAN-8, CC-A 4 rows" },
        /*460*/ { BARCODE_EAN8_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234", 0, 18.5, 10, 72, 162, 47, "" },
        /*461*/ { BARCODE_EANX_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234", 0, 18.5, 10, 72, 162, 47, "" },
        /*462*/ { BARCODE_EAN8_CC, -1, 73, "1234567", "[20]01[90]123456789012345678901234", 0, 73, 10, 72, 162, 156, "" },
        /*463*/ { BARCODE_EANX_CC, -1, 73, "1234567", "[20]01[90]123456789012345678901234", 0, 73, 10, 72, 162, 156, "" },
        /*464*/ { BARCODE_EAN8_CC, COMPLIANT_HEIGHT, 73, "1234567", "[20]01[90]123456789012345678901234", ZINT_WARN_NONCOMPLIANT, 73, 10, 72, 162, 156, "" },
        /*465*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 73, "1234567", "[20]01[90]123456789012345678901234", ZINT_WARN_NONCOMPLIANT, 73, 10, 72, 162, 156, "" },
        /*466*/ { BARCODE_EAN8_CC, COMPLIANT_HEIGHT, 73.25, "1234567", "[20]01[90]123456789012345678901234", 0, 73.25, 10, 72, 162, 156.5, "" },
        /*467*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 73.25, "1234567", "[20]01[90]123456789012345678901234", 0, 73.25, 10, 72, 162, 156.5, "" },
        /*468*/ { BARCODE_EAN8_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 50, 24, 82, 178, 110, "EAN-8, CC-B 15 rows" },
        /*469*/ { BARCODE_EANX_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 50, 24, 82, 178, 110, "EAN-8, CC-B 15 rows" },
        /*470*/ { BARCODE_EAN8_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 46.5, 24, 82, 178, 103, "" },
        /*471*/ { BARCODE_EANX_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 46.5, 24, 82, 178, 103, "" },
        /*472*/ { BARCODE_EAN8_CC, -1, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101, 24, 82, 178, 212, "" },
        /*473*/ { BARCODE_EANX_CC, -1, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101, 24, 82, 178, 212, "" },
        /*474*/ { BARCODE_EAN8_CC, COMPLIANT_HEIGHT, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 101, 24, 82, 178, 212, "" },
        /*475*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 101, 24, 82, 178, 212, "" },
        /*476*/ { BARCODE_EAN8_CC, COMPLIANT_HEIGHT, 101.25, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101.25, 24, 82, 178, 212.5, "" },
        /*477*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 101.25, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101.25, 24, 82, 178, 212.5, "" },
        /*478*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 50, 5, 145, 290, 100, "CC-A 3 rows" },
        /*479*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 7.5, 5, 145, 290, 15, "" },
        /*480*/ { BARCODE_GS1_128_CC, -1, 12.5, "[01]12345678901231", "[20]01", 0, 12.5, 5, 145, 290, 25, "" },
        /*481*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 12.5, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 12.5, 5, 145, 290, 25, "" },
        /*482*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 12.75, "[01]12345678901231", "[20]01", 0, 12.75, 5, 145, 290, 25.5, "" },
        /*483*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 9, 145, 290, 100, "CC-A 7 rows" },
        /*484*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 15.5, 9, 145, 290, 31, "" },
        /*485*/ { BARCODE_GS1_128_CC, -1, 20.7, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.700001, 9, 145, 290, 41.400002, "" },
        /*486*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 20.7, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 20.700001, 9, 145, 290, 41.400002, "" },
        /*487*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 20.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.75, 9, 145, 290, 41.5, "" },
        /*488*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 12, 145, 290, 100, "CC-B 10 rows" },
        /*489*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 145, 290, 43, "" },
        /*490*/ { BARCODE_GS1_128_CC, -1, 26.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.5, 12, 145, 290, 53, "" },
        /*491*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 26.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 26.5, 12, 145, 290, 53, "" },
        /*492*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 26.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.75, 12, 145, 290, 53.5, "" },
        /*493*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 91.5, 32, 154, 308, 183, "CC-C 30 rows" },
        /*494*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 91.5, 32, 154, 308, 183, "" },
        /*495*/ { BARCODE_GS1_128_CC, -1, 96.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 96.5, 32, 154, 308, 193, "" },
        /*496*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 96.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 96.5, 32, 154, 308, 193, "" },
        /*497*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 96.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 96.75, 32, 154, 308, 193.5, "" },
        /*498*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]1234567890", 0, 21, 5, 100, 200, 42, "CC-A 3 rows" },
        /*499*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]1234567890", 0, 7.5, 5, 100, 200, 15, "" },
        /*500*/ { BARCODE_DBAR_OMN_CC, -1, 19.9, "1234567890123", "[20]01[90]1234567890", 0, 19.9, 5, 100, 200, 39.799999, "" },
        /*501*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 19.9, "1234567890123", "[20]01[90]1234567890", ZINT_WARN_NONCOMPLIANT, 19.9, 5, 100, 200, 39.799999, "" },
        /*502*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 20, "1234567890123", "[20]01[90]1234567890", 0, 20, 5, 100, 200, 40, "" },
        /*503*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]12345678901234567890", 0, 23, 6, 100, 200, 46, "CC-A 4 rows" },
        /*504*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]12345678901234567890", 0, 9.5, 6, 100, 200, 19, "" },
        /*505*/ { BARCODE_DBAR_OMN_CC, -1, 21.9, "1234567890123", "[20]01[90]12345678901234567890", 0, 21.9, 6, 100, 200, 43.799999, "" },
        /*506*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 21.9, "1234567890123", "[20]01[90]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 21.9, 6, 100, 200, 43.799999, "" },
        /*507*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 22, "1234567890123", "[20]01[90]12345678901234567890", 0, 22, 6, 100, 200, 44, "" },
        /*508*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 35, 12, 100, 200, 70, "CC-B 10 rows" },
        /*509*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 100, 200, 43, "" },
        /*510*/ { BARCODE_DBAR_OMN_CC, -1, 33.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 33.900002, 12, 100, 200, 67.800003, "" },
        /*511*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 33.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 33.900002, 12, 100, 200, 67.800003, "" },
        /*512*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 34, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 34, 12, 100, 200, 68, "" },
        /*513*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 39, 14, 100, 200, 78, "CC-B 12 rows" },
        /*514*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 25.5, 14, 100, 200, 51, "" },
        /*515*/ { BARCODE_DBAR_OMN_CC, -1, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 37.900002, 14, 100, 200, 75.800003, "" },
        /*516*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 37.900002, 14, 100, 200, 75.800003, "" },
        /*517*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 38, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 38, 14, 100, 200, 76, "" },
        /*518*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 38, "CC-A 4 rows" },
        /*519*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01", 0, 9.5, 6, 79, 158, 19, "" },
        /*520*/ { BARCODE_DBAR_LTD_CC, -1, 18, "1234567890123", "[20]01", 0, 18, 6, 79, 158, 36, "" },
        /*521*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 18, "1234567890123", "[20]01", ZINT_WARN_NONCOMPLIANT, 18, 6, 79, 158, 36, "" },
        /*522*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 19, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 38, "" },
        /*523*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 25, 9, 79, 158, 50, "CC-A 7 rows" },
        /*524*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 15.5, 9, 79, 158, 31, "" },
        /*525*/ { BARCODE_DBAR_LTD_CC, -1, 24.9, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 24.9, 9, 79, 158, 49.799999, "" },
        /*526*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 24.9, "1234567890123", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 24.9, 9, 79, 158, 49.799999, "" },
        /*527*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 25, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 25, 9, 79, 158, 50, "" },
        /*528*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 51, 22, 88, 176, 102, "CC-B 20 rows" },
        /*529*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 41.5, 22, 88, 176, 83, "" },
        /*530*/ { BARCODE_DBAR_LTD_CC, -1, 50.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 50.900002, 22, 88, 176, 101.8, "" },
        /*531*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 50.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 50.900002, 22, 88, 176, 101.8, "" },
        /*532*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 51, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 51, 22, 88, 176, 102, "" },
        /*533*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 82, "CC-A 3 rows" },
        /*534*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 7.5, 5, 134, 268, 15, "" },
        /*535*/ { BARCODE_DBAR_EXP_CC, -1, 40, "[01]12345678901231", "[20]01", 0, 40, 5, 134, 268, 80, "" },
        /*536*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 40, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 40, 5, 134, 268, 80, "" },
        /*537*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 41, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 82, "" },
        /*538*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 45, 7, 134, 268, 90, "CC-A 5 rows" },
        /*539*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 11.5, 7, 134, 268, 23, "" },
        /*540*/ { BARCODE_DBAR_EXP_CC, -1, 44, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 44, 7, 134, 268, 88, "" },
        /*541*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 44, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 44, 7, 134, 268, 88, "" },
        /*542*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 45, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 45, 7, 134, 268, 90, "" },
        /*543*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 55, 12, 134, 268, 110, "CC-B 10 rows" },
        /*544*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 134, 268, 43, "" },
        /*545*/ { BARCODE_DBAR_EXP_CC, -1, 54, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 54, 12, 134, 268, 108, "" },
        /*546*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 54, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 54, 12, 134, 268, 108, "" },
        /*547*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 55, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 55, 12, 134, 268, 110, "" },
        /*548*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01", 0, 50, 7, 99, 226, 110, "CC-A 3 rows" },
        /*549*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01", 0, 12.5, 7, 99, 226, 35, "" },
        /*550*/ { BARCODE_UPCA_CC, -1, 81.24, "12345678901", "[20]01", 0, 81.239998, 7, 99, 226, 172.48, "" },
        /*551*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 81.24, "12345678901", "[20]01", ZINT_WARN_NONCOMPLIANT, 81.239998, 7, 99, 226, 172.48, "" },
        /*552*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 81.25, "12345678901", "[20]01", 0, 81.25, 7, 99, 226, 172.5, "" },
        /*553*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 50, 10, 99, 226, 110, "CC-A 6 rows" },
        /*554*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 18.5, 10, 99, 226, 47, "" },
        /*555*/ { BARCODE_UPCA_CC, -1, 87.24, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 87.239998, 10, 99, 226, 184.48, "" },
        /*556*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 87.24, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 87.239998, 10, 99, 226, 184.48, "" },
        /*557*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 87.25, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 87.25, 10, 99, 226, 184.5, "" },
        /*558*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 50, 16, 99, 226, 110, "CC-B 12 rows" },
        /*559*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 30.5, 16, 99, 226, 71, "" },
        /*560*/ { BARCODE_UPCA_CC, -1, 99, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 99, 16, 99, 226, 208, "" },
        /*561*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 99, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", ZINT_WARN_NONCOMPLIANT, 99, 16, 99, 226, 208, "" },
        /*562*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 99.25, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 99.25, 16, 99, 226, 208.5, "" },
        /*563*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678", 0, 50, 11, 55, 134, 110, "CC-A 7 rows" },
        /*564*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678", 0, 20.5, 11, 55, 134, 51, "" },
        /*565*/ { BARCODE_UPCE_CC, -1, 89, "1234567", "[20]01[90]123456789012345678", 0, 89, 11, 55, 134, 188, "" },
        /*566*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 89, "1234567", "[20]01[90]123456789012345678", ZINT_WARN_NONCOMPLIANT, 89, 11, 55, 134, 188, "" },
        /*567*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 89.25, "1234567", "[20]01[90]123456789012345678", 0, 89.25, 11, 55, 134, 188.5, "" },
        /*568*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 50, 16, 55, 134, 110, "CC-A 12 rows" },
        /*569*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 30.5, 16, 55, 134, 71, "" },
        /*570*/ { BARCODE_UPCE_CC, -1, 99, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 99, 16, 55, 134, 208, "" },
        /*571*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 99, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 99, 16, 55, 134, 208, "" },
        /*572*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 99.25, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 99.25, 16, 55, 134, 208.5, "" },
        /*573*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 21, 55, 134, 110, "CC-B 17 rows" },
        /*574*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 40.5, 21, 55, 134, 91, "" },
        /*575*/ { BARCODE_UPCE_CC, -1, 109, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 109, 21, 55, 134, 228, "" },
        /*576*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 109, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 109, 21, 55, 134, 228, "" },
        /*577*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 109.25, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 109.25, 21, 55, 134, 228.5, "" },
        /*578*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 52.5, 27, 55, 134, 115, "CC-B 23 rows" },
        /*579*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 52.5, 27, 55, 134, 115, "" },
        /*580*/ { BARCODE_UPCE_CC, -1, 121, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 121, 27, 55, 134, 252, "" },
        /*581*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 121, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", ZINT_WARN_NONCOMPLIANT, 121, 27, 55, 134, 252, "" },
        /*582*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 121.25, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 121.25, 27, 55, 134, 252.5, "" },
        /*583*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "CC-A 5 rows" },
        /*584*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01", 0, 13.2, 9, 56, 112, 26.4, "" },
        /*585*/ { BARCODE_DBAR_STK_CC, -1, 23.9, "1234567890123", "[20]01", 0, 23.9, 9, 56, 112, 47.799999, "" },
        /*586*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 23.9, "1234567890123", "[20]01", ZINT_WARN_NONCOMPLIANT, 23.9, 9, 56, 112, 47.799999, "" },
        /*587*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 24, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "" },
        /*588*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 38, 16, 56, 112, 76, "CC-A 12 rows" },
        /*589*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 27.200001, 16, 56, 112, 54.400002, "" },
        /*590*/ { BARCODE_DBAR_STK_CC, -1, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 37.900002, 16, 56, 112, 75.800003, "" },
        /*591*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 37.900002, 16, 56, 112, 75.800003, "" },
        /*592*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 38, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 38, 16, 56, 112, 76, "" },
        /*593*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 48, 21, 56, 112, 96, "CC-B 17 rows" },
        /*594*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 37.200001, 21, 56, 112, 74.400002, "" },
        /*595*/ { BARCODE_DBAR_STK_CC, -1, 47.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 47.900002, 21, 56, 112, 95.800003, "" },
        /*596*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 47.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 47.900002, 21, 56, 112, 95.800003, "" },
        /*597*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 48, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 48, 21, 56, 112, 96, "" },
        /*598*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]1234567890123456", 0, 82, 12, 56, 112, 164, "CC-A 6 rows" },
        /*599*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]1234567890123456", 0, 17, 12, 56, 112, 34, "" },
        /*600*/ { BARCODE_DBAR_OMNSTK_CC, -1, 81, "1234567890123", "[20]01[90]1234567890123456", 0, 81, 12, 56, 112, 162, "" },
        /*601*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 81, "1234567890123", "[20]01[90]1234567890123456", ZINT_WARN_NONCOMPLIANT, 81, 12, 56, 112, 162, "" },
        /*602*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 82, "1234567890123", "[20]01[90]1234567890123456", 0, 82, 12, 56, 112, 164, "" },
        /*603*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 94, 18, 56, 112, 188, "CC-A 12 rows" },
        /*604*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 29, 18, 56, 112, 58, "" },
        /*605*/ { BARCODE_DBAR_OMNSTK_CC, -1, 93.5, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 93.5, 18, 56, 112, 187, "" },
        /*606*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 93.5, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", ZINT_WARN_NONCOMPLIANT, 93.5, 18, 56, 112, 187, "" },
        /*607*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 94, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 94, 18, 56, 112, 188, "" },
        /*608*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 104, 23, 56, 112, 208, "CC-B 17 rows" },
        /*609*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 39, 23, 56, 112, 78, "" },
        /*610*/ { BARCODE_DBAR_OMNSTK_CC, -1, 103, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 103, 23, 56, 112, 206, "" },
        /*611*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 103, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 103, 23, 56, 112, 206, "" },
        /*612*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 104, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 104, 23, 56, 112, 208, "" },
        /*613*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156, "3 rows, CC-A 3 rows" },
        /*614*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 11, 9, 102, 204, 22, "" },
        /*615*/ { BARCODE_DBAR_EXPSTK_CC, -1, 77, "[01]12345678901231", "[20]01", 0, 77, 9, 102, 204, 154, "" },
        /*616*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 77, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 77, 9, 102, 204, 154, "" },
        /*617*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 78, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156, "" },
        /*618*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "5 rows, CC-A 3 rows" },
        /*619*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 21.5, 21, 102, 204, 43, "" },
        /*620*/ { BARCODE_DBAR_EXPSTK_CC, -1, 188, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 188, 21, 102, 204, 376, "" },
        /*621*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 188, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", ZINT_WARN_NONCOMPLIANT, 188, 21, 102, 204, 376, "" },
        /*622*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 189, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "" },
        /*623*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 80, 10, 102, 204, 160, "3 rows, CC-A 4 rows" },
        /*624*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 13, 10, 102, 204, 26, "" },
        /*625*/ { BARCODE_DBAR_EXPSTK_CC, -1, 79, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 79, 10, 102, 204, 158, "" },
        /*626*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 79, "[01]12345678901231", "[20]01[90]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 79, 10, 102, 204, 158, "" },
        /*627*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 80, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 80, 10, 102, 204, 160, "" },
        /*628*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 92, 16, 102, 204, 184, "3 rows, CC-B 10 rows" },
        /*629*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 25, 16, 102, 204, 50, "" },
        /*630*/ { BARCODE_DBAR_EXPSTK_CC, -1, 91, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 91, 16, 102, 204, 182, "" },
        /*631*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 91, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 91, 16, 102, 204, 182, "" },
        /*632*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 92, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 92, 16, 102, 204, 184, "" },
        /*633*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 203, 28, 102, 204, 406, "5 rows, CC-B 10 rows" },
        /*634*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 35.5, 28, 102, 204, 71, "" },
        /*635*/ { BARCODE_DBAR_EXPSTK_CC, -1, 202, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 202, 28, 102, 204, 404, "" },
        /*636*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 202, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 202, 28, 102, 204, 404, "" },
        /*637*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 203, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 203, 28, 102, 204, 406, "" },
        /*638*/ { BARCODE_CHANNEL, -1, 1, "1", "", 0, 1, 1, 19, 38, 2, "" },
        /*639*/ { BARCODE_CHANNEL, -1, 3.75, "123", "", 0, 3.75, 1, 23, 46, 7.5, "" },
        /*640*/ { BARCODE_CHANNEL, COMPLIANT_HEIGHT, 3.75, "123", "", ZINT_WARN_NONCOMPLIANT, 3.75, 1, 23, 46, 7.5, "Min height data-length dependent" },
        /*641*/ { BARCODE_CHANNEL, COMPLIANT_HEIGHT, 4, "123", "", 0, 4, 1, 23, 46, 8, "" },
        /*642*/ { BARCODE_CODEONE, -1, 1, "12345678901234567890", "", 0, 16, 16, 18, 36, 32, "Fixed height, symbol->height ignored" },
        /*643*/ { BARCODE_GRIDMATRIX, -1, 1, "ABC", "", 0, 18, 18, 18, 36, 36, "Fixed width-to-height ratio, symbol->height ignored" },
        /*644*/ { BARCODE_UPNQR, -1, 1, "1234567890AB", "", 0, 77, 77, 77, 154, 154, "Fixed width-to-height ratio, symbol->height ignored" },
        /*645*/ { BARCODE_ULTRA, -1, 1, "1234567890", "", 0, 13, 13, 18, 36, 26, "Fixed width-to-height ratio, symbol->height ignored" },
        /*646*/ { BARCODE_RMQR, -1, 1, "12345", "", 0, 11, 11, 27, 54, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*647*/ { BARCODE_BC412, -1, 1, "1234567", "", 0, 1, 1, 102, 204, 2, "" },
        /*648*/ { BARCODE_BC412, -1, 13.6, "1234567", "", 0, 13.6, 1, 102, 204, 27.200001, "" },
        /*649*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 13.6, "1234567", "", ZINT_WARN_NONCOMPLIANT, 13.6, 1, 102, 204, 27.200001, "" },
        /*650*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 13.65, "1234567", "", 0, 13.65, 1, 102, 204, 27.299999, "" },
        /*651*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 21.3, "1234567", "", 0, 21.3, 1, 102, 204, 42.599998, "" },
        /*652*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 21.35, "1234567", "", ZINT_WARN_NONCOMPLIANT, 21.35, 1, 102, 204, 42.700001, "" },
        /*653*/ { BARCODE_DXFILMEDGE, -1, 1, "120476", "", 0, 1, 2, 23, 46, 2, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].output_options != -1) {
            symbol->output_options = data[i].output_options;
        }
        if (data[i].height != -1) {
            symbol->height = data[i].height;
        }
        symbol->input_mode = UNICODE_MODE;
        symbol->show_hrt = 0; /* Note: disabling HRT */
        symbol->debug |= debug;

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = (int) strlen(text);

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%s) ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%s) vector NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %.5g, \"%s\", \"%s\", %s, %.9g, %d, %d, %.9g, %.9g, \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilOutputOptionsName(data[i].output_options),
                    data[i].height, data[i].data, data[i].composite, testUtilErrorName(data[i].ret),
                    symbol->height, symbol->rows, symbol->width, symbol->vector->width, symbol->vector->height, data[i].comment);
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.9g != %.9g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_height_per_row(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        float height;
        float scale;
        const char *data;
        const char *composite;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;

        const char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, "1234567890", "", 0, 21, 7, 103, 206, 42, "" },
        /*  1*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 3.5, 7, 103, 206, 7, "" },
        /*  2*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 7, 7, 103, 206, 14, "" },
        /*  3*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.25, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 8.75, 7, 103, 206, 17.5, "" },
        /*  4*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 10.5, 7, 103, 206, 21, "" },
        /*  5*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.7, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 11.900001, 7, 103, 206, 23.800001, "" },
        /*  6*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.74, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 12.18, 7, 103, 206, 24.360001, "" },
        /*  7*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.75, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 12.25, 7, 103, 206, 24.5, "" },
        /*  8*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 14, 7, 103, 206, 28, "" },
        /*  9*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.1, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 14.699999, 7, 103, 206, 29.399998, "" },
        /* 10*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.25, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 15.75, 7, 103, 206, 31.5, "" },
        /* 11*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17.5, 7, 103, 206, 35, "" },
        /* 12*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.75, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 19.25, 7, 103, 206, 38.5, "" },
        /* 13*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 3, -1, "1234567890", "", 0, 21, 7, 103, 206, 42, "Default" },
        /* 14*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 3.5, -1, "1234567890", "", 0, 24.5, 7, 103, 206, 49, "" },
        /* 15*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1000, -1, "1234567890", "", 0, 7000, 7, 103, 206, 14000, "" },
        /* 16*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, 5, 0.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 2.5, 5, 120, 240, 5, "5 rows" },
        /* 17*/ { BARCODE_PDF417COMP, HEIGHTPERROW_MODE, -1, -1, -1, 3.5, -1, "1234567890", "", 0, 24.5, 7, 69, 138, 49, "" },
        /* 18*/ { BARCODE_HIBC_PDF, HEIGHTPERROW_MODE, -1, -1, -1, 3.5, -1, "1234567890", "", 0, 28, 8, 103, 206, 56, "" },
        /* 19*/ { BARCODE_CODE16K, -1, -1, -1, -1, -1, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "" },
        /* 20*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890", "", 0, 1, 2, 70, 162, 6, "(0.5 * 2 rows + 2 binds) * 2 scale = 6 (separator will cover rows)" },
        /* 21*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 0.7, -1, "1234567890", "", 0, 1.4, 2, 70, 162, 6.8000002, "" },
        /* 22*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 0.75, -1, "1234567890", "", 0, 1.5, 2, 70, 162, 7, "" },
        /* 23*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1, -1, "1234567890", "", 0, 2, 2, 70, 162, 8, "" },
        /* 24*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1.1, -1, "1234567890", "", 0, 2.2, 2, 70, 162, 8.3999996, "" },
        /* 25*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1.2, -1, "1234567890", "", 0, 2.4000001, 2, 70, 162, 8.8000002, "" },
        /* 26*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1.25, -1, "1234567890", "", 0, 2.5, 2, 70, 162, 9, "" },
        /* 27*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1.5, -1, "1234567890", "", 0, 3, 2, 70, 162, 10, "" },
        /* 28*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 2, -1, "1234567890", "", 0, 4, 2, 70, 162, 12, "" },
        /* 29*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 10, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "Default" },
        /* 30*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 10.5, -1, "1234567890", "", 0, 21, 2, 70, 162, 46, "" },
        /* 31*/ { BARCODE_CODE49, -1, -1, -1, -1, -1, -1, "12345678901234567890", "", 0, 30, 3, 70, 162, 64, "" },
        /* 32*/ { BARCODE_CODE49, HEIGHTPERROW_MODE, -1, -1, -1, 2, -1, "12345678901234567890", "", 0, 6, 3, 70, 162, 16, "(2 * 3 rows + 2 binds) * 2 scale = 16" },
        /* 33*/ { BARCODE_CODE49, HEIGHTPERROW_MODE, -1, -1, -1, 10, -1, "12345678901234567890", "", 0, 30, 3, 70, 162, 64, "Default" },
        /* 34*/ { BARCODE_CODE49, HEIGHTPERROW_MODE, -1, -1, -1, 10.5, -1, "12345678901234567890", "", 0, 31.5, 3, 70, 162, 67, "" },
        /* 35*/ { BARCODE_CODABLOCKF, -1, -1, -1, -1, -1, -1, "1234567890123456789", "", 0, 40, 4, 101, 242, 84, "" },
        /* 36*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890123456789", "", 0, 2, 4, 101, 242, 8, "(0.5 * 4 rows + 2 binds) * 2 scale = 8" },
        /* 37*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 1, -1, "1234567890123456789", "", 0, 4, 4, 101, 242, 12, "" },
        /* 38*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 3, -1, "1234567890123456789", "", 0, 12, 4, 101, 242, 28, "" },
        /* 39*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 10, -1, "1234567890123456789", "", 0, 40, 4, 101, 242, 84, "Default when <= 12 cols" },
        /* 40*/ { BARCODE_CODABLOCKF, -1, 2, -1, -1, -1, -1, "12345678901234567890123456", "", 0, 20, 2, 200, 440, 44, "2 rows" },
        /* 41*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, 2, -1, -1, 10.15, -1, "12345678901234567890123456", "", 0, 20.299999, 2, 200, 440, 44.599998, "Default for 13 cols" },
        /* 42*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, 2, -1, -1, 2000, -1, "12345678901234567890123456", "", 0, 4000, 2, 200, 440, 8004, "" },
        /* 43*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 10.5, -1, "1234567890123456789", "", 0, 42, 4, 101, 242, 88, "" },
        /* 44*/ { BARCODE_HIBC_BLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 10.5, -1, "1234567890123456789", "", 0, 42, 4, 101, 242, 88, "" },
        /* 45*/ { BARCODE_MICROPDF417, -1, -1, -1, -1, -1, -1, "1234567890", "", 0, 12, 6, 82, 164, 24, "" },
        /* 46*/ { BARCODE_MICROPDF417, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 3, 6, 82, 164, 6, "" },
        /* 47*/ { BARCODE_MICROPDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2, -1, "1234567890", "", 0, 12, 6, 82, 164, 24, "Default" },
        /* 48*/ { BARCODE_MICROPDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.5, -1, "1234567890", "", 0, 15, 6, 82, 164, 30, "" },
        /* 49*/ { BARCODE_MICROPDF417, HEIGHTPERROW_MODE, -1, -1, -1, 3, -1, "1234567890", "", 0, 18, 6, 82, 164, 36, "" },
        /* 50*/ { BARCODE_HIBC_MICPDF, HEIGHTPERROW_MODE, -1, -1, -1, 3, -1, "1234567890", "", 0, 42, 14, 38, 76, 84, "" },
        /* 51*/ { BARCODE_DBAR_EXPSTK, -1, -1, -1, -1, -1, -1, "[8110]106141416543213500110000310123196000", "", 0, 145, 13, 102, 204, 290, "" },
        /* 52*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "[8110]106141416543213500110000310123196000", "", 0, 11, 13, 102, 204, 22, "(0.5 * 4 rows + 9 seps) * 2 scale = 22" },
        /* 53*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 9, -1, "[8110]106141416543213500110000310123196000", "", 0, 45, 13, 102, 204, 90, "" },
        /* 54*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 9.5, -1, "[8110]106141416543213500110000310123196000", "", 0, 47, 13, 102, 204, 94, "" },
        /* 55*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 10, -1, "[8110]106141416543213500110000310123196000", "", 0, 49, 13, 102, 204, 98, "" },
        /* 56*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 34, -1, "[8110]106141416543213500110000310123196000", "", 0, 145, 13, 102, 204, 290, "Default" },
        /* 57*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 50, -1, "[8110]106141416543213500110000310123196000", "", 0, 209, 13, 102, 204, 418, "" },
        /* 58*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, -1, -1, -1, -1, "[8110]106141416543213500110000310123196000", "[8112]017777777666666223456789", 0, 154, 18, 102, 204, 308, "" },
        /* 59*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, -1, -1, 0.5, -1, "[8110]106141416543213500110000310123196000", "[8112]017777777666666223456789", 0, 20, 18, 102, 204, 40, "(0.5 * 4 rows + 10 seps + 2 * 4 cc) * 2 scale = 40" },
        /* 60*/ { BARCODE_DBAR_EXPSTK_CC, HEIGHTPERROW_MODE, -1, -1, -1, 34, -1, "[8110]106141416543213500110000310123196000", "[8112]017777777666666223456789", 0, 154, 18, 102, 204, 308, "Default" },
        /* 61*/ { BARCODE_DBAR_EXPSTK_CC, HEIGHTPERROW_MODE, -1, -1, -1, 35, -1, "[8110]106141416543213500110000310123196000", "[8112]017777777666666223456789", 0, 158, 18, 102, 204, 316, "" },
        /* 62*/ { BARCODE_PHARMA_TWO, -1, -1, -1, -1, -1, -1, "1234", "", 0, 10, 2, 13, 26, 20, "" },
        /* 63*/ { BARCODE_PHARMA_TWO, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234", "", 0, 1, 2, 13, 26, 2, "" },
        /* 64*/ { BARCODE_PHARMA_TWO, HEIGHTPERROW_MODE, -1, -1, -1, 2.1, -1, "1234", "", 0, 4.1999998, 2, 13, 26, 8.3999996, "" },
        /* 65*/ { BARCODE_PHARMA_TWO, HEIGHTPERROW_MODE, -1, -1, -1, 2.2, -1, "1234", "", 0, 4.4000001, 2, 13, 26, 8.8000002, "" },
        /* 66*/ { BARCODE_PHARMA_TWO, HEIGHTPERROW_MODE, -1, -1, -1, 2.25, -1, "1234", "", 0, 4.5, 2, 13, 26, 9, "" },
        /* 67*/ { BARCODE_DBAR_OMNSTK, -1, -1, -1, -1, -1, -1, "1234567890123", "", 0, 69, 5, 50, 100, 138, "" },
        /* 68*/ { BARCODE_DBAR_OMNSTK, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890123", "", 0, 4, 5, 50, 100, 8, "(0.5 * 2 rows + 3 separators) * 2 scale = 8" },
        /* 69*/ { BARCODE_DBAR_OMNSTK, HEIGHTPERROW_MODE, -1, -1, -1, 1, -1, "1234567890123", "", 0, 5, 5, 50, 100, 10, "" },
        /* 70*/ { BARCODE_DBAR_OMNSTK, HEIGHTPERROW_MODE, -1, -1, -1, 3.2, -1, "1234567890123", "", 0, 9.3999996, 5, 50, 100, 18.799999, "" },
        /* 71*/ { BARCODE_EAN13_CC, -1, -1, -1, -1, -1, -1, "123456789012", "[20]01", 0, 50, 7, 99, 226, 110, "" },
        /* 72*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, "123456789012", "[20]01", 0, 50, 7, 99, 226, 110, "" },
        /* 73*/ { BARCODE_EAN13_CC, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "(0.5 * 1 row + 2 * 3 seps + 2 * 3 cc rows + 5 guards) * 2 scale = 35" },
        /* 74*/ { BARCODE_EANX_CC, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "(0.5 * 1 row + 2 * 3 seps + 2 * 3 cc rows + 5 guards) * 2 scale = 35" },
        /* 75*/ { BARCODE_EAN13_CC, -1, -1, -1, -1, 0.5, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "0.5 height below fixed height" },
        /* 76*/ { BARCODE_EANX_CC, -1, -1, -1, -1, 0.5, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "0.5 height below fixed height" },
        /* 77*/ { BARCODE_EAN13_CC, HEIGHTPERROW_MODE, -1, -1, -1, 4, -1, "123456789012", "[20]01", 0, 16, 7, 99, 226, 42, "" },
        /* 78*/ { BARCODE_EANX_CC, HEIGHTPERROW_MODE, -1, -1, -1, 4, -1, "123456789012", "[20]01", 0, 16, 7, 99, 226, 42, "" },
        /* 79*/ { BARCODE_EAN13_CC, -1, -1, -1, -1, 4, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "4 height below fixed height" },
        /* 80*/ { BARCODE_EANX_CC, -1, -1, -1, -1, 4, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "4 height below fixed height" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].height != -1) {
            symbol->height = data[i].height;
        }
        if (data[i].scale != -1) {
            symbol->scale = data[i].scale;
        }
        symbol->show_hrt = 0; /* Note: disabling HRT */

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, text, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%s) ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%s) vector NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, %d, %d, %.5g, %.5g, \"%s\", \"%s\", %s, %.9g, %d, %d, %.9g, %.9g, \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode),
                    data[i].option_1, data[i].option_2, data[i].option_3, data[i].height, data[i].scale,
                    data[i].data, data[i].composite, testUtilErrorName(data[i].ret),
                    symbol->height, symbol->rows, symbol->width, symbol->vector->width, symbol->vector->height, data[i].comment);
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.9g != %.9g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_hrt_content_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int show_hrt;
        int option_2;
        int output_options;
        const char *data;
        int length;

        int ret;
        float expected_vector_height;
        float expected_vector_width;
        const char *expected;
        int expected_length;
        const char *expected_content;
        int expected_content_length;
        const char *expected_errtxt;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, BARCODE_MEMORY_FILE, "12345\00067890", 11, 0, 116.279999, 268, "12345 67890", -1, "", -1, "" },
        /*  1*/ { BARCODE_CODE128, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "12345\00067890", 11, 0, 116.279999, 268, "12345 67890", -1, "12345\00067890", 11, "" },
        /*  2*/ { BARCODE_EXCODE39, -1, -1, BARCODE_MEMORY_FILE, "12345\00067890", 11, 0, 116.279999, 362, "12345 67890", -1, "", -1, "" },
        /*  3*/ { BARCODE_EXCODE39, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "12345\00067890", 11, 0, 116.279999, 362, "12345 67890", -1, "12345\00067890", 11, "" },
        /*  4*/ { BARCODE_TELEPEN, -1, -1, BARCODE_MEMORY_FILE, "12345\00067890", 11, 0, 116.279999, 448, "12345 67890", -1, "", -1, "" },
        /*  5*/ { BARCODE_TELEPEN, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "12345\00067890", 11, 0, 116.279999, 448, "12345 67890", -1, "12345\00067890n", 12, "" },
        /*  6*/ { BARCODE_EAN13, -1, -1, BARCODE_MEMORY_FILE, "123456789012", -1, 0, 118, 226, "1234567890128", -1, "", -1, "" },
        /*  7*/ { BARCODE_EANX, -1, -1, BARCODE_MEMORY_FILE, "123456789012", -1, 0, 118, 226, "1234567890128", -1, "", -1, "" },
        /*  8*/ { BARCODE_EAN13, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "123456789012", -1, 0, 118, 226, "1234567890128", -1, "1234567890128", -1, "" },
        /*  9*/ { BARCODE_EANX, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "123456789012", -1, 0, 118, 226, "1234567890128", -1, "1234567890128", -1, "" },
        /* 10*/ { BARCODE_EAN13, -1, -1, BARCODE_MEMORY_FILE, "123456789012+12", -1, 0, 118, 276, "1234567890128+12", -1, "", -1, "" },
        /* 11*/ { BARCODE_EANX, -1, -1, BARCODE_MEMORY_FILE, "123456789012+12", -1, 0, 118, 276, "1234567890128+12", -1, "", -1, "" },
        /* 12*/ { BARCODE_EAN13, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "123456789012+12", -1, 0, 118, 276, "1234567890128+12", -1, "123456789012812", -1, "" },
        /* 13*/ { BARCODE_EANX, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "123456789012+12", -1, 0, 118, 276, "1234567890128+12", -1, "123456789012812", -1, "" },
        /* 14*/ { BARCODE_CODE39, -1, -1, BARCODE_MEMORY_FILE, "ABC14", -1, 0, 116.279999, 180, "*ABC14*", -1, "", -1, "" },
        /* 15*/ { BARCODE_CODE39, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "ABC14", -1, 0, 116.279999, 180, "*ABC14*", -1, "ABC14", -1, "" },
        /* 16*/ { BARCODE_CODE39, -1, 1, BARCODE_MEMORY_FILE, "ABC14", -1, 0, 116.279999, 206, "*ABC14_*", -1, "", -1, "" }, /* Check digit space rendered as underscore */
        /* 17*/ { BARCODE_CODE39, -1, 1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "ABC14", -1, 0, 116.279999, 206, "*ABC14_*", -1, "ABC14 ", -1, "" },
        /* 18*/ { BARCODE_POSTNET, -1, -1, BARCODE_MEMORY_FILE, "12345", -1, 0, 24, 126, "", -1, "", -1, "" },
        /* 19*/ { BARCODE_POSTNET, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "12345", -1, 0, 24, 126, "", -1, "123455", -1, "" }, /* HRT not printed */
        /* 20*/ { BARCODE_POSTNET, 0, -1, BARCODE_MEMORY_FILE, "12345", -1, 0, 24, 126, "", -1, "", -1, "" },
        /* 21*/ { BARCODE_POSTNET, 0, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "12345", -1, 0, 24, 126, "", -1, "123455", -1, "" }, /* HRT not printed */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;
    int expected_length, expected_content_length;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, "mem.svg");

        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                    -1 /*option_1*/, data[i].option_2, -1 /*option_3*/, data[i].output_options,
                    data[i].data, data[i].length, debug);
        expected_length = data[i].expected_length == -1 ? (int) strlen(data[i].expected) : data[i].expected_length;
        expected_content_length = data[i].expected_content_length == -1 ? (int) strlen(data[i].expected_content)
                                                                : data[i].expected_content_length;

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%s) ret %d != 0 (%s)\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        assert_equal(symbol->text_length, expected_length, "i:%d text_length %d != expected_length %d (%s)\n",
                    i, symbol->text_length, expected_length, symbol->text);
        assert_zero(memcmp(symbol->text, data[i].expected, expected_length), "i:%d memcmp(%s, %s, %d) != 0\n",
                    i, symbol->text, data[i].expected, expected_length);
        if (symbol->output_options & BARCODE_CONTENT_SEGS) {
            assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
            assert_nonnull(symbol->content_segs[0].source, "i:%d content_segs[0].source NULL\n", i);
            assert_equal(symbol->content_segs[0].length, expected_content_length,
                        "i:%d content_segs[0].length %d != expected_content_length %d\n",
                        i, symbol->content_segs[0].length, expected_content_length);
            assert_zero(memcmp(symbol->content_segs[0].source, data[i].expected_content, expected_content_length),
                        "i:%d memcmp(%.*s, %.*s, %d) != 0\n",
                        i, symbol->content_segs[0].length, symbol->content_segs[0].source, expected_content_length,
                        data[i].expected_content, expected_content_length);
        } else {
            assert_null(symbol->content_segs, "i:%d content_segs not NULL\n", i);
        }

        ret = ZBarcode_Print(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Print(%s) ret %d != %d (%s)\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n",
                    i, symbol->errtxt, data[i].expected_errtxt);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Print(%s) vector NULL\n",
                    i, testUtilBarcodeName(data[i].symbology));
        assert_equal(symbol->vector->height, data[i].expected_vector_height,
                    "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->vector->height,
                    data[i].expected_vector_height);
        assert_equal(symbol->vector->width, data[i].expected_vector_width,
                    "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_options", test_options },
        { "test_buffer_vector", test_buffer_vector },
        { "test_has_hrt", test_has_hrt },
        { "test_upcean_hrt", test_upcean_hrt },
        { "test_row_separator", test_row_separator },
        { "test_stacking", test_stacking },
        { "test_output_options", test_output_options },
        { "test_noncomposite_string_x", test_noncomposite_string_x },
        { "test_upcean_whitespace_width", test_upcean_whitespace_width },
        { "test_scale", test_scale },
        { "test_guard_descent", test_guard_descent },
        { "test_quiet_zones", test_quiet_zones },
        { "test_text_gap", test_text_gap },
        { "test_height", test_height },
        { "test_height_per_row", test_height_per_row },
        { "test_hrt_content_segs", test_hrt_content_segs },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
