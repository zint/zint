/* main.c - Command line handling routines for Zint */

/*
    libzint - the open source barcode library
    Copyright (C) 2008-2020 Robin Stuart <rstuart114@gmail.com>

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
/* vim: set ts=4 sw=4 et : */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <getopt.h>
#include <zint.h>
#else
#include <malloc.h>
#include "getopt.h"
#include "zint.h"
#endif

#define NESET "0123456789"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

/* Print list of supported symbologies */
static void types(void) {
    printf( " 1: Code 11           52: PZN                      96: DPD Code\n"
            " 2: Standard 2of5     53: Pharma Two-Track         97: Micro QR Code\n"
            " 3: Interleaved 2of5  55: PDF417                   98: HIBC Code 128\n"
            " 4: IATA 2of5         56: Compact PDF417           99: HIBC Code 39\n"
            " 6: Data Logic        57: Maxicode                102: HIBC Data Matrix\n"
            " 7: Industrial 2of5   58: QR Code                 104: HIBC QR Code\n"
            " 8: Code 39           60: Code 128-B              106: HIBC PDF417\n"
            " 9: Extended Code 39  63: AP Standard Customer    108: HIBC MicroPDF417\n"
            "13: EAN               66: AP Reply Paid           110: HIBC Codablock-F\n"
            "14: EAN + Check       67: AP Routing              112: HIBC Aztec Code\n"
            "16: GS1-128           68: AP Redirection          115: DotCode\n"
            "18: Codabar           69: ISBN                    116: Han Xin Code\n"
            "20: Code 128          70: RM4SCC                  121: RM Mailmark\n"
            "21: Leitcode          71: Data Matrix             128: Aztec Runes\n"
            "22: Identcode         72: EAN-14                  129: Code 32\n"
            "23: Code 16k          73: VIN                     130: Comp EAN\n"
            "24: Code 49           74: Codablock-F             131: Comp GS1-128\n"
            "25: Code 93           75: NVE-18                  132: Comp DataBar Omni\n"
            "28: Flattermarken     76: Japanese Post           133: Comp DataBar Ltd\n"
            "29: GS1 DataBar Omni  77: Korea Post              134: Comp DataBar Exp\n"
            "30: GS1 DataBar Ltd   79: GS1 DataBar Stack       135: Comp UPC-A\n"
            "31: GS1 DataBar Exp   80: GS1 DataBar Stack Omni  136: Comp UPC-E\n"
            "32: Telepen Alpha     81: GS1 DataBar Exp Stack   137: Comp DataBar Stack\n"
            "34: UPC-A             82: PLANET                  138: Comp DataBar Stack Omni\n"
            "35: UPC-A + Check     84: MicroPDF                139: Comp DataBar Exp Stack\n"
            "37: UPC-E             85: USPS Intelligent Mail   140: Channel Code\n"
            "38: UPC-E + Check     86: UK Plessey              141: Code One\n"
            "40: POSTNET           87: Telepen Numeric         142: Grid Matrix\n"
            "47: MSI Plessey       89: ITF-14                  143: UPNQR\n"
            "49: FIM               90: KIX Code                144: Ultracode\n"
            "50: Logmars           92: Aztec Code              145: rMQR\n"
            "51: Pharma One-Track  93: DAFT Code\n"
            );
}

/* Output usage information */

