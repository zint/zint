/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bogdan@licentia.eu                                                    *
 *   Copyright (C) 2010-2022 Robin Stuart                                  *
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

#ifndef QZINT_H
#define QZINT_H

#include <QColor>
#include <QPainter>
#include "zint.h"

namespace Zint
{

/* QString version of `struct zint_seg` */
class QZintSeg {
public:
    QString m_text;
    int m_eci;

    QZintSeg();
    QZintSeg(const QString& text, const int ECIIndex = 0); // `ECIIndex` is comboBox index (not ECI value)
};

/* Interface */
class QZint : public QObject
{
    Q_OBJECT

public:
     enum AspectRatioMode{ IgnoreAspectRatio = 0, KeepAspectRatio = 1, CenterBarCode = 2 }; /* Legacy - not used */

public:
    QZint();
    ~QZint();

    int symbol() const; /* Symbology */
    void setSymbol(int symbol);

    int inputMode() const;
    void setInputMode(int input_mode);

    QString text() const;
    void setText(const QString& text); // Clears segs

    std::vector<QZintSeg> segs() const;
    void setSegs(const std::vector<QZintSeg>& segs); // Clears text and sets eci

    QString primaryMessage() const;
    void setPrimaryMessage(const QString& primaryMessage);

    float height() const;
    void setHeight(float height);

    int option1() const;
    void setOption1(int option_1);

    int option2() const;
    void setOption2(int option);

    int option3() const;
    void setOption3(int option);

    float scale() const;
    void setScale(float scale);

    bool dotty() const;
    void setDotty(bool botty);

    float dotSize() const;
    void setDotSize(float dot_size);

    float guardDescent() const;
    void setGuardDescent(float guardDescent);

    int structAppCount() const;
    int structAppIndex() const;
    QString structAppID() const;
    void setStructApp(const int count, const int index, const QString& id);
    void clearStructApp();

    QColor fgColor() const;
    void setFgColor(const QColor& fgColor);

    QColor bgColor() const;
    void setBgColor(const QColor& bgColor);

    bool cmyk() const;
    void setCMYK(bool cmyk);

    int borderType() const;
    void setBorderType(int borderTypeIndex);

    int borderWidth() const;
    void setBorderWidth(int borderWidth);

    int whitespace() const;
    void setWhitespace(int whitespace);

    int vWhitespace() const;
    void setVWhitespace(int vWhitespace);

    int fontSetting() const;
    void setFontSetting(int fontSettingIndex); // Sets from comboBox index
    void setFontSettingValue(int fontSetting); // Sets literal value

    bool showText() const;
    void setShowText(bool showText);

    bool gsSep() const;
    void setGSSep(bool gsSep);

    bool quietZones() const;
    void setQuietZones(bool quietZones);

    bool noQuietZones() const;
    void setNoQuietZones(bool noQuietZones);

    bool compliantHeight() const;
    void setCompliantHeight(bool compliantHeight);

    int rotateAngle() const;
    void setRotateAngle(int rotateIndex); // Sets from comboBox index
    void setRotateAngleValue(int rotateAngle); // Sets literal value

    int eci() const;
    void setECI(int ECIIndex); // Sets from comboBox index
    void setECIValue(int eci); // Sets literal value

    bool gs1Parens() const;
    void setGS1Parens(bool gs1Parens);

    bool gs1NoCheck() const;
    void setGS1NoCheck(bool gs1NoCheck);

    bool readerInit() const;
    void setReaderInit(bool readerInit);

    int warnLevel() const;
    void setWarnLevel(int warnLevel);

    bool debug() const;
    void setDebug(bool debug);

    int encodedWidth() const; // Read-only, encoded width (no. of modules encoded)
    int encodedRows() const; // Read-only, no. of rows encoded

    /* Legacy property getters/setters */
    void setWidth(int width); /* option_1 */
    int width() const;
    void setSecurityLevel(int securityLevel); /* option_2 */
    int securityLevel() const;
    void setPdf417CodeWords(int pdf417CodeWords); /* No-op */
    int pdf417CodeWords() const;
    void setHideText(bool hide); /* setShowText(!hide) */
    void setTargetSize(int width, int height);
    QString error_message() const; /* Same as lastError() */

