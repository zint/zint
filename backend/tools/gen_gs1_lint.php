<?php
/* Generate GS1 verify include "backend/gs1_lint.h" for "backend/gs1.c" */
/*
    libzint - the open source barcode library
    Copyright (C) 2021 <rstuart114@gmail.com>
*/
/* To create "backend/gs1_lint.h" (from project directory):
 *
 *   php backend/tools/gen_gs1_lint.php > backend/gs1_lint.h
 *
 * or to use local copy of "gs1-format_spec.txt":
 *
 *   php backend/tools/gen_gs1_lint.php -f <local-path>/gs1-format-spec.txt backend/gs1_lint.h
 *
 */
/* vim: set ts=4 sw=4 et : */

$basename = basename(__FILE__);
$dirname = dirname(__FILE__);
$dirdirname = basename(dirname($dirname)) . '/' . basename($dirname);

$opts = getopt('c:f:h:l:t:');

$print_copyright = isset($opts['c']) ? (bool) $opts['c'] : true;
$file = isset($opts['f']) ? $opts['f'] :
        'https://raw.githubusercontent.com/bwipp/postscriptbarcode/master/contrib/development/gs1-format-spec.txt';
$print_h_guard = isset($opts['h']) ? (bool) $opts['h'] : true;
$use_length_only = isset($opts['l']) ? (bool) $opts['l'] : true;
$tab = isset($opts['t']) ? $opts['t'] : '    ';

if (($get = file_get_contents($file)) === false) {
    exit("$basename:" . __LINE__ . " ERROR: Could not read file \"$file\"" . PHP_EOL);
}

$lines = explode("\n", $get);

$spec_ais = $spec_parts = $spec_funcs = $spec_comments = $fixed_ais = array();
$batches = array_fill(0, 100, array());

// Parse the lines into AIs and specs
$line_no = 0;
foreach ($lines as $line) {
    $line_no++;
    if ($line === '' || $line[0] === '#') {
        continue;
    }
    if (!preg_match('/^([0-9]+(?:-[0-9]+)?) +([ *] )([NXC][0-9.][ NXC0-9.,a-z=|]*)(?:# (.+))?$/', $line, $matches)) {
        exit("$basename:" . __LINE__ . " ERROR: Could not parse line $line_no" . PHP_EOL);
    }
    $ai = $matches[1];
    $fixed = trim($matches[2]);
    $spec = preg_replace('/ +dlpkey[=0-9,|]*/', '', trim($matches[3])); // Strip Digital Link primary key info
    $comment = isset($matches[4]) ? trim($matches[4]) : '';

    if (isset($spec_ais[$spec])) {
        $ais = $spec_ais[$spec];
    } else {
        $ais = array();
    }

    if (($hyphen = strpos($ai, '-')) !== false) {
        if ($fixed !== '') {
            $fixed_ais[substr($ai, 0, 2)] = true;
        }
        $ai_s = (int) substr($ai, 0, $hyphen);
        $ai_e = (int) substr($ai, $hyphen + 1);
        $ais[] = array($ai_s, $ai_e);

        $batch_s_idx = (int) ($ai_s / 100);
        $batch_e_idx = (int) ($ai_e / 100);
        if ($batch_s_idx !== $batch_e_idx) {
            if (!in_array($spec, $batches[$batch_s_idx])) {
                $batches[$batch_s_idx][] = $spec;
            }
            if (!in_array($spec, $batches[$batch_e_idx])) {
                $batches[$batch_e_idx][] = $spec;
            }
        } else {
            if (!in_array($spec, $batches[$batch_s_idx])) {
                $batches[$batch_s_idx][] = $spec;
            }
        }
    } else {
        if ($fixed !== '') {
            $fixed_ais[substr($ai, 0, 2)] = true;
        }
        $ai = (int) $ai;
        $ais[] = $ai;
        $batch_idx = (int) ($ai / 100);
        if (!in_array($spec, $batches[$batch_idx])) {
            $batches[$batch_idx][] = $spec;
        }
    }

    $spec_ais[$spec] = $ais;
    if ($comment !== '') {
        if (isset($spec_comments[$spec])) {
            if (!in_array($comment, $spec_comments[$spec])) {
                $spec_comments[$spec][] = $comment;
            }
        } else {
            $spec_comments[$spec] = array($comment);
        }
    }

    $spec_parts[$spec] = array();
    $parts = explode(' ', $spec);
    foreach ($parts as $part) {
        $checkers = explode(',', $part);
        $validator = array_shift($checkers);
        if (!preg_match('/^([NXC])([0-9]+)?(\.\.[0-9|]+)?$/', $validator, $matches)) {
            exit("$basename:" . __LINE__ . " ERROR: Could not parse validator \"$validator\" line $line_no" . PHP_EOL);
        }
        if (count($matches) === 3) {
            $min = $max = (int) $matches[2];
        } else {
            $min = $matches[2] === '' ? 1 : (int) $matches[2];
            $max = (int) substr($matches[3], 2);
        }
        if ($matches[1] === 'N') {
            $validator = "numeric";
        } elseif ($matches[1] === 'X') {
            $validator = "cset82";
        } else {
            $validator = "cset39";
        }
        $spec_parts[$spec][] = array($min, $max, $validator, $checkers);
    }
}