static void usage(void) {
    int zint_version = ZBarcode_Version();
    int version_major = zint_version / 10000;
    int version_minor = (zint_version % 10000) / 100;
    int version_release = zint_version % 100;
    int version_build;
    
    if (version_release > 10) {
        /* This is a test release */
        version_release = version_release / 10;
        version_build = zint_version % 10;
        printf( "Zint version %d.%d.%d.%d\n", version_major, version_minor, version_release, version_build);
    } else {
        /* This is a stable release */
        printf( "Zint version %d.%d.%d\n", version_major, version_minor, version_release);
    }
    
    printf( "Encode input data in a barcode and save as BMP/EMF/EPS/GIF/PCX/PNG/SVG/TIF/TXT\n\n"
            "  -b, --barcode=NUMBER  Number of barcode type. Default is 20 (Code 128)\n"
            "  --addongap=NUMBER     Set add-on gap in multiples of X-dimension for UPC/EAN\n"
            "  --batch               Treat each line of input file as a separate data set\n"
            "  --bg=COLOUR           Specify a background colour (in hex)\n"
            "  --binary              Treat input as raw binary data\n"
            "  --bind                Add boundary bars\n"
            "  --bold                Use bold text\n"
            "  --border=NUMBER       Set width of border in multiples of X-dimension\n"
            "  --box                 Add a box around the symbol\n"
            "  --cmyk                Use CMYK colour space in EPS symbols\n"
            "  --cols=NUMBER         Set the number of data columns in symbol\n"
            "  -d, --data=DATA       Set the symbol content\n"
            "  --direct              Send output to stdout\n"
            "  --dmre                Allow Data Matrix Rectangular Extended\n"
            "  --dotsize=NUMBER      Set radius of dots in dotty mode\n"
            "  --dotty               Use dots instead of squares for matrix symbols\n"
            "  --dump                Dump hexadecimal representation to stdout\n"
            "  -e, --ecinos          Display table of ECI character encodings\n"
            "  --eci=NUMBER          Set the ECI (Extended Channel Interpretation) code\n"
            "  --esc                 Process escape characters in input data\n"
            "  --fg=COLOUR           Specify a foreground colour (in hex)\n"
            "  --filetype=TYPE       Set output file type BMP/EMF/EPS/GIF/PCX/PNG/SVG/TIF/TXT\n"
            "  --fullmultibyte       Use multibyte for binary/Latin (QR/Han Xin/Grid Matrix)\n"
            "  --gs1                 Treat input as GS1 compatible data\n"
            "  --gssep               Use separator GS for GS1 (Data Matrix)\n"
            "  -h, --help            Display help message\n"
            "  --height=NUMBER       Set height of symbol in multiples of X-dimension\n"
            "  -i, --input=FILE      Read input data from FILE\n"
            "  --init                Create reader initialisation/programming symbol\n"
            "  --mask=NUMBER         Set masking pattern to use (QR/Han Xin)\n"
            "  --mirror              Use batch data to determine filename\n"
            "  --mode=NUMBER         Set encoding mode (Maxicode/Composite)\n"
            "  --nobackground        Remove background (PNG/SVG/EPS only)\n"
            "  --notext              Remove human readable text\n"
            "  -o, --output=FILE     Send output to FILE. Default is out.png\n"
            "  --primary=STRING      Set structured primary message (Maxicode/Composite)\n"
            "  -r, --reverse         Reverse colours (white on black)\n"
            "  --rotate=NUMBER       Rotate symbol by NUMBER degrees\n"
            "  --rows=NUMBER         Set number of rows (Codablock-F)\n"
            "  --scale=NUMBER        Adjust size of X-dimension\n"
            "  --secure=NUMBER       Set error correction level (ECC)\n"
            "  --separator=NUMBER    Set height of row separator bars (stacked symbologies)\n"
            "  --small               Use small text\n"
            "  --square              Force Data Matrix symbols to be square\n"
            "  -t, --types           Display table of barcode types\n"
            "  --vers=NUMBER         Set symbol version (size, check digits, other options)\n"
            "  -w, --whitesp=NUMBER  Set width of whitespace in multiples of X-dimension\n"
            "  --werror              Convert all warnings into errors\n"
            "  --wzpl                ZPL compatibility mode (allows non-standard symbols)\n"
            );
}

