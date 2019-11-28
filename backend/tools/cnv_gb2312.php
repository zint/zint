<?php
/* Convert libiconv-1.16/lib/gb2312.h to EUC-CN values (+ 0x8080). */
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
            return '0x' . dechex(hexdec($matches[1]) + 0x8080);
        }, $line);
    } else {
        if (strpos($line, 'short gb2312_2charset') !== false) {
            echo $line;
            $in = true;
        }
    }
}
