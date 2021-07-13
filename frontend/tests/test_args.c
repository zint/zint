/*
    libzint - the open source barcode library
    Copyright (C) 2020 - 2021 Robin Stuart <rstuart114@gmail.com>

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

static char *exec(const char *cmd, char *buf, int buf_size, int debug, int index) {
    FILE *fp;
    int cnt;
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
        testutil_pclose(fp);
        return NULL;
    }
    testutil_pclose(fp);

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
        if (input_mode & GS1PARENS_MODE) {
            sprintf(cmd + (int) strlen(cmd), "%s--gs1parens", strlen(cmd) ? " " : "");
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
        /* 17*/ { BARCODE_PDF417, "123", NULL, NULL, NULL,        -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL,  1,  0, 0, -1, "FF 54 7A BC 3D 4F 1D 5C 0F E8 A4\nFF 54 7A 90 2F D3 1F AB 8F E8 A4\nFF 54 6A F8 3A BF 15 3C 0F E8 A4\nFF 54 57 9E 24 E7 1A F7 CF E8 A4\nFF 54 7A E7 3D 0D 9D 73 0F E8 A4\nFF 54 7D 70 B9 CB DF 5E CF E8 A4" },
        /* 18*/ { BARCODE_DATAMATRIX, "ABC", NULL, NULL, NULL, -1,          -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA 8\nB3 4\n8F 0\nB2 C\nA6 0\nBA C\nD6 0\nEB 4\nE2 8\nFF C" },
        /* 19*/ { BARCODE_DATAMATRIX, "ABC", NULL, NULL, NULL, -1, READER_INIT, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A\nAC 7\n8A 4\nA0 3\nC2 2\nB5 1\n82 2\nBA 7\n8C C\nA0 5\n86 A\nFF F" },
        /* 20*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJK", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA AA AA\nA6 ED A9 D1\nB2 FE 92 7E\n98 E7 C3 FF\nE8 D0 90 CC\nC7 EB 8D 63\nC5 48 D3 C4\nFF FF FF FF" },
        /* 21*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJK", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 1, -1, "AA AA\nA6 FF\nB2 C0\n99 19\nA8 BA\n84 EF\nC0 9C\nE9 47\nE5 90\nB3 49\n91 36\nB3 7D\nD4 56\nF7 DD\nC5 62\nFF FF" },
        /* 22*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA A8\nA6 94 A4\nB2 AD 98\n99 09 3C\nA9 E1 00\n86 80 F4\nC2 F5 68\nF5 D5 1C\nF2 68 70\nDA 7A 1C\nB7 FA 30\nA8 E0 F4\n91 74 D8\nD6 1D 74\nD2 31 08\nF1 E9 74\nC2 BF A8\nA6 8B 6C\n83 3D D8\nF9 ED CC\nAB 5A 58\nFF FF FC" },
        /* 23*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 1, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA AA AA AA AA AA AA AA\nA6 D9 C8 0F 82 EB 81 97\nB2 BA A7 C8 F5 64 BD 2E\n99 2F EF 29 84 07 9F C7\nA8 84 99 C2 F2 1A 9F 44\n86 D5 D9 8B EF 41 BB 7F\n85 44 BF 28 F7 80 99 76\nFF FF FF FF FF FF FF FF" },
        /* 24*/ { BARCODE_DATAMATRIX, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", NULL, NULL, NULL, -1, -1, 0, -1, 1, -1, 0, -1, -1, NULL, -1, -1, 1, -1, "AA AA A8\nA6 94 A4\nB2 AD 98\n99 09 3C\nA9 E1 00\n86 80 F4\nC2 F5 68\nF5 D5 1C\nF2 68 70\nDA 7A 1C\nB7 FA 30\nA8 E0 F4\n91 74 D8\nD6 1D 74\nD2 31 08\nF1 E9 74\nC2 BF A8\nA6 8B 6C\n83 3D D8\nF9 ED CC\nAB 5A 58\nFF FF FC" },
        /* 25*/ { BARCODE_DATAMATRIX, "[91]12[92]34", NULL, NULL, NULL, GS1_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nFA 9C\nBC 00\nD7 84\nED E0\nA4 E4\nA7 40\n9D 3C\nBF 50\nFA 24\nB1 68\nE5 04\n92 70\nFF FC" },
        /* 26*/ { BARCODE_DATAMATRIX, "[91]12[92]34", NULL, NULL, NULL, GS1_MODE, GS1_GS_SEPARATOR, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 27*/ { BARCODE_DATAMATRIX, "[9\\x31]12[92]34", NULL, NULL, NULL, GS1_MODE | ESCAPE_MODE, GS1_GS_SEPARATOR, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 28*/ { BARCODE_DATAMATRIX, "(9\\x31)12(92)34", NULL, NULL, NULL, GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, GS1_GS_SEPARATOR, 0, -1, 0, -1, 0, -1, -1, NULL, -1, -1, 0, -1, "AA A8\nF9 DC\nBF 20\nD6 C4\nED 10\nA0 0C\nA7 C0\n96 5C\nBA 70\nBB A4\nE2 18\nDD 14\n9C 40\nFF FC" },
        /* 29*/ { BARCODE_EANX_CC, "[91]12", NULL, NULL, NULL,    -1, -1, 0, -1, 0, -1, 0, -1, -1, "12345678+12", -1, -1, 0, -1, "DB BC D3 9C 44 E9 D2 2C 19 E7 A2 D8 A0 00 00 00\nDB 31 1C 9C C7 29 92 47 D9 E9 40 C8 A0 00 00 00\nDA 3B EB 10 AF 09 9A 18 9D 7D 82 E8 A0 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n20 00 00 00 00 00 00 00 00 00 00 00 20 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n14 68 D1 A6 49 BD 55 C9 D4 22 48 B9 40 59 94 98" },
        /* 30*/ { BARCODE_EANX_CC, "[91]12", NULL, NULL, NULL,    -1, -1, 0, -1, 0, -1, 0, -1,  2, "12345678+12", -1, -1, 0, -1, "D3 A3 E9 DB F5 C9 DB 43 D9 CB 98 D2 20 00 00 00\nD3 25 0F 11 E4 49 D3 51 F1 AC FC D6 20 00 00 00\nD1 33 48 19 39 E9 93 18 49 D8 98 D7 20 00 00 00\nD1 A6 FC DA 1C 49 9B C5 05 E2 84 D7 A0 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n20 00 00 00 00 00 00 00 00 00 00 00 20 00 00 00\n10 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00\n14 68 D1 A6 49 BD 55 C9 D4 22 48 B9 40 59 94 98" },
        /* 31*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 32*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, 0, 26, 0, -1, -1, NULL, -1,  1, 0, -1, "FE 5B F8\n82 72 08\nBA DA E8\nBA 52 E8\nBA 2A E8\n82 0A 08\nFE AB F8\n00 D8 00\nEF F6 20\nB5 C2 28\n36 28 88\nFD 42 10\n62 2A C8\n00 95 70\nFE B7 38\n82 FD D8\nBA 97 00\nBA 43 60\nBA C8 C8\n82 C3 68\nFE EA F8" },
        /* 33*/ { BARCODE_QRCODE, "\223\137", NULL, NULL, NULL, DATA_MODE, -1, 0, -1, 0, -1, 0, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 0A 08\nBA A2 E8\nBA 0A E8\nBA 5A E8\n82 72 08\nFE AB F8\n00 A0 00\nEF AE 20\n75 B5 20\n82 F7 58\nF4 9D C8\n5E 17 28\n00 C2 20\nFE 88 80\n82 82 38\nBA EA A8\nBA 55 50\nBA D7 68\n82 BD D0\nFE B7 78" },
        /* 34*/ { BARCODE_QRCODE, "\223\137", NULL, NULL, NULL, DATA_MODE, -1, 0, -1, 0, -1, 1, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 35*/ { BARCODE_QRCODE, "\\x93\\x5F", NULL, NULL, NULL, DATA_MODE | ESCAPE_MODE, -1, 0, -1, 0, -1, 1, -1, -1, NULL, -1,  1, 0, -1, "FE 2B F8\n82 AA 08\nBA B2 E8\nBA 0A E8\nBA FA E8\n82 E2 08\nFE AB F8\n00 80 00\nD3 3B B0\n60 95 68\n7A B3 A0\n1D 0F 98\nAA D7 30\n00 E6 A8\nFE DA D0\n82 42 20\nBA 0E 38\nBA C7 18\nBA 17 68\n82 B9 40\nFE C5 28" },
        /* 36*/ { BARCODE_QRCODE, "点", NULL, NULL, NULL,         -1, -1, 0, -1, 0, -1, 0, 2, -1, NULL, -1,  1, 0, -1, "FE 4B F8\n82 92 08\nBA 42 E8\nBA 92 E8\nBA 3A E8\n82 EA 08\nFE AB F8\n00 38 00\nFB CD 50\nA5 89 18\n0B 74 B8\nFC 81 A0\n92 34 B8\n00 DE 48\nFE AB 10\n82 5E 50\nBA C9 20\nBA C9 20\nBA F4 E0\n82 81 A0\nFE B4 E8" },
        /* 37*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, 0, -1, 1, -1, -1, NULL, -1, -1, 0, -1, "FE 8A FE\n80 28 02\nBE E8 FA\nA0 94 0A\nAE 3E EA\nAE D2 EA\nAE 74 EA\n00 AA 00\n15 B4 AA\n0B 48 74\nA2 4A A4\nB5 56 2C\nA8 5A A8\n9F 18 50\nAA 07 50\n00 A6 00\nFE 20 EA\n02 C2 EA\nFA C4 EA\n0A 42 0A\nEA 52 FA\nEA 24 02\nEA AA FE" },
        /* 38*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, 0, -1, 1, 3, -1, NULL, -1, -1, 0, -1, "FE 16 FE\n80 E2 02\nBE C2 FA\nA0 A0 0A\nAE F6 EA\nAE 98 EA\nAE BA EA\n00 E0 00\n15 83 AA\n44 7E AE\n92 9C 78\n25 BF 08\n47 4B 8C\n0D F9 74\nAB E7 50\n00 3A 00\nFE C2 EA\n02 22 EA\nFA DA EA\n0A 22 0A\nEA B2 FA\nEA 9A 02\nEA E8 FE" },
        /* 39*/ { BARCODE_HANXIN, "é", NULL, NULL, NULL,  DATA_MODE, -1, 0, -1, 0, -1, 1, 4, -1, NULL, -1, -1, 0, -1, "FE 8A FE\n80 28 02\nBE E8 FA\nA0 94 0A\nAE 3E EA\nAE D2 EA\nAE 74 EA\n00 AA 00\n15 B4 AA\n0B 48 74\nA2 4A A4\nB5 56 2C\nA8 5A A8\n9F 18 50\nAA 07 50\n00 A6 00\nFE 20 EA\n02 C2 EA\nFA C4 EA\n0A 42 0A\nEA 52 FA\nEA 24 02\nEA AA FE" },
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

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i), "i:%d exec(%s) NULL\n", i, cmd);
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

static void test_input(int index, int debug) {

#define TEST_INPUT_LONG "test_67890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"

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
        /*  0*/ { BARCODE_CODE128, 1, -1, 0, "gif", "123\n456\n", "test_batch~.gif", 2, "test_batch1.gif\000test_batch2.gif" },
        /*  1*/ { BARCODE_CODE128, 1, -1, 1, "gif", "123\n456\n7890123456789\n", NULL, 3, "123.gif\000456.gif\0007890123456789.gif" },
        /*  2*/ { BARCODE_CODE128, 1, -1, 1, "svg", "123\n456\n7890123456789\n", NULL, 3, "123.svg\000456.svg\0007890123456789.svg" },
        /*  3*/ { BARCODE_CODE128, 1, -1, 1, "gif", "123\n456\n7890123456789\nA\\xA0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_xA0B.gif" },
        /*  4*/ { BARCODE_CODE128, 1, ESCAPE_MODE, 1, "gif", "123\n456\n7890123456789\nA\\xA0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_B.gif" },
        /*  5*/ { BARCODE_CODE128, 1, -1, 1, "gif", "123\n456\n7890123456789\nA\\u00A0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_u00A0B.gif" },
        /*  6*/ { BARCODE_CODE128, 1, ESCAPE_MODE, 1, "gif", "123\n456\n7890123456789\nA\\u00A0B\n", NULL, 4, "123.gif\000456.gif\0007890123456789.gif\000A_B.gif" },
        /*  7*/ { BARCODE_CODE128, 1, -1, 0, "gif", "\n", "test_batch.gif", 0, NULL },
        /*  8*/ { BARCODE_CODE128, 1, -1, 0, "gif", "123\n456\n", TEST_INPUT_LONG "~.gif", 2, TEST_INPUT_LONG "1.gif\000" TEST_INPUT_LONG "2.gif" },
        /*  9*/ { BARCODE_CODE128, 0, -1, 0, "svg", "123", TEST_INPUT_LONG "1.gif", 1, TEST_INPUT_LONG "1.svg" },
        /* 10*/ { BARCODE_CODE128, 1, -1, 0, "svg", "123\n", TEST_INPUT_LONG "1.gif", 1, TEST_INPUT_LONG "1.svg" },
        /* 11*/ { BARCODE_CODE128, 1, -1, 0, "gif", "123\n", "test_batch.jpeg", 1, "test_batch.jpeg.gif" },
        /* 12*/ { BARCODE_CODE128, 1, -1, 0, "gif", "123\n", "test_batch.jpg", 1, "test_batch.gif" },
        /* 13*/ { BARCODE_CODE128, 1, -1, 0, "emf", "123\n", "test_batch.jpeg", 1, "test_batch.jpeg.emf" },
        /* 14*/ { BARCODE_CODE128, 1, -1, 0, "emf", "123\n", "test_batch.jpg", 1, "test_batch.emf" },
        /* 15*/ { BARCODE_CODE128, 1, -1, 0, "eps", "123\n", "test_batch.ps", 1, "test_batch.eps" },
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

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i), "i:%d exec(%s) NULL\n", i, cmd);

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

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i), "i:%d exec(%s) NULL\n", i, cmd);

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

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i), "i:%d exec(%s) NULL\n", i, cmd);
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
        /*  0*/ { BARCODE_HANXIN, 0, "1", 7827, "out.gif" },
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

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i), "i:%d exec(%s) NULL\n", i, cmd);
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
        int height;
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
        /*  0*/ { -2, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 139: Invalid add-on gap value (digits only)" },
        /*  1*/ {  6, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 140: Add-on gap out of range (7 to 12), ignoring" },
        /*  2*/ { 13, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 140: Add-on gap out of range (7 to 12), ignoring" },
        /*  3*/ { -1, -2,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 107: Invalid border width value (digits only)" },
        /*  4*/ { -1, 1001, -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 108: Border width out of range (0 to 1000), ignoring" },
        /*  5*/ { -1, -1,   -1, 0.009, -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 106: Invalid dot radius value (less than 0.01), ignoring" },
        /*  6*/ { -1, -1,   -2, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 131: Invalid columns value (digits only)" },
        /*  7*/ { -1, -1,  201, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 111: Number of columns out of range (1 to 200), ignoring" },
        /*  8*/ { -1, -1,   -1, -1,    -2,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 138: Invalid ECI value (digits only)" },
        /*  9*/ { -1, -1,   -1, -1,    1000000, NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 118: ECI code out of range (0 to 999999), ignoring" },
        /* 10*/ { -1, -1,   -1, -1,    -1,      "jpg", -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 142: File type 'jpg' not supported, ignoring" },
        /* 11*/ { -1, -1,   -1, -1,    -1,      NULL,  -2, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 110: Symbol height '-2' out of range (0.5 to 1000), ignoring" },
        /* 12*/ { -1, -1,   -1, -1,    -1,      NULL,   0, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 110: Symbol height '0' out of range (0.5 to 1000), ignoring" },
        /* 13*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -2, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 148: Invalid mask value (digits only)" },
        /* 14*/ { -1, -1,   -1, -1,    -1,      NULL,  -1,  8, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 147: Mask value out of range (0 to 7), ignoring" },
        /* 15*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1,  7, -1, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 116: Mode value out of range (0 to 6), ignoring" },
        /* 16*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -2, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Error 117: Invalid rotation value (digits only)" },
        /* 17*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, 45, -1, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 137: Invalid rotation parameter (0, 90, 180 or 270 only), ignoring" },
        /* 18*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -2, -1,   -1, -1, -1, -1,   -1,   -1, "Error 132: Invalid rows value (digits only)" },
        /* 19*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, 45, -1,   -1, -1, -1, -1,   -1,   -1, "Warning 112: Number of rows out of range (1 to 44), ignoring" },
        /* 20*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -2,   -1, -1, -1, -1,   -1,   -1, "Warning 105: Invalid scale value (less than 0.01), ignoring" },
        /* 21*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, 0.49, -1, -1, -1, -1,   -1,   -1, "Warning 146: Scaling less than 0.5 will be set to 0.5 for 'gif' output" },
        /* 22*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -2, -1, -1, -1,   -1,   -1, "Error 149: Invalid Structured Carrier Message version value (digits only)" },
        /* 23*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,  100, -1, -1, -1,   -1,   -1, "Warning 150: Structured Carrier Message version out of range (0 to 99), ignoring" },
        /* 24*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -2, -1, -1,   -1,   -1, "Error 134: Invalid ECC value (digits only)" },
        /* 25*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1,  9, -1, -1,   -1,   -1, "Warning 114: ECC level out of range (0 to 8), ignoring" },
        /* 26*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -2, -1,   -1,   -1, "Error 128: Invalid separator value (digits only)" },
        /* 27*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1,  5, -1,   -1,   -1, "Warning 127: Separator value out of range (0 to 4), ignoring" },
        /* 28*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -2,   -1,   -1, "Error 133: Invalid version value (digits only)" },
        /* 29*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, 85,   -1,   -1, "Warning 113: Version value out of range (1 to 84), ignoring" },
        /* 30*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -2,   -1, "Error 153: Invalid vertical whitespace value '-2' (digits only)" },
        /* 31*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, 1001,   -1, "Warning 154: Vertical whitespace value out of range (0 to 1000), ignoring" },
        /* 32*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1,   -2, "Error 120: Invalid horizontal whitespace value '-2' (digits only)" },
        /* 33*/ { -1, -1,   -1, -1,    -1,      NULL,  -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1,   -1, 1001, "Warning 121: Horizontal whitespace value out of range (0 to 1000), ignoring" },
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
        arg_int(cmd, "--height=", data[i].height);
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

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i), "i:%d exec(%s) NULL\n", i, cmd);
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
        /*  7*/ { "c25matrix", "1", NULL, 1, "Error 119: Invalid barcode type 'c25matrix'" }, // Legacy not supported
        /*  8*/ { "C25INTER", "1", NULL, 0, "symbology: 3," },
        /*  9*/ { "c25IATA", "1", NULL, 0, "symbology: 4," },
        /* 10*/ { "c25 Logic", "1", NULL, 0, "symbology: 6," },
        /* 11*/ { "c25 Ind", "1", NULL, 0, "symbology: 7," },
        /* 12*/ { "code39", "1", NULL, 0, "symbology: 8," },
        /* 13*/ { "excode 39", "1", NULL, 0, "symbology: 9," },
        /* 14*/ { "eanx", "1", NULL, 0, "symbology: 13," },
        /* 15*/ { "ean", "1", NULL, 0, "symbology: 13," },
        /* 16*/ { "eanx chk", "1", NULL, 0, "symbology: 14," },
        /* 17*/ { "eanxchk", "1", NULL, 0, "symbology: 14," },
        /* 18*/ { "eanchk", "1", NULL, 0, "symbology: 14," },
        /* 19*/ { "GS1128", "[01]12345678901231", NULL, 0, "symbology: 16," },
        /* 20*/ { "coda bar", "A1B", NULL, 0, "symbology: 18," },
        /* 21*/ { "DPLEIT", "1", NULL, 0, "symbology: 21," },
        /* 22*/ { "DPIDENT", "1", NULL, 0, "symbology: 22," },
        /* 23*/ { "code16k", "1", NULL, 0, "symbology: 23," },
        /* 24*/ { "CODE49", "1", NULL, 0, "symbology: 24," },
        /* 25*/ { "CODE93", "1", NULL, 0, "symbology: 25," },
        /* 26*/ { "flat", "1", NULL, 0, "symbology: 28," },
        /* 27*/ { "dbar omn", "1", NULL, 0, "symbology: 29," },
        /* 28*/ { "dbar ltd", "1", NULL, 0, "symbology: 30," },
        /* 29*/ { "dbarexp", "[10]12", NULL, 0, "symbology: 31," },
        /* 30*/ { "telepen", "1", NULL, 0, "symbology: 32," },
        /* 31*/ { "upc", "1", NULL, 1, "Error 119: Invalid barcode type 'upc'" },
        /* 32*/ { "upca", "1", NULL, 0, "symbology: 34," },
        /* 33*/ { "upca_chk", "123456789012", NULL, 0, "symbology: 35," },
        /* 34*/ { "upce", "1", NULL, 0, "symbology: 37," },
        /* 35*/ { "upce chk", "12345670", NULL, 0, "symbology: 38," },
        /* 36*/ { "POSTNET ", "12345678901", NULL, 0, "symbology: 40," },
        /* 37*/ { "msi", "1", NULL, 0, "symbology: 47," },
        /* 38*/ { "MSI Plessey ", "1", NULL, 0, "symbology: 47," },
        /* 39*/ { "fim ", "A", NULL, 0, "symbology: 49," },
        /* 40*/ { "LOGMARS", "123456", NULL, 0, "symbology: 50," },
        /* 41*/ { " pharma", "123456", NULL, 0, "symbology: 51," },
        /* 42*/ { " pzn ", "1", NULL, 0, "symbology: 52," },
        /* 43*/ { "pharma two", "4", NULL, 0, "symbology: 53," },
        /* 44*/ { "BARCODE_PDF417", "1", NULL, 0, "symbology: 55," },
        /* 45*/ { "barcodepdf417comp", "1", NULL, 0, "symbology: 56," },
        /* 46*/ { "MaxiCode", "1", NULL, 0, "symbology: 57," },
        /* 47*/ { "QR CODE", "1", NULL, 0, "symbology: 58," },
        /* 48*/ { "qr", "1", NULL, 0, "symbology: 58," }, // Synonym
        /* 49*/ { "Code 128 B", "1", NULL, 0, "symbology: 60," },
        /* 50*/ { "AUS POST", "12345678901234567890123", NULL, 0, "symbology: 63," },
        /* 51*/ { "AusReply", "12345678", NULL, 0, "symbology: 66," },
        /* 52*/ { "AUSROUTE", "12345678", NULL, 0, "symbology: 67," },
        /* 53*/ { "AUS REDIRECT", "12345678", NULL, 0, "symbology: 68," },
        /* 54*/ { "isbnx", "123456789", NULL, 0, "symbology: 69," },
        /* 55*/ { "rm4scc", "1", NULL, 0, "symbology: 70," },
        /* 56*/ { "DataMatrix", "1", NULL, 0, "symbology: 71," },
        /* 57*/ { "EAN14", "1", NULL, 0, "symbology: 72," },
        /* 58*/ { "vin", "12345678701234567", NULL, 0, "symbology: 73," },
        /* 59*/ { "CodaBlock-F", "1", NULL, 0, "symbology: 74," },
        /* 60*/ { "NVE18", "1", NULL, 0, "symbology: 75," },
        /* 61*/ { "Japan Post", "1", NULL, 0, "symbology: 76," },
        /* 62*/ { "Korea Post", "1", NULL, 0, "symbology: 77," },
        /* 63*/ { "DBar Stk", "1", NULL, 0, "symbology: 79," },
        /* 64*/ { "DBar Omn Stk", "1", NULL, 0, "symbology: 80," },
        /* 65*/ { "DBar Exp Stk", "[20]01", NULL, 0, "symbology: 81," },
        /* 66*/ { "planet", "12345678901", NULL, 0, "symbology: 82," },
        /* 67*/ { "MicroPDF417", "1", NULL, 0, "symbology: 84," },
        /* 68*/ { "USPS IMail", "12345678901234567890", NULL, 0, "symbology: 85," },
        /* 69*/ { "plessey", "1", NULL, 0, "symbology: 86," },
        /* 70*/ { "telepen num", "1", NULL, 0, "symbology: 87," },
        /* 71*/ { "ITF14", "1", NULL, 0, "symbology: 89," },
        /* 72*/ { "KIX", "1", NULL, 0, "symbology: 90," },
        /* 73*/ { "Aztec", "1", NULL, 0, "symbology: 92," },
        /* 74*/ { "daft", "D", NULL, 0, "symbology: 93," },
        /* 75*/ { "DPD", "0123456789012345678901234567", NULL, 0, "symbology: 96," },
        /* 76*/ { "Micro QR", "1", NULL, 0, "symbology: 97," },
        /* 77*/ { "hibc128", "1", NULL, 0, "symbology: 98," },
        /* 78*/ { "hibccode128", "1", NULL, 0, "symbology: 98," }, // Synonym
        /* 79*/ { "hibc39", "1", NULL, 0, "symbology: 99," },
        /* 80*/ { "hibccode39", "1", NULL, 0, "symbology: 99," }, // Synonym
        /* 81*/ { "hibcdatamatrix", "1", NULL, 0, "symbology: 102," }, // Synonym
        /* 82*/ { "hibcdm", "1", NULL, 0, "symbology: 102," },
        /* 83*/ { "HIBC qr", "1", NULL, 0, "symbology: 104," },
        /* 84*/ { "HIBC QR Code", "1", NULL, 0, "symbology: 104," }, // Synonym
        /* 85*/ { "HIBCPDF", "1", NULL, 0, "symbology: 106," },
        /* 86*/ { "HIBCPDF417", "1", NULL, 0, "symbology: 106," }, // Synonym
        /* 87*/ { "HIBCMICPDF", "1", NULL, 0, "symbology: 108," },
        /* 88*/ { "HIBC Micro PDF", "1", NULL, 0, "symbology: 108," }, // Synonym
        /* 89*/ { "HIBC Micro PDF417", "1", NULL, 0, "symbology: 108," }, // Synonym
        /* 90*/ { "HIBC BlockF", "1", NULL, 0, "symbology: 110," },
        /* 91*/ { "HIBC CodaBlock-F", "1", NULL, 0, "symbology: 110," }, // Synonym
        /* 92*/ { "HIBC Aztec", "1", NULL, 0, "symbology: 112," },
        /* 93*/ { "DotCode", "1", NULL, 0, "symbology: 115," },
        /* 94*/ { "Han Xin", "1", NULL, 0, "symbology: 116," },
        /* 95*/ { "Mailmark", "01000000000000000AA00AA0A", NULL, 0, "symbology: 121," },
        /* 96*/ { "azrune", "1", NULL, 0, "symbology: 128," },
        /* 97*/ { "aztecrune", "1", NULL, 0, "symbology: 128," }, // Synonym
        /* 98*/ { "aztecrunes", "1", NULL, 0, "symbology: 128," }, // Synonym
        /* 99*/ { "code32", "1", NULL, 0, "symbology: 129," },
        /*100*/ { "eanx cc", "[20]01", "1234567890128", 0, "symbology: 130," },
        /*101*/ { "eancc", "[20]01", "1234567890128", 0, "symbology: 130," },
        /*102*/ { "GS1 128 CC", "[01]12345678901231", "[20]01", 0, "symbology: 131," },
        /*103*/ { "dbaromncc", "[20]01", "1234567890123", 0, "symbology: 132," },
        /*104*/ { "dbarltdcc", "[20]01", "1234567890123", 0, "symbology: 133," },
        /*105*/ { "dbarexpcc", "[20]01", "[01]12345678901231", 0, "symbology: 134," },
        /*106*/ { "upcacc", "[20]01", "12345678901", 0, "symbology: 135," },
        /*107*/ { "upcecc", "[20]01", "1234567", 0, "symbology: 136," },
        /*108*/ { "dbar stk cc", "[20]01", "1234567890123", 0, "symbology: 137," },
        /*109*/ { "dbaromnstkcc", "[20]01", "1234567890123", 0, "symbology: 138," },
        /*110*/ { "dbarexpstkcc", "[20]01", "[01]12345678901231", 0, "symbology: 139," },
        /*111*/ { "Channel", "1", NULL, 0, "symbology: 140," },
        /*112*/ { "CodeOne", "1", NULL, 0, "symbology: 141," },
        /*113*/ { "Grid Matrix", "1", NULL, 0, "symbology: 142," },
        /*114*/ { "UPN QR", "1", NULL, 0, "symbology: 143," },
        /*115*/ { "UPN QR Code", "1", NULL, 0, "symbology: 143," }, // Synonym
        /*116*/ { "ultra", "1", NULL, 0, "symbology: 144," },
        /*117*/ { "ultracode", "1", NULL, 0, "symbology: 144," }, // Synonym
        /*118*/ { "rMQR", "1", NULL, 0, "symbology: 145," },
        /*119*/ { "x", "1", NULL, 1, "Error 119: Invalid barcode type 'x'" },
        /*120*/ { "\177", "1", NULL, 1, "Error 119: Invalid barcode type '\177'" },
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

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i), "i:%d exec(%s) NULL\n", i, cmd);
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
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "1", -1, " --bg=", "EF9900", "" },
        /*  1*/ { BARCODE_CODE128, "1", -1, " --bg=", "EF9900AA", "" },
        /*  2*/ { BARCODE_CODE128, "1", -1, " --bg=", "GF9900", "Error 654: Malformed background colour target" },
        /*  3*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000", "" },
        /*  4*/ { BARCODE_CODE128, "1", -1, " --fg=", "00000000", "" },
        /*  5*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000F", "Error 651: Malformed foreground colour target" },
        /*  6*/ { BARCODE_CODE128, "1", -1, " --fg=", "000000FG", "Error 653: Malformed foreground colour target" },
        /*  7*/ { BARCODE_CODE128, "1", -1, " --fontsize=", "10", "" },
        /*  8*/ { BARCODE_CODE128, "1", -1, " --fontsize=", "101", "Warning 126: Font size out of range (0 to 100), ignoring" },
        /*  9*/ { BARCODE_CODE128, "1", -1, " --nobackground", "", "" },
        /* 10*/ { BARCODE_CODE128, "1", -1, " --notext", "", "" },
        /* 11*/ { BARCODE_CODE128, "1", -1, " --reverse", "", "" },
        /* 12*/ { BARCODE_CODE128, "1", -1, " --werror", NULL, "" },
        /* 13*/ { 19, "1", -1, " --werror", NULL, "Error 207: Codabar 18 not supported" },
        /* 14*/ { BARCODE_GS1_128, "[01]12345678901231", -1, "", NULL, "" },
        /* 15*/ { BARCODE_GS1_128, "0112345678901231", -1, "", NULL, "Error 252: Data does not start with an AI" },
        /* 16*/ { BARCODE_GS1_128, "0112345678901231", -1, " --gs1nocheck", NULL, "Error 252: Data does not start with an AI" },
        /* 17*/ { BARCODE_GS1_128, "[00]376104250021234569", -1, "", NULL, "" },
        /* 18*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, "", NULL, "Warning 261: AI (00) position 18: Bad checksum '8', expected '9'" },
        /* 19*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, " --gs1nocheck", NULL, "" },
        /* 20*/ { BARCODE_GS1_128, "[00]376104250021234568", -1, " --werror", NULL, "Error 261: AI (00) position 18: Bad checksum '8', expected '9'" },
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

        assert_nonnull(exec(cmd, buf, sizeof(buf) - 1, debug, i), "i:%d exec(%s) NULL\n", i, cmd);
        assert_zero(strcmp(buf, data[i].expected), "i:%d buf (%s) != expected (%s) (%s)\n", i, buf, data[i].expected, cmd);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_dump_args", test_dump_args, 1, 0, 1 },
        { "test_input", test_input, 1, 0, 1 },
        { "test_stdin_input", test_stdin_input, 1, 0, 1 },
        { "test_batch_input", test_batch_input, 1, 0, 1 },
        { "test_batch_large", test_batch_large, 1, 0, 1 },
        { "test_checks", test_checks, 1, 0, 1 },
        { "test_barcode_symbology", test_barcode_symbology, 1, 0, 1 },
        { "test_other_opts", test_other_opts, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
