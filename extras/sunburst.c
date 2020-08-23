/* 
 * sunburst.c
 * 
 * Many encoding schemes were put forward when the UPC system was being considered,
 * and this code encodes numeric data according to one of the more interesting looking
 * varieties. The system proposed by Charecogn Systems Inc. doesn't seem to have had
 * an official name, but "sunburst" seems appropriate from its appearance. The idea was
 * that the symbol would be read by a sensor mounted on a rotating head.
 * 
 * This code takes numeric data and produces an image as an SVG.
 * 
 * More details in US Patent 3,636,317, Filed April 28th 1969.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void print_head(char upc_number[]) {
    printf("<?xml version=\"1.0\" standalone=\"no\"?>\n");
    printf("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n");
    printf("   \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
    printf("<svg width=\"100\" height=\"100\" version=\"1.1\"\n");
    printf("   xmlns=\"http://www.w3.org/2000/svg\">\n");
    printf("    <desc>Sunburst %s</desc>\n\n", upc_number);
    printf("    <g id=\"sunburst\" fill = \"#000000\">\n");
    printf("        <rect x=\"0\" y=\"0\" width=\"100\" height=\"100\" fill=\"#ffffff\" />\n");
}

void print_foot() {
    printf("    </g>\n");
    printf("</svg>\n");
}

static const char *torrey[11] = {
    "01010101", "10010101", "01100101", "10100101", "01010110", "10010110",
    "01100110", "10101010", "01011001", "10011001", "01001101"
            // Two "surplus" codes were also defined as "01011010" and "10110010"
            // In these codes 0 is dark and 1 is light
};

int main(int argc, char** argv) {

    int in_length;
    char upc_number[12];
    char binary[100];
    int i;
    int posn;
    int left, right;
    double ax, ay, bx, by, cx, cy, dx, dy;
    
    if (argc != 2) {
        /* Only command line input should be the number to encode */
        printf("Usage: sunburst {number}\n");
        printf("Where {number} is the number to be encoded, up to 11 digits\n");
        return 0;
    } else {
        in_length = strlen(argv[1]);
        if (in_length > 11) {
            /* Check maximum length */
            printf("Input data too long\n");
            return 0;
        } else {
            /* Add padding if needed */
            strcpy(upc_number, "");
            for(i = in_length; i < 11; i++) {
                strcat(upc_number, "0");
            }
            strcat(upc_number, argv[1]);
        }
    }
    
    /* Check input is numeric */
    for (i = 0; i < 11; i++) {
        if ((upc_number[i] < '0') || (upc_number[i] > '9')) {
            printf("Invalid character(s) in input data\n");
            return 0;
        }
    }
    
    strcpy(binary, torrey[10]); // Start
    
    for (i = 0; i < 11; i++) {
        strcat(binary, torrey[upc_number[i] - '0']);
    }
    
    print_head(upc_number);
    
    posn = 0;
    
    do {
        if (binary[posn] == '0') {
            for (i = 0; binary[posn + i] == '0'; i++);
            left = posn;
            right = posn + i;
            
            ax = 50.0 + (18.72 * cos(0.06545 * left - 1.5708));
            ay = 50.0 + (18.72 * sin(0.06545 * left - 1.5708));
            bx = 50.0 + (50.0 * cos(0.06545 * left - 1.5708));
            by = 50.0 + (50.0 * sin(0.06545 * left - 1.5708));
            cx = 50.0 + (50.0 * cos(0.06545 * right - 1.5708));
            cy = 50.0 + (50.0 * sin(0.06545 * right - 1.5708));
            dx = 50.0 + (18.72 * cos(0.06545 * right - 1.5708));
            dy = 50.0 + (18.72 * sin(0.06545 * right - 1.5708));
            
            printf("        <path d=\"M %.2f %.2f A 50.00 50.00 0 0 1 %.2f %.2f L %.2f %.2f A 18.72 18.72 0 0 0 %.2f %.2f Z\" />\n", 
                     bx, by, cx, cy, dx, dy, ax, ay);
            
            posn += i;
        } else {
            posn++;
        }
    } while (posn < 96);
    
    print_foot();
    
    return (EXIT_SUCCESS);
}
