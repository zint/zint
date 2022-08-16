#!/bin/bash

zint -b PDF417 -d "This Text" --height=4 --heightperrow --scale=0.6 -o images/pdf417_heightperrow.svg
zint --border=10 --box -d "This Text" -w 10 --scale=0.6 -o images/code128_box.svg
zint -b QRCODE --border=1 --box -d "This Text" --quietzones --scale=1 -o images/qrcode_box.svg
zint -d "This Text" --fg=00FF00 --scale=0.6 -o images/code128_green.svg
zint -d "This Text" --fg=00FF0055 --scale=0.6 -o images/code128_green_alpha.svg
zint -d "This Text" --rotate=90 --scale=0.6 -o images/code128_rotate90.svg
zint -b DATAMATRIX --eci=17 -d "€" --scale=1 -o images/datamatrix_euro.svg
zint -b DATAMATRIX --eci=28 -d "\u5E38" --esc --scale=1 -o images/datamatrix_big5.svg
zint -b QRCODE --binary -d "\xE2\x82\xAC\xE5\xB8\xB8" --esc --scale=1 -o images/qrcode_binary_utf8.svg
zint -b CODEONE -d "123456789012345678" --dotty --vers=9 --scale=2 -o images/codeone_s_dotty.svg
zint -b AZTEC --eci=9 -d "Κείμενο" --seg1=7,"Текст" --seg2=20,"文章" --scale=1 -o images/aztec_segs.svg
zint -b DATAMATRIX -d "2nd of 3" --structapp="2,3,5006" --scale=1 -o images/datamatrix_structapp.svg
zint --bold -d "This Text" --small --scale=0.6 -o images/code128_small_bold.svg
zint -b CODE11 -d "9212320967" --scale=0.6 -o images/code11.svg
zint -b C25STANDARD -d "9212320967" --scale=0.6 -o images/c25standard.svg
zint -b C25IATA -d "9212320967" --scale=0.6 -o images/c25iata.svg
zint -b C25IND -d "9212320967" --scale=0.6 -o images/c25ind.svg
zint -b C25INTER --compliantheight -d "9212320967" --scale=0.6 -o images/c25inter.svg
zint -b C25LOGIC -d "9212320967" --scale=0.6 -o images/c25logic.svg
zint -b ITF14 --compliantheight -d "9212320967145" --scale=0.6 -o images/itf14.svg
zint -b ITF14 --box --compliantheight -d "9212320967145" --scale=0.6 -o images/itf14_border0.svg
zint -b DPLEIT -d "9212320967145" --scale=0.6 -o images/dpleit.svg
zint -b DPIDENT -d "91232096712" --scale=0.6 -o images/dpident.svg
zint -b UPCA --compliantheight -d "72527270270" --scale=0.5 -o images/upca.svg
zint -b UPCA --compliantheight -d "72527270270+12345" --scale=0.5 -o images/upca_5.svg
zint -b UPCE --compliantheight -d "1123456" --scale=0.5 -o images/upce.svg
zint -b EANX --compliantheight -d "4512345678906" --scale=0.5 -o images/eanx13.svg
zint -b EANX --compliantheight -d "54321" --scale=0.5 -o images/eanx5.svg
zint -b EANX --compliantheight -d "7432365+54321" --scale=0.5 -o images/eanx8_5.svg
zint -b ISBNX --compliantheight -d "9789295055124" --scale=0.5 -o images/isbnx.svg
zint -b PLESSEY -d "C64" --scale=0.6 -o images/plessey.svg
zint -b MSI_PLESSEY -d "6502" --vers=2 --scale=0.6 -o images/msi_plessey.svg
zint -b TELEPEN --compliantheight -d "Z80" --scale=0.6 -o images/telepen.svg
zint -b TELEPEN_NUM --compliantheight -d "466X33" --scale=0.6 -o images/telepen_num.svg
zint -b CODE39 --compliantheight -d "1A" --vers=1 --scale=0.6 -o images/code39.svg
zint -b EXCODE39 --compliantheight -d "123.45$@fd" --scale=0.6 -o images/excode39.svg
zint -b CODE93 --compliantheight -d "C93" --scale=0.6 -o images/code93.svg
zint -b PZN --compliantheight -d "2758089" --scale=0.6 -o images/pzn.svg
zint -b LOGMARS --compliantheight -d "12345/ABCDE" --vers=1 --scale=0.6 -o images/logmars.svg
zint -b CODE32 --compliantheight -d "14352312" --scale=0.6 -o images/code32.svg
zint -b HIBC_39 --compliantheight -d "14352312" --scale=0.6 -o images/hibc_39.svg
zint -b VIN -d "2FTPX28L0XCA15511" --vers=1 --scale=0.6 -o images/vin.svg
zint -b CODABAR --compliantheight -d "A37859B" --scale=0.6 -o images/codabar.svg
zint -b PHARMA --compliantheight -d "130170" --scale=0.6 -o images/pharma.svg
zint -b CODE128 --bind -d "130170X178" --scale=0.6 -o images/code128.svg
zint -b CODE128B -d "130170X178" --scale=0.6 -o images/code128b.svg
zint -b GS1_128 --compliantheight -d "[01]98898765432106[3202]012345[15]991231" --scale=0.6 -o images/gs1_128.svg
zint -b EAN14 --compliantheight -d "9889876543210" --scale=0.6 -o images/ean14.svg
zint -b NVE18 --compliantheight -d "37612345000001003" --scale=0.6 -o images/nve18.svg
zint -b HIBC_128 -d "A123BJC5D6E71" --scale=0.6 -o images/hibc_128.svg
zint -b DPD --compliantheight -d "%000393206219912345678101040" --scale=0.6 -o images/dpd.svg
zint -b DBAR_OMN --compliantheight -d "0950110153001" --scale=0.6 -o images/dbar_omn.svg
zint -b DBAR_OMN -d "0950110153001" --height=13 --scale=0.6 -o images/dbar_truncated.svg
zint -b DBAR_LTD --compliantheight -d "0950110153001" --scale=0.6 -o images/dbar_ltd.svg
zint -b DBAR_EXP --compliantheight -d "[01]98898765432106[3202]012345[15]991231" --scale=0.6 -o images/dbar_exp.svg
zint -b KOREAPOST -d "923457" --scale=0.6 -o images/koreapost.svg
zint -b CHANNEL -d "453678" --compliantheight --scale=0.6 -o images/channel.svg
zint -b BC412 -d "AQ45670" --compliantheight --scale=0.6 -o images/bc412.svg
zint -d "This" -d "That" --scale=0.6 -o images/code128stacked.svg
zint --notext --bind --separator=2 -d "This" -d "That" --scale=0.6 -o images/code128stacked_sep2.svg
zint -b CODABLOCKF -d "CODABLOCK F Symbology" --rows=3 --scale=0.6 -o images/codablockf.svg
zint -b CODE16K --compliantheight -d "ab0123456789" --scale=0.6 -o images/code16k.svg
zint -b PDF417 -d "PDF417" --scale=0.6 -o images/pdf417.svg
zint -b PDF417COMP -d "PDF417" --scale=0.6 -o images/pdf417comp.svg
zint -b MICROPDF417 -d "12345678" --scale=0.6 -o images/micropdf417.svg
zint -b DBAR_STK --compliantheight -d "9889876543210" --scale=0.6 -o images/dbar_stk.svg
zint -b DBAR_OMNSTK --compliantheight -d "9889876543210" --scale=0.6 -o images/dbar_omnstk.svg
zint -b DBAR_EXPSTK --compliantheight -d "[01]98898765432106[3202]012345[15]991231" --scale=0.6 -o images/dbar_expstk.svg
zint -b CODE49 --compliantheight -d "MULTIPLE ROWS IN CODE 49" --scale=0.6 -o images/code49.svg
zint -b EANX_CC --compliantheight -d "[99]1234-abcd" --mode=1 --primary=331234567890 --scale=0.5 -o images/eanx_cc_a.svg
zint -b EANX_CC --compliantheight -d "[99]1234-abcd" --mode=2 --primary=331234567890 --scale=0.5 -o images/eanx_cc_b.svg
zint -b GS1_128_CC --compliantheight -d "[99]1234-abcd" --mode=3 --primary="[01]03312345678903" --scale=0.5 -o images/gs1_128_cc_c.svg
zint -b PHARMA_TWO --compliantheight -d "29876543" --scale=1 -o images/pharma_two.svg
zint -b POSTNET --compliantheight -d "12345678901" --scale=1 -o images/postnet.svg
zint -b PLANET --compliantheight -d "4012345235636" --scale=1 -o images/planet.svg
zint -b CEPNET --compliantheight -d "12345678" --scale=1 -o images/cepnet.svg
zint -b AUSPOST --compliantheight -d "96184209" --scale=1 -o images/auspost.svg
zint -b AUSROUTE --compliantheight -d "34567890" --scale=1 -o images/ausroute.svg
zint -b AUSREPLY --compliantheight -d "12345678" --scale=1 -o images/ausreply.svg
zint -b AUSREDIRECT --compliantheight -d "98765432" --scale=1 -o images/ausredirect.svg
zint -b KIX --compliantheight -d "2500GG30250" --scale=1 -o images/kix.svg
zint -b RM4SCC --compliantheight -d "W1J0TR01" --scale=1 -o images/rm4scc.svg
zint -b MAILMARK --compliantheight -d "1100000000000XY11" --scale=1 -o images/mailmark.svg
zint -b USPS_IMAIL --compliantheight -d "01234567094987654321-01234" --scale=1 -o images/usps_imail.svg
zint -b JAPANPOST --compliantheight -d "15400233-16-4-205" --scale=1 -o images/japanpost.svg
zint -b HIBC_DM -d "/ACMRN123456/V200912190833" --fast --square --scale=1.2 -o images/hibc_dm.svg
zint -b QRCODE -d "QR Code Symbol" --mask=5 --scale=1.2 -o images/qrcode.svg
zint -b MICROQR -d "01234567" --scale=1.2 -o images/microqr.svg
zint -b RMQR -d "0123456" --scale=1.2 -o images/rmqr.svg
zint -b UPNQR -d "UPNQR\n\n\n\n\nJanez Novak\nDunajska 1\n1000 Ljubljana\n00000008105\n\n\nRENT\nPlačilo najemnine 10/2016\n15.11.2016\nSI56051008010486080\nRF45SBO2010\nNovo podjetje d.o.o.\nLepa cesta 15\n3698 Loški Potok\n188\n                                                                                                                                                                                                                " --esc --scale=1 -o images/upnqr.svg
zint -b MAXICODE -d "1Z00004951\GUPSN\G06X610\G159\G1234567\G1/1\G\GY\G1 MAIN ST\GNY\GNY\R\E" --esc --primary="152382802000000" --scmvv=96 --scale=1 -o images/maxicode.svg
zint -b AZTEC -d "123456789012" --scale=1 -o images/aztec.svg
zint -b AZRUNE -d "125" --scale=1 -o images/azrune.svg
zint -b CODEONE -d "1234567890123456789012" --scale=1 -o images/codeone.svg
zint -b GRIDMATRIX --eci=29 -d "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738" --scale=1 -o images/gridmatrix.svg
zint -b DOTCODE -d "[01]00012345678905[17]201231[10]ABC123456" --gs1 --scale=1 -o images/dotcode.svg
zint -b HANXIN -d "Hanxin Code symbol" --scale=1 -o images/hanxin.svg
zint -b ULTRA -d "HEIMASÍÐA KENNARAHÁSKÓLA ÍSLANDS" --scale=2 -o images/ultra.svg
zint -b FIM --compliantheight -d "C" --scale=1 -o images/fim.svg
zint -b FLAT -d "1304056" --scale=0.6 -o images/flat.svg
zint -b DAFT -d "AAFDTTDAFADTFTTFFFDATFTADTTFFTDAFAFDTF" --height=8.494 --vers=256 --scale=1 -o images/daft_rm4scc.svg
