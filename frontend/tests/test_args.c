/*
    libzint - the open source barcode library
    Copyright (C) 2020-2026 Robin Stuart <rstuart114@gmail.com>

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

#ifndef _WIN32
#include <sys/wait.h> /* For WIFEXITED/WEXITSTATUS */
#endif

#include "testcommon.h"

#ifdef _WIN32
/* Hacks to stop popen() mangling input on Windows */
static int utf8_to_wchar(const char *str, wchar_t *out) {
    unsigned int codepoint, state = 0;

    while (*str) {
        do {
            z_decode_utf8(&state, &codepoint, *str++);
        } while (*str && state != 0 && state != 12);
        if (state != 0) {
            return 0;
        }
        *out++ = codepoint;
    }
    *out = L'\0';

    return 1;
}

static int escape_cmd(const char *str, char *buf) {
    int ret = 0;
    char *out = buf;
    const unsigned char *ustr;

    for (ustr = (const unsigned char *) str; *ustr; ustr++) {
        if (*ustr >= 0x80 || *ustr < 0x20 || *ustr == '\\') {
            sprintf(out, "\\x%02X", *ustr);
            out += 4;
            ret = 1;
        } else {
            *out++ = *ustr;
        }
    }
    *out = '\0';
    if (ret) {
        if (out - buf > 5 && strcmp(out - 5, " 2>&1") == 0) {
            strcpy(out - 5, " --esc 2>&1");
        } else {
            strcpy(out, " --esc");
        }
    }

    return ret;
}
#endif

static char *exec(const char *cmd, char *buf, int buf_size, int debug, int index, int *p_exit_status) {
    FILE *fp;
    int cnt;
    int exit_status;
#ifdef _WIN32
    wchar_t wchar_cmd[8192];
    char esc_cmd[16384];
    int is_binary = strstr(cmd, " --binary") != NULL;
    int is_escaped = strstr(cmd, " --esc") != NULL;
#endif

    if (debug & ZINT_DEBUG_TEST_PRINT) printf("%d: %s\n", index, cmd);

    *buf = '\0';

#ifdef _WIN32
    if (!is_binary && utf8_to_wchar(cmd, wchar_cmd)) {
        fp = _wpopen(wchar_cmd, L"r");
    } else if (!is_escaped && is_binary && escape_cmd(cmd, esc_cmd)) {
        fp = testutil_popen(esc_cmd, "r");
    } else {
        fp = testutil_popen(cmd, "r");
    }
#else
    fp = testutil_popen(cmd, "r");
#endif
    if (!fp) {
        fprintf(stderr, "exec: failed to run '%s'\n", cmd);
        return NULL;
    }
    cnt = (int) fread(buf, 1, buf_size, fp);
    if (fgetc(fp) != EOF) {
        fprintf(stderr, "exec: failed to read full stream (%s)\n", cmd);
        exit_status = testutil_pclose(fp);
        if (p_exit_status) {
#ifndef _WIN32
            if (WIFEXITED(exit_status)) {
                exit_status = WEXITSTATUS(exit_status);
            }
#endif
            *p_exit_status = exit_status;
        }
        return NULL;
    }
    exit_status = testutil_pclose(fp);
    if (p_exit_status) {
#ifndef _WIN32
        if (WIFEXITED(exit_status)) {
            exit_status = WEXITSTATUS(exit_status);
        }
#endif
        *p_exit_status = exit_status;
    }

    if (cnt) {
        if (buf[cnt - 1] == '\r' || buf[cnt - 1] == '\n') {
            buf[cnt - 1] = '\0';
            if (cnt > 1 && (buf[cnt - 2] == '\r' || buf[cnt - 2] == '\n')) {
                buf[cnt - 2] = '\0';
            }
        }
    }

    return buf;
}

static void arg_int(char *cmd, const char *opt, int val) {
    if (val != -1) {
        sprintf(cmd + strlen(cmd), "%s%s%d", strlen(cmd) ? " " : "", opt, val);
    }
}

static void arg_bool(char *cmd, const char *opt, int val) {
    if (val == 1) {
        sprintf(cmd + strlen(cmd), "%s%s", strlen(cmd) ? " " : "", opt);
    }
}

static void arg_double(char *cmd, const char *opt, double val) {
    if (val != -1) {
        sprintf(cmd + strlen(cmd), "%s%s%.8f", strlen(cmd) ? " " : "", opt, val);
    }
}

static void arg_data(char *cmd, const char *opt, const char *data) {
    if (data != NULL) {
        sprintf(cmd + strlen(cmd), "%s%s\"%s\"", strlen(cmd) ? " " : "", opt, data);
    }
}

static void arg_seg(char *cmd, const char *opt, const char *data, const int eci) {
    if (data != NULL) {
        sprintf(cmd + strlen(cmd), "%s%s%d,\"%s\"", strlen(cmd) ? " " : "", opt, eci, data);
    } else {
        sprintf(cmd + strlen(cmd), "%s%s%d", strlen(cmd) ? " " : "", opt, eci);
    }
}

static int arg_input(char *cmd, const char *filename, const char *input) {
    FILE *fp;
    int cnt;
    if (input != NULL) {
        if (strcmp(input, "-") != 0) {
            fp = testUtilOpen(filename, "wb");
            if (!fp) {
                fprintf(stderr, "arg_input: failed to open '%s' for writing\n", filename);
                return 0;
            }
            cnt = (int) fwrite(input, 1, strlen(input), fp);
            if (cnt != (int) strlen(input)) {
                fprintf(stderr, "arg_input: failed to write %d bytes, cnt %d written (%s)\n",
                        (int) strlen(input), cnt, filename);
                fclose(fp);
                return 0;
            }
            fclose(fp);
        }
        sprintf(cmd + strlen(cmd), "%s-i \"%s\"", strlen(cmd) ? " " : "", filename);
        return 1;
    }
    return 0;
}

static void arg_input_mode(char *cmd, int input_mode) {
    if (input_mode != -1) {
        if ((input_mode & 0x07) == DATA_MODE) {
            sprintf(cmd + strlen(cmd), "%s--binary", strlen(cmd) ? " " : "");
        } else if ((input_mode & 0x07) == GS1_MODE) {
            sprintf(cmd + strlen(cmd), "%s--gs1", strlen(cmd) ? " " : "");
        }
        if (input_mode & ESCAPE_MODE) {
            sprintf(cmd + strlen(cmd), "%s--esc", strlen(cmd) ? " " : "");
        }
        if (input_mode & EXTRA_ESCAPE_MODE) {
            sprintf(cmd + strlen(cmd), "%s--extraesc", strlen(cmd) ? " " : "");
        }
        if (input_mode & FAST_MODE) {
            sprintf(cmd + strlen(cmd), "%s--fast", strlen(cmd) ? " " : "");
        }
        if (input_mode & GS1PARENS_MODE) {
            sprintf(cmd + strlen(cmd), "%s--gs1parens", strlen(cmd) ? " " : "");
        }
        if (input_mode & GS1NOCHECK_MODE) {
            sprintf(cmd + strlen(cmd), "%s--gs1nocheck", strlen(cmd) ? " " : "");
        }
        if (input_mode & GS1SYNTAXENGINE_MODE) {
            sprintf(cmd + strlen(cmd), "%s--gs1strict", strlen(cmd) ? " " : "");
        }
        if (input_mode & HEIGHTPERROW_MODE) {
            sprintf(cmd + strlen(cmd), "%s--heightperrow", strlen(cmd) ? " " : "");
        }
        if (input_mode & GS1RAW_MODE) {
            sprintf(cmd + strlen(cmd), "%s--gs1raw", strlen(cmd) ? " " : "");
        }
    }
}

static void arg_output_options(char *cmd, int output_options) {
    if (output_options != -1) {
        if (output_options & BARCODE_BIND) {
            sprintf(cmd + strlen(cmd), "%s--bind", strlen(cmd) ? " " : "");
        }
        if (output_options & BARCODE_BIND_TOP) {
            sprintf(cmd + strlen(cmd), "%s--bindtop", strlen(cmd) ? " " : "");
        }
        if (output_options & BARCODE_BOX) {
            sprintf(cmd + strlen(cmd), "%s--box", strlen(cmd) ? " " : "");
        }
        if (output_options & BARCODE_STDOUT) {
            sprintf(cmd + strlen(cmd), "%s--direct", strlen(cmd) ? " " : "");
        }
        if (output_options & READER_INIT) {
            sprintf(cmd + strlen(cmd), "%s--init", strlen(cmd) ? " " : "");
        }
        if (output_options & SMALL_TEXT) {
            sprintf(cmd + strlen(cmd), "%s--small", strlen(cmd) ? " " : "");
        }
        if (output_options & BOLD_TEXT) {
            sprintf(cmd + strlen(cmd), "%s--bold", strlen(cmd) ? " " : "");
        }
        if (output_options & CMYK_COLOUR) {
            sprintf(cmd + strlen(cmd), "%s--cmyk", strlen(cmd) ? " " : "");
        }
        if (output_options & BARCODE_DOTTY_MODE) {
            sprintf(cmd + strlen(cmd), "%s--dotty", strlen(cmd) ? " " : "");
        }
        if (output_options & GS1_GS_SEPARATOR) {
            sprintf(cmd + strlen(cmd), "%s--gssep", strlen(cmd) ? " " : "");
        }
        if (output_options & BARCODE_QUIET_ZONES) {
            sprintf(cmd + strlen(cmd), "%s--quietzones", strlen(cmd) ? " " : "");
        }
        if (output_options & BARCODE_NO_QUIET_ZONES) {
            sprintf(cmd + strlen(cmd), "%s--noquietzones", strlen(cmd) ? " " : "");
        }
        if (output_options & COMPLIANT_HEIGHT) {
            sprintf(cmd + strlen(cmd), "%s--compliantheight", strlen(cmd) ? " " : "");
        }
    }
}

