/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bogdan@licentia.eu                                                    *
 *   Copyright (C) 2010-2017 Robin Stuart                                  *
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

#include "qzint.h"
#include <stdio.h>
#include <math.h>
#include <QFontMetrics>

namespace Zint {
    static const char* fontstyle = "Arial";
    static const int fontPixelSizeSmall = 8;
    static const int fontPixelSizeLarge = 12;

    QZint::QZint() {
        m_symbol = BARCODE_CODE128;
        m_height = 0;
        m_border = NO_BORDER;
        m_borderWidth = 0;
        m_securityLevel = -1;
        m_fgColor = Qt::black;
        m_bgColor = Qt::white;
        m_zintSymbol = 0;
        m_error = 0;
        m_input_mode = UNICODE_MODE + ESCAPE_MODE;
        m_scale = 1.0;
        m_option_3 = 0;
        m_hidetext = 0;
        m_dot_size = 4.0 / 5.0;
        target_size_horiz = 0;
        target_size_vert = 0;
        m_width = 0;
        m_whitespace = 0;
    }

    QZint::~QZint() {
        if (m_zintSymbol)
            ZBarcode_Delete(m_zintSymbol);
    }

    void QZint::encode() {
        if (m_zintSymbol)
            ZBarcode_Delete(m_zintSymbol);

        m_lastError.clear();
        m_zintSymbol = ZBarcode_Create();
        m_zintSymbol->output_options = m_border;
        m_zintSymbol->symbology = m_symbol;
        m_zintSymbol->height = m_height;
        m_zintSymbol->whitespace_width = m_whitespace;
        m_zintSymbol->border_width = m_borderWidth;
        m_zintSymbol->option_1 = m_securityLevel;
        m_zintSymbol->input_mode = m_input_mode;
        m_zintSymbol->option_2 = m_width;
        m_zintSymbol->dot_size = m_dot_size;
        if (m_hidetext) {
            m_zintSymbol->show_hrt = 0;
        } else {
            m_zintSymbol->show_hrt = 1;
        }
		m_zintSymbol->option_3 = m_option_3;
        QByteArray bstr = m_text.toUtf8();
        QByteArray pstr = m_primaryMessage.left(99).toLatin1();
        strcpy(m_zintSymbol->primary, pstr.data());
        m_error = ZBarcode_Encode_and_Buffer_Vector(m_zintSymbol, (unsigned char*) bstr.data(), bstr.length(), 0);
        m_lastError = m_zintSymbol->errtxt;

        switch (m_zintSymbol->output_options) {
            case 0: m_border = NO_BORDER;
                break;
            case 2: m_border = BIND;
                break;
            case 4: m_border = BOX;
                break;
        }
        m_borderWidth = m_zintSymbol->border_width;
        m_whitespace = m_zintSymbol->whitespace_width;
    }

    int QZint::symbol() const {
        return m_symbol;
    }

    void QZint::setSymbol(int symbol) {
        m_symbol = symbol;
    }

    void QZint::setInputMode(int input_mode) {
        m_input_mode = input_mode;
    }

    QString QZint::text() const {
        return m_text;
    }

    void QZint::setText(const QString & text) {
        m_text = text;
    }

    void QZint::setTargetSize(int width, int height) {
        target_size_horiz = width;
        target_size_vert = height;
    }

    QString QZint::primaryMessage() const {
        return m_primaryMessage;
    }

    void QZint::setPrimaryMessage(const QString & primaryMessage) {
        m_primaryMessage = primaryMessage;
    }

    void QZint::setHeight(int height) {
        m_height = height;
    }

    void QZint::setWidth(int width) {
        m_width = width;
    }

    void QZint::setOption3(int option) {
        m_option_3 = option;
    }

    float QZint::scale() const {
        return m_scale;
    }

    void QZint::setScale(float scale) {
        m_scale = scale;
    }

    void QZint::setDotSize(float dot_size) {
        m_dot_size = dot_size;
    }

