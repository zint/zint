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
    printf( " 1: Code 11           51: Pharma One-Track         92: Aztec Code\n"
            " 2: Standard 2of5     52: PZN                      93: DAFT Code\n"
            " 3: Interleaved 2of5  53: Pharma Two-Track         97: Micro QR Code\n"
            " 4: IATA 2of5         55: PDF417                   98: HIBC Code 128\n"
            " 6: Data Logic        56: PDF417 Trunc             99: HIBC Code 39\n"
            " 7: Industrial 2of5   57: Maxicode                102: HIBC Data Matrix\n"
            " 8: Code 39           58: QR Code                 104: HIBC QR Code\n"
            " 9: Extended Code 39  60: Code 128-B              106: HIBC PDF417\n"
            "13: EAN               63: AP Standard Customer    108: HIBC MicroPDF417\n"
            "14: EAN + Check       66: AP Reply Paid           110: HIBC Codablock-F\n"
            "16: GS1-128           67: AP Routing              112: HIBC Aztec Code\n"
            "18: Codabar           68: AP Redirection          115: DotCode\n"
            "20: Code 128          69: ISBN                    116: Han Xin Code\n"
            "21: Leitcode          70: RM4SCC                  121: RM Mailmark\n"
            "22: Identcode         71: Data Matrix             128: Aztec Runes\n"
            "23: Code 16k          72: EAN-14                  129: Code 32\n"
            "24: Code 49           73: VIN (North America)     130: Comp EAN\n"
            "25: Code 93           74: Codablock-F             131: Comp GS1-128\n"
            "28: Flattermarken     75: NVE-18                  132: Comp DataBar Omni\n"
            "29: GS1 DataBar Omni  76: Japanese Post           133: Comp DataBar Ltd\n"
            "30: GS1 DataBar Ltd   77: Korea Post              134: Comp DataBar ExpOm\n"
            "31: GS1 DataBar ExpOm 79: GS1 DataBar Stack       135: Comp UPC-A\n"
            "32: Telepen Alpha     80: GS1 DataBar Stack Omni  136: Comp UPC-E\n"
            "34: UPC-A             81: GS1 DataBar ESO         137: Comp DataBar Stack\n"
            "35: UPC-A + Check     82: Planet                  138: Comp DataBar Stack Omni\n"
            "37: UPC-E             84: MicroPDF                139: Comp DataBar ESO\n"
            "38: UPC-E + Check     85: USPS OneCode            140: Channel Code\n"
            "40: Postnet           86: UK Plessey              141: Code One\n"
            "47: MSI Plessey       87: Telepen Numeric         142: Grid Matrix\n"
            "49: FIM               89: ITF-14                  143: UPNQR\n"
            "50: Logmars           90: KIX Code                145: rMQR\n"
            );
}

/* Output usage information */
void usage(void) {
    printf( "Zint version %d.%d.%d\n"
            "Encode input data in a barcode and save as BMP/EMF/EPS/GIF/PCX/PNG/SVG/TIF\n\n"
            "  -b, --barcode=NUMBER  Number of barcode type (default is 20 (=Code128)).\n"
            "  --batch               Treat each line of input file as a separate data set\n"
            "  --bg=COLOUR           Specify a background colour (in hex)\n"
            "  --binary              Treat input as raw binary data\n"
            "  --bind                Add boundary bars\n"
            "  --bold                Use bold text\n"
            "  --border=NUMBER       Set width of border in multiples of x-dimension\n"
            "  --box                 Add a box around the symbol\n"
            "  --cmyk                Use CMYK colour space in EPS symbols\n"
            "  --cols=NUMBER         Set the number of data columns in symbol\n"
            "  -d, --data=DATA       Set the symbol content\n"
            "  --direct              Send output to stdout\n"
            "  --dotsize=NUMBER      Set radius of dots in dotty mode\n"
            "  --dotty               Use dots instead of squares for matrix symbols\n"
            "  --dmre                Allow Data Matrix Rectangular Extended\n"
            "  --dump                Dump hexadecimal representation to stdout\n"
            "  -e, --ecinos          Display table of ECI character encodings\n"
            "  --eci=NUMBER          Set the ECI mode for raw data\n"
            "  --esc                 Process escape characters in input data\n"
            "  --filetype=TYPE       Set output file type (PNG/EPS/SVG/PNG/EPS/GIF/TXT)\n"
            "  --fg=COLOUR           Specify a foreground colour (in hex)\n"
            "  --gs1                 Treat input as GS1 compatible data\n"
            "  --gssep               Use separator GS for GS1\n"
            "  -h, --help            Display help message\n"
            "  --height=NUMBER       Set height of symbol in multiples of x-dimension\n"
            "  -i, --input=FILE      Read input data from FILE\n"
            "  --init                Create reader initialisation/programming symbol\n"
            "  --mirror              Use batch data to determine filename\n"
            "  --mode=NUMBER         Set encoding mode (Maxicode/Composite)\n"
            "  --notext              Remove human readable text\n"
            "  -o, --output=FILE     Send output to FILE. (default is out.png)\n"
            "  --primary=STRING      Set structured primary message (Maxicode/Composite)\n"
            "  --secure=NUMBER       Set error correction level\n"
            "  --scale=NUMBER        Adjust size of x-dimension\n"
            "  --small               Use half-size text in PNG images\n"
            "  --square              Force Data Matrix symbols to be square\n"
            "  -r, --reverse         Reverse colours (white on black)\n"
            "  --rotate=NUMBER       Rotate symbol by NUMBER degrees (PNG/BMP/PCX)\n"
            "  --rows=NUMBER         Set number of rows (Codablock-F)\n"
            "  -t, --types           Display table of barcode types\n"
            "  --vers=NUMBER         Set symbol version (QR Code/Han Xin)\n"
            "  -w, --whitesp=NUMBER  Set Width of whitespace in multiples of x-dimension\n"
            , ZINT_VERSION_MAJOR, ZINT_VERSION_MINOR, ZINT_VERSION_RELEASE);
}

