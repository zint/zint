/* main.c - Command line handling routines for Zint */

/*
    libzint - the open source barcode library
    Copyright (C) 2008-2016 Robin Stuart <rstuart114@gmail.com>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <getopt.h>
#include <zint.h>
#else
#include "getopt.h"
#include "zint.h"
#endif
#define NESET "0123456789"

#ifdef _MSC_VER
#include <malloc.h>
#endif

/* Print list of supported symbologies */
void types(void) {
    printf( " 1: Code 11           51: Pharma One-Track         90: KIX Code\n"
            " 2: Standard 2of5     52: PZN                      92: Aztec Code\n"
            " 3: Interleaved 2of5  53: Pharma Two-Track         93: DAFT Code\n"
            " 4: IATA 2of5         55: PDF417                   97: Micro QR Code\n"
            " 6: Data Logic        56: PDF417 Trunc             98: HIBC Code 128\n"
            " 7: Industrial 2of5   57: Maxicode                 99: HIBC Code 39\n"
            " 8: Code 39           58: QR Code                 102: HIBC Data Matrix\n"
            " 9: Extended Code 39  60: Code 128-B              104: HIBC QR Code\n"
            "13: EAN               63: AP Standard Customer    106: HIBC PDF417\n"
            "16: GS1-128           66: AP Reply Paid           108: HIBC MicroPDF417\n"
            "18: Codabar           67: AP Routing              112: HIBC Aztec Code\n"
            "20: Code 128          68: AP Redirection          128: Aztec Runes\n"
            "21: Leitcode          69: ISBN                    129: Code 23\n"
            "22: Identcode         70: RM4SCC                  130: Comp EAN\n"
            "23: Code 16k          71: Data Matrix             131: Comp GS1-128\n"
            "24: Code 49           72: EAN-14                  132: Comp DataBar Omni\n"
            "25: Code 93           75: NVE-18                  133: Comp DataBar Ltd\n"
            "28: Flattermarken     76: Japanese Post           134: Comp DataBar ExpOm\n"
            "29: GS1 DataBar Omni  77: Korea Post              135: Comp UPC-A\n"
            "30: GS1 DataBar Ltd   79: GS1 DataBar Stack       136: Comp UPC-E\n"
            "31: GS1 DataBar ExpOm 80: GS1 DataBar Stack Omni  137: Comp DataBar Stack\n"
            "32: Telepen Alpha     81: GS1 DataBar ESO         138: Comp DataBar Stack Omni\n"
            "34: UPC-A             82: Planet                  139: Comp DataBar ESO\n"
            "37: UPC-E             84: MicroPDF                140: Channel Code\n"
            "40: Postnet           85: USPS OneCode            141: Code One\n"
            "47: MSI Plessey       86: UK Plessey              142: Grid Matrix\n"
            "49: FIM               87: Telepen Numeric\n"
            "50: Logmars           89: ITF-14\n"
            );
}

