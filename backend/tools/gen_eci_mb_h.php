<?php
/* Generate ECI multibyte tables from unicode.org mapping files */
/*
    libzint - the open source barcode library
    Copyright (C) 2022-2023 Robin Stuart <rstuart114@gmail.com>
*/
/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * To create "backend/eci_big5/gb18030/gb2312/gbk/ksx1001/sjis.h" (from project root directory):
 *
 *   php backend/tools/gen_eci_mb_h.php
 *
 * NOTE: backend/tools/data/GB18030.TXT will have to be downloaded first from the tarball
 *       https://haible.de/bruno/charsets/conversion-tables/GB18030.tar.bz2
 *       using the version jdk-1.4.2/GB18030.TXT
 *
 * NOTE: tools/data/GB2312.TXT will have to be downloaded first from the tarball
 *       https://haible.de/bruno/charsets/conversion-tables/GB2312.tar.bz2
 *       using the version unicode.org-mappings/EASTASIA/GB/GB2312.TXT
 */
// 'zend.assertions' should set to 1 in php.ini

$copyright_text = <<<'EOD'

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
/* SPDX-License-Identifier: BSD-3-Clause */

EOD;

$basename = basename(__FILE__);
$dirname = dirname(__FILE__);

$opts = getopt('d:o:');
$data_dirname = isset($opts['d']) ? $opts['d'] : ($dirname . '/data'); // Where to load file from.
$out_dirname = isset($opts['o']) ? $opts['o'] : ($dirname . '/..'); // Where to put output.

$year = 2022;

function out_header(&$out, $name, $descr, $file, $start_year = 0, $extra_comment = '') {
    global $copyright_text, $basename, $year;
    $caps = strtoupper($name);
    $out[] = '/*  ' . $name . '.h - tables for Unicode to ' . $descr . ', generated by "backend/tools/' . $basename . '"';
    if ($extra_comment !== '') {
        $out[] = '    from "' . $file . '"';
        $out[] = '    ' . $extra_comment . ' */';
    } else {
        $out[] = '    from "' . $file . '" */';
    }
    $out[] = '/*';
    $out[] = '    libzint - the open source barcode library';
    if ($start_year && $start_year != $year) {
        $out[] = '    Copyright (C) ' . $start_year . '-' . $year . ' Robin Stuart <rstuart114@gmail.com>';
    } else {
        $out[] = '    Copyright (C) ' . $year . ' Robin Stuart <rstuart114@gmail.com>';
    }
    $out = array_merge($out, explode("\n", $copyright_text));
    $out[] = '#ifndef Z_' . $caps . '_H';
    $out[] = '#define Z_' . $caps . '_H';
}

/* Output a block of table entries to `$out` array */
function out_tab_entries(&$out, $arr, $cnt, $not_hex = false) {
    $line = '   ';
    for ($i = 0; $i < $cnt; $i++) {
        if ($i && $i % 8 === 0) {
            $out[] = $line;
            $line = '   ';
        }
        if ($not_hex) {
            $line .= sprintf(' %5d,', $arr[$i]);
        } else {
            $line .= sprintf(' 0x%04X,', $arr[$i]);
        }
    }
    if ($line !== '   ') {
        $out[] = $line;
    }
}

/* Output tables to `$out` array */
function out_tabs(&$out, $name, $sort, $mb, $no_u_ind = false, $u_comment = '', $mb_comment = '') {
    if ($u_comment == '') $u_comment = 'Unicode codepoints sorted';
    $cnt_sort = count($sort);
    $out[] = '';
    $out[] = '/* ' . $u_comment . ' */';
    $out[] = 'static const unsigned short ' . $name . '_u[' . $cnt_sort . '] = {';
    out_tab_entries($out, $sort, $cnt_sort);
    $out[] = '};';

    if (!empty($mb)) {
        if ($mb_comment == '') $mb_comment = 'Multibyte values sorted in Unicode order';
        $cnt = count($mb);
        $out[] = '';
        $out[] = '/* ' . $mb_comment . ' */';
        $out[] = 'static const unsigned short ' . $name . '_mb[' . $cnt . '] = {';
        $line = '   ';
        out_tab_entries($out, $mb, $cnt);
        $out[] = '};';
    }
    if (!$no_u_ind) {
        $ind_cnt = ($sort[$cnt_sort - 1] >> 8) + 1;
        $out[] = '';
        $out[] = '/* Indexes into Unicode `' . $name . '_u[]` array in blocks of 0x100 */';
        $ind_idx = count($out);
        $out[] = 'static const unsigned short ' . $name . '_u_ind[] = {';
        $line = '   ';
        $i = 0;
        foreach ($sort as $ind => $u) {
            $div = ($u - $sort[0]) >> 8;
            while ($div >= $i) {
                if ($i && $i % 8 === 0) {
                    $out[] = $line;
                    $line = '   ';
                }
                $line .= sprintf(' %5d,', $ind);
                $i++;
            }
        }
        if ($line !== '   ') {
            $out[] = $line;
            $line = '   ';
        }
        $out[] = '};';
        $out[$ind_idx] = 'static const unsigned short ' . $name . '_u_ind[' . $i . '] = {';
    }
}

