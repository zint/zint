/* main.c - Command line handling routines for Zint */
/*
    libzint - the open source barcode library
    Copyright (C) 2008-2024 Robin Stuart <rstuart114@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_MSC_VER) && !defined(__NetBSD__) && !defined(_AIX)
#  include <getopt.h>
#  include <zint.h>
#else
#  include "../getopt/getopt.h"
#  ifdef _MSC_VER
#    include "zint.h"
#    if _MSC_VER > 1200 /* VC6 */
#      pragma warning(disable: 4996) /* function or variable may be unsafe */
#    endif
#  else
#    include <zint.h>
#  endif
#endif

/* Following copied from "backend/library.c" */

/* It's assumed that int is at least 32 bits, the following will compile-time fail if not
 * https://stackoverflow.com/a/1980056 */
typedef char static_assert_int_at_least_32bits[sizeof(int) * CHAR_BIT < 32 ? -1 : 1];

/* Following copied from "backend/common.h" */

#define ARRAY_SIZE(x) ((int) (sizeof(x) / sizeof((x)[0])))

#ifdef _MSC_VER
#  include <malloc.h>
#  define z_alloca(nmemb) _alloca(nmemb)
#elif defined(__COMPCERT__)
#  define z_alloca(nmemb) malloc(nmemb) /* So links - leads to loads of leaks obs */
#else
#  if (defined(__GNUC__) && !defined(alloca) && !defined(__NetBSD__)) || defined(__NuttX__) || defined(_AIX) \
        || (defined(__sun) && defined(__SVR4) /*Solaris*/)
#    include <alloca.h>
#  endif
#  define z_alloca(nmemb) alloca(nmemb)
#endif

/* Print list of supported symbologies */
static void types(void) {
    /* Breaking up strings so don't get too long (i.e. 500 or so) */
    fputs(" # Name        Description               # Name           Description\n"
          " 1 CODE11      Code 11                  75 NVE18          NVE-18\n"
          " 2 C25STANDARD Standard 2 of 5          76 JAPANPOST      Japanese Post\n"
          " 3 C25INTER    Interleaved 2 of 5       77 KOREAPOST      Korea Post\n"
          " 4 C25IATA     IATA 2 of 5              79 DBAR_STK       GS1 DataBar Stacked\n", stdout);
    fputs(" 6 C25LOGIC    Data Logic 2 of 5        80 DBAR_OMNSTK    GS1 DataBar Stack Omni\n"
          " 7 C25IND      Industrial 2 of 5        81 DBAR_EXPSTK    GS1 DataBar Exp Stack\n"
          " 8 CODE39      Code 39                  82 PLANET         USPS PLANET\n"
          " 9 EXCODE39    Extended Code 39         84 MICROPDF417    MicroPDF417\n"
          "13 EANX        EAN-2 to EAN-13          85 USPS_IMAIL     USPS Intelligent Mail\n", stdout);
    fputs("14 EANX_CHK    EAN + Check Digit        86 PLESSEY        UK Plessey\n"
          "16 GS1_128     GS1-128                  87 TELEPEN_NUM    Telepen Numeric\n"
          "18 CODABAR     Codabar                  89 ITF14          ITF-14\n"
          "20 CODE128     Code 128                 90 KIX            Dutch Post KIX Code\n"
          "21 DPLEIT      Deutsche Post Leitcode   92 AZTEC          Aztec Code\n", stdout);
    fputs("22 DPIDENT     Deutsche Post Identcode  93 DAFT           DAFT Code\n"
          "23 CODE16K     Code 16K                 96 DPD            DPD Parcel Code 128\n"
          "24 CODE49      Code 49                  97 MICROQR        Micro QR Code\n"
          "25 CODE93      Code 93                  98 HIBC_128       HIBC Code 128\n"
          "28 FLAT        Flattermarken            99 HIBC_39        HIBC Code 39\n", stdout);
    fputs("29 DBAR_OMN    GS1 DataBar Omni        102 HIBC_DM        HIBC Data Matrix\n"
          "30 DBAR_LTD    GS1 DataBar Limited     104 HIBC_QR        HIBC QR Code\n"
          "31 DBAR_EXP    GS1 DataBar Expanded    106 HIBC_PDF       HIBC PDF417\n"
          "32 TELEPEN     Telepen Alpha           108 HIBC_MICPDF    HIBC MicroPDF417\n"
          "34 UPCA        UPC-A                   110 HIBC_BLOCKF    HIBC Codablock-F\n", stdout);
    fputs("35 UPCA_CHK    UPC-A + Check Digit     112 HIBC_AZTEC     HIBC Aztec Code\n"
          "37 UPCE        UPC-E                   115 DOTCODE        DotCode\n"
          "38 UPCE_CHK    UPC-E + Check Digit     116 HANXIN         Han Xin Code\n"
          "40 POSTNET     USPS POSTNET            119 MAILMARK_2D    Royal Mail 2D Mailmark\n"
          "47 MSI_PLESSEY MSI Plessey             120 UPU_S10        UPU S10\n", stdout);
    fputs("49 FIM         Facing Ident Mark       121 MAILMARK_4S    RM 4-State Mailmark\n"
          "50 LOGMARS     LOGMARS Code 39         128 AZRUNE         Aztec Runes\n"
          "51 PHARMA      Pharmacode One-Track    129 CODE32         Code 32\n"
          "52 PZN         Pharmazentralnummer     130 EANX_CC        Composite EAN\n"
          "53 PHARMA_TWO  Pharmacode Two-Track    131 GS1_128_CC     Composite GS1-128\n", stdout);
    fputs("54 CEPNET      Brazilian CEPNet        132 DBAR_OMN_CC    Comp DataBar Omni\n"
          "55 PDF417      PDF417                  133 DBAR_LTD_CC    Comp DataBar Limited\n"
          "56 PDF417COMP  Compact PDF417          134 DBAR_EXP_CC    Comp DataBar Expanded\n"
          "57 MAXICODE    MaxiCode                135 UPCA_CC        Composite UPC-A\n"
          "58 QRCODE      QR Code                 136 UPCE_CC        Composite UPC-E\n", stdout);
    fputs("60 CODE128AB   Code 128 (Suppress C)   137 DBAR_STK_CC    Comp DataBar Stacked\n"
          "63 AUSPOST     AP Standard Customer    138 DBAR_OMNSTK_CC Comp DataBar Stack Omn\n"
          "66 AUSREPLY    AP Reply Paid           139 DBAR_EXPSTK_CC Comp DataBar Exp Stack\n"
          "67 AUSROUTE    AP Routing              140 CHANNEL        Channel Code\n"
          "68 AUSREDIRECT AP Redirection          141 CODEONE        Code One\n", stdout);
    fputs("69 ISBNX       ISBN                    142 GRIDMATRIX     Grid Matrix\n"
          "70 RM4SCC      Royal Mail 4SCC         143 UPNQR          UPN QR Code\n"
          "71 DATAMATRIX  Data Matrix             144 ULTRA          Ultracode\n"
          "72 EAN14       EAN-14                  145 RMQR           Rectangular Micro QR\n"
          "73 VIN         Vehicle Information No. 146 BC412          BC412\n", stdout);
    fputs("74 CODABLOCKF  Codablock-F             147 DXFILMEDGE     DX Film Edge Barcode\n", stdout);
}

/* Output version information */
static void version(const int no_png) {
    const char *no_png_lib = no_png ? " (no libpng)" : "";
    const int zint_version = ZBarcode_Version();
    const int version_major = zint_version / 10000;
    const int version_minor = (zint_version % 10000) / 100;
    int version_release = zint_version % 100;
    int version_build;

    if (version_release >= 9) {
        /* This is a test release */
        version_release = version_release / 10;
        version_build = zint_version % 10;
        printf("Zint version %d.%d.%d.%d (dev)%s\n", version_major, version_minor, version_release, version_build,
                no_png_lib);
    } else {
        /* This is a stable release */
        printf("Zint version %d.%d.%d%s\n", version_major, version_minor, version_release, no_png_lib);
    }
}

/* Output usage information */
static void usage(const int no_png) {
    const char *no_png_type = no_png ? "" : "/PNG";
    const char *no_png_ext = no_png ? "gif" : "png";

    version(no_png);

    /* Breaking up strings so don't get too long (i.e. 500 or so) */
    printf("Encode input data in a barcode and save as BMP/EMF/EPS/GIF/PCX%s/SVG/TIF/TXT\n\n", no_png_type);
    fputs( "  -b, --barcode=TYPE    Number or name of barcode type. Default is 20 (CODE128)\n"
           "  --addongap=INTEGER    Set add-on gap in multiples of X-dimension for EAN/UPC\n"
           "  --batch               Treat each line of input file as a separate data set\n"
           "  --bg=COLOUR           Specify a background colour (as RGB(A) or \"C,M,Y,K\")\n"
           "  --binary              Treat input as raw binary data\n", stdout);
    fputs( "  --bind                Add boundary bars\n"
           "  --bindtop             Add top boundary bar only\n"
           "  --bold                Use bold text (HRT)\n"
           "  --border=INTEGER      Set width of border in multiples of X-dimension\n"
           "  --box                 Add a box around the symbol\n", stdout);
    fputs( "  --cmyk                Use CMYK colour space in EPS/TIF symbols\n"
           "  --cols=INTEGER        Set the number of data columns in symbol\n"
           "  --compliantheight     Warn if height not compliant, and use standard default\n"
           "  -d, --data=DATA       Set the symbol data content (segment 0)\n"
           "  --direct              Send output to stdout\n", stdout);
    fputs( "  --dmiso144            Use ISO format for 144x144 Data Matrix symbols\n"
           "  --dmre                Allow Data Matrix Rectangular Extended\n"
           "  --dotsize=NUMBER      Set radius of dots in dotty mode\n"
           "  --dotty               Use dots instead of squares for matrix symbols\n"
           "  --dump                Dump hexadecimal representation to stdout\n", stdout);
    fputs( "  -e, --ecinos          Display ECI (Extended Channel Interpretation) table\n"
           "  --eci=INTEGER         Set the ECI code for the data (segment 0)\n"
           "  --embedfont           Embed font in vector output (SVG only)\n"
           "  --esc                 Process escape sequences in input data\n"
           "  --extraesc            Process symbology-specific escape sequences (Code 128)\n", stdout);
    fputs( "  --fast                Use faster encodation or other shortcuts if available\n"
           "  --fg=COLOUR           Specify a foreground colour (as RGB(A) or \"C,M,Y,K\")\n", stdout);
    printf("  --filetype=TYPE       Set output file type BMP/EMF/EPS/GIF/PCX%s/SVG/TIF/TXT\n", no_png_type);
    fputs( "  --fullmultibyte       Use multibyte for binary/Latin (QR/Han Xin/Grid Matrix)\n"
           "  --gs1                 Treat input as GS1 compatible data\n"
           "  --gs1nocheck          Do not check validity of GS1 data\n"
           "  --gs1parens           Process parentheses \"()\" as GS1 AI delimiters, not \"[]\"\n"
           "  --gssep               Use separator GS for GS1 (Data Matrix)\n", stdout);
    fputs( "  --guarddescent=NUMBER Set height of guard bar descent in X-dims (EAN/UPC)\n"
           "  --guardwhitespace     Add quiet zone indicators (\"<\"/\">\") to HRT (EAN/UPC)\n"
           "  -h, --help            Display help message\n"
           "  --height=NUMBER       Set height of symbol in multiples of X-dimension\n"
           "  --heightperrow        Treat height as per-row\n", stdout);
    fputs( "  -i, --input=FILE      Read input data from FILE\n"
           "  --init                Create Reader Initialisation (Programming) symbol\n"
           "  --mask=INTEGER        Set masking pattern to use (QR/Han Xin/DotCode)\n"
           "  --mirror              Use batch data to determine filename\n"
           "  --mode=INTEGER        Set encoding mode (MaxiCode/Composite)\n", stdout);
    printf("  --nobackground        Remove background (EMF/EPS/GIF%s/SVG/TIF only)\n", no_png_type);
    fputs( "  --noquietzones        Disable default quiet zones\n"
           "  --notext              Remove human readable text (HRT)\n", stdout);
    printf("  -o, --output=FILE     Send output to FILE. Default is out.%s\n", no_png_ext);
    fputs( "  --primary=STRING      Set primary message (MaxiCode/Composite)\n"
           "  --quietzones          Add compliant quiet zones\n"
           "  -r, --reverse         Reverse colours (white on black)\n"
           "  --rotate=INTEGER      Rotate symbol by INTEGER (0, 90, 180, 270) degrees\n"
           "  --rows=INTEGER        Set number of rows (Codablock-F/PDF417)\n", stdout);
    fputs( "  --scale=NUMBER        Adjust size of X-dimension\n"
           "  --scalexdimdp=X[,R]   Adjust size to X-dimension X at resolution R\n"
           "  --scmvv=INTEGER       Prefix SCM with \"[)>\\R01\\Gvv\" (vv is INTEGER) (MaxiCode)\n"
           "  --secure=INTEGER      Set error correction level (ECC)\n"
           "  --segN=ECI,DATA       Set the ECI & data content for segment N, where N 1 to 9\n", stdout);
    fputs( "  --separator=INTEGER   Set height of row separator bars (stacked symbologies)\n"
           "  --small               Use small text (HRT)\n"
           "  --square              Force Data Matrix symbols to be square\n"
           "  --structapp=I,C[,ID]  Set Structured Append info (I index, C count)\n"
           "  -t, --types           Display table of barcode types\n", stdout);
    fputs( "  --textgap=NUMBER      Adjust gap between barcode and HRT in multiples of X-dim\n"
           "  --vers=INTEGER        Set symbol version (size, check digits, other options)\n"
           "  -v, --version         Display Zint version\n"
           "  --vwhitesp=INTEGER    Set height of vertical whitespace in multiples of X-dim\n"
           "  -w, --whitesp=INTEGER Set width of horizontal whitespace in multiples of X-dim\n", stdout);
    fputs( "  --werror              Convert all warnings into errors\n", stdout);
}

