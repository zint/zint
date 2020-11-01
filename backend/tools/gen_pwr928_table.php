<?php
/* Generate mod 928 powers table for `encode928()` in "composite.c" */
/*
    libzint - the open source barcode library
    Copyright (C) 2020 Robin Stuart <rstuart114@gmail.com>
*/
/* vim: set ts=4 sw=4 et : */

/* The functions "getBit", "init928" and "encode928" are copyright BSI and are
   released with permission under the following terms:

   "Copyright subsists in all BSI publications. BSI also holds the copyright, in the
   UK, of the international standardisation bodies. Except as
   permitted under the Copyright, Designs and Patents Act 1988 no extract may be
   reproduced, stored in a retrieval system or transmitted in any form or by any
   means - electronic, photocopying, recording or otherwise - without prior written
   permission from BSI.

   "This does not preclude the free use, in the course of implementing the standard,
   of necessary details such as symbols, and size, type or grade designations. If these
   details are to be used for any other purpose than implementation then the prior
   written permission of BSI must be obtained."

   The date of publication for these functions is 31 May 2006
 */

$cw = array(0, 0, 0, 0, 0, 0, 1);
$pwr928 = array( $cw );

for ($j = 1; $j < 69; $j++) {
    for ($v = 0, $i = 6; $i >= 1; $i--) {
        $v = (2 * $cw[$i]) + (int)($v / 928);
        $pwr928[$j][$i] = $cw[$i] = $v % 928;
    }
    $pwr928[$j][0] = $cw[0] = (2 * $cw[0]) + (int)($v / 928);
}

printf("static const UINT pwr928[69][7] = {\n");
for ($i = 0; $i < 69; $i++) {
    printf("    { ");
    for ($j = 0; $j < 7; $j++) {
        printf("%3d, ", $pwr928[$i][$j]);
    }
    printf("},\n");
}
printf("};\n");
