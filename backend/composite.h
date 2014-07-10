/* composite.c - Tables for UCC.EAN Composite Symbols */

/*
    libzint - the open source barcode library
    Copyright (C) 2008 Robin Stuart <robin@zint.org.uk>

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

#define NUMERIC		110
#define ALPHA		97
#define ISOIEC		105
#define	INVALID_CHAR	100
#define	ANY_ENC		120
#define ALPHA_OR_ISO	121

/* CC-A component coefficients from ISO/IEC 24728:2006 Annex F */
static const int ccaCoeffs[30] = {
	/* k = 4 */
	522, 568, 723, 809,

	/* k = 5 */
	427, 919, 460, 155, 566,

	/* k = 6 */
	861, 285, 19, 803, 17, 766,

	/* k = 7 */
	76, 925, 537, 597, 784, 691, 437,

	/* k = 8 */
	237, 308, 436, 284, 646, 653, 428, 379
};

/* rows, error codewords, k-offset of valid CC-A sizes from ISO/IEC 24723:2006 Table 9 */
static const int ccaVariants[51] = {
	5, 6, 7, 8, 9, 10, 12, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7,
	4, 4, 5, 5, 6, 6, 7, 4, 5, 6, 7, 7, 4, 5, 6, 7, 8,
	0, 0, 4, 4, 9, 9, 15, 0, 4, 9, 15, 15, 0, 4, 9, 15, 22
};

/* following is Left RAP, Centre RAP, Right RAP and Start Cluster from ISO/IEC 24723:2006 tables 10 and 11 */
static const int aRAPTable[68] = {
	39, 1, 32, 8, 14, 43, 20, 11, 1, 5, 15, 21, 40, 43, 46, 34, 29,
	0, 0, 0, 0, 0, 0, 0, 43, 33, 37, 47, 1, 20, 23, 26, 14, 9,
	19, 33, 12, 40, 46, 23, 52, 23, 13, 17, 27, 33, 52, 3, 6, 46, 41,
	6, 0, 3, 3, 3, 0, 3, 3, 0, 3, 6, 6, 0, 0, 0, 0, 3
};

/* Row Address Patterns are as defined in pdf417.h */
