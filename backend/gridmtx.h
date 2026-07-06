/*  gridmtx.h - definitions for Grid Matrix */
/*
    libzint - the open source barcode library
    Copyright (C) 2009-2026 Robin Stuart <rstuart114@gmail.com>

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

#ifndef Z_GRIDMTX_H
#define Z_GRIDMTX_H

/* Following 2 tables based on Table 11 - Recommended error correction levels */
static const short gm_recommend_cws[13] = {
     9, 30, 59, 114, 170, 237, 315, 405, 506, 618, 741,  875, 1021
};

static const short gm_max_cws[13] = {
    11, 40, 89, 146, 218, 305, 405, 521, 650, 794, 953, 1125, 1313
};

/* Maximum data codewords per ECL - ECL 1 not permitted for Version 1 */
static const short gm_data_cws[13][5] = {
    /*   90%   80%   70%  60%  50%         Total available codewords (Table 1) */
     {    0,   15,   13,  11,   9 }, /*1   18 */
     {   45,   40,   35,  30,  25 }, /*2   50 */
     {   89,   79,   69,  59,  49 }, /*3   98 */
     {  146,  130,  114,  98,  81 }, /*4   162 */
     {  218,  194,  170, 146, 121 }, /*5   242 */
     {  305,  271,  237, 203, 169 }, /*6   338 */
     {  405,  360,  315, 270, 225 }, /*7   450 */
     {  521,  463,  405, 347, 289 }, /*8   578 */
     {  650,  578,  506, 434, 361 }, /*9   722 */
     {  794,  706,  618, 530, 441 }, /*10  882 */
     {  953,  847,  741, 635, 529 }, /*11  1058 */
     { 1125, 1000,  875, 750, 625 }, /*12  1250 */
     { 1313, 1167, 1021, 875, 729 }, /*13  1458 */
};

/* Table A.1 N1 */
static const char gm_n1[13] = {
    18, 50, 98, 81, 121, 113, 113, 116, 121, 126, 118, 125, 122
};

/* Table A.1 B1 */
static const char gm_b1[13] = {
    1, 1, 1, 2, 2, 2, 2, 3, 2, 7, 5, 10, 6
};

/* Table A.1 B2 */
static const char gm_b2[13] = {
    0, 0, 0, 0, 0, 1, 2, 2, 4, 0, 4, 0, 6
};

/* Table A.1 E1, B3, E2 */
static const char gm_e1b3e2[13][5][3] = {
    /*  E1 B3  E2 */
    { {  0, 0,  0 }, {  3,  1,  0 }, {  5, 1,  0 }, {  7,  1,  0 }, {  9, 1,  0 } }, /* 1 */
    { {  5, 1,  0 }, { 10,  1,  0 }, { 15, 1,  0 }, { 20,  1,  0 }, { 25, 1,  0 } }, /* 2 */
    { {  9, 1,  0 }, { 19,  1,  0 }, { 29, 1,  0 }, { 39,  1,  0 }, { 49, 1,  0 } }, /* 3 */
    { {  8, 2,  0 }, { 16,  2,  0 }, { 24, 2,  0 }, { 32,  2,  0 }, { 41, 1, 40 } }, /* 4 */
    { { 12, 2,  0 }, { 24,  2,  0 }, { 36, 2,  0 }, { 48,  2,  0 }, { 61, 1, 60 } }, /* 5 */
    { { 11, 3,  0 }, { 23,  1, 22 }, { 34, 2, 33 }, { 45,  3,  0 }, { 57, 1, 56 } }, /* 6 */
    { { 12, 1, 11 }, { 23,  2, 22 }, { 34, 3, 33 }, { 45,  4,  0 }, { 57, 1, 56 } }, /* 7 */
    { { 12, 2, 11 }, { 23,  5,  0 }, { 35, 3, 34 }, { 47,  1, 46 }, { 58, 4, 57 } }, /* 8 */
    { { 12, 6,  0 }, { 24,  6,  0 }, { 36, 6,  0 }, { 48,  6,  0 }, { 61, 1, 60 } }, /* 9 */
    { { 13, 4, 12 }, { 26,  1, 25 }, { 38, 5, 37 }, { 51,  2, 50 }, { 63, 7,  0 } }, /* 10 */
    { { 12, 6, 11 }, { 24,  4, 23 }, { 36, 2, 35 }, { 47,  9,  0 }, { 59, 7, 58 } }, /* 11 */
    { { 13, 5, 12 }, { 25, 10,  0 }, { 38, 5, 37 }, { 50, 10,  0 }, { 63, 5, 62 } }, /* 12 */
    { { 13, 1, 12 }, { 25,  3, 24 }, { 37, 5, 36 }, { 49,  7, 48 }, { 61, 9, 60 } }  /* 13 */
};

/* Codewords to switch from `row` mode to `col` mode (Table 9 – Type conversion codes) */
static const short gm_mode_switch[8][6] = {
    /* CHN   NUM   LWR   UPR   MXD   BYT */
    {    1,    2,    3,    4,    5,    7 }, /* 0 */
    {    0, 8161, 8162, 8163, 8164, 8165 }, /* GM_CHINESE */
    { 1019,    0, 1020, 1021, 1022, 1023 }, /* GM_NUMERAL */
    {   28,   29,    0,   30,  124,  126 }, /* GM_LOWER */
    {   28,   29,   30,    0,  124,  126 }, /* GM_UPPER */
    { 1009, 1010, 1011, 1012,    0, 1015 }, /* GM_MIXED */
    {    1,    2,    3,    4,    5,    0 }, /* GM_BYTE */
    { 8160, 1018,   27,   27, 1008,    0 }, /* GM_EOD (end of data) */
};

/* Bit-lengths of above */
static const char gm_mode_len[8][6] = {
    {    4,    4,    4,    4,    4,    4 }, /* 0 */
    {    0,   13,   13,   13,   13,   13 }, /* GM_CHINESE */
    {   10,    0,   10,   10,   10,   10 }, /* GM_NUMERAL */
    {    5,    5,    0,    5,    7,    7 }, /* GM_LOWER */
    {    5,    5,    5,    0,    7,    7 }, /* GM_UPPER */
    {   10,   10,   10,   10,    0,   10 }, /* GM_MIXED */
    {    4,    4,    4,    4,    4,    0 }, /* GM_BYTE */
    {   13,   10,    5,    5,   10,    4 }, /* GM_EOD (end of data) */
};

/* vim: set ts=4 sw=4 et : */
#endif /* Z_GRIDMTX_H */
