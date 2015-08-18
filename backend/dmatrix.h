/* dmatrix.h - Handles Data Matrix ECC 200 */

/*
    libzint - the open source barcode library
    Copyright (C) 2009 Robin Stuart <robin@zint.org.uk>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright 
       notice, this list of conditions and the following disclaimer.  
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.  
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission. 

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
    SUCH DAMAGE.
*/

/*
    Containes Extended Rectangular Data Matrix (DMRE)
    See http://www.eurodatacouncil.org for information
    Contact: harald.oehlmann@eurodatacouncil.org
 */

#include "common.h"

#ifndef __IEC16022ECC200_H
#define __IEC16022ECC200_H
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int data_matrix_200(struct zint_symbol *symbol, unsigned char source[], int length);

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

#define DM_SYMBOL_OPTION_MAX 43

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

// Position in option array [symbol option value - 1]
// The position in the option array is by increasing total data codewords with square first
	
static const int intsymbol[] = {
	0,	/*  1: 10x10 */		1,	/*  2: 12x12 */		3,	/*  3: 14x14 */		5,	//  4: 16x16
	7,	/*  5: 18x18 */		9,	/*  6: 24x24	*/	12, /*  7: 22x22 */		15, //  8: 24x24
    17, /*  9: 26x26 */		22,	/* 10: 32x32 */		26, /* 11: 36x36 */		29, // 12: 40x40
    31,	/* 13: 44x44 */		32,	/* 14: 48x48 */		33,	/* 15: 52x52 */		34,	// 16: 64x64
    37,	/* 17: 72x72 */		38,	/* 18: 80x80 */		39,	/* 19: 88x88 */		41,	// 20: 96x96
    42,	/* 21:104x104*/		43,	/* 22:120x120*/		44,	/* 23:132x132*/		45,	// 24:144x144
    2,	/* 25:  8x18 */		4,	/* 26:  8x32 */		6,	/* 27: 12x26 */		10,	// 28: 12x36
    13,	/* 29: 16x36 */		18,	/* 30: 16x48 */		8,	/* 31:  8x48 */		11, /* 32:  8x64 */
    14,	/* 33: 12x48 */		16,	/* 34: 12x64 */		23,	/* 35: 16x64 */		19,	/* 36: 24x32 */
    21,	/* 37: 24x36 */		25,	/* 38: 24x48 */		28,	/* 39: 24x64 */		20, /* 40: 26x32 */
    24,	/* 41: 26x40 */		27,	/* 42: 26x48 */		30, /* 43: 26x64 */
    0 };

// Horizontal matrix size

static const int matrixH[] = {
/*0*/	10,	/* 10x10 ,3 */	12,	/* 12x12 ,5 */	8,	/*  8x18 ,5 */	14, /* 14x14 , 8 */
/*4*/	8,	/*  8x32 ,10 */	16,	/* 16x16 ,12 */	12,	/* 12x26 ,16 */	18, /* 18x18 ,18 */
/*8*/   8,	/*  8x48 ,18 */	20,	/* 20x20 ,22 */	12, /* 12x36 ,22 */	8,	/*  8x64 ,24 */
/*12*/	22,	/* 22x22 ,30 */	16,	/* 16x36 ,32 */	12,	/* 12x48 ,32 */	24, /* 24x24 ,36 */
/*16*/	12,	/* 12x64 ,43 */	26, /* 26x26 ,44 */	16, /* 16x48 ,49 */	24,	/* 24x32 ,49 */
/*20*/	26,	/* 26x32 ,52 */	24,	/* 24x36 ,55 */	32, /* 32x32 ,62 */	16, /* 16x64 ,62 */
/*24*/	26,	/* 26x40 ,70 */	24,	/* 24x48 ,80 */	36, /* 36x36 ,86 */	26,	/* 26x48 ,90 */
/*28*/	24,	/* 24x64 ,108*/	40,	/* 40x40 ,114*/	26,	/* 26x64 ,118*/	44,	/* 44x44 ,144*/
/*32*/	48,	/* 48x48,174 */	52, /* 62x52,204 */	64, /* 64x64,280 */	72,	/* 72x72,368 */
/*36*/	80, /* 80x80,456 */	88, /* 88x88,576 */	96, /* 96x96,696 */	104,/*104x104,816*/
/*40*/	120,/*120x120,1050*/132,/*132x132,1304*/144/*144x144,1558*/
		};

// Vertical matrix sizes

