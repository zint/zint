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
#include "test_ksx1001_tab.h"
#include "../ksx1001.h"

// As control convert to KS X 1001 using simple table generated from https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/KSC/KSX1001.TXT plus simple processing
static int ksx1001_wctomb_zint2(unsigned int *r, unsigned int wc) {
    int tab_length, start_i, end_i;
    int i;

    if (wc < 0x80) {
        return 0;
    }
    if (wc == 0x20AC) { // Euro sign added KS X 1001:1998
        *r = 0x2266;
        return 2;
    }
    if (wc == 0xAE) { // Registered trademark added KS X 1001:1998
        *r = 0x2267;
        return 2;
    }
    if (wc == 0x327E) { // Korean postal code symbol added KS X 1001:2002
        *r = 0x2268;
        return 2;
    }
    tab_length = ARRAY_SIZE(test_ksx1001_tab);
    start_i = test_ksx1001_tab_ind[wc >> 10];
    end_i = start_i + 0x800 > tab_length ? tab_length : start_i + 0x800;
    for (i = start_i; i < end_i; i += 2) {
        if (test_ksx1001_tab[i + 1] == wc) {
            *r = test_ksx1001_tab[i];
            return *r > 0xFF ? 2 : 1;
        }
    }
    return 0;
}

static void test_ksx1001_wctomb_zint(void) {

    int ret, ret2;
    unsigned int val, val2;
    unsigned i;

    testStart("test_ksx1001_wctomb_zint");

    for (i = 0; i < 0xFFFE; i++) {
        if (i >= 0xD800 && i <= 0xDFFF) { // UTF-16 surrogates
            continue;
        }
        val = val2 = 0;
        ret = ksx1001_wctomb_zint(&val, i);
        ret2 = ksx1001_wctomb_zint2(&val2, i);
        assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", (int) i, i, ret, ret2, val, val2);
        if (ret2) {
            assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", (int) i, i, val, val2);
        }
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_ksx1001_wctomb_zint", test_ksx1001_wctomb_zint, 0, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
