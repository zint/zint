#!/bin/bash
# Copyright (C) 2020 - 2021 Robin Stuart <rstuart114@gmail.com>
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
run_bwipp_test "test_auspost" "encode"
run_bwipp_test "test_aztec" "encode"
run_bwipp_test "test_channel" "encode"
run_bwipp_test "test_codablock" "encode"
run_bwipp_test "test_code" "encode"
run_bwipp_test "test_code1" "encode"
run_bwipp_test "test_code128" "encode"
run_bwipp_test "test_code16k" "encode"
run_bwipp_test "test_code49" "encode"
run_bwipp_test "test_composite"
run_bwipp_test "test_dmatrix"
run_bwipp_test "test_dotcode" "encode"
run_bwipp_test "test_gs1" "gs1_reduce"
run_bwipp_test "test_imail" "encode"
run_bwipp_test "test_maxicode" "encode"
run_bwipp_test "test_medical" "encode"
run_bwipp_test "test_pdf417" "encode"
run_bwipp_test "test_plessey" "encode"
run_bwipp_test "test_postal" "encode"
run_bwipp_test "test_qr" "microqr_encode"
run_bwipp_test "test_qr" "rmqr_encode"
run_bwipp_test "test_rss"
run_bwipp_test "test_telepen" "encode"
run_bwipp_test "test_upcean" "encode"
run_bwipp_test "test_ultra" "encode"
