/* 
 * daft.c
 * 
 * This code uses Zint to encode data into a USPS Intelligent
 * Mail symbol, and then converts the output to "DAFT code"
 * which is used by commercial fonts to display this and
 * similar 4-state symbologies.
 * 
 * This code can be compiled with:
 * 
 * gcc -o daft daft.c -lzint
 * 
 * The output characters are:
 * 
 * D = Descender
 * A = Ascender
 * F = Full
 * T = Tracker
 * 
 */

#include <stdio.h>
#include <zint.h>
#include <string.h>
int main(int argc, char **argv)
{
    struct zint_symbol *my_symbol;
    int error = 0;
    int x, y, glyph;
    
    my_symbol = ZBarcode_Create();
    
    my_symbol->symbology = BARCODE_USPS_IMAIL; // Change symbology here
    my_symbol->output_options = OUT_BUFFER_INTERMEDIATE;
    
    error = ZBarcode_Encode(my_symbol, argv[1], strlen(argv[1]));
    if (error != 0)
    {
        printf("%s\n", my_symbol->errtxt);
    }
    if (error >= ZINT_ERROR_TOO_LONG)
    {
        ZBarcode_Delete(my_symbol);
        return 1;
    }

    for (x = 0; x < my_symbol->width; x+= 2) {
        glyph = 0;
        if ((my_symbol->encoded_data[2][x / 8] >> (x % 8)) & 1) {
            glyph += 1;
        }
        if ((my_symbol->encoded_data[0][x / 8] >> (x % 8)) & 1) {
            glyph += 2;
        }
        
        switch (glyph) {
            case 0: printf("T"); break;
            case 1: printf("D"); break;
            case 2: printf("A"); break;
            case 3: printf("F"); break;
        }
        glyph = 0;
    }
    printf("\n");
    
    ZBarcode_Delete(my_symbol);
    return 0;
}