// Calculate total min/maxs and convert the AIs into ranges

foreach ($spec_ais as $spec => $ais) {
    // Total min/maxs
    $total_min = $total_max = 0;
    foreach ($spec_parts[$spec] as list($min, $max)) {
        $total_min += $min;
        $total_max += $max;
    }

    // Sort the AIs
    $sort_ais = array();
    foreach ($ais as $ai) {
        if (is_array($ai)) {
            $sort_ais[] = $ai[0];
        } else {
            $sort_ais[] = $ai;
        }
    }
    array_multisort($sort_ais, $ais);

    // Consolidate contiguous AIs into ranges
    $tmp_ais = array();
    foreach ($ais as $ai) {
        $cnt = count($tmp_ais);
        if ($cnt === 0) {
            $tmp_ais[] = $ai;
        } else {
            $prev_ai = $tmp_ais[$cnt - 1];
            if (is_array($prev_ai)) {
                $prev_s = $prev_ai[0];
                $prev_e = $prev_ai[1];
            } else {
                $prev_e = $prev_s = $prev_ai;
            }
            if (is_array($ai)) {
                $this_s = $ai[0];
                $this_e = $ai[0];
            } else {
                $this_s = $this_e = $ai;
            }
            if ($this_s === $prev_e + 1) {
                $tmp_ais[$cnt - 1] = array($prev_s, $this_e);
            } else {
                $tmp_ais[] = $ai;
            }
        }
    }

    // Unconsolidate ranges of 1 into separate entries
    $ais = array();
    foreach ($tmp_ais as $ai) {
        if (is_array($ai) && $ai[1] === $ai[0] + 1) {
            $ais[] = $ai[0];
            $ais[] = $ai[1];
        } else {
            $ais[] = $ai;
        }
    }

    $spec_ais[$spec] = array($total_min, $total_max, $ais);
}

// Print output

print <<<EOD
/*
 * GS1 AI checker generated by "$dirdirname/$basename" from
 * $file
 */

EOD;