    QColor QZint::fgColor() const {
        return m_fgColor;
    }

    void QZint::setFgColor(const QColor & fgColor) {
        m_fgColor = fgColor;
    }

    QColor QZint::bgColor() const {
        return m_bgColor;
    }

    void QZint::setBgColor(const QColor & bgColor) {
        m_bgColor = bgColor;
    }

    QZint::BorderType QZint::borderType() const {
        return m_border;
    }

    void QZint::setBorderType(BorderType border) {
        m_border = border;
    }

    int QZint::borderWidth() const {
        return m_borderWidth;
    }

    void QZint::setBorderWidth(int boderWidth) {
        if (boderWidth < 0 || boderWidth > 16)
            boderWidth = 0;
        m_borderWidth = boderWidth;
    }

    void QZint::setWhitespace(int whitespace) {
        m_whitespace = whitespace;
    }

    int QZint::securityLevel() const {
        return m_securityLevel;
    }

    void QZint::setSecurityLevel(int securityLevel) {
        m_securityLevel = securityLevel;
    }
    
    int QZint::getError() {
        return m_error;
    }

    QString QZint::error_message() const {
        return m_lastError;
    }

    int QZint::mode() const {
        return m_securityLevel;
    }

    void QZint::setMode(int securityLevel) {
        m_securityLevel = securityLevel;
    }

    void QZint::setHideText(bool hide) {
        m_hidetext = hide;
    }

    bool QZint::save_to_file(QString filename) {
        if (m_zintSymbol)
            ZBarcode_Delete(m_zintSymbol);

        QString fg_colour_hash = m_fgColor.name();
        QString bg_colour_hash = m_bgColor.name();

        m_lastError.clear();
        m_zintSymbol = ZBarcode_Create();
        m_zintSymbol->output_options = m_border;
        m_zintSymbol->symbology = m_symbol;
        m_zintSymbol->height = m_height;
        m_zintSymbol->whitespace_width = m_whitespace;
        m_zintSymbol->border_width = m_borderWidth;
        m_zintSymbol->option_1 = m_securityLevel;
        m_zintSymbol->input_mode = m_input_mode;
        m_zintSymbol->option_2 = m_width;
        m_zintSymbol->dot_size = m_dot_size;
        if (m_hidetext) {
            m_zintSymbol->show_hrt = 0;
        } else {
            m_zintSymbol->show_hrt = 1;
        }
		m_zintSymbol->option_3 = m_option_3;
        m_zintSymbol->scale = m_scale;
        QByteArray bstr = m_text.toUtf8();
        QByteArray pstr = m_primaryMessage.left(99).toLatin1();
        QByteArray fstr = filename.left(255).toLatin1();
        strcpy(m_zintSymbol->primary, pstr.data());
        strcpy(m_zintSymbol->outfile, fstr.data());
        QByteArray fgcol = fg_colour_hash.right(6).toLatin1();
        QByteArray bgcol = bg_colour_hash.right(6).toLatin1();
        strcpy(m_zintSymbol->fgcolour, fgcol.data());
        strcpy(m_zintSymbol->bgcolour, bgcol.data());
        m_error = ZBarcode_Encode_and_Print(m_zintSymbol, (unsigned char*) bstr.data(), bstr.length(), 0);
        if (m_error >= 5) {
            m_lastError = m_zintSymbol->errtxt;
            return false;
        } else {
            return true;
        }
    }