/* Display supported ECI codes */
static void show_eci(void) {
    /* Breaking up strings so don't get too long (i.e. 500 or so) */
    fputs("  3: ISO/IEC 8859-1 - Latin alphabet No. 1 (default)\n"
          "  4: ISO/IEC 8859-2 - Latin alphabet No. 2\n"
          "  5: ISO/IEC 8859-3 - Latin alphabet No. 3\n"
          "  6: ISO/IEC 8859-4 - Latin alphabet No. 4\n"
          "  7: ISO/IEC 8859-5 - Latin/Cyrillic alphabet\n", stdout);
    fputs("  8: ISO/IEC 8859-6 - Latin/Arabic alphabet\n"
          "  9: ISO/IEC 8859-7 - Latin/Greek alphabet\n"
          " 10: ISO/IEC 8859-8 - Latin/Hebrew alphabet\n"
          " 11: ISO/IEC 8859-9 - Latin alphabet No. 5 (Turkish)\n"
          " 12: ISO/IEC 8859-10 - Latin alphabet No. 6 (Nordic)\n", stdout);
    fputs(" 13: ISO/IEC 8859-11 - Latin/Thai alphabet\n"
          " 15: ISO/IEC 8859-13 - Latin alphabet No. 7 (Baltic)\n"
          " 16: ISO/IEC 8859-14 - Latin alphabet No. 8 (Celtic)\n"
          " 17: ISO/IEC 8859-15 - Latin alphabet No. 9\n"
          " 18: ISO/IEC 8859-16 - Latin alphabet No. 10\n", stdout);
    fputs(" 20: Shift JIS (JIS X 0208 and JIS X 0201)\n"
          " 21: Windows 1250 - Latin 2 (Central Europe)\n"
          " 22: Windows 1251 - Cyrillic\n"
          " 23: Windows 1252 - Latin 1\n"
          " 24: Windows 1256 - Arabic\n", stdout);
    fputs(" 25: UTF-16BE (High order byte first)\n"
          " 26: UTF-8\n"
          " 27: ASCII (ISO/IEC 646 IRV)\n"
          " 28: Big5 (Taiwan) Chinese Character Set\n"
          " 29: GB 2312 (PRC) Chinese Character Set\n", stdout);
    fputs(" 30: Korean Character Set EUC-KR (KS X 1001:2002)\n"
          " 31: GBK Chinese Character Set\n"
          " 32: GB 18030 Chinese Character Set\n"
          " 33: UTF-16LE (Low order byte first)\n"
          " 34: UTF-32BE (High order bytes first)\n", stdout);
    fputs(" 35: UTF-32LE (Low order bytes first)\n"
          "170: ISO/IEC 646 Invariant (ASCII subset)\n"
          "899: 8-bit binary data\n", stdout);
}

/* Verifies that a string (length <= 9) only uses digits. On success returns value in arg */
static int validate_int(const char source[], int len, int *p_val) {
    int val = 0;
    int i;
    const int length = len == -1 ? (int) strlen(source) : len;

    if (length > 9) { /* Prevent overflow */
        return 0;
    }
    for (i = 0; i < length; i++) {
        if (source[i] < '0' || source[i] > '9') {
            return 0;
        }
        val *= 10;
        val += source[i] - '0';
    }
    *p_val = val;

    return 1;
}

/* Verifies that a string is a simplified form of floating point, max 7 significant decimal digits with
   optional decimal point. On success returns val in arg. On failure sets `errbuf` */
static int validate_float(const char source[], const int allow_neg, float *p_val, char errbuf[64]) {
    static const float fract_muls[7] = { 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f, 0.0000001f };
    int val = 0;
    int neg = 0;
    const char *dot = strchr(source, '.');
    int int_len;

    if (*source == '+' || *source == '-') {
        if (*source == '-') {
            if (!allow_neg) {
                strcpy(errbuf, "negative value not permitted");
                return 0;
            }
            neg = 1;
        }
        source++;
    }

    int_len = dot ? (int) (dot - source) : (int) strlen(source);
    if (int_len > 9) {
        strcpy(errbuf, "integer part must be 7 digits maximum"); /* Say 7 not 9 to "manage expections" */
        return 0;
    }
    if (int_len) {
        int tmp_val;
        if (!validate_int(source, int_len, &val)) {
            strcpy(errbuf, "integer part must be digits only");
            return 0;
        }
        for (int_len = 0, tmp_val = val; tmp_val; tmp_val /= 10, int_len++); /* log10(val) */
        if (int_len > 7) {
            strcpy(errbuf, "integer part must be 7 digits maximum");
            return 0;
        }
    }
    if (dot && *++dot) {
        int val2, fract_len;
        const char *e;
        for (e = dot + strlen(dot) - 1; e > dot && *e == '0'; e--); /* Ignore trailing zeroes */
        fract_len = (int) (e + 1 - dot);
        if (fract_len) {
            if (fract_len > 7) {
                strcpy(errbuf, "fractional part must be 7 digits maximum");
                return 0;
            }
            if (!validate_int(dot, fract_len, &val2)) {
                strcpy(errbuf, "fractional part must be digits only");
                return 0;
            }
            if (val2 && int_len + fract_len > 7) {
                if (val) {
                    strcpy(errbuf, "7 significant digits maximum");
                } else {
                    strcpy(errbuf, "fractional part must be 7 digits maximum");
                }
                return 0;
            }
            *p_val = val + val2 * fract_muls[fract_len - 1];
        } else {
            *p_val = (float) val;
        }
    } else {
        *p_val = (float) val;
    }
    if (neg) {
        *p_val = -*p_val;
    }
    return 1;
}

/* Converts upper case characters to lower case in a string source[] */
static void to_lower(char source[]) {
    int i;
    const int src_len = (int) strlen(source);

    for (i = 0; i < src_len; i++) {
        if ((source[i] >= 'A') && (source[i] <= 'Z')) {
            source[i] |= 0x20;
        }
    }
}

