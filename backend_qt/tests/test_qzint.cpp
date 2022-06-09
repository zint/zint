/***************************************************************************
 *   Copyright (C) 2021-2022 by Robin Stuart <rstuart114@gmail.com>        *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <QtTest/QtTest>
#include "../qzint.h" /* Don't use <qzint.h> in case it's been changed */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

class TestQZint : public QObject
{
    Q_OBJECT

public:
    TestQZint() : m_skipIfFontUsed(false)
    {
    // Qt5 will trigger "detected memory leaks" if font used (libfontconfig) so skip if ASAN enabled
#if QT_VERSION < 0x60000
#  if !defined(__has_feature)
#    define __has_feature(x) 0
#  endif
#  if defined(__SANITIZE_ADDRESS__) || __has_feature(address_sanitizer)
        m_skipIfFontUsed = true;
#  endif
#endif
    }

    virtual ~TestQZint() {} // Seems to be needed to generate vtable

private:
    bool m_skipIfFontUsed; // Hack to get around Qt5 ASAN leaks

private slots:

    void setGetTest()
    {
        Zint::QZint bc;

        int symbology = BARCODE_CODE11;
        bc.setSymbol(symbology);
        QCOMPARE(bc.symbol(), symbology);

        int inputMode = UNICODE_MODE;
        bc.setInputMode(inputMode);
        QCOMPARE(bc.inputMode(), inputMode);

        QString text("text");
        bc.setText(text);
        QCOMPARE(bc.text(), text);
        QCOMPARE(bc.segs().empty(), true);

        std::vector<QString> segTexts;
        std::vector<int> segECIs;
        segTexts.push_back(QString("Τεχτ"));
        segECIs.push_back(9);
        segTexts.push_back(QString("貫やぐ禁"));
        segECIs.push_back(20);
        segTexts.push_back(QString("กขฯ"));
        segECIs.push_back(13);

        std::vector<Zint::QZintSeg> segs;
        for (int i = 0; i < (int) segTexts.size(); i++) {
            segs.push_back(Zint::QZintSeg(segTexts[i]));
            segs.back().m_eci = segECIs[i];
        }

        bc.setSegs(segs);
        QCOMPARE(bc.segs().size(), segs.size());
        for (int i = 0; i < (int) segs.size(); i++) {
            QCOMPARE(bc.segs()[i].m_text, segTexts[i]);
            QCOMPARE(bc.segs()[i].m_eci, segECIs[i]);
        }
        QCOMPARE(bc.text().isEmpty(), true);
        QCOMPARE(bc.eci(), segECIs[0]);

        bc.setText(text);
        QCOMPARE(bc.text(), text);
        QCOMPARE(bc.segs().empty(), true);

        QString primaryMessage("primary message");
        bc.setPrimaryMessage(primaryMessage);
        QCOMPARE(bc.primaryMessage(), primaryMessage);

        float height = 12.345f;
        bc.setHeight(height);
        QCOMPARE(bc.height(), height);

        int option1 = 1;
        bc.setOption1(option1);
        QCOMPARE(bc.option1(), option1);

        int option2 = 2;
        bc.setOption2(option2);
        QCOMPARE(bc.option2(), option2);

        int option3 = 3;
        bc.setOption3(option3);
        QCOMPARE(bc.option3(), option3);

        float scale = 0.678f;
        bc.setScale(scale);
        QCOMPARE(bc.scale(), scale);

        bool dotty = true;
        bc.setDotty(dotty);
        QCOMPARE(bc.dotty(), dotty);

        float dotSize = 1.234f;
        bc.setDotSize(dotSize);
        QCOMPARE(bc.dotSize(), dotSize);

        float guardDescent = 0.678f;
        bc.setGuardDescent(guardDescent);
        QCOMPARE(bc.guardDescent(), guardDescent);

        struct zint_structapp structapp = { 2, 3, "ID" };
        bc.setStructApp(structapp.count, structapp.index, structapp.id);
        QCOMPARE(bc.structAppCount(), structapp.count);
        QCOMPARE(bc.structAppIndex(), structapp.index);
        QCOMPARE(bc.structAppID(), QString(structapp.id));

        QColor fgColor(0x12, 0x34, 0x45, 0x67);
        bc.setFgColor(fgColor);
        QCOMPARE(bc.fgColor(), fgColor);

        QColor bgColor(0x89, 0xAB, 0xCD, 0xEF);
        bc.setBgColor(bgColor);
        QCOMPARE(bc.bgColor(), bgColor);

        bool cmyk = true;
        bc.setCMYK(cmyk);
        QCOMPARE(bc.cmyk(), cmyk);

        int borderTypes[] = { 0, BARCODE_BIND, BARCODE_BOX };
        for (int i = 0; i < ARRAY_SIZE(borderTypes); i++) {
            bc.setBorderType(i);
            QCOMPARE(bc.borderType(), borderTypes[i]);
        }

        int borderWidth = 4;
        bc.setBorderWidth(borderWidth);
        QCOMPARE(bc.borderWidth(), borderWidth);

        int whitespace = 5;
        bc.setWhitespace(whitespace);
        QCOMPARE(bc.whitespace(), whitespace);

        int vWhitespace = 6;
        bc.setVWhitespace(vWhitespace);
        QCOMPARE(bc.vWhitespace(), vWhitespace);

        int fontSettings[] = { 0, BOLD_TEXT, SMALL_TEXT, SMALL_TEXT | BOLD_TEXT };
        for (int i = 0; i < ARRAY_SIZE(fontSettings); i++) {
            bc.setFontSetting(i);
            QCOMPARE(bc.fontSetting(), fontSettings[i]);

            bc.setFontSettingValue(fontSettings[i]);
            QCOMPARE(bc.fontSetting(), fontSettings[i]);
        }
        bc.setFontSetting(ARRAY_SIZE(fontSettings));
        QCOMPARE(bc.fontSetting(), 0);
        bc.setFontSetting(-1);
        QCOMPARE(bc.fontSetting(), 0);

        bc.setFontSettingValue(-1);
        QCOMPARE(bc.fontSetting(), 0);
        bc.setFontSettingValue(CMYK_COLOUR);
        QCOMPARE(bc.fontSetting(), 0);

        bool showText = false;
        bc.setShowText(showText);
        QCOMPARE(bc.showText(), showText);

        bool gsSep = true;
        bc.setGSSep(gsSep);
        QCOMPARE(bc.gsSep(), gsSep);

        bool quietZones = true;
        bc.setQuietZones(quietZones);
        QCOMPARE(bc.quietZones(), quietZones);

        bool noQuietZones = true;
        bc.setNoQuietZones(noQuietZones);
        QCOMPARE(bc.noQuietZones(), noQuietZones);

        bool compliantHeight = true;
        bc.setCompliantHeight(compliantHeight);
        QCOMPARE(bc.compliantHeight(), compliantHeight);

        int rotateAngles[] = { 0, 90, 180, 270 };
        for (int i = 0; i < ARRAY_SIZE(rotateAngles); i++) {
            bc.setRotateAngle(i);
            QCOMPARE(bc.rotateAngle(), rotateAngles[i]);

            bc.setRotateAngleValue(rotateAngles[i]);
            QCOMPARE(bc.rotateAngle(), rotateAngles[i]);
        }
        bc.setRotateAngle(ARRAY_SIZE(rotateAngles));
        QCOMPARE(bc.rotateAngle(), 0);
        bc.setRotateAngle(-1);
        QCOMPARE(bc.rotateAngle(), 0);

        bc.setRotateAngleValue(-1);
        QCOMPARE(bc.rotateAngle(), 0);
        bc.setRotateAngleValue(45);
        QCOMPARE(bc.rotateAngle(), 0);

        int ecis[] = {
             0,  3,  4,  5,  6,  7,  8,  9, 10, 11,
            12, 13, 15, 16, 17, 18, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
            34, 35, 170, 899,
        };
        for (int i = 0; i < ARRAY_SIZE(ecis); i++) {
            bc.setECI(i);
            QCOMPARE(bc.eci(), ecis[i]);

            bc.setECIValue(ecis[i]);
            QCOMPARE(bc.eci(), ecis[i]);
        }
        bc.setECI(ARRAY_SIZE(ecis));
        QCOMPARE(bc.eci(), 0);
        bc.setECI(-1);
        QCOMPARE(bc.eci(), 0);
        // See also setGetECIValueTest()

        bool gs1Parens = true;
        bc.setGS1Parens(gs1Parens);
        QCOMPARE(bc.gs1Parens(), gs1Parens);

        bool gs1NoCheck = true;
        bc.setGS1NoCheck(gs1NoCheck);
        QCOMPARE(bc.gs1NoCheck(), gs1NoCheck);

        bool readerInit = true;
        bc.setReaderInit(readerInit);
        QCOMPARE(bc.readerInit(), readerInit);

        int warnLevel = WARN_FAIL_ALL;
        bc.setWarnLevel(warnLevel);
        QCOMPARE(bc.warnLevel(), warnLevel);

        bool debug = true;
        bc.setDebug(debug);
        QCOMPARE(bc.debug(), debug);

        QCOMPARE(bc.encodedWidth(), 0); // Read-only
        QCOMPARE(bc.encodedRows(), 0); // Read-only
    }