static const int matrixW[] = {
/*0*/	10, /* 10x10 */	12, /* 12x12 */	18, /*  8x18 */	14,	/* 14x14 */
/*4*/	32, /*  8x32 */	16, /* 16x16 */	26, /* 12x26 */	18, /* 18x18 */
/*8*/   48, /*  8x48 */	20, /* 20x20 */	36, /* 12x36 */	64, /*  8x64 */
/*12*/	22, /* 22x22 */	36, /* 16x36 */	48,	/* 12x48 */	24, /* 24x24 */
/*16*/	64,	/* 12x64 */	26, /* 26x26 */	48,	/* 16x48 */	32,	/* 24x32 */
/*20*/	32,	/* 26x32 */	36,	/* 24x36 */	32, /* 32x32 */	64,	/* 16x64 */
/*24*/	40,	/* 26x40 */	48,	/* 24x48 */	36,	/* 36x36 */	48,	/* 26x48 */
/*28*/	64,	/* 24x64 */	40,	/* 40x40 */	64,	/* 26x64 */	44,	/* 44x44 */
/*32*/	48, /* 48x48 */	52, /* 52x52 */	64, /* 64x64 */	72,	/* 72x72 */
/*36*/	80, /* 80x80 */	88, /* 88x88 */	96, /* 96x96 */	104,/* 104x104 */
/*40*/	120,/*120x120*/ 132,/*132x132*/ 144 /*144x144*/
		};

// Horizontal submodule size (including subfinder)

static const int matrixFH[] = {
/*0*/	10,	/* 10x10 */	12, /* 12x12 */	8,	/*  8x18 */	14,	/* 14x14 */
/*4*/	8,	/*  8x32 */	16,	/* 16x16 */	12,	/* 12x26 */	18, /* 18x18 */
/*8*/   8,	/*  8x48 */	20, /* 20x20 */	12, /* 12x36 */	8,	/*  8x64 */
/*12*/	22,	/* 22x22 */	16,	/* 16x36 */	12,	/* 12x48 */	24,	/* 24x24 */
/*16*/	12,	/* 12x64 */ 26,	/* 26x26 */	16,	/* 16x48 */	24,	/* 24x32 */
/*20*/	26,	/* 26x32 */	24,	/* 24x36 */	16,	/* 32x32 */	16,	/* 16x64 */
/*24*/	26,	/* 26x40 */	24,	/* 24x48 */	18,	/* 36x36 */	26,	/* 26x48 */
/*28*/	24,	/* 24x64 */	20,	/* 40x40 */	26,	/* 26x64 */	22,	/* 44x44 */
/*32*/	24,	/* 48x48 */	26,	/* 52x52 */	16,	/* 64x64 */	18,	/* 72x72 */
/*36*/	20,	/* 80x80 */	22,	/* 88x88 */	24,	/* 96x96 */	26,	/*104x104*/
/*40*/	20,	/*120x120*/	22,	/*132x132*/	24	/*144x144*/
		};

// Vertical submodule size (including subfinder)

static const int matrixFW[] = {
/*0*/	10,	/* 10x10 */	12,	/* 12x12 */	18,	/*  8x18 */	14,	/* 14x14 */
/*4*/	16,	/*  8x32 */	16,	/* 16x16 */	26,	/* 12x26 */	18, /* 18x18 */
/*8*/   24,	/*  8x48 */	20,	/* 20x20 */	18,	/* 12x36 */	16,	/*  8x64 */
/*12*/	22,	/* 22x22 */	18,	/* 16x36 */	24,	/* 12x48 */	24,	/* 24x24 */
/*16*/	16,	/* 12x64 */	26,	/* 26x26 */	24,	/* 16x48 */	16,	/* 24x32 */
/*20*/	16,	/* 26x32 */	18,	/* 24x36 */	16,	/* 32x32 */	16,	/* 16x64 */
/*24*/	20,	/* 26x40 */	24,	/* 24x48 */	18,	/* 36x36 */	24,	/* 26x48 */
/*28*/	16,	/* 24x64 */	20,	/* 40x40 */	16,	/* 26x64 */	22,	/* 44x44 */
/*32*/	24,	/* 48x48 */	26,	/* 52x52 */	16,	/* 64x64 */	18,	/* 72x72 */
/*36*/	20,	/* 80x80 */	22,	/* 88x88 */	24,	/* 96x96 */	26,	/*104x104*/
/*40*/	20,	/*120x120*/	22,	/*132x132*/	24	/*144x144*/
		};

// Total Data Codewords

