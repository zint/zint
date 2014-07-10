/* gs1.h - Verifies GS1 data */

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
#ifndef __GS1_H
#define __GS1_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int gs1_verify(struct zint_symbol *symbol, const uint8_t source[], const unsigned int src_len, char reduced[]);
extern int ugs1_verify(struct zint_symbol *symbol, const uint8_t source[], const unsigned int src_len, uint8_t reduced[]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GS1_H */
