<?php
/* Update Zint version number in various files */
/*
    libzint - the open source barcode library
    Copyright (C) 2020 - 2021 Robin Stuart <rstuart114@gmail.com>
*/
/* Run from project directory
 *
 *      php tools/update_version.php ZINT_VERSION_MAJOR ZINT_VERSION_MINOR ZINT_VERSION_RELEASE [ZINT_VERSION_BUILD]
 *
 * e.g. before release
 *      php tools/update_version.php 3 4 5
 * after release
 *      php tools/update_version.php 3 4 5 9
 */
/* vim: set ts=4 sw=4 et : */

$basename = basename(__FILE__);
$dirname = dirname(__FILE__);

$data_dirname = $dirname . '/../';

if ($argc < 4) {
    exit("$basename: ZINT_VERSION_MAJOR ZINT_VERSION_MINOR ZINT_VERSION_RELEASE [ZINT_VERSION_BUILD]" . PHP_EOL);
}

$major = $argv[1];
$minor = $argv[2];
$release = $argv[3];
$build = $argc > 4 ? $argv[4] : "0";
if (!ctype_digit($major) || !ctype_digit($minor) || !ctype_digit($release) || !ctype_digit($build)) {
    exit("$basename: ZINT_VERSION_MAJOR ZINT_VERSION_MINOR ZINT_VERSION_RELEASE [ZINT_VERSION_BUILD] must be numeric" . PHP_EOL);
}
$major = (int) $major;
$minor = (int) $minor;
$release = (int) $release;
$build = (int) $build;
if ($major === 0) {
    exit("$basename: ZINT_VERSION_MAJOR zero" . PHP_EOL);
}
if ($build && $build !== 9) {
    exit("$basename: ZINT_VERSION_BUILD not 9" . PHP_EOL);
}

$v_base_str = $v_str = "$major.$minor.$release";
if ($build) {
    $v_str .= ".$build";
}
$rc_str = "$major,$minor,$release,$build";

function version_replace($to_do, $file, $match_pattern, $replace_pattern, $replace_str) {
    global $basename;

    if (($get = file_get_contents($file)) === false) {
        exit("$basename: ERROR: Could not read file \"$file\"" . PHP_EOL);
    }

    $lines = explode("\n", $get);
    $done = 0;
    foreach ($lines as $li => $line) {
        if (preg_match($match_pattern, $line)) {
            $cnt = 0;
            $lines[$li] = preg_replace($replace_pattern, $replace_str, $line, 1, $cnt);
            if ($cnt === 0 || $lines[$li] === NULL) {
                exit("$basename: ERROR: Could not replace \"$match_pattern\" in file \"$file\"" . PHP_EOL);
            }
            $done++;
        }
        if ($done === $to_do) {
            break;
        }
    }
    if ($done !== $to_do) {
        exit("$basename: ERROR: Only did $done replacements of $to_do in file \"$file\"" . PHP_EOL);
    }
    if (!file_put_contents($file, implode("\n", $lines))) {
        exit("$basename: ERROR: Could not write file \"$file\"" . PHP_EOL);
    }
}

function rc_replace($file, $rc_str) {
    global $basename;

    if (($get = file_get_contents($file)) === false) {
        exit("$basename: ERROR: Could not read file \"$file\"" . PHP_EOL);
    }

    $lines = explode("\n", $get);
    $done = 0;
    foreach ($lines as $li => $line) {
        if (preg_match('/#define[ \t]+VER_FILEVERSION/', $line)) {
            $cnt = 0;
            $lines[$li] = preg_replace('/[0-9,]+/', $rc_str, $line, 1, $cnt);
            if ($cnt === 0 || $lines[$li] === NULL) {
                exit("$basename: ERROR: Could not replace \"$match_pattern\" in file \"$file\"" . PHP_EOL);
            }
            $done++;
        }
        if ($done === 2) {
            break;
        }
    }
    if ($done !== 2) {
        exit("$basename: ERROR: Only did $done replacements of 2 in file \"$file\"" . PHP_EOL);
    }
    if (!file_put_contents($file, implode("\n", $lines))) {
        exit("$basename: ERROR: Could not write file \"$file\"" . PHP_EOL);
    }
}

// CMakeLists.txt

$file = $data_dirname . 'CMakeLists.txt';

if (($get = file_get_contents($file)) === false) {
    exit("$basename: ERROR: Could not read file \"$file\"" . PHP_EOL);
}

