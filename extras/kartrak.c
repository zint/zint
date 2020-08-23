/* 
 * kartrak.c
 * 
 * This code generates KarTrak codes as were previously used in the rail industry of the US
 * (description below). Output is as an SVG file. This system is now obsolete but perhaps
 * this will be of interest to model railway enthusiasts.
 * 
 * "KarTrak, sometimes KarTrak ACI (for Automatic Car Identification) is a colored
 * bar code system designed to automatically identify rail cars and other rolling stock.
 * KarTrak was made a requirement in North America, but technical problems led to
 * abandonment of the system in the late 1970s."
 * 
 * https://en.wikipedia.org/wiki/KarTrak
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_head(char car_number[], double bordersize) {
    printf("<?xml version=\"1.0\" standalone=\"no\"?>\n");
    printf("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n");
    printf("   \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
    printf("<svg width=\"%.2fin\" height=\"%.2fin\" version=\"1.1\"\n", 5.75 + (2 * bordersize), 17.5 + (2 * bordersize));
    printf("   xmlns=\"http://www.w3.org/2000/svg\">\n");
    printf("    <desc>KarTrak ACI %s</desc>\n\n", car_number);
    printf("    <g id=\"kartrak\" fill = \"#000000\">\n");
    printf("        <rect x=\"0in\" y=\"0in\" width=\"%.2fin\" height=\"%.2fin\" fill=\"#000000\" />\n", 5.75 + (2 * bordersize), 17.5 + (2 * bordersize));
}

void print_foot() {
    printf("    </g>\n");
    printf("</svg>\n");
}

void print_check(double x, double y) {
    /* Print checkerboard */
    int w, h;
    
    for (h = 0; h < 6; h++) {
        for (w = 0; w < 69; w++) {
            if (((w + h) % 2) == 0) {
                printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"0.08in\" height=\"0.08in\" fill=\"#ffffff\" />\n", x + (0.083 * w), y + (0.083 * h));
            }
        }
    }
}

void hrt(double x, double y, char c[]) {
    /* Add text to the left */
    printf("        <text x=\"%.2fin\" y=\"%.2fin\" font-family=\"Verdana\" font-size=\"25\">%s</text>\n", x + 0.2, y + 0.9, c);
}

void back_square(double x, double y) {
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"0.17in\" height=\"0.25in\" fill=\"#ffffff\" />\n", x + 0.2, y + 0.7);
}

void print_zero(double x, double y) {
    print_check(x, y);
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"5.75in\" height=\"0.5in\" fill=\"#0000e0\" />\n", x, y + 0.5);
    hrt(x, y, "0");
}

void print_one(double x, double y) {
    print_check(x, y);
    print_check(x, y + 0.5);
    back_square(x, y);
    hrt(x, y, "1");
}

void print_two(double x, double y) {
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"5.75in\" height=\"0.5in\" fill=\"#ef0000\" />\n", x, y);
    print_check(x, y + 0.5);
    back_square(x, y);
    hrt(x, y, "2");
}

void print_three(double x, double y) {
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"5.75in\" height=\"0.5in\" fill=\"#ef0000\" />\n", x, y + 0.5);
    hrt(x, y, "3");
}

void print_four(double x, double y) {
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"5.75in\" height=\"1.00in\" fill=\"#ef0000\" />\n", x, y);
    hrt(x, y, "4");
}

void print_five(double x, double y) {
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"5.75in\" height=\"0.5in\" fill=\"#0000e0\" />\n", x, y + 0.5);
    hrt(x, y, "5");
}

void print_six(double x, double y) {
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"5.75in\" height=\"0.5in\" fill=\"#0000e0\" />\n", x, y);
    print_check(x, y + 0.5);
    back_square(x, y);
    hrt(x, y, "6");
}

void print_seven(double x, double y) {
    print_check(x, y);
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"5.75in\" height=\"0.5in\" fill=\"#ef0000\" />\n", x, y + 0.5);
    hrt(x, y, "7");
}

void print_eight(double x, double y) {
    print_check(x, y + 0.5);
    back_square(x, y);
    hrt(x, y, "8");
}

void print_nine(double x, double y) {
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"5.75in\" height=\"1.00in\" fill=\"#0000e0\" />\n", x, y);
    hrt(x, y, "9");
}

void print_ten(double x, double y) {
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"5.75in\" height=\"0.5in\" fill=\"#ef0000\" />\n", x, y);
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"5.75in\" height=\"0.5in\" fill=\"#0000e0\" />\n", x, y + 0.5);
    hrt(x, y, "10");
}

void print_start(double bordersize) {
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"4.50in\" height=\"0.5in\" fill=\"#0000e0\" />\n", bordersize, 16.5 + bordersize);
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"4.50in\" height=\"0.5in\" fill=\"#ef0000\" />\n", bordersize + 1.25, 17.0 + bordersize);
    hrt(bordersize, 16.0 + bordersize, "START");
}

void print_stop(double bordersize) {
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"4.50in\" height=\"0.5in\" fill=\"#0000e0\" />\n", bordersize + 1.25, 1.35 + bordersize);
    printf("        <rect x=\"%.2fin\" y=\"%.2fin\" width=\"4.50in\" height=\"0.5in\" fill=\"#ef0000\" />\n", bordersize, 1.85 + bordersize);
    hrt(bordersize, 1.35 + bordersize, "STOP");
}

void print_label(int posn, double bordersize, int digit) {
    double y = ((17.5 + bordersize + 0.375) - ((posn + 1) * 1.375));
    
    switch (digit) {
        case 0: print_zero(bordersize, y); break;
        case 1: print_one(bordersize, y); break;
        case 2: print_two(bordersize, y); break;
        case 3: print_three(bordersize, y); break;
        case 4: print_four(bordersize, y); break;
        case 5: print_five(bordersize, y); break;
        case 6: print_six(bordersize, y); break;
        case 7: print_seven(bordersize, y); break;
        case 8: print_eight(bordersize, y); break;
        case 9: print_nine(bordersize, y); break;
        case 10: print_ten(bordersize, y); break;
    }
}

int main(int argc, char** argv) {

    int in_length;
    char car_number[12];
    int i;
    int checksum = 0;
    int checkdigit;
    double bordersize = 3.0;
    
    if (argc != 2) {
        /* Only command line input should be the number to encode */
        printf("Usage: kartrak {number}\n");
        printf("Where {number} is the number to be encoded, up to 10 digits\n");
        return 0;
    } else {
        in_length = strlen(argv[1]);
        if (in_length > 10) {
            /* Check maximum length */
            printf("Input data too long\n");
            return 0;
        } else {
            /* Add padding if needed */
            strcpy(car_number, "");
            for(i = in_length; i < 10; i++) {
                strcat(car_number, "0");
            }
            strcat(car_number, argv[1]);
        }
    }
    
    /* Check input is numeric */
    for (i = 0; i < 10; i++) {
        if ((car_number[i] < '0') || (car_number[i] > '9')) {
            printf("Invalid character(s) in input data\n");
            return 0;
        }
        
        checksum += (car_number[i] - '0') * (1 << i);
    }
    
    /* Calculate check digit */
    checkdigit = checksum % 11;
    
    print_head(car_number, bordersize);
    
    /* Start character */
    print_start(bordersize);
    
    /* Data */
    for (i = 0; i < 10; i++) {
        print_label((i + 1), bordersize, car_number[i] - '0');
    }
    
    /* Stop character */
    print_stop(bordersize);
    
    /* Check digit */
    print_label(12, bordersize, checkdigit);
    
    print_foot();
    
    return (EXIT_SUCCESS);
}
