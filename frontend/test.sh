echo testing Code 11
zint -o bar01.png -b 1 --height=50 --border=10 -d 87654321
zint -o bar01.eps -b 1 --height=50 --border=10 -d 87654321
zint -o bar01.svg -b 1 --height=50 --border=10 -d 87654321
echo testing Code 2 of 5 Standard
zint -o bar02.png -b 2 --height=50 --border=10 -d 87654321
zint -o bar02.eps -b 2 --height=50 --border=10 -d 87654321
zint -o bar02.svg -b 2 --height=50 --border=10 -d 87654321
echo testing Interleaved 2 of 5
zint -o bar03.png -b 3 --height=50 --border=10 -d 87654321
zint -o bar03.eps -b 3 --height=50 --border=10 -d 87654321
zint -o bar03.svg -b 3 --height=50 --border=10 -d 87654321
echo testing Code 2 of 5 IATA
zint -o bar04.png -b 4 --height=50 --border=10 -d 87654321
zint -o bar04.eps -b 4 --height=50 --border=10 -d 87654321
zint -o bar04.svg -b 4 --height=50 --border=10 -d 87654321
echo testing Code 2 of 5 Data Logic
zint -o bar06.png -b 6 --height=50 --border=10 -d 87654321
zint -o bar06.eps -b 6 --height=50 --border=10 -d 87654321
zint -o bar06.svg -b 6 --height=50 --border=10 -d 87654321
echo testing Code 2 of 5 Industrial
zint -o bar07.png -b 7 --height=50 --border=10 -d 87654321
zint -o bar07.eps -b 7 --height=50 --border=10 -d 87654321
zint -o bar07.svg -b 7 --height=50 --border=10 -d 87654321
echo testing Code 39
zint -o bar08.png -b 8 --height=50 --border=10 -d CODE39
zint -o bar08.eps -b 8 --height=50 --border=10 -d CODE39
zint -o bar08.svg -b 8 --height=50 --border=10 -d CODE39
echo testing Extended Code 39
zint -o bar09.png -b 9 --height=50 --border=10 -d 'Code 39e'
zint -o bar09.eps -b 9 --height=50 --border=10 -d 'Code 39e'
zint -o bar09.svg -b 9 --height=50 --border=10 -d 'Code 39e'
echo testing EAN8
zint -o bar10.png -b 13 --height=50 --border=10 -d 7654321
zint -o bar10.eps -b 13 --height=50 --border=10 -d 7654321
zint -o bar10.svg -b 13 --height=50 --border=10 -d 7654321
echo testing EAN8 - 2 digits add on
zint -o bar11.png -b 13 --height=50 --border=10 -d 7654321+21
zint -o bar11.eps -b 13 --height=50 --border=10 -d 7654321+21
zint -o bar11.svg -b 13 --height=50 --border=10 -d 7654321+21
echo testing EAN8 - 5 digits add-on
zint -o bar12.png -b 13 --height=50 --border=10 -d 7654321+54321
zint -o bar12.eps -b 13 --height=50 --border=10 -d 7654321+54321
zint -o bar12.svg -b 13 --height=50 --border=10 -d 7654321+54321
echo testing EAN13
zint -o bar13.png -b 13 --height=50 --border=10 -d 210987654321
zint -o bar13.eps -b 13 --height=50 --border=10 -d 210987654321
zint -o bar13.svg -b 13 --height=50 --border=10 -d 210987654321
echo testing EAN13 - 2 digits add-on
zint -o bar14.png -b 13 --height=50 --border=10 -d 210987654321+21
zint -o bar14.eps -b 13 --height=50 --border=10 -d 210987654321+21
zint -o bar14.svg -b 13 --height=50 --border=10 -d 210987654321+21
echo testing EAN13 - 5 digits add-on
zint -o bar15.png -b 13 --height=50 --border=10 -d 210987654321+54321
zint -o bar15.eps -b 13 --height=50 --border=10 -d 210987654321+54321
zint -o bar15.svg -b 13 --height=50 --border=10 -d 210987654321+54321
echo testing GS1-128
zint -o bar16.png -b 16 --height=50 --border=10 -d "[01]98898765432106[3202]012345[15]991231"
zint -o bar16.eps -b 16 --height=50 --border=10 -d "[01]98898765432106[3202]012345[15]991231"
zint -o bar16.svg -b 16 --height=50 --border=10 -d "[01]98898765432106[3202]012345[15]991231"
echo testing CodaBar
zint -o bar18.png -b 18 --height=50 --border=10 -d D765432C
zint -o bar18.eps -b 18 --height=50 --border=10 -d D765432C
zint -o bar18.svg -b 18 --height=50 --border=10 -d D765432C
echo testing Code 128
zint -o bar20.png -b 20 --height=50 --border=10 -d 'Code 128'
zint -o bar20.eps -b 20 --height=50 --border=10 -d 'Code 128'
zint -o bar20.svg -b 20 --height=50 --border=10 -d 'Code 128'
echo testing Deutshe Post Leitcode
zint -o bar21.png -b 21 --height=50 --border=10 -d 3210987654321
zint -o bar21.eps -b 21 --height=50 --border=10 -d 3210987654321
zint -o bar21.svg -b 21 --height=50 --border=10 -d 3210987654321
echo testing Deutche Post Identcode
zint -o bar22.png -b 22 --height=50 --border=10 -d 10987654321
zint -o bar22.eps -b 22 --height=50 --border=10 -d 10987654321
zint -o bar22.svg -b 22 --height=50 --border=10 -d 10987654321
echo testing Code 16k
zint -o bar23.png -b 23 --height=50 --border=10 -d "Demonstration Code 16k symbol generated by libzint"
zint -o bar23.eps -b 23 --height=50 --border=10 -d "Demonstration Code 16k symbol generated by libzint"
zint -o bar23.svg -b 23 --height=50 --border=10 -d "Demonstration Code 16k symbol generated by libzint"
zint -o bar23a.png -b 23 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
zint -o bar23a.eps -b 23 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
zint -o bar23a.svg -b 23 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
echo testing Code 49
zint -o bar24.png -b 24 -d "Demonstration Code 49"
zint -o bar24.eps -b 24 -d "Demonstration Code 49"
zint -o bar24.svg -b 24 -d "Demonstration Code 49"
echo testing Code 93
zint -o bar25.png -b 25 --height=50 --border=10 -d 'Code 93'
zint -o bar25.eps -b 25 --height=50 --border=10 -d 'Code 93'
zint -o bar25.svg -b 25 --height=50 --border=10 -d 'Code 93'
echo testing Flattermarken
zint -o bar28.png -b 28 --height=50 --border=10 -d 87654321
zint -o bar28.eps -b 28 --height=50 --border=10 -d 87654321
zint -o bar28.svg -b 28 --height=50 --border=10 -d 87654321
echo testing GS1 DataBar-14
zint -o bar29.png -b 29 --height=33 --border=10 -d 2001234567890
zint -o bar29.eps -b 29 --height=33 --border=10 -d 2001234567890
zint -o bar29.svg -b 29 --height=33 --border=10 -d 2001234567890
echo testing GS1 DataBar Limited
zint -o bar30.png -b 30 --height=50 --border=10 -w 2 -d 31234567890
zint -o bar30.eps -b 30 --height=50 --border=10 -w 2 -d 31234567890
zint -o bar30.svg -b 30 --height=50 --border=10 -w 2 -d 31234567890
echo testing GS1 DataBar Expanded
zint -o bar31.png -b 31 --height=50 --border=10 -d "[01]90012345678908[3103]001750"
zint -o bar31.eps -b 31 --height=50 --border=10 -d "[01]90012345678908[3103]001750"
zint -o bar31.svg -b 31 --height=50 --border=10 -d "[01]90012345678908[3103]001750"
echo testing Telepen Alpha
zint -o bar32.png -b 32 --height=50 --border=10 -d 'Telepen'
zint -o bar32.eps -b 32 --height=50 --border=10 -d 'Telepen'
zint -o bar32.svg -b 32 --height=50 --border=10 -d 'Telepen'
echo testing UPC A
zint -o bar34.png -b 34 --height=50 --border=10 -d 10987654321
zint -o bar34.eps -b 34 --height=50 --border=10 -d 10987654321
zint -o bar34.svg -b 34 --height=50 --border=10 -d 10987654321
echo testing UPC A - 2 digit add-on
zint -o bar35.png -b 34 --height=50 --border=10 -d 10987654321+21
zint -o bar35.eps -b 34 --height=50 --border=10 -d 10987654321+21
zint -o bar35.svg -b 34 --height=50 --border=10 -d 10987654321+21
echo testing UPC A - 5 digit add-on
zint -o bar36.png -b 36 --height=50 --border=10 -d 10987654321+54321
zint -o bar36.eps -b 36 --height=50 --border=10 -d 10987654321+54321
zint -o bar36.svg -b 36 --height=50 --border=10 -d 10987654321+54321
echo testing UPC E
zint -o bar37.png -b 37 --height=50 --border=10 -d 654321
zint -o bar37.eps -b 37 --height=50 --border=10 -d 654321
zint -o bar37.svg -b 37 --height=50 --border=10 -d 654321
echo testing UPC E - 2 digit add-on
zint -o bar38.png -b 37 --height=50 --border=10 -d 654321+21
zint -o bar38.eps -b 37 --height=50 --border=10 -d 654321+21
zint -o bar38.svg -b 37 --height=50 --border=10 -d 654321+21
echo testing UPC E - 5 digit add-on
zint -o bar39.png -b 37 --height=50 --border=10 -d 654321+54321
zint -o bar39.eps -b 37 --height=50 --border=10 -d 654321+54321
zint -o bar39.svg -b 37 --height=50 --border=10 -d 654321+54321
echo testing PostNet-6
zint -o bar41.png -b 40 --border=10 -d 54321
zint -o bar41.eps -b 40 --border=10 -d 54321
zint -o bar41.svg -b 40 --border=10 -d 54321
echo testing PostNet-10
zint -o bar43.png -b 40 --border=10 -d 987654321
zint -o bar43.eps -b 40 --border=10 -d 987654321
zint -o bar43.svg -b 40 --border=10 -d 987654321
echo testing PostNet-12
zint -o bar45.png -b 40 --border=10 -d 10987654321
zint -o bar45.eps -b 40 --border=10 -d 10987654321
zint -o bar45.svg -b 40 --border=10 -d 10987654321
echo testing MSI Code
zint -o bar47.png -b 47 --height=50 --border=10 -d 87654321
zint -o bar47.eps -b 47 --height=50 --border=10 -d 87654321
zint -o bar47.svg -b 47 --height=50 --border=10 -d 87654321
echo testing FIM
zint -o bar49.png -b 49 --height=50 --border=10 -d D
zint -o bar49.eps -b 49 --height=50 --border=10 -d D
zint -o bar49.svg -b 49 --height=50 --border=10 -d D
echo testing LOGMARS
zint -o bar50.png -b 50 --height=50 --border=10 -d LOGMARS
zint -o bar50.eps -b 50 --height=50 --border=10 -d LOGMARS
zint -o bar50.svg -b 50 --height=50 --border=10 -d LOGMARS
echo testing Pharmacode One-Track
zint -o bar51.png -b 51 --height=50 --border=10 -d 123456
zint -o bar51.eps -b 51 --height=50 --border=10 -d 123456
zint -o bar51.svg -b 51 --height=50 --border=10 -d 123456
echo testing Pharmazentralnumber
zint -o bar52.png -b 52 --height=50 --border=10 -d 654321
zint -o bar52.eps -b 52 --height=50 --border=10 -d 654321
zint -o bar52.svg -b 52 --height=50 --border=10 -d 654321
echo testing Pharmacode Two-Track
zint -o bar53.png -b 53 --height=50 --border=10 -d 12345678
zint -o bar53.eps -b 53 --height=50 --border=10 -d 12345678
zint -o bar53.svg -b 53 --height=50 --border=10 -d 12345678
echo testing PDF417
zint -o bar55.png -b 55 --border=10 -d "Demonstration PDF417 symbol generated by libzint"
zint -o bar55.eps -b 55 --border=10 -d "Demonstration PDF417 symbol generated by libzint"
zint -o bar55.svg -b 55 --border=10 -d "Demonstration PDF417 symbol generated by libzint"
echo testing PDF417 Truncated
zint -o bar56.png -b 56 --border=10 -d "Demonstration PDF417 symbol generated by libzint"
zint -o bar56.eps -b 56 --border=10 -d "Demonstration PDF417 symbol generated by libzint"
zint -o bar56.svg -b 56 --border=10 -d "Demonstration PDF417 symbol generated by libzint"
echo testing Maxicode
zint -o bar57.png -b 57 --border=10 --primary="999999999840012" -d "Demonstration Maxicode symbol generated by libzint"
zint -o bar57.eps -b 57 --border=10 --primary="999999999840012" -d "Demonstration Maxicode symbol generated by libzint"
zint -o bar57.svg -b 57 --border=10 --primary="999999999840012" -d "Demonstration Maxicode symbol generated by libzint"
echo testing QR Code
zint -o bar58.png -b 58 --border=10 -d "Demonstration QR Code symbol generated by libzint"
zint -o bar58.eps -b 58 --border=10 -d "Demonstration QR Code symbol generated by libzint"
zint -o bar58.svg -b 58 --border=10 -d "Demonstration QR Code symbol generated by libzint"
zint -o bar58k.png -b 58 --kanji --border=10 -d "画像内の単語を非表示にする"
zint -o bar58k.eps -b 58 --kanji --border=10 -d "画像内の単語を非表示にする"
zint -o bar58k.svg -b 58 --kanji --border=10 -d "画像内の単語を非表示にする"
echo testing Code 128 Subset B
zint -o bar60.png -b 60 --height=50 --border=10 -d 87654321
zint -o bar60.eps -b 60 --height=50 --border=10 -d 87654321
zint -o bar60.svg -b 60 --height=50 --border=10 -d 87654321
echo testing Australian Post Standard Customer
zint -o bar63.png -b 63 --border=10 -d 87654321
zint -o bar63.eps -b 63 --border=10 -d 87654321
zint -o bar63.svg -b 63 --border=10 -d 87654321
echo testing Australian Post Customer 2
zint -o bar64.png -b 63 --border=10 -d 87654321AUSPS
zint -o bar64.eps -b 63 --border=10 -d 87654321AUSPS
zint -o bar64.svg -b 63 --border=10 -d 87654321AUSPS
echo testing Australian Post Customer 3
zint -o bar65.png -b 63 --border=10 -d '87654321 AUSTRALIA'
zint -o bar65.eps -b 63 --border=10 -d '87654321 AUSTRALIA'
zint -o bar65.svg -b 63 --border=10 -d '87654321 AUSTRALIA'
echo testing Australian Post Reply Paid
zint -o bar66.png -b 66 --border=10 -d 87654321
zint -o bar66.eps -b 66 --border=10 -d 87654321
zint -o bar66.svg -b 66 --border=10 -d 87654321
echo testing Australian Post Routing
zint -o bar67.png -b 67 --border=10 -d 87654321
zint -o bar67.eps -b 67 --border=10 -d 87654321
zint -o bar67.svg -b 67 --border=10 -d 87654321
echo testing Australian Post Redirection
zint -o bar68.png -b 68 --border=10 -d 87654321
zint -o bar68.eps -b 68 --border=10 -d 87654321
zint -o bar68.svg -b 68 --border=10 -d 87654321
echo testing ISBN Code
zint -o bar69.png -b 69 --height=50 --border=10 -d 0333638514
zint -o bar69.eps -b 69 --height=50 --border=10 -d 0333638514
zint -o bar69.svg -b 69 --height=50 --border=10 -d 0333638514
echo testing Royal Mail 4 State
zint -o bar70.png -b 70 --border=10 -d ROYALMAIL
zint -o bar70.eps -b 70 --border=10 -d ROYALMAIL
zint -o bar70.svg -b 70 --border=10 -d ROYALMAIL
echo testing Data Matrix
zint -o bar71.png -b 71 --border=10 -d "Demonstration Data Matrix symbol generated by libzint"
zint -o bar71.eps -b 71 --border=10 -d "Demonstration Data Matrix symbol generated by libzint"
zint -o bar71.svg -b 71 --border=10 -d "Demonstration Data Matrix symbol generated by libzint"
zint -o bar71a.png -b 71 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
zint -o bar71a.eps -b 71 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
zint -o bar71a.svg -b 71 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
echo testing Data Matrix ECC 050
zint -o bar71b.png --mode=3 -b 71 --border=10 -d "Demonstration Data Matrix symbol generated by libzint"
zint -o bar71b.eps --mode=3 -b 71 --border=10 -d "Demonstration Data Matrix symbol generated by libzint"
zint -o bar71b.svg --mode=3 -b 71 --border=10 -d "Demonstration Data Matrix symbol generated by libzint"
echo testing EAN-14
zint -o bar72.png -b 72 --height=50 --border=10 -d 3210987654321
zint -o bar72.eps -b 72 --height=50 --border=10 -d 3210987654321
zint -o bar72.svg -b 72 --height=50 --border=10 -d 3210987654321
echo testing Codablock-F
zint -o bar74.png -b 74 --border=10 -d "Demonstration Codablock-F symbol generated by libzint"
zint -o bar74.eps -b 74 --border=10 -d "Demonstration Codablock-F symbol generated by libzint"
zint -o bar74.svg -b 74 --border=10 -d "Demonstration Codablock-F symbol generated by libzint"
zint -o bar74a.png -b 74 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
zint -o bar74a.eps -b 74 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
zint -o bar74a.svg -b 74 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
echo testing NVE-18
zint -o bar75.png -b 75 --height=50 --border=10 -d 76543210987654321
zint -o bar75.eps -b 75 --height=50 --border=10 -d 76543210987654321
zint -o bar75.svg -b 75 --height=50 --border=10 -d 76543210987654321
echo testing Japanese Post
zint -o bar76.png -b 76 --border=10 -d "10000131-3-2-503"
zint -o bar76.eps -b 76 --border=10 -d "10000131-3-2-503"
zint -o bar76.svg -b 76 --border=10 -d "10000131-3-2-503"
echo testing Korea Post
zint -o bar77.png -b 77 --height=50 --border=10 -d 123456
zint -o bar77.eps -b 77 --height=50 --border=10 -d 123456
zint -o bar77.svg -b 77 --height=50 --border=10 -d 123456
echo testing GS1 DataBar Truncated
zint -o bar78.png -b 29 --height=13 --border=10 -d 1234567890
zint -o bar78.eps -b 29 --height=13 --border=10 -d 1234567890
zint -o bar78.svg -b 29 --height=13 --border=10 -d 1234567890
echo testing GS1 DataBar Stacked
zint -o bar79.png -b 79 --border=10 -d 1234567890
zint -o bar79.eps -b 79 --border=10 -d 1234567890
zint -o bar79.svg -b 79 --border=10 -d 1234567890
echo testing GS1 DataBar Stacked Omnidirectional
zint -o bar80.png -b 80 --height=69 --border=10 -d 3456789012
zint -o bar80.eps -b 80 --height=69 --border=10 -d 3456789012
zint -o bar80.svg -b 80 --height=69 --border=10 -d 3456789012
echo testing GS1 DataBar Expanded Stacked
zint -o bar81.png -b 81 --border=10 -d "[01]98898765432106[3202]012345[15]991231"
zint -o bar81.eps -b 81 --border=10 -d "[01]98898765432106[3202]012345[15]991231"
zint -o bar81.svg -b 81 --border=10 -d "[01]98898765432106[3202]012345[15]991231"
echo testing Planet 12 Digit
zint -o bar82.png -b 82 --border=10 -d 10987654321
zint -o bar82.eps -b 82 --border=10 -d 10987654321
zint -o bar82.svg -b 82 --border=10 -d 10987654321
echo testing Planet 14 Digit
zint -o bar83.png -b 82 --border=10 -d 3210987654321
zint -o bar83.eps -b 82 --border=10 -d 3210987654321
zint -o bar83.svg -b 82 --border=10 -d 3210987654321
echo testing Micro PDF417
zint -o bar84.png -b 84 --border=10 -d "Demonstration MicroPDF417 symbol generated by libzint"
zint -o bar84.eps -b 84 --border=10 -d "Demonstration MicroPDF417 symbol generated by libzint"
zint -o bar84.svg -b 84 --border=10 -d "Demonstration MicroPDF417 symbol generated by libzint"
echo testing USPS OneCode 4-State Customer Barcode
zint -o bar85.png -b 85 --border=10 -d 01234567094987654321
zint -o bar85.eps -b 85 --border=10 -d 01234567094987654321
zint -o bar85.svg -b 85 --border=10 -d 01234567094987654321
echo testing Plessey Code with bidirectional reading support
zint -o bar86.png -b 86 --height=50 --border=10 -d 87654321
zint -o bar86.eps -b 86 --height=50 --border=10 -d 87654321
zint -o bar86.svg -b 86 --height=50 --border=10 -d 87654321
echo testing Telepen Numeric
zint -o bar87.png -b 87 --height=50 --border=10 -d 87654321
zint -o bar87.eps -b 87 --height=50 --border=10 -d 87654321
zint -o bar87.svg -b 87 --height=50 --border=10 -d 87654321
echo testing ITF-14
zint -o bar89.png -b 89 --height=50 --border=10 -d 3210987654321
zint -o bar89.eps -b 89 --height=50 --border=10 -d 3210987654321
zint -o bar89.svg -b 89 --height=50 --border=10 -d 3210987654321
echo testing KIX Code
zint -o bar90.png -b 90 --border=10 -d '1231FZ13Xhs'
zint -o bar90.eps -b 90 --border=10 -d '1231FZ13Xhs'
zint -o bar90.svg -b 90 --border=10 -d '1231FZ13Xhs'
echo testing Aztec Code
zint -o bar92.png -b 92 --border=10 -d "Demonstration Aztec Code symbol generated by libzint"
zint -o bar92.eps -b 92 --border=10 -d "Demonstration Aztec Code symbol generated by libzint"
zint -o bar92.svg -b 92 --border=10 -d "Demonstration Aztec Code symbol generated by libzint"
zint -o bar92a.png -b 92 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
zint -o bar92a.eps -b 92 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
zint -o bar92a.svg -b 92 --gs1 --border=10 -d "[01]98898765432106[02]13012345678909[10]1234567ABCDEFG[3202]012345[15]991231"
echo testing DAFT Code
zint -o bar93.png -b 93 --border=10 -d "daftdaftdaftdaftdaftdaftdaftdaftdaft"
zint -o bar93.eps -b 93 --border=10 -d "daftdaftdaftdaftdaftdaftdaftdaftdaft"
zint -o bar93.svg -b 93 --border=10 -d "daftdaftdaftdaftdaftdaftdaftdaftdaft"
echo testing Micro QR Code
zint -o bar97.png -b 97 --border=10 -d "MicroQR Code"
zint -o bar97.eps -b 97 --border=10 -d "MicroQR Code"
zint -o bar97.svg -b 97 --border=10 -d "MicroQR Code"
zint -o bar97k.png -b 97 --kanji --border=10 -d "小さい"
zint -o bar97k.eps -b 97 --kanji --border=10 -d "小さい"
zint -o bar97k.svg -b 97 --kanji --border=10 -d "小さい"
echo testing HIBC LIC 128
zint -o bar98.png -b 98 --border=10 -d "A99912345/9901510X3"
zint -o bar98.eps -b 98 --border=10 -d "A99912345/9901510X3"
zint -o bar98.svg -b 98 --border=10 -d "A99912345/9901510X3"
echo testing HIBC LIC 39
zint -o bar99.png -b 99 --border=10 -d "A123BJC5D6E71"
zint -o bar99.eps -b 99 --border=10 -d "A123BJC5D6E71"
zint -o bar99.svg -b 99 --border=10 -d "A123BJC5D6E71"
echo testing HIBC LIC Data Matrix
zint -o bar102.png -b 102 --border=10 -d "A99912345/9901510X3"
zint -o bar102.eps -b 102 --border=10 -d "A99912345/9901510X3"
zint -o bar102.svg -b 102 --border=10 -d "A99912345/9901510X3"
echo testing HIBC LIC QR-Code
zint -o bar104.png -b 104 --border=10 -d "A99912345/9901510X3"
zint -o bar104.eps -b 104 --border=10 -d "A99912345/9901510X3"
zint -o bar104.svg -b 104 --border=10 -d "A99912345/9901510X3"
echo testing HIBC LIC PDF417
zint -o bar106.png -b 106 --border=10 -d "A99912345/9901510X3"
zint -o bar106.eps -b 106 --border=10 -d "A99912345/9901510X3"
zint -o bar106.svg -b 106 --border=10 -d "A99912345/9901510X3"
echo testing HIBC LIC MicroPDF417
zint -o bar108.png -b 108 --border=10 -d "A99912345/9901510X3"
zint -o bar108.eps -b 108 --border=10 -d "A99912345/9901510X3"
zint -o bar108.svg -b 108 --border=10 -d "A99912345/9901510X3"
echo testing HIBC LIC Codablock F
zint -o bar110.png -b 110 --border=10 -d "A99912345/9901510X3"
zint -o bar110.eps -b 110 --border=10 -d "A99912345/9901510X3"
zint -o bar110.svg -b 110 --border=10 -d "A99912345/9901510X3"
echo testing Aztec Runes
zint -o bar128.png -b 128 --border=10 -d 125
zint -o bar128.eps -b 128 --border=10 -d 125
zint -o bar128.svg -b 128 --border=10 -d 125
echo testing Code 23
zint -o bar129.png -b 129 --border=10 -d "12345678"
zint -o bar129.eps -b 129 --border=10 -d "12345678"
zint -o bar129.svg -b 129 --border=10 -d "12345678"
echo testing EAN-8 Composite with CC-A
zint -o bar130.png -b 130 --height=100 --border=10 --mode=1 --primary=1234567 -d "[21]A12345678"
zint -o bar130.eps -b 130 --height=100 --border=10 --mode=1 --primary=1234567 -d "[21]A12345678"
zint -o bar130.svg -b 130 --height=100 --border=10 --mode=1 --primary=1234567 -d "[21]A12345678"
echo testing EAN-13 Composite with CC-A
zint -o bar130a.png -b 130 --height=100 --border=10 --mode=1 --primary=331234567890 -d "[99]1234-abcd"
zint -o bar130a.eps -b 130 --height=100 --border=10 --mode=1 --primary=331234567890 -d "[99]1234-abcd"
zint -o bar130a.svg -b 130 --height=100 --border=10 --mode=1 --primary=331234567890 -d "[99]1234-abcd"
echo testing UCC/EAN-128 Composite with CC-A
zint -o bar131.png -b 131 --height=100 --border=10 --mode=1 --primary="[01]03212345678906" -d "[10]1234567ABCDEFG"
zint -o bar131.eps -b 131 --height=100 --border=10 --mode=1 --primary="[01]03212345678906" -d "[10]1234567ABCDEFG"
zint -o bar131.svg -b 131 --height=100 --border=10 --mode=1 --primary="[01]03212345678906" -d "[10]1234567ABCDEFG"
echo testing UCC/EAN-128 Composite with CC-C
zint -o bar131a.png -b 131 --height=100 --border=10 --mode=3 --primary="[00]030123456789012340" -d "[02]13012345678909[10]1234567ABCDEFG"
zint -o bar131a.eps -b 131 --height=100 --border=10 --mode=3 --primary="[00]030123456789012340" -d "[02]13012345678909[10]1234567ABCDEFG"
zint -o bar131a.svg -b 131 --height=100 --border=10 --mode=3 --primary="[00]030123456789012340" -d "[02]13012345678909[10]1234567ABCDEFG"
echo testing RSS-14 Composite with CC-A
zint -o bar132.png -b 132 --height=100 --border=10 --mode=1 --primary=361234567890 -d "[11]990102"
zint -o bar132.eps -b 132 --height=100 --border=10 --mode=1 --primary=361234567890 -d "[11]990102"
zint -o bar132.svg -b 132 --height=100 --border=10 --mode=1 --primary=361234567890 -d "[11]990102"
echo testing RSS Limited Composite with CC-B
zint -o bar133.png -b 133 --height=100 --border=10 --mode=2 --primary=351234567890 -d "[21]abcdefghijklmnopqrstuv"
zint -o bar133.eps -b 133 --height=100 --border=10 --mode=2 --primary=351234567890 -d "[21]abcdefghijklmnopqrstuv"
zint -o bar133.svg -b 133 --height=100 --border=10 --mode=2 --primary=351234567890 -d "[21]abcdefghijklmnopqrstuv"
echo testing RSS Expanded Composite with CC-A
zint -o bar134.png -b 134 --height=100 --border=10 --mode=1 --primary="[01]93712345678904[3103]001234" -d "[91]1A2B3C4D5E"
zint -o bar134.eps -b 134 --height=100 --border=10 --mode=1 --primary="[01]93712345678904[3103]001234" -d "[91]1A2B3C4D5E"
zint -o bar134.svg -b 134 --height=100 --border=10 --mode=1 --primary="[01]93712345678904[3103]001234" -d "[91]1A2B3C4D5E"
echo testing UPC-A Composite with CC-A
zint -o bar135.png -b 135 --height=100 --border=10 --mode=1 --primary=10987654321 -d "[15]021231"
zint -o bar135.eps -b 135 --height=100 --border=10 --mode=1 --primary=10987654321 -d "[15]021231"
zint -o bar135.svg -b 135 --height=100 --border=10 --mode=1 --primary=10987654321 -d "[15]021231"
echo testing UPC-E Composite with CC-A
zint -o bar136.png -b 136 --height=100 --border=10 --mode=1 --primary=121230 -d "[15]021231"
zint -o bar136.eps -b 136 --height=100 --border=10 --mode=1 --primary=121230 -d "[15]021231"
zint -o bar136.svg -b 136 --height=100 --border=10 --mode=1 --primary=121230 -d "[15]021231"
echo testing RSS-14 Stacked Composite with CC-A
zint -o bar137.png -b 137 --border=10 --mode=1 --primary=341234567890 -d "[17]010200"
zint -o bar137.eps -b 137 --border=10 --mode=1 --primary=341234567890 -d "[17]010200"
zint -o bar137.svg -b 137 --border=10 --mode=1 --primary=341234567890 -d "[17]010200"
echo testing RSS-14 Stacked Omnidirectional Composite with CC-A
zint -o bar138.png -b 138 --border=10 --mode=1 --primary=341234567890 -d "[17]010200"
zint -o bar138.eps -b 138 --border=10 --mode=1 --primary=341234567890 -d "[17]010200"
zint -o bar138.svg -b 138 --border=10 --mode=1 --primary=341234567890 -d "[17]010200"
echo testing RSS Expanded Stacked Composite with CC-A
zint -o bar139.png -b 139 --height=150 --border=10 --mode=1 --primary="[01]00012345678905[10]ABCDEF" -d "[21]12345678"
zint -o bar139.eps -b 139 --height=150 --border=10 --mode=1 --primary="[01]00012345678905[10]ABCDEF" -d "[21]12345678"
zint -o bar139.svg -b 139 --height=150 --border=10 --mode=1 --primary="[01]00012345678905[10]ABCDEF" -d "[21]12345678"
echo testing Channel Code
zint -o bar140.png -b 140 --height=100 --border=10 -d "12345"
zint -o bar140.eps -b 140 --height=100 --border=10 -d "12345"
zint -o bar140.svg -b 140 --height=100 --border=10 -d "12345"
echo testing Code One
zint -o bar141.png -b 141 --border=10 -d "Demonstration Code One symbol generated by libzint"
zint -o bar141.eps -b 141 --border=10 -d "Demonstration Code One symbol generated by libzint"
zint -o bar141.svg -b 141 --border=10 -d "Demonstration Code One symbol generated by libzint"
echo testing PNG rotation
zint -o barrot0.png -b 130 --height=50 --border=10 --mode=1 --rotate=0 --primary=331234567890+01234 -d "[99]1234-abcd"
zint -o barrot90.png -b 130 --height=50 --border=10 --mode=1 --rotate=90 --primary=331234567890+01234 -d "[99]1234-abcd"
zint -o barrot180.png -b 130 --height=50 --border=10 --mode=1 --rotate=180 --primary=331234567890+01234 -d "[99]1234-abcd"
zint -o barrot270.png -b 130 --height=50 --border=10 --mode=1 --rotate=270 --primary=331234567890+01234 -d "[99]1234-abcd"
echo testing Extended ASCII support
zint -o barext.png --height=50 --border=10 -d "größer"
zint -o barext.svg --height=50 --border=10 -d "größer"
