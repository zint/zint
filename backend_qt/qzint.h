/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bogdan@licentia.eu                                                    *
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

#ifndef BARCODERENDER_H
#define BARCODERENDER_H
#include <QColor>
#include <QPainter>

#include "zint.h"

namespace Zint
{

class QZint
{
private:

public:
     enum BorderType{NO_BORDER=0, BIND=2, BOX=4};
     enum AspectRatioMode{IgnoreAspectRatio=0, KeepAspectRatio=1, CenterBarCode=2};

public:
    QZint();
    ~QZint();

    int  symbol() const;
    void setSymbol(int symbol);

    int inputMode() const;
    void setInputMode(int input_mode);

    QString text() const;
    void setText(const QString & text);

    QString primaryMessage() const;
    void setPrimaryMessage(const QString & primaryMessage);

    int height() const;
    void setHeight(int height);

    int option2() const;
    void setOption2(int option);

    void setOption3(int option);

    float scale() const;
    void setScale(float scale);

    void setDotSize(float dot_size);

    QColor fgColor() const;
    void setFgColor(const QColor & fgColor);

    QColor bgColor() const;
    void setBgColor(const QColor & bgColor);

    BorderType borderType() const;
    void setBorderType(BorderType border);

    int borderWidth() const;
    void setBorderWidth(int boderWidth);

    int securityLevel() const;
    void setSecurityLevel(int securityLevel);

    int mode() const;
    void setMode(int securityLevel);

    void setWhitespace(int whitespace);

    void setHideText(bool hide);

    void setTargetSize(int width, int height);

    void setGSSep(bool gssep);

    void setDebug(bool debug);

    int getError() const;

    QString error_message() const;

    const QString & lastError() const;
    bool hasErrors() const;

    bool save_to_file(QString filename);

    void render(QPainter & painter, const QRectF & paintRect, AspectRatioMode mode=IgnoreAspectRatio);

private:
    void resetSymbol();
    void encode();

private:
    int m_symbol;
    QString m_text;
    QString m_primaryMessage;
    int m_height;
    BorderType m_border;
    int m_borderWidth;
    int m_option_2;
    int m_securityLevel;
    int m_input_mode;
    QColor m_fgColor;
    QColor m_bgColor;
    QString m_lastError;
    int m_error;
    int m_whitespace;
    zint_symbol * m_zintSymbol;
    float m_scale;
    int m_option_3;
    bool m_hidetext;
    float m_dot_size;
    int target_size_horiz;
    int target_size_vert;
    bool m_gssep;
    bool m_debug;
};
}
#endif

