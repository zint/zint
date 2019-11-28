<?php
/* Convert libiconv-1.16/lib/jisx0208.h to Shift JIS values. */
/*
    libzint - the open source barcode library
    Copyright (C) 2008-2019 Robin Stuart <rstuart114@gmail.com>
*/
/* vim: set ts=4 sw=4 et : */

$in = false;
while ($line = fgets(STDIN)) {
    if ($in) {
        if (strncmp($line, '};', 2) === 0) {
            echo $line;
            break;
        }
        echo preg_replace_callback('/0x([0-9a-f]{4})/', function ($matches) {
            $dec = hexdec($matches[1]);
            $c1 = $dec >> 8;
            $c2 = ($dec & 0xFF);
            $t1 = ($c1 - 0x21) >> 1;
            $t2 = ((($c1 - 0x21) & 1) ? 0x5e : 0) + ($c2 - 0x21);
            $r = (($t1 < 0x1f ? ($t1 + 0x81) : ($t1 + 0xc1)) << 8) | ($t2 < 0x3f ? ($t2 + 0x40) : ($t2 + 0x41));
            return '0x' . dechex($r);
        }, $line);
    } else {
        if (strpos($line, 'jisx0208_2charset') !== false) {
            echo $line;
            $in = true;
        }
    }
}
