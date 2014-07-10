/*
 * This is a simple Reed-Solomon encoder
 * (C) Cliff Hones 2004
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

// It is not written with high efficiency in mind, so is probably
// not suitable for real-time encoding.  The aim was to keep it
// simple, general and clear.
//
// <Some notes on the theory and implementation need to be added here>

// Usage:
// First call rs_init_gf(poly) to set up the Galois Field parameters.
// Then  call rs_init_code(size, index) to set the encoding size
// Then  call rs_encode(datasize, data, out) to encode the data.
//
// These can be called repeatedly as required - but note that
// rs_init_code must be called following any rs_init_gf call.
//
// If the parameters are fixed, some of the statics below can be
// replaced with constants in the obvious way, and additionally
// malloc/free can be avoided by using static arrays of a suitable
// size.

#include <stdio.h>		// only needed for debug (main)
#include <stdlib.h>		// only needed for malloc/free
#include "reedsol.h"
static int gfpoly;
static int symsize;		// in bits
static int logmod;		// 2**symsize - 1
static int rlen;

static int *logt = NULL, *alog = NULL, *rspoly = NULL;

// rs_init_gf(poly) initialises the parameters for the Galois Field.
// The symbol size is determined from the highest bit set in poly
// This implementation will support sizes up to 30 bits (though that
// will result in very large log/antilog tables) - bit sizes of
// 8 or 4 are typical
//
// The poly is the bit pattern representing the GF characteristic
// polynomial.  e.g. for ECC200 (8-bit symbols) the polynomial is
// a**8 + a**5 + a**3 + a**2 + 1, which translates to 0x12d.

void rs_init_gf(int poly)
{
	int m, b, p, v;

	// Find the top bit, and hence the symbol size
	for (b = 1, m = 0; b <= poly; b <<= 1)
		m++;
	b >>= 1;
	m--;
	gfpoly = poly;
	symsize = m;

	// Calculate the log/alog tables
	logmod = (1 << m) - 1;
	logt = (int *)malloc(sizeof(int) * (logmod + 1));
	alog = (int *)malloc(sizeof(int) * logmod);

	for (p = 1, v = 0; v < logmod; v++) {
		alog[v] = p;
		logt[p] = v;
		p <<= 1;
		if (p & b)
			p ^= poly;
	}
}

// rs_init_code(nsym, index) initialises the Reed-Solomon encoder
// nsym is the number of symbols to be generated (to be appended
// to the input data).  index is usually 1 - it is the index of
// the constant in the first term (i) of the RS generator polynomial:
// (x + 2**i)*(x + 2**(i+1))*...   [nsym terms]
// For ECC200, index is 1.

void rs_init_code(int nsym, int index)
{
	int i, k;

	rspoly = (int *)malloc(sizeof(int) * (nsym + 1));

	rlen = nsym;

	rspoly[0] = 1;
	for (i = 1; i <= nsym; i++) {
		rspoly[i] = 1;
		for (k = i - 1; k > 0; k--) {
			if (rspoly[k])
				rspoly[k] = alog[(logt[rspoly[k]] + index) % logmod];
			rspoly[k] ^= rspoly[k - 1];
		}
		rspoly[0] = alog[(logt[rspoly[0]] + index) % logmod];
		index++;
	}
}

void rs_encode(int len, const uint8_t *data, uint8_t *res)
{
	int i, k, m;
	for (i = 0; i < rlen; i++)
		res[i] = 0;
	for (i = 0; i < len; i++) {
		m = res[rlen - 1] ^ data[i];
		for (k = rlen - 1; k > 0; k--) {
			if (m && rspoly[k])
				res[k] = res[k - 1] ^ alog[(logt[m] + logt[rspoly[k]]) % logmod];
			else
				res[k] = res[k - 1];
		}
		if (m && rspoly[0])
			res[0] = alog[(logt[m] + logt[rspoly[0]]) % logmod];
		else
			res[0] = 0;
	}
}

void rs_encode_long(int len, const unsigned int *data, unsigned int *res)
{ /* The same as above but for larger bitlengths - Aztec code compatible */
	int i, k, m;
	for (i = 0; i < rlen; i++)
		res[i] = 0;
	for (i = 0; i < len; i++) {
		m = res[rlen - 1] ^ data[i];
		for (k = rlen - 1; k > 0; k--) {
			if (m && rspoly[k])
				res[k] = res[k - 1] ^ alog[(logt[m] + logt[rspoly[k]]) % logmod];
			else
				res[k] = res[k - 1];
		}
		if (m && rspoly[0])
			res[0] = alog[(logt[m] + logt[rspoly[0]]) % logmod];
		else
			res[0] = 0;
	}
}

void rs_free(void)
{ /* Free memory */
	free(logt);
	free(alog);
	free(rspoly);
	rspoly = NULL;
}
