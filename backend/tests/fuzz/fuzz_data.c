/*  fuzz_data.c - fuzzer for libzint (DATA_MODE, all symbologies except GS1_128, DBAR_EXP/STK & composites) */
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
    struct zint_symbol *symbol;
    int idx;

    /* Ignore empty or very large input */
    if (size < 1 || size > 10000) {
        return 0;
    }

    symbol = ZBarcode_Create();
    assert(symbol);

    for (idx = 0; idx < ZARRAY_SIZE(settings); idx++) {
        const unsigned char *input;
        int length;
        int ret;

        if (!ZBarcode_ValidID(idx)) {
            continue;
        }
        if (idx == BARCODE_GS1_128 || idx == BARCODE_DBAR_EXP || idx == BARCODE_DBAR_EXPSTK
                || (ZBarcode_Cap(idx, ZINT_CAP_COMPOSITE) & ZINT_CAP_COMPOSITE)) {
            continue;
        }

        input = data;
        length = set_symbol(symbol, idx, 1 /*chk_sane*/, 0 /*no_eci*/, &input, size);
        if (!length) {
            continue;
        }

        ret = ZBarcode_Encode(symbol, input, length);
        assert(ret != ZINT_ERROR_ENCODING_PROBLEM);
    }

    ZBarcode_Delete(symbol);

    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* vim: set ts=4 sw=4 et : */
