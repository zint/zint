/* pdf417.h - PDF417 tables and coefficients */

/*
    libzint - the open source barcode library
    Copyright (C) 2008 Robin Stuart <robin@zint.org.uk>
    Portions Copyright (C) 2004 Grandzebu

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

/* this file contains the character table, the pre-calculated coefficients and the
   codeword patterns taken from lines 416 to 454 of pdf417.frm */

#define TRUE 1
#define FALSE 0
#define TEX 900
#define BYT 901
#define NUM 902

#define BRSET "ABCDEFabcdefghijklmnopqrstuvwxyz*+-"

/* PDF417 error correction coefficients from Grand Zebu */
extern const int coefrs[1022];

extern const char *codagemc[2787];

/* converts values into bar patterns - replacing Grand Zebu's true type font */
extern const char *PDFttf[35];

/* MicroPDF417 coefficients from ISO/IEC 24728:2006 Annex F */
extern const int Microcoeffs[344];

/* rows, columns, error codewords, k-offset of valid MicroPDF417 sizes from ISO/IEC 24728:2006 */
extern const int MicroVariants[170];
/* rows, columns, error codewords, k-offset */

/* following is Left RAP, Centre RAP, Right RAP and Start Cluster from ISO/IEC 24728:2006 tables 10, 11 and 12 */
extern const int RAPTable[136];

/* Left and Right Row Address Pattern from Table 2 */
extern const char *RAPLR[53];

/* Centre Row Address Pattern from Table 2 */
extern const char *RAPC[53];

extern void byteprocess(int *chainemc, int *mclength, uint8_t chaine[], int start, int length, int block);
