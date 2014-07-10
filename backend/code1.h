/* code1.h - Lookup info for USS Code One */

/*
    libzint - the open source barcode library
    Copyright (C) 2009 Robin Stuart <robin@zint.org.uk>

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

static const int c40_shift[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

static const int c40_value[] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
	3,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,4,5,6,7,8,9,10,11,12,13,
	15,16,17,18,19,20,21,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
	22,23,24,25,26,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };

static const int text_shift[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3 };

static const int text_value[] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
	3,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,4,5,6,7,8,9,10,11,12,13,
	15,16,17,18,19,20,21,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
	22,23,24,25,26,0,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,27,28,29,30,31 };


static const int c1_height[] = { 16, 22, 28, 40, 52, 70, 104, 148 };
static const int c1_width[] = { 18, 22, 32, 42, 54, 76, 98, 134 };
static const int c1_data_length[] = { 10, 19, 44, 91, 182, 370, 732, 1480 };
static const int c1_ecc_length[] = { 10, 16, 26, 44, 70, 140, 280, 560 };
static const int c1_blocks[] = { 1, 1, 1, 1, 1, 2, 4, 8 };
static const int c1_data_blocks[] = { 10, 19, 44, 91, 182, 185, 183, 185 };
static const int c1_ecc_blocks[] = { 10, 16, 26, 44, 70, 70, 70, 70 };
static const int c1_grid_width[] = { 4, 5, 7, 9, 12, 17, 22, 30 };
static const int c1_grid_height[] = { 5, 7, 10, 15, 21, 30, 46, 68 };

#define C1_ASCII	1
#define C1_C40		2
#define C1_DECIMAL	3
#define C1_TEXT		4
#define C1_EDI		5
#define C1_BYTE		6
