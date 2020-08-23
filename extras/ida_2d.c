/* 
 * ida_2d.c
 * 
 * This code uses Zint to encode data into a QR Code and then outputs
 * the symbol as text suitable for use with the IDAutomation2D font
 * 
 * This code can be adapted to use any matrix symbology by changing the
 * line indicated.
 * 
 * This code can be compiled with:
 * 
 * gcc -o ida_2d ida_2d.c -lzint
 * 
 * Fonts can be downloaded from https://www.idautomation.com/
 * 
 */

#include <stdio.h>
#include <zint.h>
#include <string.h>
int main(int argc, char **argv)
{
    struct zint_symbol *my_symbol;
    int error = 0;
    int x, y, sub, glyph;
    
    my_symbol = ZBarcode_Create();
    
    my_symbol->symbology = BARCODE_QRCODE; // Change symbology here
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
    
    for (y = 0; y < my_symbol->rows; y += 4) {
        for (x = 0; x < my_symbol->width; x++) {
            glyph = 0;
            for (sub = 0; sub < 4; sub++) {
                glyph *= 2;
                if ((y + sub) < my_symbol->rows) {
                    if (((my_symbol->encoded_data[y + sub][x / 8] >> (x % 8)) & 1) == 0) {
                        glyph += 1;
                    }
                } else {
                    glyph += 1;
                }
            }
            glyph += 'A';
            printf("%c", glyph);
        }
        printf("\n");
    }
    
    ZBarcode_Delete(my_symbol);
    return 0;
}