/* Return symbology id if `barcode_name` a barcode name */
static int get_barcode_name(const char *barcode_name) {
    /* Must be sorted for binary search to work */
    static const struct { int symbology; const char *n; } names[] = {
        { BARCODE_C25LOGIC, "2of5datalogic" }, /* Synonym */
        { BARCODE_C25IATA, "2of5iata" }, /* Synonym */
        { BARCODE_C25IND, "2of5ind" }, /* Synonym */
        { BARCODE_C25IND, "2of5industrial" }, /* Synonym */
        { BARCODE_C25INTER, "2of5inter" }, /* Synonym */
        { BARCODE_C25INTER, "2of5interleaved" }, /* Synonym */
        { BARCODE_C25LOGIC, "2of5logic" }, /* Synonym */
        { BARCODE_C25STANDARD, "2of5matrix" }, /* Synonym */
        { BARCODE_C25STANDARD, "2of5standard" }, /* Synonym */
        { BARCODE_AUSPOST, "auspost" },
        { BARCODE_AUSREDIRECT, "ausredirect" },
        { BARCODE_AUSREPLY, "ausreply" },
        { BARCODE_AUSROUTE, "ausroute" },
        { BARCODE_AZRUNE, "azrune" },
        { BARCODE_AZTEC, "aztec" },
        { BARCODE_AZTEC, "azteccode" }, /* Synonym */
        { BARCODE_AZRUNE, "aztecrune" }, /* Synonym */
        { BARCODE_AZRUNE, "aztecrunes" }, /* Synonym */
        { BARCODE_BC412, "bc412" },
        { BARCODE_C25LOGIC, "c25datalogic" }, /* Synonym */
        { BARCODE_C25IATA, "c25iata" },
        { BARCODE_C25IND, "c25ind" },
        { BARCODE_C25IND, "c25industrial" }, /* Synonym */
        { BARCODE_C25INTER, "c25inter" },
        { BARCODE_C25INTER, "c25interleaved" }, /* Synonym */
        { BARCODE_C25LOGIC, "c25logic" },
        { BARCODE_C25STANDARD, "c25matrix" },
        { BARCODE_C25STANDARD, "c25standard" },
        { BARCODE_CEPNET, "cepnet" },
        { BARCODE_CHANNEL, "channel" },
        { BARCODE_CHANNEL, "channelcode" }, /* Synonym */
        { BARCODE_CODABAR, "codabar" },
        { BARCODE_CODABLOCKF, "codablockf" },
        { BARCODE_CODE11, "code11" },
        { BARCODE_CODE128, "code128" },
        { BARCODE_CODE128AB, "code128ab" },
        { BARCODE_CODE128AB, "code128b" }, /* Synonym */
        { BARCODE_CODE16K, "code16k" },
        { BARCODE_C25LOGIC, "code2of5datalogic" }, /* Synonym */
        { BARCODE_C25IATA, "code2of5iata" }, /* Synonym */
        { BARCODE_C25IND, "code2of5ind" }, /* Synonym */
        { BARCODE_C25IND, "code2of5industrial" }, /* Synonym */
        { BARCODE_C25INTER, "code2of5inter" }, /* Synonym */
        { BARCODE_C25INTER, "code2of5interleaved" }, /* Synonym */
        { BARCODE_C25LOGIC, "code2of5logic" }, /* Synonym */
        { BARCODE_C25STANDARD, "code2of5matrix" }, /* Synonym */
        { BARCODE_C25STANDARD, "code2of5standard" }, /* Synonym */
        { BARCODE_CODE32, "code32" },
        { BARCODE_CODE39, "code39" },
        { BARCODE_CODE49, "code49" },
        { BARCODE_CODE93, "code93" },
        { BARCODE_CODEONE, "codeone" },
        { BARCODE_DAFT, "daft" },
        { BARCODE_DBAR_EXP, "databarexp" }, /* Synonym */
        { BARCODE_DBAR_EXP, "databarexpanded" }, /* Synonym */
        { BARCODE_DBAR_EXP_CC, "databarexpandedcc" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK, "databarexpandedstacked" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK_CC, "databarexpandedstackedcc" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK, "databarexpandedstk" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK_CC, "databarexpandedstkcc" }, /* Synonym */
        { BARCODE_DBAR_EXP_CC, "databarexpcc" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK, "databarexpstk" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK_CC, "databarexpstkcc" }, /* Synonym */
        { BARCODE_DBAR_LTD, "databarlimited" }, /* Synonym */
        { BARCODE_DBAR_LTD_CC, "databarlimitedcc" }, /* Synonym */
        { BARCODE_DBAR_LTD, "databarltd" }, /* Synonym */
        { BARCODE_DBAR_LTD_CC, "databarltdcc" }, /* Synonym */
        { BARCODE_DBAR_OMN, "databaromn" }, /* Synonym */
        { BARCODE_DBAR_OMN_CC, "databaromncc" }, /* Synonym */
        { BARCODE_DBAR_OMN, "databaromni" }, /* Synonym */
        { BARCODE_DBAR_OMN_CC, "databaromnicc" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK, "databaromnstk" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK_CC, "databaromnstkcc" }, /* Synonym */
        { BARCODE_DBAR_STK, "databarstacked" }, /* Synonym */
        { BARCODE_DBAR_STK_CC, "databarstackedcc" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK, "databarstackedomn" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK_CC, "databarstackedomncc" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK, "databarstackedomni" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK_CC, "databarstackedomnicc" }, /* Synonym */
        { BARCODE_DBAR_STK, "databarstk" }, /* Synonym */
        { BARCODE_DBAR_STK_CC, "databarstkcc" }, /* Synonym */
        { BARCODE_DATAMATRIX, "datamatrix" },
        { BARCODE_DBAR_EXP, "dbarexp" },
        { BARCODE_DBAR_EXP, "dbarexpanded" }, /* Synonym */
        { BARCODE_DBAR_EXP_CC, "dbarexpandedcc" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK, "dbarexpandedstacked" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK_CC, "dbarexpandedstackedcc" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK, "dbarexpandedstk" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK_CC, "dbarexpandedstkcc" }, /* Synonym */
        { BARCODE_DBAR_EXP_CC, "dbarexpcc" },
        { BARCODE_DBAR_EXPSTK, "dbarexpstk" },
        { BARCODE_DBAR_EXPSTK_CC, "dbarexpstkcc" },
        { BARCODE_DBAR_LTD, "dbarlimited" }, /* Synonym */
        { BARCODE_DBAR_LTD_CC, "dbarlimitedcc" }, /* Synonym */
        { BARCODE_DBAR_LTD, "dbarltd" },
        { BARCODE_DBAR_LTD_CC, "dbarltdcc" },
        { BARCODE_DBAR_OMN, "dbaromn" },
        { BARCODE_DBAR_OMN_CC, "dbaromncc" },
        { BARCODE_DBAR_OMN, "dbaromni" }, /* Synonym */
        { BARCODE_DBAR_OMN_CC, "dbaromnicc" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK, "dbaromnstk" },
        { BARCODE_DBAR_OMNSTK_CC, "dbaromnstkcc" },
        { BARCODE_DBAR_STK, "dbarstacked" }, /* Synonym */
        { BARCODE_DBAR_STK_CC, "dbarstackedcc" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK, "dbarstackedomn" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK_CC, "dbarstackedomncc" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK, "dbarstackedomni" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK_CC, "dbarstackedomnicc" }, /* Synonym */
        { BARCODE_DBAR_STK, "dbarstk" },
        { BARCODE_DBAR_STK_CC, "dbarstkcc" },
        { BARCODE_DOTCODE, "dotcode" },
        { BARCODE_DPD, "dpd" },
        { BARCODE_DPIDENT, "dpident" },
        { BARCODE_DPLEIT, "dpleit" },
        { BARCODE_DXFILMEDGE, "dxfilmedge" },
        { BARCODE_EANX, "ean" }, /* Synonym */
        { BARCODE_GS1_128, "ean128" }, /* Synonym */
        { BARCODE_GS1_128_CC, "ean128cc" }, /* Synonym */
        { BARCODE_EAN14, "ean14" },
        { BARCODE_EANX_CC, "eancc" }, /* Synonym */
        { BARCODE_EANX_CHK, "eanchk" }, /* Synonym */
        { BARCODE_EANX, "eanx" },
        { BARCODE_EANX_CC, "eanxcc" },
        { BARCODE_EANX_CHK, "eanxchk" },
        { BARCODE_EXCODE39, "excode39" },
        { BARCODE_EXCODE39, "extendedcode39" }, /* Synonym */
        { BARCODE_FIM, "fim" },
        { BARCODE_FLAT, "flat" },
        { BARCODE_GRIDMATRIX, "gridmatrix" },
        { BARCODE_GS1_128, "gs1128" },
        { BARCODE_GS1_128_CC, "gs1128cc" },
        { BARCODE_HANXIN, "hanxin" },
        { BARCODE_HIBC_128, "hibc128" },
        { BARCODE_HIBC_39, "hibc39" },
        { BARCODE_HIBC_AZTEC, "hibcaztec" },
        { BARCODE_HIBC_BLOCKF, "hibcblockf" },
        { BARCODE_HIBC_BLOCKF, "hibccodablockf" }, /* Synonym */
        { BARCODE_HIBC_128, "hibccode128" }, /* Synonym */
        { BARCODE_HIBC_39, "hibccode39" }, /* Synonym */
        { BARCODE_HIBC_DM, "hibcdatamatrix" }, /* Synonym */
        { BARCODE_HIBC_DM, "hibcdm" },
        { BARCODE_HIBC_MICPDF, "hibcmicpdf" },
        { BARCODE_HIBC_MICPDF, "hibcmicropdf" }, /* Synonym */
        { BARCODE_HIBC_MICPDF, "hibcmicropdf417" }, /* Synonym */
        { BARCODE_HIBC_PDF, "hibcpdf" },
        { BARCODE_HIBC_PDF, "hibcpdf417" }, /* Synonym */
        { BARCODE_HIBC_QR, "hibcqr" },
        { BARCODE_HIBC_QR, "hibcqrcode" }, /* Synonym */
        { BARCODE_C25IATA, "iata2of5" }, /* Synonym */
        { BARCODE_C25IATA, "iatacode2of5" }, /* Synonym */
        { BARCODE_C25IND, "industrial2of5" }, /* Synonym */
        { BARCODE_C25IND, "industrialcode2of5" }, /* Synonym */
        { BARCODE_C25INTER, "interleaved2of5" }, /* Synonym */
        { BARCODE_C25INTER, "interleavedcode2of5" }, /* Synonym */
        { BARCODE_ISBNX, "isbn" }, /* Synonym */
        { BARCODE_ISBNX, "isbnx" },
        { BARCODE_ITF14, "itf14" },
        { BARCODE_JAPANPOST, "japanpost" },
        { BARCODE_KIX, "kix" },
        { BARCODE_KOREAPOST, "koreapost" },
        { BARCODE_LOGMARS, "logmars" },
        { BARCODE_MAILMARK_4S, "mailmark" }, /* Synonym */
        { BARCODE_MAILMARK_2D, "mailmark2d" },
        { BARCODE_MAILMARK_4S, "mailmark4s" },
        { BARCODE_MAILMARK_4S, "mailmark4state" }, /* Synonym */
        { BARCODE_MAXICODE, "maxicode" },
        { BARCODE_MICROPDF417, "micropdf417" },
        { BARCODE_MICROQR, "microqr" },
        { BARCODE_MICROQR, "microqrcode" }, /* Synonym */
        { BARCODE_MSI_PLESSEY, "msi" }, /* Synonym */
        { BARCODE_MSI_PLESSEY, "msiplessey" },
        { BARCODE_NVE18, "nve18" },
        { BARCODE_USPS_IMAIL, "onecode" }, /* Synonym */
        { BARCODE_PDF417, "pdf417" },
        { BARCODE_PDF417COMP, "pdf417comp" },
        { BARCODE_PDF417COMP, "pdf417trunc" }, /* Synonym */
        { BARCODE_PHARMA, "pharma" },
        { BARCODE_PHARMA_TWO, "pharmatwo" },
        { BARCODE_PLANET, "planet" },
        { BARCODE_PLESSEY, "plessey" },
        { BARCODE_POSTNET, "postnet" },
        { BARCODE_PZN, "pzn" },
        { BARCODE_QRCODE, "qr" }, /* Synonym */
        { BARCODE_QRCODE, "qrcode" },
        { BARCODE_RM4SCC, "rm4scc" },
        { BARCODE_RMQR, "rmqr" },
        { BARCODE_DBAR_OMN, "rss14" }, /* Synonym */
        { BARCODE_DBAR_OMN_CC, "rss14cc" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK_CC, "rss14omnicc" }, /* Synonym */
        { BARCODE_DBAR_STK, "rss14stack" }, /* Synonym */
        { BARCODE_DBAR_STK_CC, "rss14stackcc" }, /* Synonym */
        { BARCODE_DBAR_OMNSTK, "rss14stackomni" }, /* Synonym */
        { BARCODE_DBAR_EXP, "rssexp" }, /* Synonym */
        { BARCODE_DBAR_EXP_CC, "rssexpcc" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK, "rssexpstack" }, /* Synonym */
        { BARCODE_DBAR_EXPSTK_CC, "rssexpstackcc" }, /* Synonym */
        { BARCODE_DBAR_LTD, "rssltd" }, /* Synonym */
        { BARCODE_DBAR_LTD_CC, "rssltdcc" }, /* Synonym */
        { BARCODE_C25STANDARD, "standardcode2of5" }, /* Synonym */
        { BARCODE_TELEPEN, "telepen" },
        { BARCODE_TELEPEN_NUM, "telepennum" },
        { BARCODE_ULTRA, "ultra" },
        { BARCODE_ULTRA, "ultracode" }, /* Synonym */
        { BARCODE_UPCA, "upca" },
        { BARCODE_UPCA_CC, "upcacc" },
        { BARCODE_UPCA_CHK, "upcachk" },
        { BARCODE_UPCE, "upce" },
        { BARCODE_UPCE_CC, "upcecc" },
        { BARCODE_UPCE_CHK, "upcechk" },
        { BARCODE_UPNQR, "upnqr" },
        { BARCODE_UPNQR, "upnqrcode" }, /* Synonym */
        { BARCODE_UPU_S10, "upus10" },
        { BARCODE_USPS_IMAIL, "uspsimail" },
        { BARCODE_VIN, "vin" },
    };
    int s = 0, e = ARRAY_SIZE(names) - 1;

    char n[30] = {0};
    int i, j, length;

    /* Ignore case and any "BARCODE" prefix */
    strncpy(n, barcode_name, 29);
    to_lower(n);
    length = (int) strlen(n);
    if (strncmp(n, "barcode", 7) == 0) {
        memmove(n, n + 7, length - 7 + 1); /* Include NUL char */
        length = (int) strlen(n);
    }

    /* Ignore any non-alphanumeric characters */
    for (i = 0, j = 0; i < length; i++) {
        if ((n[i] >= 'a' && n[i] <= 'z') || (n[i] >= '0' && n[i] <= '9')) {
            n[j++] = n[i];
        }
    }
    if (j == 0) {
        return 0;
    }
    n[j] = '\0';

    while (s <= e) {
        const int m = (s + e) / 2;
        const int cmp = strcmp(names[m].n, n);
        if (cmp < 0) {
            s = m + 1;
        } else if (cmp > 0) {
            e = m - 1;
        } else {
            return names[m].symbology;
        }
    }

    return 0;
}

