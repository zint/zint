/* 2of5.c - Handles Code 2 of 5 barcodes */

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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

static const char *C25MatrixTable[10] = {"113311", "311131", "131131", "331111", "113131", "313111",
	"133111", "111331", "311311", "131311"};

static const char *C25IndustTable[10] = {"1111313111", "3111111131", "1131111131", "3131111111", "1111311131",
	"3111311111", "1131311111", "1111113131", "3111113111", "1131113111"};

static const char *C25InterTable[10] = {"11331", "31113", "13113", "33111", "11313", "31311", "13311", "11133",
	"31131", "13131"};


static inline char check_digit(unsigned int count)
{
	return itoc((10 - (count % 10)) % 10);
}


int matrix_two_of_five(struct zint_symbol *symbol, uint8_t source[], int length)
{ /* Code 2 of 5 Standard (Code 2 of 5 Matrix) */

	int error_number;
	char dest[512]; /* 6 + 80 * 6 + 6 + 1 ~ 512*/

	error_number = 0;

	if(length > 80) {
		strcpy(symbol->errtxt, "Input too long");
		return ZERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if(error_number == ZERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	/* start character */
	strcpy(dest, "411111");

	for(int i = 0; i < length; i++) {
		lookup(NEON, C25MatrixTable, source[i], dest);
	}

	/* Stop character */
	concat (dest, "41111");

	expand(symbol, dest);
	ustrcpy(symbol->text, source);
	return error_number;
}

int industrial_two_of_five(struct zint_symbol *symbol, uint8_t source[], int length)
{ /* Code 2 of 5 Industrial */

	int error_number;
	char dest[512]; /* 6 + 40 * 10 + 6 + 1 */

	error_number = 0;

	if(length > 45) {
		strcpy(symbol->errtxt, "Input too long");
		return ZERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if(error_number == ZERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid character in data");
		return error_number;
	}

	/* start character */
	strcpy(dest, "313111");

	for(int i = 0; i < length; i++) {
		lookup(NEON, C25IndustTable, source[i], dest);
	}

	/* Stop character */
	concat (dest, "31113");

	expand(symbol, dest);
	ustrcpy(symbol->text, source);
	return error_number;
}

int iata_two_of_five(struct zint_symbol *symbol, uint8_t source[], int length)
{ /* Code 2 of 5 IATA */
	int error_number;
	char dest[512]; /* 4 + 45 * 10 + 3 + 1 */

	error_number = 0;

	if(length > 45) {
		strcpy(symbol->errtxt, "Input too long");
		return ZERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if(error_number == ZERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	/* start */
	strcpy(dest, "1111");

	for(int i = 0; i < length; i++) {
		lookup(NEON, C25IndustTable, source[i], dest);
	}

	/* stop */
	concat (dest, "311");

	expand(symbol, dest);
	ustrcpy(symbol->text, source);
	return error_number;
}

int logic_two_of_five(struct zint_symbol *symbol, uint8_t source[], int length)
{ /* Code 2 of 5 Data Logic */

	int error_number;
	char dest[512]; /* 4 + 80 * 6 + 3 + 1 */

	error_number = 0;

	if(length > 80) {
		strcpy(symbol->errtxt, "Input too long");
		return ZERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if(error_number == ZERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	/* start character */
	strcpy(dest, "1111");

	for(int i = 0; i < length; i++) {
		lookup(NEON, C25MatrixTable, source[i], dest);
	}

	/* Stop character */
	concat (dest, "311");

	expand(symbol, dest);
	ustrcpy(symbol->text, source);
	return error_number;
}

int interleaved_two_of_five(struct zint_symbol *symbol, uint8_t source[], int length)
{ /* Code 2 of 5 Interleaved */

	int error_number;
	char bars[7], spaces[7], mixed[14], dest[1000];
	uint8_t temp[length + 2];

	error_number = 0;

	if(length > 89) {
		strcpy(symbol->errtxt, "Input too long");
		return ZERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if (error_number == ZERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	ustrcpy(temp, (uint8_t *) "");
	/* Input must be an even number of characters for Interlaced 2 of 5 to work:
	   if an odd number of characters has been entered then add a leading zero */
	if (length & 1) {
		ustrcpy(temp, (uint8_t *) "0");
		length++;
	}
	uconcat(temp, source);

	/* start character */
	strcpy(dest, "1111");

	for(int i = 0; i < length; i+=2) {
		/* look up the bars and the spaces and put them in two strings */
		strcpy(bars, "");
		lookup(NEON, C25InterTable, temp[i], bars);
		strcpy(spaces, "");
		lookup(NEON, C25InterTable, temp[i + 1], spaces);

		/* then merge (interlace) the strings together */
		int k = 0;
		for(int j = 0; j <= 4; j++)
		{
			mixed[k] = bars[j]; k++;
			mixed[k] = spaces[j]; k++;
		}
		mixed[k] = '\0';
		concat (dest, mixed);
	}

	/* Stop character */
	concat (dest, "311");

	expand(symbol, dest);
	ustrcpy(symbol->text, temp);
	return error_number;

}

int itf14(struct zint_symbol *symbol, uint8_t source[], int length)
{
	int error_number, zeroes;
	unsigned int count;
	char localstr[16];

	error_number = 0;

	count = 0;

	if(length > 13) {
		strcpy(symbol->errtxt, "Input too long");
		return ZERROR_TOO_LONG;
	}

	error_number = is_sane(NEON, source, length);
	if(error_number == ZERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid character in data");
		return error_number;
	}

	/* Add leading zeros as required */
	zeroes = 13 - length;
	for(int i = 0; i < zeroes; i++) {
		localstr[i] = '0';
	}
	strcpy(localstr + zeroes, (char *)source);

	/* Calculate the check digit - the same method used for EAN-13 */

	for (int i = 12; i >= 0; i--) {
		count += ctoi(localstr[i]);

		if (!(i & 1)) {
			count += 2 * ctoi(localstr[i]);
		}
	}

	localstr[13] = check_digit(count);
	localstr[14] = '\0';
	error_number = interleaved_two_of_five(symbol, (uint8_t *)localstr, strlen(localstr));
	ustrcpy(symbol->text, (uint8_t*)localstr);
	return error_number;
}

int dpleit(struct zint_symbol *symbol, uint8_t source[], int length)
{ /* Deutshe Post Leitcode */
	int error_number;
	unsigned int count;
	char localstr[16];
	int zeroes;

	error_number = 0;
	count = 0;
	if(length > 13) {
		strcpy(symbol->errtxt, "Input wrong length");
		return ZERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if(error_number == ZERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	zeroes = 13 - length;
	for(int i = 0; i < zeroes; i++)
		localstr[i] = '0';
	strcpy(localstr + zeroes, (char *)source);

	for (int i = 12; i >= 0; i--)
	{
		count += 4 * ctoi(localstr[i]);

		if (i & 1) {
			count += 5 * ctoi(localstr[i]);
		}
	}

	localstr[13] = check_digit(count);
	localstr[14] = '\0';
	error_number = interleaved_two_of_five(symbol, (uint8_t *)localstr, strlen(localstr));
	ustrcpy(symbol->text, (uint8_t*)localstr);
	return error_number;
}

int dpident(struct zint_symbol *symbol, uint8_t source[], int length)
{ /* Deutsche Post Identcode */
	int error_number, zeroes;
	unsigned int count;
	char localstr[16];

	count = 0;
	if (length > 11) {
		strcpy(symbol->errtxt, "Input wrong length");
		return ZERROR_TOO_LONG;
	}

	error_number = is_sane(NEON, source, length);
	if (error_number == ZERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	zeroes = 11 - length;
	for (int i = 0; i < zeroes; i++)
		localstr[i] = '0';
	strcpy(localstr + zeroes, (char *)source);

	for (int i = 10; i >= 0; i--)
	{
		count += 4 * ctoi(localstr[i]);

		if (i & 1) {
			count += 5 * ctoi(localstr[i]);
		}
	}

	localstr[11] = check_digit(count);
	localstr[12] = '\0';
	error_number = interleaved_two_of_five(symbol, (uint8_t *)localstr, strlen(localstr));
	ustrcpy(symbol->text, (uint8_t*)localstr);
	return error_number;
}
