/* common.c - Contains functions needed for a number of barcodes */

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

int ustrlen(const uint8_t data[]) {
	/* Local replacement for strlen() with uint8_t strings */
	int i;
	for (i=0;data[i];i++);

	return i;
}

void ustrcpy(uint8_t target[], const uint8_t source[]) {
	/* Local replacement for strcpy() with uint8_t strings */
	int i, len;

	len = ustrlen(source);
	for(i = 0; i < len; i++) {
		target[i] = source[i];
	}
	target[i] = '\0';
}

void concat(char dest[], const char source[])
{ /* Concatinates dest[] with the contents of source[], copying /0 as well */
	unsigned int i, j, n;

	j = strlen(dest);
	n = strlen(source);
	for(i = 0; i <= n; i++) {
		dest[i + j] = source[i]; }
}

void uconcat(uint8_t dest[], const uint8_t source[])
{ /* Concatinates dest[] with the contents of source[], copying /0 as well */
	unsigned int i, j;

	j = ustrlen(dest);
	for(i = 0; i <= ustrlen(source); i++) {
		dest[i + j] = source[i]; }
}


int ctoi(char source)
{ /* Converts a character 0-9 to its equivalent integer value */
	if((source >= '0') && (source <= '9'))
		return (source - '0');
	return(source - 'A' + 10);
}

/** Converts an integer value to its hexadecimal character */
char itoc(int source)
{
	if ((source >= 0) && (source <= 9)) {
		return ('0' + source); }
	else {
		return ('A' + (source - 10)); }
}

/** Converts lower case characters to upper case in a string source[] */
void to_upper(uint8_t source[])
{
	unsigned int src_len = ustrlen(source);

	for (unsigned int i = 0; i < src_len; i++) {
		if ((source[i] >= 'a') && (source[i] <= 'z')) {
			source[i] = (source[i] - 'a') + 'A'; }
	}
}

int is_sane(char test_string[], uint8_t source[], int length)
{ /* Verifies that a string only uses valid characters */
	unsigned int latch;
	unsigned int lt = strlen(test_string);

	for(unsigned int i = 0; i < length; i++) {
		latch = FALSE;
		for(unsigned int j = 0; j < lt; j++) {
			if (source[i] == test_string[j]) {
				latch = TRUE;
				break;
			}
		}
		if (!latch) {
			return ZERROR_INVALID_DATA;
		}
	}

	return 0;
}

int posn(char set_string[], char data)
{ /* Returns the position of data in set_string */
	unsigned int n = strlen(set_string);

	for(unsigned int i = 0; i < n; i++)
		if (data == set_string[i])
			return i;
	return 0;
}

/** Replaces huge switch statements for looking up in tables */
void lookup(char set_string[], const char *table[], char data, char dest[])
{
	unsigned int n = strlen(set_string);

	for(unsigned int i = 0; i < n; i++)
		if (data == set_string[i])
			concat(dest, table[i]);
}

int module_is_set(struct zint_symbol *symbol, int y_coord, int x_coord)
{
	return (symbol->encoded_data[y_coord][x_coord / 7] >> (x_coord % 7)) & 1;
}

void set_module(struct zint_symbol *symbol, int y_coord, int x_coord)
{
	symbol->encoded_data[y_coord][x_coord / 7] |= 1 << (x_coord % 7);
}

void unset_module(struct zint_symbol *symbol, int y_coord, int x_coord)
{
	symbol->encoded_data[y_coord][x_coord / 7] &= ~(1 << (x_coord % 7));
}

void expand(struct zint_symbol *symbol, char data[])
{ /* Expands from a width pattern to a bit pattern */

	unsigned int reader, n = strlen(data);
	int writer;
	char latch;

	writer = 0;
	latch = '1';

	for(reader = 0; reader < n; reader++) {
		for(int i = 0; i < ctoi(data[reader]); i++) {
			if(latch == '1') { set_module(symbol, symbol->rows, writer); }
			writer++;
		}

		latch = (latch == '1' ? '0' : '1');
	}

	if(symbol->symbology != BARCODE_PHARMA) {
		if(writer > symbol->width) {
			symbol->width = writer;
		}
	} else {
		/* Pharmacode One ends with a space - adjust for this */
		if(writer > symbol->width + 2) {
			symbol->width = writer - 2;
		}
	}
	symbol->rows = symbol->rows + 1;
}

