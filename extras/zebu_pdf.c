/* 
 * zebu_pdf.c
 * 
 * This code uses Zint to encode data into a PDF417 and then outputs
 * the symbol as text suitable for use with Code PDF417 font by
 * Grand Zebu.
 * 
 * This code can be compiled with:
 * 
 * gcc -o zebu_pdf zebu_pdf.c -lzint
 * 
 * Grand Zebu's font can be downloaded from:
 * 
 * https://grandzebu.net/informatique/codbar-en/pdf417.htm
 * 
 */

#include <stdio.h>
#include <zint.h>
#include <string.h>
int main(int argc, char **argv)
{
    struct zint_symbol *my_symbol;
    int error = 0;
    int x, y, sub, glyph, group;
    
    my_symbol = ZBarcode_Create();
    
    my_symbol->symbology = BARCODE_PDF417;
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
    
    for (y = 0; y < my_symbol->rows; y++) {
        printf("+*");
        sub = 0;
        glyph = 0;
        group = 0;
        for (x = 18; x < my_symbol->width - 19; x++) {
            glyph *= 2;
            if ((my_symbol->encoded_data[y][x / 8] >> (x % 8)) & 1) {
                glyph++;
            }
            sub++;
            if (sub == 5) {
                if (glyph <= 5) {
                    printf("%c", glyph + 'A');
                } else {
                    printf("%c", (glyph - 6) + 'a');
                }
                glyph = 0;
                sub = 0;
                group++;
            }
            if (group == 3) {
                printf("*");
                x += 2;
                group = 0;
            }
        }
        printf("-\n");
    }
    
    ZBarcode_Delete(my_symbol);
    return 0;
}