/* Helper to output special-case URO (Unified Repertoire and Ordering) block (U+4E00-U+9FFF) tables */
function out_uro_tabs(&$out, $name, $tab_uro_u, $tab_uro_mb_ind) {
    $cnt = count($tab_uro_u);
    $out[] = '';
    $out[] = '/* Unicode usage bit-flags for URO (U+4E00-U+9FFF) block */';
    $out[] = 'static const unsigned short ' . $name . '_uro_u[' . $cnt . '] = {';
    out_tab_entries($out, $tab_uro_u, $cnt);
    $cnt = count($tab_uro_mb_ind);
    $out[] = '};';
    $out[] = '';
    $out[] = '/* Multibyte indexes for URO (U+4E00-U+9FFF) block */';
    $out[] = 'static const unsigned short ' . $name . '_uro_mb_ind[' . $cnt . '] = {';
    out_tab_entries($out, $tab_uro_mb_ind, $cnt, true /*not_hex*/);
    $out[] = '};';
}

// BIG5

$out = array();

out_header($out, 'big5', 'Big5', 'https://unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/OTHER/BIG5.TXT', 2021);

$file = 'https://unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/OTHER/BIG5.TXT';

// Read the file.

if (($get = file_get_contents($file)) === false) {
    error_log($error = "$basename: ERROR: Could not read mapping file \"$file\"");
    exit($error . PHP_EOL);
}

$lines = explode("\n", $get);

// Parse the file.

$sort = array();
$mb = array();
foreach ($lines as $line) {
    $line = trim($line);
    if ($line === '' || strncmp($line, '0x', 2) !== 0 || strpos($line, "*** NO MAPPING ***") !== false) {
        continue;
    }
    $matches = array();
    if (preg_match('/^0x([0-9A-F]{4})[ \t]+0x([0-9A-F]{4})[ \t].*$/', $line, $matches)) {
        $d = hexdec($matches[1]);
        $u = hexdec($matches[2]);
        $sort[] = $u;
        $mb[] = $d;
    }
}

array_multisort($sort, $mb);

// Calculate URO (U+4E00-U+9FFF) table
for ($u_i = 0, $cnt = count($sort); $u_i < $cnt && $sort[$u_i] < 0x4E00; $u_i++);

$start_u_i = $u_i;
$big5_uro_u = $big5_uro_mb_ind = array();
$sort_search = array_flip($sort);
for ($u = 0x4E00; $u <= 0x9FAF; $u += 16) {
    $used = 0;
    $next_u_i = $u_i;
    for ($j = 0; $j < 16; $j++) {
        if (isset($sort_search[$u + $j])) {
            $i = $sort_search[$u + $j];
            $used |= 1 << $j;
            $next_u_i = $i + 1;
            $end_u_i = $i;
        }
    }
    $big5_uro_u[] = $used;
    $big5_uro_mb_ind[] = $u_i;
    $u_i = $next_u_i;
}

// Output URO tables
out_uro_tabs($out, 'big5', $big5_uro_u, $big5_uro_mb_ind);

// Remove URO block from Unicode table
array_splice($sort, $start_u_i, $end_u_i - $start_u_i + 1);

// Output Big5 tables

out_tabs($out, 'big5', $sort, $mb, true /*no_ind*/);

$out[] = '';
$out[] = '#endif /* Z_BIG5_H */';

file_put_contents($out_dirname . '/big5.h', implode("\n", $out) . "\n");

// EUC-KR (KS X 1001)

$out = array();

out_header($out, 'ksx1001', 'EUC-KR (KS X 1001)',
            'https://unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/KSC/KSX1001.TXT', 2021);

$file = 'https://unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/KSC/KSX1001.TXT';

// Read the file.