if ($print_copyright) {
print <<<'EOD'
/*
    libzint - the open source barcode library
    Copyright (C) 2021 Robin Stuart <rstuart114@gmail.com>

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


EOD;
}

if ($print_h_guard) {
print <<<'EOD'
#ifndef GS1_LINT_H
#define GS1_LINT_H


EOD;
}

// Print the spec validator/checkers functions

foreach ($spec_parts as $spec => $spec_part) {
    $spec_funcs[$spec] = $spec_func = str_replace(array(' ', '.', ','), '_', strtolower($spec));
    $comment = '';
    if (isset($spec_comments[$spec])) {
        $comment = ' (Used by';
        foreach ($spec_comments[$spec] as $i => $spec_comment) {
            if ($i) {
                if ($i > 3) {
                    $comment .= '...';
                    break;
                }
                $comment .= ', ';
            } else {
                $comment .= ' ';
            }
            $comment .= $spec_comment;
        }
        $comment .= ')';
    }
    print <<<EOD
/* $spec$comment */
static int $spec_func(const unsigned char *data, const int data_len,
$tab$tab{$tab}int *p_err_no, int *p_err_posn, char err_msg[50]) {
{$tab}return 
EOD;

    list($total_min, $total_max) = $spec_ais[$spec];
    if ($total_min === $total_max) {
        print "data_len == $total_max";
    } else {
        print "data_len >= $total_min && data_len <= $total_max";
    }

    if ($use_length_only) {
        // Call checkers checking for length only first
        $length_only_arg = ", 1 /*length_only*/";
        $offset = 0;
        foreach ($spec_part as list($min, $max, $validator, $checkers)) {
            foreach ($checkers as $checker) {
                print <<<EOD

$tab$tab{$tab}&& $checker(data, data_len, $offset, $min, $max, p_err_no, p_err_posn, err_msg$length_only_arg)
EOD;
            }

            $offset += $max;
        }
    }

    // Validator and full checkers
    $length_only_arg = $use_length_only ? ", 0" : "";
    $offset = 0;
    foreach ($spec_part as list($min, $max, $validator, $checkers)) {
        print <<<EOD

$tab$tab{$tab}&& $validator(data, data_len, $offset, $min, $max, p_err_no, p_err_posn, err_msg)
EOD;

        foreach ($checkers as $checker) {
            print <<<EOD

$tab$tab{$tab}&& $checker(data, data_len, $offset, $min, $max, p_err_no, p_err_posn, err_msg$length_only_arg)
EOD;
        }

        $offset += $max;
    }
    print ";\n}\n\n";
}

// Print main routine

print <<<EOD
/* Entry point. Returns 1 on success, 0 on failure: `*p_err_no` set to 1 if unknown AI, 2 if bad data length */
static int gs1_lint(const int ai, const unsigned char *data, const int data_len, int *p_err_no, int *p_err_posn,
$tab$tab{$tab}char err_msg[50]) {

$tab/* Assume data length failure */
$tab*p_err_no = 2;

EOD;

// Split AIs into batches of 100 to lessen the number of comparisons

$not_first_batch = false;
$last_batch_e = -1;
foreach ($batches as $batch => $batch_specs) {
    if (empty($batch_specs)) {
        continue;
    }
    $batch_s = $batch * 100;
    $batch_e = $batch_s + 100;
    if ($not_first_batch) {
        print "\n$tab} else if (ai < $batch_e) {\n\n";
    } else {
        print "\n{$tab}if (ai < $batch_e) {\n\n";
        $not_first_batch = true;
    }
    foreach ($batch_specs as $spec) {
        $total_min = $spec_ais[$spec][0];
        $total_max = $spec_ais[$spec][1];
        $ais = $spec_ais[$spec][2];

        $str = "$tab{$tab}if (";
        print $str;
        $width = strlen($str);

        // Count the applicable AIs
        $ais_cnt = 0;
        foreach ($ais as $ai) {
            if (is_array($ai)) {
                if ($ai[1] < $batch_s || $ai[0] >= $batch_e) {
                    continue;
                }
            } else {
                if ($ai < $batch_s || $ai >= $batch_e) {
                    continue;
                }
            }
            $ais_cnt++;
        }

        // Output
        $not_first_ai = false;
        foreach ($ais as $ai) {
            if (is_array($ai)) {
                if ($ai[1] < $batch_s || $ai[0] >= $batch_e) {
                    continue;
                }
            } else {
                if ($ai < $batch_s || $ai >= $batch_e) {
                    continue;
                }
            }

            $str = '';
            if ($not_first_ai) {
                $str .= " || ";
            } else {
                $not_first_ai = true;
            }
            if (is_array($ai)) {
                if ($ai[0] === $last_batch_e) { // Don't need 1st element of range if excluded by previous batch
                    $str .= "ai <= " . $ai[1];
                } else if ($ai[1] + 1 == $batch_e) { // Don't need 2nd element of range if excluded by this batch
                    $str .= "ai >= " . $ai[0];
                } else {
                    if ($ais_cnt > 1) {
                        $str .= "(ai >= " . $ai[0] . " && ai <= " . $ai[1] . ")";
                    } else {
                        $str .= "ai >= " . $ai[0] . " && ai <= " . $ai[1];
                    }
                }
            } else {
                $str .= "ai == " . $ai;
            }
            if ($width + strlen($str) > 118) {
                print "\n";
                $str2 = "$tab$tab$tab   ";
                print $str2;
                $width = strlen($str2);
            }
            print $str;
            $width += strlen($str);
        }
        $spec_func = $spec_funcs[$spec];
        print <<<EOD
) {
$tab$tab{$tab}return $spec_func(data, data_len, p_err_no, p_err_posn, err_msg);
$tab$tab}

EOD;
    }
    $last_batch_e = $batch_e;
}

print <<<EOD
$tab}

{$tab}/* Unknown AI */
{$tab}*p_err_no = 1;
{$tab}return 0;
}

EOD;

if ($print_h_guard) {
print <<<'EOD'

#endif /* GS1_LINT_H */

EOD;
}
