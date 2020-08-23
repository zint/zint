/* 
 * cuecat.c
 * 
 * This code creates barcodes as intended for use with the CueCat scheme (without the
 * "cue" symbol which may still be trademarked). As the system was ultimately not
 * successful this is now simply a curiosity.
 * 
 * "The CueCat, styled :CueCat with a leading colon, is a cat-shaped handheld barcode
 * reader that was released in 2000 by the now-defunct Digital Convergence Corporation.
 * The CueCat enabled a user to open a link to an Internet URL by scanning a barcode —
 * called a "cue" by Digital Convergence — appearing in an article or catalog or on
 * some other printed matter."
 * 
 * For more information:
 * https://linas.org/banned/cuecat/www.fluent-access.com.wtpapers.cuecat.index.html
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static const char *C128Table[107] = {
    /* Code 128 character encodation */
    "212222", "222122", "222221", "121223", "121322", "131222", "122213",
    "122312", "132212", "221213", "221312", "231212", "112232", "122132", "122231", "113222",
    "123122", "123221", "223211", "221132", "221231", "213212", "223112", "312131", "311222",
    "321122", "321221", "312212", "322112", "322211", "212123", "212321", "232121", "111323",
    "131123", "131321", "112313", "132113", "132311", "211313", "231113", "231311", "112133",
    "112331", "132131", "113123", "113321", "133121", "313121", "211331", "231131", "213113",
    "213311", "213131", "311123", "311321", "331121", "312113", "312311", "332111", "314111",
    "221411", "431111", "111224", "111422", "121124", "121421", "141122", "141221", "112214",
    "112412", "122114", "122411", "142112", "142211", "241211", "221114", "413111", "241112",
    "134111", "111242", "121142", "121241", "114212", "124112", "124211", "411212", "421112",
    "421211", "212141", "214121", "412121", "111143", "111341", "131141", "114113", "114311",
    "411113", "411311", "113141", "114131", "311141", "411131", "211412", "211214", "211232",
    "2331112"
};

void print_head(char cat_number[]) {
    printf("<?xml version=\"1.0\" standalone=\"no\"?>\n");
    printf("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n");
    printf("   \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
    printf("<svg width=\"149.60\" height=\"36.00\" version=\"1.1\"\n");
    printf("   xmlns=\"http://www.w3.org/2000/svg\">\n");
    printf("    <desc>CueCat %s</desc>\n\n", cat_number);
    printf("    <g id=\"cuecat\" fill = \"#000000\">\n");
    printf("        <rect x=\"0\" y=\"0\" width=\"149.60\" height=\"36.00\" fill=\"#ffffff\" />\n");
}

void print_cue() {
    /* Just dots and triangles as the :C symbol may still be a trademark */
    printf("        <circle cx=\"7.00\" cy=\"12.50\" r=\"3.50\" fill=\"red\" />\n");
    printf("        <circle cx=\"7.00\" cy=\"23.50\" r=\"3.50\" fill=\"red\" />\n");
    printf("        <polygon points=\"14.00,4.00 14.00,32.00 25.60,32.00\" />\n");
    printf("        <polygon points=\"134.00,4.00 145.60,4.00 145.60,32.00\" />\n");
}

void print_data(char pattern[]) {
    /* Output the lines of the barcode at an attractive 22.5 degree angle */
    double posn = 24;
    int length = strlen(pattern);
    int i;
    
    for (i = 0; i < length; i++) {
        if ((i % 2) == 0) {
            printf("        <polygon points=\"%.2f,4.00 %.2f,4.00 %.2f,32.00 %.2f,32.00\" />\n",
                posn, posn + (pattern[i] - '0'), posn + (pattern[i] - '0') + 11.6, posn + 11.6);
        }
        posn += (pattern[i] - '0');
    }
}

void print_hrt(char cat_number[]) {
    /* Put readable text at the bottom of the symbol */
    char hrt[25];
    int i, j;
    
    printf("        <rect x=\"57.00\" y=\"28.00\" width=\"61.00\" height=\"5.00\" fill=\"white\" />\n");
    
    strcpy(hrt, "C ");
    for (i = 0, j = 2; i < strlen(cat_number); i++) {
        hrt[j] = cat_number[i];
        j++;
        if ((i % 2) != 0) {
            hrt[j] = ' ';
            j++;
        }
    }
    hrt[j] = '\0';
    printf("        <text x=\"58.00\" y=\"32.00\" font-family=\"Verdana\" font-size=\"5\">%s</text>\n", hrt);
}

void print_foot() {
    printf("    </g>\n");
    printf("</svg>\n");
}

int main(int argc, char** argv) {
    int in_length;
    char cat_number[16];
    char pattern[90];
    int cw[7];
    int i;
    int total_sum;
    
    if (argc != 2) {
        /* Only command line input should be the number to encode */
        printf("Usage: cuecat {number}\n");
        printf("Where {number} is the number to be encoded, up to 14 digits\n");
        return 0;
    } else {
        in_length = strlen(argv[1]);
        if (in_length > 14) {
            /* Check maximum length */
            printf("Input data too long\n");
            return 0;
        } else {
            /* Add padding if needed */
            strcpy(cat_number, "");
            for(i = in_length; i < 14; i++) {
                strcat(cat_number, "0");
            }
            strcat(cat_number, argv[1]);
        }
    }
    
    /* Check input is numeric */
    for (i = 0; i < 10; i++) {
        if (!(isdigit(cat_number[i]))) {
            printf("Invalid character(s) in input data\n");
            return 0;
        }
    }
    
    // There is no 'Start' character
    
    strcpy(pattern, "");
    for (i = 0; i < 7; i ++) {
        cw[i] = (cat_number[i * 2] - '0') * 10;
        cw[i] += cat_number[(i * 2) + 1] - '0';
        strcat(pattern, C128Table[cw[i]]);
        
        if (cw[i] >= 96) {
            /* CueCat can't decode number pairs above 95 */
            printf("Invalid input data\n");
            return 0;
        }
    }

    
    /* check digit calculation */
    total_sum = 0;

    for (i = 0; i < 7; i++) {
        if (i > 0) {
            cw[i] *= i;
        }
        total_sum += cw[i];
    }
    strcat(pattern, C128Table[total_sum % 103]);
    
    strcat(pattern, C128Table[106]); // Stop
    
    /* Start ouputting SVG file */
    print_head(cat_number);
    print_cue();
    print_data(pattern);
    print_hrt(cat_number);
    print_foot();
}