$lines = explode("\n", $get);
$done = 0;
foreach ($lines as $li => $line) {
    if (preg_match('/\(ZINT_VERSION_(MAJOR|MINOR|RELEASE|BUILD)/', $line, $matches)) {
        $cnt = 0;
        $mmr = $matches[1] === "MAJOR" ? $major : ($matches[1] === "MINOR" ? $minor : ($matches[1] === "RELEASE" ? $release : $build));
        $lines[$li] = preg_replace('/[0-9]+\)/', $mmr . ')', $line, 1, $cnt);
        if ($cnt === 0 || $lines[$li] === NULL) {
            exit("$basename: ERROR: Could not replace ZINT_VERSION_{$matches[1]} in file \"$file\"" . PHP_EOL);
        }
        $done++;
    }
    if ($done === 4) {
        break;
    }
}
if ($done !== 4) {
    exit("$basename: ERROR: Only did $done replacements of 4 in file \"$file\"" . PHP_EOL);
}
if (!file_put_contents($file, implode("\n", $lines))) {
    exit("$basename: ERROR: Could not write file \"$file\"" . PHP_EOL);
}

// zint.spec

version_replace(1, $data_dirname . 'zint.spec', '/^Version:/', '/[0-9.]+/', $v_base_str);

// zint.nsi

version_replace(1, $data_dirname . 'zint.nsi', '/^!define +PRODUCT_VERSION/', '/"[0-9.]+"/', '"' . $v_str . '"');

// backend/libzint.rc

rc_replace($data_dirname . 'backend/libzint.rc', $rc_str);

// backend/zintconfig.h

$file = $data_dirname . 'backend/zintconfig.h';

if (($get = file_get_contents($file)) === false) {
    exit("$basename: ERROR: Could not read file \"$file\"" . PHP_EOL);
}

$lines = explode("\n", $get);
$done = 0;
foreach ($lines as $li => $line) {
    if (preg_match('/define[ \t]+ZINT_VERSION_(MAJOR|MINOR|RELEASE)[ \t]+/', $line, $matches)) {
        $cnt = 0;
        $mmr = $matches[1] === "MAJOR" ? $major : ($matches[1] === "MINOR" ? $minor : $release);
        $lines[$li] = preg_replace('/[0-9]+/', $mmr, $line, 1, $cnt);
        if ($cnt === 0 || $lines[$li] === NULL) {
            exit("$basename: ERROR: Could not replace ZINT_VERSION_{$matches[1]} in file \"$file\"" . PHP_EOL);
        }
        $done++;
    } elseif (preg_match('/define[ \t]+ZINT_VERSION_BUILD[ \t]+/', $line)) {
        $cnt = 0;
        $lines[$li] = preg_replace('/(BUILD[ \t]+)[0-9]+/', '${1}' . $build, $line, 1, $cnt);
        if ($cnt === 0 || $lines[$li] === NULL) {
            exit("$basename: ERROR: Could not replace ZINT_VERSION_BUILD in file \"$file\"" . PHP_EOL);
        }
        $done++;
    }
    if ($done === 4) {
        break;
    }
}
if ($done !== 4) {
    exit("$basename: ERROR: Only did $done replacements of 4 in file \"$file\"" . PHP_EOL);
}
if (!file_put_contents($file, implode("\n", $lines))) {
    exit("$basename: ERROR: Could not write file \"$file\"" . PHP_EOL);
}

// backend/Makefile.mingw

version_replace(1, $data_dirname . 'backend/Makefile.mingw', '/^ZINT_VERSION:=-DZINT_VERSION=/', '/[0-9.]+/', $v_str);

// backend_tcl/configure.in

version_replace(1, $data_dirname . 'backend_tcl/configure.in', '/^AC_INIT\(\[zint\],[ \t]*\[/', '/[0-9.]+/', $v_base_str);

// backend_tcl/zint_tcl.dsp

version_replace(2, $data_dirname . 'backend_tcl/zint_tcl.dsp', '/ZINT_VERSION="\\\\"/', '/ZINT_VERSION="\\\\"[0-9.]+\\\\""/', 'ZINT_VERSION="\\"' . $v_str . '\\""');

// backend_tcl/lib/zint/pkgIndex.tcl

version_replace(1, $data_dirname . 'backend_tcl/lib/zint/pkgIndex.tcl', '/zint /', '/zint [0-9.]+/', 'zint ' . $v_base_str . '');

// frontend/zint.rc

rc_replace($data_dirname . 'frontend/zint.rc', $rc_str);

// frontend/Makefile.mingw

version_replace(1, $data_dirname . 'frontend/Makefile.mingw', '/^ZINT_VERSION:=-DZINT_VERSION=/', '/[0-9.]+/', $v_str);