    void setGetECIValueTest_data()
    {
        QTest::addColumn<int>("value");
        QTest::addColumn<int>("eci");

        QTest::newRow("-1") << -1 << 0;
        QTest::newRow("0") << 0 << 0;
        QTest::newRow("1") << 1 << 0;
        QTest::newRow("2") << 2 << 0;
        QTest::newRow("14") << 14 << 0;
        QTest::newRow("19") << 19 << 0;
        QTest::newRow("31") << 31 << 31;
        QTest::newRow("36") << 36 << 0;
        QTest::newRow("169") << 169 << 0;
        QTest::newRow("171") << 171 << 0;
        QTest::newRow("898") << 898 << 0;
        QTest::newRow("900") << 900 << 0;
        QTest::newRow("1000") << 1000 << 0;
    }

    void setGetECIValueTest()
    {
        Zint::QZint bc;

        QFETCH(int, value);
        QFETCH(int, eci);

        bc.setECIValue(value);
        QCOMPARE(bc.eci(), eci);
    }

    void legacyTest()
    {
        Zint::QZint bc;

        int width = 12;
        bc.setWidth(width);
        QCOMPARE(bc.width(), width);
        QCOMPARE(bc.option1(), width);

        int securityLevel = 2;
        bc.setSecurityLevel(securityLevel);
        QCOMPARE(bc.securityLevel(), securityLevel);
        QCOMPARE(bc.option2(), securityLevel);

        int pdf417CodeWords = 123;
        bc.setPdf417CodeWords(pdf417CodeWords);
        QCOMPARE(bc.pdf417CodeWords(), 0); // No-op

        bool hideText = true;
        bc.setHideText(hideText);
        QCOMPARE(bc.showText(), !hideText);

        // No get for target size
    }

    void capTest_data()
    {
        QTest::addColumn<int>("symbology");
        QTest::addColumn<int>("cap_flag");

        QTest::newRow("BARCODE_CODE11") << BARCODE_CODE11
            << (ZINT_CAP_HRT | ZINT_CAP_STACKABLE);
        QTest::newRow("BARCODE_CODE128") << BARCODE_CODE128
            << (ZINT_CAP_HRT | ZINT_CAP_STACKABLE | ZINT_CAP_READER_INIT);
        QTest::newRow("BARCODE_EANX") << BARCODE_EANX
            << (ZINT_CAP_HRT | ZINT_CAP_STACKABLE | ZINT_CAP_EXTENDABLE | ZINT_CAP_QUIET_ZONES
                | ZINT_CAP_COMPLIANT_HEIGHT);
        QTest::newRow("BARCODE_EANX_CC") << BARCODE_EANX_CC
            << (ZINT_CAP_HRT | ZINT_CAP_EXTENDABLE | ZINT_CAP_COMPOSITE | ZINT_CAP_GS1 | ZINT_CAP_QUIET_ZONES
                | ZINT_CAP_COMPLIANT_HEIGHT);
        QTest::newRow("BARCODE_QRCODE") << BARCODE_QRCODE
            << (ZINT_CAP_ECI | ZINT_CAP_GS1 | ZINT_CAP_DOTTY | ZINT_CAP_FIXED_RATIO | ZINT_CAP_FULL_MULTIBYTE
                | ZINT_CAP_MASK | ZINT_CAP_STRUCTAPP);
    }