/* Display supported ECI codes */
static void show_eci(void) {
    printf( " 3: ISO-8859-1 - Latin alphabet No. 1 (default)\n"
            " 4: ISO-8859-2 - Latin alphabet No. 2\n"
            " 5: ISO-8859-3 - Latin alphabet No. 3\n"
            " 6: ISO-8859-4 - Latin alphabet No. 4\n"
            " 7: ISO-8859-5 - Latin/Cyrillic alphabet\n"
            " 8: ISO-8859-6 - Latin/Arabic alphabet\n"
            " 9: ISO-8859-7 - Latin/Greek alphabet\n"
            "10: ISO-8859-8 - Latin/Hebrew alphabet\n"
            "11: ISO-8859-9 - Latin alphabet No. 5\n"
            "12: ISO-8859-10 - Latin alphabet No. 6\n"
            "13: ISO-8859-11 - Latin/Thai alphabet\n"
            "15: ISO-8859-13 - Latin alphabet No. 7\n"
            "16: ISO-8859-14 - Latin alphabet No. 8 (Celtic)\n"
            "17: ISO-8859-15 - Latin alphabet No. 9\n"
            "18: ISO-8859-16 - Latin alphabet No. 10\n"
            "20: ** Shift-JIS (JISX 0208 amd JISX 0201)\n"
            "21: Windows-1250\n"
            "22: Windows-1251\n"
            "23: Windows-1252\n"
            "24: Windows-1256\n"
            "25: ** UCS-2 Unicode (High order byte first)\n"
            "26: Unicode (UTF-8)\n"
            "27: ISO-646:1991 7-bit character set\n"
            "28: ** Big5 (Taiwan) Chinese Character Set\n"
            "29: ** GB (PRC) Chinese Character Set\n"
            "30: ** Korean Character Set (KSX1001:1998)\n"
            "** See note in section 4.10 of the manual\n"
    );
}

/* Verifies that a string only uses valid characters */
static int validator(char test_string[], char source[]) {
    unsigned int i, j;

    for (i = 0; i < strlen(source); i++) {
        unsigned int latch = 0;
        for (j = 0; j < strlen(test_string); j++) {
            if (source[i] == test_string[j]) {
                latch = 1;
            }
        }
        if (!(latch)) {
            return ZINT_ERROR_INVALID_DATA;
        }
    }

    return 0;
}

/* Converts an integer value to its hexadecimal character */
static char itoc(int source) {
    if ((source >= 0) && (source <= 9)) {
        return ('0' + source);
    } else {
        return ('A' + (source - 10));
    }
}

/* Converts upper case characters to lower case in a string source[] */
static void to_lower(char source[]) {
    int i, src_len = strlen(source);

    for (i = 0; i < src_len; i++) {
        if ((source[i] >= 'A') && (source[i] <= 'Z')) {
            source[i] = (source[i] - 'A') + 'a';
        }
    }
}

static char *filetypes[] = {
    "bmp", "emf", "eps", "gif", "pcx", "png", "svg", "tif", "txt", // TODO: Determine if PNG available
};

