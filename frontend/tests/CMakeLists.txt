# Copyright (C) 2020 Robin Stuart <rstuart114@gmail.com>
# Adapted from qrencode/tests/CMakeLists.txt
# Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
# vim: set ts=4 sw=4 et :

cmake_minimum_required(VERSION 3.5)
project(zint_frontend_tests)

enable_testing()

include(${zint-package_SOURCE_DIR}/cmake/zint_add_test.cmake)

zint_add_test(args test_args)