if (($get = file_get_contents($file)) === false) {
    error_log($error = "$basename: ERROR: Could not read mapping file \"$file\"");
    exit($error . PHP_EOL);
}

$lines = explode("\n", $get);

// Parse the file.

$sort = array();
$mb = array();
foreach ($lines as $line) {
    $line = trim($line);
    if ($line === '' || strncmp($line, '0x', 2) !== 0 || strpos($line, "*** NO MAPPING ***") !== false) {
        continue;
    }
    $matches = array();
    if (preg_match('/^0x([0-9A-F]{4})[ \t]+0x([0-9A-F]{4})[ \t].*$/', $line, $matches)) {
        $d = hexdec($matches[1]) + 0x8080; // Convert to EUC-KR
        $u = hexdec($matches[2]);
        $sort[] = $u;
        $mb[] = $d;
    }
}

// Add some characters defined later than in KSX1001.TXT

$sort[] = 0x20AC; // Euro sign added KS X 1001:1998
$mb[] = 0x2266 + 0x8080;

$sort[] = 0xAE; // Registered trademark added KS X 1001:1998
$mb[] = 0x2267 + 0x8080;

$sort[] = 0x327E; // Korean postal code symbol added KS X 1001:2002
$mb[]= 0x2268 + 0x8080;

array_multisort($sort, $mb);

// Calculate URO (U+4E00-U+9FFF) table
for ($u_i = 0, $cnt = count($sort); $u_i < $cnt && $sort[$u_i] < 0x4E00; $u_i++);

$start_u_i = $u_i;
$ksx1001_uro_u = $ksx1001_uro_mb_ind = array();
$sort_search = array_flip($sort);
for ($u = 0x4E00; $u <= 0x9F9F; $u += 16) {
    $used = 0;
    $next_u_i = $u_i;
    for ($j = 0; $j < 16; $j++) {
        if (isset($sort_search[$u + $j])) {
            $i = $sort_search[$u + $j];
            $used |= 1 << $j;
            $next_u_i = $i + 1;
            $end_u_i = $i;
        }
    }
    $ksx1001_uro_u[] = $used;
    $ksx1001_uro_mb_ind[] = $u_i;
    $u_i = $next_u_i;
}

// Output URO tables
out_uro_tabs($out, 'ksx1001', $ksx1001_uro_u, $ksx1001_uro_mb_ind);

// Remove URO block from Unicode table
array_splice($sort, $start_u_i, $end_u_i - $start_u_i + 1);

// Output KS X 1001 tables
out_tabs($out, 'ksx1001', $sort, $mb);

$out[] = '';
$out[] = '#endif /* Z_KSX1001_H */';

file_put_contents($out_dirname . '/ksx1001.h', implode("\n", $out) . "\n");

// Shift JIS

$out = array();

out_header($out, 'sjis', 'Shift JIS', 'https://unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/JIS/SHIFTJIS.TXT', 2009);

$file = 'https://unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/JIS/SHIFTJIS.TXT';

// Read the file.

if (($get = file_get_contents($file)) === false) {
    error_log($error = "$basename: ERROR: Could not read mapping file \"$file\"");
    exit($error . PHP_EOL);
}

$lines = explode("\n", $get);

// Parse the file.

$sort = array();
$mb = array();
foreach ($lines as $line) {
    $line = trim($line);
    if ($line === '' || strncmp($line, '0x', 2) !== 0 || strpos($line, "*** NO MAPPING ***") !== false) {
        continue;
    }
    $matches = array();
    if (preg_match('/^0x([0-9A-F]{2,4})[ \t]+0x([0-9A-F]{4})[ \t].*$/', $line, $matches)) {
        $d = hexdec($matches[1]);
        if ($d < 0x80 && $d != 0x5C && $d != 0x7E) {
            continue;
        }
        $u = hexdec($matches[2]);
        // PUA characters (user-defined range), dealt with programatically by `u_sjis()`
        // See CJKV Information Processing by Ken Lunde, 2nd ed., Table 4-86, p.286
        // https://file.allitebooks.com/20160708/CJKV%20Information%20Processing.pdf
        if ($u >= 0xE000 && $u <= 0xE757) {
            continue;
        }
        $sort[] = $u;
        $mb[] = $d;
    }
}

array_multisort($sort, $mb);

// Calculate URO (U+4E00-U+9FFF) table
for ($u_i = 0, $cnt = count($sort); $u_i < $cnt && $sort[$u_i] < 0x4E00; $u_i++);