/* Whether `filetype` supported by Zint */
static int supported_filetype(char *filetype) {
    char lc_filetype[4] = {0};
    int i;

    strncpy(lc_filetype, filetype, 3);
    to_lower(lc_filetype);

    for (i = 0; i < (int) ARRAY_SIZE(filetypes); i++) {
        if (strcmp(lc_filetype, filetypes[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Get file extension, excluding those of 4 or more letters */
static char *get_extension(char *file) {
    char *dot;

    dot = strrchr(file, '.');
    if (dot && strlen(file) - (dot - file) <= 4) { /* Only recognize up to 3 letter extensions */
        return dot + 1;
    }
    return NULL;
}

/* Set extension of `file` to `filetype`, replacing existing extension if any.
 * Does nothing if file already has `filetype` extension */
static void set_extension(char *file, char *filetype) {
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
    strcat(file, filetype);
}

static char *raster_filetypes[] = {
    "bmp", "gif", "pcx", "png", "tif", // TODO: Determine if PNG available
};

static int is_raster(char *filetype) {
    int i;
    char lc_filetype[4] = {0};

    strcpy(lc_filetype, filetype);
    to_lower(lc_filetype);

    for (i = 0; i < (int) ARRAY_SIZE(raster_filetypes); i++) {
        if (strcmp(lc_filetype, raster_filetypes[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int batch_process(struct zint_symbol *symbol, char *filename, int mirror_mode, char *filetype, int rotate_angle) {
    FILE *file;
    unsigned char buffer[7828] = {0}; // 7828 maximum HanXin input
    unsigned char character = 0;
    int posn = 0, error_number = 0, line_count = 1;
    char output_file[256];
    char number[12], reverse_number[12];
    int inpos, local_line_count;
    char format_string[256], reversed_string[256], format_char;
    int format_len, i, o;
    char adjusted[2] = {0};

    if (symbol->outfile[0] == '\0') {
        strcpy(format_string, "~~~~~.");
        strcat(format_string, filetype);
    } else {
        strcpy(format_string, symbol->outfile);
        set_extension(format_string, filetype);
    }

    if (!strcmp(filename, "-")) {
        file = stdin;
    } else {
        file = fopen(filename, "rb");
        if (!file) {
            strcpy(symbol->errtxt, "102: Unable to read input file");
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
            if (posn > 0 && buffer[posn - 1] == '\r') {
                /* CR+LF - assume Windows formatting and remove CR */
                posn--;
                buffer[posn] = '\0';
            }

            if (mirror_mode == 0) {
                inpos = 0;
                local_line_count = line_count;
                memset(number, 0, sizeof(number));
                memset(reverse_number, 0, sizeof(reverse_number));
                memset(reversed_string, 0, sizeof(reversed_string));
                memset(output_file, 0, sizeof(output_file));
                do {
                    number[inpos] = itoc(local_line_count % 10);
                    local_line_count /= 10;
                    inpos++;
                } while (local_line_count > 0);
                number[inpos] = '\0';

                for (i = 0; i < inpos; i++) {
                    reverse_number[i] = number[inpos - i - 1];
                }

                format_len = strlen(format_string);
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
                                adjusted[0] = '*';
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
                o = 0;
                do {
                    if (buffer[i] < 0x20) {
                        output_file[o] = '_';
                    } else {
                        switch (buffer[i]) {
                            case 0x21: // !
                            case 0x22: // "
                            case 0x2a: // *
                            case 0x2f: // /
                            case 0x3a: // :
                            case 0x3c: // <
                            case 0x3e: // >
                            case 0x3f: // ?
                            case 0x5c: // Backslash
                            case 0x7c: // |
                            case 0x7f: // DEL
                                output_file[o] = '_';
                                break;
                            default:
                                output_file[o] = buffer[i];
                                break;
                        }
                    }

                    // Skip escape characters
                    if ((buffer[i] == 0x5c) && (symbol->input_mode & ESCAPE_MODE)) {
                        i++;
                        if (buffer[i] == 'x') {
                            i += 2;
                        }
                    }
                    i++;
                    o++;
                } while (i < posn && o < 251);

                /* Add file extension */
                output_file[o] = '.';
                output_file[o + 1] = '\0';

                strcat(output_file, filetype);
            }

            strcpy(symbol->outfile, output_file);
            error_number = ZBarcode_Encode_and_Print(symbol, buffer, posn, rotate_angle);
            if (error_number != 0) {
                fprintf(stderr, "On line %d: %s\n", line_count, symbol->errtxt);
                fflush(stderr);
            }
            ZBarcode_Clear(symbol);
            memset(buffer, 0, sizeof(buffer));
            posn = 0;
            line_count++;
        } else {
            buffer[posn] = character;
            posn++;
        }
        if (posn >= (int) sizeof(buffer)) {
            fprintf(stderr, "On line %d: Error 103: Input data too long\n", line_count);
            fflush(stderr);
            do {
                character = fgetc(file);
            } while ((!feof(file)) && (character != '\n'));
        }
    } while ((!feof(file)) && (line_count < 2000000000));

    if (character != '\n') {
        fprintf(stderr, "Warning 104: No newline at end of file\n");
        fflush(stderr);
    }

    fclose(file);
    return error_number;
}

typedef struct { char *arg; int opt; } arg_opt;

int main(int argc, char **argv) {
    struct zint_symbol *my_symbol;
    int error_number = 0;
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
    char filetype[4] = {0};
    int i;
    int ret;
    char *outfile_extension;
    int data_arg_num = 0;
#ifndef _MSC_VER
    arg_opt arg_opts[argc];
#else
    arg_opt *arg_opts = (arg_opt *) _alloca(argc * sizeof(arg_opt));
#endif

    my_symbol = ZBarcode_Create();
    my_symbol->input_mode = UNICODE_MODE;

    if (argc == 1) {
        usage();
        exit(1);
    }

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"addongap", 1, 0, 0},
            {"barcode", 1, 0, 'b'},
            {"batch", 0, 0, 0},
            {"binary", 0, 0, 0},
            {"bg", 1, 0, 0},
            {"bind", 0, 0, 0},
            {"bold", 0, 0, 0},
            {"border", 1, 0, 0},
            {"box", 0, 0, 0},
            {"cmyk", 0, 0, 0},
            {"cols", 1, 0, 0},
            {"data", 1, 0, 'd'},
            {"direct", 0, 0, 0},
            {"dmre", 0, 0, 0},
            {"dotsize", 1, 0, 0},
            {"dotty", 0, 0, 0},
            {"dump", 0, 0, 0},
            {"eci", 1, 0, 0},
            {"ecinos", 0, 0, 'e'},
            {"esc", 0, 0, 0},
            {"fg", 1, 0, 0},
            {"filetype", 1, 0, 0},
            {"fontsize", 1, 0, 0},
            {"fullmultibyte", 0, 0, 0},
            {"gs1", 0, 0, 0},
            {"gssep", 0, 0, 0},
            {"height", 1, 0, 0},
            {"help", 0, 0, 'h'},
            {"init", 0, 0, 0},
            {"input", 1, 0, 'i'},
            {"mirror", 0, 0, 0},
            {"mask", 1, 0, 0},
            {"mode", 1, 0, 0},
            {"nobackground", 0, 0, 0},
            {"notext", 0, 0, 0},
            {"output", 1, 0, 'o'},
            {"primary", 1, 0, 0},
            {"reverse", 0, 0, 'r'},
            {"rotate", 1, 0, 0},
            {"rows", 1, 0, 0},
            {"scale", 1, 0, 0},
            {"secure", 1, 0, 0},
            {"separator", 1, 0, 0},
            {"small", 0, 0, 0},
            {"square", 0, 0, 0},
            {"types", 0, 0, 't'},
            {"verbose", 0, 0, 0}, // Currently undocumented, output some debug info
            {"vers", 1, 0, 0},
            {"whitesp", 1, 0, 'w'},
            {0, 0, 0, 0}
        };
        int c = getopt_long(argc, argv, "b:d:ehi:o:rtw:", long_options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                if (!strcmp(long_options[option_index].name, "bind")) {
                    my_symbol->output_options += BARCODE_BIND;
                }
                if (!strcmp(long_options[option_index].name, "box")) {
                    my_symbol->output_options += BARCODE_BOX;
                }
                if (!strcmp(long_options[option_index].name, "init")) {
                    my_symbol->output_options += READER_INIT;
                }
                if (!strcmp(long_options[option_index].name, "small")) {
                    my_symbol->output_options += SMALL_TEXT;
                }
                if (!strcmp(long_options[option_index].name, "bold")) {
                    my_symbol->output_options += BOLD_TEXT;
                }
                if (!strcmp(long_options[option_index].name, "cmyk")) {
                    my_symbol->output_options += CMYK_COLOUR;
                }
                if (!strcmp(long_options[option_index].name, "dotty")) {
                    my_symbol->output_options += BARCODE_DOTTY_MODE;
                }
                if (!strcmp(long_options[option_index].name, "gssep")) {
                    my_symbol->output_options += GS1_GS_SEPARATOR;
                }
                if (!strcmp(long_options[option_index].name, "direct")) {
                    my_symbol->output_options += BARCODE_STDOUT;
                }
                if (!strcmp(long_options[option_index].name, "dump")) {
                    my_symbol->output_options += BARCODE_STDOUT;
                    strcpy(my_symbol->outfile, "dummy.txt");
                }
                if (!strcmp(long_options[option_index].name, "gs1")) {
                    my_symbol->input_mode = (my_symbol->input_mode & ~0x07) | GS1_MODE;
                }
                if (!strcmp(long_options[option_index].name, "binary")) {
                    my_symbol->input_mode = (my_symbol->input_mode & ~0x07) | DATA_MODE;
                }
                if (!strcmp(long_options[option_index].name, "fg")) {
                    strncpy(my_symbol->fgcolour, optarg, 9);
                }
                if (!strcmp(long_options[option_index].name, "bg")) {
                    strncpy(my_symbol->bgcolour, optarg, 9);
                }
                if (!strcmp(long_options[option_index].name, "fullmultibyte")) {
                    fullmultibyte = 1;
                }
                if (!strcmp(long_options[option_index].name, "mask")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 148: Invalid mask value\n");
                        exit(1);
                    }
                    mask = atoi(optarg) + 1;
                    if (mask <= 0 || mask > 8) {
                        /* Values >= 1 and <= 8 (i.e. mask pattern >= 0 and <= 7) only permitted */
                        fprintf(stderr, "Warning 147: Invalid mask value\n");
                        fflush(stderr);
                        mask = 0;
                    }
                }
                if (!strcmp(long_options[option_index].name, "notext")) {
                    my_symbol->show_hrt = 0;
                }
                if (!strcmp(long_options[option_index].name, "square")) {
                    my_symbol->option_3 = DM_SQUARE;
                }
                /* Square overwrites DMRE */
                if (!strcmp(long_options[option_index].name, "dmre")
                    && my_symbol->option_3 != DM_SQUARE) {
                    my_symbol->option_3 = DM_DMRE;
                }
                if (!strcmp(long_options[option_index].name, "scale")) {
                    my_symbol->scale = (float) (atof(optarg));
                    if (my_symbol->scale < 0.01f) {
                        /* Zero and negative values are not permitted */
                        fprintf(stderr, "Warning 105: Invalid scale value\n");
                        fflush(stderr);
                        my_symbol->scale = 1.0f;
                    }
                }
                if (!strcmp(long_options[option_index].name, "separator")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 128: Invalid separator value\n");
                        exit(1);
                    }
                    separator = atoi(optarg);
                    if (separator < 0 || separator > 4) {
                        /* Negative and greater than 4 values are not permitted */
                        fprintf(stderr, "Warning 127: Invalid separator value\n");
                        fflush(stderr);
                        separator = 0;
                    }
                }
                if (!strcmp(long_options[option_index].name, "addongap")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 139: Invalid add-on gap value\n");
                        exit(1);
                    }
                    addon_gap = atoi(optarg);
                    if (addon_gap < 7 || addon_gap > 12) {
                        fprintf(stderr, "Warning 140: Invalid add-on gap value\n");
                        fflush(stderr);
                        addon_gap = 0;
                    }
                }
                if (!strcmp(long_options[option_index].name, "dotsize")) {
                    my_symbol->dot_size = (float) (atof(optarg));
                    if (my_symbol->dot_size < 0.01f) {
                        /* Zero and negative values are not permitted */
                        fprintf(stderr, "Warning 106: Invalid dot radius value\n");
                        fflush(stderr);
                        my_symbol->dot_size = 4.0f / 5.0f;
                    }
                }
                if (!strcmp(long_options[option_index].name, "border")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 107: Invalid border width value\n");
                        exit(1);
                    }
                    if ((atoi(optarg) >= 0) && (atoi(optarg) <= 1000)) {
                        my_symbol->border_width = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 108: Border width out of range\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "height")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 109: Invalid symbol height value\n");
                        exit(1);
                    }
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 1000)) {
                        my_symbol->height = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 110: Symbol height out of range\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "cols")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 131: Invalid columns value\n");
                        exit(1);
                    }
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 67)) {
                        my_symbol->option_2 = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 111: Number of columns out of range\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "rows")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 132: Invalid rows value\n");
                        exit(1);
                    }
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 44)) {
                        my_symbol->option_1 = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 112: Number of rows out of range\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "vers")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 133: Invalid version value\n");
                        exit(1);
                    }
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 84)) {
                        my_symbol->option_2 = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 113: Invalid version\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "secure")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 134: Invalid ECC value\n");
                        exit(1);
                    }
                    if ((atoi(optarg) >= 0) && (atoi(optarg) <= 8)) {
                        my_symbol->option_1 = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 114: ECC level out of range\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "primary")) {
                    if (strlen(optarg) <= 127) {
                        strcpy(my_symbol->primary, optarg);
                    } else {
                        fprintf(stderr, "Error 115: Primary data string too long\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "mode")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 136: Invalid mode value\n");
                        exit(1);
                    }
                    if ((atoi(optarg) >= 0) && (atoi(optarg) <= 6)) {
                        my_symbol->option_1 = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 116: Invalid mode\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "rotate")) {
                    /* Only certain inputs allowed */
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 117: Invalid rotation value\n");
                        exit(1);
                    }
                    switch (atoi(optarg)) {
                        case 90: rotate_angle = 90;
                            break;
                        case 180: rotate_angle = 180;
                            break;
                        case 270: rotate_angle = 270;
                            break;
                        case 0: rotate_angle = 0;
                            break;
                        default:
                            fprintf(stderr, "Warning 137: Invalid rotation parameter\n");
                            fflush(stderr);
                            break;
                    }
                }
                if (!strcmp(long_options[option_index].name, "batch")) {
                    if (data_cnt == 0) {
                        /* Switch to batch processing mode */
                        batch_mode = 1;
                    } else {
                        fprintf(stderr, "Warning 141: Can't use batch mode if data given, ignoring\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "mirror")) {
                    /* Use filenames which reflect content */
                    mirror_mode = 1;
                }
                if (!strcmp(long_options[option_index].name, "filetype")) {
                    /* Select the type of output file */
                    if (!supported_filetype(optarg)) {
                        fprintf(stderr, "Warning 142: File type '%s' not supported, ignoring\n", optarg);
                        fflush(stderr);
                    } else {
                        strncpy(filetype, optarg, (size_t) 3);
                    }
                }
                if (!strcmp(long_options[option_index].name, "eci")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 138: Invalid ECI value\n");
                        exit(1);
                    }
                    if ((atoi(optarg) >= 0) && (atoi(optarg) <= 999999)) {
                        my_symbol->eci = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 118: Invalid ECI code\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "esc")) {
                    my_symbol->input_mode |= ESCAPE_MODE;
                }
                if (!strcmp(long_options[option_index].name, "verbose")) {
                    my_symbol->debug = 1;
                }
                if (!strcmp(long_options[option_index].name, "fontsize")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 130: Invalid font size value\n");
                        exit(1);
                    }
                    if ((atoi(optarg) >= 0) && (atoi(optarg) <= 100)) {
                        my_symbol->fontsize = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 126: Invalid font size\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "nobackground")) {
                    strcpy(my_symbol->bgcolour, "ffffff00");
                }
                if (!strcmp(long_options[option_index].name, "werror")) {
                    my_symbol->warn_level = WARN_FAIL_ALL;
                }
                if (!strcmp(long_options[option_index].name, "wzpl")) {
                    my_symbol->warn_level = WARN_ZPL_COMPAT;
                }
                break;

            case 'h':
                usage();
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
                error_number = validator(NESET, optarg);
                if (error_number == ZINT_ERROR_INVALID_DATA) {
                    fprintf(stderr, "Error 119: Invalid barcode type '%s'\n", optarg);
                    exit(1);
                }
                my_symbol->symbology = atoi(optarg);
                break;

            case 'w':
                error_number = validator(NESET, optarg);
                if (error_number == ZINT_ERROR_INVALID_DATA) {
                    fprintf(stderr, "Error 120: Invalid whitespace value '%s'\n", optarg);
                    exit(1);
                }
                if ((atoi(optarg) >= 0) && (atoi(optarg) <= 1000)) {
                    my_symbol->whitespace_width = atoi(optarg);
                } else {
                    fprintf(stderr, "Warning 121: Whitespace value out of range\n");
                    fflush(stderr);
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
                }
                break;

            case 'i': /* Take data from file */
                if (batch_mode == 0 || input_cnt == 0) {
                    arg_opts[data_arg_num].arg = optarg;
                    arg_opts[data_arg_num].opt = c;
                    data_arg_num++;
                    input_cnt++;
                } else {
                    fprintf(stderr, "Warning 143: Can only define one input file in batch mode, ignoring '%s'\n", optarg);
                    fflush(stderr);
                }
                break;

            case 'o':
                strncpy(my_symbol->outfile, optarg, 255);
                break;

            case 'r':
                strcpy(my_symbol->fgcolour, "ffffff");
                strcpy(my_symbol->bgcolour, "000000");
                break;

            case '?':
                break;

            default:
                fprintf(stderr, "Error 123: ?? getopt error 0%o\n", c);
                fflush(stderr);
                break;
        }
    }

    if (optind < argc) {
        fprintf(stderr, "Error 125: Invalid option\n");
        while (optind < argc)
            fprintf(stderr, "%s", argv[optind++]);
        fprintf(stderr, "\n");
        fflush(stderr);
    }

    if (data_arg_num) {
        unsigned int cap = ZBarcode_Cap(my_symbol->symbology, ZINT_CAP_STACKABLE | ZINT_CAP_EXTENDABLE |
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

        if (batch_mode) {
            /* Take each line of text as a separate data set */
            if (data_arg_num > 1) {
                fprintf(stderr, "Warning 144: Processing first input file '%s' only\n", arg_opts[0].arg);
                fflush(stderr);
            }
            if (filetype[0] == '\0') {
                outfile_extension = get_extension(my_symbol->outfile);
                // TODO: Determine if PNG available
                strcpy(filetype, outfile_extension && supported_filetype(outfile_extension) ? outfile_extension : "png");
            }
            if (my_symbol->scale < 0.5f && is_raster(filetype)) {
                fprintf(stderr, "Warning 145: Scaling less than 0.5 will be set to 0.5 for '%s' output\n", filetype);
                fflush(stderr);
            }
            error_number = batch_process(my_symbol, arg_opts[0].arg, mirror_mode, filetype, rotate_angle);
            if (error_number != 0) {
                fprintf(stderr, "%s\n", my_symbol->errtxt);
                fflush(stderr);
            }
        } else {
            if (*filetype != '\0') {
                set_extension(my_symbol->outfile, filetype);
            }
            if (my_symbol->scale < 0.5f && is_raster(get_extension(my_symbol->outfile))) {
                fprintf(stderr, "Warning 146: Scaling less than 0.5 will be set to 0.5 for '%s' output\n", get_extension(my_symbol->outfile));
                fflush(stderr);
            }
            for (i = 0; i < data_arg_num; i++) {
                if (arg_opts[i].opt == 'd') {
                    ret = ZBarcode_Encode(my_symbol, (unsigned char *) arg_opts[i].arg, strlen(arg_opts[i].arg));
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
    }

    ZBarcode_Delete(my_symbol);

    return error_number;
}