/* Display supported ECI codes */
void show_eci(void) {
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
            "28: ** Big-5 (Taiwan) Chinese Character Set\n"
            "29: ** GB (PRC) Chinese Character Set\n"
            "30: ** Korean Character Set (KSX1001:1998)\n"
            "** See note in section 4.10 of the manual\n"
    );
}

/* Verifies that a string only uses valid characters */
int validator(char test_string[], char source[]) {
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

/* Concatinates dest[] with the contents of source[], copying /0 as well */
static void concat(char dest[], char source[]) {
    unsigned int i, j, n;

    j = strlen(dest);
    n = strlen(source);
    for (i = 0; i <= n; i++) {
        dest[i + j] = source[i];
    }
}

int batch_process(struct zint_symbol *symbol, char *filename, int mirror_mode, char *filetype, int rotate_angle) {
    FILE *file;
    unsigned char buffer[7100];
    unsigned char character = 0;
    int posn = 0, error_number = 0, line_count = 1;
    char output_file[256];
    char number[12], reverse_number[12];
    int inpos, local_line_count;
    char format_string[127], reversed_string[127], format_char;
    int format_len, i, o;
    char adjusted[2];

    memset(buffer, 0, sizeof (unsigned char) * 7100);
    memset(format_string, 0, sizeof (unsigned char) * 127);
    if (symbol->outfile[0] == '\0') {
        strcpy(format_string, "~~~~~.");
        strcat(format_string, filetype);
    } else {
        if (strlen(format_string) < 127) {
            strcpy(format_string, symbol->outfile);
        } else {
            strcpy(symbol->errtxt, "101: Format string too long");
            return ZINT_ERROR_INVALID_DATA;
        }
    }
    memset(adjusted, 0, sizeof (char) * 2);

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
            if (buffer[posn - 1] == '\r') {
                /* CR+LF - assume Windows formatting and remove CR */
                posn--;
                buffer[posn] = '\0';
            }

            if (mirror_mode == 0) {
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
            } else {
                /* Name the output file from the data being processed */
                i = 0;
                o = 0;
                do {
                //for (i = 0; (i < posn && i < 250); i++) {
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
                } while (i < posn && o < 250);

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
            memset(buffer, 0, sizeof (unsigned char) * 7100);
            posn = 0;
            line_count++;
        } else {
            buffer[posn] = character;
            posn++;
        }
        if (posn > 7090) {
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

int main(int argc, char **argv) {
    struct zint_symbol *my_symbol;
    int error_number;
    int rotate_angle;
    int generated;
    int batch_mode;
    int mirror_mode;
    char filetype[4];
    int i;

    error_number = 0;
    rotate_angle = 0;
    generated = 0;
    my_symbol = ZBarcode_Create();
    my_symbol->input_mode = UNICODE_MODE;
    batch_mode = 0;
    mirror_mode = 0;

    for (i = 0; i < 4; i++) {
        filetype[i] = '\0';
    }

    if (argc == 1) {
        usage();
        exit(1);
    }

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help", 0, 0, 'h'},
            {"types", 0, 0, 't'},
            {"ecinos", 0, 0, 'e'},
            {"bind", 0, 0, 0},
            {"box", 0, 0, 0},
            {"direct", 0, 0, 0},
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
            {"rows", 1, 0, 0},
            {"vers", 1, 0, 0},
            {"rotate", 1, 0, 0},
            {"secure", 1, 0, 0},
            {"reverse", 1, 0, 'r'},
            {"mode", 1, 0, 0},
            {"primary", 1, 0, 0},
            {"scale", 1, 0, 0},
            {"gs1", 0, 0, 0},
            {"gssep", 0, 0, 0},
            {"binary", 0, 0, 0},
            {"notext", 0, 0, 0},
            {"square", 0, 0, 0},
            {"dmre", 0, 0, 0},
            {"init", 0, 0, 0},
            {"small", 0, 0, 0},
            {"bold", 0, 0, 0},
            {"cmyk", 0, 0, 0},
            {"batch", 0, 0, 0},
            {"mirror", 0, 0, 0},
            {"dotty", 0, 0, 0},
            {"dotsize", 1, 0, 0},
            {"eci", 1, 0, 0},
            {"filetype", 1, 0, 0},
            {"esc", 0, 0, 0},
            {"fontsize", 1, 0, 0},
            {"verbose", 0, 0, 0}, // Currently undocumented, output some debug info
            {0, 0, 0, 0}
        };
        int c = getopt_long(argc, argv, "htb:w:d:o:i:rcmpe", long_options, &option_index);
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
                    strncpy(my_symbol->outfile, "dummy.txt", 10);
                }
                if (!strcmp(long_options[option_index].name, "gs1")) {
                    my_symbol->input_mode = GS1_MODE;
                }
                if (!strcmp(long_options[option_index].name, "binary")) {
                    if (my_symbol->input_mode & ESCAPE_MODE) {
                        my_symbol->input_mode = DATA_MODE + ESCAPE_MODE;
                    } else {
                        my_symbol->input_mode = DATA_MODE;
                    }
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
                        fprintf(stderr, "Warning 105: Invalid scale value\n");
                        fflush(stderr);
                        my_symbol->scale = 1.0;
                    }
                }
                if (!strcmp(long_options[option_index].name, "dotsize")) {
                    my_symbol->dot_size = (float) (atof(optarg));
                    if (my_symbol->dot_size < 0.01) {
                        /* Zero and negative values are not permitted */
                        fprintf(stderr, "Warning 106: Invalid dot radius value\n");
                        fflush(stderr);
                        my_symbol->dot_size = 4.0F / 5.0F;
                    }
                }
                if (!strcmp(long_options[option_index].name, "border")) {
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 107: Invalid border width\n");
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
                        fprintf(stderr, "Error 109: Invalid symbol height\n");
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
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 66)) {
                        my_symbol->option_2 = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 111: Number of columns out of range\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "rows")) {
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 44)) {
                        my_symbol->option_1 = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 112: Number of rows out of range\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "vers")) {
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 84)) {
                        my_symbol->option_2 = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 113: Invalid Version\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "secure")) {
                    if ((atoi(optarg) >= 1) && (atoi(optarg) <= 8)) {
                        my_symbol->option_1 = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 114: ECC level out of range\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "primary")) {
                    if (strlen(optarg) <= 90) {
                        strcpy(my_symbol->primary, optarg);
                    } else {
                        fprintf(stderr, "Error 115: Primary data string too long");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "mode")) {
                    if ((optarg[0] >= '0') && (optarg[0] <= '6')) {
                        my_symbol->option_1 = optarg[0] - '0';
                    } else {
                        fprintf(stderr, "Warning 116: Invalid mode\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "rotate")) {
                    /* Only certain inputs allowed */
                    error_number = validator(NESET, optarg);
                    if (error_number == ZINT_ERROR_INVALID_DATA) {
                        fprintf(stderr, "Error 117: Invalid rotation parameter\n");
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
                if (!strcmp(long_options[option_index].name, "mirror")) {
                    /* Use filenames which reflect content */
                    mirror_mode = 1;
                }
                if (!strcmp(long_options[option_index].name, "filetype")) {
                    /* Select the type of output file */
                    strncpy(filetype, optarg, (size_t) 3);
                }
                if (!strcmp(long_options[option_index].name, "eci")) {
                    if ((atoi(optarg) >= 0) && (atoi(optarg) <= 999999)) {
                        my_symbol->eci = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 118: Invalid ECI code\n");
                        fflush(stderr);
                    }
                }
                if (!strcmp(long_options[option_index].name, "esc")) {
                    if (!(my_symbol->input_mode & ESCAPE_MODE)) {
                        my_symbol->input_mode += ESCAPE_MODE;
                    }
                }
                if (!strcmp(long_options[option_index].name, "verbose")) {
                    my_symbol->debug = 1;
                }
                if (!strcmp(long_options[option_index].name, "fontsize")) {
                    if ((atoi(optarg) >= 0) && (atoi(optarg) <= 100)) {
                        my_symbol->fontsize = atoi(optarg);
                    } else {
                        fprintf(stderr, "Warning 125: Invalid font size\n");
                        fflush(stderr);
                    }
                }
                break;

            case 'h':
                usage();
                break;

            case 't':
                types();
                break;

            case 'e':
                show_eci();
                break;

            case 'b':
                error_number = validator(NESET, optarg);
                if (error_number == ZINT_ERROR_INVALID_DATA) {
                    fprintf(stderr, "Error 119: Invalid barcode type\n");
                    exit(1);
                }
                my_symbol->symbology = atoi(optarg);
                break;

            case 'w':
                error_number = validator(NESET, optarg);
                if (error_number == ZINT_ERROR_INVALID_DATA) {
                    fprintf(stderr, "Error 120: Invalid whitespace value\n");
                    exit(1);
                }
                if ((atoi(optarg) >= 0) && (atoi(optarg) <= 1000)) {
                    my_symbol->whitespace_width = atoi(optarg);
                } else {
                    fprintf(stderr, "Warning 121: Whitespace value out of range");
                    fflush(stderr);
                }
                break;

            case 'd': /* we have some data! */
                if (batch_mode == 0) {
                    if (filetype[0] != '\0') {
                        strcat(my_symbol->outfile, ".");
                        strcat(my_symbol->outfile, filetype);
                    }
                    error_number = ZBarcode_Encode(my_symbol, (unsigned char*) optarg, strlen(optarg));
                    generated = 1;
                    if (error_number != 0) {
                            fprintf(stderr, "%s\n", my_symbol->errtxt);
                            fflush(stderr);
                    }
                    if (error_number < 5) {
                        error_number = ZBarcode_Print(my_symbol, rotate_angle);

                        if (error_number != 0) {
                            fprintf(stderr, "%s\n", my_symbol->errtxt);
                            fflush(stderr);
                            ZBarcode_Delete(my_symbol);
                            return 1;
                        }
                    }
                } else {
                    fprintf(stderr, "Warning 122: Can't define data in batch mode");
                    fflush(stderr);
                }
                break;

            case 'i': /* Take data from file */
                if (batch_mode == 0) {
                    error_number = ZBarcode_Encode_File(my_symbol, optarg);
                    generated = 1;
                    if (error_number != 0) {
                        fprintf(stderr, "%s\n", my_symbol->errtxt);
                        fflush(stderr);
                    }
                    if (error_number < 5) {
                        error_number = ZBarcode_Print(my_symbol, rotate_angle);
                        if (error_number != 0) {
                            fprintf(stderr, "%s\n", my_symbol->errtxt);
                            fflush(stderr);
                            ZBarcode_Delete(my_symbol);
                            return 1;
                        }
                    }
                } else {
                    /* Take each line of text as a separate data set */
                    if (filetype[0] == '\0') {
                        strcpy(filetype, "png");
                    }
                    error_number = batch_process(my_symbol, optarg, mirror_mode, filetype, rotate_angle);
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
                fprintf(stderr, "Error 123: ?? getopt error 0%o\n", c);
                fflush(stderr);
        }
    }

    if (optind < argc) {
        fprintf(stderr, "Error 125: Invalid option\n");
        while (optind < argc)
            fprintf(stderr, "%s", argv[optind++]);
        fprintf(stderr, "\n");
        fflush(stderr);
    }

    if (generated == 0) {
        fprintf(stderr, "Warning 124: No data received, no symbol generated\n");
        fflush(stderr);
    }

    ZBarcode_Delete(my_symbol);

    return error_number;
}