    void capTest()
    {
        Zint::QZint bc;

        QFETCH(int, symbology);
        QFETCH(int, cap_flag);

        bc.setSymbol(symbology);
        QCOMPARE(bc.hasHRT(), (cap_flag & ZINT_CAP_HRT) != 0);
        QCOMPARE(bc.isStackable(), (cap_flag & ZINT_CAP_STACKABLE) != 0);
        QCOMPARE(bc.isExtendable(), (cap_flag & ZINT_CAP_EXTENDABLE) != 0);
        QCOMPARE(bc.isComposite(), (cap_flag & ZINT_CAP_COMPOSITE) != 0);
        QCOMPARE(bc.supportsECI(), (cap_flag & ZINT_CAP_ECI) != 0);
        QCOMPARE(bc.supportsGS1(), (cap_flag & ZINT_CAP_GS1) != 0);
        QCOMPARE(bc.isDotty(), (cap_flag & ZINT_CAP_DOTTY) != 0);
        QCOMPARE(bc.hasDefaultQuietZones(), (cap_flag & ZINT_CAP_QUIET_ZONES) != 0);
        QCOMPARE(bc.isFixedRatio(), (cap_flag & ZINT_CAP_FIXED_RATIO) != 0);
        QCOMPARE(bc.supportsReaderInit(), (cap_flag & ZINT_CAP_READER_INIT) != 0);
        QCOMPARE(bc.supportsFullMultibyte(), (cap_flag & ZINT_CAP_FULL_MULTIBYTE) != 0);
        QCOMPARE(bc.hasMask(), (cap_flag & ZINT_CAP_MASK) != 0);
        QCOMPARE(bc.supportsStructApp(), (cap_flag & ZINT_CAP_STRUCTAPP) != 0);
        QCOMPARE(bc.hasCompliantHeight(), (cap_flag & ZINT_CAP_COMPLIANT_HEIGHT) != 0);
    }

    void renderTest_data()
    {
        QTest::addColumn<int>("symbology");
        QTest::addColumn<QString>("text");
        QTest::addColumn<int>("getError");
        QTest::addColumn<QString>("error_message");
        QTest::addColumn<int>("encodedWidth");
        QTest::addColumn<int>("encodedRows");

        QTest::newRow("BARCODE_QRCODE") << BARCODE_QRCODE << "1234" << 0 << "" << 21 << 21;
        if (!m_skipIfFontUsed) {
            QTest::newRow("BARCODE_QRCODE no text") << BARCODE_QRCODE << "" << ZINT_ERROR_INVALID_DATA << "Error 778: No input data (segment 0 empty)" << 0 << 0;
        }
    }

    void renderTest()
    {
        Zint::QZint bc;

        bool bRet;
        QPainter painter;
        constexpr int width = 100, height = 100;
        QPixmap paintDevice(width, height);
        QRectF paintRect(0, 0, width, height);
        Zint::QZint::AspectRatioMode mode;
        QSignalSpy spyEncoded(&bc, SIGNAL(encoded()));
        QSignalSpy spyErrored(&bc, SIGNAL(errored()));

        mode = Zint::QZint::AspectRatioMode::KeepAspectRatio; // Legacy - ignored

        QFETCH(int, symbology);
        QFETCH(QString, text);
        QFETCH(int, getError);
        QFETCH(QString, error_message);
        QFETCH(int, encodedWidth);
        QFETCH(int, encodedRows);

        bc.setSymbol(symbology);
        bc.setText(text);

        bRet = painter.begin(&paintDevice);
        QCOMPARE(bRet, true);

        bc.render(painter, paintRect, mode);

        bRet = painter.end();
        QCOMPARE(bRet, true);

        QCOMPARE(bc.getError(), getError);
        QCOMPARE(bc.error_message(), error_message);
        QCOMPARE(bc.lastError(), error_message);
        QCOMPARE(bc.hasErrors(), getError != 0);
        QCOMPARE(bc.encodedWidth(), encodedWidth);
        QCOMPARE(bc.encodedRows(), encodedRows);

        if (getError) {
            QCOMPARE(spyEncoded.count(), 0);
            QCOMPARE(spyErrored.count(), 1);
        } else {
            QCOMPARE(spyEncoded.count(), 1);
            QCOMPARE(spyErrored.count(), 0);
        }
    }

    void saveToFileTest_data()
    {
        QTest::addColumn<int>("symbology");
        QTest::addColumn<QString>("text");
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<bool>("expected_bRet");

        QTest::newRow("BARCODE_DATAMATRIX gif") << BARCODE_DATAMATRIX << "1234" << "test_save_to_file.gif" << true;
        QTest::newRow("BARCODE_DATAMATRIX unknown format") << BARCODE_DATAMATRIX << "1234" << "test_save_to_file.ext" << false;
    }

    void saveToFileTest()
    {
        Zint::QZint bc;

        bool bRet;
        int ret;

        QFETCH(int, symbology);
        QFETCH(QString, text);
        QFETCH(QString, fileName);
        QFETCH(bool, expected_bRet);

        bc.setSymbol(symbology);
        bc.setText(text);

        bRet = bc.save_to_file(fileName);
        QCOMPARE(bRet, expected_bRet);

        if (bRet) {
            ret = remove(fileName.toLatin1());
            QCOMPARE(ret, 0);
        }
    }