    void QZint::render(QPainter & painter, const QRectF & paintRect, AspectRatioMode mode) {
        struct zint_vector_rect *rect;
        struct zint_vector_hexagon *hex;
        struct zint_vector_circle *circle;
        struct zint_vector_string *string;
        qreal radius;

        encode();

        QFont fontSmall(fontstyle);
        fontSmall.setPixelSize(fontPixelSizeSmall);
        QFont fontLarge(fontstyle);
        fontLarge.setPixelSize(fontPixelSizeLarge);

        if (m_error >= 5) {
            // Display error message instead of barcode
            fontLarge.setPointSize(14);
            painter.setFont(fontLarge);
            painter.drawText(paintRect, Qt::AlignCenter, m_lastError);
            return;
        }

        painter.save();
        painter.setClipRect(paintRect, Qt::IntersectClip);

        qreal xtr = paintRect.x();
        qreal ytr = paintRect.y();
        qreal scale;
        
        qreal gwidth = m_zintSymbol->vector->width;
        qreal gheight = m_zintSymbol->vector->height;

        if (paintRect.width() / gwidth < paintRect.height() / gheight) {
            scale = paintRect.width() / gwidth;
        } else {
            scale = paintRect.height() / gheight;
        }
        
        xtr += (qreal) (paintRect.width() - gwidth * scale) / 2.0;
        ytr += (qreal) (paintRect.height() - gheight * scale) / 2.0;

        painter.setBackground(QBrush(m_bgColor));
        painter.fillRect(paintRect, QBrush(m_bgColor));
        
        painter.translate(xtr, ytr);
        painter.scale(scale, scale);
        
        //Red square for diagnostics
        //painter.fillRect(QRect(0, 0, m_zintSymbol->vector->width, m_zintSymbol->vector->height), QBrush(QColor(255,0,0,255)));

        QPen p;

        p.setWidth(1);
        p.setColor(m_fgColor);
        painter.setPen(p);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Plot rectangles
        rect = m_zintSymbol->vector->rectangles;
        while (rect) {
            painter.fillRect(rect->x, rect->y, rect->width, rect->height, QBrush(m_fgColor));
            rect = rect->next;
        }
        
        // Plot hexagons
        hex = m_zintSymbol->vector->hexagons;
        while (hex) {
            radius = hex->diameter / 2.0;
            
            QPainterPath pt;
            pt.moveTo(hex->x, hex->y + (1.0 * radius));
            pt.lineTo(hex->x + (0.86 * radius), hex->y + (0.5 * radius));
            pt.lineTo(hex->x + (0.86 * radius), hex->y - (0.5 * radius));
            pt.lineTo(hex->x, hex->y - (1.0 * radius));
            pt.lineTo(hex->x - (0.86 * radius), hex->y - (0.5 * radius));
            pt.lineTo(hex->x - (0.86 * radius), hex->y + (0.5 * radius));
            pt.lineTo(hex->x, hex->y + (1.0 * radius));
            painter.fillPath(pt, QBrush(m_fgColor));
            
            hex = hex->next;
        }
        
        // Plot dots (circles)
        circle = m_zintSymbol->vector->circles;
        while (circle) {
            if (circle->colour) {
                p.setColor(m_bgColor);
                p.setWidth(0);
                painter.setPen(p);
                painter.setBrush(QBrush(m_bgColor));
            } else {
                p.setColor(m_fgColor);
                p.setWidth(0);
                painter.setPen(p);
                painter.setBrush(QBrush(m_fgColor));
            }
            painter.drawEllipse(QPointF(circle->x, circle->y), (double) circle->diameter / 2.0, (double) circle->diameter / 2.0);
            circle = circle->next;
        }
        
        // Plot text
        string = m_zintSymbol->vector->strings;
        if (string) {
            painter.setFont(fontLarge);
        }
        QFontMetrics fm(fontLarge);
        while (string) {
            QString content = QString::fromUtf8((const char *) string->text, -1);
            int width = fm.width(content, -1);
            int height = fm.height();
            painter.drawText(string->x - (width / 2.0), string->y - height, width, height, Qt::AlignHCenter | Qt::AlignBottom, content);
            string = string->next;
        }
        
        painter.restore();
    }

    const QString & QZint::lastError() const {
        return m_lastError;
    }

    bool QZint::hasErrors() {
        return m_lastError.length();
    }

}

