/*  fuzz_gs1.c - fuzzer for libzint (GS1-enabled symbologies, GS1_MODE) */
/*
    libzint - the open source barcode library
    Copyright (C) 2024 Robin Stuart <rstuart114@gmail.com>

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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if 0
#define Z_FUZZ_DEBUG                 /* Set `symbol->debug` flag */
#define Z_FUZZ_SET_OUTPUT_OPTIONS    /* Set `symbol->output_options` */
#endif
#include "fuzz.h"

int LLVMFuzzerTestOneInput(const unsigned char *data, size_t size) {
    static const int symbologies[] = {
        BARCODE_AZTEC, BARCODE_CODE16K, BARCODE_CODE49, BARCODE_CODEONE, BARCODE_DATAMATRIX, BARCODE_DBAR_EXP,
        BARCODE_DBAR_EXPSTK, BARCODE_DOTCODE, BARCODE_GS1_128, BARCODE_QRCODE, BARCODE_RMQR, BARCODE_ULTRA,
        BARCODE_EANX_CC, BARCODE_GS1_128_CC, BARCODE_DBAR_OMN_CC, BARCODE_DBAR_LTD_CC, BARCODE_DBAR_EXP_CC,
        BARCODE_UPCA_CC, BARCODE_UPCE_CC, BARCODE_DBAR_STK_CC, BARCODE_DBAR_OMNSTK_CC, BARCODE_DBAR_EXPSTK_CC,
    };
    struct zint_symbol *symbol;
    unsigned char *gs1_buf;
    int i;

    /* Ignore empty or very large input */
    if (size < 1 || size > 10000) {
        return 0;
    }

    symbol = ZBarcode_Create();
    assert(symbol);

    gs1_buf = (unsigned char *) malloc(size + 2);
    assert(gs1_buf);
    gs1_buf[0] = '['; /* Add dummy AI - along with GS1NOCHECK_MODE disables GS1 verification */
    gs1_buf[1] = ']';

    for (i = 0; i < ZARRAY_SIZE(symbologies); i++) {
        static const char primary_ai[] = "[01]12345678901231";
        static const char primary_upce[] = "12345670";
        static const char primary[] = "123456789012";
        const int idx = symbologies[i];
        const int is_composite = ZBarcode_Cap(idx, ZINT_CAP_COMPOSITE) & ZINT_CAP_COMPOSITE;
        const unsigned char *input;
        int length;
        int ret;

        assert(ZBarcode_ValidID(idx));

        input = data;
        length = set_symbol(symbol, idx, !is_composite /*chk_sane*/, 1 /*no_eci*/, &input, size);
        if (!length) {
            continue;
        }

        if (is_composite) {
            if (idx == BARCODE_GS1_128_CC || idx == BARCODE_DBAR_EXP_CC || idx == BARCODE_DBAR_EXPSTK_CC) {
                strcpy(symbol->primary, primary_ai);
            } else if (idx == BARCODE_UPCE_CC) {
                strcpy(symbol->primary, primary_upce);
            } else {
                strcpy(symbol->primary, primary);
            }
        }

        /* Try it first without GS1NOCHECK_MODE */
        symbol->input_mode = (symbol->input_mode & ~0x07) | GS1_MODE;

        ret = ZBarcode_Encode(symbol, input, length);
        assert(ret != ZINT_ERROR_ENCODING_PROBLEM);

        ZBarcode_Clear(symbol);

        /* Now with GS1NOCHECK_MODE */
        symbol->input_mode = (symbol->input_mode & ~0x07) | GS1_MODE | GS1NOCHECK_MODE;
        symbol->input_mode &= ~GS1PARENS_MODE;

        memcpy(gs1_buf + 2, input, length);

        ret = ZBarcode_Encode(symbol, gs1_buf, length + 2);
        assert(ret != ZINT_ERROR_ENCODING_PROBLEM);
    }

    (void) free(gs1_buf);
    ZBarcode_Delete(symbol);

    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* vim: set ts=4 sw=4 et : */