$start_u_i = $u_i;
$sjis_uro_u = $sjis_uro_mb_ind = array();
$sort_search = array_flip($sort);
for ($u = 0x4E00; $u <= 0x9FAF; $u += 16) {
    $used = 0;
    $next_u_i = $u_i;
    for ($j = 0; $j < 16; $j++) {
        if (isset($sort_search[$u + $j])) {
            $i = $sort_search[$u + $j];
            $used |= 1 << $j;
            $next_u_i = $i + 1;
            $end_u_i = $i;
        }
    }
    $sjis_uro_u[] = $used;
    $sjis_uro_mb_ind[] = $u_i;
    $u_i = $next_u_i;
}

// Output URO tables
out_uro_tabs($out, 'sjis', $sjis_uro_u, $sjis_uro_mb_ind);

// Remove URO block from Unicode table
array_splice($sort, $start_u_i, $end_u_i - $start_u_i + 1);

// Output Shift JIS tables
out_tabs($out, 'sjis', $sort, $mb, true /*no_ind*/);

$out[] = '';
$out[] = '#endif /* Z_SJIS_H */';

file_put_contents($out_dirname . '/sjis.h', implode("\n", $out) . "\n");

// GB 2312

$out = array();

out_header($out, 'gb2312', 'GB 2312-1980 (EUC-CN)',
            'unicode.org-mappings/EASTASIA/GB/GB2312.TXT', 2009,
            '(see https://haible.de/bruno/charsets/conversion-tables/GB2312.tar.bz2)');

$file = $data_dirname . '/' . 'GB2312.TXT';

// Read the file.

if (($get = file_get_contents($file)) === false) {
    error_log($error = "$basename: ERROR: Could not read mapping file \"$file\"");
    exit($error . PHP_EOL);
}

$lines = explode("\n", $get);

// Parse the file.

$sort = array();
$mb = array();
$in_gb2312 = array();
foreach ($lines as $line) {
    $line = trim($line);
    if ($line === '' || strncmp($line, '0x', 2) !== 0 || strpos($line, "*** NO MAPPING ***") !== false) {
        continue;
    }
    $matches = array();
    if (preg_match('/^0x([0-9A-F]{2,4})[ \t]+0x([0-9A-F]{4})[ \t].*$/', $line, $matches)) {
        $d = hexdec($matches[1]);
        if ($d < 0x80) {
            continue;
        }
        $u = hexdec($matches[2]);
        $sort[] = $u;
        $mb[] = $d + 0x8080; // Convert to EUC-CN
        $in_gb2312[$u] = true;
    }
}

array_multisort($sort, $mb);

// Calculate URO (U+4E00-U+9FFF) table
for ($u_i = 0, $cnt = count($sort); $u_i < $cnt && $sort[$u_i] < 0x4E00; $u_i++);

$start_u_i = $u_i;
$gb2312_uro_u = $gb2312_uro_mb_ind = array();
$sort_search = array_flip($sort);
for ($u = 0x4E00; $u <= 0x9CEF; $u += 16) {
    $used = 0;
    $next_u_i = $u_i;
    for ($j = 0; $j < 16; $j++) {
        if (isset($sort_search[$u + $j])) {
            $i = $sort_search[$u + $j];
            $used |= 1 << $j;
            $next_u_i = $i + 1;
            $end_u_i = $i;
        }
    }
    $gb2312_uro_u[] = $used;
    $gb2312_uro_mb_ind[] = $u_i;
    $u_i = $next_u_i;
}

// Output URO tables
out_uro_tabs($out, 'gb2312', $gb2312_uro_u, $gb2312_uro_mb_ind);

// Remove URO block from Unicode table
array_splice($sort, $start_u_i, $end_u_i - $start_u_i + 1);

// Output GB 2312 tables
out_tabs($out, 'gb2312', $sort, $mb);

$out[] = '';
$out[] = '#endif /* Z_GB2312_H */';

file_put_contents($out_dirname . '/gb2312.h', implode("\n", $out) . "\n");

// GBK

$out = array();

out_header($out, 'gbk', 'GBK, excluding mappings in GB 2312',
            'https://unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP936.TXT');

// Note this has weird 0x80 mapping to U+20AC (EURO SIGN) which needs to be ignored
$file = 'https://unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP936.TXT';

// Read the file.

if (($get = file_get_contents($file)) === false) {
    error_log($error = "$basename: ERROR: Could not read mapping file \"$file\"");
    exit($error . PHP_EOL);
}

$lines = explode("\n", $get);