/* Tests args that can be detected with `--dump` */
static void test_dump_args(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        const char *data;
        const char *data2;
        const char *input;
        const char *input2;
        int input_mode;
        int output_options;
        int batch;
        int cols;
        int dmb256;
        int dmc40;
        int dmre;
        int eci;
        int fullmultibyte;
        int mask;
        int mode;
        const char *primary;
        int rows;
        int secure;
        int square;
        int vers;

        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ {              -1, "123", NULL, NULL, NULL,        -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D3 96 72 F7 65 C9 61 8E B" },
        /*  1*/ { BARCODE_CODE128, "123", NULL, NULL, NULL,        -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D3 96 72 F7 65 C9 61 8E B" },
        /*  2*/ { BARCODE_CODE128, "123", "456", NULL, NULL,       -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D3 96 72 F7 65 C9 61 8E B\nD3 97 62 F7 67 49 19 8E B" },
        /*  3*/ { BARCODE_CODE128, "123", NULL, NULL, NULL,        -1, -1, 1, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "Warning 141: Can't use batch mode if data given, **IGNORED**\nD3 96 72 F7 65 C9 61 8E B" },
        /*  4*/ { BARCODE_CODE128, NULL, NULL, "123\n45\n", NULL,  -1, -1, 1, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D3 96 72 F7 65 C9 61 8E B\nD3 97 62 3B 63 AC" },
        /*  5*/ { BARCODE_CODE128, NULL, NULL, "123\n45\n", "7\n", -1, -1, 1, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "Warning 144: First input file 'test_dump_args1.txt' only processed, subsequent input files **IGNORED**\nD3 96 72 F7 65 C9 61 8E B\nD3 97 62 3B 63 AC" },
        /*  6*/ { BARCODE_CODE128, "\t", NULL, NULL, NULL,         -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 90 D2 1A 63 AC" },
        /*  7*/ { BARCODE_CODE128, "\\t", NULL, NULL, NULL, ESCAPE_MODE, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 90 D2 1A 63 AC" },
        /*  8*/ { BARCODE_CODE128, "\\^Ab\011", NULL, NULL, NULL, EXTRA_ESCAPE_MODE, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 9E 8A 43 43 48 D1 8E B" },
        /*  9*/ { BARCODE_CODE128, "\\^Ab\\t", NULL, NULL, NULL, ESCAPE_MODE | EXTRA_ESCAPE_MODE, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 9E 8A 43 43 48 D1 8E B" },
        /* 10*/ { BARCODE_CODE128, "123", NULL, NULL, NULL, -1, BARCODE_BIND | BARCODE_BOX | BARCODE_BIND_TOP | SMALL_TEXT | BOLD_TEXT | CMYK_COLOUR, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D3 96 72 F7 65 C9 61 8E B" },
        /* 11*/ { BARCODE_CODE128, "123", NULL, NULL, NULL, -1, BARCODE_DOTTY_MODE, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "Error 224: Selected symbology cannot be rendered as dots" },
        /* 12*/ { BARCODE_CODE128, "123", NULL, NULL, NULL,        -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, "1", -1, -1, 0, -1, "Warning 162: Primary '1' given but not MaxiCode or GS1 Composite, **IGNORED**\nD3 96 72 F7 65 C9 61 8E B" },
        /* 13*/ { BARCODE_CODABLOCKF, "ABCDEF", NULL, NULL, NULL,  -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 97 BA 86 51 88 B1 11 AC 46 D8 C7 58\nD0 97 BB 12 46 88 C5 1A 3C 55 CC C7 58" },
        /* 14*/ { BARCODE_CODABLOCKF, "ABCDEF", NULL, NULL, NULL,  -1, -1, 0, 10, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 97 BA 86 51 88 B1 11 AC 44 68 BC 98 EB\nD0 97 BB 12 46 2B BD 7B A3 47 8A 8D 18 EB" },
        /* 15*/ { BARCODE_CODABLOCKF, "ABCDEF", NULL, NULL, NULL,  -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL,  3, -1, 0, -1, "D0 97 BA 58 51 88 B1 11 AC 46 36 C7 58\nD0 97 BB 12 46 88 C5 77 AF 74 62 C7 58\nD0 97 BA CE 5D EB DD 1A 3C 56 88 C7 58" },
        /* 16*/ { BARCODE_PDF417, "ABCDEF", NULL, NULL, NULL,  -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL,  3, -1, 0, -1, "FF 54 75 70 3D 4F 1E AF 0A 73 86 82 C3 A9 C1 FD 14 8\nFF 54 7A 84 39 76 1F 6A 0F D2 E7 26 0B EA C1 FD 14 8\nFF 54 6A 7C 20 C5 D0 CD E8 76 E6 1D 72 A1 E1 FD 14 8" },
        /* 17*/ { BARCODE_CODE16K, "ABCDEF", NULL, NULL, NULL,  -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL,  3, -1, 0, -1, "E5 73 6B 9D D3 BB 94 EE 34\nCD 72 EE 74 BD 97 B2 F6 64\nD9 2F 65 EC BD BC B4 8E 4C" },
        /* 18*/ { BARCODE_CODE49, "ABCDEF", NULL, NULL, NULL,  -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL,  3, -1, 0, -1, "BF 6C A7 B0 A1 10 B2 C1 BC\nB3 CB AB BF 33 CB AE 41 BC\nB3 CB BA 0B AA 38 34 7B BC" },
        /* 19*/ { BARCODE_CODE11, NULL, NULL, "123", NULL,         -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "B2 D6 96 CA B5 6D 64" },
        /* 20*/ { BARCODE_CODE11, NULL, NULL, "123", NULL,         -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "B2 D6 96 CA B5 6D 64" },
        /* 21*/ { BARCODE_CODE11, NULL, NULL, "123", NULL,         -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0,  1, "B2 D6 96 CA B5 64" },
        /* 22*/ { BARCODE_CODE11, "123", NULL, "456", NULL,        -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0,  2, "B2 D6 96 CA B2\nB2 B6 DA 9A B2" },
        /* 23*/ { BARCODE_CODE11, "123", "456", "789", "012",      -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0,  2, "B2 D6 96 CA B2\nB2 B6 DA 9A B2\nB2 A6 D2 D5 64\nB2 AD AD 2D 64" },
        /* 24*/ { BARCODE_PDF417, "123", NULL, NULL, NULL,         -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1,  0, 0, -1, "FF 54 7A BC 3A 9C 1D 5C 0F E8 A4\nFF 54 7E AE 3C 11 5F AB 8F E8 A4\nFF 54 6A F8 29 9F 1D 5F 8F E8 A4\nFF 54 57 9E 37 BA 1A F7 CF E8 A4\nFF 54 75 CC 36 F0 5D 73 0F E8 A4" },
        /* 25*/ { BARCODE_DATAMATRIX, "ABC", NULL, NULL, NULL,     -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA 8\nB3 4\n8F 0\nB2 C\nA6 0\nBA C\nD6 0\nEB 4\nE2 8\nFF C" },
        /* 26*/ { BARCODE_DATAMATRIX, "ABC", NULL, NULL, NULL, -1, READER_INIT, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A\nAC 7\n8A 4\nA0 3\nC2 2\nB5 1\n82 2\nBA 7\n8C C\nA0 5\n86 A\nFF F" },
        /* 27*/ { BARCODE_DATAMATRIX, "ABCDEFGH", NULL, NULL, NULL, FAST_MODE, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nA6 8C\nB2 F0\n98 B4\nB9 A8\nB8 CC\nF0 78\nA0 3C\n99 70\n85 1C\nDA B0\nE5 94\nA7 50\nFF FC" },
        /* 28*/ { BARCODE_DATAMATRIX, "ABCDEFGH", NULL, NULL, NULL, -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\n80 04\n82 60\nC5 24\n98 A8\nA3 9C\nCB B8\nAF DC\n86 58\nF6 44\nAC 18\n90 54\nCF 30\nFF FC" },
        /* 29*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJK", NULL, NULL, NULL, -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA AA AA\n80 09 F9 BD\n82 4A E2 58\nC5 CD C9 A5\nD8 5C A5 FC\nE0 35 88 69\nCC FC B3 E6\nFF FF FF FF" },
        /* 30*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJK", NULL, NULL, NULL, -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 1, -1, "AA AA\n80 25\n82 24\nC5 5D\n98 90\nA4 C7\nC8 A6\nB9 E9\n8E 02\nDE 91\nCD 6C\nA0 BB\n85 80\n98 2D\nE4 CA\nFF FF" },
        /* 31*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA A8\nBA 5A 44\n8B 4D 28\nBF 77 64\n97 85 50\nBA D8 AC\nCD ED B8\nD4 B5 2C\nD1 A8 00\n81 FB 2C\nE4 75 78\n96 E8 2C\nF3 75 78\nEE 1D 04\nCA BA 98\nB1 8F B4\nA0 4F 00\nE4 A7 74\nF1 D3 90\nEF E1 BC\n91 10 38\nFF FF FC" },
        /* 32*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, -1, -1, 1, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA AA AA AA AA AA AA\nBA 03 BA 7D E5 31 B0 0D\n8B 6A 93 B6 E0 0A B8 3C\nBF 1D EA A7 EB ED A1 FB\n96 66 86 B6 C9 AE 92 40\nBF 65 E7 95 BC B7 FA E3\nCC 7C 90 CC D1 24 AB 5A\nFF FF FF FF FF FF FF FF" },
        /* 33*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, -1, -1, 1, -1, 0, -1, -1, NULL, -1, -1, 1, -1, "Warning 157: Previous '--dmre' overwritten by '--square'\nAA AA A8\nBA 5A 44\n8B 4D 28\nBF 77 64\n97 85 50\nBA D8 AC\nCD ED B8\nD4 B5 2C\nD1 A8 00\n81 FB 2C\nE4 75 78\n96 E8 2C\nF3 75 78\nEE 1D 04\nCA BA 98\nB1 8F B4\nA0 4F 00\nE4 A7 74\nF1 D3 90\nEF E1 BC\n91 10 38\nFF FF FC" },
        /* 34*/ { BARCODE_DATAMATRIX, "[91]12[92]34", NULL, NULL, NULL, GS1_MODE, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nFA 9C\nBC 00\nD7 84\nED E0\nA4 E4\nA7 40\n9D 3C\nBF 50\nFA 24\nB1 68\nE5 04\n92 70\nFF FC" },
        /* 35*/ { BARCODE_DATAMATRIX, "[91]12[92]34", NULL, NULL, NULL, GS1_MODE, GS1_GS_SEPARATOR, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 36*/ { BARCODE_DATAMATRIX, "[9\\x31]12[92]34", NULL, NULL, NULL, GS1_MODE | ESCAPE_MODE, GS1_GS_SEPARATOR, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 37*/ { BARCODE_DATAMATRIX, "(9\\x31)12(92)34", NULL, NULL, NULL, GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, GS1_GS_SEPARATOR, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 38*/ { BARCODE_DATAMATRIX, "abc", NULL, NULL, NULL,    -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA 8\nB3 C\nC6 0\nBA C\nBD 0\nB4 C\nDC 0\nEB C\nD1 8\nFF C" },
        /* 39*/ { BARCODE_DATAMATRIX, "abc", NULL, NULL, NULL,    -1, -1, 0, -1,  0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A\n8B F\nD4 C\nC2 7\n9E C\nCF 3\n8E 8\nBB F\n86 2\n95 D\nCB A\nFF F" },
        /* 40*/ { BARCODE_DATAMATRIX, "abc", NULL, NULL, NULL,    -1, -1, 0, -1, -1,  1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A\n8C F\n9E 4\nC5 7\nA9 6\n9F F\n97 0\nFA 9\nAA C\nDD D\nD2 2\nFF F" },
        /* 41*/ { BARCODE_DATAMATRIX, "abc", NULL, NULL, NULL,    -1, -1, 0, -1,  0,  1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "Warning 161: Previous '--dmb256' overwritten by '--dmc40'\nAA A\n8C F\n9E 4\nC5 7\nA9 6\n9F F\n97 0\nFA 9\nAA C\nDD D\nD2 2\nFF F" },
        /* 42*/ { BARCODE_DATAMATRIX, "A\\^1BC\\^^1DEF", NULL, NULL, NULL, EXTRA_ESCAPE_MODE, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA AA AA\nE0 B9 88 59\nD0 08 A3 14\n8D 9D DD DB\n8E 3E E5 E8\nDF 3B A5 A1\nB6 20 A6 02\nFF FF FF FF" },
        /* 43*/ { BARCODE_EANX_CC, "[91]12", NULL, NULL, NULL,    -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, "12345678+12", -1, -1, 0, -1, "DB BC D3 9C 44 E9 D2 2C 19 E7 A2 D8 A0 00 00 00\nDB 31 1C 9C C7 29 92 47 D9 E9 40 C8 A0 00 00 00\nDA 3B EB 10 AF 09 9A 18 9D 7D 82 E8 A0 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n20 00 00 00 00 00 00 00 00 00 00 00 20 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n14 68 D1 A6 49 BD 55 C9 D4 22 48 B9 40 59 94 98" },
        /* 44*/ { BARCODE_EANX_CC, "[91]12", NULL, NULL, NULL,    -1, -1, 0, -1, -1, -1, 0, -1, 0, -1,  2, "12345678+12", -1, -1, 0, -1, "D3 A3 E9 DB F5 C9 DB 43 D9 CB 98 D2 20 00 00 00\nD3 25 0F 11 E4 49 D3 51 F1 AC FC D6 20 00 00 00\nD1 33 48 19 39 E9 93 18 49 D8 98 D7 20 00 00 00\nD1 A6 FC DA 1C 49 9B C5 05 E2 84 D7 A0 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n20 00 00 00 00 00 00 00 00 00 00 00 20 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n14 68 D1 A6 49 BD 55 C9 D4 22 48 B9 40 59 94 98" },
        /* 45*/ { BARCODE_EANX_CC, "[91]12", NULL, NULL, NULL,    -1, -1, 0, -1, -1, -1, 0, -1, 0, -1,  2, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678", -1, -1, 0, -1, "Warning 115: Primary data string too long (127 character maximum), **TRUNCATING**\nError 449: Input length 127 wrong (linear component)" },
        /* 46*/ { BARCODE_MAXICODE, "abc", NULL, NULL, NULL,    -1, -1, 0, -1, -1, -1, 0, -1, 0, -1,  -1, "1234567", -1, -1, 0, -1, "C0 55 55 5C\nC0 00 00 08\nE7 AA AA A0\n55 55 55 58\n00 00 00 0C\nAA AA AA A8\n55 55 55 50\n00 00 00 08\nAA AA AA A4\n55 B1 35 50\n00 90 08 0C\nAA C0 52 A0\n54 40 05 58\n01 00 08 00\nAA C0 0A A0\n55 00 01 58\n02 80 08 08\nAB 80 1E A8\n54 00 05 54\n03 80 0C 00\nA8 00 02 AC\n55 40 25 50\n00 60 40 0C\nAA 60 66 A0\n55 55 53 B0\n00 00 07 A0\nAA AA AE 0C\nA3 CE E2 C8\nB3 F4 AB 20\n47 0F 3C 48\nD0 6C C5 D8\nE4 CD BF 18\n81 5F 38 58" },
        /* 47*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1,  1, 0, -1, "Warning 760: Converted to Shift JIS but no ECI specified\nFE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 48*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, -1, -1, 0, 26, 0, -1, -1, NULL, -1,  1, 0, -1, "FE 5B F8\n82 72 08\nBA DA E8\nBA 52 E8\nBA 2A E8\n82 0A 08\nFE AB F8\n00 D8 00\nEF F6 20\nB5 C2 28\n36 28 88\nFD 42 10\n62 2A C8\n00 95 70\nFE B7 38\n82 FD D8\nBA 97 00\nBA 43 60\nBA C8 C8\n82 C3 68\nFE EA F8" },
        /* 49*/ { BARCODE_QRCODE, "\223\137", NULL, NULL, NULL, DATA_MODE, -1, 0, -1, -1, -1, 0, -1, 0, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 0A 08\nBA A2 E8\nBA 0A E8\nBA 5A E8\n82 72 08\nFE AB F8\n00 A0 00\nEF AE 20\n75 B5 20\n82 F7 58\nF4 9D C8\n5E 17 28\n00 C2 20\nFE 88 80\n82 82 38\nBA EA A8\nBA 55 50\nBA D7 68\n82 BD D0\nFE B7 78" },
        /* 50*/ { BARCODE_QRCODE, "\223\137", NULL, NULL, NULL, DATA_MODE, -1, 0, -1, -1, -1, 0, -1, 1, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 51*/ { BARCODE_QRCODE, "\\x93\\x5F", NULL, NULL, NULL, DATA_MODE | ESCAPE_MODE, -1, 0, -1, -1, -1, 0, -1, 1, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 52*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, -1, -1, 0, -1, 0, 2, -1, NULL, -1,  1, 0, -1, "Warning 760: Converted to Shift JIS but no ECI specified\nFE 4B F8\n82 92 08\nBA 42 E8\nBA 92 E8\nBA 3A E8\n82 EA 08\nFE AB F8\n00 38 00\nFB CD 50\nA5 89 18\n0B 74 B8\nFC 81 A0\n92 34 B8\n00 DE 48\nFE AB 10\n82 5E 50\nBA C9 20\nBA C9 20\nBA F4 E0\n82 81 A0\nFE B4 E8" },
        /* 53*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, -1, -1, 0, -1, 1, -1, -1, NULL, -1, -1, 0, -1, "FE 8A FE\n80 28 02\nBE E8 FA\nA0 94 0A\nAE 3E EA\nAE D2 EA\nAE 74 EA\n00 AA 00\n15 B4 80\n0B 48 74\nA2 4A A4\nB5 56 2C\nA8 5A A8\n9F 18 50\n02 07 50\n00 A6 00\nFE 20 EA\n02 C2 EA\nFA C4 EA\n0A 42 0A\nEA 52 FA\nEA 24 02\nEA AA FE" },
        /* 54*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, -1, -1, 0, -1, 1, 3, -1, NULL, -1, -1, 0, -1, "FE 16 FE\n80 E2 02\nBE C2 FA\nA0 A0 0A\nAE F6 EA\nAE 98 EA\nAE BA EA\n00 E0 00\n15 83 80\n44 7E AE\n92 9C 78\n25 BF 08\n47 4B 8C\n0D F9 74\n03 E7 50\n00 3A 00\nFE C2 EA\n02 22 EA\nFA DA EA\n0A 22 0A\nEA B2 FA\nEA 9A 02\nEA E8 FE" },
        /* 55*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, -1, -1, 0, -1, 1, 4, -1, NULL, -1, -1, 0, -1, "FE 8A FE\n80 28 02\nBE E8 FA\nA0 94 0A\nAE 3E EA\nAE D2 EA\nAE 74 EA\n00 AA 00\n15 B4 80\n0B 48 74\nA2 4A A4\nB5 56 2C\nA8 5A A8\n9F 18 50\n02 07 50\n00 A6 00\nFE 20 EA\n02 C2 EA\nFA C4 EA\n0A 42 0A\nEA 52 FA\nEA 24 02\nEA AA FE" },
        /* 56*/ { BARCODE_TELEPEN, "ABC\020123", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, -1, -1, 0, -1, -1, -1, -1, NULL, -1, -1, 0, 1, "AB A8 BB B8 E3 B8 AE EA EE BA EA BA E2 22 BA B8 EA 2A" },
        /* 57*/ { BARCODE_TELEPEN_NUM, "123\020ABC", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, -1, -1, 0, -1, -1, -1, -1, NULL, -1, -1, 0, 1, "AA E8 EA BA E2 22 EE BA BB B8 E3 B8 AE EA BA B8 E8 AA" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];

    const char *input1_filename = "test_dump_args1.txt";
    const char *input2_filename = "test_dump_args2.txt";
    int have_input1;
    int have_input2;

    testStart("test_dump_args");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint --dump");
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        arg_int(cmd, "-b ", data[i].b);
        arg_data(cmd, "-d ", data[i].data);
        arg_data(cmd, "-d ", data[i].data2);
        have_input1 = arg_input(cmd, input1_filename, data[i].input);
        have_input2 = arg_input(cmd, input2_filename, data[i].input2);
        arg_input_mode(cmd, data[i].input_mode);
        arg_output_options(cmd, data[i].output_options);
        arg_bool(cmd, "--batch", data[i].batch);
        arg_int(cmd, "--cols=", data[i].cols);
        arg_int(cmd, "--dmb256=", data[i].dmb256);
        arg_int(cmd, "--dmc40=", data[i].dmc40);
        arg_bool(cmd, "--dmre", data[i].dmre);
        arg_int(cmd, "--eci=", data[i].eci);
        arg_bool(cmd, "--fullmultibyte", data[i].fullmultibyte);
        arg_int(cmd, "--mask=", data[i].mask);
        arg_int(cmd, "--mode=", data[i].mode);
        arg_data(cmd, "--primary=", data[i].primary);
        arg_int(cmd, "--rows=", data[i].rows);
        arg_int(cmd, "--secure=", data[i].secure);
        arg_bool(cmd, "--square", data[i].square);
        arg_int(cmd, "--vers=", data[i].vers);

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s) (%s)\n",
                    i, buf, data[i].expected, cmd);

        if (have_input1) {
            assert_zero(testUtilRemove(input1_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                        i, input1_filename, errno, strerror(errno));
        }
        if (have_input2) {
            assert_zero(testUtilRemove(input2_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                        i, input2_filename, errno, strerror(errno));
        }
    }

    testFinish();
}

/* Tests segs */
static void test_dump_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        int batch;
        const char *data;
        const char *data_seg1;
        const char *data_seg2;
        int eci;
        int eci_seg1;
        int eci_seg2;
        int num_seg2;

        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ {              -1, 0, "123", NULL, NULL, -1, -1, -1, -1, "D3 96 72 F7 65 C9 61 8E B" },
        /*  1*/ {              -1, 0, "123", NULL, NULL, -1, 3, -1, -1, "Error 166: Invalid segment argument, expect \"ECI,DATA\"" },
        /*  2*/ {              -1, 0, "123", "456", NULL, -1, -1, -1, -1, "Error 166: Invalid segment ECI (digits only)" },
        /*  3*/ {              -1, 0, "123", "456", NULL, -1, 1000000, -1, -1, "Error 166: Segment ECI code '1000000' out of range (0 to 999999)" },
        /*  4*/ {              -1, 0, "123", "456", NULL, -1, 3, -1, -1, "Error 775: Symbology does not support multiple segments" },
        /*  5*/ {   BARCODE_AZTEC, 0, "123", "456", NULL, -1, 3, -1, -1, "2B 7A\nC7 02\nF0 6E\n3F FE\n70 1C\nB7 D6\nB4 58\n15 54\n94 56\nB7 DC\n30 1A\n1F FC\n4C 66\n22 DA\n1E C6" },
        /*  6*/ {   BARCODE_AZTEC, 0, "123", NULL, "789", -1, -1, 3, -1, "Error 172: Segments must be consecutive - segment 1 missing" },
        /*  7*/ {   BARCODE_AZTEC, 1, "123", "456", NULL, -1, 3, -1, -1, "Warning 122: Can't define data in batch mode, **IGNORED** '123'\nWarning 165: Can't define segments in batch mode, **IGNORED** '3,456'\nWarning 124: No data received, no symbol generated" },
        /*  8*/ {   BARCODE_AZTEC, 0, "123", "456", "789", -1, 3, 4, 1, "Error 164: Duplicate segment 1" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];
    char seg2_buf[16];

    testStart("test_dump_segs");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint --dump");
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        arg_int(cmd, "-b ", data[i].b);

        if (data[i].batch) {
            strcat(cmd, " --batch");
        }
        if (data[i].data && data[i].data[0]) {
            arg_data(cmd, "-d ", data[i].data);
        }
        if (data[i].eci > 0) {
            arg_int(cmd, "--eci=", data[i].eci);
        }

        if (data[i].data_seg1 && data[i].data_seg1[0]) {
            arg_seg(cmd, "--seg1=", data[i].data_seg1, data[i].eci_seg1);
        } else if (data[i].eci_seg1 >= 0) {
            arg_seg(cmd, "--seg1=", NULL, data[i].eci_seg1);
        }

        sprintf(seg2_buf, "--seg%d=", data[i].num_seg2 != -1 ? data[i].num_seg2 : 2);
        if (data[i].data_seg2 && data[i].data_seg2[0]) {
            arg_seg(cmd, seg2_buf, data[i].data_seg2, data[i].eci_seg2);
        } else if (data[i].eci_seg2 >= 0) {
            arg_seg(cmd, seg2_buf, NULL, data[i].eci_seg2);
        }

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s) (%s)\n",
                    i, buf, data[i].expected, cmd);
    }

    testFinish();
}

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

#define TEST_INPUT_LONG "test_67890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" /* 250 chars */

#define TEST_MIRRORED_DIR_LONG "testdir_9012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789/" /* 220 chars */
#define TEST_MIRRORED_DIR_TOO_LONG "testdir_901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901/" /* 222 chars */

#ifndef _WIN32
#define TEST_INPUT_AMPERSAND_EXPECTED "***1.gif\000***2.gif"
#define TEST_INPUT_BSLASH   "\\"
#define TEST_INPUT_EXCLAIM  "!"
#define TEST_INPUT_DQUOTE   "\""
#define TEST_INPUT_ASTERISK "*"
#define TEST_INPUT_COLON    ":"
#define TEST_INPUT_LANGLE   "<"
#define TEST_INPUT_RANGLE   ">"
#define TEST_INPUT_QUESTION "?"
#define TEST_INPUT_PIPE     "|"
#else
#define TEST_INPUT_AMPERSAND_EXPECTED "+++1.gif\000+++2.gif"
#define TEST_INPUT_BSLASH   "_"
#define TEST_INPUT_EXCLAIM  "_"
#define TEST_INPUT_DQUOTE   "_"
#define TEST_INPUT_ASTERISK "_"
#define TEST_INPUT_COLON    "_"
#define TEST_INPUT_LANGLE   "_"
#define TEST_INPUT_RANGLE   "_"
#define TEST_INPUT_QUESTION "_"
#define TEST_INPUT_PIPE     "_"
#endif

    struct item {
        int b;
        int batch;
        int input_mode;
        int mirror;
        const char *opt;
        const char *filetype;
        const char *input_filename;
        const char *input;
        const char *outfile;

        int num_expected;
        const char *expected;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, 1, -1, 0, NULL, "gif", NULL, "123\n456\n", "", 2, "00001.gif\00000002.gif" },
        /*  1*/ { BARCODE_CODE128, 1, -1, 0, NULL, "gif", NULL, "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n", "~~~.gif", 10, "001.gif\000002.gif\000003.gif\000004.gif\000005.gif\000006.gif\000007.gif\000008.gif\000009.gif\000010.gif" },
        /*  2*/ { BARCODE_CODE128, 1, -1, 0, NULL, "gif", NULL, "123\n456\n", "@@@@.gif", 2, TEST_INPUT_AMPERSAND_EXPECTED },
        /*  3*/ { BARCODE_CODE128, 1, -1, 0, NULL, "gif", NULL, "123\n456\n789\n", "#####.gif", 3, "    1.gif\000    2.gif\000    3.gif" },
        /*  4*/ { BARCODE_CODE128, 1, -1, 0, NULL, "gif", NULL, "123\n456\n", "test_batch~.gif", 2, "test_batch1.gif\000test_batch2.gif" },
        /*  5*/ { BARCODE_CODE128, 1, -1, 1, NULL, "gif", NULL, "123\n456\n7890123456789\n", NULL, 3, "123.gif\000456.gif\0007890123456789.gif" },
        /*  6*/ { BARCODE_CODE128, 1, -1, 1, NULL, "gif", NULL, "123\n456\n7890123456789\n", "test_input_dir/", 3, "test_input_dir/123.gif\000test_input_dir/456.gif\000test_input_dir/7890123456789.gif" },
        /*  7*/ { BARCODE_CODE128, 1, -1, 1, NULL, "svg", NULL, "123\n456\n7890123456789\n", NULL, 3, "123.svg\000456.svg\0007890123456789.svg" },
        /*  8*/ { BARCODE_CODE128, 1, -1, 1, NULL, "gif", NULL, "123\n456\n7890123456789\nA\\xA0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A" TEST_INPUT_BSLASH "xA0B.gif" },
        /*  9*/ { BARCODE_CODE128, 1, ESCAPE_MODE, 1, NULL, "gif", NULL, "123\n456\n7890123456789\nA\\xA0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_B.gif" },
        /* 10*/ { BARCODE_CODE128, 1, -1, 1, NULL, "gif", NULL, "1!2\"3*\n/:45<6>\n?7890\\\\12345|6789\177\nA\\U0000A0B\n", NULL, 4, "1" TEST_INPUT_EXCLAIM "2" TEST_INPUT_DQUOTE "3" TEST_INPUT_ASTERISK ".gif\000_" TEST_INPUT_COLON "45" TEST_INPUT_LANGLE "6" TEST_INPUT_RANGLE ".gif\000" TEST_INPUT_QUESTION "7890" TEST_INPUT_BSLASH TEST_INPUT_BSLASH "12345" TEST_INPUT_PIPE "6789_.gif\000A" TEST_INPUT_BSLASH "U0000A0B.gif" },
        /* 11*/ { BARCODE_CODE128, 1, ESCAPE_MODE, 1, NULL, "gif", NULL, "!\"*\n/:45<6>\n?7890\\\\12345|6789\177\nA\\U0000A0B\n", NULL, 4, TEST_INPUT_EXCLAIM TEST_INPUT_DQUOTE TEST_INPUT_ASTERISK ".gif\000_" TEST_INPUT_COLON "45" TEST_INPUT_LANGLE "6" TEST_INPUT_RANGLE ".gif\000" TEST_INPUT_QUESTION "7890_12345" TEST_INPUT_PIPE "6789_.gif\000A_B.gif" },
        /* 12*/ { BARCODE_CODE128, 1, -1, 0, NULL, "gif", NULL, "\n", "test_batch.gif", 0, NULL },
        /* 13*/ { BARCODE_CODE128, 1, -1, 0, NULL, "gif", NULL, "123\n456\n", TEST_INPUT_LONG "~.gif", 2, TEST_INPUT_LONG "1.gif\000" TEST_INPUT_LONG "2.gif" },
        /* 14*/ { BARCODE_CODE128, 0, -1, 0, NULL, "svg", NULL, "123", TEST_INPUT_LONG "1.gif", 1, TEST_INPUT_LONG "1.svg" },
        /* 15*/ { BARCODE_CODE128, 1, -1, 0, NULL, "svg", NULL, "123\n", TEST_INPUT_LONG "1.gif", 1, TEST_INPUT_LONG "1.svg" },
        /* 16*/ { BARCODE_CODE128, 1, -1, 0, NULL, "gif", NULL, "123\n", "test_batch.jpeg", 1, "test_batch.gif" },
        /* 17*/ { BARCODE_CODE128, 1, -1, 0, NULL, "gif", NULL, "123\n", "test_batch.jpg", 1, "test_batch.gif" },
        /* 18*/ { BARCODE_CODE128, 1, -1, 0, NULL, "emf", NULL, "123\n", "test_batch.jpegg", 1, "test_batch.jpegg.emf" },
        /* 19*/ { BARCODE_CODE128, 1, -1, 0, NULL, "emf", NULL, "123\n", "test_batch.jpg", 1, "test_batch.emf" },
        /* 20*/ { BARCODE_CODE128, 1, -1, 0, NULL, "eps", NULL, "123\n", "test_batch.ps", 1, "test_batch.eps" },
        /* 21*/ { BARCODE_CODE128, 1, -1, 1, NULL, "gif", NULL, "1234567890123456789012345678901\n1234567890123456789012345678902\n", TEST_MIRRORED_DIR_LONG, 2, TEST_MIRRORED_DIR_LONG "1234567890123456789012345678901.gif\000" TEST_MIRRORED_DIR_LONG "1234567890123456789012345678902.gif" },
        /* 22*/ { BARCODE_CODE128, 1, -1, 1, NULL, "gif", NULL, "123\n456\n", TEST_MIRRORED_DIR_TOO_LONG, 2, "123.gif\000456.gif" },
        /* 23*/ { BARCODE_CODE128, 1, -1, 0, NULL, "gif", "testé_input.txt", "123\n456\n", "", 2, "00001.gif\00000002.gif" },
        /* 24*/ { BARCODE_CODE128, 1, -1, 0, NULL, "gif", "testก_input.txt", "123\n456\n", "test_input_δir/testé~~~.gif", 2, "test_input_δir/testé001.gif\000test_input_δir/testé002.gif" },
        /* 25*/ { BARCODE_CODE128, 0, -1, 1, NULL, NULL, "", "123", "out", 1, "123.XXX" },
        /* 26*/ { BARCODE_CODE128, 0, -1, 1, NULL, NULL, "", "123", "out.jpg", 1, "123.XXX" },
        /* 27*/ { BARCODE_CODE128, 0, -1, 1, NULL, "svg", "", "123", NULL, 1, "123.svg" },
        /* 28*/ { BARCODE_CODE128, 0, -1, 1, NULL, "svg", "", "!é?", NULL, 1, TEST_INPUT_EXCLAIM "é" TEST_INPUT_QUESTION ".svg" },
        /* 29*/ { BARCODE_CODE128, 1, -1, 1, " --scale=0.4", "gif", NULL, "123\n", NULL, 1, "123.gif" },
        /* 30*/ { BARCODE_QRCODE, 0, -1, 1, NULL, "svg", "", TEST_INPUT_LONG "1", NULL, 1, TEST_INPUT_LONG "1.svg" },
        /* 31*/ { BARCODE_QRCODE, 0, -1, 1, NULL, "svg", "", TEST_INPUT_LONG "12", NULL, 1, TEST_INPUT_LONG "1.svg" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[8192];

    const char *input_filename;
    const char *outfile;
    char outfile_buf[256] = {0};

    testStart("test_input");

    for (i = 0; i < data_size; i++) {
        int j;
        const char *slash;
        int expected_len;

        if (testContinue(p_ctx, i)) continue;
#ifdef _WIN32
        if (data[i].outfile && (int) strlen(data[i].outfile) > 50) {
            if (debug & ZINT_DEBUG_TEST_PRINT) {
                printf("%d not Windows compatible (outfile length %d > 50)\n", i, (int) strlen(data[i].outfile));
            }
            continue;
        }
        if (!data[i].outfile && data[i].num_expected == 1 && strlen(data[i].expected) > 50) {
            if (debug & ZINT_DEBUG_TEST_PRINT) {
                printf("%d not Windows compatible (expected length %d > 50)\n", i, (int) strlen(data[i].expected));
            }
            continue;
        }
#endif

        strcpy(cmd, "zint");
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        arg_int(cmd, "-b ", data[i].b);
        arg_bool(cmd, "--batch", data[i].batch);
        arg_input_mode(cmd, data[i].input_mode);
        arg_bool(cmd, "--mirror", data[i].mirror);
        if (data[i].opt) {
            strcat(cmd, data[i].opt);
        }
        arg_data(cmd, "--filetype=", data[i].filetype);
        input_filename = data[i].input_filename ? data[i].input_filename : "test_input.txt";
        if (*input_filename) {
            arg_input(cmd, input_filename, data[i].input);
        } else {
            arg_data(cmd, "-d ", data[i].input);
        }
        arg_data(cmd, "-o ", data[i].outfile);

        if (!data[i].expected
                || (data[i].batch && data[i].mirror && data[i].outfile && data[i].outfile[0]
                    && strcmp(data[i].outfile, TEST_MIRRORED_DIR_LONG) == 0)
                || (data[i].opt && strncmp(data[i].opt, " --scale", 8) == 0)) {
            printf("++++ Following %s expected, ignore: ", data[i].expected ? "warning" : "error");
            fflush(stdout);
        }
        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);

        outfile = data[i].expected;
        if (data[i].num_expected == 1 && (expected_len = (int) strlen(outfile)) > 3
                && strcmp(outfile + expected_len - 3, "XXX") == 0) {
            strcpy(outfile_buf, outfile);
            strcpy(outfile_buf + expected_len - 3, ZBarcode_NoPng() ? "gif" : "png");
            outfile = outfile_buf;
        }
        for (j = 0; j < data[i].num_expected; j++) {
            assert_nonzero(testUtilExists(outfile), "i:%d j:%d testUtilExists(%s) != 1\n", i, j, outfile);
            assert_zero(testUtilRemove(outfile), "i:%d j:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                        i, j, outfile, errno, strerror(errno));
            outfile += strlen(outfile) + 1;
        }

        if (*input_filename) {
            assert_zero(testUtilRemove(input_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                        i, input_filename, errno, strerror(errno));
        }

        /* Remove directory if any */
        if (data[i].outfile && (slash = strrchr(data[i].outfile, '/')) != NULL
                && strcmp(data[i].outfile, TEST_MIRRORED_DIR_TOO_LONG) != 0) {
            char dirpath[256];
            assert_nonzero((size_t) (slash - data[i].outfile) < sizeof(dirpath), "i: %d output directory too long\n",
                            i);
            strncpy(dirpath, data[i].outfile, slash - data[i].outfile);
            dirpath[slash - data[i].outfile] = '\0';
            assert_zero(testUtilRmDir(dirpath), "i:%d testUtilRmDir(%s) != 0 (%d: %s)\n",
                        i, dirpath, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_stdin_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        const char *data;
        const char *input;
        const char *outfile;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "123", "-", "test_stdin_input.gif" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];

    const char *input_filename = "-";

    testStart("test_stdin_input");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        sprintf(cmd, "echo '%s' | zint", data[i].data);
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        arg_int(cmd, "-b ", data[i].b);
        arg_input(cmd, input_filename, data[i].input);
        arg_data(cmd, "-o ", data[i].outfile);

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);

        assert_nonzero(testUtilExists(data[i].outfile), "i:%d testUtilExists(%s) != 1\n", i, data[i].outfile);
        assert_zero(testUtilRemove(data[i].outfile), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                    i, data[i].outfile, errno, strerror(errno));
    }

    testFinish();
}

/* Note ordering of `--batch` before/after data/input args affects error messages */
static void test_batch_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        const char *data;
        const char *input;
        const char *input2;
        const char *opt;

        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "123", NULL, NULL, NULL, "Warning 122: Can't define data in batch mode, **IGNORED** '123'\nWarning 124: No data received, no symbol generated" },
        /*  1*/ { BARCODE_CODE128, "123", "123\n456\n", NULL, NULL, "Warning 122: Can't define data in batch mode, **IGNORED** '123'\nD3 96 72 F7 65 C9 61 8E B\nD3 97 62 F7 67 49 19 8E B" },
        /*  2*/ { BARCODE_CODE128, NULL, "123\n456\n", "789\n", NULL, "Warning 143: Can only define one input file in batch mode, **IGNORED** 'test_batch_input2.txt'\nD3 96 72 F7 65 C9 61 8E B\nD3 97 62 F7 67 49 19 8E B" },
        /*  3*/ { BARCODE_CODE128, NULL, "123\r\n456\n", NULL, NULL, "D3 96 72 F7 65 C9 61 8E B\nD3 97 62 F7 67 49 19 8E B" },
        /*  4*/ { BARCODE_CODE128, NULL, "123\r\n456\r\n", NULL, NULL, "D3 96 72 F7 65 C9 61 8E B\nD3 97 62 F7 67 49 19 8E B" },
        /*  5*/ { BARCODE_CODE128, NULL, "123\n456", NULL, NULL, "D3 96 72 F7 65 C9 61 8E B\nWarning 104: No newline at end of input file, last line **IGNORED**" },
        /*  6*/ { BARCODE_EAN13, NULL, "123\r\n456\n", NULL, " --scalexdimdp", "A3 46 8D 1A 34 6A B9 72 CD B2 15 0A\nA3 46 8D 1A 34 6A B9 72 B9 3A 84 EA" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];

    const char *input1_filename = "test_batch_input1.txt";
    const char *input2_filename = "test_batch_input2.txt";
    int have_input1;
    int have_input2;

    testStart("test_batch_input");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint --dump --batch");
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        arg_int(cmd, "-b ", data[i].b);
        arg_data(cmd, "-d ", data[i].data);
        have_input1 = arg_input(cmd, input1_filename, data[i].input);
        have_input2 = arg_input(cmd, input2_filename, data[i].input2);
        if (data[i].opt) {
            strcat(cmd, data[i].opt);
        }

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s)\n", i, buf, data[i].expected);

        if (have_input1) {
            assert_zero(testUtilRemove(input1_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                        i, input1_filename, errno, strerror(errno));
        }
        if (have_input2) {
            assert_zero(testUtilRemove(input2_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                        i, input2_filename, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_batch_large(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        int mirror;
        const char *pattern;
        int length;
        int noeol;

        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_HANXIN, 0, "1", 7827, 0, "00001.gif" },
        /*  1*/ { BARCODE_HANXIN, 1, "1", 7827, 0, "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111.gif" },
        /*  2*/ { BARCODE_HANXIN, 0, "1", 7828, 0, NULL },
        /*  3*/ { BARCODE_HANXIN, 0, "1", ZINT_MAX_DATA_LEN, 0, NULL },
        /*  4*/ { BARCODE_HANXIN, 0, "1", ZINT_MAX_DATA_LEN + 1, 1, NULL },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[16384];
    char data_buf[ZINT_MAX_DATA_LEN + 4];
    char buf[16384];

    const char *input_filename = "test_batch_large.txt";
    int have_input;

    testStart("test_batch_large");

#ifdef _WIN32
    testSkip("Test not compatible with Windows");
    return;
#endif

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint --batch --filetype=gif");
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        arg_int(cmd, "-b ", data[i].b);
        arg_bool(cmd, "--mirror", data[i].mirror);

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        if (!data[i].noeol) {
            strcat(data_buf, "\n");
        }
        have_input = arg_input(cmd, input_filename, data_buf);

        if (!data[i].expected) {
            if (data[i].noeol) {
                printf("++++ Following error and warning expected, ignore: ");
            } else {
                printf("++++ Following error expected, ignore: ");
            }
            fflush(stdout);
        }
        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        if (data[i].expected) {
            assert_zero(testUtilRemove(data[i].expected), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                        i, data[i].expected, errno, strerror(errno));
        } else {
            assert_zero(testUtilExists("out.gif"), "i:%d testUtilExists(out.gif) != 0 (%d: %s) (%s)\n",
                        i, errno, strerror(errno), cmd);
        }

        if (have_input && (p_ctx->index == -1 || data[i].expected)) {
            assert_zero(testUtilRemove(input_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                        i, input_filename, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_checks(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int addongap;
        int border;
        int cols;
        double dotsize;
        double textgap;
        int eci;
        const char *filetype;
        double height;
        double guard_descent;
        int mask;
        int mode;
        int rotate;
        int rows;
        double scale;
        int scmvv;
        int secure;
        int separator;
        int vers;
        int vwhitesp;
        int w;

        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { -2, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 139: Invalid add-on gap value (digits only)" },
        /*  1*/ {  6, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 140: Add-on gap '6' out of range (7 to 12), **IGNORED**" },
        /*  2*/ { 13, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 140: Add-on gap '13' out of range (7 to 12), **IGNORED**" },
        /*  3*/ { 12, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "" },
        /*  4*/ { -1, -2,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 107: Invalid border width value (digits only)" },
        /*  5*/ { -1, 101,  -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 108: Border width '101' out of range (0 to 100), **IGNORED**" },
        /*  6*/ { -1, 100,  -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "" },
        /*  7*/ { -1, -1,   -1, -1, .12345678, -1,    NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 194: Invalid text gap floating point (fractional part must be 7 digits maximum)" },
        /*  8*/ { -1, -1,   -1, -1, -5.1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 195: Text gap '-5.1' out of range (-5 to 10), **IGNORED**" },
        /*  9*/ { -1, -1,   -1, -1, 10.01,   -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 195: Text gap '10.01' out of range (-5 to 10), **IGNORED**" },
        /* 10*/ { -1, -1,   -1, -1, 10.0,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "" },
        /* 11*/ { -1, -1,   -1, 12345678,  -1, -1,    NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 181: Invalid dot radius floating point (integer part must be 7 digits maximum)" },
        /* 12*/ { -1, -1,   -1, 0.009, -1,   -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 106: Invalid dot radius value (less than 0.01), **IGNORED**" },
        /* 13*/ { -1, -1,   -1, 0.01, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "" },
        /* 14*/ { -1, -1,   -2, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 131: Invalid columns value (digits only)" },
        /* 15*/ { -1, -1,  201, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 111: Number of columns '201' out of range (1 to 200), **IGNORED**" },
        /* 16*/ { -1, -1,   -1, -1,   -1,    -2,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 138: Invalid ECI code (digits only)" },
        /* 17*/ { -1, -1,   -1, -1,   -1,    1000000, NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 118: ECI code '1000000' out of range (0 to 999999), **IGNORED**" },
        /* 18*/ { -1, -1,   -1, -1,   -1,    -1,      "jpg", -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 142: File type 'jpg' not supported, **IGNORED**" },
        /* 19*/ { -1, -1,   -1, -1,   -1,    -1,      "giff", -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 142: File type 'giff' not supported, **IGNORED**" },
        /* 20*/ { -1, -1,   -1, -1,   -1,    -1,      "tiff", -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "" }, /* Allow "tiff" */
        /* 21*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -2,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 183: Invalid symbol height floating point (negative value not permitted)" },
        /* 22*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,   0,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 110: Symbol height '0' out of range (0.5 to 2000), **IGNORED**" },
        /* 23*/ { -1, -1,   -1, -1,   -1,    -1,      NULL, 2001,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 110: Symbol height '2001' out of range (0.5 to 2000), **IGNORED**" },
        /* 24*/ { -1, -1,   -1, -1,   -1,    -1,      NULL, 2000,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "" },
        /* 25*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -2, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 182: Invalid guard bar descent floating point (negative value not permitted)" },
        /* 26*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1, 50.1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 135: Guard bar descent '50.1' out of range (0 to 50), **IGNORED**" },
        /* 27*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1, 50.0, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "" },
        /* 28*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -2, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 148: Invalid mask value (digits only)" },
        /* 29*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1,  8, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 147: Mask value '8' out of range (0 to 7), **IGNORED**" },
        /* 30*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1,  7, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 116: Mode value '7' out of range (0 to 6), **IGNORED**" },
        /* 31*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -2, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 136: Invalid mode value (digits only)" },
        /* 32*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1,  6, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "" },
        /* 33*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -2, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 117: Invalid rotation value (digits only)" },
        /* 34*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, 45, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 137: Rotation value '45' out of range (0, 90, 180 or 270 only), **IGNORED**" },
        /* 35*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1,  0, -1, -1,   -1, -1, -1, -1,   -1,   -1, "" },
        /* 36*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -2, -1,   -1, -1, -1, -1,   -1,   -1, "Error 132: Invalid rows value (digits only)" },
        /* 37*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, 91, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 112: Number of rows '91' out of range (1 to 90), **IGNORED**" },
        /* 38*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -2,   -1, -1, -1, -1,   -1,   -1, "Error 184: Invalid scale floating point (negative value not permitted)" },
        /* 39*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, 0.49, -1, -1, -1, -1,   -1,   -1, "Warning 146: Scaling less than 0.5 will be set to 0.5 for 'gif' output" },
        /* 40*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, 0.009, -1, -1, -1, -1,  -1,   -1, "Warning 105: Invalid scale value '0.009' (less than 0.01), **IGNORED**" },
        /* 41*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -2, -1, -1, -1,   -1,   -1, "Error 149: Invalid Structured Carrier Message version value (digits only)" },
        /* 42*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,  100, -1, -1, -1,   -1,   -1, "Warning 150: Structured Carrier Message version '100' out of range (0 to 99), **IGNORED**" },
        /* 43*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -2, -1, -1,   -1,   -1, "Error 134: Invalid ECC value (digits only)" },
        /* 44*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1,  9, -1, -1,   -1,   -1, "Warning 114: ECC level '9' out of range (0 to 8), **IGNORED**" },
        /* 45*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -2, -1,   -1,   -1, "Error 128: Invalid separator value (digits only)" },
        /* 46*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1,  5, -1,   -1,   -1, "Warning 127: Separator value '5' out of range (0 to 4), **IGNORED**" },
        /* 47*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1,  4, -1,   -1,   -1, "" },
        /* 48*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -2,   -1,   -1, "Error 133: Invalid version value (digits only)" },
        /* 49*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, 1000, -1,   -1, "Warning 113: Version value '1000' out of range (1 to 999), **IGNORED**" },
        /* 50*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -2,   -1, "Error 153: Invalid vertical whitespace value '-2' (digits only)" },
        /* 51*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,  101,   -1, "Warning 154: Vertical whitespace value '101' out of range (0 to 100), **IGNORED**" },
        /* 52*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,    0,   -1, "" },
        /* 53*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -2, "Error 120: Invalid horizontal whitespace value '-2' (digits only)" },
        /* 54*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,  101, "Warning 121: Horizontal whitespace value '101' out of range (0 to 100), **IGNORED**" },
        /* 55*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,  100, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];
    const char *outfilename = "out.gif";

    testStart("test_checks");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint -d 1 --filetype=gif");
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        if (data[i].addongap != -1) {
            arg_int(cmd, "--addongap=", data[i].addongap);
            arg_int(cmd, "-b ", BARCODE_EAN8);
        }
        arg_int(cmd, "--border=", data[i].border);
        arg_int(cmd, "--cols=", data[i].cols);
        arg_double(cmd, "--dotsize=", data[i].dotsize);
        arg_double(cmd, "--textgap=", data[i].textgap);
        arg_int(cmd, "--eci=", data[i].eci);
        arg_data(cmd, "--filetype=", data[i].filetype);
        arg_double(cmd, "--height=", data[i].height);
        arg_double(cmd, "--guarddescent=", data[i].guard_descent);
        arg_int(cmd, "--mask=", data[i].mask);
        arg_int(cmd, "--mode=", data[i].mode);
        arg_int(cmd, "--rotate=", data[i].rotate);
        arg_int(cmd, "--rows=", data[i].rows);
        arg_double(cmd, "--scale=", data[i].scale);
        arg_int(cmd, "--scmvv=", data[i].scmvv);
        arg_int(cmd, "--secure=", data[i].secure);
        arg_int(cmd, "--separator=", data[i].separator);
        arg_int(cmd, "--vers=", data[i].vers);
        arg_int(cmd, "--vwhitesp=", data[i].vwhitesp);
        arg_int(cmd, "-w ", data[i].w);

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s) (%s)\n",
                    i, buf, data[i].expected, cmd);

        if (strncmp(data[i].expected, "Warning", 7) == 0) {
            assert_zero(testUtilRemove(outfilename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                        i, outfilename, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_barcode_symbology(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *bname;
        const char *data;
        const char *primary;
        int fail;
        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "_", "1", NULL, 1, "Error 119: Invalid barcode type '_'" },
        /*  1*/ { "a", "1", NULL, 1, "Error 119: Invalid barcode type 'a'" },
        /*  2*/ { "code128", "1", NULL, 0, "BARCODE_CODE128 (20)," },
        /*  3*/ { "code12", "1", NULL, 1, "Error 119: Invalid barcode type 'code12'" },
        /*  4*/ { "code1288", "1", NULL, 1, "Error 119: Invalid barcode type 'code1288'" },
        /*  5*/ { "BARCODE_CODE11", "1", NULL, 0, "BARCODE_CODE11 (1)," },
        /*  6*/ { "C25 Standard", "1", NULL, 0, "BARCODE_C25STANDARD (2)," },
        /*  7*/ { "Code 2 of 5 Standard", "1", NULL, 0, "BARCODE_C25STANDARD (2)," }, /* Synonym */
        /*  8*/ { "Standard Code 2 of 5", "1", NULL, 0, "BARCODE_C25STANDARD (2)," }, /* Synonym */
        /*  9*/ { "C25INTER", "1", NULL, 0, "BARCODE_C25INTER (3)," },
        /* 10*/ { "interleaved code 2 of 5", "1", NULL, 0, "BARCODE_C25INTER (3)," }, /* Synonym */
        /* 11*/ { "2of5datalogic", "1", NULL, 0, "BARCODE_C25LOGIC (6)," }, /* Synonym */
        /* 12*/ { "ean8", "1", NULL, 0, "BARCODE_EAN8 (10)," },
        /* 13*/ { "eanx", "1", NULL, 0, "BARCODE_EANX (13)," },
        /* 14*/ { "ean", "1", NULL, 0, "BARCODE_EANX (13)," }, /* Synonym */
        /* 15*/ { "eanx chk", "1", NULL, 0, "BARCODE_EANX_CHK (14)," },
        /* 16*/ { "eanxchk", "1", NULL, 0, "BARCODE_EANX_CHK (14)," },
        /* 17*/ { "ean13", "1", NULL, 0, "BARCODE_EAN13 (15)," },
        /* 18*/ { "ean 128", "[01]12345678901231", NULL, 0, "BARCODE_GS1_128 (16)," }, /* Synonym */
        /* 19*/ { "databar omn", "1", NULL, 0, "BARCODE_DBAR_OMN (29)," }, /* Synonym */
        /* 20*/ { "databar omni", "1", NULL, 0, "BARCODE_DBAR_OMN (29)," }, /* Synonym */
        /* 21*/ { "barcodeMailmark", "01000000000000000AA00AA0A", NULL, 0, "BARCODE_MAILMARK_4S (121)," }, /* Synonym */
        /* 22*/ { "Mailmark 4s", "01000000000000000AA00AA0A", NULL, 0, "BARCODE_MAILMARK_4S (121)," }, /* Synonym */
        /* 23*/ { "Mailmark 4-state", "01000000000000000AA00AA0A", NULL, 0, "BARCODE_MAILMARK_4S (121)," }, /* Synonym */
        /* 24*/ { "Mailmark 2D", "012100123412345678AB19XY1A 0", NULL, 0, "BARCODE_MAILMARK_2D (119)," },
        /* 25*/ { "v iN", "1FTCR10UXTPA78180", NULL, 0, "BARCODE_VIN (73)," },
        /* 26*/ { "eanx cc", "[20]01", "1234567890128", 0, "BARCODE_EANX_CC (130)," },
        /* 27*/ { "eancc", "[20]01", "1234567890128", 0, "BARCODE_EANX_CC (130)," }, /* Synonym */
        /* 28*/ { "upcacc", "[20]01", "12345678901", 0, "BARCODE_UPCA_CC (135)," },
        /* 29*/ { "upcecc", "[20]01", "1234567", 0, "BARCODE_UPCE_CC (136)," },
        /* 30*/ { "x", "1", NULL, 1, "Error 119: Invalid barcode type 'x'" },
        /* 31*/ { "\177", "1", NULL, 1, "Error 119: Invalid barcode type '\177'" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[8192];
    const char *outfilename = "out.gif";

    testStart("test_barcode_symbology");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint --filetype=gif");
        strcat(cmd, " --verbose");

        arg_data(cmd, "-b ", data[i].bname);
        arg_data(cmd, "-d ", data[i].data);
        arg_data(cmd, "--primary=", data[i].primary);

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        if (!data[i].fail) {
            assert_zero(testUtilRemove(outfilename), "i:%d testUtilRemove(%s) != 0 (%d: %s) (%s)\n",
                        i, outfilename, errno, strerror(errno), cmd);
        }
        assert_nonnull(strstr(buf, data[i].expected), "i:%d strstr(%s, %s) == NULL (%s)\n",
                    i, buf, data[i].expected, cmd);
    }

    testFinish();
}

#ifdef ZINT_HAVE_GS1SE
#define TEST_OTHER_OPTS_GS1STRICT_ERROR "Error 267: AI (00): The numeric check digit is incorrect. (00)37610425002123456|8|"
#else
#define TEST_OTHER_OPTS_GS1STRICT_ERROR "Warning 261: AI (00) data position 18: Bad checksum '8', expected '9'"
#endif

static void test_other_opts(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        const char *data;
        int input_mode;
        const char *opt;
        const char *opt_data;

        const char *expected;
        int strstr_cmp;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "1", -1, " --test", NULL, "++++ Following warning expected, ignore: Warning 188: Directory for mirrored output too long (greater than 220), **IGNORED**\n++++ Following error expected, ignore: Error 102: Unable to read input file 'test_nosuch.file' (", 1 }, /* Do internal test */
        /*  1*/ { BARCODE_AZTEC, "1", -1, " --azfull", NULL, "", 0 },
        /*  2*/ { BARCODE_CODE128, "1", -1, " --bg=", "EF9900", "", 0 },
        /*  3*/ { BARCODE_CODE128, "1", -1, " -bg=", "EF9900", "", 0 },
        /*  4*/ { BARCODE_CODE128, "1", -1, " --bg=", "EF9900AA", "", 0 },
        /*  5*/ { BARCODE_CODE128, "1", -1, " --bg=", "GF9900", "Error 881: Malformed background RGB colour 'GF9900' (hexadecimal only)", 0 },
        /*  6*/ { BARCODE_CODE128, "1", -1, " --bgcolor=", "EF9900", "", 0 },
        /*  7*/ { BARCODE_CODE128, "1", -1, " --bgcolour=", "EF9900", "", 0 },
        /*  8*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000", "", 0 },
        /*  9*/ { BARCODE_CODE128, "1", -1, " --fg=", "00000000", "", 0 },
        /* 10*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000F", "Error 880: Malformed foreground RGB colour (6 or 8 characters only)", 0 },
        /* 11*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000FG", "Error 881: Malformed foreground RGB colour '000000FG' (hexadecimal only)", 0 },
        /* 12*/ { BARCODE_CODE128, "1", -1, " --fg=", "0,0,0,100", "", 0 },
        /* 13*/ { BARCODE_CODE128, "1", -1, " --fgcolor=", "111111", "", 0 },
        /* 14*/ { BARCODE_CODE128, "1", -1, " --fgcolour=", "111111", "", 0 },
        /* 15*/ { BARCODE_CODE128, "1", -1, " --compliantheight", NULL, "", 0 },
        /* 16*/ { BARCODE_DATAMATRIX, "1", -1, " --dmiso144", NULL, "", 0 },
        /* 17*/ { BARCODE_EANX, "123456", -1, " --guardwhitespace", NULL, "", 0 },
        /* 18*/ { BARCODE_EANX, "123456", -1, " --embedfont", NULL, "", 0 },
        /* 19*/ { BARCODE_CODE128, "1", -1, " --nobackground", NULL, "", 0 },
        /* 20*/ { BARCODE_CODE128, "1", -1, " --noquietzones", NULL, "", 0 },
        /* 21*/ { BARCODE_CODE128, "1", -1, " --notext", NULL, "", 0 },
        /* 22*/ { BARCODE_CODE128, "1", -1, " --quietzones", NULL, "", 0 },
        /* 23*/ { BARCODE_CODE128, "1", -1, " --reverse", NULL, "", 0 },
        /* 24*/ { BARCODE_CODE128, "1", -1, " --werror", NULL, "", 0 },
        /* 25*/ { 19, "1", -1, " --werror", NULL, "Error 207: Codabar 18 not supported", 0 },
        /* 26*/ { BARCODE_GS1_128, "[01]12345678901231", -1, "", NULL, "", 0 },
        /* 27*/ { BARCODE_GS1_128, "0112345678901231", -1, "", NULL, "Error 252: Data does not start with an AI", 0 },
        /* 28*/ { BARCODE_GS1_128, "0112345678901231", -1, " --gs1nocheck", NULL, "Error 252: Data does not start with an AI", 0 },
        /* 29*/ { BARCODE_GS1_128, "[00]376104250021234569", -1, "", NULL, "", 0 },
        /* 30*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, "", NULL, "Warning 261: AI (00) data position 18: Bad checksum '8', expected '9'", 0 },
        /* 31*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, " --gs1nocheck", NULL, "", 0 },
        /* 32*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, " --werror", NULL, "Error 261: AI (00) data position 18: Bad checksum '8', expected '9'", 0 },
        /* 33*/ { BARCODE_GS1_128, "[00]376104250021234569", -1, " --gs1strict", NULL, "", 0 },
        /* 34*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, " --gs1strict", NULL, TEST_OTHER_OPTS_GS1STRICT_ERROR, 0 },
        /* 35*/ { BARCODE_GS1_128, "00376104250021234569", -1, " --gs1raw", NULL, "", 0 },
        /* 36*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "1", "Error 155: Invalid Structured Append argument, expect \"index,count[,ID]\"", 0 },
        /* 37*/ { BARCODE_AZTEC, "1", -1, " --structapp=", ",", "Error 155: Structured Append index too short", 0 },
        /* 38*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "1234567890,", "Error 155: Structured Append index too long", 0 },
        /* 39*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,", "Error 155: Structured Append count too short", 0 },
        /* 40*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,1234567890", "Error 155: Structured Append count too long", 0 },
        /* 41*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,123456789,", "Error 155: Structured Append ID too short", 0 },
        /* 42*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,123456789,123456789012345678901234567890123", "Error 155: Structured Append ID too long", 0 },
        /* 43*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,123456789,12345678901234567890123456789012", "Error 701: Structured Append count '123456789' out of range (2 to 26)", 0 },
        /* 44*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "26,26,12345678901234567890123456789012", "", 0 },
        /* 45*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "A,26,12345678901234567890123456789012", "Error 155: Invalid Structured Append index (digits only)", 0 },
        /* 46*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "26,A,12345678901234567890123456789012", "Error 155: Invalid Structured Append count (digits only)", 0 },
        /* 47*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "26,1,12345678901234567890123456789012", "Error 155: Invalid Structured Append count '1', must be greater than or equal to 2", 0 },
        /* 48*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "0,2,12345678901234567890123456789012", "Error 155: Structured Append index '0' out of range (1 to count '2')", 0 },
        /* 49*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "3,2,12345678901234567890123456789012", "Error 155: Structured Append index '3' out of range (1 to count '2')", 0 },
        /* 50*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "2,3,12345678901234567890123456789012", "", 0 },
        /* 51*/ { BARCODE_PDF417, "1", -1, " --heightperrow", NULL, "", 0 },
        /* 52*/ { -1, NULL, -1, " -v", NULL, "Zint version ", 1 },
        /* 53*/ { -1, NULL, -1, " --version", NULL, "Zint version ", 1 },
        /* 54*/ { -1, NULL, -1, " -h", NULL, "Encode input data in a barcode ", 1 },
        /* 55*/ { -1, NULL, -1, " -e", NULL, "3: ISO/IEC 8859-1 ", 1 },
        /* 56*/ { -1, NULL, -1, " -t", NULL, "1 CODE11 ", 1 },
        /* 57*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "12345678", "Error 189: scalexdimdp X-dim invalid floating point: integer part must be 7 digits maximum", 0 },
        /* 58*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "1234567890123", "Error 189: scalexdimdp X-dim too long", 0 },
        /* 59*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "123456.12", "Error 189: scalexdimdp X-dim invalid floating point: 7 significant digits maximum", 0 },
        /* 60*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", ",12.34", "Error 189: scalexdimdp X-dim too short", 0 },
        /* 61*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "12.34,", "Error 189: scalexdimdp resolution too short", 0 },
        /* 62*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "12mm1", "Error 189: scalexdimdp X-dim unknown units: mm1", 0 },
        /* 63*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "1inc", "Error 189: scalexdimdp X-dim unknown units: inc", 0 },
        /* 64*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "12.34in,123x", "Error 189: scalexdimdp resolution unknown units: x", 0 },
        /* 65*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "12,123.45678", "Error 189: scalexdimdp resolution invalid floating point: 7 significant digits maximum", 0 },
        /* 66*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "10.1,1000", "Warning 185: scalexdimdp X-dim '10.1' out of range (greater than 10), **IGNORED**", 0 },
        /* 67*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "10,1000.1", "Warning 186: scalexdimdp resolution '1000.1' out of range (greater than 1000), **IGNORED**", 0 },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[8192];
    const char *outfilename = ZBarcode_NoPng() ? "out.gif" : "out.png";

    testStart("test_other_opts");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint");

        arg_int(cmd, "-b ", data[i].b);
        arg_input_mode(cmd, data[i].input_mode);
        arg_data(cmd, "-d ", data[i].data);
        if (data[i].opt_data != NULL) {
            arg_data(cmd, data[i].opt, data[i].opt_data);
        } else {
            strcat(cmd, data[i].opt);
        }

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        if (data[i].strstr_cmp) {
            assert_nonnull(strstr(buf, data[i].expected), "i:%d strstr buf (%s) != expected (%s) (%s)\n",
                            i, buf, data[i].expected, cmd);
        } else {
            assert_zero(strcmp(buf, data[i].expected), "i:%d strcmp buf (%s) != expected (%s) (%s)\n",
                        i, buf, data[i].expected, cmd);
            if (strstr(data[i].expected, "Error") == NULL) {
                assert_zero(testUtilRemove(outfilename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                            i, outfilename, errno, strerror(errno));
            }
        }
    }

    testFinish();
}

static void test_combos(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        const char *data;
        const char *opts;

        const char *expected;
        const char *outfilename;
        int strstr_cmp;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { -1, "1", " --direct -o direct.gif", "Warning 193: Output file given, '--direct' option **IGNORED**", "direct.gif", 0 },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[8192];

    testStart("test_combos");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint");

        arg_int(cmd, "-b ", data[i].b);
        arg_data(cmd, "-d ", data[i].data);
        strcat(cmd, data[i].opts);

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        if (data[i].strstr_cmp) {
            assert_nonnull(strstr(buf, data[i].expected), "i:%d strstr buf (%s) != expected (%s) (%s)\n",
                            i, buf, data[i].expected, cmd);
        } else {
            assert_zero(strcmp(buf, data[i].expected), "i:%d strcmp buf (%s) != expected (%s) (%s)\n",
                        i, buf, data[i].expected, cmd);
        }
        if (data[i].outfilename != NULL) {
            assert_zero(remove(data[i].outfilename), "i:%d remove(%s) != 0 (%d: %s)\n",
                        i, data[i].outfilename, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_exit_status(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        const char *data;
        int input_mode;
        const char *opt;
        const char *opt_data;

        int expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "1", -1, " --bg=", "EF9900", 0 },
        /*  1*/ { BARCODE_CODE128, "1", -1, " --bg=", "GF9900", ZINT_ERROR_INVALID_OPTION }, /* Caught by libzint */
        /*  2*/ { BARCODE_CODE128, "1", -1, " --border=", "1001", ZINT_WARN_INVALID_OPTION }, /* Caught by CLI */
        /*  3*/ { BARCODE_CODE128, "1", -1, " --data=", "\200", ZINT_ERROR_INVALID_DATA }, /* Caught by libzint */
        /*  4*/ { BARCODE_CODE128, "1", -1, " --separator=", "-1", ZINT_ERROR_INVALID_OPTION }, /* Caught by CLI */
        /*  5*/ { BARCODE_CODE128, "1", -1, " --separator", NULL, ZINT_ERROR_INVALID_OPTION },
        /*  6*/ { BARCODE_CODE128, "1", -1, " --separator=", NULL, 0 }, /* Separator arg treated as 0 */
        /*  7*/ { BARCODE_CODE128, "1", -1, " --unknown", NULL, ZINT_ERROR_INVALID_OPTION },
    };
    int data_size = ARRAY_SIZE(data);
    int i;
    int exit_status;

    char cmd[4096];
    char buf[8192];
    const char *outfilename = ZBarcode_NoPng() ? "out.gif" : "out.png";

    testStart("test_exit_status");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint");
        *buf = '\0';

        arg_int(cmd, "-b ", data[i].b);
        arg_input_mode(cmd, data[i].input_mode);
        arg_data(cmd, "-d ", data[i].data);
        if (data[i].opt_data != NULL) {
            arg_data(cmd, data[i].opt, data[i].opt_data);
        } else {
            strcat(cmd, data[i].opt);
        }

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, &exit_status), "i:%d exec(%s) NULL\n", i, cmd);
        assert_equal(exit_status, data[i].expected, "i:%d exit_status %d != expected (%d) (%s), (cmd: %s)\n",
                        i, exit_status, data[i].expected, buf, cmd);
        if (data[i].expected < ZINT_ERROR) {
            assert_zero(testUtilRemove(outfilename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n",
                        i, outfilename, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_bad_args(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        const char *data;
        int input_mode;
        const char *opt;
        const char *opt_data;

        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, NULL, -1, NULL, NULL, "Error 109: Option '-d' requires an argument" },
        /*  1*/ { BARCODE_CODE128, "1", -1, " -o", NULL, "Error 109: Option '-o' requires an argument" },
        /*  2*/ { BARCODE_CODE128, "1", -1, " --fast=", "1", "Error 126: Option '--fast' does not take an argument" },
        /*  3*/ { BARCODE_CODE128, "1", -1, " -x", NULL, "Error 101: Unknown option '-x'" },
        /*  4*/ { BARCODE_CODE128, "1", -1, " --x", NULL, "Error 101: Unknown option '--x'" },
        /*  5*/ { BARCODE_CODE128, "1", -1, " --dm", NULL, "Error 187: Ambiguous option '--dm'" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;
    int exit_status;

    char cmd[4096];
    char buf[8192];

    testStart("test_bad_args");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint");
        *buf = '\0';

        arg_int(cmd, "-b ", data[i].b);
        arg_input_mode(cmd, data[i].input_mode);
        if (data[i].data) {
            arg_data(cmd, "-d ", data[i].data);
        } else {
            strcat(cmd, " -d");
        }
        if (data[i].opt) {
            if (data[i].opt_data != NULL) {
                arg_data(cmd, data[i].opt, data[i].opt_data);
            } else {
                strcat(cmd, data[i].opt);
            }
        }

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, &exit_status), "i:%d exec(%s) NULL\n", i, cmd);
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s) (%s)\n",
                    i, buf, data[i].expected, cmd);
    }

    testFinish();
}

static void test_too_many_args(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        const char *args0;
        const char *args1;
        const char *args2;
        const char *args3;

        const char *expected;
    };
    /* s/\v(\/\*)[ 0-9]*(\*\/)/\=printf("%s%3d%s", submatch(1), (@z+setreg('z',@z+1)), submatch(2))/ | let @z=0: */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128,
                    " -d 000 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 010 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 020 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 030 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 040 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 050 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 060 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 070 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 080 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 090 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9",
                    " -d 100 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 110 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 120 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 130 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 140 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 150 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 160 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 170 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 180 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 190 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9",
                    "",
                    "",
                    ""
                },
        /*  1*/ { BARCODE_CODE128,
                    " -d 000 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 010 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 020 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 030 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 040 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 050 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 060 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 070 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 080 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 090 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9",
                    " -d 100 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 110 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 120 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 130 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 140 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 150 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 160 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 170 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 180 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 190 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9",
                    " -d 200",
                    "",
                    "Error 770: Too many stacked symbols (maximum 200)"
                },
        /*  1*/ { BARCODE_CODE128,
                    " -d 000 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 010 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 020 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 030 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 040 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 050 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 060 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 070 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 080 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 090 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9",
                    " -d 100 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 110 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 120 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 130 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 140 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 150 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 160 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 170 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 180 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 190 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9",
                    " -d 200 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 210 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 220 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 230 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 240 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 250 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 260 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 270 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 280 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 290 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9",
                    " -d 300",
                    "Error 129: Too many data args (maximum 300)"
                },
        /*  2*/ { BARCODE_CODE128,
                    " -d 000 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 010 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 020 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 030 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 040 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 050 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 060 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 070 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 080 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 090 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9",
                    " -d 100 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 110 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 120 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 130 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 140 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 150 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 160 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 170 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 180 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 190 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9",
                    " -d 200 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 210 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 220 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 230 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 240 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 250 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 260 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 270 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 280 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9 -d 290 -d 1 -d 2 -d 3 -d 4 -d 5 -d 6 -d 7 -d 8 -d 9",
                    " -i 300",
                    "Error 130: Too many data args (maximum 300)"
                },
        /*  3*/ { BARCODE_CODE16K,
                    " -d 000 -d 1",
                    "",
                    "",
                    "",
                    "Error 173: Symbology must be stackable if multiple data arguments given"
                },
        /*  4*/ { BARCODE_CODE128,
                    " -d 000 blah",
                    "",
                    "",
                    "",
                    "Warning 191: Extra argument 'blah' **IGNORED**"
                },
        /*  5*/ { BARCODE_CODE128,
                    " -d 000 blah blah",
                    "",
                    "",
                    "",
                    "Warning 192: Extra arguments beginning with 'blah' **IGNORED**"
                },
        /*  6*/ { BARCODE_CODE128,
                    " -d 000 -d 1 --seg1=0,1",
                    "",
                    "",
                    "",
                    "Error 170: Cannot specify segments and multiple data arguments together"
                },
        /*  7*/ { BARCODE_CODE128,
                    " -i gosh.txt --seg1=0,1",
                    "",
                    "",
                    "",
                    "Error 171: Cannot use input argument with segment arguments"
                },
        /*  8*/ { BARCODE_CODE128,
                    " -i gosh.txt --mirror",
                    "",
                    "",
                    "",
                    "Warning 163: '--mirror' given but no data argument, **IGNORED**\nError 229: Unable to read input file \"gosh.txt\" (2: No such file or directory)"
                },
    };
    int data_size = ARRAY_SIZE(data);
    int i;
    int exit_status;

    char cmd[4096];
    char buf[8192];

    testStart("test_too_many_args");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint");
        *buf = '\0';

        arg_int(cmd, "-b ", data[i].b);
        strcat(cmd, data[i].args0);
        strcat(cmd, data[i].args1);
        strcat(cmd, data[i].args2);
        strcat(cmd, data[i].args3);

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, &exit_status), "i:%d exec(%s) == NULL\n", i, cmd);
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s) (%s)\n",
                    i, buf, data[i].expected, cmd);
    }

    testFinish();
}

static void test_optional_args(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        const char *data;
        const char *primary;
        const char *opt;

        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, "123", NULL, " --dmb256", "AA A\n88 F\nD0 C\nC7 F\nAF A\nFF 3\nF5 4\nA1 B\nB5 0\nE1 D\nE9 A\nFF F" },
        /*  1*/ { BARCODE_DATAMATRIX, "123", NULL, " --dmb256=", "AA A\n88 F\nD0 C\nC7 F\nAF A\nFF 3\nF5 4\nA1 B\nB5 0\nE1 D\nE9 A\nFF F" },
        /*  2*/ { BARCODE_DATAMATRIX, "123", NULL, " --dmb256=A", "Error 158: Invalid Data Matrix Base 256 mode length value (digits only)" },
        /*  3*/ { BARCODE_DATAMATRIX, "123", NULL, " --dmc40", "AA 8\n80 4\nC0 8\n81 C\nB7 0\nAB C\nE1 0\nE5 4\nBC 0\nFF C" },
        /*  4*/ { BARCODE_DATAMATRIX, "123", NULL, " --dmc40=", "AA 8\n80 4\nC0 8\n81 C\nB7 0\nAB C\nE1 0\nE5 4\nBC 0\nFF C" },
        /*  5*/ { BARCODE_DATAMATRIX, "123", NULL, " --dmc40=-1", "Error 160: Invalid Data Matrix C40 mode length value (digits only)" },
        /*  6*/ { BARCODE_DATAMATRIX, "123", NULL, " --dmc40 --dmb256", "Warning 159: '--dmc40' already set, '--dmb256' **IGNORED**\nAA 8\n80 4\nC0 8\n81 C\nB7 0\nAB C\nE1 0\nE5 4\nBC 0\nFF C" },
        /*  7*/ { BARCODE_DATAMATRIX, "123", NULL, " --square --dmre", "Warning 156: '--square' already set, '--dmre' **IGNORED**\nAA 8\n85 4\nE5 8\nC9 4\nCC 0\nB1 C\n98 0\nB6 4\nA2 0\nFF C" },
        /*  8*/ { BARCODE_EAN13, "123", NULL, " --scalexdimdp", "A3 46 8D 1A 34 6A B9 72 CD B2 15 0A" },
        /*  9*/ { BARCODE_MAXICODE, "123", "1234567", " --scmvv", "D7 6F BF FC\n55 70 D8 08\nDB 12 A6 78\n55 55 55 50\n00 00 00 08\nAA AA AA A8\n55 55 55 54\n00 00 00 00\nAA AA AA AC\n55 B1 35 58\n00 90 08 0C\nAA C0 52 A8\n54 40 05 50\n01 00 08 00\nAA C0 0A AC\n55 00 01 58\n02 80 08 0C\nAB 80 1E A0\n54 00 05 5C\n03 80 0C 08\nA8 00 02 A4\n55 40 25 58\n00 60 40 08\nAA 60 66 A0\n55 55 5B FC\n00 00 02 30\nAA AA A1 E0\n5E CF C1 E8\nD0 62 E5 84\n20 1B 10 D8\nDB 8E B6 80\n21 96 26 C0\nC3 C9 89 F8" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[8192];

    testStart("test_optional_args");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint --dump");
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        arg_int(cmd, "-b ", data[i].b);
        if (data[i].primary) {
            arg_data(cmd, "--primary=", data[i].primary);
        }
        arg_data(cmd, "-d ", data[i].data);
        strcat(cmd, data[i].opt);

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s) (%s)\n",
                    i, buf, data[i].expected, cmd);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_dump_args", test_dump_args },
        { "test_dump_segs", test_dump_segs },
        { "test_input", test_input },
        { "test_stdin_input", test_stdin_input },
        { "test_batch_input", test_batch_input },
        { "test_batch_large", test_batch_large },
        { "test_checks", test_checks },
        { "test_barcode_symbology", test_barcode_symbology },
        { "test_other_opts", test_other_opts },
        { "test_combos", test_combos },
        { "test_exit_status", test_exit_status },
        { "test_bad_args", test_bad_args },
        { "test_too_many_args", test_too_many_args },
        { "test_optional_args", test_optional_args },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
