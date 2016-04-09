/*  hanxin.h - definitions for Han Xin code

    libzint - the open source barcode library
    Copyright (C) 2009-2016 Robin Stuart <rstuart114@gmail.com>

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

/* Data from table B1: Data capacity of Han Xin Code */
static int hx_total_codewords[] = {
    25, 37, 50, 54, 69, 84, 100, 117, 136, 155, 161, 181, 203, 225, 249,
    273, 299, 325, 353, 381, 411, 422, 453, 485, 518, 552, 587, 623, 660,
    698, 737, 754, 794, 836, 878, 922, 966, 1011, 1058, 1105, 1126, 1175,
    1224, 1275, 1327, 1380, 1434, 1489, 1513, 1569, 1628, 1686, 1745, 1805,
    1867, 1929, 1992, 2021, 2086, 2151, 2218, 2286, 2355, 2425, 2496, 2528,
    2600, 2673, 2749, 2824, 2900, 2977, 3056, 3135, 3171, 3252, 3334, 3416,
    3500, 3585, 3671, 3758, 3798, 3886
};

static int hx_data_codewords_L1[] = {
    21, 31, 42, 46, 57, 70, 84, 99, 114, 131, 135, 153, 171, 189, 209, 229,
    251, 273, 297, 321, 345, 354, 381, 407, 436, 464, 493, 523, 554, 586, 619,
    634, 666, 702, 738, 774, 812, 849, 888, 929, 946, 987, 1028, 1071, 1115,
    1160, 1204, 1251, 1271, 1317, 1368, 1416, 1465, 1517, 1569, 1621, 1674,
    1697, 1752, 1807, 1864, 1920, 1979, 2037, 2096, 2124, 2184, 2245, 2309,
    2372, 2436, 2501, 2568, 2140, 2633, 2663, 2732, 2800, 2870, 2940, 3011,
    3083, 3156, 3190, 3264
};

static int hx_data_codewords_L2[] = {
    17, 25, 34, 38, 49, 58, 70, 81, 96, 109, 113, 127, 143, 157, 175,191, 209,
    227, 247, 267, 287, 296, 317, 339, 362, 386, 411, 437, 462, 488, 515, 528,
    556, 586, 614, 646, 676, 707, 740, 773, 788, 823, 856, 893, 929, 966, 1004,
    1043, 1059, 1099, 1140, 1180, 1221, 1263, 1307, 1351, 1394, 1415, 1460,
    1505, 1552, 1600, 1649, 1697, 1748, 1770, 1820, 1871, 1925, 1976, 2030,
    2083, 2140, 2195, 2219, 2276, 2334, 2392, 2450, 2509, 2569, 2630, 2658,
    2720
};

static int hx_data_codewords_L3[] = {
    13, 19, 26, 30, 37, 46, 54, 63, 74, 83, 87, 97, 109, 121, 135, 147, 161,
    175, 191, 205, 221, 228, 245, 261, 280, 298, 317, 337, 358, 376, 397, 408,
    428, 452, 474, 498, 522, 545, 572, 597, 608, 635, 660, 689, 717, 746, 774,
    805, 817, 847, 880, 910, 943, 975, 1009, 1041, 1076, 1091, 1126, 1161, 1198,
    1600, 1271, 1309, 1348, 1366, 1404, 1443, 1485, 1524, 1566, 1607,1650, 1693,
    1713, 1756, 1800, 1844, 1890, 1935, 1983, 2030, 2050, 2098
};

static int hx_data_codewords_L4[] = {
    9, 15, 20, 22, 27, 34, 40, 47, 54, 61, 65, 73, 81, 89, 99, 109, 119, 129,
    141, 153, 165, 168, 181, 195, 208, 220, 235, 251, 264, 280, 295, 302, 318,
    334, 352, 368, 386, 405, 424, 441, 450, 490, 509, 531, 552, 574, 595, 605,
    627, 652, 674, 697, 721, 747, 771, 796, 809, 834, 861, 892, 914, 969, 998,
    1012, 1040, 1069, 1099, 1130, 1160, 1191, 1222, 1253, 1269, 1300, 1334,
    1366, 1433, 1469, 1504, 1520, 1554
};

/* Value 'k' from Annex A */
static int hx_module_k[] = {
     0,  0,  0, 14, 16, 16, 17, 18, 19, 20,
    14, 15, 16, 16, 17, 17, 18, 19, 20, 20,
    21, 16, 17, 17, 18, 18, 19, 19, 20, 20,
    21, 17, 17, 18, 18, 19, 19, 19, 20, 20,
    17, 17, 18, 18, 18, 19, 19, 19, 17, 17,
    18, 18, 18, 18, 19, 19, 19, 17, 17, 18,
    18, 18, 18, 19, 19, 17, 17, 17, 18, 18,
    18, 18, 19, 19, 17, 17, 17, 18, 18, 18,
    18, 18, 17, 17
};

/* Value 'r' from Annex A */
static int hx_module_r[] = {
     0,  0,  0, 15, 15, 17, 18, 19, 20, 21,
    15, 15, 15, 17, 17, 19, 19, 19, 19, 21,
    21, 17, 16, 18, 17, 19, 18, 20, 19, 21,
    20, 17, 19, 17, 19, 17, 19, 21, 19, 21,
    18, 20, 17, 19, 21, 18, 20, 22, 17, 19,
    15, 17, 19, 21, 17, 19, 21, 18, 20, 15,
    17, 19, 21, 16, 18, 17, 19, 21, 15, 17,
    19, 21, 15, 17, 18, 20, 22, 15, 17, 19,
    21, 23, 17, 19
};

/* Value of 'm' from Annex A */
static int hx_module_m[] = {
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 7, 7, 7,
    7, 7, 7, 7, 7, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 9, 9, 9, 9, 9, 9,
    9, 9, 10, 10
};