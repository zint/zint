/*
    libzint - the open source barcode library
    Copyright (C) 2020-2025 Robin Stuart <rstuart114@gmail.com>

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

#include "testcommon.h"
#include "../reedsol.h"

/* Print out the log/alog tables for "backend/reedsol_logs.h" */
static void print_logs(const char *name, int logmod, unsigned int *logt, unsigned int *alog, int u16, int last) {
    int i;
    const char *type = u16 ? "short" : "char";
    const char *format = u16 ? " 0x%04X," : " 0x%02X,";

    printf("static const unsigned %s logt_%s[%d] = {", type, name, logmod + 1);
    for (i = 0; i < logmod + 1; i++) {
        if (i % 16 == 0) printf("\n   ");
        printf(format, i ? logt[i] : 0);
    }
    printf("\n};\n");

    printf("static const unsigned %s alog_%s[%d] = {", type, name, logmod * 2);
    for (i = 0; i < logmod; i++) {
        if (i % 16 == 0) printf("\n   ");
        printf(format, alog[i]);
    }
    /* Double antilog table */
    for (i = 0; i < logmod; i++) {
        if (i % 16 == 0) printf("\n   ");
        printf(format, alog[i]);
    }
    printf("\n};\n");
    if (!last) {
        printf("\n");
    }
}

static void gen_logs(const unsigned int prime_poly, int logmod, unsigned int *logt, unsigned int *alog) {
    int b, p, v;

    b = logmod + 1;

    /* Calculate the log/alog tables */
    for (p = 1, v = 0; v < logmod; v++) {
        alog[v] = p;
        logt[p] = v;
        p <<= 1;
        if (p & b)
            p ^= prime_poly;
    }
}

