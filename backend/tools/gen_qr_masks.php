<?php
/* Generate pre-calculated `qr_masks[]` for "qr.h" */
/*
    libzint - the open source barcode library
    Copyright (C) 2026 Robin Stuart <rstuart114@gmail.com>
*/
/* SPDX-License-Identifier: BSD-3-Clause */

/* Taken from Barcode Writer in Pure PostScript (BWIPP)
   Copyright (c) 2004-2026 Terry Burton */
/* SPDX-License-Identifier: MIT */

/* See https://sourceforge.net/p/zint/mailman/message/59278637/
   and https://github.com/bwipp/postscriptbarcode/commit/763fb4ffbfad7b379723dd3570183c769b28c786
   and "Pre-calculated QR and MicroQR tables" comment in "qr.h"

   Paste output after comment
*/

$qr_masks = array();
$mqr_masks = array();

for ($y = 0; $y < 12; $y++) {
    for ($x = 0; $x < 6; $x++) {
        $bits = 0;
        $mbits = 0;
        if (($y + $x) % 2 === 0) { /* QR 000, period 2x2 */
            $bits |= 0x01;
        }
        if ($y % 2 === 0) { /* QR 001, MicroQR 00, period 1x2 */
            $bits |= 0x02;
            $mbits |= 0x01;
        }
        if ($x % 3 === 0) { /* QR 010, period 3x1 */
            $bits |= 0x04;
        }
        if (($y + $x) % 3 === 0) { /* QR 011, period 3x3 */
            $bits |= 0x08;
        }
        if (((int)($y / 2) + (int)($x / 3)) % 2 === 0) { /* QR 100, MicroQR 01, period 6x4 */
            $bits |= 0x10;
            $mbits |= 0x02;
        }
        if ((($y * $x) % 2 + ($y * $x) % 3) === 0) { /* QR 101, period 6x6 */
            $bits |= 0x20;
        }
        if ((($y * $x) % 2 + ($y * $x) % 3) % 2 === 0) { /* QR 110, MicroQR 10, period 6x6 */
            $bits |= 0x40;
            $mbits |= 0x04;
        }
        if ((($y + $x) % 2 + ($y * $x) % 3) % 2 === 0) { /* QR 111, MicroQR 11, period 6x6 */
            $bits |= 0x80;
            $mbits |= 0x08;
        }
        $qr_masks[$y][$x] = $bits;
        $mqr_masks[$y][$x] = $mbits;
    }
}

printf("static const unsigned char qr_masks[12][6] = {\n");
for ($y = 0; $y < 12; $y++) {
    printf("    { ");
    for ($x = 0; $x < 6; $x++) {
        printf("0x%02X%s", $qr_masks[$y][$x], $x == 5 ? " " : ", ");
    }
    printf("}%s\n", $y == 11 ? "" : ",");
}
printf("};\n");

printf("\nstatic const unsigned char microqr_masks[12][6] = {\n");
for ($y = 0; $y < 12; $y++) {
    printf("    { ");
    for ($x = 0; $x < 6; $x++) {
        printf("0x%02X%s", $mqr_masks[$y][$x], $x == 5 ? " " : ", ");
    }
    printf("}%s\n", $y == 11 ? "" : ",");
}
printf("};\n");

/* vim: set ts=4 sw=4 et : */