/* Output usage information */
void usage(void) {
    printf( "Zint version %s\n"
            "Encode input data in a barcode and save as a PNG, EPS or SVG file.\n\n"
            "  -h, --help            Display this message.\n"
            "  -t, --types           Display table of barcode types\n"
            "  -i, --input=FILE      Read data from FILE.\n"
            "  -o, --output=FILE     Write image to FILE. (default is out.png)\n"
            "  -d, --data=DATA       Barcode content.\n"
            "  -b, --barcode=NUMBER  Number of barcode type (default is 20 (=Code128)).\n"
            "  --height=NUMBER       Height of symbol in multiples of x-dimension.\n"
            "  -w, --whitesp=NUMBER  Width of whitespace in multiples of x-dimension.\n"
            "  --border=NUMBER       Width of border in multiples of x-dimension.\n"
            "  --box                 Add a box.\n"
            "  --bind                Add boundary bars.\n"
            "  -r, --reverse         Reverse colours (white on black).\n"
            "  --fg=COLOUR           Specify a foreground colour.\n"
            "  --bg=COLOUR           Specify a background colour.\n"
            "  --scale=NUMBER        Adjust size of output image.\n"
            "  --directpng           Send PNG output to stdout\n"
            "  --directeps           Send EPS output to stdout\n"
            "  --directsvg           Send SVG output to stdout\n"
            "  --dump                Dump hexadecimal representation to stdout\n"
            "  --rotate=NUMBER       Rotate symbol (PNG output only).\n"
            "  --cols=NUMBER         (PDF417) Number of columns.\n"
            "  --vers=NUMBER         (QR Code) Version\n"
            "  --secure=NUMBER       (PDF417 and QR Code) Error correction level.\n"
            "  --primary=STRING      (Maxicode and Composite) Structured primary message.\n"
            "  --mode=NUMBER         (Maxicode and Composite) Set encoding mode.\n"
            "  --gs1                 Treat input as GS1 data\n"
            "  --binary              Treat input as Binary data\n"
            "  --notext              Remove human readable text\n"
            "  --square              Force Data Matrix symbols to be square\n"
            "  --dmre                Allow Data Matrix Rectangular Extended\n"
            "  --init                Create reader initialisation symbol (Code 128)\n"
            "  --smalltext           Use half-size text in PNG images\n"
            "  --batch               Treat each line of input as a separate data set\n"
            , ZINT_VERSION);
}