/* Dummy to generate static log/antilog tables for "backend/reedsol_logs.h" */
static void test_generate(const testCtx *const p_ctx) {

    struct item {
        const char *name;
        int logmod;
        unsigned int prime_poly;
        int u16;
    };
    struct item data[] = {
        { "0x13", 15, 0x13, 0 },
        { "0x25", 31, 0x25, 0 },
        { "0x43", 63, 0x43, 0 },
        { "0x89", 127, 0x89, 0 },
        { "0x11d", 255, 0x11d, 0 },
        { "0x12d", 255, 0x12d, 0 },
        { "0x163", 255, 0x163, 0 },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    unsigned int logt[4096];
    unsigned int alog[8192];

    if (!p_ctx->generate) {
        return;
    }

    for (i = 0; i < data_size; i++) {
        gen_logs(data[i].prime_poly, data[i].logmod, logt, alog);
        print_logs(data[i].name, data[i].logmod, logt, alog, data[i].u16, i + 1 == data_size);
    }
}

static void test_encoding(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        unsigned int prime_poly;
        int nsym;
        int index;
        int datalen;
        unsigned char data[256];

        unsigned char expected[256];
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { 0x43, 4, 1, 7, { 4, 20, 49, 37, 49, 38, 23 }, { 58, 53, 17, 54 } }, /* AUSPOST Australia Post Customer Barcoding Technical Specifications Diagram 10 */
        /*  1*/ { 0x43, 7, 1, 10, { 9, 50, 1, 41, 47, 2, 39, 37, 1, 27 }, { 40, 20, 10, 16, 8, 50, 38 } }, /* AZTEC ISO/IEC 24778:2008 Section G.4 */
        /*  2*/ { 0x13, 5, 1, 2, { 0, 9 }, { 9, 1, 3, 2, 12 } }, /* AZTEC ISO/IEC 24778:2008 Section G.4 Mode Message */
        /*  3*/ { 0x12d, 5, 1, 3, { 142, 164, 186 }, { 102, 88, 5, 25, 114 } }, /* DATAMATRIX ISO/IEC 16022:2006 Annex O */
        /*  4*/ { 0x89, 25, 1, 25, { 42, 13, 54, 39, 124, 91, 121, 65, 28, 40, 95, 48, 0, 126, 0, 126, 0, 126, 0, 126, 0, 126, 0, 126, 0 }, { 102, 70, 92, 79, 2, 52, 98, 62, 48, 33, 14, 4, 101, 17, 55, 89, 100, 23, 35, 112, 54, 20, 2, 47, 123 } }, /* GRIDMATRIX AIMD014 Section 6.8 */
        /*  5*/ { 0x163, 4, 1, 21, { 0x11, 0xED, 0xC8, 0xC5, 0x40, 0x0F, 0xF4 }, { 0x1D, 0x68, 0xB4, 0xEB } }, /* HANXIN ISO/IEC DIS 20830:2019 Annex K.1 */
        /*  6*/ { 0x163, 24, 1, 27, { 0x11, 0xED, 0xC8, 0xC5, 0x40, 0x0F, 0xF4, 0x8A, 0x2C, 0xC3, 0x4E, 0x3D, 0x09, 0x25, 0x9A, 0x7A, 0x29, 0xAB, 0xEA, 0x3E, 0x46, 0x4C, 0x7E, 0x73, 0xE8, 0x6C, 0xC7 }, { 0x77, 0x49, 0xCD, 0x0B, 0x99, 0x9A, 0x5D, 0x74, 0x84, 0xB0, 0x11, 0xAD, 0x82, 0xA4, 0xCF, 0x99, 0xA2, 0xDD, 0xA5, 0x7A, 0xE0, 0x0C, 0x57, 0x08 } }, /* HANXIN ISO/IEC DIS 20830:2019 Annex K.2 1st block */
        /*  7*/ { 0x163, 24, 1, 27, { 0xE7, 0x3E, 0x33, 0x29, 0xE8, 0xFC, }, { 0x9B, 0x03, 0x94, 0xB7, 0xFE, 0xA1, 0x79, 0x1B, 0x00, 0x5D, 0x20, 0xCF, 0xCF, 0x4A, 0x69, 0xA6, 0x11, 0xAA, 0xE6, 0x5F, 0x8A, 0x68, 0xA7, 0xA2 } }, /* HANXIN ISO/IEC DIS 20830:2019 Annex K.2 2nd block */
        /*  8*/ { 0x163, 24, 1, 29, { 0x00 }, { 0x00 } }, /* HANXIN ISO/IEC DIS 20830:2019 Annex K.2 3rd block */
        /*  9*/ { 0x25, 6, 1, 16, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4 }, { 15, 23, 3, 23, 7, 14 } }, /* MAILMARK_4S Royal Mail Mailmark barcode C encoding and decoding Example 2.3.1 */
        /* 10*/ { 0x25, 6, 1, 16, { 15, 22, 3, 25, 23, 26, 7, 3, 20, 14, 1, 4, 16, 3, 9, 28 }, { 24, 6, 16, 24, 22, 27 } }, /* MAILMARK_4S Royal Mail Mailmark barcode C encoding and decoding Example 2.3.2 */
        /* 11*/ { 0x25, 7, 1, 19, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4 }, { 18, 11, 14, 7, 20, 1, 20 } }, /* MAILMARK_4S Royal Mail Mailmark barcode L encoding and decoding Example 2.3.1 */
        /* 12*/ { 0x25, 7, 1, 19, { 0, 8, 21, 10, 29, 1, 29, 21, 2, 24, 15, 2, 19, 1, 4, 15, 11, 4, 16 }, { 16, 16, 6, 8, 9, 7, 19 } }, /* MAILMARK_4S Royal Mail Mailmark barcode L encoding and decoding Example 2.3.2 */
        /* 13*/ { 0x43, 10, 1, 10, { 4, 13, 63, 1, 24, 9, 59, 3, 15, 4 }, { 16, 5, 20, 22, 34, 53, 51, 42, 2, 50 } }, /* MAXICODE Annex H Primary */
        /* 14*/ { 0x43, 20, 1, 42, { 5, 57, 49, 47, 8, 18, 59, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33 }, { 23, 2, 8, 62, 17, 23, 19, 14, 19, 19, 30, 2, 63, 13, 39, 6, 6, 58, 2, 31 } }, /* MAXICODE Annex H Secondary odd */
        /* 15*/ { 0x43, 20, 1, 42, { 47, 40, 57, 3, 1, 19, 41, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33 }, { 14, 61, 53, 63, 45, 51, 32, 0, 8, 4, 35, 35, 5, 60, 17, 39, 28, 22, 15, 1 } }, /* MAXICODE Annex H Secondary even */
        /* 16*/ { 0x11d, 10, 0, 16, { 0x10, 0x20, 0x0C, 0x56, 0x61, 0x80, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11 }, { 0x55, 0x2C, 0x87, 0xC7, 0x36, 0xED, 0xC1, 0xD4, 0x24, 0xA5 } }, /* QRCODE Annex I.2 */
        /* 17*/ { 0x11d, 5, 0, 5, { 0x40, 0x18, 0xAC, 0xC3, 0x00 }, { 0x30, 0xAE, 0x22, 0x0D, 0x86 } }, /* QRCODE Annex I.3 */
        /* 18*/ { 0x163, 256, 0, 1, { 0xFF }, { 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255 } },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {
        int j;
        rs_t rs;
        unsigned char res[1024];

        if (testContinue(p_ctx, i)) continue;

        rs_init_gf(&rs, data[i].prime_poly);
        rs_init_code(&rs, data[i].nsym, data[i].index);
        rs_encode(&rs, data[i].datalen, data[i].data, res);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            fprintf(stderr, "res "); for (j = data[i].nsym - 1; j >= 0; j--) fprintf(stderr, "%d, ", res[j]); fprintf(stderr, "\n");
            fprintf(stderr, "exp "); for (j = 0; j < data[i].nsym; j++) fprintf(stderr, "%d, ", data[i].expected[j]); fprintf(stderr, "\n");
        }
        for (j = 0; j < data[i].nsym; j++) {
            int k = data[i].nsym - 1 - j;
            assert_equal(res[k], data[i].expected[j], "i:%d res[%d] %d != expected[%d] %d\n", i, k, res[k], j, data[i].expected[j]);
        }
    }

    testFinish();
}

