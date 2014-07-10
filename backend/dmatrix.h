/* dmatrix.h - Handles Data Matrix ECC 200 */

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

#include "common.h"

#ifndef __IEC16022ECC200_H
#define __IEC16022ECC200_H
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int data_matrix_200(struct zint_symbol *symbol, uint8_t source[], int length);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#define MAXBARCODE 3116

#define DM_ASCII	1
#define DM_C40		2
#define DM_TEXT		3
#define DM_X12		4
#define DM_EDIFACT	5
#define DM_BASE256	6

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
	
static const int intsymbol[] = {
	0,1,3,5,7,8,10,12,13,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,2,4,6,9,11,14 };

static const int matrixH[] = {
	10, 12, 8, 14, 8, 16, 12, 18, 20, 12, 22, 16, 24, 26, 16, 32, 36, 40, 44, 48,
	52, 64, 72, 80, 88, 96, 104, 120, 132, 144 };

static const int matrixW[] = {
	10, 12, 18, 14, 32, 16, 26, 18, 20, 36, 22, 36, 24, 26, 48, 32, 36, 40, 44,
	48, 52, 64, 72, 80, 88, 96, 104, 120, 132, 144 };

static const int matrixFH[] = {
	10, 12, 8, 14, 8, 16, 12, 18, 20, 12, 22, 16, 24, 26, 16, 16, 18, 20, 22, 24,
	26, 16, 18, 20, 22, 24, 26, 20, 22, 24 };

static const int matrixFW[] = {
	10, 12, 18, 14, 16, 16, 26, 18, 20, 18, 22, 18, 24, 26, 24, 16, 18, 20, 22,
	24, 26, 16, 18, 20, 22, 24, 26, 20, 22, 24 };

static const int matrixbytes[] = {
	3, 5, 5, 8, 10, 12, 16, 18, 22, 22, 30, 32, 36, 44, 49, 62, 86, 114, 144,
	174, 204, 280, 368, 456, 576, 696, 816, 1050, 1304, 1558 };

static const int matrixdatablock[] = {
	3, 5, 5, 8, 10, 12, 16, 18, 22, 22, 30, 32, 36, 44, 49, 62, 86, 114, 144,
	174, 102, 140, 92, 114, 144, 174, 136, 175, 163, 156 };
	
static const int matrixrsblock[] = {
	5, 7, 7, 10, 11, 12, 14, 14, 18, 18, 20, 24, 24, 28, 28, 36, 42, 48, 56, 68,
	42, 56, 36, 48, 56, 68, 56, 68, 62, 62 };

#endif				/* __IEC16022ECC200_H */