// Parse the file.

$sort = array();
$mb = array();
$in_gbk = array();
foreach ($lines as $line) {
    $line = trim($line);
    if ($line === '' || strncmp($line, '0x', 2) !== 0 || strpos($line, "*** NO MAPPING ***") !== false) {
        continue;
    }
    $matches = array();
    if (preg_match('/^0x([0-9A-F]{2,4})[ \t]+0x([0-9A-F]{4})[ \t].*$/', $line, $matches)) {
        $d = hexdec($matches[1]);
        if ($d <= 0x80) { // Ignore weird 0x80 mapping to U+20AC (EURO SIGN) if any (present in Unicode Public mapping file)
            continue;
        }
        $u = hexdec($matches[2]);
        $in_gbk[$u] = true;
        if ($u != 0x2015 && isset($in_gb2312[$u])) { // U+2015 mapped differently by GBK
            continue;
        }
        $sort[] = $u;
        $mb[] = $d;
    }
}

array_multisort($sort, $mb);

// Calculate URO (U+4E00-U+9FFF) table
for ($u_i = 0, $cnt = count($sort); $u_i < $cnt && $sort[$u_i] < 0x4E00; $u_i++);

$start_u_i = $u_i;
$gbk_uro_u = $gbk_uro_mb_ind = array();
$sort_search = array_flip($sort);
for ($u = 0x4E00; $u <= 0x9FAF; $u += 16) {
    $used = 0;
    $next_u_i = $u_i;
    for ($j = 0; $j < 16; $j++) {
        if (isset($sort_search[$u + $j])) {
            $i = $sort_search[$u + $j];
            $used |= 1 << $j;
            $next_u_i = $i + 1;
            $end_u_i = $i;
        }
    }
    $gbk_uro_u[] = $used;
    $gbk_uro_mb_ind[] = $u_i;
    $u_i = $next_u_i;
}

// Output URO tables
out_uro_tabs($out, 'gbk', $gbk_uro_u, $gbk_uro_mb_ind);

// Remove URO block from Unicode table
array_splice($sort, $start_u_i, $end_u_i - $start_u_i + 1);

// Output GBK tables
out_tabs($out, 'gbk', $sort, $mb, true /*no_ind*/);

$out[] = '';
$out[] = '#endif /* Z_GBK_H */';

file_put_contents($out_dirname . '/gbk.h', implode("\n", $out) . "\n");

// GB 18030

$out = array();

out_header($out, 'gb18030', 'GB 18030-2005', 'jdk-1.4.2/GB18030.TXT', 2016,
            '(see https://haible.de/bruno/charsets/conversion-tables/GB18030.tar.bz2)');

$file = $data_dirname . '/' . 'GB18030.TXT';

// Read the file.

if (($get = file_get_contents($file)) === false) {
    error_log($error = "$basename: ERROR: Could not read mapping file \"$file\"");
    exit($error . PHP_EOL);
}

$lines = explode("\n", $get);

// Parse the file.

$sort2 = array();
$mb2 = array();
$sort4 = array();
$mb4 = array();

foreach ($lines as $line) {
    $line = trim($line);
    if ($line === '' || strncmp($line, '0x', 2) !== 0 || strpos($line, "*** NO MAPPING ***") !== false) {
        continue;
    }
    if (preg_match('/^0x([0-9A-F]{2,8})[ \t]+0x([0-9A-F]{5})/', $line)) { // Exclude U+10000..10FFFF to save space
        continue;
    }
    $matches = array();
    if (preg_match('/^0x([0-9A-F]{2,8})[ \t]+0x([0-9A-F]{4}).*$/', $line, $matches)) {
        $d = hexdec($matches[1]);
        if ($d < 0x80) {
            continue;
        }
        $u = hexdec($matches[2]);
        // 2-byte extension GB 18030-2005 change, were PUA, see Table 3-37, p.109, Lunde 2nd ed.
        if (($u >= 0x9FB4 && $u <= 0x9FBB) || ($u >= 0xFE10 && $u <= 0xFE19)) {
            //continue;
        }
        // 4-byte extension change, PUA
        if ($u == 0xE7C7) {
            continue;
        }
        if ($d < 0x10000) {
            if (isset($in_gbk[$u])) {
                continue;
            }
            // User-defined, dealt with programatically by `u_gb18030()`
            if ($u >= 0xE000 && $u <= 0xE765) {
                continue;
            }
            $sort2[] = $u;
            $mb2[] = $d;
        } else if ($u < 0x10000) {
            $sort4[] = $u;
            $mb4[] = $d;
        }
    }
}