static void test_encoding_uint(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        unsigned int prime_poly;
        int nsym;
        int index;
        int datalen;
        unsigned int data[256];

        unsigned int expected[256];
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { 0x43, 4, 1, 7, { 4, 20, 49, 37, 49, 38, 23 }, { 58, 53, 17, 54 } }, /* AUSPOST Australia Post Customer Barcoding Technical Specifications Diagram 10 */
        /*  1*/ { 0x43, 7, 1, 10, { 9, 50, 1, 41, 47, 2, 39, 37, 1, 27 }, { 40, 20, 10, 16, 8, 50, 38 } }, /* AZTEC ISO/IEC 24778:2008 Section G.4 */
        /*  2*/ { 0x13, 5, 1, 2, { 0, 9 }, { 9, 1, 3, 2, 12 } }, /* AZTEC ISO/IEC 24778:2008 Section G.4 Mode Message */
        /*  3*/ { 0x12d, 5, 1, 3, { 142, 164, 186 }, { 102, 88, 5, 25, 114 } }, /* DATAMATRIX ISO/IEC 16022:2006 Annex O */
        /*  4*/ { 0x89, 25, 1, 25, { 42, 13, 54, 39, 124, 91, 121, 65, 28, 40, 95, 48, 0, 126, 0, 126, 0, 126, 0, 126, 0, 126, 0, 126, 0 }, { 102, 70, 92, 79, 2, 52, 98, 62, 48, 33, 14, 4, 101, 17, 55, 89, 100, 23, 35, 112, 54, 20, 2, 47, 123 } }, /* GRIDMATRIX AIMD014 Section 6.8 */
        /*  5*/ { 0x163, 4, 1, 21, { 0x11, 0xED, 0xC8, 0xC5, 0x40, 0x0F, 0xF4 }, { 0x1D, 0x68, 0xB4, 0xEB } }, /* HANXIN ISO/IEC DIS 20830:2019 Annex K.1 */
        /*  6*/ { 0x163, 24, 1, 27, { 0x11, 0xED, 0xC8, 0xC5, 0x40, 0x0F, 0xF4, 0x8A, 0x2C, 0xC3, 0x4E, 0x3D, 0x09, 0x25, 0x9A, 0x7A, 0x29, 0xAB, 0xEA, 0x3E, 0x46, 0x4C, 0x7E, 0x73, 0xE8, 0x6C, 0xC7 }, { 0x77, 0x49, 0xCD, 0x0B, 0x99, 0x9A, 0x5D, 0x74, 0x84, 0xB0, 0x11, 0xAD, 0x82, 0xA4, 0xCF, 0x99, 0xA2, 0xDD, 0xA5, 0x7A, 0xE0, 0x0C, 0x57, 0x08 } }, /* HANXIN ISO/IEC DIS 20830:2019 Annex K.2 1st block */
        /*  7*/ { 0x163, 24, 1, 27, { 0xE7, 0x3E, 0x33, 0x29, 0xE8, 0xFC, }, { 0x9B, 0x03, 0x94, 0xB7, 0xFE, 0xA1, 0x79, 0x1B, 0x00, 0x5D, 0x20, 0xCF, 0xCF, 0x4A, 0x69, 0xA6, 0x11, 0xAA, 0xE6, 0x5F, 0x8A, 0x68, 0xA7, 0xA2 } }, /* HANXIN ISO/IEC DIS 20830:2019 Annex K.2 2nd block */
        /*  8*/ { 0x163, 24, 1, 29, { 0x00 }, { 0x00 } }, /* HANXIN ISO/IEC DIS 20830:2019 Annex K.2 3rd block */
        /*  9*/ { 0x25, 6, 1, 16, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4 }, { 15, 23, 3, 23, 7, 14 } }, /* MAILMARK_4S Royal Mail Mailmark barcode C encoding and decoding Example 2.3.1 */
        /* 10*/ { 0x25, 6, 1, 16, { 15, 22, 3, 25, 23, 26, 7, 3, 20, 14, 1, 4, 16, 3, 9, 28 }, { 24, 6, 16, 24, 22, 27 } }, /* MAILMARK_4S Royal Mail Mailmark barcode C encoding and decoding Example 2.3.2 */
        /* 11*/ { 0x25, 7, 1, 19, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4 }, { 18, 11, 14, 7, 20, 1, 20 } }, /* MAILMARK_4S Royal Mail Mailmark barcode L encoding and decoding Example 2.3.1 */
        /* 12*/ { 0x25, 7, 1, 19, { 0, 8, 21, 10, 29, 1, 29, 21, 2, 24, 15, 2, 19, 1, 4, 15, 11, 4, 16 }, { 16, 16, 6, 8, 9, 7, 19 } }, /* MAILMARK_4S Royal Mail Mailmark barcode L encoding and decoding Example 2.3.2 */
        /* 13*/ { 0x43, 10, 1, 10, { 4, 13, 63, 1, 24, 9, 59, 3, 15, 4 }, { 16, 5, 20, 22, 34, 53, 51, 42, 2, 50 } }, /* MAXICODE Annex H Primary */
        /* 14*/ { 0x43, 20, 1, 42, { 5, 57, 49, 47, 8, 18, 59, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33 }, { 23, 2, 8, 62, 17, 23, 19, 14, 19, 19, 30, 2, 63, 13, 39, 6, 6, 58, 2, 31 } }, /* MAXICODE Annex H Secondary odd */
        /* 15*/ { 0x43, 20, 1, 42, { 47, 40, 57, 3, 1, 19, 41, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33 }, { 14, 61, 53, 63, 45, 51, 32, 0, 8, 4, 35, 35, 5, 60, 17, 39, 28, 22, 15, 1 } }, /* MAXICODE Annex H Secondary even */
        /* 16*/ { 0x11d, 10, 0, 16, { 0x10, 0x20, 0x0C, 0x56, 0x61, 0x80, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11 }, { 0x55, 0x2C, 0x87, 0xC7, 0x36, 0xED, 0xC1, 0xD4, 0x24, 0xA5 } }, /* QRCODE Annex I.2 */
        /* 17*/ { 0x11d, 5, 0, 5, { 0x40, 0x18, 0xAC, 0xC3, 0x00 }, { 0x30, 0xAE, 0x22, 0x0D, 0x86 } }, /* QRCODE Annex I.3 */
        /* 18*/ { 0x163, 256, 0, 1, { 0xFF }, { 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255 } },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {
        int j;
        rs_t rs;
        unsigned int res[1024];

        if (testContinue(p_ctx, i)) continue;

        rs_init_gf(&rs, data[i].prime_poly);
        rs_init_code(&rs, data[i].nsym, data[i].index);
        rs_encode_uint(&rs, data[i].datalen, data[i].data, res);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            fprintf(stderr, "res "); for (j = data[i].nsym - 1; j >= 0; j--) fprintf(stderr, "%d, ", res[j]); fprintf(stderr, "\n");
            fprintf(stderr, "exp "); for (j = 0; j < data[i].nsym; j++) fprintf(stderr, "%d, ", data[i].expected[j]); fprintf(stderr, "\n");
        }
        for (j = 0; j < data[i].nsym; j++) {
            int k = data[i].nsym - 1 - j;
            assert_equal(res[k], data[i].expected[j], "i:%d res[%d] %d != expected[%d] %d\n", i, k, res[k], j, data[i].expected[j]);
        }
    }

    testFinish();
}

