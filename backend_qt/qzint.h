/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bogdan@licentia.eu                                                    *
 *   Copyright (C) 2010-2021 Robin Stuart                                  *
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

#ifndef QZINT_H
#define QZINT_H

#include <QColor>
#include <QPainter>
#include "zint.h"

namespace Zint
{

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
    void setText(const QString& text);

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

    bool debug() const;
    void setDebug(bool debug);

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
    bool isExtendable(int symbology = 0) const;
    bool supportsECI(int symbology = 0) const;
    bool supportsGS1(int symbology = 0) const;
    bool hasDefaultQuietZones(int symbology = 0) const;
    bool isFixedRatio(int symbology = 0) const;
    bool isDotty(int symbology = 0) const;
    bool supportsReaderInit(int symbology = 0) const;
    bool hasCompliantHeight(int symbology = 0) const;

    int getError() const;

    const QString& lastError() const;
    bool hasErrors() const;

    bool save_to_file(QString filename);

    /* Note: legacy argument `mode` is not used */
    void render(QPainter& painter, const QRectF& paintRect, AspectRatioMode mode = IgnoreAspectRatio);

    int getVersion() const;

signals:
    void encoded();
    void errored();

private:
    void resetSymbol();
    void encode();
    static Qt::GlobalColor colourToQtColor(int colour);

private:
    int m_symbol;
    QString m_text;
    QString m_primaryMessage;
    float m_height;
    int m_borderType;
    int m_borderWidth;
    int m_fontSetting;
    int m_option_1;
    int m_option_2;
    int m_option_3;
    int m_input_mode;
    QColor m_fgColor;
    QColor m_bgColor;
    bool m_cmyk;
    QString m_lastError;
    int m_error;
    int m_whitespace;
    int m_vwhitespace;
    zint_symbol * m_zintSymbol;
    float m_scale;
    bool m_show_hrt;
    int m_eci;
    int m_rotate_angle;
    bool m_dotty;
    float m_dot_size;
    float m_guardDescent;
    struct zint_structapp m_structapp;
    bool m_gs1parens;
    bool m_gs1nocheck;
    bool m_gssep;
    bool m_quiet_zones;
    bool m_no_quiet_zones;
    bool m_compliant_height;
    bool m_reader_init;
    bool m_debug;

    int target_size_horiz; /* Legacy */
    int target_size_vert; /* Legacy */
};

} /* namespace Zint */

#endif /* QZINT_H */
