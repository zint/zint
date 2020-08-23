/* 
 * stroke.c
 * 
 * This code uses Zint to encode data in QR Code and then output in
 * the correct format to use StrokeScribe 2D font. This can be adapted
 * to encode any matrix symbology using this font.
 * 
 * The same code can also be used to resolve PDF417 symbols with the
 * StrokeScribe 417 font and linear symbols with the StrokeScribe 1D
 * font, all of which are available from the same souce.
 * 
 * This code can be compiled with:
 * 
 * gcc -o stroke stroke.c -lzint
 * 
 * The fonts are available from:
 * 
 * https://strokescribe.com/en/free-version-barcode-truetype-fonts.html
 * 
 */

#include <stdio.h>
#include <zint.h>
#include <string.h>
int main(int argc, char **argv)
{
    struct zint_symbol *my_symbol;
    int error = 0;
    int x, y, glyph, sub;
    
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

    sub = 0;
    glyph = 0;
    for (y = 0; y < my_symbol->rows; y++) {
        for (x = 0; x < my_symbol->width; x++) {
            glyph *= 2;
            if ((my_symbol->encoded_data[y][x / 8] >> (x % 8)) & 1) {
                glyph += 1;
            }
            sub++;
            if (sub == 5) {
                if (glyph <= 25) {
                    printf("%c", glyph + 'A');
                } else {
                    printf("%c", (glyph - 26) + 'a');
                }
                sub = 0;
                glyph = 0;
            }
        }
        if (sub == 4) {
            printf("%c", glyph + 'g');
        }
        if (sub == 3) {
            if (glyph <= 3) {
                printf("%c", glyph + 'w');
            } else {
                printf("%c", (glyph - 4) + '0');
            }
        }
        if (sub == 2) {
            printf("%c", glyph + '4');
        }
        if (sub == 1) {
            printf("%c", glyph + '8');
        }
        printf("\n");
        sub = 0;
        glyph = 0;
    }
    
    ZBarcode_Delete(my_symbol);
    return 0;
}