static void test_uint_encoding(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        unsigned int prime_poly;
        int logmod;
        int nsym;
        int index;
        int datalen;
        unsigned int data[256];

        unsigned int expected[256];
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { 0x409, 1023, 4, 1, 7, { 0x3FF, 0x000, 0x100, 0x1FF, 0x3FF, 0x000, 0x123 }, { 674, 993, 153, 229 } },
        /*  1*/ { 0x1069, 4095, 4, 1, 7, { 0xFFF, 0x000, 0x700, 0x7FF, 0xFFF, 0x000, 0x123 }, { 575, 3494, 2350, 3472 } },
        /*  2*/ { 0x1000, 4095, 4, 0, 7, { 0xFFF, 0x000, 0x700, 0x7FF, 0xFFF, 0x000, 0x123 }, { 64, 0, 65, 1 } },
        /*  3*/ { 0x1000, 4095, 256, 0, 1, { 0xFFF }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2048, 0, 512, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 } },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {
        int j;
        rs_uint_t rs_uint = {0}; /* Suppress clang -fsanitize=memory false positive */
        unsigned int res[1024];

        if (testContinue(p_ctx, i)) continue;

        assert_nonzero(rs_uint_init_gf(&rs_uint, data[i].prime_poly, data[i].logmod), "i:%d rs_uint_init_gf() == 0\n", i);
        rs_uint_init_code(&rs_uint, data[i].nsym, data[i].index);
        rs_uint_encode(&rs_uint, data[i].datalen, data[i].data, res);
        rs_uint_free(&rs_uint);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            fprintf(stderr, "res "); for (j = data[i].nsym - 1; j >= 0; j--) fprintf(stderr, "%d, ", res[j]); fprintf(stderr, "\n");
            fprintf(stderr, "exp "); for (j = 0; j < data[i].nsym; j++) fprintf(stderr, "%d, ", data[i].expected[j]); fprintf(stderr, "\n");
        }
        for (j = 0; j < data[i].nsym; j++) {
            int k = data[i].nsym - 1 - j;
            assert_equal(res[k], data[i].expected[j], "i:%d res[%d] %d != expected[%d] %d\n", i, k, (int) res[k], j, (int) data[i].expected[j]);
        }

        /* Simulate rs_uint_init_gf() malloc() failure and rs_uint_init_gf()'s return val not being checked */
        assert_nonzero(rs_uint_init_gf(&rs_uint, data[i].prime_poly, data[i].logmod), "i:%d rs_uint_init_gf() == 0\n", i);
        free(rs_uint.logt);
        rs_uint.logt = NULL;
        free(rs_uint.alog);
        rs_uint.alog = NULL;

        rs_uint_init_code(&rs_uint, data[i].nsym, data[i].index);
        rs_uint_encode(&rs_uint, data[i].datalen, data[i].data, res);
        rs_uint_free(&rs_uint);

        for (j = 0; j < data[i].nsym; j++) {
            int k = data[i].nsym - 1 - j;
            assert_zero(res[k], "i:%d res[%d] %d != 0\n", i, k, (int) res[k]);
        }
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_generate", test_generate },
        { "test_encoding", test_encoding },
        { "test_encoding_uint", test_encoding_uint },
        { "test_uint_encoding", test_uint_encoding },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
