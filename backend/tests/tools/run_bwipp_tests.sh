#!/bin/bash
# Copyright (C) 2021-2024 Robin Stuart <rstuart114@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause
# vim: set ts=4 sw=4 et :
set -e

function run_bwipp_test() {
    if [ -z "$2" ]; then
        echo -e "\n$1"
        backend/tests/$1 -d $(expr 128 + 16 + 32) || exit 1
    else
        echo -e "\n$1 -f $2"
        backend/tests/$1 -f "$2" -d $(expr 128 + 16 + 32) || exit 1
    fi
}

run_bwipp_test "test_2of5" "encode"
run_bwipp_test "test_auspost" "input"
run_bwipp_test "test_auspost" "encode"
run_bwipp_test "test_aztec" "encode"
run_bwipp_test "test_aztec" "encode_segs"
run_bwipp_test "test_bc412"
run_bwipp_test "test_channel" "encode"
run_bwipp_test "test_codablock" "input"
run_bwipp_test "test_codablock" "encode"
run_bwipp_test "test_codablock" "fuzz"
run_bwipp_test "test_code" "encode"
run_bwipp_test "test_code1" "encode"
run_bwipp_test "test_code1" "encode_segs"
run_bwipp_test "test_code1" "fuzz"
run_bwipp_test "test_code128"
run_bwipp_test "test_code16k" "input"
run_bwipp_test "test_code16k" "encode"
run_bwipp_test "test_code49" "input"
run_bwipp_test "test_code49" "encode"
run_bwipp_test "test_composite"
run_bwipp_test "test_dmatrix" "input"
run_bwipp_test "test_dmatrix" "encode"
run_bwipp_test "test_dmatrix" "encode_segs"
run_bwipp_test "test_dotcode" "input"
run_bwipp_test "test_dotcode" "encode"
run_bwipp_test "test_dotcode" "encode_segs"
run_bwipp_test "test_gs1" "gs1_reduce"
run_bwipp_test "test_imail" "input"
run_bwipp_test "test_imail" "encode"
run_bwipp_test "test_mailmark" "2d_encode"
run_bwipp_test "test_maxicode" "large"
run_bwipp_test "test_maxicode" "input"
run_bwipp_test "test_maxicode" "encode"
run_bwipp_test "test_maxicode" "encode_segs"
run_bwipp_test "test_medical" "input"
run_bwipp_test "test_medical" "encode"
run_bwipp_test "test_pdf417" "input"
run_bwipp_test "test_pdf417" "encode"
run_bwipp_test "test_pdf417" "encode_segs"
run_bwipp_test "test_pdf417" "fuzz"
run_bwipp_test "test_plessey" "encode"
run_bwipp_test "test_postal" "input"
run_bwipp_test "test_postal" "encode"
run_bwipp_test "test_qr"
run_bwipp_test "test_rss"
run_bwipp_test "test_telepen" "encode"
run_bwipp_test "test_upcean" "upce_input"
run_bwipp_test "test_upcean" "encode"
run_bwipp_test "test_ultra" "encode"
