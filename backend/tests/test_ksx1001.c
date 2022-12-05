/*
    libzint - the open source barcode library
    Copyright (C) 2021-2022 Robin Stuart <rstuart114@gmail.com>

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
#include "test_ksx1001_tab.h"
#include "../ksx1001.h"
/* For local "private" testing using previous libiconv adaptation, not included for licensing reasons */
#if 0
#define TEST_JUST_SAY_GNO
#endif
#ifdef TEST_JUST_SAY_GNO
#include "../just_say_gno/ksx1001_gnu.h"
#endif

INTERNAL int u_ksx1001_test(const unsigned int u, unsigned char *dest);

/* Version of `u_ksx1001()` taking unsigned int destination for backward-compatible testing */
static int u_ksx1001_int(const unsigned int u, unsigned int *d) {
    unsigned char dest[2];
    int ret = u_ksx1001_test(u, dest);
    if (ret) {
        *d = ret == 1 ? dest[0] : ((dest[0] << 8) | dest[1]);
    }
    return ret;
}

/* As control convert to KS X 1001 using simple table generated from
   https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/KSC/KSX1001.TXT plus simple processing
*/
static int u_ksx1001_int2(unsigned int u, unsigned int *dest) {
    int tab_length, start_i, end_i;
    int i;

    if (u < 0x80) {
        *dest = u;
        return 1;
    }
    if (u == 0x20AC) { /* Euro sign added KS X 1001:1998 */
        *dest = 0x2266 + 0x8080;
        return 2;
    }
    if (u == 0xAE) { /* Registered trademark added KS X 1001:1998 */
        *dest = 0x2267 + 0x8080;
        return 2;
    }
    if (u == 0x327E) { /* Korean postal code symbol added KS X 1001:2002 */
        *dest = 0x2268 + 0x8080;
        return 2;
    }
    tab_length = ARRAY_SIZE(test_ksx1001_tab);
    start_i = test_ksx1001_tab_ind[u >> 10];
    end_i = start_i + 0x800 > tab_length ? tab_length : start_i + 0x800;
    for (i = start_i; i < end_i; i += 2) {
        if (test_ksx1001_tab[i + 1] == u) {
            *dest = test_ksx1001_tab[i] + 0x8080;
            return 2;
        }
    }
    return 0;
}

#include <time.h>

#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)
#define TEST_PERF_RATIO(a1, a2) (a2 ? TEST_PERF_TIME(a1) / TEST_PERF_TIME(a2) : 0)

#ifdef TEST_JUST_SAY_GNO
#define TEST_INT_PERF_ITERATIONS    100
#endif

static void test_u_ksx1001_int(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    int ret, ret2;
    unsigned int val, val2;
    unsigned i;

#ifdef TEST_JUST_SAY_GNO
    int j;
    clock_t start;
    clock_t total = 0, total_gno = 0;
#else
    (void)debug;
#endif

    testStart("test_u_ksx1001_int");

#ifdef TEST_JUST_SAY_GNO
    if ((debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        printf("test_u_ksx1001_int perf iterations: %d\n", TEST_INT_PERF_ITERATIONS);
    }
#endif

    for (i = 0; i < 0xFFFE; i++) {
        if (i >= 0xD800 && i <= 0xDFFF) { /* UTF-16 surrogates */
            continue;
        }
        if (testContinue(p_ctx, i)) continue;
        val = val2 = 0;
        ret = u_ksx1001_int(i, &val);
        ret2 = u_ksx1001_int2(i, &val2);
        assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", (int) i, i, ret, ret2, val, val2);
        if (ret2) {
            assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", (int) i, i, val, val2);
        }
#ifdef TEST_JUST_SAY_GNO
        if (i >= 0x80) { /* `ksx1001_wctomb_zint()` doesn't handle ASCII */
            if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
                val2 = 0;
                ret2 = ksx1001_wctomb_zint(&val2, i);
            } else {
                for (j = 0; j < TEST_INT_PERF_ITERATIONS; j++) {
                    val = val2 = 0;

                    start = clock();
                    ret = u_ksx1001_int(i, &val);
                    total += clock() - start;

                    start = clock();
                    ret2 = ksx1001_wctomb_zint(&val2, i);
                    total_gno += clock() - start;
                }
            }

            assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", (int) i, i, ret, ret2, val, val2);
            if (ret2) {
                val2 += 0x8080; /* `ksx1001_wctomb_zint()` returns pure KS X 1001 values, convert to EUC-KR */
                assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", (int) i, i, val, val2);
            }
        }
#endif
    }

#ifdef TEST_JUST_SAY_GNO
    if ((debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        printf("test_u_ksx1001_int perf totals: new % 8gms, gno % 8gms ratio %g\n",
                TEST_PERF_TIME(total), TEST_PERF_TIME(total_gno), TEST_PERF_RATIO(total, total_gno));
    }
#endif

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_u_ksx1001_int", test_u_ksx1001_int },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
