/*
    libzint - the open source barcode library
    Copyright (C) 2020-2024 Robin Stuart <rstuart114@gmail.com>

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
#include <sys/wait.h>
#endif

#include "testcommon.h"

#ifdef _WIN32
/* Hacks to stop popen() mangling input on Windows */
static int utf8_to_wchar(const char *str, wchar_t *out) {
    unsigned int codepoint, state = 0;

    while (*str) {
        do {
            decode_utf8(&state, &codepoint, *str++);
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
                fprintf(stderr, "arg_input: failed to write %d bytes, cnt %d written (%s)\n", (int) strlen(input), cnt, filename);
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
        if (input_mode & HEIGHTPERROW_MODE) {
            sprintf(cmd + strlen(cmd), "%s--heightperrow", strlen(cmd) ? " " : "");
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
        char *data;
        char *data2;
        char *input;
        char *input2;
        int input_mode;
        int output_options;
        int batch;
        int cols;
        int dmre;
        int eci;
        int fullmultibyte;
        int mask;
        int mode;
        char *primary;
        int rows;
        int secure;
        int square;
        int vers;

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ {              -1, "123", NULL, NULL, NULL,       -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D3 96 72 F7 65 C9 61 8E B" },
        /*  1*/ { BARCODE_CODE128, "123", NULL, NULL, NULL,       -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D3 96 72 F7 65 C9 61 8E B" },
        /*  2*/ { BARCODE_CODE128, "123", "456", NULL, NULL,      -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D3 96 72 F7 65 C9 61 8E B\nD3 97 62 F7 67 49 19 8E B" },
        /*  3*/ { BARCODE_CODE128, "123", NULL, NULL, NULL,       -1, -1, 1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "Warning 141: Can't use batch mode if data given, ignoring\nD3 96 72 F7 65 C9 61 8E B" },
        /*  4*/ { BARCODE_CODE128, NULL, NULL, "123\n45\n", NULL, -1, -1, 1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D3 96 72 F7 65 C9 61 8E B\nD3 97 62 3B 63 AC" },
        /*  5*/ { BARCODE_CODE128, NULL, NULL, "123\n45\n", "7\n",-1, -1, 1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "Warning 144: Processing first input file 'test_dump_args1.txt' only\nD3 96 72 F7 65 C9 61 8E B\nD3 97 62 3B 63 AC" },
        /*  6*/ { BARCODE_CODE128, "\t", NULL, NULL, NULL,        -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 90 D2 1A 63 AC" },
        /*  7*/ { BARCODE_CODE128, "\\t", NULL, NULL, NULL, ESCAPE_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 90 D2 1A 63 AC" },
        /*  8*/ { BARCODE_CODE128, "\\^Ab\011", NULL, NULL, NULL, EXTRA_ESCAPE_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 9E 8A 43 43 48 D1 8E B" },
        /*  9*/ { BARCODE_CODE128, "\\^Ab\\t", NULL, NULL, NULL, ESCAPE_MODE | EXTRA_ESCAPE_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 9E 8A 43 43 48 D1 8E B" },
        /* 10*/ { BARCODE_CODE128, "123", NULL, NULL, NULL, -1, BARCODE_BIND | BARCODE_BOX | BARCODE_BIND_TOP | SMALL_TEXT | BOLD_TEXT | CMYK_COLOUR, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D3 96 72 F7 65 C9 61 8E B" },
        /* 11*/ { BARCODE_CODE128, "123", NULL, NULL, NULL, -1, BARCODE_DOTTY_MODE, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "Error 224: Selected symbology cannot be rendered as dots" },
        /* 12*/ { BARCODE_CODABLOCKF, "ABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 97 BA 86 51 88 B1 11 AC 46 D8 C7 58\nD0 97 BB 12 46 88 C5 1A 3C 55 CC C7 58" },
        /* 13*/ { BARCODE_CODABLOCKF, "ABCDEF", NULL, NULL, NULL, -1, -1, 0, 10, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 97 BA 86 51 88 B1 11 AC 44 68 BC 98 EB\nD0 97 BB 12 46 2B BD 7B A3 47 8A 8D 18 EB" },
        /* 14*/ { BARCODE_CODABLOCKF, "ABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL,  3, -1, 0, -1, "D0 97 BA 58 51 88 B1 11 AC 46 36 C7 58\nD0 97 BB 12 46 88 C5 77 AF 74 62 C7 58\nD0 97 BA CE 5D EB DD 1A 3C 56 88 C7 58" },
        /* 15*/ { BARCODE_CODE11, NULL, NULL, "123", NULL,        -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "B2 D6 96 CA B5 6D 64" },
        /* 16*/ { BARCODE_CODE11, NULL, NULL, "123", NULL,        -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0,  1, "B2 D6 96 CA B5 64" },
        /* 17*/ { BARCODE_CODE11, "123", NULL, "456", NULL,       -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0,  2, "B2 D6 96 CA B2\nB2 B6 DA 9A B2" },
        /* 18*/ { BARCODE_CODE11, "123", "456", "789", "012",     -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0,  2, "B2 D6 96 CA B2\nB2 B6 DA 9A B2\nB2 A6 D2 D5 64\nB2 AD AD 2D 64" },
        /* 19*/ { BARCODE_PDF417, "123", NULL, NULL, NULL,        -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1,  0, 0, -1, "FF 54 7A BC 3A 9C 1D 5C 0F E8 A4\nFF 54 7E AE 3C 11 5F AB 8F E8 A4\nFF 54 6A F8 29 9F 1D 5F 8F E8 A4\nFF 54 57 9E 37 BA 1A F7 CF E8 A4\nFF 54 75 CC 36 F0 5D 73 0F E8 A4" },
        /* 20*/ { BARCODE_DATAMATRIX, "ABC", NULL, NULL, NULL, -1,          -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA 8\nB3 4\n8F 0\nB2 C\nA6 0\nBA C\nD6 0\nEB 4\nE2 8\nFF C" },
        /* 21*/ { BARCODE_DATAMATRIX, "ABC", NULL, NULL, NULL, -1, READER_INIT, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A\nAC 7\n8A 4\nA0 3\nC2 2\nB5 1\n82 2\nBA 7\n8C C\nA0 5\n86 A\nFF F" },
        /* 22*/ { BARCODE_DATAMATRIX, "ABCDEFGH", NULL, NULL, NULL, FAST_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nA6 8C\nB2 F0\n98 B4\nB9 A8\nB8 CC\nF0 78\nA0 3C\n99 70\n85 1C\nDA B0\nE5 94\nA7 50\nFF FC" },
        /* 23*/ { BARCODE_DATAMATRIX, "ABCDEFGH", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\n80 04\n82 60\nC5 24\n98 A8\nA3 9C\nCB B8\nAF DC\n86 58\nF6 44\nAC 18\n90 54\nCF 30\nFF FC" },
        /* 24*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJK", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA AA AA\n80 09 F9 BD\n82 4A E2 58\nC5 CD C9 A5\nD8 5C A5 FC\nE0 35 88 69\nCC FC B3 E6\nFF FF FF FF" },
        /* 25*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJK", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 1, -1, "AA AA\n80 25\n82 24\nC5 5D\n98 90\nA4 C7\nC8 A6\nB9 E9\n8E 02\nDE 91\nCD 6C\nA0 BB\n85 80\n98 2D\nE4 CA\nFF FF" },
        /* 26*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA A8\nBA 5A 44\n8B 4D 28\nBF 77 64\n97 85 50\nBA D8 AC\nCD ED B8\nD4 B5 2C\nD1 A8 00\n81 FB 2C\nE4 75 78\n96 E8 2C\nF3 75 78\nEE 1D 04\nCA BA 98\nB1 8F B4\nA0 4F 00\nE4 A7 74\nF1 D3 90\nEF E1 BC\n91 10 38\nFF FF FC" },
        /* 27*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 1, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA AA AA AA AA AA AA\nBA 03 BA 7D E5 31 B0 0D\n8B 6A 93 B6 E0 0A B8 3C\nBF 1D EA A7 EB ED A1 FB\n96 66 86 B6 C9 AE 92 40\nBF 65 E7 95 BC B7 FA E3\nCC 7C 90 CC D1 24 AB 5A\nFF FF FF FF FF FF FF FF" },
        /* 28*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 1, -1, 0, -1, -1, NULL, -1, -1, 1, -1, "AA AA A8\nBA 5A 44\n8B 4D 28\nBF 77 64\n97 85 50\nBA D8 AC\nCD ED B8\nD4 B5 2C\nD1 A8 00\n81 FB 2C\nE4 75 78\n96 E8 2C\nF3 75 78\nEE 1D 04\nCA BA 98\nB1 8F B4\nA0 4F 00\nE4 A7 74\nF1 D3 90\nEF E1 BC\n91 10 38\nFF FF FC" },
        /* 29*/ { BARCODE_DATAMATRIX, "[91]12[92]34", NULL, NULL, NULL, GS1_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nFA 9C\nBC 00\nD7 84\nED E0\nA4 E4\nA7 40\n9D 3C\nBF 50\nFA 24\nB1 68\nE5 04\n92 70\nFF FC" },
        /* 30*/ { BARCODE_DATAMATRIX, "[91]12[92]34", NULL, NULL, NULL, GS1_MODE, GS1_GS_SEPARATOR, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 31*/ { BARCODE_DATAMATRIX, "[9\\x31]12[92]34", NULL, NULL, NULL, GS1_MODE | ESCAPE_MODE, GS1_GS_SEPARATOR, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 32*/ { BARCODE_DATAMATRIX, "(9\\x31)12(92)34", NULL, NULL, NULL, GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, GS1_GS_SEPARATOR, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 33*/ { BARCODE_EANX_CC, "[91]12", NULL, NULL, NULL,    -1, -1, 0, -1, 0, -1, 0, -1, -1, "12345678+12", -1, -1, 0, -1, "DB BC D3 9C 44 E9 D2 2C 19 E7 A2 D8 A0 00 00 00\nDB 31 1C 9C C7 29 92 47 D9 E9 40 C8 A0 00 00 00\nDA 3B EB 10 AF 09 9A 18 9D 7D 82 E8 A0 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n20 00 00 00 00 00 00 00 00 00 00 00 20 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n14 68 D1 A6 49 BD 55 C9 D4 22 48 B9 40 59 94 98" },
        /* 34*/ { BARCODE_EANX_CC, "[91]12", NULL, NULL, NULL,    -1, -1, 0, -1, 0, -1, 0, -1,  2, "12345678+12", -1, -1, 0, -1, "D3 A3 E9 DB F5 C9 DB 43 D9 CB 98 D2 20 00 00 00\nD3 25 0F 11 E4 49 D3 51 F1 AC FC D6 20 00 00 00\nD1 33 48 19 39 E9 93 18 49 D8 98 D7 20 00 00 00\nD1 A6 FC DA 1C 49 9B C5 05 E2 84 D7 A0 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n20 00 00 00 00 00 00 00 00 00 00 00 20 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n14 68 D1 A6 49 BD 55 C9 D4 22 48 B9 40 59 94 98" },
        /* 35*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1,  1, 0, -1, "Warning 760: Converted to Shift JIS but no ECI specified\nFE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 36*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, 0, 26, 0, -1, -1, NULL, -1,  1, 0, -1, "FE 5B F8\n82 72 08\nBA DA E8\nBA 52 E8\nBA 2A E8\n82 0A 08\nFE AB F8\n00 D8 00\nEF F6 20\nB5 C2 28\n36 28 88\nFD 42 10\n62 2A C8\n00 95 70\nFE B7 38\n82 FD D8\nBA 97 00\nBA 43 60\nBA C8 C8\n82 C3 68\nFE EA F8" },
        /* 37*/ { BARCODE_QRCODE, "\223\137", NULL, NULL, NULL, DATA_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 0A 08\nBA A2 E8\nBA 0A E8\nBA 5A E8\n82 72 08\nFE AB F8\n00 A0 00\nEF AE 20\n75 B5 20\n82 F7 58\nF4 9D C8\n5E 17 28\n00 C2 20\nFE 88 80\n82 82 38\nBA EA A8\nBA 55 50\nBA D7 68\n82 BD D0\nFE B7 78" },
        /* 38*/ { BARCODE_QRCODE, "\223\137", NULL, NULL, NULL, DATA_MODE, -1, 0, -1, 0, -1, 1, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 39*/ { BARCODE_QRCODE, "\\x93\\x5F", NULL, NULL, NULL, DATA_MODE | ESCAPE_MODE, -1, 0, -1, 0, -1, 1, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 40*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, 0, -1, 0, 2, -1, NULL, -1,  1, 0, -1, "Warning 760: Converted to Shift JIS but no ECI specified\nFE 4B F8\n82 92 08\nBA 42 E8\nBA 92 E8\nBA 3A E8\n82 EA 08\nFE AB F8\n00 38 00\nFB CD 50\nA5 89 18\n0B 74 B8\nFC 81 A0\n92 34 B8\n00 DE 48\nFE AB 10\n82 5E 50\nBA C9 20\nBA C9 20\nBA F4 E0\n82 81 A0\nFE B4 E8" },
        /* 41*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, 0, -1, 1, -1, -1, NULL, -1, -1, 0, -1, "FE 8A FE\n80 28 02\nBE E8 FA\nA0 94 0A\nAE 3E EA\nAE D2 EA\nAE 74 EA\n00 AA 00\n15 B4 80\n0B 48 74\nA2 4A A4\nB5 56 2C\nA8 5A A8\n9F 18 50\n02 07 50\n00 A6 00\nFE 20 EA\n02 C2 EA\nFA C4 EA\n0A 42 0A\nEA 52 FA\nEA 24 02\nEA AA FE" },
        /* 42*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, 0, -1, 1, 3, -1, NULL, -1, -1, 0, -1, "FE 16 FE\n80 E2 02\nBE C2 FA\nA0 A0 0A\nAE F6 EA\nAE 98 EA\nAE BA EA\n00 E0 00\n15 83 80\n44 7E AE\n92 9C 78\n25 BF 08\n47 4B 8C\n0D F9 74\n03 E7 50\n00 3A 00\nFE C2 EA\n02 22 EA\nFA DA EA\n0A 22 0A\nEA B2 FA\nEA 9A 02\nEA E8 FE" },
        /* 43*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, 0, -1, 1, 4, -1, NULL, -1, -1, 0, -1, "FE 8A FE\n80 28 02\nBE E8 FA\nA0 94 0A\nAE 3E EA\nAE D2 EA\nAE 74 EA\n00 AA 00\n15 B4 80\n0B 48 74\nA2 4A A4\nB5 56 2C\nA8 5A A8\n9F 18 50\n02 07 50\n00 A6 00\nFE 20 EA\n02 C2 EA\nFA C4 EA\n0A 42 0A\nEA 52 FA\nEA 24 02\nEA AA FE" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];

    char *input1_filename = "test_dump_args1.txt";
    char *input2_filename = "test_dump_args2.txt";
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
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s) (%s)\n", i, buf, data[i].expected, cmd);

        if (have_input1) {
            assert_zero(testUtilRemove(input1_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, input1_filename, errno, strerror(errno));
        }
        if (have_input2) {
            assert_zero(testUtilRemove(input2_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, input2_filename, errno, strerror(errno));
        }
    }

    testFinish();
}

/* Tests segs */
static void test_dump_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        char *data;
        char *data_seg1;
        char *data_seg2;
        int eci;
        int eci_seg1;
        int eci_seg2;

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ {              -1, "123", NULL, NULL, -1, -1, -1, "D3 96 72 F7 65 C9 61 8E B" },
        /*  1*/ {              -1, "123", NULL, NULL, -1, 3, -1, "Error 166: Invalid segment argument, expect \"ECI,DATA\"" },
        /*  2*/ {              -1, "123", "456", NULL, -1, -1, -1, "Error 167: Invalid segment ECI (digits only)" },
        /*  3*/ {              -1, "123", "456", NULL, -1, 1000000, -1, "Error 168: Segment ECI code '1000000' out of range (0 to 999999)" },
        /*  4*/ {              -1, "123", "456", NULL, -1, 3, -1, "Error 775: Symbology does not support multiple segments" },
        /*  5*/ {   BARCODE_AZTEC, "123", "456", NULL, -1, 3, -1, "2B 7A\nC7 02\nF0 6E\n3F FE\n70 1C\nB7 D6\nB4 58\n15 54\n94 56\nB7 DC\n30 1A\n1F FC\n4C 66\n22 DA\n1E C6" },
        /*  6*/ {   BARCODE_AZTEC, "123", NULL, "789", -1, -1, 3, "Error 172: Segments must be consecutive - segment 1 missing" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];

    testStart("test_dump_segs");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        strcpy(cmd, "zint --dump");
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        arg_int(cmd, "-b ", data[i].b);

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

        if (data[i].data_seg2 && data[i].data_seg2[0]) {
            arg_seg(cmd, "--seg2=", data[i].data_seg2, data[i].eci_seg2);
        } else if (data[i].eci_seg2 >= 0) {
            arg_seg(cmd, "--seg2=", NULL, data[i].eci_seg2);
        }

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s) (%s)\n", i, buf, data[i].expected, cmd);
    }

    testFinish();
}

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

#define TEST_INPUT_LONG "test_67890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"

#define TEST_MIRRORED_DIR_LONG "testdir_9012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789/"
#define TEST_MIRRORED_DIR_TOO_LONG "testdir_901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901/"

#ifndef _WIN32
#define TEST_INPUT_AMPERSAND_EXPECTED "***1.gif\000***2.gif"
#else
#define TEST_INPUT_AMPERSAND_EXPECTED "+++1.gif\000+++2.gif"
#endif

    struct item {
        int b;
        int batch;
        int input_mode;
        int mirror;
        char *filetype;
        char *input_filename;
        char *input;
        char *outfile;

        int num_expected;
        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, 1, -1, 0, "gif", NULL, "123\n456\n", "", 2, "00001.gif\00000002.gif" },
        /*  1*/ { BARCODE_CODE128, 1, -1, 0, "gif", NULL, "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n", "~~~.gif", 10, "001.gif\000002.gif\000003.gif\000004.gif\000005.gif\000006.gif\000007.gif\000008.gif\000009.gif\000010.gif" },
        /*  2*/ { BARCODE_CODE128, 1, -1, 0, "gif", NULL, "123\n456\n", "@@@@.gif", 2, TEST_INPUT_AMPERSAND_EXPECTED },
        /*  3*/ { BARCODE_CODE128, 1, -1, 0, "gif", NULL, "123\n456\n789\n", "#####.gif", 3, "    1.gif\000    2.gif\000    3.gif" },
        /*  4*/ { BARCODE_CODE128, 1, -1, 0, "gif", NULL, "123\n456\n", "test_batch~.gif", 2, "test_batch1.gif\000test_batch2.gif" },
        /*  5*/ { BARCODE_CODE128, 1, -1, 1, "gif", NULL, "123\n456\n7890123456789\n", NULL, 3, "123.gif\000456.gif\0007890123456789.gif" },
        /*  6*/ { BARCODE_CODE128, 1, -1, 1, "gif", NULL, "123\n456\n7890123456789\n", "test_input_dir/", 3, "test_input_dir/123.gif\000test_input_dir/456.gif\000test_input_dir/7890123456789.gif" },
        /*  7*/ { BARCODE_CODE128, 1, -1, 1, "svg", NULL, "123\n456\n7890123456789\n", NULL, 3, "123.svg\000456.svg\0007890123456789.svg" },
        /*  8*/ { BARCODE_CODE128, 1, -1, 1, "gif", NULL, "123\n456\n7890123456789\nA\\xA0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_xA0B.gif" },
        /*  9*/ { BARCODE_CODE128, 1, ESCAPE_MODE, 1, "gif", NULL, "123\n456\n7890123456789\nA\\xA0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_B.gif" },
        /* 10*/ { BARCODE_CODE128, 1, -1, 1, "gif", NULL, "123\n456\n7890123456789\nA\\u00A0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_u00A0B.gif" },
        /* 11*/ { BARCODE_CODE128, 1, ESCAPE_MODE, 1, "gif", NULL, "123\n456\n7890123456789\nA\\u00A0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_B.gif" },
        /* 12*/ { BARCODE_CODE128, 1, -1, 1, "gif", NULL, "1!2\"3*\n/:45<6>\n?7890\\\\12345|6789\177\nA\\U0000A0B\n", NULL, 4, "1_2_3_.gif\000__45_6_.gif\000_7890__12345_6789_.gif\000A_U0000A0B.gif" },
        /* 13*/ { BARCODE_CODE128, 1, ESCAPE_MODE, 1, "gif", NULL, "!\"*\n/:45<6>\n?7890\\\\12345|6789\177\nA\\U0000A0B\n", NULL, 4, "___.gif\000__45_6_.gif\000_7890_12345_6789_.gif\000A_B.gif" },
        /* 14*/ { BARCODE_CODE128, 1, -1, 1, "gif", NULL, "1\\d123*9\n\\o1234:5\n#$%&'()+,-.;=@[]^`{}\n", NULL, 3, "1_d123_9.gif\000_o1234_5.gif\000#$%&'()+,-.;=@[]^`{}.gif" },
        /* 15*/ { BARCODE_CODE128, 1, ESCAPE_MODE, 1, "gif", NULL, "1\\d123*2\n\\o1234:5\n#$%&'()+,-.;=@[]^`{}\n", NULL, 3, "1__2.gif\000_4_5.gif\000#$%&'()+,-.;=@[]^`{}.gif" },
        /* 16*/ { BARCODE_CODE128, 1, -1, 0, "gif", NULL, "\n", "test_batch.gif", 0, NULL },
        /* 17*/ { BARCODE_CODE128, 1, -1, 0, "gif", NULL, "123\n456\n", TEST_INPUT_LONG "~.gif", 2, TEST_INPUT_LONG "1.gif\000" TEST_INPUT_LONG "2.gif" },
        /* 18*/ { BARCODE_CODE128, 0, -1, 0, "svg", NULL, "123", TEST_INPUT_LONG "1.gif", 1, TEST_INPUT_LONG "1.svg" },
        /* 19*/ { BARCODE_CODE128, 1, -1, 0, "svg", NULL, "123\n", TEST_INPUT_LONG "1.gif", 1, TEST_INPUT_LONG "1.svg" },
        /* 20*/ { BARCODE_CODE128, 1, -1, 0, "gif", NULL, "123\n", "test_batch.jpeg", 1, "test_batch.jpeg.gif" },
        /* 21*/ { BARCODE_CODE128, 1, -1, 0, "gif", NULL, "123\n", "test_batch.jpg", 1, "test_batch.gif" },
        /* 22*/ { BARCODE_CODE128, 1, -1, 0, "emf", NULL, "123\n", "test_batch.jpeg", 1, "test_batch.jpeg.emf" },
        /* 23*/ { BARCODE_CODE128, 1, -1, 0, "emf", NULL, "123\n", "test_batch.jpg", 1, "test_batch.emf" },
        /* 24*/ { BARCODE_CODE128, 1, -1, 0, "eps", NULL, "123\n", "test_batch.ps", 1, "test_batch.eps" },
        /* 25*/ { BARCODE_CODE128, 1, -1, 1, "gif", NULL, "1234567890123456789012345678901\n1234567890123456789012345678902\n", TEST_MIRRORED_DIR_LONG, 2, TEST_MIRRORED_DIR_LONG "1234567890123456789012345678901.gif\000" TEST_MIRRORED_DIR_LONG "1234567890123456789012345678902.gif" },
        /* 26*/ { BARCODE_CODE128, 1, -1, 1, "gif", NULL, "123\n456\n", TEST_MIRRORED_DIR_TOO_LONG, 2, "123.gif\000456.gif" },
        /* 27*/ { BARCODE_CODE128, 1, -1, 0, "gif", "testé_input.txt", "123\n456\n", "", 2, "00001.gif\00000002.gif" },
        /* 28*/ { BARCODE_CODE128, 1, -1, 0, "gif", "testก_input.txt", "123\n456\n", "test_input_δir/testé~~~.gif", 2, "test_input_δir/testé001.gif\000test_input_δir/testé002.gif" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];

    char *input_filename;
    char *outfile;

    testStart("test_input");

    for (i = 0; i < data_size; i++) {
        int j;
        char *slash;

        if (testContinue(p_ctx, i)) continue;
#ifdef _WIN32
        if (data[i].outfile && (int) strlen(data[i].outfile) > 50) {
            if (debug & ZINT_DEBUG_TEST_PRINT) printf("%d not Windows compatible (outfile length %d > 50)\n", i, (int) strlen(data[i].outfile));
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
        arg_data(cmd, "--filetype=", data[i].filetype);
        input_filename = data[i].input_filename ? data[i].input_filename : "test_input.txt";
        arg_input(cmd, input_filename, data[i].input);
        arg_data(cmd, "-o ", data[i].outfile);

        if (!data[i].expected
                || (data[i].batch && data[i].mirror && data[i].outfile && data[i].outfile[0]
                    && strcmp(data[i].outfile, TEST_MIRRORED_DIR_LONG) == 0)) {
            printf("++++ Following %s expected, ignore: ", data[i].expected ? "warning" : "error");
            fflush(stdout);
        }
        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);

        outfile = data[i].expected;
        for (j = 0; j < data[i].num_expected; j++) {
            assert_nonzero(testUtilExists(outfile), "i:%d j:%d testUtilExists(%s) != 1\n", i, j, outfile);
            assert_zero(testUtilRemove(outfile), "i:%d j:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, j, outfile, errno, strerror(errno));
            outfile += strlen(outfile) + 1;
        }

        assert_zero(testUtilRemove(input_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, input_filename, errno, strerror(errno));

        /* Remove directory if any */
        if (data[i].outfile && (slash = strrchr(data[i].outfile, '/')) != NULL && strcmp(data[i].outfile, TEST_MIRRORED_DIR_TOO_LONG) != 0) {
            char dirpath[256];
            assert_nonzero((size_t) (slash - data[i].outfile) < sizeof(dirpath), "i: %d output directory too long\n", i);
            strncpy(dirpath, data[i].outfile, slash - data[i].outfile);
            dirpath[slash - data[i].outfile] = '\0';
            assert_zero(testUtilRmDir(dirpath), "i:%d testUtilRmDir(%s) != 0 (%d: %s)\n", i, dirpath, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_stdin_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        char *data;
        char *input;
        char *outfile;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "123", "-", "test_stdin_input.gif" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];

    char *input_filename = "-";

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
        assert_zero(testUtilRemove(data[i].outfile), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, data[i].outfile, errno, strerror(errno));
    }

    testFinish();
}

/* Note ordering of `--batch` before/after data/input args affects error messages */
static void test_batch_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        char *data;
        char *input;
        char *input2;

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "123", NULL, NULL, "Warning 122: Can't define data in batch mode, ignoring '123'\nWarning 124: No data received, no symbol generated" },
        /*  1*/ { BARCODE_CODE128, "123", "123\n456\n", NULL, "Warning 122: Can't define data in batch mode, ignoring '123'\nD3 96 72 F7 65 C9 61 8E B\nD3 97 62 F7 67 49 19 8E B" },
        /*  2*/ { BARCODE_CODE128, NULL, "123\n456\n", "789\n", "Warning 143: Can only define one input file in batch mode, ignoring 'test_batch_input2.txt'\nD3 96 72 F7 65 C9 61 8E B\nD3 97 62 F7 67 49 19 8E B" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];

    char *input1_filename = "test_batch_input1.txt";
    char *input2_filename = "test_batch_input2.txt";
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

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s)\n", i, buf, data[i].expected);

        if (have_input1) {
            assert_zero(testUtilRemove(input1_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, input1_filename, errno, strerror(errno));
        }
        if (have_input2) {
            assert_zero(testUtilRemove(input2_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, input2_filename, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_batch_large(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        int mirror;
        char *pattern;
        int length;

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_HANXIN, 0, "1", 7827, "00001.gif" },
        /*  1*/ { BARCODE_HANXIN, 1, "1", 7827, "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111.gif" },
        /*  2*/ { BARCODE_HANXIN, 0, "1", 7828, NULL },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[16384];
    char data_buf[8192];
    char buf[16384];

    char *input_filename = "test_batch_large.txt";
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
        strcat(data_buf, "\n");
        have_input = arg_input(cmd, input_filename, data_buf);

        if (!data[i].expected) {
            printf("++++ Following error expected, ignore: ");
            fflush(stdout);
        }
        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        if (data[i].expected) {
            assert_zero(testUtilRemove(data[i].expected), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, data[i].expected, errno, strerror(errno));
        } else {
            assert_zero(testUtilExists("out.gif"), "i:%d testUtilExists(out.gif) != 0 (%d: %s) (%s)\n", i, errno, strerror(errno), cmd);
        }

        if (have_input) {
            assert_zero(testUtilRemove(input_filename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, input_filename, errno, strerror(errno));
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
        char *filetype;
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

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { -2, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 139: Invalid add-on gap value (digits only)" },
        /*  1*/ {  6, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 140: Add-on gap '6' out of range (7 to 12), ignoring" },
        /*  2*/ { 13, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 140: Add-on gap '13' out of range (7 to 12), ignoring" },
        /*  3*/ { -1, -2,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 107: Invalid border width value (digits only)" },
        /*  4*/ { -1, 1001, -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 108: Border width '1001' out of range (0 to 1000), ignoring" },
        /*  5*/ { -1, -1,   -1, -1, -5.1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 195: Text gap '-5.1' out of range (-5 to 10), ignoring" },
        /*  6*/ { -1, -1,   -1, -1, 10.01,   -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 195: Text gap '10.01' out of range (-5 to 10), ignoring" },
        /*  7*/ { -1, -1,   -1, 12345678,  -1, -1,    NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 181: Invalid dot radius floating point (integer part must be 7 digits maximum)" },
        /*  8*/ { -1, -1,   -1, 0.009, -1,   -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 106: Invalid dot radius value (less than 0.01), ignoring" },
        /*  9*/ { -1, -1,   -2, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 131: Invalid columns value (digits only)" },
        /* 10*/ { -1, -1,  201, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 111: Number of columns '201' out of range (1 to 200), ignoring" },
        /* 11*/ { -1, -1,   -1, -1,   -1,    -2,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 138: Invalid ECI code (digits only)" },
        /* 12*/ { -1, -1,   -1, -1,   -1,    1000000, NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 118: ECI code '1000000' out of range (0 to 999999), ignoring" },
        /* 13*/ { -1, -1,   -1, -1,   -1,    -1,      "jpg", -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 142: File type 'jpg' not supported, ignoring" },
        /* 14*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -2,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 183: Invalid symbol height floating point (negative value not permitted)" },
        /* 15*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,   0,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 110: Symbol height '0' out of range (0.5 to 2000), ignoring" },
        /* 16*/ { -1, -1,   -1, -1,   -1,    -1,      NULL, 2001,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 110: Symbol height '2001' out of range (0.5 to 2000), ignoring" },
        /* 17*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -2, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 182: Invalid guard bar descent floating point (negative value not permitted)" },
        /* 18*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1, 50.1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 135: Guard bar descent '50.1' out of range (0 to 50), ignoring" },
        /* 19*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -2, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 148: Invalid mask value (digits only)" },
        /* 20*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1,  8, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 147: Mask value '8' out of range (0 to 7), ignoring" },
        /* 21*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1,  7, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 116: Mode value '7' out of range (0 to 6), ignoring" },
        /* 22*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -2, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 117: Invalid rotation value (digits only)" },
        /* 23*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, 45, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 137: Rotation value '45' out of range (0, 90, 180 or 270 only), ignoring" },
        /* 24*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -2, -1,   -1, -1, -1, -1,   -1,   -1, "Error 132: Invalid rows value (digits only)" },
        /* 25*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, 91, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 112: Number of rows '91' out of range (1 to 90), ignoring" },
        /* 26*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -2,   -1, -1, -1, -1,   -1,   -1, "Error 184: Invalid scale floating point (negative value not permitted)" },
        /* 27*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, 0.49, -1, -1, -1, -1,   -1,   -1, "Warning 146: Scaling less than 0.5 will be set to 0.5 for 'gif' output" },
        /* 28*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -2, -1, -1, -1,   -1,   -1, "Error 149: Invalid Structured Carrier Message version value (digits only)" },
        /* 29*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,  100, -1, -1, -1,   -1,   -1, "Warning 150: Structured Carrier Message version '100' out of range (0 to 99), ignoring" },
        /* 30*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -2, -1, -1,   -1,   -1, "Error 134: Invalid ECC value (digits only)" },
        /* 31*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1,  9, -1, -1,   -1,   -1, "Warning 114: ECC level '9' out of range (0 to 8), ignoring" },
        /* 32*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -2, -1,   -1,   -1, "Error 128: Invalid separator value (digits only)" },
        /* 33*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1,  5, -1,   -1,   -1, "Warning 127: Separator value '5' out of range (0 to 4), ignoring" },
        /* 34*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -2,   -1,   -1, "Error 133: Invalid version value (digits only)" },
        /* 35*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, 1000, -1,   -1, "Warning 113: Version value '1000' out of range (1 to 999), ignoring" },
        /* 36*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -2,   -1, "Error 153: Invalid vertical whitespace value '-2' (digits only)" },
        /* 37*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, 1001,   -1, "Warning 154: Vertical whitespace value '1001' out of range (0 to 1000), ignoring" },
        /* 38*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -2, "Error 120: Invalid horizontal whitespace value '-2' (digits only)" },
        /* 39*/ { -1, -1,   -1, -1,   -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1, 1001, "Warning 121: Horizontal whitespace value '1001' out of range (0 to 1000), ignoring" },
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

        arg_int(cmd, "--addongap=", data[i].addongap);
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
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s) (%s)\n", i, buf, data[i].expected, cmd);

        if (strncmp(data[i].expected, "Warning", 7) == 0) {
            assert_zero(testUtilRemove(outfilename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, outfilename, errno, strerror(errno));
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
        /*  3*/ { "code218", "1", NULL, 1, "Error 119: Invalid barcode type 'code218'" },
        /*  4*/ { "code12", "1", NULL, 1, "Error 119: Invalid barcode type 'code12'" },
        /*  5*/ { "BARCODE_CODE11", "1", NULL, 0, "BARCODE_CODE11 (1)," },
        /*  6*/ { "C25 Standard", "1", NULL, 0, "BARCODE_C25STANDARD (2)," },
        /*  7*/ { "c25matrix", "1", NULL, 0, "BARCODE_C25STANDARD (2)," }, /* Legacy now supported */
        /*  8*/ { "2 of 5 Standard", "1", NULL, 0, "BARCODE_C25STANDARD (2)," }, /* Synonym */
        /*  9*/ { "2 of 5 Matrix", "1", NULL, 0, "BARCODE_C25STANDARD (2)," }, /* Synonym */
        /* 10*/ { "Code 2 of 5 Standard", "1", NULL, 0, "BARCODE_C25STANDARD (2)," }, /* Synonym */
        /* 11*/ { "Code 2 of 5 Matrix", "1", NULL, 0, "BARCODE_C25STANDARD (2)," }, /* Synonym */
        /* 12*/ { "Standard Code 2 of 5", "1", NULL, 0, "BARCODE_C25STANDARD (2)," }, /* Synonym */
        /* 13*/ { "C25INTER", "1", NULL, 0, "BARCODE_C25INTER (3)," },
        /* 14*/ { "c25 interleaved", "1", NULL, 0, "BARCODE_C25INTER (3)," }, /* Synonym */
        /* 15*/ { "code 2 of 5 inter", "1", NULL, 0, "BARCODE_C25INTER (3)," }, /* Synonym */
        /* 16*/ { "code 2 of 5 interleaved", "1", NULL, 0, "BARCODE_C25INTER (3)," }, /* Synonym */
        /* 17*/ { "2 of 5 inter", "1", NULL, 0, "BARCODE_C25INTER (3)," }, /* Synonym */
        /* 18*/ { "2 of 5 interleaved", "1", NULL, 0, "BARCODE_C25INTER (3)," }, /* Synonym */
        /* 19*/ { "interleaved 2 of 5", "1", NULL, 0, "BARCODE_C25INTER (3)," }, /* Synonym */
        /* 20*/ { "interleaved code 2 of 5", "1", NULL, 0, "BARCODE_C25INTER (3)," }, /* Synonym */
        /* 21*/ { "c25IATA", "1", NULL, 0, "BARCODE_C25IATA (4)," },
        /* 22*/ { "2of5IATA", "1", NULL, 0, "BARCODE_C25IATA (4)," }, /* Synonym */
        /* 23*/ { "code2of5IATA", "1", NULL, 0, "BARCODE_C25IATA (4)," }, /* Synonym */
        /* 24*/ { "IATA2of5", "1", NULL, 0, "BARCODE_C25IATA (4)," }, /* Synonym */
        /* 25*/ { "IATAcode2of5", "1", NULL, 0, "BARCODE_C25IATA (4)," }, /* Synonym */
        /* 26*/ { "c25 Logic", "1", NULL, 0, "BARCODE_C25LOGIC (6)," },
        /* 27*/ { "c25 Data Logic", "1", NULL, 0, "BARCODE_C25LOGIC (6)," }, /* Synonym */
        /* 28*/ { "Code 2 of 5 Logic", "1", NULL, 0, "BARCODE_C25LOGIC (6)," }, /* Synonym */
        /* 29*/ { "Code 2 of 5 Data Logic", "1", NULL, 0, "BARCODE_C25LOGIC (6)," }, /* Synonym */
        /* 30*/ { "2 of 5 Logic", "1", NULL, 0, "BARCODE_C25LOGIC (6)," }, /* Synonym */
        /* 31*/ { "2 of 5 Data Logic", "1", NULL, 0, "BARCODE_C25LOGIC (6)," }, /* Synonym */
        /* 32*/ { "c25 Ind", "1", NULL, 0, "BARCODE_C25IND (7)," },
        /* 33*/ { "c25 Industrial", "1", NULL, 0, "BARCODE_C25IND (7)," }, /* Synonym */
        /* 34*/ { "code 2 of 5 Ind", "1", NULL, 0, "BARCODE_C25IND (7)," }, /* Synonym */
        /* 35*/ { "code 2 of 5 Industrial", "1", NULL, 0, "BARCODE_C25IND (7)," }, /* Synonym */
        /* 36*/ { "2 of 5 Ind", "1", NULL, 0, "BARCODE_C25IND (7)," }, /* Synonym */
        /* 37*/ { "2 of 5 Industrial", "1", NULL, 0, "BARCODE_C25IND (7)," }, /* Synonym */
        /* 38*/ { "Industrial 2 of 5", "1", NULL, 0, "BARCODE_C25IND (7)," }, /* Synonym */
        /* 39*/ { "Industrial code 2 of 5", "1", NULL, 0, "BARCODE_C25IND (7)," }, /* Synonym */
        /* 40*/ { "code39", "1", NULL, 0, "BARCODE_CODE39 (8)," },
        /* 41*/ { "excode 39", "1", NULL, 0, "BARCODE_EXCODE39 (9)," },
        /* 42*/ { "Extended Code 39", "1", NULL, 0, "BARCODE_EXCODE39 (9)," },
        /* 43*/ { "eanx", "1", NULL, 0, "BARCODE_EANX (13)," },
        /* 44*/ { "ean", "1", NULL, 0, "BARCODE_EANX (13)," },
        /* 45*/ { "eanx chk", "1", NULL, 0, "BARCODE_EANX_CHK (14)," },
        /* 46*/ { "eanxchk", "1", NULL, 0, "BARCODE_EANX_CHK (14)," },
        /* 47*/ { "eanchk", "1", NULL, 0, "BARCODE_EANX_CHK (14)," },
        /* 48*/ { "GS1128", "[01]12345678901231", NULL, 0, "BARCODE_GS1_128 (16)," },
        /* 49*/ { "ean 128", "[01]12345678901231", NULL, 0, "BARCODE_GS1_128 (16)," },
        /* 50*/ { "coda bar", "A1B", NULL, 0, "BARCODE_CODABAR (18)," },
        /* 51*/ { "DPLEIT", "1", NULL, 0, "BARCODE_DPLEIT (21)," },
        /* 52*/ { "DPIDENT", "1", NULL, 0, "BARCODE_DPIDENT (22)," },
        /* 53*/ { "code16k", "1", NULL, 0, "BARCODE_CODE16K (23)," },
        /* 54*/ { "CODE49", "1", NULL, 0, "BARCODE_CODE49 (24)," },
        /* 55*/ { "CODE93", "1", NULL, 0, "BARCODE_CODE93 (25)," },
        /* 56*/ { "flat", "1", NULL, 0, "BARCODE_FLAT (28)," },
        /* 57*/ { "dbar omn", "1", NULL, 0, "BARCODE_DBAR_OMN (29)," },
        /* 58*/ { "dbar omni", "1", NULL, 0, "BARCODE_DBAR_OMN (29)," },
        /* 59*/ { "rss14", "1", NULL, 0, "BARCODE_DBAR_OMN (29)," },
        /* 60*/ { "databar omn", "1", NULL, 0, "BARCODE_DBAR_OMN (29)," },
        /* 61*/ { "databar omni", "1", NULL, 0, "BARCODE_DBAR_OMN (29)," },
        /* 62*/ { "dbar ltd", "1", NULL, 0, "BARCODE_DBAR_LTD (30)," },
        /* 63*/ { "dbar limited", "1", NULL, 0, "BARCODE_DBAR_LTD (30)," },
        /* 64*/ { "rss ltd", "1", NULL, 0, "BARCODE_DBAR_LTD (30)," },
        /* 65*/ { "databar ltd", "1", NULL, 0, "BARCODE_DBAR_LTD (30)," },
        /* 66*/ { "databar limited", "1", NULL, 0, "BARCODE_DBAR_LTD (30)," },
        /* 67*/ { "dbarexp", "[10]12", NULL, 0, "BARCODE_DBAR_EXP (31)," },
        /* 68*/ { "dbarexpanded", "[10]12", NULL, 0, "BARCODE_DBAR_EXP (31)," },
        /* 69*/ { "rss exp", "[10]12", NULL, 0, "BARCODE_DBAR_EXP (31)," },
        /* 70*/ { "databarexp", "[10]12", NULL, 0, "BARCODE_DBAR_EXP (31)," },
        /* 71*/ { "databarexpanded", "[10]12", NULL, 0, "BARCODE_DBAR_EXP (31)," },
        /* 72*/ { "telepen", "1", NULL, 0, "BARCODE_TELEPEN (32)," },
        /* 73*/ { "upc", "1", NULL, 1, "Error 119: Invalid barcode type 'upc'" },
        /* 74*/ { "upca", "1", NULL, 0, "BARCODE_UPCA (34)," },
        /* 75*/ { "upca_chk", "123456789012", NULL, 0, "BARCODE_UPCA_CHK (35)," },
        /* 76*/ { "upce", "1", NULL, 0, "BARCODE_UPCE (37)," },
        /* 77*/ { "upce chk", "12345670", NULL, 0, "BARCODE_UPCE_CHK (38)," },
        /* 78*/ { "POSTNET ", "12345678901", NULL, 0, "BARCODE_POSTNET (40)," },
        /* 79*/ { "msi", "1", NULL, 0, "BARCODE_MSI_PLESSEY (47)," },
        /* 80*/ { "MSI Plessey ", "1", NULL, 0, "BARCODE_MSI_PLESSEY (47)," },
        /* 81*/ { "fim ", "A", NULL, 0, "BARCODE_FIM (49)," },
        /* 82*/ { "LOGMARS", "123456", NULL, 0, "BARCODE_LOGMARS (50)," },
        /* 83*/ { " pharma", "123456", NULL, 0, "BARCODE_PHARMA (51)," },
        /* 84*/ { " pzn ", "1", NULL, 0, "BARCODE_PZN (52)," },
        /* 85*/ { "pharma two", "4", NULL, 0, "BARCODE_PHARMA_TWO (53)," },
        /* 86*/ { "cepnet", "12345678", NULL, 0, "BARCODE_CEPNET (54)," },
        /* 87*/ { "BARCODE_PDF417", "1", NULL, 0, "BARCODE_PDF417 (55)," },
        /* 88*/ { "pdf", "1", NULL, 1, "Error 119: Invalid barcode type 'pdf'" },
        /* 89*/ { "barcodepdf417comp", "1", NULL, 0, "BARCODE_PDF417COMP (56)," },
        /* 90*/ { "pdf417trunc", "1", NULL, 0, "BARCODE_PDF417COMP (56)," },
        /* 91*/ { "MaxiCode", "1", NULL, 0, "BARCODE_MAXICODE (57)," },
        /* 92*/ { "QR CODE", "1", NULL, 0, "BARCODE_QRCODE (58)," },
        /* 93*/ { "qr", "1", NULL, 0, "BARCODE_QRCODE (58)," }, /* Synonym */
        /* 94*/ { "Code 128 B", "1", NULL, 0, "BARCODE_CODE128AB (60)," },
        /* 95*/ { "Code 128 aB", "1", NULL, 0, "BARCODE_CODE128AB (60)," },
        /* 96*/ { "AUS POST", "12345678901234567890123", NULL, 0, "BARCODE_AUSPOST (63)," },
        /* 97*/ { "AusReply", "12345678", NULL, 0, "BARCODE_AUSREPLY (66)," },
        /* 98*/ { "AUSROUTE", "12345678", NULL, 0, "BARCODE_AUSROUTE (67)," },
        /* 99*/ { "AUS REDIRECT", "12345678", NULL, 0, "BARCODE_AUSREDIRECT (68)," },
        /*100*/ { "isbnx", "123456789", NULL, 0, "BARCODE_ISBNX (69)," },
        /*101*/ { "rm4scc", "1", NULL, 0, "BARCODE_RM4SCC (70)," },
        /*102*/ { "DataMatrix", "1", NULL, 0, "BARCODE_DATAMATRIX (71)," },
        /*103*/ { "EAN14", "1", NULL, 0, "BARCODE_EAN14 (72)," },
        /*104*/ { "vin", "12345678701234567", NULL, 0, "BARCODE_VIN (73)" },
        /*105*/ { "CodaBlock-F", "1", NULL, 0, "BARCODE_CODABLOCKF (74)," },
        /*106*/ { "NVE18", "1", NULL, 0, "BARCODE_NVE18 (75)," },
        /*107*/ { "Japan Post", "1", NULL, 0, "BARCODE_JAPANPOST (76)," },
        /*108*/ { "Korea Post", "1", NULL, 0, "BARCODE_KOREAPOST (77)," },
        /*109*/ { "DBar Stk", "1", NULL, 0, "BARCODE_DBAR_STK (79)," },
        /*110*/ { "DBar Stacked", "1", NULL, 0, "BARCODE_DBAR_STK (79)," },
        /*111*/ { "rss14stack", "1", NULL, 0, "BARCODE_DBAR_STK (79)," },
        /*112*/ { "DataBar Stk", "1", NULL, 0, "BARCODE_DBAR_STK (79)," },
        /*113*/ { "DataBar Stacked", "1", NULL, 0, "BARCODE_DBAR_STK (79)," },
        /*114*/ { "DBar Omn Stk", "1", NULL, 0, "BARCODE_DBAR_OMNSTK (80)," },
        /*115*/ { "DBar Stacked Omni", "1", NULL, 0, "BARCODE_DBAR_OMNSTK (80)," },
        /*116*/ { "RSS14STACK OMNI", "1", NULL, 0, "BARCODE_DBAR_OMNSTK (80)," },
        /*117*/ { "DataBar Omn Stk", "1", NULL, 0, "BARCODE_DBAR_OMNSTK (80)," },
        /*118*/ { "DataBar Stacked Omn", "1", NULL, 0, "BARCODE_DBAR_OMNSTK (80)," },
        /*119*/ { "DataBar Stacked Omni", "1", NULL, 0, "BARCODE_DBAR_OMNSTK (80)," },
        /*120*/ { "DBar Exp Stk", "[20]01", NULL, 0, "BARCODE_DBAR_EXPSTK (81)," },
        /*121*/ { "DBar Expanded Stacked", "[20]01", NULL, 0, "BARCODE_DBAR_EXPSTK (81)," },
        /*122*/ { "rss_expstack", "[20]01", NULL, 0, "BARCODE_DBAR_EXPSTK (81)," },
        /*123*/ { "DataBar Exp Stk", "[20]01", NULL, 0, "BARCODE_DBAR_EXPSTK (81)," },
        /*124*/ { "DataBar Expanded Stk", "[20]01", NULL, 0, "BARCODE_DBAR_EXPSTK (81)," },
        /*125*/ { "DataBar Expanded Stacked", "[20]01", NULL, 0, "BARCODE_DBAR_EXPSTK (81)," },
        /*126*/ { "planet", "12345678901", NULL, 0, "BARCODE_PLANET (82)," },
        /*127*/ { "MicroPDF417", "1", NULL, 0, "BARCODE_MICROPDF417 (84)," },
        /*128*/ { "USPS IMail", "12345678901234567890", NULL, 0, "BARCODE_USPS_IMAIL (85)," },
        /*129*/ { "OneCode", "12345678901234567890", NULL, 0, "BARCODE_USPS_IMAIL (85)," },
        /*130*/ { "plessey", "1", NULL, 0, "BARCODE_PLESSEY (86)," },
        /*131*/ { "telepen num", "1", NULL, 0, "BARCODE_TELEPEN_NUM (87)," },
        /*132*/ { "ITF14", "1", NULL, 0, "BARCODE_ITF14 (89)," },
        /*133*/ { "KIX", "1", NULL, 0, "BARCODE_KIX (90)," },
        /*134*/ { "Aztec", "1", NULL, 0, "BARCODE_AZTEC (92)," },
        /*135*/ { "Aztec Code", "1", NULL, 0, "BARCODE_AZTEC (92)," }, /* Synonym */
        /*136*/ { "daft", "D", NULL, 0, "BARCODE_DAFT (93)," },
        /*137*/ { "DPD", "0123456789012345678901234567", NULL, 0, "BARCODE_DPD (96)," },
        /*138*/ { "Micro QR", "1", NULL, 0, "BARCODE_MICROQR (97)," },
        /*139*/ { "Micro QR Code", "1", NULL, 0, "BARCODE_MICROQR (97)," },
        /*140*/ { "hibc128", "1", NULL, 0, "BARCODE_HIBC_128 (98)," },
        /*141*/ { "hibccode128", "1", NULL, 0, "BARCODE_HIBC_128 (98)," }, /* Synonym */
        /*142*/ { "hibc39", "1", NULL, 0, "BARCODE_HIBC_39 (99)," },
        /*143*/ { "hibccode39", "1", NULL, 0, "BARCODE_HIBC_39 (99)," }, /* Synonym */
        /*144*/ { "hibcdatamatrix", "1", NULL, 0, "BARCODE_HIBC_DM (102)," }, /* Synonym */
        /*145*/ { "hibcdm", "1", NULL, 0, "BARCODE_HIBC_DM (102)," },
        /*146*/ { "HIBC qr", "1", NULL, 0, "BARCODE_HIBC_QR (104)," },
        /*147*/ { "HIBC QR Code", "1", NULL, 0, "BARCODE_HIBC_QR (104)," }, /* Synonym */
        /*148*/ { "HIBCPDF", "1", NULL, 0, "BARCODE_HIBC_PDF (106)," },
        /*149*/ { "HIBCPDF417", "1", NULL, 0, "BARCODE_HIBC_PDF (106)," }, /* Synonym */
        /*150*/ { "HIBCMICPDF", "1", NULL, 0, "BARCODE_HIBC_MICPDF (108)," },
        /*151*/ { "HIBC Micro PDF", "1", NULL, 0, "BARCODE_HIBC_MICPDF (108)," }, /* Synonym */
        /*152*/ { "HIBC Micro PDF417", "1", NULL, 0, "BARCODE_HIBC_MICPDF (108)," }, /* Synonym */
        /*153*/ { "HIBC BlockF", "1", NULL, 0, "BARCODE_HIBC_BLOCKF (110)," },
        /*154*/ { "HIBC CodaBlock-F", "1", NULL, 0, "BARCODE_HIBC_BLOCKF (110)," }, /* Synonym */
        /*155*/ { "HIBC Aztec", "1", NULL, 0, "BARCODE_HIBC_AZTEC (112)," },
        /*156*/ { "DotCode", "1", NULL, 0, "BARCODE_DOTCODE (115)," },
        /*157*/ { "Han Xin", "1", NULL, 0, "BARCODE_HANXIN (116)," },
        /*158*/ { "Mailmark", "01000000000000000AA00AA0A", NULL, 0, "BARCODE_MAILMARK_4S (121)," },
        /*159*/ { "Mailmark 4-state", "01000000000000000AA00AA0A", NULL, 0, "BARCODE_MAILMARK_4S (121)," },
        /*160*/ { "Mailmark 2D", "012100123412345678AB19XY1A 0", NULL, 0, "BARCODE_MAILMARK_2D (119)," },
        /*161*/ { "azrune", "1", NULL, 0, "BARCODE_AZRUNE (128)," },
        /*162*/ { "aztecrune", "1", NULL, 0, "BARCODE_AZRUNE (128)," }, /* Synonym */
        /*163*/ { "aztecrunes", "1", NULL, 0, "BARCODE_AZRUNE (128)," }, /* Synonym */
        /*164*/ { "code32", "1", NULL, 0, "BARCODE_CODE32 (129)," },
        /*165*/ { "eanx cc", "[20]01", "1234567890128", 0, "BARCODE_EANX_CC (130)," },
        /*166*/ { "eancc", "[20]01", "1234567890128", 0, "BARCODE_EANX_CC (130)," },
        /*167*/ { "GS1 128 CC", "[01]12345678901231", "[20]01", 0, "BARCODE_GS1_128_CC (131)," },
        /*168*/ { "EAN128 CC", "[01]12345678901231", "[20]01", 0, "BARCODE_GS1_128_CC (131)," },
        /*169*/ { "dbaromncc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_OMN_CC (132)," },
        /*170*/ { "dbaromnicc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_OMN_CC (132)," },
        /*171*/ { "rss14 cc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_OMN_CC (132)," },
        /*172*/ { "databaromncc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_OMN_CC (132)," },
        /*173*/ { "databaromnicc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_OMN_CC (132)," },
        /*174*/ { "dbarltdcc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_LTD_CC (133)," },
        /*175*/ { "dbarlimitedcc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_LTD_CC (133)," },
        /*176*/ { "rss ltd cc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_LTD_CC (133)," },
        /*177*/ { "databarltdcc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_LTD_CC (133)," },
        /*178*/ { "databarlimitedcc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_LTD_CC (133)," },
        /*179*/ { "dbarexpcc", "[20]01", "[01]12345678901231", 0, "BARCODE_DBAR_EXP_CC (134)," },
        /*180*/ { "rss exp cc", "[20]01", "[01]12345678901231", 0, "BARCODE_DBAR_EXP_CC (134)," },
        /*181*/ { "databarexpcc", "[20]01", "[01]12345678901231", 0, "BARCODE_DBAR_EXP_CC (134)," },
        /*182*/ { "databar expanded cc", "[20]01", "[01]12345678901231", 0, "BARCODE_DBAR_EXP_CC (134)," },
        /*183*/ { "upcacc", "[20]01", "12345678901", 0, "BARCODE_UPCA_CC (135)," },
        /*184*/ { "upcecc", "[20]01", "1234567", 0, "BARCODE_UPCE_CC (136)," },
        /*185*/ { "dbar stk cc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_STK_CC (137)," },
        /*186*/ { "rss14stackcc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_STK_CC (137)," },
        /*187*/ { "databar stk cc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_STK_CC (137)," },
        /*188*/ { "databar stacked cc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_STK_CC (137)," },
        /*189*/ { "dbaromnstkcc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_OMNSTK_CC (138)," },
        /*190*/ { "BARCODE_RSS14_OMNI_CC", "[20]01", "1234567890123", 0, "BARCODE_DBAR_OMNSTK_CC (138)," },
        /*191*/ { "databaromnstkcc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_OMNSTK_CC (138)," },
        /*192*/ { "databar stacked omncc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_OMNSTK_CC (138)," },
        /*193*/ { "databar stacked omni cc", "[20]01", "1234567890123", 0, "BARCODE_DBAR_OMNSTK_CC (138)," },
        /*194*/ { "dbarexpstkcc", "[20]01", "[01]12345678901231", 0, "BARCODE_DBAR_EXPSTK_CC (139)," },
        /*195*/ { "RSS EXPSTACK CC", "[20]01", "[01]12345678901231", 0, "BARCODE_DBAR_EXPSTK_CC (139)," },
        /*196*/ { "databarexpstkcc", "[20]01", "[01]12345678901231", 0, "BARCODE_DBAR_EXPSTK_CC (139)," },
        /*197*/ { "databar expanded stkcc", "[20]01", "[01]12345678901231", 0, "BARCODE_DBAR_EXPSTK_CC (139)," },
        /*198*/ { "databar expanded stacked cc", "[20]01", "[01]12345678901231", 0, "BARCODE_DBAR_EXPSTK_CC (139)," },
        /*199*/ { "Channel", "1", NULL, 0, "BARCODE_CHANNEL (140)," },
        /*200*/ { "Channel Code", "1", NULL, 0, "BARCODE_CHANNEL (140)," },
        /*201*/ { "CodeOne", "1", NULL, 0, "BARCODE_CODEONE (141)," },
        /*202*/ { "Grid Matrix", "1", NULL, 0, "BARCODE_GRIDMATRIX (142)," },
        /*203*/ { "UPN QR", "1", NULL, 0, "BARCODE_UPNQR (143)," },
        /*204*/ { "UPN QR Code", "1", NULL, 0, "BARCODE_UPNQR (143)," }, /* Synonym */
        /*205*/ { "ultra", "1", NULL, 0, "BARCODE_ULTRA (144)," },
        /*206*/ { "ultracode", "1", NULL, 0, "BARCODE_ULTRA (144)," }, /* Synonym */
        /*207*/ { "rMQR", "1", NULL, 0, "BARCODE_RMQR (145)," },
        /*208*/ { "bc412", "1234567", NULL, 0, "BARCODE_BC412 (146)," },
        /*209*/ { "x", "1", NULL, 1, "Error 119: Invalid barcode type 'x'" },
        /*210*/ { "\177", "1", NULL, 1, "Error 119: Invalid barcode type '\177'" },
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
            assert_zero(testUtilRemove(outfilename), "i:%d testUtilRemove(%s) != 0 (%d: %s) (%s)\n", i, outfilename, errno, strerror(errno), cmd);
        }
        assert_nonnull(strstr(buf, data[i].expected), "i:%d strstr(%s, %s) == NULL (%s)\n", i, buf, data[i].expected, cmd);
    }

    testFinish();
}

static void test_other_opts(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        char *data;
        int input_mode;
        char *opt;
        char *opt_data;

        char *expected;
        int strstr_cmp;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "1", -1, " --bg=", "EF9900", "", 0 },
        /*  1*/ { BARCODE_CODE128, "1", -1, " -bg=", "EF9900", "", 0 },
        /*  2*/ { BARCODE_CODE128, "1", -1, " --bg=", "EF9900AA", "", 0 },
        /*  3*/ { BARCODE_CODE128, "1", -1, " --bg=", "GF9900", "Error 881: Malformed background RGB colour 'GF9900' (hexadecimal only)", 0 },
        /*  4*/ { BARCODE_CODE128, "1", -1, " --bgcolor=", "EF9900", "", 0 },
        /*  5*/ { BARCODE_CODE128, "1", -1, " --bgcolour=", "EF9900", "", 0 },
        /*  6*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000", "", 0 },
        /*  7*/ { BARCODE_CODE128, "1", -1, " --fg=", "00000000", "", 0 },
        /*  8*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000F", "Error 880: Malformed foreground RGB colour (6 or 8 characters only)", 0 },
        /*  9*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000FG", "Error 881: Malformed foreground RGB colour '000000FG' (hexadecimal only)", 0 },
        /* 10*/ { BARCODE_CODE128, "1", -1, " --fg=", "0,0,0,100", "", 0 },
        /* 11*/ { BARCODE_CODE128, "1", -1, " --fgcolor=", "111111", "", 0 },
        /* 12*/ { BARCODE_CODE128, "1", -1, " --fgcolour=", "111111", "", 0 },
        /* 13*/ { BARCODE_CODE128, "1", -1, " --compliantheight", "", "", 0 },
        /* 14*/ { BARCODE_DATAMATRIX, "1", -1, " --dmiso144", "", "", 0 },
        /* 15*/ { BARCODE_EANX, "123456", -1, " --guardwhitespace", "", "", 0 },
        /* 16*/ { BARCODE_EANX, "123456", -1, " --embedfont", "", "", 0 },
        /* 17*/ { BARCODE_CODE128, "1", -1, " --nobackground", "", "", 0 },
        /* 18*/ { BARCODE_CODE128, "1", -1, " --noquietzones", "", "", 0 },
        /* 19*/ { BARCODE_CODE128, "1", -1, " --notext", "", "", 0 },
        /* 20*/ { BARCODE_CODE128, "1", -1, " --quietzones", "", "", 0 },
        /* 21*/ { BARCODE_CODE128, "1", -1, " --reverse", "", "", 0 },
        /* 22*/ { BARCODE_CODE128, "1", -1, " --werror", NULL, "", 0 },
        /* 23*/ { 19, "1", -1, " --werror", NULL, "Error 207: Codabar 18 not supported", 0 },
        /* 24*/ { BARCODE_GS1_128, "[01]12345678901231", -1, "", NULL, "", 0 },
        /* 25*/ { BARCODE_GS1_128, "0112345678901231", -1, "", NULL, "Error 252: Data does not start with an AI", 0 },
        /* 26*/ { BARCODE_GS1_128, "0112345678901231", -1, " --gs1nocheck", NULL, "Error 252: Data does not start with an AI", 0 },
        /* 27*/ { BARCODE_GS1_128, "[00]376104250021234569", -1, "", NULL, "", 0 },
        /* 28*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, "", NULL, "Warning 261: AI (00) position 18: Bad checksum '8', expected '9'", 0 },
        /* 29*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, " --gs1nocheck", NULL, "", 0 },
        /* 30*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, " --werror", NULL, "Error 261: AI (00) position 18: Bad checksum '8', expected '9'", 0 },
        /* 31*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "1", "Error 155: Invalid Structured Append argument, expect \"index,count[,ID]\"", 0 },
        /* 32*/ { BARCODE_AZTEC, "1", -1, " --structapp=", ",", "Error 156: Structured Append index too short", 0 },
        /* 33*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "1234567890,", "Error 156: Structured Append index too long", 0 },
        /* 34*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,", "Error 159: Structured Append count too short", 0 },
        /* 35*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,1234567890", "Error 159: Structured Append count too long", 0 },
        /* 36*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,123456789,", "Error 158: Structured Append ID too short", 0 },
        /* 37*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,1234567890,", "Error 157: Structured Append count too long", 0 },
        /* 38*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,123456789,123456789012345678901234567890123", "Error 158: Structured Append ID too long", 0 },
        /* 39*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,123456789,12345678901234567890123456789012", "Error 701: Structured Append count '123456789' out of range (2 to 26)", 0 },
        /* 40*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "26,26,12345678901234567890123456789012", "", 0 },
        /* 41*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "A,26,12345678901234567890123456789012", "Error 160: Invalid Structured Append index (digits only)", 0 },
        /* 42*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "26,A,12345678901234567890123456789012", "Error 161: Invalid Structured Append count (digits only)", 0 },
        /* 43*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "26,1,12345678901234567890123456789012", "Error 162: Invalid Structured Append count '1', must be greater than or equal to 2", 0 },
        /* 44*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "0,2,12345678901234567890123456789012", "Error 163: Structured Append index '0' out of range (1 to count '2')", 0 },
        /* 45*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "3,2,12345678901234567890123456789012", "Error 163: Structured Append index '3' out of range (1 to count '2')", 0 },
        /* 46*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "2,3,12345678901234567890123456789012", "", 0 },
        /* 47*/ { BARCODE_PDF417, "1", -1, " --heightperrow", "", "", 0 },
        /* 48*/ { -1, NULL, -1, " -v", NULL, "Zint version ", 1 },
        /* 49*/ { -1, NULL, -1, " --version", NULL, "Zint version ", 1 },
        /* 50*/ { -1, NULL, -1, " -h", NULL, "Encode input data in a barcode ", 1 },
        /* 51*/ { -1, NULL, -1, " -e", NULL, "3: ISO/IEC 8859-1 ", 1 },
        /* 52*/ { -1, NULL, -1, " -t", NULL, "1 CODE11 ", 1 },
        /* 53*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "12345678", "Error 178: scalexdimdp X-dim invalid floating point (integer part must be 7 digits maximum)", 0 },
        /* 54*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "1234567890123", "Error 176: scalexdimdp X-dim too long", 0 },
        /* 55*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "123456.12", "Error 178: scalexdimdp X-dim invalid floating point (7 significant digits maximum)", 0 },
        /* 56*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", ",12.34", "Error 174: scalexdimdp X-dim too short", 0 },
        /* 57*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "12.34,", "Error 175: scalexdimdp resolution too short", 0 },
        /* 58*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "12mm1", "Error 177: scalexdimdp X-dim units must occur at end", 0 },
        /* 59*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "1inc", "Error 177: scalexdimdp X-dim units must occur at end", 0 },
        /* 60*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "1234x", "Error 178: scalexdimdp X-dim invalid floating point (integer part must be digits only)", 0 },
        /* 61*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "12.34in,123x", "Error 180: scalexdimdp resolution invalid floating point (integer part must be digits only)", 0 },
        /* 62*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "12,123.45678", "Error 180: scalexdimdp resolution invalid floating point (7 significant digits maximum)", 0 },
        /* 63*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "10.1,1000", "Warning 185: scalexdimdp X-dim '10.1' out of range (greater than 10), ignoring", 0 },
        /* 64*/ { BARCODE_EANX, "501234567890", -1, " --scalexdimdp=", "10,1000.1", "Warning 186: scalexdimdp resolution '1000.1' out of range (greater than 1000), ignoring", 0 },
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
            assert_nonnull(strstr(buf, data[i].expected), "i:%d strstr buf (%s) != expected (%s) (%s)\n", i, buf, data[i].expected, cmd);
        } else {
            assert_zero(strcmp(buf, data[i].expected), "i:%d strcmp buf (%s) != expected (%s) (%s)\n", i, buf, data[i].expected, cmd);
            if (strstr(data[i].expected, "Error") == NULL) {
                assert_zero(testUtilRemove(outfilename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, outfilename, errno, strerror(errno));
            }
        }
    }

    testFinish();
}

static void test_combos(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        char *data;
        char *opts;

        char *expected;
        char *outfilename;
        int strstr_cmp;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { -1, "1", " --direct -o direct.gif", "Warning 193: Output file given, ignoring '--direct' option", "direct.gif", 0 },
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
            assert_nonnull(strstr(buf, data[i].expected), "i:%d strstr buf (%s) != expected (%s) (%s)\n", i, buf, data[i].expected, cmd);
        } else {
            assert_zero(strcmp(buf, data[i].expected), "i:%d strcmp buf (%s) != expected (%s) (%s)\n", i, buf, data[i].expected, cmd);
        }
        if (data[i].outfilename != NULL) {
            assert_zero(remove(data[i].outfilename), "i:%d remove(%s) != 0 (%d: %s)\n", i, data[i].outfilename, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_exit_status(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int b;
        char *data;
        int input_mode;
        char *opt;
        char *opt_data;

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
        assert_equal(exit_status, data[i].expected, "i:%d exit_status %d != expected (%d) (%s), (cmd: %s)\n", i, exit_status, data[i].expected, buf, cmd);
        if (data[i].expected < ZINT_ERROR) {
            assert_zero(testUtilRemove(outfilename), "i:%d testUtilRemove(%s) != 0 (%d: %s)\n", i, outfilename, errno, strerror(errno));
        }
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
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
