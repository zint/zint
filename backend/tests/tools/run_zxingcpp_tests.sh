#!/bin/bash
# Copyright (C) 2021-2024 Robin Stuart <rstuart114@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause
# vim: set ts=4 sw=4 et :
set -e

function run_zxingcpp_test() {
    if [ -z "$2" ]; then
        echo -e "\n$1"
        backend/tests/$1 -d $(expr 512 + 16 + 32) || exit 1
    else
        echo -e "\n$1 -f $2"
        backend/tests/$1 -f "$2" -d $(expr 512 + 16 + 32) || exit 1
    fi
}

run_zxingcpp_test "test_2of5" "encode"
run_zxingcpp_test "test_aztec" "large"
run_zxingcpp_test "test_aztec" "encode"
run_zxingcpp_test "test_aztec" "encode_segs"
run_zxingcpp_test "test_aztec" "fuzz"
run_zxingcpp_test "test_codablock" "input"
run_zxingcpp_test "test_codablock" "encode"
run_zxingcpp_test "test_codablock" "fuzz"
run_zxingcpp_test "test_code" "encode"
run_zxingcpp_test "test_code128"
run_zxingcpp_test "test_code16k" "input"
run_zxingcpp_test "test_code16k" "encode"
run_zxingcpp_test "test_dmatrix" "input"
run_zxingcpp_test "test_dmatrix" "encode"
run_zxingcpp_test "test_dmatrix" "encode_segs"
run_zxingcpp_test "test_dotcode" "input"
run_zxingcpp_test "test_dotcode" "encode"
run_zxingcpp_test "test_dotcode" "encode_segs"
run_zxingcpp_test "test_hanxin"
run_zxingcpp_test "test_mailmark" "2d_encode"
run_zxingcpp_test "test_maxicode" "large"
run_zxingcpp_test "test_maxicode" "input"
run_zxingcpp_test "test_maxicode" "encode"
run_zxingcpp_test "test_maxicode" "encode_segs"
run_zxingcpp_test "test_maxicode" "best_supported_set"
run_zxingcpp_test "test_medical" "encode"
run_zxingcpp_test "test_pdf417" "reader_init"
run_zxingcpp_test "test_pdf417" "input"
run_zxingcpp_test "test_pdf417" "encode"
run_zxingcpp_test "test_pdf417" "encode_segs"
run_zxingcpp_test "test_pdf417" "fuzz"
run_zxingcpp_test "test_qr"
run_zxingcpp_test "test_rss" "binary_div_modulo_divisor"
run_zxingcpp_test "test_rss" "examples"
run_zxingcpp_test "test_upcean" "upce_input"
run_zxingcpp_test "test_upcean" "encode"