// backend_qt/backend_vc8.pro

version_replace(1, $data_dirname . 'backend_qt/backend_vc8.pro', '/^VERSION[ \t]*=/', '/[0-9.]+/', $v_str);
-
// backend_qt/backend_qt.pro

version_replace(1, $data_dirname . 'backend_qt/backend_qt.pro', '/ZINT_VERSION="/', '/[0-9.]+/', $v_str);
version_replace(1, $data_dirname . 'backend_qt/backend_qt.pro', '/^VERSION[ \t]*=/', '/[0-9.]+/', $v_str);

// docs/manual.txt

version_replace(1, $data_dirname . 'docs/manual.txt', '/^The current version of Zint/', '/[0-9]+\.[0-9]+\.[0-9]+(\.[0-9]+)?/', $v_str);

// frontend_qt/res/qtZint.rc

rc_replace($data_dirname . 'frontend_qt/res/qtZint.rc', $rc_str);

// win32/libzint.vcxproj

version_replace(2, $data_dirname . 'win32/libzint.vcxproj', '/ZINT_VERSION="/', '/ZINT_VERSION="[0-9.]+"/', 'ZINT_VERSION="' . $v_str . '"');

// win32/zint.vcxproj

version_replace(2, $data_dirname . 'win32/zint.vcxproj', '/ZINT_VERSION="/', '/ZINT_VERSION="[0-9.]+"/', 'ZINT_VERSION="' . $v_str . '"');

// win32/zint_cmdline_vc6/zint.rc

rc_replace($data_dirname . 'win32/zint_cmdline_vc6/zint.rc', $rc_str);

// win32/zint_cmdline_vc6/zint_cmdline_vc6.dsp

version_replace(2, $data_dirname . 'win32/zint_cmdline_vc6/zint_cmdline_vc6.dsp', '/ZINT_VERSION="/', '/ZINT_VERSION="\\\\"[0-9.]+\\\\""/', 'ZINT_VERSION="\\"' . $v_str . '\\""');

// win32/vs2008/libzint.vcproj

version_replace(3, $data_dirname . 'win32/vs2008/libzint.vcproj', '/ZINT_VERSION=&quot;/', '/&quot;[0-9.]+/', '&quot;' . $v_str);

// win32/vs2008/zint.vcproj

version_replace(3, $data_dirname . 'win32/vs2008/zint.vcproj', '/ZINT_VERSION=&quot;/', '/&quot;[0-9.]+/', '&quot;' . $v_str);

// win32/vs2015/libzint.vcxproj

version_replace(6, $data_dirname . 'win32/vs2015/libzint.vcxproj', '/ZINT_VERSION="/', '/ZINT_VERSION="[0-9.]+"/', 'ZINT_VERSION="' . $v_str . '"');

// win32/vs2015/zint.vcxproj

version_replace(6, $data_dirname . 'win32/vs2015/zint.vcxproj', '/ZINT_VERSION="/', '/ZINT_VERSION="[0-9.]+"/', 'ZINT_VERSION="' . $v_str . '"');

// win32/vs2015/vsx/libzintMD.vcxproj

version_replace(1, $data_dirname . 'win32/vs2015/vsx/libzintMD.vcxproj', '/ZINT_VERSION="/', '/ZINT_VERSION="[0-9.]+"/', 'ZINT_VERSION="' . $v_str . '"');

// win32/vs2019/libzint.vcxproj

version_replace(2, $data_dirname . 'win32/vs2019/libzint.vcxproj', '/ZINT_VERSION="/', '/ZINT_VERSION="[0-9.]+"/', 'ZINT_VERSION="' . $v_str . '"');

// win32/vs2019/zint.vcxproj

version_replace(2, $data_dirname . 'win32/vs2019/zint.vcxproj', '/ZINT_VERSION="/', '/ZINT_VERSION="[0-9.]+"/', 'ZINT_VERSION="' . $v_str . '"');

// Leaving auto-generated files:
//  backend_tcl/configure (PACKAGE_VERSION and PACKAGE_STRING) - generated by autoconf from configure.in
//  frontend_qt/Inno_Setup_qtzint.iss (MyAppVersion)

print PHP_EOL;
print '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!' . PHP_EOL;
print '!!!  REMEMBER: run "autoconf" and "./configure" in "backend_tcl/" !!!' . PHP_EOL;
print '!!!  REMEMBER: update version and date in "ChangeLog"             !!!' . PHP_EOL;
print '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!' . PHP_EOL;
print PHP_EOL;
