mkdir isotest_out
cd isotest_out
echo Creating images for Code 49...
zint -o aimbc6_fig1.gif -b 24 -d "MULTIPLE ROWS IN CODE 49"
zint -o aimbc6_fig3.gif -b 24 -d "EXAMPLE 2"
zint -o aimbc6_figb1.gif -b 24 -d "EXAMPLE 2"
echo Creating images for Channel Code...
zint -o aimbc12_fig1.gif -b 140 -d 1234
zint -o aimbc12_fige1.gif -b 140 -d 1000000
zint -o aimbc12_figf1a.gif -b 140 -d 00
zint -o aimbc12_figf1b.gif -b 140 -d 000
zint -o aimbc12_figf1c.gif -b 140 -d 0000
zint -o aimbc12_figf1d.gif -b 140 -d 00000
zint -o aimbc12_figf1e.gif -b 140 -d 000000
zint -o aimbc12_figf1f.gif -b 140 -d 0000000
zint -o aimbc12_figf1g.gif -b 140 -d 02
zint -o aimbc12_figf1h.gif -b 140 -d 004
zint -o aimbc12_figf1i.gif -b 140 -d 0005
zint -o aimbc12_figf1j.gif -b 140 -d 00010
zint -o aimbc12_figf1k.gif -b 140 -d 000100
zint -o aimbc12_figf1l.gif -b 140 -d 0001000
zint -o aimbc12_figf1m.gif -b 140 -d 05
zint -o aimbc12_figf1n.gif -b 140 -d 007
zint -o aimbc12_figf1o.gif -b 140 -d 0010
zint -o aimbc12_figf1p.gif -b 140 -d 00100
zint -o aimbc12_figf1q.gif -b 140 -d 001000
zint -o aimbc12_figf1r.gif -b 140 -d 0010000
zint -o aimbc12_figf1s.gif -b 140 -d 08
zint -o aimbc12_figf1t.gif -b 140 -d 010
zint -o aimbc12_figf1u.gif -b 140 -d 0100
zint -o aimbc12_figf1v.gif -b 140 -d 01000
zint -o aimbc12_figf1w.gif -b 140 -d 010000
zint -o aimbc12_figf1x.gif -b 140 -d 0100000
zint -o aimbc12_figf1y.gif -b 140 -d 10
zint -o aimbc12_figf1z.gif -b 140 -d 100
zint -o aimbc12_figf1aa.gif -b 140 -d 1000
zint -o aimbc12_figf1ab.gif -b 140 -d 10000
zint -o aimbc12_figf1ac.gif -b 140 -d 100000
zint -o aimbc12_figf1ad.gif -b 140 -d 1000000
zint -o aimbc12_figf1ae.gif -b 140 -d 26
zint -o aimbc12_figf1af.gif -b 140 -d 292
zint -o aimbc12_figf1ag.gif -b 140 -d 3493
zint -o aimbc12_figf1ah.gif -b 140 -d 44072
zint -o aimbc12_figf1ai.gif -b 140 -d 576688
zint -o aimbc12_figf1aj.gif -b 140 -d 7742862
echo Creating images for Codablock F...
zint -o aim_cbf_fig1.gif -b 74 -d "CODABLOCK F 34567890123456789010040digit"
echo Creating images for DotCode...
zint -o aimd013_fig1a.gif -b 115 --scale=10 --cols=64 --gs1 -d "[01]00012345678906[17]201231[10]ABC123456"
zint -o aimd013_fig1b.gif -b 115 --scale=10 --cols=29 --gs1 -d "[01]00012345678906[17]201231[10]ABC123456"
zint -o aimd013_fig5.gif -b 115 --scale=10 --cols=23 --gs1 -d "[17]070620[10]ABC123456"
# Fig 6 is same as fig 5
zint -o aimd013_fig7.gif -b 115 --scale=10 -d "2741"
zint -o aimd013_fig8a.gif -b 115 --scale=10 --cols=40 --gs1 -d "[01]00012345678905"
zint -o aimd013_fig8b.gif -b 115 --scale=10 --cols=18 --gs1 -d "[01]00012345678905"
zint -o aimd013_fig8c.gif -b 115 --scale=10 --cols=35 --gs1 -d "[01]00012345678905"
zint -o aimd013_fig8d.gif -b 115 --scale=10 --cols=17 --gs1 -d "[01]00012345678905"
# Fig 10 is same as fig 1b
zint -o aimd013_fig11.gif -b 115 --scale=10 --cols=35 -d "Dots can be Square!"
zint -o aimd013_tableg1.gif -b 115 --scale=10 --cols=13 --gs1 -d "[99]8766"
echo Creating images for Grid Matrix...
zint -o aimd014_fig1.gif -b 142 -d "Grid Matrix"
zint -o aimd014_fig12.gif -b 142 -d "Grid Matrix"
zint -o aimd014_figd1.gif -b 142 -d "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
echo Creating images of Han Xin...
zint -o aimd015_fig1.gif -b 116 --esc --vers=24 -d "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。它的主要技术特色是：1． 具有高度的汉字表示能力和汉字压缩效率。2． 信息容量大。 3． 编码范围广，可以将照片、指纹、掌纹、签字、声音、文字等凡可数字化的信息进行编码。4． 支持加密技术。5．抗污损和畸变能力强。6．修正错误能力强。7 ．可供用户选择的纠错能力。8．容易制作且成本低。9．汉信码支持84个版本，可以由用户自主进行选择，最小码仅有指甲大小。10． 外形美观。"
zint -o aimd015_fig3.gif -b 116 -d "汉信码标准"
zint -o aimd015_fig4.gif -b 116 --esc --vers=4 -d "汉信码标准\r\n中国物品编码中心"
zint -o aimd015_fig5.gif -b 116 --esc --vers=24 -d "汉信码标准\r\n中国物品编码中心\r\n北京网路畅想科技发展有限公司\r\n张成海、赵楠、黄燕滨、罗秋科、王毅、张铎、王越\r\n施煜、边峥、修兴强\r\n汉信码标准\r\n中国物品编码中心\r\n北京网路畅想科技发展有限公司"
zint -o aimd015_fig6.gif -b 116 --esc --vers=40 -d "本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40\r\n"
zint -o aimd015_fig7.gif -b 116 --esc --vers=62 -d "本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40\r\n本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40\r\n本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法RS、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术   122"
zint -o aimd015_fig8.gif -b 116 --esc --vers=84 -d "本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：84\r\n本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：84\r\n本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：84\r\n"
echo Creating images of Ultracode
zint -o aimdtsc15032_intro.gif -b 144 -d "ULTRACODE_123456789!"
zint -o aimdtsc15032_fig1.gif -b 144 --secure=2 -d "ULTRACODE_123456789!"
# Fig G.1 is the same as Fig 1
zint -o aimdtsc15032_figg2.gif -b 144 --secure=2 -d "HEIMASÍÐA KENNARAHÁSKÓLA ÍSLANDS"
zint -o aimdtsc15032_figg3.gif -b 144 --secure=2 -d "אולטרה-קוד1234"
zint -o aimdtsc15032_figg4a.gif -b 144 --secure=2 -d "https://aimglobal.org/jcrv3tX"
zint -o aimdtsc15032_figg6.gif -b 144 --gs1 -d "[01]03453120000011[17]121125[10]ABCD1234"
echo Creating images of EAN/UPC...
zint -o en797_fig1.gif -b 13 -d 501234567890
zint -o en797_fig2.gif -b 13 -d 2012345
zint -o en797_fig3.gif -b 34 -d 01234567890
zint -o en797_fig4.gif -b 37 -d 0012345
zint -o en797_fig5.gif -b 13 -d 501234567890+12
zint -o en797_fig6.gif -b 13 -d 501234567890+86104
zint -o en798_fig1.gif -b 18 -d A37859B
echo Creating images for Code 16k...
zint -o en12323_fig3.gif -b 23 -d "ab0123456789"
echo Creating images for Code 128...
zint -o iso15417_fig1.gif -b 20 -d AIM
echo Creating images for PDF417...
zint -o iso15438_fig1.gif -b 55 -d "PDF417 Symbology Standard"
echo Creating images for Code 39...
zint -o iso16388_fig1.gif -b 8 -d 1A
echo Creating images for Data Matrix...
zint -o iso16022_fig1.gif -b 71 -d "A1B2C3D4E5F6G7H8I9J0K1L2"
zint -o iso16022_figo2.gif -b 71 -d "123456"
zint -o iso16022_figr1.gif -b 71 -d "30Q324343430794<OQQ"
echo Creating images for Maxicode
zint -o iso16023_fig2.gif -b 57 --mode=4 -d "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL..."
zint -o iso16023_figb2.gif -b 57 --mode=2 --primary="152382802840001" -d "[)>\R01\G961Z00004951\GUPSN\G06X610\G159\G1234567\G1/1\G\GY\G634 ALPHA DR\GPITTSBURGH\GPA\R\E" --esc
zint -o iso16023_figh1.gif -b 57 --mode=4 -d "Maxi Code (19 chars)"
echo Creating images for QR Code...
zint -o iso18004_fig1.gif -b 58 -d "QR Code Symbol"
zint -o iso18004_fig2.gif -b 97 -d "01234567"
zint -o iso18004_fig29.gif -b 58 -d "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
zint -o iso18004_figi2.gif -b 58 -d "01234567"
zint -o iso18004_figi4.gif -b 97 -d "01234567"
echo Creating images for Composite DataBar...
zint -o iso24723_fig1.gif -b 133 --primary=1311234567890 -d "[17]010615[10]A123456"
zint -o iso24723_fig2.gif -b 131 --mode=3 --primary="[01]93812345678901" -d "[10]ABCD123456[410]3898765432108"
zint -o iso24723_fig3.gif -b 136 --mode=1 --primary=121230 -d "[15]021231"
zint -o iso24723_fig4.gif -b 130 --mode=1 --primary=1234567 -d "[21]A12345678"
zint -o iso24723_fig5.gif -b 130 --mode=1 --primary=331234567890 -d "[99]1234-abcd"
zint -o iso24723_fig6.gif -b 137 --mode=1 --primary=341234567890 -d "[17]010200"
zint -o iso24723_fig7.gif -b 133 --mode=2 --primary=351234567890 -d "[21]abcdefghijklmnopqrstuv" --gs1nocheck
zint -o iso24723_fig8.gif -b 132 --mode=1 --primary=361234567890 -d "[11]990102"
zint -o iso24723_fig9.gif -b 134 --mode=1 --primary="[01]93712345678904[3103]001234" -d "[91]1A2B3C4D5E"
zint -o iso24723_fig10.gif -b 139 --mode=1 --primary="[01]00012345678905[10]ABCDEF" -d "[21]12345678"
zint -o iso24723_fig11.gif -b 131 --mode=1 --primary="[01]03212345678906" -d "[21]A1B2C3D4E5F6G7H8"
zint -o iso24723_fig12.gif -b 131 --mode=3 --primary="[00]030123456789012340" -d "[02]13012345678909[10]1234567ABCDEFG"
echo Creating images for GS1 DataBar...
zint -o iso24724_fig1.gif -b 29 -d 2001234567890
zint -o iso24724_fig2.gif -b 29 -d 0441234567890
zint -o iso24724_fig4.gif -b 29 --height=11 -d 0001234567890
zint -o iso24724_fig5.gif -b 79 -d 0001234567890
zint -o iso24724_fig6.gif -b 80 -d 0003456789012
zint -o iso24724_fig7.gif -b 30 -d 1501234567890
zint -o iso24724_fig8.gif -b 30 -d 0031234567890
zint -o iso24724_fig10.gif -b 31 -d "[01]98898765432106[3202]012345[15]991231"
zint -o iso24724_fig11.gif -b 31 -d "[01]90012345678903[3103]001750"
zint -o iso24724_fig12.gif -b 81 -d "[01]98898765432106[3202]012345[15]991231"
zint -o iso24724_fig13.gif -b 81 -d "[01]95012345678903[3103]000123"
zint -o iso24724_figf2.gif -b 30 -d 0009876543210
zint -o iso24724_figr3.gif -b 31 -d "[10]12A"
echo Creating images for MicroPDF417...
zint -o iso24728_fig1a.gif -b 84 --cols=1 -d "ABCDEFGHIJKLMNOPQRSTUV"
zint -o iso24728_fig1b.gif -b 84 --cols=2 -d "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCD"
zint -o iso24728_fig1c.gif -b 84 --cols=3 -d "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMN"
zint -o iso24728_fig1d.gif -b 84 --cols=4 -d "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZAB"
echo Creating images for Aztec Code...
zint -o iso24778_figa1a.gif -b 128 -d 0
zint -o iso24778_figa1b.gif -b 128 -d 25
zint -o iso24778_figa1c.gif -b 128 -d 125
zint -o iso24778_figa1d.gif -b 128 -d 255
zint -o iso24778_figg2.gif -b 92 -d "Code 2D!"
zint -o iso24778_figi1a.gif -b 92 -d "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
zint -o iso24778_figi1b.gif -b 92 -d "3333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333"
