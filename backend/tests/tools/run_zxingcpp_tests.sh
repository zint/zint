#!/bin/bash
# Copyright (C) 2021-2022 Robin Stuart <rstuart114@gmail.com>
# vim: set ts=4 sw=4 et :
set -e

function run_zxingcpp_test() {
    echo -e "\n$1 -f $2"
    backend/tests/$1 -f "$2" -d $(expr 512 + 16 + 32) || exit 1
}

run_zxingcpp_test "test_2of5" "encode"
run_zxingcpp_test "test_aztec" "encode"
run_zxingcpp_test "test_aztec" "encode_segs"
run_zxingcpp_test "test_code" "encode"
run_zxingcpp_test "test_code128" "encode"
run_zxingcpp_test "test_dmatrix" "input"
run_zxingcpp_test "test_dmatrix" "encode"
run_zxingcpp_test "test_dmatrix" "encode_segs"
run_zxingcpp_test "test_dotcode" "input"
run_zxingcpp_test "test_dotcode" "encode"
run_zxingcpp_test "test_dotcode" "encode_segs"
run_zxingcpp_test "test_hanxin" "input"
run_zxingcpp_test "test_hanxin" "encode"
run_zxingcpp_test "test_hanxin" "encode_segs"
run_zxingcpp_test "test_maxicode" "input"
run_zxingcpp_test "test_maxicode" "encode"
run_zxingcpp_test "test_maxicode" "encode_segs"
run_zxingcpp_test "test_medical" "encode"
run_zxingcpp_test "test_pdf417" "encode"
run_zxingcpp_test "test_pdf417" "encode_segs"
run_zxingcpp_test "test_qr" "qr_input"
run_zxingcpp_test "test_qr" "qr_optimize"
run_zxingcpp_test "test_qr" "qr_encode"
run_zxingcpp_test "test_qr" "qr_encode_segs"
run_zxingcpp_test "test_rss" "binary_div_modulo_divisor"
run_zxingcpp_test "test_rss" "examples"
run_zxingcpp_test "test_upcean" "upce_input"
run_zxingcpp_test "test_upcean" "encode"