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

    void setDotSize(float dot_size);

    float guardDescent() const;
    void setGuardDescent(float guardDescent);

    QColor fgColor() const;
    void setFgColor(const QColor& fgColor);

    QColor bgColor() const;
    void setBgColor(const QColor& bgColor);

    void setCMYK(bool cmyk);

    int borderType() const;
    void setBorderType(int borderTypeIndex);

    int borderWidth() const;
    void setBorderWidth(int boderWidth);

    void setWhitespace(int whitespace);

    void setVWhitespace(int vwhitespace);

    void setFontSetting(int fontSettingIndex);

    void setShowText(bool show);

    void setGSSep(bool gssep);

    void setQuietZones(bool quietZones);
    void setNoQuietZones(bool noQuietZones);

    int rotateAngle() const;
    void setRotateAngle(int rotateIndex);

    void setECI(int ECIIndex);

    void setGS1Parens(bool gs1parens);

    void setGS1NoCheck(bool gs1nocheck);

    void setReaderInit(bool reader_init);

    void setDebug(bool debug);

    /* Legacy property getters/setters */
    void setWidth(int width); /* option_1 */
    int width() const;
    void setSecurityLevel(int securityLevel); /* option_2 */
    int securityLevel() const;
    void setPdf417CodeWords(int pdf417CodeWords); /* no op */
    int pdf417CodeWords() const;
    void setHideText(bool hide); /* setShowText(!hide) */
    void setTargetSize(int width, int height);

    /* Test capabilities - ZBarcode_Cap() */
    bool hasHRT(int symbology = 0) const;
    bool isExtendable(int symbology = 0) const;
    bool supportsECI(int symbology = 0) const;
    bool supportsGS1(int symbology = 0) const;
    bool hasDefaultQuietZones(int symbology = 0) const;
    bool isFixedRatio(int symbology = 0) const;
    bool isDotty(int symbology = 0) const;
    bool supportsReaderInit(int symbology = 0) const;

    int getError() const;

    QString error_message() const;

    const QString& lastError() const;
    bool hasErrors() const;

    bool save_to_file(QString filename);

    /* Note: legacy argument `mode` is not used */
    void render(QPainter& painter, const QRectF& paintRect, AspectRatioMode mode = IgnoreAspectRatio);
    
    int getVersion() const;

signals:
    void encoded();

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
    bool m_gs1parens;
    bool m_gs1nocheck;
    bool m_gssep;
    bool m_quiet_zones;
    bool m_no_quiet_zones;
    bool m_reader_init;
    bool m_debug;

    int target_size_horiz; /* Legacy */
    int target_size_vert; /* Legacy */
};

}

#endif /* QZINT_H */
