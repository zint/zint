<?php
/* Generate ISO 3166 include "backend/iso3166.h" for "backend/gs1.c" */
/*
    libzint - the open source barcode library
    Copyright (C) 2021 <rstuart114@gmail.com>
*/
/* To create "backend/iso3166.h" (from project directory):
 *
 *   php backend/tools/gen_iso3166_h.php > backend/iso3166.h
 */
/* vim: set ts=4 sw=4 et : */

$basename = basename(__FILE__);
$dirname = dirname(__FILE__);
$dirdirname = basename(dirname($dirname)) . '/' . basename($dirname);

$opts = getopt('c:h:t:');

$print_copyright = isset($opts['c']) ? (bool) $opts['c'] : true;
$print_h_guard = isset($opts['h']) ? (bool) $opts['h'] : true;
$tab = isset($opts['t']) ? $opts['t'] : '    ';

$numeric = array(
        /*AFG*/   4, /*ALB*/   8, /*ATA*/  10, /*DZA*/  12, /*ASM*/  16, /*AND*/  20, /*AGO*/  24, /*ATG*/  28, /*AZE*/  31, /*ARG*/  32,
        /*AUS*/  36, /*AUT*/  40, /*BHS*/  44, /*BHR*/  48, /*BGD*/  50, /*ARM*/  51, /*BRB*/  52, /*BEL*/  56, /*BMU*/  60, /*BTN*/  64,
        /*BOL*/  68, /*BIH*/  70, /*BWA*/  72, /*BVT*/  74, /*BRA*/  76, /*BLZ*/  84, /*IOT*/  86, /*SLB*/  90, /*VGB*/  92, /*BRN*/  96,
        /*BGR*/ 100, /*MMR*/ 104, /*BDI*/ 108, /*BLR*/ 112, /*KHM*/ 116, /*CMR*/ 120, /*CAN*/ 124, /*CPV*/ 132, /*CYM*/ 136, /*CAF*/ 140,
        /*LKA*/ 144, /*TCD*/ 148, /*CHL*/ 152, /*CHN*/ 156, /*TWN*/ 158, /*CXR*/ 162, /*CCK*/ 166, /*COL*/ 170, /*COM*/ 174, /*MYT*/ 175,
        /*COG*/ 178, /*COD*/ 180, /*COK*/ 184, /*CRI*/ 188, /*HRV*/ 191, /*CUB*/ 192, /*CYP*/ 196, /*CZE*/ 203, /*BEN*/ 204, /*DNK*/ 208,
        /*DMA*/ 212, /*DOM*/ 214, /*ECU*/ 218, /*SLV*/ 222, /*GNQ*/ 226, /*ETH*/ 231, /*ERI*/ 232, /*EST*/ 233, /*FRO*/ 234, /*FLK*/ 238,
        /*SGS*/ 239, /*FJI*/ 242, /*FIN*/ 246, /*ALA*/ 248, /*FRA*/ 250, /*GUF*/ 254, /*PYF*/ 258, /*ATF*/ 260, /*DJI*/ 262, /*GAB*/ 266,
        /*GEO*/ 268, /*GMB*/ 270, /*PSE*/ 275, /*DEU*/ 276, /*GHA*/ 288, /*GIB*/ 292, /*KIR*/ 296, /*GRC*/ 300, /*GRL*/ 304, /*GRD*/ 308,
        /*GLP*/ 312, /*GUM*/ 316, /*GTM*/ 320, /*GIN*/ 324, /*GUY*/ 328, /*HTI*/ 332, /*HMD*/ 334, /*VAT*/ 336, /*HND*/ 340, /*HKG*/ 344,
        /*HUN*/ 348, /*ISL*/ 352, /*IND*/ 356, /*IDN*/ 360, /*IRN*/ 364, /*IRQ*/ 368, /*IRL*/ 372, /*ISR*/ 376, /*ITA*/ 380, /*CIV*/ 384,
        /*JAM*/ 388, /*JPN*/ 392, /*KAZ*/ 398, /*JOR*/ 400, /*KEN*/ 404, /*PRK*/ 408, /*KOR*/ 410, /*KWT*/ 414, /*KGZ*/ 417, /*LAO*/ 418,
        /*LBN*/ 422, /*LSO*/ 426, /*LVA*/ 428, /*LBR*/ 430, /*LBY*/ 434, /*LIE*/ 438, /*LTU*/ 440, /*LUX*/ 442, /*MAC*/ 446, /*MDG*/ 450,
        /*MWI*/ 454, /*MYS*/ 458, /*MDV*/ 462, /*MLI*/ 466, /*MLT*/ 470, /*MTQ*/ 474, /*MRT*/ 478, /*MUS*/ 480, /*MEX*/ 484, /*MCO*/ 492,
        /*MNG*/ 496, /*MDA*/ 498, /*MNE*/ 499, /*MSR*/ 500, /*MAR*/ 504, /*MOZ*/ 508, /*OMN*/ 512, /*NAM*/ 516, /*NRU*/ 520, /*NPL*/ 524,
        /*NLD*/ 528, /*CUW*/ 531, /*ABW*/ 533, /*SXM*/ 534, /*BES*/ 535, /*NCL*/ 540, /*VUT*/ 548, /*NZL*/ 554, /*NIC*/ 558, /*NER*/ 562,
        /*NGA*/ 566, /*NIU*/ 570, /*NFK*/ 574, /*NOR*/ 578, /*MNP*/ 580, /*UMI*/ 581, /*FSM*/ 583, /*MHL*/ 584, /*PLW*/ 585, /*PAK*/ 586,
        /*PAN*/ 591, /*PNG*/ 598, /*PRY*/ 600, /*PER*/ 604, /*PHL*/ 608, /*PCN*/ 612, /*POL*/ 616, /*PRT*/ 620, /*GNB*/ 624, /*TLS*/ 626,
        /*PRI*/ 630, /*QAT*/ 634, /*REU*/ 638, /*ROU*/ 642, /*RUS*/ 643, /*RWA*/ 646, /*BLM*/ 652, /*SHN*/ 654, /*KNA*/ 659, /*AIA*/ 660,
        /*LCA*/ 662, /*MAF*/ 663, /*SPM*/ 666, /*VCT*/ 670, /*SMR*/ 674, /*STP*/ 678, /*SAU*/ 682, /*SEN*/ 686, /*SRB*/ 688, /*SYC*/ 690,
        /*SLE*/ 694, /*SGP*/ 702, /*SVK*/ 703, /*VNM*/ 704, /*SVN*/ 705, /*SOM*/ 706, /*ZAF*/ 710, /*ZWE*/ 716, /*ESP*/ 724, /*SSD*/ 728,
        /*SDN*/ 729, /*ESH*/ 732, /*SUR*/ 740, /*SJM*/ 744, /*SWZ*/ 748, /*SWE*/ 752, /*CHE*/ 756, /*SYR*/ 760, /*TJK*/ 762, /*THA*/ 764,
        /*TGO*/ 768, /*TKL*/ 772, /*TON*/ 776, /*TTO*/ 780, /*ARE*/ 784, /*TUN*/ 788, /*TUR*/ 792, /*TKM*/ 795, /*TCA*/ 796, /*TUV*/ 798,
        /*UGA*/ 800, /*UKR*/ 804, /*MKD*/ 807, /*EGY*/ 818, /*GBR*/ 826, /*GGY*/ 831, /*JEY*/ 832, /*IMN*/ 833, /*TZA*/ 834, /*USA*/ 840,
        /*VIR*/ 850, /*BFA*/ 854, /*URY*/ 858, /*UZB*/ 860, /*VEN*/ 862, /*WLF*/ 876, /*WSM*/ 882, /*YEM*/ 887, /*ZMB*/ 894,
);