    void getAsCLITest_data()
    {
        QTest::addColumn<bool>("autoHeight");
        QTest::addColumn<float>("heightPerRow");
        QTest::addColumn<QString>("outfile");

        QTest::addColumn<int>("symbology");
        QTest::addColumn<int>("inputMode");
        QTest::addColumn<QString>("text");
        QTest::addColumn<QString>("primary");
        QTest::addColumn<float>("height");
        QTest::addColumn<int>("option1");
        QTest::addColumn<int>("option2");
        QTest::addColumn<int>("option3");
        QTest::addColumn<float>("scale");
        QTest::addColumn<bool>("dotty");
        QTest::addColumn<float>("dotSize");
        QTest::addColumn<float>("guardDescent");
        QTest::addColumn<int>("structAppCount");
        QTest::addColumn<int>("structAppIndex");
        QTest::addColumn<QString>("structAppID");
        QTest::addColumn<QColor>("fgColor");
        QTest::addColumn<QColor>("bgColor");
        QTest::addColumn<bool>("cmyk");
        QTest::addColumn<int>("borderTypeIndex");
        QTest::addColumn<int>("borderWidth");
        QTest::addColumn<int>("whitespace");
        QTest::addColumn<int>("vWhitespace");
        QTest::addColumn<int>("fontSetting");
        QTest::addColumn<bool>("showText");
        QTest::addColumn<bool>("gsSep");
        QTest::addColumn<bool>("quietZones");
        QTest::addColumn<bool>("noQuietZones");
        QTest::addColumn<bool>("compliantHeight");
        QTest::addColumn<int>("rotateAngle");
        QTest::addColumn<int>("eci");
        QTest::addColumn<bool>("gs1Parens");
        QTest::addColumn<bool>("gs1NoCheck");
        QTest::addColumn<bool>("readerInit");
        QTest::addColumn<int>("warnLevel");
        QTest::addColumn<bool>("debug");

        QTest::addColumn<QString>("expected_cmd");
        QTest::addColumn<QString>("expected_win");
        QTest::addColumn<QString>("expected_longOptOnly");
        QTest::addColumn<QString>("expected_barcodeNames");
        QTest::addColumn<QString>("expected_noexe");

        QTest::newRow("BARCODE_AUSPOST") << true << 0.0f << ""
            << BARCODE_AUSPOST << DATA_MODE // symbology-inputMode
            << "12345678" << "" // text-primary
            << 30.0f << -1 << 0 << 0 << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 63 --binary --compliantheight -d '12345678'"
            << "zint.exe -b 63 --binary --compliantheight -d \"12345678\""
            << "zint --barcode=63 --binary --compliantheight --data='12345678'"
            << "zint -b AUSPOST --binary --compliantheight -d '12345678'"
            << "zint -b 63 --binary --compliantheight -d \"12345678\"";

        QTest::newRow("BARCODE_AZTEC") << false << 0.0f << ""
            << BARCODE_AZTEC << UNICODE_MODE // symbology-inputMode
            << "12345678Ж0%var%" << "" // text-primary
            << 0.0f << 1 << 0 << 0 << 4.0f << true << 0.9f // height-dotSize
            << 5.0f << 2 << 1 << "as\"dfa'sdf" << QColor(Qt::blue) << QColor(Qt::white) // guardDescent-bgColor
            << true << 0 << 0 << 2 << 3 << 0 // cmyk-fontSetting
            << true << false << false << false << false << 0 // showText-rotateAngle
            << 7 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 92 --cmyk --eci=7 -d '12345678Ж0%var%' --dotsize=0.9 --dotty --fg=0000FF --scale=4"
                " --secure=1 --structapp='1,2,as\"dfa'\\''sdf' --vwhitesp=3 -w 2"
            << "zint.exe -b 92 --cmyk --eci=7 -d \"12345678Ж0%var%\" --dotsize=0.9 --dotty --fg=0000FF --scale=4"
                " --secure=1 --structapp=\"1,2,as\\\"dfa'sdf\" --vwhitesp=3 -w 2"
            << "" << "" << "";

        QTest::newRow("BARCODE_C25INTER") << true << 0.0f << ""
            << BARCODE_C25INTER << UNICODE_MODE // symbology-inputMode
            << "12345" << "" // text-primary
            << 0.0f << -1 << 2 << 0 << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << SMALL_TEXT // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 3 --compliantheight -d '12345' --small --vers=2"
            << "zint.exe -b 3 --compliantheight -d \"12345\" --small --vers=2"
            << "" << "" << "";

        QTest::newRow("BARCODE_CHANNEL") << false << 0.0f << ""
            << BARCODE_CHANNEL << UNICODE_MODE // symbology-inputMode
            << "453678" << "" // text-primary
            << 19.7f << -1 << 7 << 0 << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(255, 255, 255, 0) // guardDescent-bgColor
            << false << 1 << 2 << 0 << 0 << BOLD_TEXT // cmyk-fontSetting
            << true << false << true << false << false << 90 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << true // eci-debug
            << "zint -b 140 --bind --bold --border=2 -d '453678' --height=19.7 --nobackground --quietzones"
                " --rotate=90 --verbose --vers=7"
            << "zint.exe -b 140 --bind --bold --border=2 -d \"453678\" --height=19.7 --nobackground --quietzones"
                " --rotate=90 --verbose --vers=7"
            << "" << "" << "";

        QTest::newRow("BARCODE_GS1_128_CC") << false << 0.0f << ""
            << BARCODE_GS1_128_CC << UNICODE_MODE // symbology-inputMode
            << "[01]12345678901231[15]121212" << "[11]901222[99]ABCDE" // text-primary
            << 71.142f << 3 << 0 << 0 << 3.5f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << false << false << true << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 131 --compliantheight -d '[11]901222[99]ABCDE' --height=71.142 --mode=3 --notext"
                " --primary='[01]12345678901231[15]121212' --quietzones --scale=3.5"
            << "zint.exe -b 131 --compliantheight -d \"[11]901222[99]ABCDE\" --height=71.142 --mode=3 --notext"
                " --primary=\"[01]12345678901231[15]121212\" --quietzones --scale=3.5"
            << "" << "" << "";

        QTest::newRow("BARCODE_CODE16K") << false << 11.7f << ""
            << BARCODE_CODE16K << (UNICODE_MODE | HEIGHTPERROW_MODE) // symbology-inputMode
            << "12345678901234567890123456789012" << "" // text-primary
            << 0.0f << 4 << 0 << 2 << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 1 << 1 << 0 << 0 << SMALL_TEXT // cmyk-fontSetting
            << true << false << false << true << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 23 --compliantheight -d '12345678901234567890123456789012'"
                " --height=11.7 --heightperrow --noquietzones --rows=4 --separator=2 --small"
            << "zint.exe -b 23 --compliantheight -d \"12345678901234567890123456789012\""
                " --height=11.7 --heightperrow --noquietzones --rows=4 --separator=2 --small"
            << "" << "" << "";

        QTest::newRow("BARCODE_CODE49") << true << 0.0f << ""
            << BARCODE_CODE49 << UNICODE_MODE // symbology-inputMode
            << "12345678901234567890" << "" // text-primary
            << 30.0f << -1 << 0 << 0 << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 24 --compliantheight -d '12345678901234567890'"
            << "zint.exe -b 24 --compliantheight -d \"12345678901234567890\""
            << "" << "" << "";

        QTest::newRow("BARCODE_CODABLOCKF") << true << 0.0f << ""
            << BARCODE_CODABLOCKF << (DATA_MODE | ESCAPE_MODE) // symbology-inputMode
            << "T\\n\\xA0t\\\"" << "" // text-primary
            << 0.0f << 2 << 5 << 3 << 3.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 2 << 4 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << true << WARN_DEFAULT << false // eci-debug
            << "zint -b 74 --binary --border=4 --box --cols=5 --compliantheight -d 'T\\n\\xA0t\\\"' --esc --init"
                " --rows=2 --scale=3 --separator=3"
            << "zint.exe -b 74 --binary --border=4 --box --cols=5 --compliantheight -d \"T\\n\\xA0t\\\\\"\" --esc --init"
                " --rows=2 --scale=3 --separator=3"
            << "" << "" << "";

        QTest::newRow("BARCODE_DAFT") << false << 0.0f << ""
            << BARCODE_DAFT << UNICODE_MODE // symbology-inputMode
            << "daft" << "" // text-primary
            << 9.2f << -1 << 251 << 0 << 1.0f << false << 0.7f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(0x30, 0x31, 0x32, 0x33) << QColor(0xBF, 0xBE, 0xBD, 0xBC) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 93 --bg=BFBEBDBC -d 'daft' --fg=30313233 --height=9.2 --vers=251"
            << "zint.exe -b 93 --bg=BFBEBDBC -d \"daft\" --fg=30313233 --height=9.2 --vers=251"
            << "" << "" << "";

        QTest::newRow("BARCODE_DATAMATRIX") << true << 0.0f << ""
            << BARCODE_DATAMATRIX << GS1_MODE // symbology-inputMode
            << "[20]12" << "" // text-primary
            << 0.0f << -1 << 0 << DM_SQUARE << 1.0f << false << 0.7f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << true << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 71 -d '[20]12' --gs1 --gssep --square"
            << "zint.exe -b 71 -d \"[20]12\" --gs1 --gssep --square"
            << "" << "" << "";

        QTest::newRow("BARCODE_DATAMATRIX") << false << 0.0f << ""
            << BARCODE_DATAMATRIX << (DATA_MODE | ESCAPE_MODE | FAST_MODE) // symbology-inputMode
            << "ABCDEFGH\\x01I" << "" // text-primary
            << 0.0f << -1 << 0 << 0 << 1.0f << false << 0.7f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 71 --binary -d 'ABCDEFGH\\x01I' --esc --fast"
            << "zint.exe -b 71 --binary -d \"ABCDEFGH\\x01I\" --esc --fast"
            << "" << "" << "";

        QTest::newRow("BARCODE_DBAR_EXPSTK_CC") << false << 40.8f << ""
            << BARCODE_DBAR_EXPSTK_CC << (DATA_MODE | HEIGHTPERROW_MODE) // symbology-inputMode
            << "[91]ABCDEFGHIJKL" << "[11]901222[99]ABCDE" // text-primary
            << 0.0f << -1 << 0 << 2 << 1.0f << true << 0.9f // height-dotSize
            << 3.0f << 2 << 1 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 139 --binary --compliantheight -d '[11]901222[99]ABCDE' --height=40.8 --heightperrow"
                " --primary='[91]ABCDEFGHIJKL' --rows=2"
            << "zint.exe -b 139 --binary --compliantheight -d \"[11]901222[99]ABCDE\" --height=40.8 --heightperrow"
                " --primary=\"[91]ABCDEFGHIJKL\" --rows=2"
            << "" << "" << "";

        QTest::newRow("BARCODE_DOTCODE") << false << 1.0f << ""
            << BARCODE_DOTCODE << GS1_MODE // symbology-inputMode
            << "[20]01" << "" // text-primary
            << 30.0f << -1 << 8 << ((0 + 1) << 8) << 1.0f << false << 0.7f // height-dotSize
            << 0.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 115 --cols=8 -d '[20]01' --dotsize=0.7 --gs1 --mask=0"
            << "zint.exe -b 115 --cols=8 -d \"[20]01\" --dotsize=0.7 --gs1 --mask=0"
            << "" << "" << "";

        QTest::newRow("BARCODE_EANX") << true << 0.0f << ""
            << BARCODE_EANX << UNICODE_MODE // symbology-inputMode
            << "123456789012+12" << "" // text-primary
            << 0.0f << -1 << 8 << 0 << 1.0f << true << 0.8f // height-dotSize
            << 0.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 13 --addongap=8 --compliantheight -d '123456789012+12' --guarddescent=0"
            << "zint.exe -b 13 --addongap=8 --compliantheight -d \"123456789012+12\" --guarddescent=0"
            << "" << "" << "";

        QTest::newRow("BARCODE_GRIDMATRIX") << false << 0.0f << ""
            << BARCODE_GRIDMATRIX << UNICODE_MODE // symbology-inputMode
            << "Your Data Here!" << "" // text-primary
            << 0.0f << 1 << 5 << 0 << 0.5f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << true << false << true << 270 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 142 -d 'Your Data Here!' --quietzones --rotate=270 --scale=0.5 --secure=1 --vers=5"
            << "zint.exe -b 142 -d \"Your Data Here!\" --quietzones --rotate=270 --scale=0.5 --secure=1 --vers=5"
            << "" << "" << "";

        QTest::newRow("BARCODE_HANXIN") << false << 0.0f << ""
            << BARCODE_HANXIN << (UNICODE_MODE | ESCAPE_MODE) // symbology-inputMode
            << "éβÿ啊\\e\"'" << "" // text-primary
            << 30.0f << 2 << 5 << ((0 + 1) << 8) << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 29 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 116 --eci=29 -d 'éβÿ啊\\e\"'\\''' --esc --mask=0 --secure=2 --vers=5"
            << "zint.exe -b 116 --eci=29 -d \"éβÿ啊\\e\\\"'\" --esc --mask=0 --secure=2 --vers=5"
            << "" << "" << "";

        QTest::newRow("BARCODE_HIBC_DM") << false << 10.0f << ""
            << BARCODE_HIBC_DM << UNICODE_MODE // symbology-inputMode
            << "1234" << "" // text-primary
            << 0.0f << -1 << 8 << DM_DMRE << 1.0f << false << 0.7f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << true << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 102 -d '1234' --dmre --vers=8"
            << "zint.exe -b 102 -d \"1234\" --dmre --vers=8"
            << "" << "" << "";

        QTest::newRow("BARCODE_HIBC_PDF") << false << 0.0f << ""
            << BARCODE_HIBC_PDF << (DATA_MODE | HEIGHTPERROW_MODE) // symbology-inputMode
            << "TEXT" << "" // text-primary
            << 3.5f << 3 << 4 << 10 << 10.0f << false << 0.8f // height-dotSize
            << 5.0f << 2 << 1 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << true << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 106 --binary --cols=4 -d 'TEXT' --height=3.5 --heightperrow --quietzones"
                " --rows=10 --scale=10 --secure=3 --structapp=1,2"
            << "zint.exe -b 106 --binary --cols=4 -d \"TEXT\" --height=3.5 --heightperrow --quietzones"
                " --rows=10 --scale=10 --secure=3 --structapp=1,2"
            << "" << "" << "";

        QTest::newRow("BARCODE_ITF14") << true << 0.0f << ""
            << BARCODE_ITF14 << UNICODE_MODE // symbology-inputMode
            << "9212320967145" << "" // text-primary
            << 30.0f << -1 << 0 << 0 << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 89 --compliantheight -d '9212320967145'"
            << "zint.exe -b 89 --compliantheight -d \"9212320967145\""
            << "" << "" << "";

        QTest::newRow("BARCODE_ITF14") << true << 0.0f << ""
            << BARCODE_ITF14 << UNICODE_MODE // symbology-inputMode
            << "9212320967145" << "" // text-primary
            << 30.0f << -1 << 0 << 0 << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 1 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 89 --border=1 --compliantheight -d '9212320967145'"
            << "zint.exe -b 89 --border=1 --compliantheight -d \"9212320967145\""
            << "" << "" << "";

        QTest::newRow("BARCODE_MAXICODE") << true << 0.0f << ""
            << BARCODE_MAXICODE << (UNICODE_MODE | ESCAPE_MODE) // symbology-inputMode
            << "152382802840001"
            << "1Z00004951\\GUPSN\\G06X610\\G159\\G1234567\\G1/1\\G\\GY\\G1 MAIN ST\\GTOWN\\GNY\\R\\E" // text-primary
            << 0.0f << -1 << (96 + 1) << 0 << 2.5f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << true << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 57 -d '1Z00004951\\GUPSN\\G06X610\\G159\\G1234567\\G1/1\\G\\GY\\G1 MAIN ST\\GTOWN\\GNY\\R\\E'"
                " --esc --primary='152382802840001' --quietzones --scale=2.5 --scmvv=96"
            << "zint.exe -b 57 -d \"1Z00004951\\GUPSN\\G06X610\\G159\\G1234567\\G1/1\\G\\GY\\G1 MAIN ST\\GTOWN\\GNY\\R\\E\""
                " --esc --primary=\"152382802840001\" --quietzones --scale=2.5 --scmvv=96"
            << "" << "" << "";

        QTest::newRow("BARCODE_MICROQR") << false << 0.0f << ""
            << BARCODE_MICROQR << UNICODE_MODE // symbology-inputMode
            << "1234" << "" // text-primary
            << 30.0f << 2 << 3 << (ZINT_FULL_MULTIBYTE | (3 + 1) << 8) << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 97 -d '1234' --fullmultibyte --mask=3 --secure=2 --vers=3"
            << "zint.exe -b 97 -d \"1234\" --fullmultibyte --mask=3 --secure=2 --vers=3"
            << "" << "" << "";

        QTest::newRow("BARCODE_QRCODE") << true << 0.0f << ""
            << BARCODE_QRCODE << GS1_MODE // symbology-inputMode
            << "(01)12" << "" // text-primary
            << 0.0f << 1 << 5 << (ZINT_FULL_MULTIBYTE | (0 + 1) << 8) << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << true << false << true << 0 // showText-rotateAngle
            << 0 << true << true << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 58 -d '(01)12' --fullmultibyte --gs1 --gs1parens --gs1nocheck --mask=0 --quietzones"
                " --secure=1 --vers=5"
            << "zint.exe -b 58 -d \"(01)12\" --fullmultibyte --gs1 --gs1parens --gs1nocheck --mask=0 --quietzones"
                " --secure=1 --vers=5"
            << "" << "" << "";

        QTest::newRow("BARCODE_RMQR") << true << 0.0f << ""
            << BARCODE_RMQR << UNICODE_MODE // symbology-inputMode
            << "テ" << "" // text-primary
            << 30.0f << -1 << 8 << 0 << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 180 // showText-rotateAngle
            << 20 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 145 --eci=20 -d 'テ' --rotate=180 --vers=8"
            << "zint.exe -b 145 --eci=20 -d \"テ\" --rotate=180 --vers=8"
            << "" << "" << "";

        QTest::newRow("BARCODE_ULTRA") << false << 0.0f << ""
            << BARCODE_ULTRA << (GS1_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE) // symbology-inputMode
            << "(01)1" << "" // text-primary
            << 0.0f << 6 << 2 << 0 << 1.0f << true << 0.8f // height-dotSize
            << 5.0f << 2 << 1 << "4" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 144 -d '(01)1' --gs1 --gs1parens --gs1nocheck --secure=6 --structapp='1,2,4' --vers=2"
            << "zint.exe -b 144 -d \"(01)1\" --gs1 --gs1parens --gs1nocheck --secure=6 --structapp=\"1,2,4\" --vers=2"
            << "" << "" << "";

        QTest::newRow("BARCODE_UPCE_CC") << true << 0.0f << "out.svg"
            << BARCODE_UPCE_CC << UNICODE_MODE // symbology-inputMode
            << "12345670+1234" << "[11]901222[99]ABCDE" // text-primary
            << 0.0f << -1 << 0 << 0 << 1.0f << false << 0.8f // height-dotSize
            << 6.5f << 0 << 0 << "" << QColor(0xEF, 0x29, 0x29) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << (BOLD_TEXT | SMALL_TEXT) // cmyk-fontSetting
            << true << false << false << true << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_FAIL_ALL << false // eci-debug
            << "zint -b 136 --bold --compliantheight -d '[11]901222[99]ABCDE' --fg=EF2929 --guarddescent=6.5"
                " --noquietzones -o 'out.svg' --primary='12345670+1234' --small --werror"
            << "zint.exe -b 136 --bold --compliantheight -d \"[11]901222[99]ABCDE\" --fg=EF2929 --guarddescent=6.5"
                " --noquietzones -o \"out.svg\" --primary=\"12345670+1234\" --small --werror"
            << "zint --barcode=136 --bold --compliantheight --data='[11]901222[99]ABCDE' --fg=EF2929"
                " --guarddescent=6.5 --noquietzones --output='out.svg' --primary='12345670+1234' --small --werror"
            << "zint -b UPCE_CC --bold --compliantheight -d '[11]901222[99]ABCDE' --fg=EF2929 --guarddescent=6.5"
                " --noquietzones -o 'out.svg' --primary='12345670+1234' --small --werror"
            << "zint -b 136 --bold --compliantheight -d \"[11]901222[99]ABCDE\" --fg=EF2929 --guarddescent=6.5"
                " --noquietzones -o \"out.svg\" --primary=\"12345670+1234\" --small --werror";

        QTest::newRow("BARCODE_VIN") << false << 2.0f << ""
            << BARCODE_VIN << UNICODE_MODE // symbology-inputMode
            << "12345678701234567" << "" // text-primary
            << 20.0f << -1 << 1 << 0 << 1.0f << false << 0.8f // height-dotSize
            << 5.0f << 0 << 0 << "" << QColor(Qt::black) << QColor(Qt::white) // guardDescent-bgColor
            << false << 0 << 0 << 0 << 0 << 0 // cmyk-fontSetting
            << true << false << false << false << true << 0 // showText-rotateAngle
            << 0 << false << false << false << WARN_DEFAULT << false // eci-debug
            << "zint -b 73 -d '12345678701234567' --height=20 --vers=1"
            << "zint.exe -b 73 -d \"12345678701234567\" --height=20 --vers=1"
            << "" << "" << "";
    }

    void getAsCLITest()
    {
        Zint::QZint bc;

        QString cmd;

        QFETCH(bool, autoHeight);
        QFETCH(float, heightPerRow);
        QFETCH(QString, outfile);

        QFETCH(int, symbology);
        QFETCH(int, inputMode);
        QFETCH(QString, text);
        QFETCH(QString, primary);
        QFETCH(float, height);
        QFETCH(int, option1);
        QFETCH(int, option2);
        QFETCH(int, option3);
        QFETCH(float, scale);
        QFETCH(bool, dotty);
        QFETCH(float, dotSize);
        QFETCH(float, guardDescent);
        QFETCH(int, structAppCount);
        QFETCH(int, structAppIndex);
        QFETCH(QString, structAppID);
        QFETCH(QColor, fgColor);
        QFETCH(QColor, bgColor);
        QFETCH(bool, cmyk);
        QFETCH(int, borderTypeIndex);
        QFETCH(int, borderWidth);
        QFETCH(int, whitespace);
        QFETCH(int, vWhitespace);
        QFETCH(int, fontSetting);
        QFETCH(bool, showText);
        QFETCH(bool, gsSep);
        QFETCH(bool, quietZones);
        QFETCH(bool, noQuietZones);
        QFETCH(bool, compliantHeight);
        QFETCH(int, rotateAngle);
        QFETCH(int, eci);
        QFETCH(bool, gs1Parens);
        QFETCH(bool, gs1NoCheck);
        QFETCH(bool, readerInit);
        QFETCH(int, warnLevel);
        QFETCH(bool, debug);

        QFETCH(QString, expected_cmd);
        QFETCH(QString, expected_win);
        QFETCH(QString, expected_longOptOnly);
        QFETCH(QString, expected_barcodeNames);
        QFETCH(QString, expected_noexe);

        bc.setSymbol(symbology);
        bc.setInputMode(inputMode);
        if (primary.isEmpty()) {
            bc.setText(text);
        } else {
            bc.setText(primary);
            bc.setPrimaryMessage(text);
        }
        bc.setHeight(height);
        bc.setOption1(option1);
        bc.setOption2(option2);
        bc.setOption3(option3);
        bc.setScale(scale);
        bc.setDotty(dotty);
        bc.setDotSize(dotSize);
        bc.setGuardDescent(guardDescent);
        bc.setStructApp(structAppCount, structAppIndex, structAppID);
        bc.setFgColor(fgColor);
        bc.setBgColor(bgColor);
        bc.setCMYK(cmyk);
        bc.setBorderType(borderTypeIndex);
        bc.setBorderWidth(borderWidth);
        bc.setWhitespace(whitespace);
        bc.setVWhitespace(vWhitespace);
        bc.setFontSettingValue(fontSetting);
        bc.setShowText(showText);
        bc.setGSSep(gsSep);
        bc.setQuietZones(quietZones);
        bc.setNoQuietZones(noQuietZones);
        bc.setCompliantHeight(compliantHeight);
        bc.setRotateAngleValue(rotateAngle);
        bc.setECIValue(eci);
        bc.setGS1Parens(gs1Parens);
        bc.setGS1NoCheck(gs1NoCheck);
        bc.setReaderInit(readerInit);
        bc.setWarnLevel(warnLevel);
        bc.setDebug(debug);

        cmd = bc.getAsCLI(false /*win*/, false /*longOptOnly*/, false /*barcodeNames*/, false /*noEXE*/,
                    autoHeight, heightPerRow, outfile);
        QCOMPARE(cmd, expected_cmd);

        cmd = bc.getAsCLI(true /*win*/, false /*longOptOnly*/, false /*barcodeNames*/, false /*noEXE*/,
                    autoHeight, heightPerRow, outfile);
        QCOMPARE(cmd, expected_win);

        if (!expected_longOptOnly.isEmpty()) {
            cmd = bc.getAsCLI(false /*win*/, true /*longOptOnly*/, false /*barcodeNames*/, false /*noEXE*/,
                        autoHeight, heightPerRow, outfile);
            QCOMPARE(cmd, expected_longOptOnly);
        }

        if (!expected_barcodeNames.isEmpty()) {
            cmd = bc.getAsCLI(false /*win*/, false /*longOptOnly*/, true /*barcodeNames*/, false /*noEXE*/,
                        autoHeight, heightPerRow, outfile);
            QCOMPARE(cmd, expected_barcodeNames);
        }

        if (!expected_noexe.isEmpty()) {
            cmd = bc.getAsCLI(true /*win*/, false /*longOptOnly*/, false /*barcodeNames*/, true /*noEXE*/,
                        autoHeight, heightPerRow, outfile);
            QCOMPARE(cmd, expected_noexe);
        }
    }

    void getAsCLISegsTest()
    {
        Zint::QZint bc;

        QString cmd;
        QString expected_cmd;
        QString expected_win;

        std::vector<QString> segTexts;
        std::vector<int> segECIs;
        segTexts.push_back(QString("Τεχτ"));
        segECIs.push_back(9);
        segTexts.push_back(QString("Téxt"));
        segECIs.push_back(3);
        segTexts.push_back(QString("กขฯ"));
        segECIs.push_back(13);
        segTexts.push_back(QString("貫やぐ禁"));
        segECIs.push_back(20);

        std::vector<Zint::QZintSeg> segs;
        for (int i = 0; i < (int) segTexts.size(); i++) {
            segs.push_back(Zint::QZintSeg(segTexts[i]));
            segs.back().m_eci = segECIs[i];
        }

        bc.setSymbol(BARCODE_QRCODE);
        bc.setSegs(segs);
        bc.setDotty(true);

        expected_cmd = "zint -b 58 --eci=9 -d 'Τεχτ' --seg1=3,'Téxt' --seg2=13,'กขฯ' --seg3=20,'貫やぐ禁' --dotty";
        cmd = bc.getAsCLI(false /*win*/);
        QCOMPARE(cmd, expected_cmd);

        expected_win = "zint.exe -b 58 --eci=9 -d \"Τεχτ\" --seg1=3,\"Téxt\" --seg2=13,\"กขฯ\" --seg3=20,\"貫やぐ禁\" --dotty";
        cmd = bc.getAsCLI(true /*win*/);
        QCOMPARE(cmd, expected_win);
    }

    void qZintAndLibZintEqual_data()
    {
        QTest::addColumn<int>("symbology");
        QTest::addColumn<int>("rotateAngles");
        QTest::addColumn<QString>("text");

        QTest::newRow("symbology=BARCODE_DATAMATRIX  rotateAngles=0   text=1234")          << BARCODE_DATAMATRIX << 0   << "1234";
        QTest::newRow("symbology=BARCODE_QRCODE      rotateAngles=0   text=Hello%20World") << BARCODE_QRCODE     << 0   << "Hello%20World";
        QTest::newRow("symbology=BARCODE_QRCODE      rotateAngles=90  text=Hello%20World") << BARCODE_QRCODE     << 90  << "Hello%20World";
        QTest::newRow("symbology=BARCODE_QRCODE      rotateAngles=180 text=Hello%20World") << BARCODE_QRCODE     << 180 << "Hello%20World";
        QTest::newRow("symbology=BARCODE_QRCODE      rotateAngles=270 text=Hello%20World") << BARCODE_QRCODE     << 270 << "Hello%20World";
    }

    void qZintAndLibZintEqual()
    {
        QFETCH(int, symbology);
        QFETCH(int, rotateAngles);
        QFETCH(QString, text);
        QString fileName("test_qZintAndLibZintEqual_%1.gif");
        QString fileNameForLibZint(fileName.arg("libZint"));
        QString fileNameForQZint(fileName.arg("qZint"));

        Zint::QZint bc;
        QSharedPointer<zint_symbol> symbol(ZBarcode_Create(), ZBarcode_Delete);

        bc.setSymbol(symbology);
        symbol->symbology = symbology;

        bc.setText(text);
        bc.setRotateAngleValue(rotateAngles);

        qstrcpy(symbol->outfile, qUtf8Printable(fileNameForLibZint));

        bc.save_to_file(fileNameForQZint);
        ZBarcode_Encode_and_Print(symbol.data(), reinterpret_cast<const unsigned char*>(qUtf8Printable(text)), 0, rotateAngles);

        QImage imageWrittenByVanilla(fileNameForLibZint);
        QImage imageWrittenByQZint(fileNameForQZint);

        QCOMPARE(imageWrittenByQZint.isNull(), false);
        QCOMPARE(imageWrittenByVanilla.isNull(), false);

        QCOMPARE(imageWrittenByQZint == imageWrittenByVanilla, true);

        QFile::remove(fileNameForLibZint);
        QFile::remove(fileNameForQZint);
    }
};

QTEST_MAIN(TestQZint)
#include "test_qzint.moc"

/* vim: set ts=4 sw=4 et : */