/* 2-byte extension GB 18030-2005 change, was PUA U+E7C7 below, see Table 3-39, p.111, Lunde 2nd ed. */
$sort2[] = 0x1E3F; $mb2[] = 0xA8BC;

/* 2-byte extension GB 18030-2005 change, were PUA, see Table 3-37, p.109, Lunde 2nd ed. */
$sort2[] = 0x9FB4; $mb2[] = 0xFE59;
$sort2[] = 0x9FB5; $mb2[] = 0xFE61;
$sort2[] = 0x9FB6; $mb2[] = 0xFE66;
$sort2[] = 0x9FB7; $mb2[] = 0xFE67;
$sort2[] = 0x9FB8; $mb2[] = 0xFE6D;
$sort2[] = 0x9FB9; $mb2[] = 0xFE7E;
$sort2[] = 0x9FBA; $mb2[] = 0xFE90;
$sort2[] = 0x9FBB; $mb2[] = 0xFEA0;

$sort2[] = 0xFE10; $mb2[] = 0xA6D9;
$sort2[] = 0xFE11; $mb2[] = 0xA6DB;
$sort2[] = 0xFE12; $mb2[] = 0xA6DA;
$sort2[] = 0xFE13; $mb2[] = 0xA6DC;
$sort2[] = 0xFE14; $mb2[] = 0xA6DD;
$sort2[] = 0xFE15; $mb2[] = 0xA6DE;
$sort2[] = 0xFE16; $mb2[] = 0xA6DF;
$sort2[] = 0xFE17; $mb2[] = 0xA6EC;
$sort2[] = 0xFE18; $mb2[] = 0xA6ED;
$sort2[] = 0xFE19; $mb2[] = 0xA6F3;

/* 4-byte extension PUA */
// Dealt with by `u_gb18030()`
//$sort4[] = 0xE7C7;
//$mb4[] = 0x8135F437;

// Calculate Unicode start/end codepoints mapping to consecutive 4-byte blocks

array_multisort($sort4, $mb4);

$gb18030_4_u_b = array();
$gb18030_4_u_e = array();
$gb18030_4_mb_o = array();

// Start/end points
$prev_u = $begin_u = $sort4[0];
for ($i = 1, $cnt = count($sort4); $i < $cnt; $i++) {
    $u = $sort4[$i];
    if ($u === $prev_u + 1) {
        $prev_u++;
        continue;
    }
    $gb18030_4_u_b[] = $begin_u;
    $gb18030_4_u_e[] = $prev_u;
    $begin_u = $prev_u = $u;
}
$gb18030_4_u_b[] = $begin_u;
$gb18030_4_u_e[] = $prev_u;

// Gaps between blocks
$gb18030_4_mb_o[] = 0;
for ($i = 1, $cnt = count($gb18030_4_u_b); $i < $cnt; $i++) {
    $gb18030_4_mb_o[] = $gb18030_4_u_b[$i] - ($gb18030_4_u_e[$i - 1] + 1) + $gb18030_4_mb_o[count($gb18030_4_mb_o) - 1];
}

// Output GB 18030 tables

array_multisort($sort2, $mb2);
out_tabs($out, 'gb18030_2', $sort2, $mb2, true /*no_ind*/);

// Start codepoints `gb18030_4_u_b` array not needed by `u_gb18030()`
$cnt = count($gb18030_4_u_e);
$out[] = '';
$out[] = '/* End Unicode codepoints of blocks mapping consecutively to 4-byte multibyte blocks */';
$out[] = 'static const unsigned short gb18030_4_u_e[' . $cnt .'] = {';
out_tab_entries($out, $gb18030_4_u_e, $cnt);
$out[] = '};';
$cnt = count($gb18030_4_mb_o);
$out[] = '';
$out[] = '/* Cumulative gaps between Unicode blocks mapping consecutively to 4-byte multibyte blocks,';
$out[] = '   used to adjust multibyte offsets */';
$out[] = 'static const unsigned short gb18030_4_mb_o[' . $cnt .'] = {';
out_tab_entries($out, $gb18030_4_mb_o, $cnt, true /*not_hex*/);
$out[] = '};';

$out[] = '';
$out[] = '#endif /* Z_GB18030_H */';

file_put_contents($out_dirname . '/gb18030.h', implode("\n", $out) . "\n");

/* vim: set ts=4 sw=4 et : */