int is_stackable(int symbology) {
	/* Indicates which symbologies can have row binding */
	if(symbology < BARCODE_PDF417) { return 1; }
	if(symbology == BARCODE_CODE128B) { return 1; }
	if(symbology == BARCODE_ISBNX) { return 1; }
	if(symbology == BARCODE_EAN14) { return 1; }
	if(symbology == BARCODE_NVE18) { return 1; }
	if(symbology == BARCODE_KOREAPOST) { return 1; }
	if(symbology == BARCODE_PLESSEY) { return 1; }
	if(symbology == BARCODE_TELEPEN_NUM) { return 1; }
	if(symbology == BARCODE_ITF14) { return 1; }
	if(symbology == BARCODE_CODE32) { return 1; }

	return 0;
}

int is_extendable(int symbology) {
	/* Indicates which symbols can have addon */
	if(symbology == BARCODE_EANX) { return 1; }
	if(symbology == BARCODE_UPCA) { return 1; }
	if(symbology == BARCODE_UPCE) { return 1; }
	if(symbology == BARCODE_ISBNX) { return 1; }
	if(symbology == BARCODE_UPCA_CC) { return 1; }
	if(symbology == BARCODE_UPCE_CC) { return 1; }
	if(symbology == BARCODE_EANX_CC) { return 1; }

	return 0;
}

int roundup(float input)
{
	float remainder;
	int integer_part;

	integer_part = (int)input;
	remainder = input - integer_part;

	if(remainder > 0.1) {
		integer_part++;
	}

	return integer_part;
}

int istwodigits(uint8_t source[], int position)
{
	if((source[position] >= '0') && (source[position] <= '9')) {
		if((source[position + 1] >= '0') && (source[position + 1] <= '9')) {
			return 1;
		}
	}

	return 0;
}

float froundup(float input)
{
	float fraction, output = 0.0;

	fraction = input - (int)input;
	if(fraction > 0.01) { output = (input - fraction) + 1.0; } else { output = input; }

	return output;
}

int latin1_process(struct zint_symbol *symbol, uint8_t source[], uint8_t preprocessed[], int *length)
{
	int j = 0, i = 0, next;

	/* Convert Unicode to Latin-1 for those symbologies which only support Latin-1 */
	do {
		next = -1;
		if(source[i] < 128) {
			preprocessed[j] = source[i];
			j++;
			next = i + 1;
		} else {
			if(source[i] == 0xC2) {
				preprocessed[j] = source[i + 1];
				j++;
				next = i + 2;
			}
			if(source[i] == 0xC3) {
				preprocessed[j] = source[i + 1] + 64;
				j++;
				next = i + 2;
			}
		}
		if(next == -1) {
			strcpy(symbol->errtxt, "error: Invalid character in input string (only Latin-1 characters supported)");
			return ZERROR_INVALID_DATA;
		}
		i = next;
	} while(i < *length);
	preprocessed[j] = '\0';
	*length = j;

	return 0;
}

int utf8toutf16(struct zint_symbol *symbol, uint8_t source[], int vals[], int *length)
{
	int bpos, jpos, error_number;
	int next;

	bpos = 0;
	jpos = 0;
	error_number = 0;
	next = 0;

	do {
		if(source[bpos] <= 0x7f) {
			/* 1 byte mode (7-bit ASCII) */
			vals[jpos] = source[bpos];
			next = bpos + 1;
			jpos++;
		} else {
			if((source[bpos] >= 0x80) && (source[bpos] <= 0xbf)) {
				strcpy(symbol->errtxt, "Corrupt Unicode data");
				return ZERROR_INVALID_DATA;
			}
			if((source[bpos] >= 0xc0) && (source[bpos] <= 0xc1)) {
				strcpy(symbol->errtxt, "Overlong encoding not supported");
				return ZERROR_INVALID_DATA;
			}

			if((source[bpos] >= 0xc2) && (source[bpos] <= 0xdf)) {
				/* 2 byte mode */
				vals[jpos] = ((source[bpos] & 0x1f) << 6) + (source[bpos + 1] & 0x3f);
				next = bpos + 2;
				jpos++;
			} else
			if((source[bpos] >= 0xe0) && (source[bpos] <= 0xef)) {
				/* 3 byte mode */
				vals[jpos] = ((source[bpos] & 0x0f) << 12) + ((source[bpos + 1] & 0x3f) << 6) + (source[bpos + 2] & 0x3f);
				next = bpos + 3;
				jpos ++;
			} else
			if(source[bpos] >= 0xf0) {
				strcpy(symbol->errtxt, "Unicode sequences of more than 3 bytes not supported");
				return ZERROR_INVALID_DATA;
			}
		}

		bpos = next;

	} while(bpos < *length);
	*length = jpos;

	return error_number;
}