static const int matrixbytes[] = {
/*0*/	3,		/* 10x10 */	5,		/* 12x12 */	5,		/* 8x18 */	8,		/* 14x14 */
/*4*/	10,		/* 8x32 */	12,		/* 16x16 */	16,		/* 12x26 */	18,		/* 18x18 */
/*8*/   18,		/* 8x48 */	22,		/* 20x20 */	22,		/* 12x36 */	24,		/* 8x64 */
/*12*/	30,		/* 22x22 */	32,		/* 16x36 */	32,		/* 12x48 */	36,		/* 24x24 */
/*16*/	43,		/* 12x64 */	44,		/* 26x26 */	49,		/* 16x48 */	49,		/* 24x32 */
/*20*/	52,		/* 26x32 */	55,		/* 24x36 */	62,		/* 32x32 */	62,		/* 16x64 */
/*24*/	70,		/* 26x40 */	80,		/* 24x48 */	86,		/* 36x36 */	90,		/* 26x48 */
/*28*/	108,	/* 24x64 */	114,	/* 40x40 */	118,	/* 26x64 */	144,	/* 44x44 */
/*32*/	174,	/* 48x48 */	204,	/* 52x52 */	280,	/* 64x64 */	368,	/* 72x72 */
/*36*/	456,	/* 80x80 */	576,	/* 88x88 */	696,	/* 96x96 */	816,	/*104x104*/
/*40*/	1050,	/*120x120*/	1304,	/*132x132*/	1558	/*144x144*/
		};

// Data Codewords per RS-Block

static const int matrixdatablock[] = {
/*0*/	3,	/* 10x10 */	5,	/* 12x12 */	5,	/* 8x18 */	8,	/* 14x14 */
/*4*/	10,	/* 8x32 */	12,	/* 16x16 */	16,	/* 12x26 */	18, /* 18x18 */
/*8*/   18, /* 8x48 */	22,	/* 20x20 */	22,	/* 12x36 */	24,	/* 8x64 */
/*12*/	30,	/* 22x22 */	32,	/* 16x36 */	32, /* 12x48 */	36,	/* 24x24 */
/*16*/	43,	/* 12x64 */	44,	/* 26x26 */	49,	/* 16x48 */	49,	/* 24x32 */
/*20*/	52,	/* 26x32 */	55,	/* 24x36 */	62,	/* 32x32 */	62,	/* 16x64 */
/*24*/	70,	/* 26x40 */	80,	/* 24x48 */	86,	/* 36x36 */	90,	/* 26x48 */
/*28*/	108,/* 24x64 */	114,/* 40x40 */	118,/* 26x64 */	144,/* 44x44 */
/*32*/	174,/* 48x48 */	102,/* 52x52 */	140,/* 64x64 */	92,	/* 72x72 */
/*36*/	114,/* 80x80 */	144,/* 88x88 */	174,/* 96x96 */	136,/*104x104*/
/*40*/	175,/*120x120*/	163,/*132x132*/	156	/*144x144*/
		};

// ECC Codewords per RS-Block

static const int matrixrsblock[] = {
/*0*/	5,	/* 10x10 */	7,	/* 12x12 */	7,	/*  8x18 */	10,	/* 14x14 */
/*4*/	11,	/*  8x32 */	12,	/* 16x16 */ 14,	/* 12x26 */ 14,	/* 18x18 */
/*8*/	15, /*  8x48 */	18,	/* 20x20 */	18,	/* 12x36 */ 18,	/*  8x64 */
/*12*/	20,	/* 22x22 */ 24,	/* 16x36 */ 23, /* 12x48 */	24,	/* 24x24 */
/*16*/	27,	/* 12x64 */	28,	/* 26x26 */ 28,	/* 16x48 */	28,	/* 24x32 */
/*20*/	32,	/* 26x32 */	33,	/* 24x36 */	36,	/* 32x32 */	36,	/* 16x64 */
/*24*/	38,	/* 26x40 */	41,	/* 24x48 */	42,	/* 36x36 */	42,	/* 26x48 */
/*28*/	46,	/* 24x64 */	48,	/* 40x40 */	50,	/* 26x64 */	56,	/* 44x44 */
/*32*/	68, /* 48x48 */	42,	/* 52x52 */ 56,	/* 64x64 */ 36,	/* 72x72 */
/*36*/	48, /* 80x80 */	56,	/* 88x88 */ 68,	/* 96x96 */ 56,	/*104x104*/
/*40*/	68,	/*120x120*/	62,	/*132x132*/	62	/*144x144*/
		};

#endif				/* __IEC16022ECC200_H */