$numeric_tab = array();
$val = 0;
$byte = 0;
$max = $numeric[count($numeric) - 1];
for ($i = 0; $i <= $max; $i++) {
    if ($i && $i % 8 == 0) {
        $numeric_tab[$byte++] = $val;
        $val = 0;
    }
    if (in_array($i, $numeric)) {
        $val |= 1 << ($i & 0x7);
    }
}
$numeric_tab[$byte++] = $val;
$numeric_cnt = count($numeric_tab);

$alpha2 = array(
        "AD", "AE", "AF", "AG", "AI", "AL", "AM", "AO", "AQ", "AR",
        "AS", "AT", "AU", "AW", "AX", "AZ", "BA", "BB", "BD", "BE",
        "BF", "BG", "BH", "BI", "BJ", "BL", "BM", "BN", "BO", "BQ",
        "BR", "BS", "BT", "BV", "BW", "BY", "BZ", "CA", "CC", "CD",
        "CF", "CG", "CH", "CI", "CK", "CL", "CM", "CN", "CO", "CR",
        "CU", "CV", "CW", "CX", "CY", "CZ", "DE", "DJ", "DK", "DM",
        "DO", "DZ", "EC", "EE", "EG", "EH", "ER", "ES", "ET", "FI",
        "FJ", "FK", "FM", "FO", "FR", "GA", "GB", "GD", "GE", "GF",
        "GG", "GH", "GI", "GL", "GM", "GN", "GP", "GQ", "GR", "GS",
        "GT", "GU", "GW", "GY", "HK", "HM", "HN", "HR", "HT", "HU",
        "ID", "IE", "IL", "IM", "IN", "IO", "IQ", "IR", "IS", "IT",
        "JE", "JM", "JO", "JP", "KE", "KG", "KH", "KI", "KM", "KN",
        "KP", "KR", "KW", "KY", "KZ", "LA", "LB", "LC", "LI", "LK",
        "LR", "LS", "LT", "LU", "LV", "LY", "MA", "MC", "MD", "ME",
        "MF", "MG", "MH", "MK", "ML", "MM", "MN", "MO", "MP", "MQ",
        "MR", "MS", "MT", "MU", "MV", "MW", "MX", "MY", "MZ", "NA",
        "NC", "NE", "NF", "NG", "NI", "NL", "NO", "NP", "NR", "NU",
        "NZ", "OM", "PA", "PE", "PF", "PG", "PH", "PK", "PL", "PM",
        "PN", "PR", "PS", "PT", "PW", "PY", "QA", "RE", "RO", "RS",
        "RU", "RW", "SA", "SB", "SC", "SD", "SE", "SG", "SH", "SI",
        "SJ", "SK", "SL", "SM", "SN", "SO", "SR", "SS", "ST", "SV",
        "SX", "SY", "SZ", "TC", "TD", "TF", "TG", "TH", "TJ", "TK",
        "TL", "TM", "TN", "TO", "TR", "TT", "TV", "TW", "TZ", "UA",
        "UG", "UM", "US", "UY", "UZ", "VA", "VC", "VE", "VG", "VI",
        "VN", "VU", "WF", "WS", "YE", "YT", "ZA", "ZM", "ZW",
);

