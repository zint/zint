/*
    libzint - the open source barcode library
    Copyright (C) 2020-2022 Robin Stuart <rstuart114@gmail.com>

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

#include "testcommon.h"
#ifndef _WIN32
#include <sys/wait.h>
#endif

#ifdef _WIN32
/* Hacks to stop popen() mangling input on Windows */
static int utf8_to_wchar(const char *str, wchar_t *out) {
    unsigned int codepoint, state = 0;

    while (*str) {
        do {
            decode_utf8(&state, &codepoint, *str++);
        } while (*str && state != 0 && state != 12);
        if (state != 0) {
            fprintf(stderr, "utf8_to_wchar: warning: invalid UTF-8\n");
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
        sprintf(cmd + (int) strlen(cmd), "%s%s%d", strlen(cmd) ? " " : "", opt, val);
    }
}

static void arg_bool(char *cmd, const char *opt, int val) {
    if (val == 1) {
        sprintf(cmd + (int) strlen(cmd), "%s%s", strlen(cmd) ? " " : "", opt);
    }
}

static void arg_double(char *cmd, const char *opt, double val) {
    if (val != -1) {
        sprintf(cmd + (int) strlen(cmd), "%s%s%g", strlen(cmd) ? " " : "", opt, val);
    }
}

static void arg_data(char *cmd, const char *opt, const char *data) {
    if (data != NULL) {
        sprintf(cmd + (int) strlen(cmd), "%s%s\"%s\"", strlen(cmd) ? " " : "", opt, data);
    }
}

static void arg_seg(char *cmd, const char *opt, const char *data, const int eci) {
    if (data != NULL) {
        sprintf(cmd + (int) strlen(cmd), "%s%s%d,\"%s\"", strlen(cmd) ? " " : "", opt, eci, data);
    } else {
        sprintf(cmd + (int) strlen(cmd), "%s%s%d", strlen(cmd) ? " " : "", opt, eci);
    }
}

static int arg_input(char *cmd, const char *filename, const char *input) {
    FILE *fp;
    int cnt;
    if (input != NULL) {
        if (strcmp(input, "-") != 0) {
            fp = fopen(filename, "wb");
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
        sprintf(cmd + (int) strlen(cmd), "%s-i \"%s\"", strlen(cmd) ? " " : "", filename);
        return 1;
    }
    return 0;
}

static void arg_input_mode(char *cmd, int input_mode) {
    if (input_mode != -1) {
        if ((input_mode & 0x07) == DATA_MODE) {
            sprintf(cmd + (int) strlen(cmd), "%s--binary", strlen(cmd) ? " " : "");
        } else if ((input_mode & 0x07) == GS1_MODE) {
            sprintf(cmd + (int) strlen(cmd), "%s--gs1", strlen(cmd) ? " " : "");
        }
        if (input_mode & ESCAPE_MODE) {
            sprintf(cmd + (int) strlen(cmd), "%s--esc", strlen(cmd) ? " " : "");
        }
        if (input_mode & FAST_MODE) {
            sprintf(cmd + (int) strlen(cmd), "%s--fast", strlen(cmd) ? " " : "");
        }
        if (input_mode & GS1PARENS_MODE) {
            sprintf(cmd + (int) strlen(cmd), "%s--gs1parens", strlen(cmd) ? " " : "");
        }
        if (input_mode & GS1NOCHECK_MODE) {
            sprintf(cmd + (int) strlen(cmd), "%s--gs1nocheck", strlen(cmd) ? " " : "");
        }
        if (input_mode & HEIGHTPERROW_MODE) {
            sprintf(cmd + (int) strlen(cmd), "%s--heightperrow", strlen(cmd) ? " " : "");
        }
    }
}

static void arg_output_options(char *cmd, int output_options) {
    if (output_options != -1) {
        if (output_options & BARCODE_BIND) {
            sprintf(cmd + (int) strlen(cmd), "%s--bind", strlen(cmd) ? " " : "");
        }
        if (output_options & BARCODE_BOX) {
            sprintf(cmd + (int) strlen(cmd), "%s--box", strlen(cmd) ? " " : "");
        }
        if (output_options & BARCODE_STDOUT) {
            sprintf(cmd + (int) strlen(cmd), "%s--direct", strlen(cmd) ? " " : "");
        }
        if (output_options & READER_INIT) {
            sprintf(cmd + (int) strlen(cmd), "%s--init", strlen(cmd) ? " " : "");
        }
        if (output_options & SMALL_TEXT) {
            sprintf(cmd + (int) strlen(cmd), "%s--small", strlen(cmd) ? " " : "");
        }
        if (output_options & BOLD_TEXT) {
            sprintf(cmd + (int) strlen(cmd), "%s--bold", strlen(cmd) ? " " : "");
        }
        if (output_options & CMYK_COLOUR) {
            sprintf(cmd + (int) strlen(cmd), "%s--cmyk", strlen(cmd) ? " " : "");
        }
        if (output_options & BARCODE_DOTTY_MODE) {
            sprintf(cmd + (int) strlen(cmd), "%s--dotty", strlen(cmd) ? " " : "");
        }
        if (output_options & GS1_GS_SEPARATOR) {
            sprintf(cmd + (int) strlen(cmd), "%s--gssep", strlen(cmd) ? " " : "");
        }
        if (output_options & BARCODE_QUIET_ZONES) {
            sprintf(cmd + (int) strlen(cmd), "%s--quietzones", strlen(cmd) ? " " : "");
        }
        if (output_options & BARCODE_NO_QUIET_ZONES) {
            sprintf(cmd + (int) strlen(cmd), "%s--noquietzones", strlen(cmd) ? " " : "");
        }
        if (output_options & COMPLIANT_HEIGHT) {
            sprintf(cmd + (int) strlen(cmd), "%s--compliantheight", strlen(cmd) ? " " : "");
        }
    }
}

// Tests args that can be detected with `--dump`
static void test_dump_args(int index, int debug) {

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
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ {              -1, "123", NULL, NULL, NULL,       -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D2 13 9B 39 65 C8 C9 8E B" },
        /*  1*/ { BARCODE_CODE128, "123", NULL, NULL, NULL,       -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D2 13 9B 39 65 C8 C9 8E B" },
        /*  2*/ { BARCODE_CODE128, "123", "456", NULL, NULL,      -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D2 13 9B 39 65 C8 C9 8E B\nD2 19 3B 72 67 4E 4D 8E B" },
        /*  3*/ { BARCODE_CODE128, "123", NULL, NULL, NULL,       -1, -1, 1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "Warning 141: Can't use batch mode if data given, ignoring\nD2 13 9B 39 65 C8 C9 8E B" },
        /*  4*/ { BARCODE_CODE128, NULL, NULL, "123\n45\n", NULL, -1, -1, 1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D2 13 9B 39 65 C8 C9 8E B\nD3 97 62 3B 63 AC" },
        /*  5*/ { BARCODE_CODE128, NULL, NULL, "123\n45\n", "7\n",-1, -1, 1, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "Warning 144: Processing first input file 'test_dump_args1.txt' only\nD2 13 9B 39 65 C8 C9 8E B\nD3 97 62 3B 63 AC" },
        /*  6*/ { BARCODE_CODE128, "\t", NULL, NULL, NULL,        -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 90 D2 1A 63 AC" },
        /*  7*/ { BARCODE_CODE128, "\\t", NULL, NULL, NULL, ESCAPE_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 90 D2 1A 63 AC" },
        /*  8*/ { BARCODE_CODE128, "123", NULL, NULL, NULL, -1, BARCODE_BIND | BARCODE_BOX | SMALL_TEXT | BOLD_TEXT | CMYK_COLOUR, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D2 13 9B 39 65 C8 C9 8E B" },
        /*  9*/ { BARCODE_CODE128, "123", NULL, NULL, NULL, -1, BARCODE_DOTTY_MODE, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "Error 224: Selected symbology cannot be rendered as dots" },
        /* 10*/ { BARCODE_CODABLOCKF, "ABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 97 BA 86 51 88 B1 11 AC 46 D8 C7 58\nD0 97 BB 12 46 88 C5 1A 3C 55 CC C7 58" },
        /* 11*/ { BARCODE_CODABLOCKF, "ABCDEF", NULL, NULL, NULL, -1, -1, 0, 10, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "D0 97 BA 86 51 88 B1 11 AC 44 68 BC 98 EB\nD0 97 BB 12 46 2B BD 7B A3 47 8A 8D 18 EB" },
        /* 12*/ { BARCODE_CODABLOCKF, "ABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL,  3, -1, 0, -1, "D0 97 BA 58 51 88 B1 11 AC 46 36 C7 58\nD0 97 BB 12 46 88 C5 77 AF 74 62 C7 58\nD0 97 BA CE 5D EB DD 1A 3C 56 88 C7 58" },
        /* 13*/ { BARCODE_CODE11, NULL, NULL, "123", NULL,        -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "B2 D6 96 CA B5 6D 64" },
        /* 14*/ { BARCODE_CODE11, NULL, NULL, "123", NULL,        -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0,  1, "B2 D6 96 CA B5 64" },
        /* 15*/ { BARCODE_CODE11, "123", NULL, "456", NULL,       -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0,  2, "B2 D6 96 CA B2\nB2 B6 DA 9A B2" },
        /* 16*/ { BARCODE_CODE11, "123", "456", "789", "012",     -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0,  2, "B2 D6 96 CA B2\nB2 B6 DA 9A B2\nB2 A6 D2 D5 64\nB2 AD AD 2D 64" },
        /* 17*/ { BARCODE_PDF417, "123", NULL, NULL, NULL,        -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1,  0, 0, -1, "FF 54 7A BC 3A 9C 1D 5C 0F E8 A4\nFF 54 7E AE 3C 11 5F AB 8F E8 A4\nFF 54 6A F8 29 9F 1D 5F 8F E8 A4\nFF 54 57 9E 37 BA 1A F7 CF E8 A4\nFF 54 75 CC 36 F0 5D 73 0F E8 A4" },
        /* 18*/ { BARCODE_DATAMATRIX, "ABC", NULL, NULL, NULL, -1,          -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA 8\nB3 4\n8F 0\nB2 C\nA6 0\nBA C\nD6 0\nEB 4\nE2 8\nFF C" },
        /* 19*/ { BARCODE_DATAMATRIX, "ABC", NULL, NULL, NULL, -1, READER_INIT, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A\nAC 7\n8A 4\nA0 3\nC2 2\nB5 1\n82 2\nBA 7\n8C C\nA0 5\n86 A\nFF F" },
        /* 20*/ { BARCODE_DATAMATRIX, "ABCDEFGH", NULL, NULL, NULL, FAST_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nA6 8C\nB2 F0\n98 B4\nB9 A8\nB8 CC\nF0 78\nA0 3C\n99 70\n85 1C\nDA B0\nE5 94\nA7 50\nFF FC" },
        /* 21*/ { BARCODE_DATAMATRIX, "ABCDEFGH", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\n80 04\n82 60\nC5 24\n98 A8\nA3 9C\nCB B8\nAF DC\n86 58\nF6 44\nAC 18\n90 54\nCF 30\nFF FC" },
        /* 22*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJK", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA AA AA\n80 09 F9 BD\n82 4A E2 58\nC5 CD C9 A5\nD8 5C A5 FC\nE0 35 88 69\nCC FC B3 E6\nFF FF FF FF" },
        /* 23*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJK", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 1, -1, "AA AA\n80 25\n82 24\nC5 5D\n98 90\nA4 C7\nC8 A6\nB9 E9\n8E 02\nDE 91\nCD 6C\nA0 BB\n85 80\n98 2D\nE4 CA\nFF FF" },
        /* 24*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA A8\nBA 5A 44\n8B 4D 28\nBF 77 64\n97 85 50\nBA D8 AC\nCD ED B8\nD4 B5 2C\nD1 A8 00\n81 FB 2C\nE4 75 78\n96 E8 2C\nF3 75 78\nEE 1D 04\nCA BA 98\nB1 8F B4\nA0 4F 00\nE4 A7 74\nF1 D3 90\nEF E1 BC\n91 10 38\nFF FF FC" },
        /* 25*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 1, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA AA AA AA AA AA AA\nBA 03 BA 7D E5 31 B0 0D\n8B 6A 93 B6 E0 0A B8 3C\nBF 1D EA A7 EB ED A1 FB\n96 66 86 B6 C9 AE 92 40\nBF 65 E7 95 BC B7 FA E3\nCC 7C 90 CC D1 24 AB 5A\nFF FF FF FF FF FF FF FF" },
        /* 26*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 1, -1, 0, -1, -1, NULL, -1, -1, 1, -1, "AA AA A8\nBA 5A 44\n8B 4D 28\nBF 77 64\n97 85 50\nBA D8 AC\nCD ED B8\nD4 B5 2C\nD1 A8 00\n81 FB 2C\nE4 75 78\n96 E8 2C\nF3 75 78\nEE 1D 04\nCA BA 98\nB1 8F B4\nA0 4F 00\nE4 A7 74\nF1 D3 90\nEF E1 BC\n91 10 38\nFF FF FC" },
        /* 27*/ { BARCODE_DATAMATRIX, "[91]12[92]34", NULL, NULL, NULL, GS1_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nFA 9C\nBC 00\nD7 84\nED E0\nA4 E4\nA7 40\n9D 3C\nBF 50\nFA 24\nB1 68\nE5 04\n92 70\nFF FC" },
        /* 28*/ { BARCODE_DATAMATRIX, "[91]12[92]34", NULL, NULL, NULL, GS1_MODE, GS1_GS_SEPARATOR, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 29*/ { BARCODE_DATAMATRIX, "[9\\x31]12[92]34", NULL, NULL, NULL, GS1_MODE | ESCAPE_MODE, GS1_GS_SEPARATOR, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 30*/ { BARCODE_DATAMATRIX, "(9\\x31)12(92)34", NULL, NULL, NULL, GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, GS1_GS_SEPARATOR, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 31*/ { BARCODE_EANX_CC, "[91]12", NULL, NULL, NULL,    -1, -1, 0, -1, 0, -1, 0, -1, -1, "12345678+12", -1, -1, 0, -1, "DB BC D3 9C 44 E9 D2 2C 19 E7 A2 D8 A0 00 00 00\nDB 31 1C 9C C7 29 92 47 D9 E9 40 C8 A0 00 00 00\nDA 3B EB 10 AF 09 9A 18 9D 7D 82 E8 A0 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n20 00 00 00 00 00 00 00 00 00 00 00 20 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n14 68 D1 A6 49 BD 55 C9 D4 22 48 B9 40 59 94 98" },
        /* 32*/ { BARCODE_EANX_CC, "[91]12", NULL, NULL, NULL,    -1, -1, 0, -1, 0, -1, 0, -1,  2, "12345678+12", -1, -1, 0, -1, "D3 A3 E9 DB F5 C9 DB 43 D9 CB 98 D2 20 00 00 00\nD3 25 0F 11 E4 49 D3 51 F1 AC FC D6 20 00 00 00\nD1 33 48 19 39 E9 93 18 49 D8 98 D7 20 00 00 00\nD1 A6 FC DA 1C 49 9B C5 05 E2 84 D7 A0 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n20 00 00 00 00 00 00 00 00 00 00 00 20 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n14 68 D1 A6 49 BD 55 C9 D4 22 48 B9 40 59 94 98" },
        /* 33*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1,  1, 0, -1, "Warning 543: Converted to Shift JIS but no ECI specified\nFE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 34*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, 0, 26, 0, -1, -1, NULL, -1,  1, 0, -1, "FE 5B F8\n82 72 08\nBA DA E8\nBA 52 E8\nBA 2A E8\n82 0A 08\nFE AB F8\n00 D8 00\nEF F6 20\nB5 C2 28\n36 28 88\nFD 42 10\n62 2A C8\n00 95 70\nFE B7 38\n82 FD D8\nBA 97 00\nBA 43 60\nBA C8 C8\n82 C3 68\nFE EA F8" },
        /* 35*/ { BARCODE_QRCODE, "\223\137", NULL, NULL, NULL, DATA_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 0A 08\nBA A2 E8\nBA 0A E8\nBA 5A E8\n82 72 08\nFE AB F8\n00 A0 00\nEF AE 20\n75 B5 20\n82 F7 58\nF4 9D C8\n5E 17 28\n00 C2 20\nFE 88 80\n82 82 38\nBA EA A8\nBA 55 50\nBA D7 68\n82 BD D0\nFE B7 78" },
        /* 36*/ { BARCODE_QRCODE, "\223\137", NULL, NULL, NULL, DATA_MODE, -1, 0, -1, 0, -1, 1, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 37*/ { BARCODE_QRCODE, "\\x93\\x5F", NULL, NULL, NULL, DATA_MODE | ESCAPE_MODE, -1, 0, -1, 0, -1, 1, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 38*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, 0, -1, 0, 2, -1, NULL, -1,  1, 0, -1, "Warning 543: Converted to Shift JIS but no ECI specified\nFE 4B F8\n82 92 08\nBA 42 E8\nBA 92 E8\nBA 3A E8\n82 EA 08\nFE AB F8\n00 38 00\nFB CD 50\nA5 89 18\n0B 74 B8\nFC 81 A0\n92 34 B8\n00 DE 48\nFE AB 10\n82 5E 50\nBA C9 20\nBA C9 20\nBA F4 E0\n82 81 A0\nFE B4 E8" },
        /* 39*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, 0, -1, 1, -1, -1, NULL, -1, -1, 0, -1, "FE 8A FE\n80 28 02\nBE E8 FA\nA0 94 0A\nAE 3E EA\nAE D2 EA\nAE 74 EA\n00 AA 00\n15 B4 80\n0B 48 74\nA2 4A A4\nB5 56 2C\nA8 5A A8\n9F 18 50\n02 07 50\n00 A6 00\nFE 20 EA\n02 C2 EA\nFA C4 EA\n0A 42 0A\nEA 52 FA\nEA 24 02\nEA AA FE" },
        /* 40*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, 0, -1, 1, 3, -1, NULL, -1, -1, 0, -1, "FE 16 FE\n80 E2 02\nBE C2 FA\nA0 A0 0A\nAE F6 EA\nAE 98 EA\nAE BA EA\n00 E0 00\n15 83 80\n44 7E AE\n92 9C 78\n25 BF 08\n47 4B 8C\n0D F9 74\n03 E7 50\n00 3A 00\nFE C2 EA\n02 22 EA\nFA DA EA\n0A 22 0A\nEA B2 FA\nEA 9A 02\nEA E8 FE" },
        /* 41*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, 0, -1, 1, 4, -1, NULL, -1, -1, 0, -1, "FE 8A FE\n80 28 02\nBE E8 FA\nA0 94 0A\nAE 3E EA\nAE D2 EA\nAE 74 EA\n00 AA 00\n15 B4 80\n0B 48 74\nA2 4A A4\nB5 56 2C\nA8 5A A8\n9F 18 50\n02 07 50\n00 A6 00\nFE 20 EA\n02 C2 EA\nFA C4 EA\n0A 42 0A\nEA 52 FA\nEA 24 02\nEA AA FE" },
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

        if (index != -1 && i != index) continue;

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
            assert_zero(remove(input1_filename), "i:%d remove(%s) != 0 (%d: %s)\n", i, input1_filename, errno, strerror(errno));
        }
        if (have_input2) {
            assert_zero(remove(input2_filename), "i:%d remove(%s) != 0 (%d: %s)\n", i, input2_filename, errno, strerror(errno));
        }
    }

    testFinish();
}

// Tests segs
static void test_dump_segs(int index, int debug) {

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
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ {              -1, "123", NULL, NULL, -1, -1, -1, "D2 13 9B 39 65 C8 C9 8E B" },
        /*  1*/ {              -1, "123", NULL, NULL, -1, 3, -1, "Error 166: Invalid segment argument, expect \"ECI,DATA\"" },
        /*  2*/ {              -1, "123", "456", NULL, -1, -1, -1, "Error 167: Invalid segment ECI (digits only)" },
        /*  3*/ {              -1, "123", "456", NULL, -1, 1000000, -1, "Error 168: Segment ECI code out of range (0 to 999999)" },
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

        if (index != -1 && i != index) continue;

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

static void test_input(int index, int debug) {

#define TEST_INPUT_LONG "test_67890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"

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
        char *input;
        char *outfile;

        int num_expected;
        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, 1, -1, 0, "gif", "123\n456\n", "", 2, "00001.gif\00000002.gif" },
        /*  1*/ { BARCODE_CODE128, 1, -1, 0, "gif", "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n", "~~~.gif", 10, "001.gif\000002.gif\000003.gif\000004.gif\000005.gif\000006.gif\000007.gif\000008.gif\000009.gif\000010.gif" },
        /*  2*/ { BARCODE_CODE128, 1, -1, 0, "gif", "123\n456\n", "@@@@.gif", 2, TEST_INPUT_AMPERSAND_EXPECTED },
        /*  3*/ { BARCODE_CODE128, 1, -1, 0, "gif", "123\n456\n789\n", "#####.gif", 3, "    1.gif\000    2.gif\000    3.gif" },
        /*  4*/ { BARCODE_CODE128, 1, -1, 0, "gif", "123\n456\n", "test_batch~.gif", 2, "test_batch1.gif\000test_batch2.gif" },
        /*  5*/ { BARCODE_CODE128, 1, -1, 1, "gif", "123\n456\n7890123456789\n", NULL, 3, "123.gif\000456.gif\0007890123456789.gif" },
        /*  6*/ { BARCODE_CODE128, 1, -1, 1, "svg", "123\n456\n7890123456789\n", NULL, 3, "123.svg\000456.svg\0007890123456789.svg" },
        /*  7*/ { BARCODE_CODE128, 1, -1, 1, "gif", "123\n456\n7890123456789\nA\\xA0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_xA0B.gif" },
        /*  8*/ { BARCODE_CODE128, 1, ESCAPE_MODE, 1, "gif", "123\n456\n7890123456789\nA\\xA0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_B.gif" },
        /*  9*/ { BARCODE_CODE128, 1, -1, 1, "gif", "123\n456\n7890123456789\nA\\u00A0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_u00A0B.gif" },
        /* 10*/ { BARCODE_CODE128, 1, ESCAPE_MODE, 1, "gif", "123\n456\n7890123456789\nA\\u00A0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_B.gif" },
        /* 11*/ { BARCODE_CODE128, 1, -1, 0, "gif", "\n", "test_batch.gif", 0, NULL },
        /* 12*/ { BARCODE_CODE128, 1, -1, 0, "gif", "123\n456\n", TEST_INPUT_LONG "~.gif", 2, TEST_INPUT_LONG "1.gif\000" TEST_INPUT_LONG "2.gif" },
        /* 13*/ { BARCODE_CODE128, 0, -1, 0, "svg", "123", TEST_INPUT_LONG "1.gif", 1, TEST_INPUT_LONG "1.svg" },
        /* 14*/ { BARCODE_CODE128, 1, -1, 0, "svg", "123\n", TEST_INPUT_LONG "1.gif", 1, TEST_INPUT_LONG "1.svg" },
        /* 15*/ { BARCODE_CODE128, 1, -1, 0, "gif", "123\n", "test_batch.jpeg", 1, "test_batch.jpeg.gif" },
        /* 16*/ { BARCODE_CODE128, 1, -1, 0, "gif", "123\n", "test_batch.jpg", 1, "test_batch.gif" },
        /* 17*/ { BARCODE_CODE128, 1, -1, 0, "emf", "123\n", "test_batch.jpeg", 1, "test_batch.jpeg.emf" },
        /* 18*/ { BARCODE_CODE128, 1, -1, 0, "emf", "123\n", "test_batch.jpg", 1, "test_batch.emf" },
        /* 19*/ { BARCODE_CODE128, 1, -1, 0, "eps", "123\n", "test_batch.ps", 1, "test_batch.eps" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];

    char *input_filename = "test_input.txt";
    char *outfile;

    testStart("test_input");

    for (i = 0; i < data_size; i++) {
        int j;

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);
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
        arg_input(cmd, input_filename, data[i].input);
        arg_data(cmd, "-o ", data[i].outfile);

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);

        outfile = data[i].expected;
        for (j = 0; j < data[i].num_expected; j++) {
            assert_nonzero(testUtilExists(outfile), "i:%d j:%d testUtilExists(%s) != 1\n", i, j, outfile);
            assert_zero(remove(outfile), "i:%d j:%d remove(%s) != 0 (%d: %s)\n", i, j, outfile, errno, strerror(errno));
            outfile += strlen(outfile) + 1;
        }

        assert_zero(remove(input_filename), "i:%d remove(%s) != 0 (%d: %s)\n", i, input_filename, errno, strerror(errno));
    }

    testFinish();
}

static void test_stdin_input(int index, int debug) {

    struct item {
        int b;
        char *data;
        char *input;
        char *outfile;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
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

        if (index != -1 && i != index) continue;

        sprintf(cmd, "echo '%s' | zint", data[i].data);
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        arg_int(cmd, "-b ", data[i].b);
        arg_input(cmd, input_filename, data[i].input);
        arg_data(cmd, "-o ", data[i].outfile);

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);

        assert_nonzero(testUtilExists(data[i].outfile), "i:%d testUtilExists(%s) != 1\n", i, data[i].outfile);
        assert_zero(remove(data[i].outfile), "i:%d remove(%s) != 0 (%d: %s)\n", i, data[i].outfile, errno, strerror(errno));
    }

    testFinish();
}

// Note ordering of `--batch` before/after data/input args affects error messages
static void test_batch_input(int index, int debug) {

    struct item {
        int b;
        char *data;
        char *input;
        char *input2;

        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "123", NULL, NULL, "Warning 122: Can't define data in batch mode, ignoring '123'\nWarning 124: No data received, no symbol generated" },
        /*  1*/ { BARCODE_CODE128, "123", "123\n456\n", NULL, "Warning 122: Can't define data in batch mode, ignoring '123'\nD2 13 9B 39 65 C8 C9 8E B\nD2 19 3B 72 67 4E 4D 8E B" },
        /*  3*/ { BARCODE_CODE128, NULL, "123\n456\n", "789\n", "Warning 143: Can only define one input file in batch mode, ignoring 'test_batch_input2.txt'\nD2 13 9B 39 65 C8 C9 8E B\nD2 19 3B 72 67 4E 4D 8E B" },
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

        if (index != -1 && i != index) continue;

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
            assert_zero(remove(input1_filename), "i:%d remove(%s) != 0 (%d: %s)\n", i, input1_filename, errno, strerror(errno));
        }
        if (have_input2) {
            assert_zero(remove(input2_filename), "i:%d remove(%s) != 0 (%d: %s)\n", i, input2_filename, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_batch_large(int index, int debug) {

    struct item {
        int b;
        int mirror;
        char *pattern;
        int length;

        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
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

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);

        strcpy(cmd, "zint --batch --filetype=gif");
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        arg_int(cmd, "-b ", data[i].b);
        arg_bool(cmd, "--mirror", data[i].mirror);

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        strcat(data_buf, "\n");
        have_input = arg_input(cmd, input_filename, data_buf);

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        if (data[i].expected) {
            assert_zero(remove(data[i].expected), "i:%d remove(%s) != 0 (%d: %s)\n", i, data[i].expected, errno, strerror(errno));
        } else {
            assert_zero(testUtilExists("out.gif"), "i:%d testUtilExists(out.gif) != 0 (%d: %s) (%s)\n", i, errno, strerror(errno), cmd);
        }

        if (have_input) {
            assert_zero(remove(input_filename), "i:%d remove(%s) != 0 (%d: %s)\n", i, input_filename, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_checks(int index, int debug) {

    struct item {
        int addongap;
        int border;
        int cols;
        double dotsize;
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
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { -2, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 139: Invalid add-on gap value (digits only)" },
        /*  1*/ {  6, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 140: Add-on gap out of range (7 to 12), ignoring" },
        /*  2*/ { 13, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 140: Add-on gap out of range (7 to 12), ignoring" },
        /*  3*/ { -1, -2,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 107: Invalid border width value (digits only)" },
        /*  4*/ { -1, 1001, -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 108: Border width out of range (0 to 1000), ignoring" },
        /*  5*/ { -1, -1,   -1, 0.009, -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 106: Invalid dot radius value (less than 0.01), ignoring" },
        /*  6*/ { -1, -1,   -2, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 131: Invalid columns value (digits only)" },
        /*  7*/ { -1, -1,  201, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 111: Number of columns out of range (1 to 200), ignoring" },
        /*  8*/ { -1, -1,   -1, -1,    -2,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 138: Invalid ECI value (digits only)" },
        /*  9*/ { -1, -1,   -1, -1,    1000000, NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 118: ECI code out of range (0 to 999999), ignoring" },
        /* 10*/ { -1, -1,   -1, -1,    -1,      "jpg", -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 142: File type 'jpg' not supported, ignoring" },
        /* 11*/ { -1, -1,   -1, -1,    -1,      NULL,  -2,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 110: Symbol height '-2' out of range (0.5 to 2000), ignoring" },
        /* 12*/ { -1, -1,   -1, -1,    -1,      NULL,   0,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 110: Symbol height '0' out of range (0.5 to 2000), ignoring" },
        /* 13*/ { -1, -1,   -1, -1,    -1,      NULL, 2001,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 110: Symbol height '2001' out of range (0.5 to 2000), ignoring" },
        /* 14*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -2, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 155: Guard bar descent '-2' out of range (0 to 50), ignoring" },
        /* 15*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, 50.1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 155: Guard bar descent '50.1' out of range (0 to 50), ignoring" },
        /* 16*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -2, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 148: Invalid mask value (digits only)" },
        /* 17*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1,  8, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 147: Mask value out of range (0 to 7), ignoring" },
        /* 18*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1,  7, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 116: Mode value out of range (0 to 6), ignoring" },
        /* 19*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -2, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 117: Invalid rotation value (digits only)" },
        /* 20*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, 45, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 137: Invalid rotation parameter (0, 90, 180 or 270 only), ignoring" },
        /* 21*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -2, -1,   -1, -1, -1, -1,   -1,   -1, "Error 132: Invalid rows value (digits only)" },
        /* 22*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, 91, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 112: Number of rows out of range (1 to 90), ignoring" },
        /* 23*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -2,   -1, -1, -1, -1,   -1,   -1, "Warning 105: Invalid scale value (less than 0.01), ignoring" },
        /* 24*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, 0.49, -1, -1, -1, -1,   -1,   -1, "Warning 146: Scaling less than 0.5 will be set to 0.5 for 'gif' output" },
        /* 25*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -2, -1, -1, -1,   -1,   -1, "Error 149: Invalid Structured Carrier Message version value (digits only)" },
        /* 26*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,  100, -1, -1, -1,   -1,   -1, "Warning 150: Structured Carrier Message version out of range (0 to 99), ignoring" },
        /* 27*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -2, -1, -1,   -1,   -1, "Error 134: Invalid ECC value (digits only)" },
        /* 28*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1,  9, -1, -1,   -1,   -1, "Warning 114: ECC level out of range (0 to 8), ignoring" },
        /* 29*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -2, -1,   -1,   -1, "Error 128: Invalid separator value (digits only)" },
        /* 30*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1,  5, -1,   -1,   -1, "Warning 127: Separator value out of range (0 to 4), ignoring" },
        /* 31*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -2,   -1,   -1, "Error 133: Invalid version value (digits only)" },
        /* 32*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, 1000, -1,   -1, "Warning 113: Version value out of range (1 to 999), ignoring" },
        /* 33*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -2,   -1, "Error 153: Invalid vertical whitespace value '-2' (digits only)" },
        /* 34*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, 1001,   -1, "Warning 154: Vertical whitespace value out of range (0 to 1000), ignoring" },
        /* 35*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -2, "Error 120: Invalid horizontal whitespace value '-2' (digits only)" },
        /* 36*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,   -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1, 1001, "Warning 121: Horizontal whitespace value out of range (0 to 1000), ignoring" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[4096];
    char *outfilename = "out.gif";

    testStart("test_checks");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        strcpy(cmd, "zint -d 1 --filetype=gif");
        if (debug & ZINT_DEBUG_PRINT) {
            strcat(cmd, " --verbose");
        }

        arg_int(cmd, "--addongap=", data[i].addongap);
        arg_int(cmd, "--border=", data[i].border);
        arg_int(cmd, "--cols=", data[i].cols);
        arg_double(cmd, "--dotsize=", data[i].dotsize);
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
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s)\n", i, buf, data[i].expected);

        if (strncmp(data[i].expected, "Warning", 7) == 0) {
            assert_zero(remove(outfilename), "i:%d remove(%s) != 0 (%d: %s)\n", i, outfilename, errno, strerror(errno));
        }
    }

    testFinish();
}

static void test_barcode_symbology(int index, int debug) {

    struct item {
        const char *bname;
        const char *data;
        const char *primary;
        int fail;
        const char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    static const struct item data[] = {
        /*  0*/ { "_", "1", NULL, 1, "Error 119: Invalid barcode type '_'" },
        /*  1*/ { "a", "1", NULL, 1, "Error 119: Invalid barcode type 'a'" },
        /*  2*/ { "code128", "1", NULL, 0, "symbology: 20," },
        /*  3*/ { "code218", "1", NULL, 1, "Error 119: Invalid barcode type 'code218'" },
        /*  4*/ { "code12", "1", NULL, 1, "Error 119: Invalid barcode type 'code12'" },
        /*  5*/ { "BARCODE_CODE11", "1", NULL, 0, "symbology: 1," },
        /*  6*/ { "C25 Standard", "1", NULL, 0, "symbology: 2," },
        /*  7*/ { "c25matrix", "1", NULL, 0, "symbology: 2," }, // Legacy now supported
        /*  8*/ { "2 of 5 Standard", "1", NULL, 0, "symbology: 2," }, // Synonym
        /*  9*/ { "2 of 5 Matrix", "1", NULL, 0, "symbology: 2," }, // Synonym
        /* 10*/ { "Code 2 of 5 Standard", "1", NULL, 0, "symbology: 2," }, // Synonym
        /* 11*/ { "Code 2 of 5 Matrix", "1", NULL, 0, "symbology: 2," }, // Synonym
        /* 12*/ { "Standard Code 2 of 5", "1", NULL, 0, "symbology: 2," }, // Synonym
        /* 13*/ { "C25INTER", "1", NULL, 0, "symbology: 3," },
        /* 14*/ { "c25 interleaved", "1", NULL, 0, "symbology: 3," }, // Synonym
        /* 15*/ { "code 2 of 5 inter", "1", NULL, 0, "symbology: 3," }, // Synonym
        /* 16*/ { "code 2 of 5 interleaved", "1", NULL, 0, "symbology: 3," }, // Synonym
        /* 17*/ { "2 of 5 inter", "1", NULL, 0, "symbology: 3," }, // Synonym
        /* 18*/ { "2 of 5 interleaved", "1", NULL, 0, "symbology: 3," }, // Synonym
        /* 19*/ { "interleaved 2 of 5", "1", NULL, 0, "symbology: 3," }, // Synonym
        /* 20*/ { "interleaved code 2 of 5", "1", NULL, 0, "symbology: 3," }, // Synonym
        /* 21*/ { "c25IATA", "1", NULL, 0, "symbology: 4," },
        /* 22*/ { "2of5IATA", "1", NULL, 0, "symbology: 4," }, // Synonym
        /* 23*/ { "code2of5IATA", "1", NULL, 0, "symbology: 4," }, // Synonym
        /* 24*/ { "IATA2of5", "1", NULL, 0, "symbology: 4," }, // Synonym
        /* 25*/ { "IATAcode2of5", "1", NULL, 0, "symbology: 4," }, // Synonym
        /* 26*/ { "c25 Logic", "1", NULL, 0, "symbology: 6," },
        /* 27*/ { "c25 Data Logic", "1", NULL, 0, "symbology: 6," }, // Synonym
        /* 28*/ { "Code 2 of 5 Logic", "1", NULL, 0, "symbology: 6," }, // Synonym
        /* 29*/ { "Code 2 of 5 Data Logic", "1", NULL, 0, "symbology: 6," }, // Synonym
        /* 30*/ { "2 of 5 Logic", "1", NULL, 0, "symbology: 6," }, // Synonym
        /* 31*/ { "2 of 5 Data Logic", "1", NULL, 0, "symbology: 6," }, // Synonym
        /* 32*/ { "c25 Ind", "1", NULL, 0, "symbology: 7," },
        /* 33*/ { "c25 Industrial", "1", NULL, 0, "symbology: 7," }, // Synonym
        /* 34*/ { "code 2 of 5 Ind", "1", NULL, 0, "symbology: 7," }, // Synonym
        /* 35*/ { "code 2 of 5 Industrial", "1", NULL, 0, "symbology: 7," }, // Synonym
        /* 36*/ { "2 of 5 Ind", "1", NULL, 0, "symbology: 7," }, // Synonym
        /* 37*/ { "2 of 5 Industrial", "1", NULL, 0, "symbology: 7," }, // Synonym
        /* 38*/ { "Industrial 2 of 5", "1", NULL, 0, "symbology: 7," }, // Synonym
        /* 39*/ { "Industrial code 2 of 5", "1", NULL, 0, "symbology: 7," }, // Synonym
        /* 40*/ { "code39", "1", NULL, 0, "symbology: 8," },
        /* 41*/ { "excode 39", "1", NULL, 0, "symbology: 9," },
        /* 42*/ { "Extended Code 39", "1", NULL, 0, "symbology: 9," },
        /* 43*/ { "eanx", "1", NULL, 0, "symbology: 13," },
        /* 44*/ { "ean", "1", NULL, 0, "symbology: 13," },
        /* 45*/ { "eanx chk", "1", NULL, 0, "symbology: 14," },
        /* 46*/ { "eanxchk", "1", NULL, 0, "symbology: 14," },
        /* 47*/ { "eanchk", "1", NULL, 0, "symbology: 14," },
        /* 48*/ { "GS1128", "[01]12345678901231", NULL, 0, "symbology: 16," },
        /* 49*/ { "ean 128", "[01]12345678901231", NULL, 0, "symbology: 16," },
        /* 50*/ { "coda bar", "A1B", NULL, 0, "symbology: 18," },
        /* 51*/ { "DPLEIT", "1", NULL, 0, "symbology: 21," },
        /* 52*/ { "DPIDENT", "1", NULL, 0, "symbology: 22," },
        /* 53*/ { "code16k", "1", NULL, 0, "symbology: 23," },
        /* 54*/ { "CODE49", "1", NULL, 0, "symbology: 24," },
        /* 55*/ { "CODE93", "1", NULL, 0, "symbology: 25," },
        /* 56*/ { "flat", "1", NULL, 0, "symbology: 28," },
        /* 57*/ { "dbar omn", "1", NULL, 0, "symbology: 29," },
        /* 58*/ { "rss14", "1", NULL, 0, "symbology: 29," },
        /* 59*/ { "databar omn", "1", NULL, 0, "symbology: 29," },
        /* 60*/ { "databar omni", "1", NULL, 0, "symbology: 29," },
        /* 61*/ { "dbar ltd", "1", NULL, 0, "symbology: 30," },
        /* 62*/ { "rss ltd", "1", NULL, 0, "symbology: 30," },
        /* 63*/ { "databar ltd", "1", NULL, 0, "symbology: 30," },
        /* 64*/ { "databar limited", "1", NULL, 0, "symbology: 30," },
        /* 65*/ { "dbarexp", "[10]12", NULL, 0, "symbology: 31," },
        /* 66*/ { "rss exp", "[10]12", NULL, 0, "symbology: 31," },
        /* 67*/ { "databarexp", "[10]12", NULL, 0, "symbology: 31," },
        /* 68*/ { "databarexpanded", "[10]12", NULL, 0, "symbology: 31," },
        /* 69*/ { "telepen", "1", NULL, 0, "symbology: 32," },
        /* 70*/ { "upc", "1", NULL, 1, "Error 119: Invalid barcode type 'upc'" },
        /* 71*/ { "upca", "1", NULL, 0, "symbology: 34," },
        /* 72*/ { "upca_chk", "123456789012", NULL, 0, "symbology: 35," },
        /* 73*/ { "upce", "1", NULL, 0, "symbology: 37," },
        /* 74*/ { "upce chk", "12345670", NULL, 0, "symbology: 38," },
        /* 75*/ { "POSTNET ", "12345678901", NULL, 0, "symbology: 40," },
        /* 76*/ { "msi", "1", NULL, 0, "symbology: 47," },
        /* 77*/ { "MSI Plessey ", "1", NULL, 0, "symbology: 47," },
        /* 78*/ { "fim ", "A", NULL, 0, "symbology: 49," },
        /* 79*/ { "LOGMARS", "123456", NULL, 0, "symbology: 50," },
        /* 80*/ { " pharma", "123456", NULL, 0, "symbology: 51," },
        /* 81*/ { " pzn ", "1", NULL, 0, "symbology: 52," },
        /* 82*/ { "pharma two", "4", NULL, 0, "symbology: 53," },
        /* 83*/ { "BARCODE_PDF417", "1", NULL, 0, "symbology: 55," },
        /* 84*/ { "pdf", "1", NULL, 1, "Error 119: Invalid barcode type 'pdf'" },
        /* 85*/ { "barcodepdf417comp", "1", NULL, 0, "symbology: 56," },
        /* 86*/ { "pdf417trunc", "1", NULL, 0, "symbology: 56," },
        /* 87*/ { "MaxiCode", "1", NULL, 0, "symbology: 57," },
        /* 88*/ { "QR CODE", "1", NULL, 0, "symbology: 58," },
        /* 89*/ { "qr", "1", NULL, 0, "symbology: 58," }, // Synonym
        /* 90*/ { "Code 128 B", "1", NULL, 0, "symbology: 60," },
        /* 91*/ { "AUS POST", "12345678901234567890123", NULL, 0, "symbology: 63," },
        /* 92*/ { "AusReply", "12345678", NULL, 0, "symbology: 66," },
        /* 93*/ { "AUSROUTE", "12345678", NULL, 0, "symbology: 67," },
        /* 94*/ { "AUS REDIRECT", "12345678", NULL, 0, "symbology: 68," },
        /* 95*/ { "isbnx", "123456789", NULL, 0, "symbology: 69," },
        /* 96*/ { "rm4scc", "1", NULL, 0, "symbology: 70," },
        /* 97*/ { "DataMatrix", "1", NULL, 0, "symbology: 71," },
        /* 98*/ { "EAN14", "1", NULL, 0, "symbology: 72," },
        /* 99*/ { "vin", "12345678701234567", NULL, 0, "symbology: 73," },
        /*100*/ { "CodaBlock-F", "1", NULL, 0, "symbology: 74," },
        /*101*/ { "NVE18", "1", NULL, 0, "symbology: 75," },
        /*102*/ { "Japan Post", "1", NULL, 0, "symbology: 76," },
        /*103*/ { "Korea Post", "1", NULL, 0, "symbology: 77," },
        /*104*/ { "DBar Stk", "1", NULL, 0, "symbology: 79," },
        /*105*/ { "rss14stack", "1", NULL, 0, "symbology: 79," },
        /*106*/ { "DataBar Stk", "1", NULL, 0, "symbology: 79," },
        /*107*/ { "DataBar Stacked", "1", NULL, 0, "symbology: 79," },
        /*108*/ { "DBar Omn Stk", "1", NULL, 0, "symbology: 80," },
        /*109*/ { "RSS14STACK OMNI", "1", NULL, 0, "symbology: 80," },
        /*110*/ { "DataBar Omn Stk", "1", NULL, 0, "symbology: 80," },
        /*111*/ { "DataBar Stacked Omn", "1", NULL, 0, "symbology: 80," },
        /*112*/ { "DataBar Stacked Omni", "1", NULL, 0, "symbology: 80," },
        /*113*/ { "DBar Exp Stk", "[20]01", NULL, 0, "symbology: 81," },
        /*114*/ { "rss_expstack", "[20]01", NULL, 0, "symbology: 81," },
        /*115*/ { "DataBar Exp Stk", "[20]01", NULL, 0, "symbology: 81," },
        /*116*/ { "DataBar Expanded Stk", "[20]01", NULL, 0, "symbology: 81," },
        /*117*/ { "DataBar Expanded Stacked", "[20]01", NULL, 0, "symbology: 81," },
        /*118*/ { "planet", "12345678901", NULL, 0, "symbology: 82," },
        /*119*/ { "MicroPDF417", "1", NULL, 0, "symbology: 84," },
        /*120*/ { "USPS IMail", "12345678901234567890", NULL, 0, "symbology: 85," },
        /*121*/ { "OneCode", "12345678901234567890", NULL, 0, "symbology: 85," },
        /*122*/ { "plessey", "1", NULL, 0, "symbology: 86," },
        /*123*/ { "telepen num", "1", NULL, 0, "symbology: 87," },
        /*124*/ { "ITF14", "1", NULL, 0, "symbology: 89," },
        /*125*/ { "KIX", "1", NULL, 0, "symbology: 90," },
        /*126*/ { "Aztec", "1", NULL, 0, "symbology: 92," },
        /*127*/ { "Aztec Code", "1", NULL, 0, "symbology: 92," }, // Synonym
        /*128*/ { "daft", "D", NULL, 0, "symbology: 93," },
        /*129*/ { "DPD", "0123456789012345678901234567", NULL, 0, "symbology: 96," },
        /*130*/ { "Micro QR", "1", NULL, 0, "symbology: 97," },
        /*131*/ { "Micro QR Code", "1", NULL, 0, "symbology: 97," },
        /*132*/ { "hibc128", "1", NULL, 0, "symbology: 98," },
        /*133*/ { "hibccode128", "1", NULL, 0, "symbology: 98," }, // Synonym
        /*134*/ { "hibc39", "1", NULL, 0, "symbology: 99," },
        /*135*/ { "hibccode39", "1", NULL, 0, "symbology: 99," }, // Synonym
        /*136*/ { "hibcdatamatrix", "1", NULL, 0, "symbology: 102," }, // Synonym
        /*137*/ { "hibcdm", "1", NULL, 0, "symbology: 102," },
        /*138*/ { "HIBC qr", "1", NULL, 0, "symbology: 104," },
        /*139*/ { "HIBC QR Code", "1", NULL, 0, "symbology: 104," }, // Synonym
        /*140*/ { "HIBCPDF", "1", NULL, 0, "symbology: 106," },
        /*141*/ { "HIBCPDF417", "1", NULL, 0, "symbology: 106," }, // Synonym
        /*142*/ { "HIBCMICPDF", "1", NULL, 0, "symbology: 108," },
        /*143*/ { "HIBC Micro PDF", "1", NULL, 0, "symbology: 108," }, // Synonym
        /*144*/ { "HIBC Micro PDF417", "1", NULL, 0, "symbology: 108," }, // Synonym
        /*145*/ { "HIBC BlockF", "1", NULL, 0, "symbology: 110," },
        /*146*/ { "HIBC CodaBlock-F", "1", NULL, 0, "symbology: 110," }, // Synonym
        /*147*/ { "HIBC Aztec", "1", NULL, 0, "symbology: 112," },
        /*148*/ { "DotCode", "1", NULL, 0, "symbology: 115," },
        /*149*/ { "Han Xin", "1", NULL, 0, "symbology: 116," },
        /*150*/ { "Mailmark", "01000000000000000AA00AA0A", NULL, 0, "symbology: 121," },
        /*151*/ { "azrune", "1", NULL, 0, "symbology: 128," },
        /*152*/ { "aztecrune", "1", NULL, 0, "symbology: 128," }, // Synonym
        /*153*/ { "aztecrunes", "1", NULL, 0, "symbology: 128," }, // Synonym
        /*154*/ { "code32", "1", NULL, 0, "symbology: 129," },
        /*155*/ { "eanx cc", "[20]01", "1234567890128", 0, "symbology: 130," },
        /*156*/ { "eancc", "[20]01", "1234567890128", 0, "symbology: 130," },
        /*157*/ { "GS1 128 CC", "[01]12345678901231", "[20]01", 0, "symbology: 131," },
        /*158*/ { "EAN128 CC", "[01]12345678901231", "[20]01", 0, "symbology: 131," },
        /*159*/ { "dbaromncc", "[20]01", "1234567890123", 0, "symbology: 132," },
        /*160*/ { "rss14 cc", "[20]01", "1234567890123", 0, "symbology: 132," },
        /*161*/ { "databaromncc", "[20]01", "1234567890123", 0, "symbology: 132," },
        /*162*/ { "databaromnicc", "[20]01", "1234567890123", 0, "symbology: 132," },
        /*163*/ { "dbarltdcc", "[20]01", "1234567890123", 0, "symbology: 133," },
        /*164*/ { "rss ltd cc", "[20]01", "1234567890123", 0, "symbology: 133," },
        /*165*/ { "databarltdcc", "[20]01", "1234567890123", 0, "symbology: 133," },
        /*166*/ { "databarlimitedcc", "[20]01", "1234567890123", 0, "symbology: 133," },
        /*167*/ { "dbarexpcc", "[20]01", "[01]12345678901231", 0, "symbology: 134," },
        /*168*/ { "rss exp cc", "[20]01", "[01]12345678901231", 0, "symbology: 134," },
        /*169*/ { "databarexpcc", "[20]01", "[01]12345678901231", 0, "symbology: 134," },
        /*170*/ { "databar expanded cc", "[20]01", "[01]12345678901231", 0, "symbology: 134," },
        /*171*/ { "upcacc", "[20]01", "12345678901", 0, "symbology: 135," },
        /*172*/ { "upcecc", "[20]01", "1234567", 0, "symbology: 136," },
        /*173*/ { "dbar stk cc", "[20]01", "1234567890123", 0, "symbology: 137," },
        /*174*/ { "rss14stackcc", "[20]01", "1234567890123", 0, "symbology: 137," },
        /*175*/ { "databar stk cc", "[20]01", "1234567890123", 0, "symbology: 137," },
        /*176*/ { "databar stacked cc", "[20]01", "1234567890123", 0, "symbology: 137," },
        /*177*/ { "dbaromnstkcc", "[20]01", "1234567890123", 0, "symbology: 138," },
        /*178*/ { "BARCODE_RSS14_OMNI_CC", "[20]01", "1234567890123", 0, "symbology: 138," },
        /*179*/ { "databaromnstkcc", "[20]01", "1234567890123", 0, "symbology: 138," },
        /*180*/ { "databar stacked omncc", "[20]01", "1234567890123", 0, "symbology: 138," },
        /*181*/ { "databar stacked omni cc", "[20]01", "1234567890123", 0, "symbology: 138," },
        /*182*/ { "dbarexpstkcc", "[20]01", "[01]12345678901231", 0, "symbology: 139," },
        /*183*/ { "RSS EXPSTACK CC", "[20]01", "[01]12345678901231", 0, "symbology: 139," },
        /*184*/ { "databarexpstkcc", "[20]01", "[01]12345678901231", 0, "symbology: 139," },
        /*185*/ { "databar expanded stkcc", "[20]01", "[01]12345678901231", 0, "symbology: 139," },
        /*186*/ { "databar expanded stacked cc", "[20]01", "[01]12345678901231", 0, "symbology: 139," },
        /*187*/ { "Channel", "1", NULL, 0, "symbology: 140," },
        /*188*/ { "Channel Code", "1", NULL, 0, "symbology: 140," },
        /*189*/ { "CodeOne", "1", NULL, 0, "symbology: 141," },
        /*190*/ { "Grid Matrix", "1", NULL, 0, "symbology: 142," },
        /*191*/ { "UPN QR", "1", NULL, 0, "symbology: 143," },
        /*192*/ { "UPN QR Code", "1", NULL, 0, "symbology: 143," }, // Synonym
        /*193*/ { "ultra", "1", NULL, 0, "symbology: 144," },
        /*194*/ { "ultracode", "1", NULL, 0, "symbology: 144," }, // Synonym
        /*195*/ { "rMQR", "1", NULL, 0, "symbology: 145," },
        /*196*/ { "x", "1", NULL, 1, "Error 119: Invalid barcode type 'x'" },
        /*197*/ { "\177", "1", NULL, 1, "Error 119: Invalid barcode type '\177'" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[8192];
    char *outfilename = "out.gif";

    testStart("test_barcode_symbology");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        strcpy(cmd, "zint --filetype=gif");
        strcat(cmd, " --verbose");

        arg_data(cmd, "-b ", data[i].bname);
        arg_data(cmd, "-d ", data[i].data);
        arg_data(cmd, "--primary=", data[i].primary);

        strcat(cmd, " 2>&1");

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i, NULL), "i:%d exec(%s) NULL\n", i, cmd);
        assert_nonnull(strstr(buf, data[i].expected), "i:%d strstr(%s, %s) == NULL (%s)\n", i, buf, data[i].expected, cmd);
        if (!data[i].fail) {
            assert_zero(remove(outfilename), "i:%d remove(%s) != 0 (%d: %s) (%s)\n", i, outfilename, errno, strerror(errno), cmd);
        }
    }

    testFinish();
}

static void test_other_opts(int index, int debug) {

    struct item {
        int b;
        char *data;
        int input_mode;
        char *opt;
        char *opt_data;

        char *expected;
        int strstr_cmp;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "1", -1, " --bg=", "EF9900", "", 0 },
        /*  1*/ { BARCODE_CODE128, "1", -1, " -bg=", "EF9900", "", 0 },
        /*  2*/ { BARCODE_CODE128, "1", -1, " --bg=", "EF9900AA", "", 0 },
        /*  3*/ { BARCODE_CODE128, "1", -1, " --bg=", "GF9900", "Error 654: Malformed background colour 'GF9900' (hexadecimal only)", 0 },
        /*  4*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000", "", 0 },
        /*  5*/ { BARCODE_CODE128, "1", -1, " --fg=", "00000000", "", 0 },
        /*  6*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000F", "Error 651: Malformed foreground colour (6 or 8 characters only)", 0 },
        /*  7*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000FG", "Error 653: Malformed foreground colour '000000FG' (hexadecimal only)", 0 },
        /*  8*/ { BARCODE_CODE128, "1", -1, " --compliantheight", "", "", 0 },
        /*  9*/ { BARCODE_CODE128, "1", -1, " --fontsize=", "10", "", 0 },
        /* 10*/ { BARCODE_CODE128, "1", -1, " --fontsize=", "101", "Warning 126: Font size out of range (0 to 100), ignoring", 0 },
        /* 11*/ { BARCODE_CODE128, "1", -1, " --nobackground", "", "", 0 },
        /* 12*/ { BARCODE_CODE128, "1", -1, " --noquietzones", "", "", 0 },
        /* 13*/ { BARCODE_CODE128, "1", -1, " --notext", "", "", 0 },
        /* 14*/ { BARCODE_CODE128, "1", -1, " --quietzones", "", "", 0 },
        /* 15*/ { BARCODE_CODE128, "1", -1, " --reverse", "", "", 0 },
        /* 16*/ { BARCODE_CODE128, "1", -1, " --werror", NULL, "", 0 },
        /* 17*/ { 19, "1", -1, " --werror", NULL, "Error 207: Codabar 18 not supported", 0 },
        /* 18*/ { BARCODE_GS1_128, "[01]12345678901231", -1, "", NULL, "", 0 },
        /* 19*/ { BARCODE_GS1_128, "0112345678901231", -1, "", NULL, "Error 252: Data does not start with an AI", 0 },
        /* 20*/ { BARCODE_GS1_128, "0112345678901231", -1, " --gs1nocheck", NULL, "Error 252: Data does not start with an AI", 0 },
        /* 21*/ { BARCODE_GS1_128, "[00]376104250021234569", -1, "", NULL, "", 0 },
        /* 22*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, "", NULL, "Warning 261: AI (00) position 18: Bad checksum '8', expected '9'", 0 },
        /* 23*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, " --gs1nocheck", NULL, "", 0 },
        /* 24*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, " --werror", NULL, "Error 261: AI (00) position 18: Bad checksum '8', expected '9'", 0 },
        /* 25*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "1", "Error 155: Invalid Structured Append argument, expect \"index,count[,ID]\"", 0 },
        /* 26*/ { BARCODE_AZTEC, "1", -1, " --structapp=", ",", "Error 156: Structured Append index too short", 0 },
        /* 27*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "1234567890,", "Error 156: Structured Append index too long", 0 },
        /* 28*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,", "Error 159: Structured Append count too short", 0 },
        /* 29*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,1234567890", "Error 159: Structured Append count too long", 0 },
        /* 30*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,123456789,", "Error 158: Structured Append ID too short", 0 },
        /* 31*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,1234567890,", "Error 157: Structured Append count too long", 0 },
        /* 32*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,123456789,123456789012345678901234567890123", "Error 158: Structured Append ID too long", 0 },
        /* 33*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "123456789,123456789,12345678901234567890123456789012", "Error 701: Structured Append count out of range (2-26)", 0 },
        /* 34*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "26,26,12345678901234567890123456789012", "", 0 },
        /* 35*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "A,26,12345678901234567890123456789012", "Error 160: Invalid Structured Append index (digits only)", 0 },
        /* 36*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "26,A,12345678901234567890123456789012", "Error 161: Invalid Structured Append count (digits only)", 0 },
        /* 37*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "26,1,12345678901234567890123456789012", "Error 162: Invalid Structured Append count, must be >= 2", 0 },
        /* 38*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "0,2,12345678901234567890123456789012", "Error 163: Structured Append index out of range (1-2)", 0 },
        /* 39*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "3,2,12345678901234567890123456789012", "Error 163: Structured Append index out of range (1-2)", 0 },
        /* 40*/ { BARCODE_AZTEC, "1", -1, " --structapp=", "2,3,12345678901234567890123456789012", "", 0 },
        /* 41*/ { BARCODE_PDF417, "1", -1, " --heightperrow", "", "", 0 },
        /* 42*/ { -1, NULL, -1, " -v", NULL, "Zint version ", 1 },
        /* 43*/ { -1, NULL, -1, " --version", NULL, "Zint version ", 1 },
        /* 44*/ { -1, NULL, -1, " -h", NULL, "Encode input data in a barcode ", 1 },
        /* 45*/ { -1, NULL, -1, " -e", NULL, "3: ISO/IEC 8859-1 ", 1 },
        /* 46*/ { -1, NULL, -1, " -t", NULL, "1 CODE11 ", 1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char cmd[4096];
    char buf[8192];

    testStart("test_other_opts");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

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
        }
    }

    testFinish();
}

static void test_exit_status(int index, int debug) {

    struct item {
        int b;
        char *data;
        int input_mode;
        char *opt;
        char *opt_data;

        int expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "1", -1, " --bg=", "EF9900", 0 },
        /*  1*/ { BARCODE_CODE128, "1", -1, " --bg=", "GF9900", ZINT_ERROR_INVALID_OPTION }, // Caught by libzint
        /*  2*/ { BARCODE_CODE128, "1", -1, " --border=", "1001", ZINT_WARN_INVALID_OPTION }, // Caught by CLI
        /*  3*/ { BARCODE_CODE128, "1", -1, " --data=", "\200", ZINT_ERROR_INVALID_DATA }, // Caught by libzint
        /*  4*/ { BARCODE_CODE128, "1", -1, " --separator=", "-1", ZINT_ERROR_INVALID_OPTION }, // Caught by CLI
    };
    int data_size = ARRAY_SIZE(data);
    int i;
    int exit_status;

    char cmd[4096];
    char buf[8192];

    testStart("test_exit_status");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

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
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_dump_args", test_dump_args, 1, 0, 1 },
        { "test_dump_segs", test_dump_segs, 1, 0, 1 },
        { "test_input", test_input, 1, 0, 1 },
        { "test_stdin_input", test_stdin_input, 1, 0, 1 },
        { "test_batch_input", test_batch_input, 1, 0, 1 },
        { "test_batch_large", test_batch_large, 1, 0, 1 },
        { "test_checks", test_checks, 1, 0, 1 },
        { "test_barcode_symbology", test_barcode_symbology, 1, 0, 1 },
        { "test_other_opts", test_other_opts, 1, 0, 1 },
        { "test_exit_status", test_exit_status, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
