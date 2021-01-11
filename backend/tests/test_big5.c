/*
    libzint - the open source barcode library
    Copyright (C) 2021 Robin Stuart <rstuart114@gmail.com>

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

#include "testcommon.h"
#include "test_big5_tab.h"
#include "../big5.h"

// As control convert to Big5 using simple table generated from https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/OTHER/BIG5.TXT plus simple processing
static int big5_wctomb_zint2(unsigned int *r, unsigned int wc) {
    if (wc < 0x80) {
        return 0;
    }
    int tab_length = ARRAY_SIZE(test_big5_tab);
    int start_i = test_big5_tab_ind[wc >> 10];
    int end_i = start_i + 0x800 > tab_length ? tab_length : start_i + 0x800;
    for (int i = start_i; i < end_i; i += 2) {
        if (test_big5_tab[i + 1] == wc) {
            *r = test_big5_tab[i];
            return *r > 0xFF ? 2 : 1;
        }
    }
    return 0;
}

static void test_big5_wctomb_zint(void) {

    testStart("");

    int ret, ret2;
    unsigned int val, val2;

    for (unsigned int i = 0; i < 0xFFFE; i++) {
        if (i >= 0xD800 && i < 0xE000) { // UTF-16 surrogates
            continue;
        }
        val = val2 = 0;
        ret = big5_wctomb_zint(&val, i);
        ret2 = big5_wctomb_zint2(&val2, i);
        assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", i, i, ret, ret2, val, val2);
        if (ret2) {
            assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", i, i, val, val2);
        }
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_big5_wctomb_zint", test_big5_wctomb_zint, 0, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