    /* Test capabilities - ZBarcode_Cap() */
    bool hasHRT(int symbology = 0) const;
    bool isStackable(int symbology = 0) const;
    bool isExtendable(int symbology = 0) const;
    bool isComposite(int symbology = 0) const;
    bool supportsECI(int symbology = 0) const;
    bool supportsGS1(int symbology = 0) const;
    bool isDotty(int symbology = 0) const;
    bool hasDefaultQuietZones(int symbology = 0) const;
    bool isFixedRatio(int symbology = 0) const;
    bool supportsReaderInit(int symbology = 0) const;
    bool supportsFullMultibyte(int symbology = 0) const;
    bool hasMask(int symbology = 0) const;
    bool supportsStructApp(int symbology = 0) const;
    bool hasCompliantHeight(int symbology = 0) const;

    int getError() const;

    const QString& lastError() const;
    bool hasErrors() const;

    bool save_to_file(const QString& filename);

    /* Note: legacy argument `mode` is not used */
    void render(QPainter& painter, const QRectF& paintRect, AspectRatioMode mode = IgnoreAspectRatio);

    int getVersion() const;

    /* Translate settings into Command Line equivalent. Set `win` to use Windows escaping of data.
       If `autoHeight` set then `--height=` option will not be emitted.
       If HEIGHTPERROW_MODE set and non-zero `heightPerRow` given then use that for height instead of internal
       height */
    QString getAsCLI(const bool win, const bool longOptOnly = false, const bool barcodeNames = false,
                const bool noEXE = false, const bool autoHeight = false, const float heightPerRow = 0.0f,
                const QString& outfile = "") const;

signals:
    void encoded();
    void errored();

private:
    void resetSymbol();
    void encode();

    int convertSegs(struct zint_seg segs[], std::vector<QByteArray>& bstrs);

    static Qt::GlobalColor colourToQtColor(int colour);

    /* `getAsCLI()` helpers */
    static void arg_str(QString& cmd, const char *const opt, const QString& val);
    static void arg_int(QString& cmd, const char *const opt, const int val, const bool allowZero = false);
    static void arg_bool(QString& cmd, const char *const opt, const bool val);
    static void arg_color(QString& cmd, const char *const opt, const QColor val);
    static void arg_data(QString& cmd, const char *const opt, const QString& val, const bool win);
    static void arg_seg(QString& cmd, const int seg_no, const QZintSeg& val, const bool win);
    static void arg_data_esc(QString& cmd, const char *const opt, QString& text, const bool win);
    static void arg_float(QString& cmd, const char *const opt, const float val, const bool allowZero = false);
    static void arg_structapp(QString& cmd, const char *const opt, const int count, const int index,
                                const QString& id, const bool win);

private:
    zint_symbol *m_zintSymbol;
    int m_symbol;
    int m_input_mode;
    QString m_text;
    QString m_primaryMessage;
    std::vector<QZintSeg> m_segs;
    float m_height;
    int m_option_1;
    int m_option_2;
    int m_option_3;
    float m_scale;
    bool m_dotty;
    float m_dot_size;
    float m_guardDescent;
    struct zint_structapp m_structapp;
    QColor m_fgColor;
    QColor m_bgColor;
    bool m_cmyk;
    int m_borderType;
    int m_borderWidth;
    int m_whitespace;
    int m_vwhitespace;
    int m_fontSetting;
    bool m_show_hrt;
    bool m_gssep;
    bool m_quiet_zones;
    bool m_no_quiet_zones;
    bool m_compliant_height;
    int m_rotate_angle;
    int m_eci;
    bool m_gs1parens;
    bool m_gs1nocheck;
    bool m_reader_init;
    int m_warn_level;
    bool m_debug;
    int m_encodedWidth;
    int m_encodedRows;
    QString m_lastError;
    int m_error;

    int target_size_horiz; /* Legacy */
    int target_size_vert; /* Legacy */
};

} /* namespace Zint */

/* vim: set ts=4 sw=4 et : */
#endif /* QZINT_H */
