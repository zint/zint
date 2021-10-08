/***************************************************************************
 *   Copyright (C) 2021 by Robin Stuart <rstuart114@gmail.com>             *
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
/* vim: set ts=4 sw=4 et : */

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
#  if defined(__SANITIZE_ADDRESS__) || (defined(__has_feature) && __has_feature(address_sanitizer))
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
            24, 25, 26, 27, 28, 29, 30, 899,
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

        bool debug = true;
        bc.setDebug(debug);
        QCOMPARE(bc.debug(), debug);
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
        QTest::newRow("31") << 31 << 0;
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

        QTest::newRow("BARCODE_CODE11") << BARCODE_CODE11 << (ZINT_CAP_HRT);
        QTest::newRow("BARCODE_CODE128") << BARCODE_CODE128 << (ZINT_CAP_HRT | ZINT_CAP_READER_INIT);
        QTest::newRow("BARCODE_EANX") << BARCODE_EANX << (ZINT_CAP_HRT | ZINT_CAP_EXTENDABLE | ZINT_CAP_QUIET_ZONES | ZINT_CAP_COMPLIANT_HEIGHT);
        QTest::newRow("BARCODE_QRCODE") << BARCODE_QRCODE << (ZINT_CAP_ECI | ZINT_CAP_GS1 | ZINT_CAP_DOTTY | ZINT_CAP_FIXED_RATIO);
    }

    void capTest()
    {
        Zint::QZint bc;

        QFETCH(int, symbology);
        QFETCH(int, cap_flag);

        bc.setSymbol(symbology);
        QCOMPARE(bc.hasHRT(), (cap_flag & ZINT_CAP_HRT) != 0);
        QCOMPARE(bc.isExtendable(), (cap_flag & ZINT_CAP_EXTENDABLE) != 0);
        QCOMPARE(bc.supportsECI(), (cap_flag & ZINT_CAP_ECI) != 0);
        QCOMPARE(bc.supportsGS1(), (cap_flag & ZINT_CAP_GS1) != 0);
        QCOMPARE(bc.hasDefaultQuietZones(), (cap_flag & ZINT_CAP_QUIET_ZONES) != 0);
        QCOMPARE(bc.isFixedRatio(), (cap_flag & ZINT_CAP_FIXED_RATIO) != 0);
        QCOMPARE(bc.isDotty(), (cap_flag & ZINT_CAP_DOTTY) != 0);
        QCOMPARE(bc.supportsReaderInit(), (cap_flag & ZINT_CAP_READER_INIT) != 0);
        QCOMPARE(bc.hasCompliantHeight(), (cap_flag & ZINT_CAP_COMPLIANT_HEIGHT) != 0);
    }

    void renderTest_data()
    {
        QTest::addColumn<int>("symbology");
        QTest::addColumn<QString>("text");
        QTest::addColumn<int>("getError");
        QTest::addColumn<QString>("error_message");

        QTest::newRow("BARCODE_QRCODE") << BARCODE_QRCODE << "1234" << 0 << "";
        if (!m_skipIfFontUsed) {
            QTest::newRow("BARCODE_QRCODE no text") << BARCODE_QRCODE << "" << ZINT_ERROR_INVALID_DATA << "Error 205: No input data";
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
};

QTEST_MAIN(TestQZint)
#include "test_qzint.moc"