/* Verifies that a string only uses valid characters */
int validator(char test_string[], char source[]) {
    unsigned int i, j, latch;

    for (i = 0; i < strlen(source); i++) {
        latch = 0;
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

int escape_char_process(struct zint_symbol *my_symbol, unsigned char input_string[], int length) {
    int error_number;
    int i, j;

#ifndef _MSC_VER
    unsigned char escaped_string[length + 1];
#else
    unsigned char* escaped_string = (unsigned char*) _alloca(length + 1);
#endif

    i = 0;
    j = 0;

    do {
        if (input_string[i] == '\\') {
            switch (input_string[i + 1]) {
                case '0': escaped_string[j] = 0x00; /* Null */
                    i += 2;
                    break;
                case 'E': escaped_string[j] = 0x04; /* End of Transmission */
                    i += 2;
                    break;
                case 'a': escaped_string[j] = 0x07; /* Bell */
                    i += 2;
                    break;
                case 'b': escaped_string[j] = 0x08; /* Backspace */
                    i += 2;
                    break;
                case 't': escaped_string[j] = 0x09; /* Horizontal tab */
                    i += 2;
                    break;
                case 'n': escaped_string[j] = 0x0a; /* Line feed */
                    i += 2;
                    break;
                case 'v': escaped_string[j] = 0x0b; /* Vertical tab */
                    i += 2;
                    break;
                case 'f': escaped_string[j] = 0x0c; /* Form feed */
                    i += 2;
                    break;
                case 'r': escaped_string[j] = 0x0d; /* Carriage return */
                    i += 2;
                    break;
                case 'e': escaped_string[j] = 0x1b; /* Escape */
                    i += 2;
                    break;
                case 'G': escaped_string[j] = 0x1d; /* Group Separator */
                    i += 2;
                    break;
                case 'R': escaped_string[j] = 0x1e; /* Record Separator */
                    i += 2;
                    break;
                case '\\': escaped_string[j] = '\\';
                    i += 2;
                    break;
                default: escaped_string[j] = input_string[i];
                    i++;
                    break;
            }
        } else {
            escaped_string[j] = input_string[i];
            i++;
        }
        j++;
    } while (i < length);
    escaped_string[j] = '\0';

    error_number = ZBarcode_Encode(my_symbol, escaped_string, j);

    return error_number;
}

/* Converts an integer value to its hexadecimal character */
static char itoc(int source) { 
    if ((source >= 0) && (source <= 9)) {
        return ('0' + source);
    } else {
        return ('A' + (source - 10));
    }
}

/* Concatinates dest[] with the contents of source[], copying /0 as well */
static void concat(char dest[], char source[]) { 
    unsigned int i, j, n;

    j = strlen(dest);
    n = strlen(source);
    for (i = 0; i <= n; i++) {
        dest[i + j] = source[i];
    }
}

int batch_process(struct zint_symbol *symbol, char *filename) {
    FILE *file;
    unsigned char buffer[7100];
    unsigned char character = 0;
    int posn = 0, error_number = 0, line_count = 1;
    char output_file[127];
    char number[12], reverse_number[12];
    int inpos, local_line_count;
    char format_string[127], reversed_string[127], format_char;
    int format_len, i;
    char adjusted[2];

    memset(buffer, 0, sizeof (unsigned char) * 7100);
    if (symbol->outfile[0] == '\0') {
        strcpy(format_string, "~~~~~.png");
    } else {
        if (strlen(format_string) < 127) {
            strcpy(format_string, symbol->outfile);
        } else {
            strcpy(symbol->errtxt, "Format string too long");
            return ZINT_ERROR_INVALID_DATA;
        }
    }
    memset(adjusted, 0, sizeof (char) * 2);

    if (!strcmp(filename, "-")) {
        file = stdin;
    } else {
        file = fopen(filename, "rb");
        if (!file) {
            strcpy(symbol->errtxt, "Unable to read input file");
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
            if (buffer[posn - 1] == '\r') {
                /* CR+LF - assume Windows formatting and remove CR */
                posn--;
                buffer[posn] = '\0';
            }
            inpos = 0;
            local_line_count = line_count;
            memset(number, 0, sizeof (char) * 12);
            memset(reverse_number, 0, sizeof (char) * 12);
            memset(reversed_string, 0, sizeof (char) * 127);
            memset(output_file, 0, sizeof (char) * 127);
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
                concat(reversed_string, adjusted);
            }

            for (i = 0; i < format_len; i++) {
                output_file[i] = reversed_string[format_len - i - 1];
            }

            strcpy(symbol->outfile, output_file);
            error_number = ZBarcode_Encode_and_Print(symbol, buffer, posn, 0);
            if (error_number != 0) {
                fprintf(stderr, "On line %d: %s\n", line_count, symbol->errtxt);
                fflush(stderr);
            }
            ZBarcode_Clear(symbol);
            memset(buffer, 0, sizeof (unsigned char) * 7100);
            posn = 0;
            line_count++;
        } else {
            buffer[posn] = character;
            posn++;
        }
        if (posn > 7090) {
            fprintf(stderr, "On line %d: Input data too long\n", line_count);
            fflush(stderr);
            do {
                character = fgetc(file);
            } while ((!feof(file)) && (character != '\n'));
        }
    } while ((!feof(file)) && (line_count < 2000000000));

    if (character != '\n') {
        fprintf(stderr, "Warning: No newline at end of file\n");
        fflush(stderr);
    }

    fclose(file);
    return error_number;
}

int main(int argc, char **argv) {
    struct zint_symbol *my_symbol;
    int c;
    int error_number;
    int rotate_angle;
    int generated;
    int batch_mode;

    error_number = 0;
    rotate_angle = 0;
    generated = 0;
    my_symbol = ZBarcode_Create();
    my_symbol->input_mode = UNICODE_MODE;
    batch_mode = 0;

    if (argc == 1) {
        usage();
        exit(1);
    }

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help", 0, 0, 'h'},
            {"types", 0, 0, 't'},
            {"bind", 0, 0, 0},
            {"box", 0, 0, 0},
            {"directeps", 0, 0, 0},
            {"directpng", 0, 0, 0},
            {"directsvg", 0, 0, 0},
            {"dump", 0, 0, 0},
            {"barcode", 1, 0, 'b'},
            {"height", 1, 0, 0},
            {"whitesp", 1, 0, 'w'},
            {"border", 1, 0, 0},
            {"data", 1, 0, 'd'},
            {"output", 1, 0, 'o'},
            {"input", 1, 0, 'i'},
            {"fg", 1, 0, 0},
            {"bg", 1, 0, 0},
            {"cols", 1, 0, 0},
            {"vers", 1, 0, 0},
            {"rotate", 1, 0, 0},
            {"secure", 1, 0, 0},
            {"reverse", 1, 0, 'r'},
            {"mode", 1, 0, 0},
            {"primary", 1, 0, 0},
            {"scale", 1, 0, 0},
            {"gs1", 0, 0, 0},
            {"kanji", 0, 0, 0},
            {"sjis", 0, 0, 0},
            {"binary", 0, 0, 0},
            {"notext", 0, 0, 0},
            {"square", 0, 0, 0},
            {"dmre", 0, 0, 0},
            {"init", 0, 0, 0},
            {"smalltext", 0, 0, 0},
            {"batch", 0, 0, 0},
            {0, 0, 0, 0}
        };
        c = getopt_long(argc, argv, "htb:w:d:o:i:rcmp", long_options, &option_index);
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
                if (!strcmp(long_options[option_index].name, "smalltext")) {
                    my_symbol->output_options += SMALL_TEXT;
                }
                if (!strcmp(long_options[option_index].name, "directeps")) {
                    my_symbol->output_options += BARCODE_STDOUT;
                    strncpy(my_symbol->outfile, "dummy.eps", 10);
                }
                if (!strcmp(long_options[option_index].name, "directpng")) {
                    my_symbol->output_options += BARCODE_STDOUT;
                    strncpy(my_symbol->outfile, "dummy.png", 10);
                }
                if (!strcmp(long_options[option_index].name, "directsvg")) {
                    my_symbol->output_options += BARCODE_STDOUT;
                    strncpy(my_symbol->outfile, "dummy.svg", 10);
                }
                if (!strcmp(long_options[option_index].name, "dump")) {
                    my_symbol->output_options += BARCODE_STDOUT;
                    strncpy(my_symbol->outfile, "dummy.txt", 10);
                }
                if (!strcmp(long_options[option_index].name, "gs1")) {
                    my_symbol->input_mode = GS1_MODE;
                }
                if (!strcmp(long_options[option_index].name, "kanji")) {
                    my_symbol->input_mode = KANJI_MODE;
                }
                if (!strcmp(long_options[option_index].name, "sjis")) {
                    my_symbol->input_mode = SJIS_MODE;
                }
                if (!strcmp(long_options[option_index].name, "binary")) {
                    my_symbol->input_mode = DATA_MODE;
                }
                if (!strcmp(long_options[option_index].name, "fg")) {
                    strncpy(my_symbol->fgcolour, optarg, 7);
                }
                if (!strcmp(long_options[option_index].name, "bg")) {
                    strncpy(my_symbol->bgcolour, optarg, 7);
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
                    if (my_symbol->scale < 0.01) {
                        /* Zero and negative values are not permitted */
                        fprintf(stderr, "Invalid scale value\n");
                        fflush(stderr);
                        my_symbol->scale = 1.0;
                    }
                }
                if (!strcmp(long_options[option_index].name, "border")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Invalid border width\n");
                        exit(1);
                    }
                    if ((atoi(optarg) >= 0) && (atoi(optarg) <= 1000)) {
                        my_symbol->border_width = atoi(optarg);
                    } else {
                        fprintf(stderr, "Border width out of range\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "height")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Invalid symbol height\n");
                        exit(1);
                    }
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 1000)) {
                        my_symbol->height = atoi(optarg);
                    } else {
                        fprintf(stderr, "Symbol height out of range\n");
                        fflush(stderr);
                    }
                }

                if (!strcmp(long_options[option_index].name, "cols")) {
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 30)) {
                        my_symbol->option_2 = atoi(optarg);
                    } else {
                        fprintf(stderr, "Number of columns out of range\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "vers")) {
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 47)) {
                        my_symbol->option_2 = atoi(optarg);
                    } else {
                        fprintf(stderr, "Invalid QR Code version\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "secure")) {
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 8)) {
                        my_symbol->option_1 = atoi(optarg);
                    } else {
                        fprintf(stderr, "ECC level out of range\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "primary")) {
                    if (strlen(optarg) <= 90) {
                        strcpy(my_symbol->primary, optarg);
                    } else {
                        fprintf(stderr, "Primary data string too long");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "mode")) {
                    if ((optarg[0] >= '0') && (optarg[0] <= '6')) {
                        my_symbol->option_1 = optarg[0] - '0';
                    } else {
                        fprintf(stderr, "Invalid mode\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "rotate")) {
                    /* Only certain inputs allowed */
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Invalid rotation parameter\n");
                        exit(1);
                    }
                    switch (atoi(optarg)) {
                        case 90: rotate_angle = 90;
                            break;
                        case 180: rotate_angle = 180;
                            break;
                        case 270: rotate_angle = 270;
                            break;
                        default: rotate_angle = 0;
                            break;
                    }
                }
                if (!strcmp(long_options[option_index].name, "batch")) {
                    /* Switch to batch processing mode */
                    batch_mode = 1;
                }
                break;

            case 'h':
                usage();
                break;

            case 't':
                types();
                break;

            case 'b':
                error_number = validator(NESET, optarg);
                if (error_number == ZINT_ERROR_INVALID_DATA) {
                    fprintf(stderr, "Invalid barcode type\n");
                    exit(1);
                }
                my_symbol->symbology = atoi(optarg);
                break;

            case 'w':
                error_number = validator(NESET, optarg);
                if (error_number == ZINT_ERROR_INVALID_DATA) {
                    fprintf(stderr, "Invalid whitespace value\n");
                    exit(1);
                }
                if ((atoi(optarg) >= 0) && (atoi(optarg) <= 1000)) {
                    my_symbol->whitespace_width = atoi(optarg);
                } else {
                    fprintf(stderr, "Whitespace value out of range");
                    fflush(stderr);
                }
                break;

            case 'd': /* we have some data! */
                if (batch_mode == 0) {
                    error_number = escape_char_process(my_symbol, (unsigned char*) optarg, strlen(optarg));
                    if (error_number == 0) {
                        error_number = ZBarcode_Print(my_symbol, rotate_angle);
                    }
                    generated = 1;
                    if (error_number != 0) {
                        fprintf(stderr, "%s\n", my_symbol->errtxt);
                        fflush(stderr);
                        ZBarcode_Delete(my_symbol);
                        return 1;
                    }
                } else {
                    fprintf(stderr, "Cannot define data in batch mode");
                    fflush(stderr);
                }
                break;

            case 'i': /* Take data from file */
                if (batch_mode == 0) {
                    error_number = ZBarcode_Encode_File(my_symbol, optarg);
                    if (error_number == 0) {
                        error_number = ZBarcode_Print(my_symbol, rotate_angle);
                    }
                    generated = 1;
                    if (error_number != 0) {
                        fprintf(stderr, "%s\n", my_symbol->errtxt);
                        fflush(stderr);
                        ZBarcode_Delete(my_symbol);
                        return 1;
                    }
                } else {
                    /* Take each line of text as a separate data set */
                    error_number = batch_process(my_symbol, optarg);
                    generated = 1;
                    if (error_number != 0) {
                        fprintf(stderr, "%s\n", my_symbol->errtxt);
                        fflush(stderr);
                        ZBarcode_Delete(my_symbol);
                        return 1;
                    }
                }
                break;

            case 'o':
                strncpy(my_symbol->outfile, optarg, 250);
                break;

            case 'r':
                strcpy(my_symbol->fgcolour, "ffffff");
                strcpy(my_symbol->bgcolour, "000000");
                break;

            case '?':
                break;

            default:
                fprintf(stderr, "?? getopt error 0%o\n", c);
                fflush(stderr);
        }
    }

    if (optind < argc) {
        fprintf(stderr, "Invalid option ");
        while (optind < argc)
            fprintf(stderr, "%s", argv[optind++]);
        fprintf(stderr, "\n");
        fflush(stderr);
    }

    if (generated == 0) {
        fprintf(stderr, "error: No data received, no symbol generated\n");
        fflush(stderr);
    }

    ZBarcode_Delete(my_symbol);

    return error_number;
}