/* Whether `filetype` supported by Zint. Sets `png_refused` if `no_png` and PNG requested */
static int supported_filetype(const char *filetype, const int no_png, int *png_refused) {
    static const char filetypes[][4] = {
        "bmp", "emf", "eps", "gif", "pcx", "png", "svg", "tif", "txt",
    };
    char lc_filetype[4] = {0};
    int i;

    if (png_refused) {
        *png_refused = 0;
    }
    strncpy(lc_filetype, filetype, 3);
    to_lower(lc_filetype);

    if (no_png && strcmp(lc_filetype, "png") == 0) {
        if (png_refused) {
            *png_refused = 1;
        }
        return 0;
    }

    for (i = 0; i < ARRAY_SIZE(filetypes); i++) {
        if (strcmp(lc_filetype, filetypes[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Get file extension, excluding those of 4 or more letters */
static char *get_extension(const char *file) {
    char *dot;

    dot = strrchr(file, '.');
    if (dot && strlen(file) - (dot - file) <= 4) { /* Only recognize up to 3 letter extensions */
        return dot + 1;
    }
    return NULL;
}

/* Set extension of `file` to `filetype`, replacing existing extension if any.
 * Does nothing if file already has `filetype` extension */
static void set_extension(char *file, const char *filetype) {
    char lc_filetype[4] = {0};
    char *extension;
    char lc_extension[4];

    strncpy(lc_filetype, filetype, 3);
    to_lower(lc_filetype);

    extension = get_extension(file);
    if (extension) {
        strcpy(lc_extension, extension);
        to_lower(lc_extension);
        if (strcmp(lc_filetype, lc_extension) == 0) {
            return;
        }
        *(extension - 1) = '\0'; /* Cut off at dot */
    }
    if (strlen(file) > 251) {
        file[251] = '\0';
    }
    strcat(file, ".");
    strncat(file, filetype, 3);
}

/* Whether `filetype` is raster type */
static int is_raster(const char *filetype, const int no_png) {
    static const char raster_filetypes[][4] = {
        "bmp", "gif", "pcx", "png", "tif",
    };
    int i;
    char lc_filetype[4] = {0};

    if (filetype == NULL) {
        return 0;
    }
    strcpy(lc_filetype, filetype);
    to_lower(lc_filetype);

    if (no_png && strcmp(lc_filetype, "png") == 0) {
        return 0;
    }

    for (i = 0; i < ARRAY_SIZE(raster_filetypes); i++) {
        if (strcmp(lc_filetype, raster_filetypes[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Helper for `validate_scalexdimdp()` to search for units, returning -2 on error, -1 if not found, else index */
static int validate_units(char *buf, const char units[][5], int units_size) {
    int i;
    char *unit;

    to_lower(buf);
    for (i = 0; i < units_size; i++) {
        if ((unit = strstr(buf, units[i])) != NULL) {
            if (strlen(units[i]) != strlen(unit)) {
                return -2;
            }
            *unit = '\0';
            break;
        }
    }
    if (i == units_size) {
        i = -1;
    }
    return i;
}

/* Parse and validate argument "xdim[,resolution]" to "--scalexdimdp" */
static int validate_scalexdimdp(const char *arg, float *p_x_dim_mm, float *p_dpmm) {
    static const char x_units[][5] = { "mm", "in" };
    static const char r_units[][5] = { "dpmm", "dpi" };
    char x_buf[7 + 1 + 4 + 1] = {0}; /* Allow for 7 digits + dot + 4-char unit + NUL */
    char r_buf[7 + 1 + 4 + 1] = {0}; /* As above */
    int units_i; /* For `validate_units()` */
    char errbuf[64]; /* For `validate_float()` */
    const char *comma = strchr(arg, ',');
    if (comma) {
        if (comma == arg || comma - arg >= ARRAY_SIZE(x_buf)) {
            fprintf(stderr, "Error 174: scalexdimdp X-dim too %s\n", comma == arg ? "short" : "long");
            return 0;
        }
        strncpy(x_buf, arg, comma - arg);
        comma++;
        if (!*comma || strlen(comma) >= ARRAY_SIZE(r_buf)) {
            fprintf(stderr, "Error 175: scalexdimdp resolution too %s\n", !*comma ? "short" : "long");
            return 0;
        }
        strcpy(r_buf, comma);
    } else {
        if (!*arg || strlen(arg) >= ARRAY_SIZE(x_buf)) {
            fprintf(stderr, "Error 176: scalexdimdp X-dim too %s\n", !*arg ? "short" : "long");
            return 0;
        }
        strcpy(x_buf, arg);
    }
    if ((units_i = validate_units(x_buf, x_units, ARRAY_SIZE(x_units))) == -2) {
        fprintf(stderr, "Error 177: scalexdimdp X-dim units must occur at end\n");
        return 0;
    }
    if (!validate_float(x_buf, 0 /*allow_neg*/, p_x_dim_mm, errbuf)) {
        fprintf(stderr, "Error 178: scalexdimdp X-dim invalid floating point (%s)\n", errbuf);
        return 0;
    }
    if (units_i > 0) { /* Ignore mm */
        *p_x_dim_mm /= 25.4f /*in*/;
    }
    *p_dpmm = 0.0f;
    if (comma) {
        if ((units_i = validate_units(r_buf, r_units, ARRAY_SIZE(r_units))) == -2) {
            fprintf(stderr, "Error 179: scalexdimdp resolution units must occur at end\n");
            return 0;
        }
        if (!validate_float(r_buf, 0 /*allow_neg*/, p_dpmm, errbuf)) {
            fprintf(stderr, "Error 180: scalexdimdp resolution invalid floating point (%s)\n", errbuf);
            return 0;
        }
        if (units_i > 0) { /* Ignore dpmm */
            *p_dpmm /= 25.4f /*dpi*/;
        }
    }
    if (*p_dpmm == 0.0f) {
        *p_dpmm = 12.0f; /* 300 dpi */
    }

    return 1;
}

/* Parse and validate Structured Append argument "index,count[,ID]" to "--structapp" */
static int validate_structapp(const char *arg, struct zint_structapp *structapp) {
    char index[10] = {0}, count[10] = {0};
    const char *comma = strchr(arg, ',');
    const char *comma2;
    if (!comma) {
        fprintf(stderr, "Error 155: Invalid Structured Append argument, expect \"index,count[,ID]\"\n");
        return 0;
    }
    if (comma == arg || comma - arg > 9) {
        fprintf(stderr, "Error 156: Structured Append index too %s\n", comma == arg ? "short" : "long");
        return 0;
    }
    strncpy(index, arg, comma - arg);
    comma++;
    comma2 = strchr(comma, ',');
    if (comma2) {
        if (comma2 == comma || comma2 - comma > 9) {
            fprintf(stderr, "Error 157: Structured Append count too %s\n", comma2 == comma ? "short" : "long");
            return 0;
        }
        strncpy(count, comma, comma2 - comma);
        comma2++;
        if (!*comma2 || strlen(comma2) > 32) {
            fprintf(stderr, "Error 158: Structured Append ID too %s\n", !*comma2 ? "short" : "long");
            return 0;
        }
        strncpy(structapp->id, comma2, 32);
    } else {
        if (!*comma || strlen(comma) > 9) {
            fprintf(stderr, "Error 159: Structured Append count too %s\n", !*comma ? "short" : "long");
            return 0;
        }
        strcpy(count, comma);
    }
    if (!validate_int(index, -1 /*len*/, &structapp->index)) {
        fprintf(stderr, "Error 160: Invalid Structured Append index (digits only)\n");
        return 0;
    }
    if (!validate_int(count, -1 /*len*/, &structapp->count)) {
        fprintf(stderr, "Error 161: Invalid Structured Append count (digits only)\n");
        return 0;
    }
    if (structapp->count < 2) {
        fprintf(stderr, "Error 162: Invalid Structured Append count '%d', must be greater than or equal to 2\n",
                structapp->count);
        return 0;
    }
    if (structapp->index < 1 || structapp->index > structapp->count) {
        fprintf(stderr, "Error 163: Structured Append index '%d' out of range (1 to count '%d')\n", structapp->index,
                structapp->count);
        return 0;
    }

    return 1;
}

/* Parse and validate the segment argument "ECI,DATA" to "--segN" */
static int validate_seg(const char *arg, const int N, struct zint_seg segs[10]) {
    char eci[10] = {0};
    const char *comma = strchr(arg, ',');
    if (!comma || comma == arg || comma - arg > 9 || *(comma + 1) == '\0') {
        fprintf(stderr, "Error 166: Invalid segment argument, expect \"ECI,DATA\"\n");
        return 0;
    }
    strncpy(eci, arg, comma - arg);
    if (!validate_int(eci, -1 /*len*/, &segs[N].eci)) {
        fprintf(stderr, "Error 167: Invalid segment ECI (digits only)\n");
        return 0;
    }
    if (segs[N].eci > 999999) {
        fprintf(stderr, "Error 168: Segment ECI code '%d' out of range (0 to 999999)\n", segs[N].eci);
        return 0;
    }
    segs[N].length = (int) strlen(comma + 1);
    segs[N].source = (unsigned char *) (comma + 1);
    return 1;
}

#ifdef _WIN32
static FILE *win_fopen(const char *filename, const char *mode); /* Forward ref */
#endif

/* Batch mode - output symbol for each line of text in `filename` */
static int batch_process(struct zint_symbol *symbol, const char *filename, const int mirror_mode,
            const char *filetype, const int output_given, const int rotate_angle) {
    FILE *file;
    unsigned char buffer[ZINT_MAX_DATA_LEN] = {0}; /* Maximum HanXin input */
    unsigned char character = 0;
    int buf_posn = 0, error_number = 0, warn_number = 0, line_count = 1;
    char output_file[256];
    char number[12], reverse_number[12];
    int inpos, local_line_count;
    char format_string[256], reversed_string[256], format_char;
    int format_len, i, o, mirror_start_o = 0;
    char adjusted[2] = {0};
    const int from_stdin = strcmp(filename, "-") == 0; /* Suppress clang-19 warning clang-analyzer-unix.Stream */

    if (mirror_mode) {
        /* Use directory if any from outfile */
        if (output_given && symbol->outfile[0]) {
#ifndef _WIN32
            const char *dir = strrchr(symbol->outfile, '/');
#else
            const char *dir = strrchr(symbol->outfile, '\\');
            if (!dir) {
                dir = strrchr(symbol->outfile, '/');
            }
#endif
            if (dir) {
                mirror_start_o = (int) (dir + 1 - symbol->outfile);
                if (mirror_start_o > 221) { /* Insist on leaving at least ~30 chars for filename */
                    fprintf(stderr, "Warning 188: directory for mirrored batch output too long (greater than 220),"
                            " ignoring\n");
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION; /* TODO: maybe new warning ZINT_WARN_INVALID_INPUT? */
                    mirror_start_o = 0;
                } else {
                    memcpy(output_file, symbol->outfile, mirror_start_o);
                }
            }
        }
    } else {
        if (symbol->outfile[0] == '\0' || !output_given) {
            strcpy(format_string, "~~~~~.");
            strncat(format_string, filetype, 3);
        } else {
            strcpy(format_string, symbol->outfile);
            set_extension(format_string, filetype);
        }
    }

    if (from_stdin) {
        file = stdin;
    } else {
#ifdef _WIN32
        file = win_fopen(filename, "rb");
#else
        file = fopen(filename, "rb");
#endif
        if (!file) {
            fprintf(stderr, "Error 102: Unable to read input file '%s' (%d: %s)\n", filename, errno, strerror(errno));
            fflush(stderr);
            return ZINT_ERROR_INVALID_DATA;
        }
    }

    do {
        int intChar;
        intChar = fgetc(file);
        if (intChar == EOF) {
            break;
        }
        character = (unsigned char) intChar;
        if (character == '\n') {
            if (buf_posn > 0 && buffer[buf_posn - 1] == '\r') {
                /* CR+LF - assume Windows formatting and remove CR */
                buf_posn--;
                buffer[buf_posn] = '\0';
            }

            if (mirror_mode == 0) {
                inpos = 0;
                local_line_count = line_count;
                memset(number, 0, sizeof(number));
                memset(reverse_number, 0, sizeof(reverse_number));
                memset(reversed_string, 0, sizeof(reversed_string));
                memset(output_file, 0, sizeof(output_file));
                do {
                    number[inpos] = (local_line_count % 10) + '0';
                    local_line_count /= 10;
                    inpos++;
                } while (local_line_count > 0);
                number[inpos] = '\0';

                for (i = 0; i < inpos; i++) {
                    reverse_number[i] = number[inpos - i - 1];
                }

                format_len = (int) strlen(format_string);
                for (i = format_len; i > 0; i--) {
                    format_char = format_string[i - 1];

                    switch (format_char) {
                        case '#':
                            if (inpos > 0) {
                                adjusted[0] = reverse_number[inpos - 1];
                                inpos--;
                            } else {
                                adjusted[0] = ' ';
                            }
                            break;
                        case '~':
                            if (inpos > 0) {
                                adjusted[0] = reverse_number[inpos - 1];
                                inpos--;
                            } else {
                                adjusted[0] = '0';
                            }
                            break;
                        case '@':
                            if (inpos > 0) {
                                adjusted[0] = reverse_number[inpos - 1];
                                inpos--;
                            } else {
#ifndef _WIN32
                                adjusted[0] = '*';
#else
                                adjusted[0] = '+';
#endif
                            }
                            break;
                        default:
                            adjusted[0] = format_string[i - 1];
                            break;
                    }
                    strcat(reversed_string, adjusted);
                }

                for (i = 0; i < format_len; i++) {
                    output_file[i] = reversed_string[format_len - i - 1];
                }
            } else {
                /* Name the output file from the data being processed */
                i = 0;
                o = mirror_start_o;
                do {
                    if (buffer[i] < 0x20) {
                        output_file[o] = '_';
                    } else {
                        switch (buffer[i]) {
                            case '!':
                            case '"':
                            case '*':
                            case '/':
                            case ':':
                            case '<':
                            case '>':
                            case '?':
                            case '\\':
                            case '|':
                            case 0x7f: /* DEL */
                                output_file[o] = '_';
                                break;
                            default:
                                output_file[o] = buffer[i];
                                break;
                        }
                    }

                    /* Skip escape characters */
                    if ((buffer[i] == '\\') && (symbol->input_mode & ESCAPE_MODE)) {
                        i++;
                        if (buffer[i] == 'x') {
                            i += 2;
                        } else if (buffer[i] == 'd' || buffer[i] == 'o') {
                            i += 3;
                        } else if (buffer[i] == 'u') {
                            i += 4;
                        } else if (buffer[i] == 'U') {
                            i += 6;
                        }
                    }
                    i++;
                    o++;
                } while (i < buf_posn && o < 251);

                /* Add file extension */
                output_file[o] = '.';
                output_file[o + 1] = '\0';

                strncat(output_file, filetype, 3);
            }

            strcpy(symbol->outfile, output_file);
            warn_number = ZBarcode_Encode_and_Print(symbol, buffer, buf_posn, rotate_angle);
            if (warn_number != 0) {
                fprintf(stderr, "On line %d: %s\n", line_count, symbol->errtxt);
                fflush(stderr);
                if (warn_number >= ZINT_ERROR) {
                    error_number = warn_number;
                }
            }
            ZBarcode_Clear(symbol);
            memset(buffer, 0, sizeof(buffer));
            buf_posn = 0;
            line_count++;
        } else {
            buffer[buf_posn] = character;
            buf_posn++;
        }
        if (buf_posn >= (int) sizeof(buffer)) {
            fprintf(stderr, "On line %d: Error 103: Input data too long\n", line_count);
            fflush(stderr);
            do {
                if ((intChar = fgetc(file)) == EOF) {
                    break;
                }
                character = (unsigned char) intChar;
            } while ((!feof(file)) && (character != '\n'));
        }
    } while ((!feof(file)) && (line_count < 2000000000));

    if (character != '\n') {
        fprintf(stderr, "Warning 104: No newline at end of file\n");
        fflush(stderr);
        warn_number = ZINT_WARN_INVALID_OPTION; /* TODO: maybe new warning e.g. ZINT_WARN_INVALID_INPUT? */
    }

    if (!from_stdin) {
        if (fclose(file) != 0) {
            fprintf(stderr, "Warning 196: Failure on closing input file '%s' (%d: %s)\n", filename, errno,
                    strerror(errno));
            fflush(stderr);
            warn_number = ZINT_WARN_INVALID_OPTION; /* TODO: maybe new warning e.g. ZINT_WARN_INVALID_INPUT? */
        }
    }
    if (error_number == 0) {
        error_number = warn_number;
    }
    return error_number;
}

/* Stuff to convert args on Windows command line to UTF-8 */
#ifdef _WIN32
#include <windows.h>

#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS    0x00000080
#endif

static int win_argc = 0;
static char **win_argv = NULL;

/* Free Windows args */
static void win_free_args(void) {
    int i;
    if (!win_argv) {
        return;
    }
    for (i = 0; i < win_argc; i++) {
        if (!win_argv[i]) {
            break;
        }
        free(win_argv[i]);
        win_argv[i] = NULL;
    }
    free(win_argv);
    win_argv = NULL;
}

/* Using Wine version of `CommandLineToArgvW()` (slightly adapted) to avoid loading shell32.dll - see
   https://source.winehq.org/git/wine.git/blob/5a66eab72:/dlls/shcore/main.c#l264
   and https://news.ycombinator.com/item?id=18596841 */
/*
 * Copyright 2002 Jon Griffiths
 * Copyright 2016 Sebastian Lackner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */
static WCHAR **win_CommandLineToArgvW(const WCHAR *cmdline, int *numargs) {
    int qcount, bcount;
    const WCHAR *s;
    WCHAR **argv;
    DWORD argc;
    WCHAR *d;

    /* Adapted to require non-empty command line */
    if (*cmdline == 0) {
        return NULL;
    }

    /* --- First count the arguments */
    argc = 1;
    s = cmdline;
    /* The first argument, the executable path, follows special rules */
    if (*s == '"') {
        /* The executable path ends at the next quote, no matter what */
        s++;
        while (*s)
            if (*s++ == '"')
                break;
    } else {
        /* The executable path ends at the next space, no matter what */
        while (*s && *s != ' ' && *s != '\t')
            s++;
    }
    /* Skip to the first argument, if any */
    while (*s == ' ' || *s == '\t')
        s++;
    if (*s)
        argc++;

    /* Analyze the remaining arguments */
    qcount = bcount = 0;
    while (*s) {
        if ((*s == ' ' || *s == '\t') && qcount == 0) {
            /* Skip to the next argument and count it if any */
            while (*s == ' ' || *s == '\t')
                s++;
            if (*s)
                argc++;
            bcount = 0;
        } else if (*s == '\\') {
            /* '\', count them */
            bcount++;
            s++;
        } else if (*s == '"') {
            /* '"' */
            if ((bcount & 1) == 0)
                qcount++; /* Unescaped '"' */
            s++;
            bcount = 0;
            /* Consecutive quotes, see comment in copying code below */
            while (*s == '"') {
                qcount++;
                s++;
            }
            qcount = qcount % 3;
            if (qcount == 2)
                qcount = 0;
        } else {
            /* A regular character */
            bcount = 0;
            s++;
        }
    }

    /* Allocate in a single lump, the string array, and the strings that go
     * with it. This way the caller can make a single LocalFree() call to free
     * both, as per MSDN.
     */
    argv = LocalAlloc(LMEM_FIXED, (argc + 1) * sizeof(WCHAR *) + (lstrlenW(cmdline) + 1) * sizeof(WCHAR));
    if (!argv)
        return NULL;

    /* --- Then split and copy the arguments */
    argv[0] = d = lstrcpyW((WCHAR *)(argv + argc + 1), cmdline);
    argc = 1;
    /* The first argument, the executable path, follows special rules */
    if (*d == '"') {
        /* The executable path ends at the next quote, no matter what */
        s = d + 1;
        while (*s) {
            if (*s == '"') {
                s++;
                break;
            }
            *d++ = *s++;
        }
    } else {
        /* The executable path ends at the next space, no matter what */
        while (*d && *d != ' ' && *d != '\t')
            d++;
        s = d;
        if (*s)
            s++;
    }
    /* Close the executable path */
    *d++ = 0;
    /* Skip to the first argument and initialize it if any */
    while (*s == ' ' || *s == '\t')
        s++;
    if (!*s) {
        /* There are no parameters so we are all done */
        argv[argc] = NULL;
        *numargs = argc;
        return argv;
    }

    /* Split and copy the remaining arguments */
    argv[argc++] = d;
    qcount = bcount = 0;
    while (*s) {
        if ((*s == ' ' || *s == '\t') && qcount == 0) {
            /* Close the argument */
            *d++ = 0;
            bcount = 0;

            /* Skip to the next one and initialize it if any */
            do {
                s++;
            } while (*s == ' ' || *s == '\t');
            if (*s)
                argv[argc++] = d;
        } else if (*s == '\\') {
            *d++ = *s++;
            bcount++;
        } else if (*s == '"') {
            if ((bcount & 1) == 0) {
                /* Preceded by an even number of '\', this is half that
                 * number of '\', plus a quote which we erase.
                 */
                d -= bcount / 2;
                qcount++;
            } else {
                /* Preceded by an odd number of '\', this is half that
                 * number of '\' followed by a '"'
                 */
                d = d - bcount / 2 - 1;
                *d++ = '"';
            }
            s++;
            bcount = 0;
            /* Now count the number of consecutive quotes. Note that qcount
             * already takes into account the opening quote if any, as well as
             * the quote that lead us here.
             */
            while (*s == '"') {
                if (++qcount == 3) {
                    *d++ = '"';
                    qcount = 0;
                }
                s++;
            }
            if (qcount == 2)
                qcount = 0;
        } else {
            /* A regular character */
            *d++ = *s++;
            bcount = 0;
        }
    }
    *d = '\0';
    argv[argc] = NULL;
    *numargs = argc;

    return argv;
}

/* For Windows replace args with UTF-8 versions */
static void win_args(int *p_argc, char ***p_argv) {
    int i;
    LPWSTR *szArgList = win_CommandLineToArgvW(GetCommandLineW(), &win_argc);
    if (szArgList) {
        if (!(win_argv = (char **) calloc(win_argc + 1, sizeof(char *)))) {
            LocalFree(szArgList);
        } else {
            for (i = 0; i < win_argc; i++) {
                const int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, szArgList[i], -1, NULL, 0,
                                                    NULL /*lpDefaultChar*/, NULL /*lpUsedDefaultChar*/);
                if (len == 0 || !(win_argv[i] = malloc(len + 1))) {
                    win_free_args();
                    LocalFree(szArgList);
                    return;
                }
                if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, szArgList[i], -1, win_argv[i], len,
                                        NULL /*lpDefaultChar*/, NULL /*lpUsedDefaultChar*/) == 0) {
                    win_free_args();
                    LocalFree(szArgList);
                    return;
                }
            }
            for (i = 0; i < win_argc; i++) {
                (*p_argv)[i] = win_argv[i];
            }
            *p_argc = win_argc;
            LocalFree(szArgList);
        }
    }
}

/* Convert UTF-8 to Windows wide chars. Ticket #288, props Marcel */
#define utf8_to_wide(u, w, r) \
    { \
        int lenW; /* Includes NUL terminator */ \
        if ((lenW = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, u, -1, NULL, 0)) == 0) return r; \
        w = (wchar_t *) z_alloca(sizeof(wchar_t) * lenW); \
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, u, -1, w, lenW) == 0) return r; \
    }

/* Do `fopen()` on Windows, assuming `filename` is UTF-8 encoded. Ticket #288, props Marcel */
static FILE *win_fopen(const char *filename, const char *mode) {
    wchar_t *filenameW, *modeW;

    utf8_to_wide(filename, filenameW, NULL);
    utf8_to_wide(mode, modeW, NULL);

    return _wfopen(filenameW, modeW);
}
#endif /* _WIN32 */

/* Helper to free Windows args on exit */
static int do_exit(int error_number) {
#ifdef _WIN32
    win_free_args();
#endif
    exit(error_number);
    return error_number; /* Not reached */
}

typedef struct { const char *arg; int opt; } arg_opt;

int main(int argc, char **argv) {
    struct zint_symbol *my_symbol;
    struct zint_seg segs[10] = {{0}};
    int error_number = 0;
    int warn_number = 0;
    int rotate_angle = 0;
    int help = 0;
    int data_cnt = 0;
    int input_cnt = 0;
    int batch_mode = 0;
    int mirror_mode = 0;
    int fullmultibyte = 0;
    int mask = 0;
    int separator = 0;
    int addon_gap = 0;
    int rows = 0;
    char filetype[4] = {0};
    int output_given = 0;
    int png_refused;
    int val;
    int i;
    int ret;
    char *outfile_extension;
    int data_arg_num = 0;
    int seg_count = 0;
    float x_dim_mm = 0.0f, dpmm = 0.0f;
    float float_opt;
    char errbuf[64]; /* For `validate_float()` */
    arg_opt *arg_opts = (arg_opt *) z_alloca(sizeof(arg_opt) * argc);

    const int no_png = ZBarcode_NoPng();

    if (argc == 1) {
        usage(no_png);
        exit(ZINT_ERROR_INVALID_DATA);
    }

    my_symbol = ZBarcode_Create();
    if (!my_symbol) {
        fprintf(stderr, "Error 151: Memory failure\n");
        exit(ZINT_ERROR_MEMORY);
    }
    my_symbol->input_mode = UNICODE_MODE;

#ifdef _WIN32
    win_args(&argc, &argv);
#endif

    opterr = 0; /* Disable `getopt_long_only()` printing errors */
    while (1) {
        enum options {
            OPT_ADDONGAP = 128, OPT_BATCH, OPT_BINARY, OPT_BG, OPT_BIND, OPT_BIND_TOP, OPT_BOLD, OPT_BORDER, OPT_BOX,
            OPT_CMYK, OPT_COLS, OPT_COMPLIANTHEIGHT,
            OPT_DIRECT, OPT_DMISO144, OPT_DMRE, OPT_DOTSIZE, OPT_DOTTY, OPT_DUMP,
            OPT_ECI, OPT_EMBEDFONT, OPT_ESC, OPT_EXTRAESC, OPT_FAST, OPT_FG, OPT_FILETYPE, OPT_FULLMULTIBYTE,
            OPT_GS1, OPT_GS1NOCHECK, OPT_GS1PARENS, OPT_GSSEP, OPT_GUARDDESCENT, OPT_GUARDWHITESPACE,
            OPT_HEIGHT, OPT_HEIGHTPERROW, OPT_INIT, OPT_MIRROR, OPT_MASK, OPT_MODE,
            OPT_NOBACKGROUND, OPT_NOQUIETZONES, OPT_NOTEXT, OPT_PRIMARY, OPT_QUIETZONES,
            OPT_ROTATE, OPT_ROWS, OPT_SCALE, OPT_SCALEXDIM, OPT_SCMVV, OPT_SECURE,
            OPT_SEG1, OPT_SEG2, OPT_SEG3, OPT_SEG4, OPT_SEG5, OPT_SEG6, OPT_SEG7, OPT_SEG8, OPT_SEG9,
            OPT_SEPARATOR, OPT_SMALL, OPT_SQUARE, OPT_STRUCTAPP, OPT_TEXTGAP,
            OPT_VERBOSE, OPT_VERS, OPT_VWHITESP, OPT_WERROR
        };
        static const struct option long_options[] = {
            {"addongap", 1, NULL, OPT_ADDONGAP},
            {"barcode", 1, NULL, 'b'},
            {"batch", 0, NULL, OPT_BATCH},
            {"binary", 0, NULL, OPT_BINARY},
            {"bg", 1, 0, OPT_BG},
            {"bgcolor", 1, 0, OPT_BG}, /* Synonym */
            {"bgcolour", 1, 0, OPT_BG}, /* Synonym */
            {"bind", 0, NULL, OPT_BIND},
            {"bindtop", 0, NULL, OPT_BIND_TOP},
            {"bold", 0, NULL, OPT_BOLD},
            {"border", 1, NULL, OPT_BORDER},
            {"box", 0, NULL, OPT_BOX},
            {"cmyk", 0, NULL, OPT_CMYK},
            {"cols", 1, NULL, OPT_COLS},
            {"compliantheight", 0, NULL, OPT_COMPLIANTHEIGHT},
            {"data", 1, NULL, 'd'},
            {"direct", 0, NULL, OPT_DIRECT},
            {"dmiso144", 0, NULL, OPT_DMISO144},
            {"dmre", 0, NULL, OPT_DMRE},
            {"dotsize", 1, NULL, OPT_DOTSIZE},
            {"dotty", 0, NULL, OPT_DOTTY},
            {"dump", 0, NULL, OPT_DUMP},
            {"eci", 1, NULL, OPT_ECI},
            {"ecinos", 0, NULL, 'e'},
            {"embedfont", 0, NULL, OPT_EMBEDFONT},
            {"esc", 0, NULL, OPT_ESC},
            {"extraesc", 0, NULL, OPT_EXTRAESC},
            {"fast", 0, NULL, OPT_FAST},
            {"fg", 1, 0, OPT_FG},
            {"fgcolor", 1, 0, OPT_FG}, /* Synonym */
            {"fgcolour", 1, 0, OPT_FG}, /* Synonym */
            {"filetype", 1, NULL, OPT_FILETYPE},
            {"fullmultibyte", 0, NULL, OPT_FULLMULTIBYTE},
            {"gs1", 0, 0, OPT_GS1},
            {"gs1nocheck", 0, NULL, OPT_GS1NOCHECK},
            {"gs1parens", 0, NULL, OPT_GS1PARENS},
            {"gssep", 0, NULL, OPT_GSSEP},
            {"guarddescent", 1, NULL, OPT_GUARDDESCENT},
            {"guardwhitespace", 0, NULL, OPT_GUARDWHITESPACE},
            {"height", 1, NULL, OPT_HEIGHT},
            {"heightperrow", 0, NULL, OPT_HEIGHTPERROW},
            {"help", 0, NULL, 'h'},
            {"init", 0, NULL, OPT_INIT},
            {"input", 1, NULL, 'i'},
            {"mirror", 0, NULL, OPT_MIRROR},
            {"mask", 1, NULL, OPT_MASK},
            {"mode", 1, NULL, OPT_MODE},
            {"nobackground", 0, NULL, OPT_NOBACKGROUND},
            {"noquietzones", 0, NULL, OPT_NOQUIETZONES},
            {"notext", 0, NULL, OPT_NOTEXT},
            {"output", 1, NULL, 'o'},
            {"primary", 1, NULL, OPT_PRIMARY},
            {"quietzones", 0, NULL, OPT_QUIETZONES},
            {"reverse", 0, NULL, 'r'},
            {"rotate", 1, NULL, OPT_ROTATE},
            {"rows", 1, NULL, OPT_ROWS},
            {"scale", 1, NULL, OPT_SCALE},
            {"scalexdimdp", 1, NULL, OPT_SCALEXDIM},
            {"scmvv", 1, NULL, OPT_SCMVV},
            {"secure", 1, NULL, OPT_SECURE},
            {"seg1", 1, NULL, OPT_SEG1},
            {"seg2", 1, NULL, OPT_SEG2},
            {"seg3", 1, NULL, OPT_SEG3},
            {"seg4", 1, NULL, OPT_SEG4},
            {"seg5", 1, NULL, OPT_SEG5},
            {"seg6", 1, NULL, OPT_SEG6},
            {"seg7", 1, NULL, OPT_SEG7},
            {"seg8", 1, NULL, OPT_SEG8},
            {"seg9", 1, NULL, OPT_SEG9},
            {"separator", 1, NULL, OPT_SEPARATOR},
            {"small", 0, NULL, OPT_SMALL},
            {"square", 0, NULL, OPT_SQUARE},
            {"structapp", 1, NULL, OPT_STRUCTAPP},
            {"textgap", 1, NULL, OPT_TEXTGAP},
            {"types", 0, NULL, 't'},
            {"verbose", 0, NULL, OPT_VERBOSE}, /* Currently undocumented, output some debug info */
            {"vers", 1, NULL, OPT_VERS},
            {"version", 0, NULL, 'v'},
            {"vwhitesp", 1, NULL, OPT_VWHITESP},
            {"werror", 0, NULL, OPT_WERROR},
            {"whitesp", 1, NULL, 'w'},
            {NULL, 0, NULL, 0}
        };
        const int c = getopt_long_only(argc, argv, "b:d:ehi:o:rtvw:", long_options, NULL);
        if (c == -1) break;

        switch (c) {
            case OPT_ADDONGAP:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 139: Invalid add-on gap value (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (val >= 7 && val <= 12) {
                    addon_gap = val;
                } else {
                    fprintf(stderr, "Warning 140: Add-on gap '%d' out of range (7 to 12), ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_BATCH:
                if (data_cnt == 0) {
                    /* Switch to batch processing mode */
                    batch_mode = 1;
                } else {
                    fprintf(stderr, "Warning 141: Can't use batch mode if data given, ignoring\n");
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_BG:
                strncpy(my_symbol->bgcolour, optarg, 15); /* Allow for "CCC,MMM,YYY,KKK" */
                break;
            case OPT_BINARY:
                my_symbol->input_mode = (my_symbol->input_mode & ~0x07) | DATA_MODE;
                break;
            case OPT_BIND:
                my_symbol->output_options |= BARCODE_BIND;
                break;
            case OPT_BIND_TOP:
                my_symbol->output_options |= BARCODE_BIND_TOP;
                break;
            case OPT_BOLD:
                my_symbol->output_options |= BOLD_TEXT;
                break;
            case OPT_BORDER:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 107: Invalid border width value (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (val <= 1000) { /* `val` >= 0 always */
                    my_symbol->border_width = val;
                } else {
                    fprintf(stderr, "Warning 108: Border width '%d' out of range (0 to 1000), ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_BOX:
                my_symbol->output_options |= BARCODE_BOX;
                break;
            case OPT_CMYK:
                my_symbol->output_options |= CMYK_COLOUR;
                break;
            case OPT_COLS:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 131: Invalid columns value (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if ((val >= 1) && (val <= 200)) {
                    my_symbol->option_2 = val;
                } else {
                    fprintf(stderr, "Warning 111: Number of columns '%d' out of range (1 to 200), ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_COMPLIANTHEIGHT:
                my_symbol->output_options |= COMPLIANT_HEIGHT;
                break;
            case OPT_DIRECT:
                my_symbol->output_options |= BARCODE_STDOUT;
                break;
            case OPT_DMISO144:
                my_symbol->option_3 |= DM_ISO_144;
                break;
            case OPT_DMRE:
                /* Square overwrites DMRE */
                if ((my_symbol->option_3 & 0x7F) != DM_SQUARE) {
                    my_symbol->option_3 = DM_DMRE | (my_symbol->option_3 & ~0x7F);
                }
                break;
            case OPT_DOTSIZE:
                if (!validate_float(optarg, 0 /*allow_neg*/, &float_opt, errbuf)) {
                    fprintf(stderr, "Error 181: Invalid dot radius floating point (%s)\n", errbuf);
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (float_opt >= 0.01f) {
                    my_symbol->dot_size = float_opt;
                } else {
                    fprintf(stderr, "Warning 106: Invalid dot radius value (less than 0.01), ignoring\n");
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_DOTTY:
                my_symbol->output_options |= BARCODE_DOTTY_MODE;
                break;
            case OPT_DUMP:
                my_symbol->output_options |= BARCODE_STDOUT;
                strcpy(my_symbol->outfile, "dummy.txt");
                break;
            case OPT_ECI:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 138: Invalid ECI code (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (val <= 999999) { /* `val` >= 0 always */
                    my_symbol->eci = val;
                } else {
                    fprintf(stderr, "Warning 118: ECI code '%d' out of range (0 to 999999), ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_EMBEDFONT:
                my_symbol->output_options |= EMBED_VECTOR_FONT;
                break;
            case OPT_ESC:
                my_symbol->input_mode |= ESCAPE_MODE;
                break;
            case OPT_EXTRAESC:
                my_symbol->input_mode |= EXTRA_ESCAPE_MODE;
                break;
            case OPT_FAST:
                my_symbol->input_mode |= FAST_MODE;
                break;
            case OPT_FG:
                strncpy(my_symbol->fgcolour, optarg, 15); /* Allow for "CCC,MMM,YYY,KKK" */
                break;
            case OPT_FILETYPE:
                /* Select the type of output file */
                if (supported_filetype(optarg, no_png, &png_refused)) {
                    strncpy(filetype, optarg, (size_t) 3);
                } else {
                    if (png_refused) {
                        fprintf(stderr, "Warning 152: PNG format disabled at compile time, ignoring\n");
                    } else {
                        fprintf(stderr, "Warning 142: File type '%s' not supported, ignoring\n", optarg);
                    }
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_FULLMULTIBYTE:
                fullmultibyte = 1;
                break;
            case OPT_GS1:
                my_symbol->input_mode = (my_symbol->input_mode & ~0x07) | GS1_MODE;
                break;
            case OPT_GS1NOCHECK:
                my_symbol->input_mode |= GS1NOCHECK_MODE;
                break;
            case OPT_GS1PARENS:
                my_symbol->input_mode |= GS1PARENS_MODE;
                break;
            case OPT_GSSEP:
                my_symbol->output_options |= GS1_GS_SEPARATOR;
                break;
            case OPT_GUARDDESCENT:
                if (!validate_float(optarg, 0 /*allow_neg*/, &float_opt, errbuf)) {
                    fprintf(stderr, "Error 182: Invalid guard bar descent floating point (%s)\n", errbuf);
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (float_opt >= 0.0f && float_opt <= 50.0f) {
                    my_symbol->guard_descent = float_opt;
                } else {
                    fprintf(stderr, "Warning 135: Guard bar descent '%g' out of range (0 to 50), ignoring\n",
                            float_opt);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_GUARDWHITESPACE:
                my_symbol->output_options |= EANUPC_GUARD_WHITESPACE;
                break;
            case OPT_HEIGHT:
                if (!validate_float(optarg, 0 /*allow_neg*/, &float_opt, errbuf)) {
                    fprintf(stderr, "Error 183: Invalid symbol height floating point (%s)\n", errbuf);
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (float_opt >= 0.5f && float_opt <= 2000.0f) {
                    my_symbol->height = float_opt;
                } else {
                    fprintf(stderr, "Warning 110: Symbol height '%g' out of range (0.5 to 2000), ignoring\n",
                            float_opt);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_HEIGHTPERROW:
                my_symbol->input_mode |= HEIGHTPERROW_MODE;
                break;
            case OPT_INIT:
                my_symbol->output_options |= READER_INIT;
                break;
            case OPT_MIRROR:
                /* Use filenames which reflect content */
                mirror_mode = 1;
                break;
            case OPT_MASK:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 148: Invalid mask value (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (val <= 7) { /* `val` >= 0 always */
                    mask = val + 1;
                } else {
                    /* mask pattern >= 0 and <= 7 (i.e. values >= 1 and <= 8) only permitted */
                    fprintf(stderr, "Warning 147: Mask value '%d' out of range (0 to 7), ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_MODE:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 136: Invalid mode value (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (val <= 6) { /* `val` >= 0 always */
                    my_symbol->option_1 = val;
                } else {
                    fprintf(stderr, "Warning 116: Mode value '%d' out of range (0 to 6), ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_NOBACKGROUND:
                strcpy(my_symbol->bgcolour, "ffffff00");
                break;
            case OPT_NOQUIETZONES:
                my_symbol->output_options |= BARCODE_NO_QUIET_ZONES;
                break;
            case OPT_NOTEXT:
                my_symbol->show_hrt = 0;
                break;
            case OPT_PRIMARY:
                if (strlen(optarg) <= 127) {
                    strcpy(my_symbol->primary, optarg);
                } else {
                    strncpy(my_symbol->primary, optarg, 127);
                    fprintf(stderr,
                            "Warning 115: Primary data string too long (127 character maximum), truncating\n");
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_QUIETZONES:
                my_symbol->output_options |= BARCODE_QUIET_ZONES;
                break;
            case OPT_ROTATE:
                /* Only certain inputs allowed */
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 117: Invalid rotation value (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                switch (val) {
                    case 0:
                    case 90:
                    case 180:
                    case 270:
                        rotate_angle = val;
                        break;
                    default:
                        fprintf(stderr, "Warning 137: Rotation value '%d' out of range (0, 90, 180 or 270 only),"
                                " ignoring\n", val);
                        fflush(stderr);
                        warn_number = ZINT_WARN_INVALID_OPTION;
                        break;
                }
                break;
            case OPT_ROWS:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 132: Invalid rows value (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if ((val >= 1) && (val <= 90)) {
                    rows = val;
                } else {
                    fprintf(stderr, "Warning 112: Number of rows '%d' out of range (1 to 90), ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_SCALE:
                if (!validate_float(optarg, 0 /*allow_neg*/, &float_opt, errbuf)) {
                    fprintf(stderr, "Error 184: Invalid scale floating point (%s)\n", errbuf);
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (float_opt >= 0.01f) {
                    my_symbol->scale = float_opt;
                } else {
                    fprintf(stderr, "Warning 105: Invalid scale value '%g' (less than 0.01), ignoring\n", float_opt);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_SCALEXDIM:
                if (!validate_scalexdimdp(optarg, &x_dim_mm, &dpmm)) {
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (x_dim_mm > 10.0f || dpmm > 1000.0f) {
                    if (x_dim_mm > 10.0f) {
                        fprintf(stderr, "Warning 185: scalexdimdp X-dim '%g' out of range (greater than 10),"
                                " ignoring\n", x_dim_mm);
                    } else {
                        fprintf(stderr, "Warning 186: scalexdimdp resolution '%g' out of range (greater than 1000),"
                                " ignoring\n", dpmm);
                    }
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                    x_dim_mm = dpmm = 0.0f;
                }
                break;
            case OPT_SCMVV:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 149: Invalid Structured Carrier Message version value (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (val <= 99) { /* `val` >= 0 always */
                    my_symbol->option_2 = val + 1;
                } else {
                    /* Version 00-99 only */
                    fprintf(stderr, "Warning 150: Structured Carrier Message version '%d' out of range (0 to 99),"
                            " ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_SECURE:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 134: Invalid ECC value (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (val <= 8) { /* `val` >= 0 always */
                    my_symbol->option_1 = val;
                } else {
                    fprintf(stderr, "Warning 114: ECC level '%d' out of range (0 to 8), ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_SEG1:
            case OPT_SEG2:
            case OPT_SEG3:
            case OPT_SEG4:
            case OPT_SEG5:
            case OPT_SEG6:
            case OPT_SEG7:
            case OPT_SEG8:
            case OPT_SEG9:
                if (batch_mode == 0) {
                    val = c - OPT_SEG1 + 1; /* Segment number */
                    if (segs[val].source) {
                        fprintf(stderr, "Error 164: Duplicate segment %d\n", val);
                        return do_exit(ZINT_ERROR_INVALID_OPTION);
                    }
                    if (!validate_seg(optarg, c - OPT_SEG1 + 1, segs)) {
                        return do_exit(ZINT_ERROR_INVALID_OPTION);
                    }
                    if (val >= seg_count) {
                        seg_count = val + 1;
                    }
                } else {
                    fprintf(stderr, "Warning 165: Can't define segments in batch mode, ignoring '%s'\n", optarg);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_SEPARATOR:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 128: Invalid separator value (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (val <= 4) { /* `val` >= 0 always */
                    separator = val;
                } else {
                    /* Greater than 4 values are not permitted */
                    fprintf(stderr, "Warning 127: Separator value '%d' out of range (0 to 4), ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_SMALL:
                my_symbol->output_options |= SMALL_TEXT;
                break;
            case OPT_SQUARE:
                my_symbol->option_3 = DM_SQUARE | (my_symbol->option_3 & ~0x7F);
                break;
            case OPT_STRUCTAPP:
                memset(&my_symbol->structapp, 0, sizeof(my_symbol->structapp));
                if (!validate_structapp(optarg, &my_symbol->structapp)) {
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                break;
            case OPT_TEXTGAP:
                if (!validate_float(optarg, 1 /*allow_neg*/, &float_opt, errbuf)) {
                    fprintf(stderr, "Error 194: Invalid text gap floating point (%s)\n", errbuf);
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (float_opt >= -5.0f && float_opt <= 10.0f) {
                    my_symbol->text_gap = float_opt;
                } else {
                    fprintf(stderr, "Warning 195: Text gap '%g' out of range (-5 to 10), ignoring\n",
                            float_opt);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_VERBOSE:
                my_symbol->debug = 1;
                break;
            case OPT_VERS:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 133: Invalid version value (digits only)\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if ((val >= 1) && (val <= 999)) {
                    my_symbol->option_2 = val;
                } else {
                    fprintf(stderr, "Warning 113: Version value '%d' out of range (1 to 999), ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_VWHITESP:
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 153: Invalid vertical whitespace value '%s' (digits only)\n", optarg);
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (val <= 1000) { /* `val` >= 0 always */
                    my_symbol->whitespace_height = val;
                } else {
                    fprintf(stderr,
                            "Warning 154: Vertical whitespace value '%d' out of range (0 to 1000), ignoring\n", val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;
            case OPT_WERROR:
                my_symbol->warn_level = WARN_FAIL_ALL;
                break;

            case 'h':
                usage(no_png);
                help = 1;
                break;
            case 'v':
                version(no_png);
                help = 1;
                break;
            case 't':
                types();
                help = 1;
                break;
            case 'e':
                show_eci();
                help = 1;
                break;

            case 'b':
                if (!validate_int(optarg, -1 /*len*/, &val) && !(val = get_barcode_name(optarg))) {
                    fprintf(stderr, "Error 119: Invalid barcode type '%s'\n", optarg);
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                my_symbol->symbology = val;
                break;

            case 'w':
                if (!validate_int(optarg, -1 /*len*/, &val)) {
                    fprintf(stderr, "Error 120: Invalid horizontal whitespace value '%s' (digits only)\n", optarg);
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (val <= 1000) { /* `val` >= 0 always */
                    my_symbol->whitespace_width = val;
                } else {
                    fprintf(stderr,
                            "Warning 121: Horizontal whitespace value '%d' out of range (0 to 1000), ignoring\n",
                            val);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;

            case 'd': /* we have some data! */
                if (batch_mode == 0) {
                    arg_opts[data_arg_num].arg = optarg;
                    arg_opts[data_arg_num].opt = c;
                    data_arg_num++;
                    data_cnt++;
                } else {
                    fprintf(stderr, "Warning 122: Can't define data in batch mode, ignoring '%s'\n", optarg);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;

            case 'i': /* Take data from file */
                if (batch_mode == 0 || input_cnt == 0) {
                    arg_opts[data_arg_num].arg = optarg;
                    arg_opts[data_arg_num].opt = c;
                    data_arg_num++;
                    input_cnt++;
                } else {
                    fprintf(stderr, "Warning 143: Can only define one input file in batch mode, ignoring '%s'\n",
                            optarg);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
                break;

            case 'o':
                strncpy(my_symbol->outfile, optarg, 255);
                output_given = 1;
                break;

            case 'r':
                strcpy(my_symbol->fgcolour, "ffffff");
                strcpy(my_symbol->bgcolour, "000000");
                break;

            case '?':
                if (optopt) {
                    for (i = 0; i < ARRAY_SIZE(long_options) && long_options[i].val != optopt; i++);
                    if (i == ARRAY_SIZE(long_options)) { /* Shouldn't happen */
                        fprintf(stderr, "Error 125: ?? unknown optopt '%d'\n", optopt); /* Not reached */
                        return do_exit(ZINT_ERROR_ENCODING_PROBLEM);
                    }
                    if (long_options[i].has_arg) {
                        fprintf(stderr, "Error 109: option '%s' requires an argument\n", argv[optind - 1]);
                    } else {
                        const char *eqs = strchr(argv[optind - 1], '=');
                        const int optlen = eqs ? (int) (eqs - argv[optind - 1]) : (int) strlen(argv[optind - 1]);
                        fprintf(stderr, "Error 126: option '%.*s' does not take an argument\n", optlen,
                                argv[optind - 1]);
                    }
                } else {
                    fprintf(stderr, "Error 101: unknown option '%s'\n", argv[optind - 1]);
                }
                return do_exit(ZINT_ERROR_INVALID_OPTION);
                break;

            default: /* Shouldn't happen */
                fprintf(stderr, "Error 123: ?? getopt error 0%o\n", c); /* Not reached */
                return do_exit(ZINT_ERROR_ENCODING_PROBLEM);
                break;
        }
    }
    if (optind != argc) {
        if (optind + 1 == argc) {
            fprintf(stderr, "Warning 191: extra argument '%s' ignoring\n", argv[optind]);
        } else {
            fprintf(stderr, "Warning 192: extra arguments beginning with '%s' ignoring\n", argv[optind]);
        }
        fflush(stderr);
        warn_number = ZINT_WARN_INVALID_OPTION;
    }

    if (data_arg_num) {
        const int symbology = my_symbol->symbology;
        const unsigned int cap = ZBarcode_Cap(symbology, ZINT_CAP_STACKABLE | ZINT_CAP_EXTENDABLE |
                                    ZINT_CAP_FULL_MULTIBYTE | ZINT_CAP_MASK);
        if (fullmultibyte && (cap & ZINT_CAP_FULL_MULTIBYTE)) {
            my_symbol->option_3 = ZINT_FULL_MULTIBYTE;
        }
        if (mask && (cap & ZINT_CAP_MASK)) {
            my_symbol->option_3 |= mask << 8;
        }
        if (separator && (cap & ZINT_CAP_STACKABLE)) {
            my_symbol->option_3 = separator;
        }
        if (addon_gap && (cap & ZINT_CAP_EXTENDABLE)) {
            my_symbol->option_2 = addon_gap;
        }
        if (rows) {
            if (symbology == BARCODE_PDF417 || symbology == BARCODE_PDF417COMP || symbology == BARCODE_HIBC_PDF
                    || symbology == BARCODE_DBAR_EXPSTK || symbology == BARCODE_DBAR_EXPSTK_CC) {
                my_symbol->option_3 = rows;
            } else if (symbology == BARCODE_CODABLOCKF || symbology == BARCODE_HIBC_BLOCKF
                    || symbology == BARCODE_CODE16K || symbology == BARCODE_CODE49) {
                my_symbol->option_1 = rows;
            }
        }

        if (output_given && (my_symbol->output_options & BARCODE_STDOUT)) {
            my_symbol->output_options &= ~BARCODE_STDOUT;
            fprintf(stderr, "Warning 193: Output file given, ignoring '--direct' option\n");
            fflush(stderr);
            warn_number = ZINT_WARN_INVALID_OPTION;
        }
        if (batch_mode) {
            /* Take each line of text as a separate data set */
            if (data_arg_num > 1) {
                fprintf(stderr, "Warning 144: Processing first input file '%s' only\n", arg_opts[0].arg);
                fflush(stderr);
                warn_number = ZINT_WARN_INVALID_OPTION;
            }
            if (seg_count) {
                fprintf(stderr, "Warning 169: Ignoring segment arguments\n");
                fflush(stderr);
                warn_number = ZINT_WARN_INVALID_OPTION;
            }
            if (filetype[0] == '\0') {
                outfile_extension = get_extension(my_symbol->outfile);
                if (outfile_extension && supported_filetype(outfile_extension, no_png, NULL)) {
                    strcpy(filetype, outfile_extension);
                } else {
                    strcpy(filetype, no_png ? "gif" : "png");
                }
            }
            if (dpmm) { /* Allow `x_dim_mm` to be zero */
                if (x_dim_mm == 0.0f) {
                    x_dim_mm = ZBarcode_Default_Xdim(symbology);
                }
                float_opt = ZBarcode_Scale_From_XdimDp(symbology, x_dim_mm, dpmm, filetype);
                if (float_opt > 0.0f) {
                    my_symbol->scale = float_opt;
                    my_symbol->dpmm = dpmm;
                } else {
                    fprintf(stderr, "Warning 187: Invalid scalexdimdp X-dim '%g', resolution '%g' combo, ignoring\n",
                            x_dim_mm, dpmm);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
            }
            if (((symbology != BARCODE_MAXICODE && my_symbol->scale < 0.5f) || my_symbol->scale < 0.2f)
                    && is_raster(filetype, no_png)) {
                const int min = symbology != BARCODE_MAXICODE ? 5 : 2;
                fprintf(stderr, "Warning 145: Scaling less than 0.%d will be set to 0.%d for '%s' output\n", min, min,
                        filetype);
                fflush(stderr);
                warn_number = ZINT_WARN_INVALID_OPTION;
            }
            error_number = batch_process(my_symbol, arg_opts[0].arg, mirror_mode, filetype, output_given,
                            rotate_angle);
        } else {
            if (seg_count) {
                if (data_arg_num > 1) {
                    fprintf(stderr, "Error 170: Cannot specify segments and multiple data arguments together\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                if (arg_opts[0].opt != 'd') { /* For simplicity disallow input args for now */
                    fprintf(stderr, "Error 171: Cannot use input argument with segment arguments\n");
                    return do_exit(ZINT_ERROR_INVALID_OPTION);
                }
                segs[0].eci = my_symbol->eci;
                segs[0].source = (unsigned char *) arg_opts[0].arg;
                segs[0].length = (int) strlen(arg_opts[0].arg);
                for (i = 0; i < seg_count; i++) {
                    if (segs[i].source == NULL) {
                        fprintf(stderr, "Error 172: Segments must be consecutive - segment %d missing\n", i);
                        return do_exit(ZINT_ERROR_INVALID_OPTION);
                    }
                }
            }
            if (filetype[0] != '\0') {
                set_extension(my_symbol->outfile, filetype);
            }
            if (dpmm) { /* Allow `x_dim_mm` to be zero */
                if (x_dim_mm == 0.0f) {
                    x_dim_mm = ZBarcode_Default_Xdim(symbology);
                }
                float_opt = ZBarcode_Scale_From_XdimDp(symbology, x_dim_mm, dpmm, get_extension(my_symbol->outfile));
                if (float_opt > 0.0f) {
                    my_symbol->scale = float_opt;
                    my_symbol->dpmm = dpmm;
                } else {
                    fprintf(stderr, "Warning 190: Invalid scalexdimdp X-dim '%g', resolution '%g' combo, ignoring\n",
                            x_dim_mm, dpmm);
                    fflush(stderr);
                    warn_number = ZINT_WARN_INVALID_OPTION;
                }
            }
            if (((symbology != BARCODE_MAXICODE && my_symbol->scale < 0.5f) || my_symbol->scale < 0.2f)
                    && is_raster(get_extension(my_symbol->outfile), no_png)) {
                const int min = symbology != BARCODE_MAXICODE ? 5 : 2;
                fprintf(stderr, "Warning 146: Scaling less than 0.%d will be set to 0.%d for '%s' output\n", min, min,
                        get_extension(my_symbol->outfile));
                fflush(stderr);
                warn_number = ZINT_WARN_INVALID_OPTION;
            }
            for (i = 0; i < data_arg_num; i++) {
                if (arg_opts[i].opt == 'd') {
                    if (seg_count) {
                        ret = ZBarcode_Encode_Segs(my_symbol, segs, seg_count);
                    } else {
                        if (i == 1 && (ZBarcode_Cap(symbology, ZINT_CAP_STACKABLE) & ZINT_CAP_STACKABLE) == 0) {
                            fprintf(stderr,
                                    "Error 173: Symbology must be stackable if multiple data arguments given\n");
                            fflush(stderr);
                            error_number = ZINT_ERROR_INVALID_DATA;
                            break;
                        }
                        ret = ZBarcode_Encode(my_symbol, (unsigned char *) arg_opts[i].arg,
                                (int) strlen(arg_opts[i].arg));
                    }
                } else {
                    ret = ZBarcode_Encode_File(my_symbol, arg_opts[i].arg);
                }
                if (ret != 0) {
                    fprintf(stderr, "%s\n", my_symbol->errtxt);
                    fflush(stderr);
                    if (error_number < ZINT_ERROR) {
                        error_number = ret;
                    }
                }
            }
            if (error_number < ZINT_ERROR) {
                error_number = ZBarcode_Print(my_symbol, rotate_angle);

                if (error_number != 0) {
                    fprintf(stderr, "%s\n", my_symbol->errtxt);
                    fflush(stderr);
                }
            }
        }
    } else if (help == 0) {
        fprintf(stderr, "Warning 124: No data received, no symbol generated\n");
        fflush(stderr);
        warn_number = ZINT_WARN_INVALID_OPTION;
    }

    ZBarcode_Delete(my_symbol);

    return do_exit(error_number ? error_number : warn_number);
}

/* vim: set ts=4 sw=4 et : */