$alpha2_tab = array();
$val = 0;
$byte = 0;
for ($i = 0; $i < 26; $i++) {
    for ($j = 0; $j < 26; $j++) {
        $ij = $i * 26 + $j;
        if ($ij && $ij % 8 == 0) {
            $alpha2_tab[$byte++] = $val;
            $val = 0;
        }
        $cc = chr(65 + $i) . chr(65 + $j);
        if (in_array($cc, $alpha2)) {
            $val |= 1 << ($ij & 0x7);
        }
    }
}
$alpha2_tab[$byte++] = $val;
$alpha2_cnt = count($alpha2_tab);

print <<<EOD
/*
 * ISO 3166 country codes generated by "$dirdirname/$basename"
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
#ifndef ISO3166_H
#define ISO3166_H

EOD;
}

print <<<EOD

/* Whether ISO 3166-1 numeric */
static int iso3166_numeric(int cc) {
{$tab}static const unsigned char codes[$numeric_cnt] = {
EOD;

for ($i = 0; $i < $numeric_cnt; $i++) {
    if ($i % 8 == 0) {
        print "\n$tab$tab";
    } else {
        print " ";
    }
    printf("0x%02X,", $numeric_tab[$i]);
}
print <<<EOD

{$tab}};
{$tab}int b = cc >> 3;

{$tab}if (b < 0 || b >= $numeric_cnt) {
{$tab}{$tab}return 0;
{$tab}}
{$tab}return codes[b] & (1 << (cc & 0x7)) ? 1 : 0;
}

/* Whether ISO 3166-1 alpha2 */
static int iso3166_alpha2(const char *cc) {
{$tab}static const unsigned char codes[$alpha2_cnt] = {
EOD;

for ($i = 0; $i < $alpha2_cnt; $i++) {
    if ($i % 8 == 0) {
        print "\n$tab$tab";
    } else {
        print " ";
    }
    printf("0x%02X,", $alpha2_tab[$i]);
}
print <<<EOD

{$tab}};
{$tab}int cc_int;

{$tab}if (cc[0] < 'A' || cc[0] > 'Z' || cc[1] < 'A' || cc[1] > 'Z') {
{$tab}{$tab}return 0;
{$tab}}
{$tab}cc_int = (cc[0] - 'A') * 26 + (cc[1] - 'A');

{$tab}return codes[cc_int >> 3] & (1 << (cc_int & 0x7)) ? 1 : 0;
}

EOD;

if ($print_h_guard) {
print <<<'EOD'

#endif /* ISO3166_H */

EOD;
}
