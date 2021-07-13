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

#ifdef _MSC_VER
#if _MSC_VER >= 1900 /* MSVC 2015 */
#pragma warning(disable: 4996) /* function or variable may be unsafe */
#endif
#endif

//#include <QDebug>
#include "qzint.h"
#include <stdio.h>
#include <math.h>
#include <QFontMetrics>
/* the following include was necessary to compile with QT 5.18 on Windows */
/* QT 8.7 did not require it. */
#include <QPainterPath>

namespace Zint {
    static const char *fontStyle = "Helvetica";
    static const char *fontStyleError = "Helvetica";
    static const int fontSizeError = 14; /* Point size */

    QZint::QZint() {
        m_symbol = BARCODE_CODE128;
        m_height = 0.0f;
        m_borderType = 0;
        m_borderWidth = 0;
        m_fontSetting = 0;
        m_option_1 = -1;
        m_fgColor = Qt::black;
        m_bgColor = Qt::white;
        m_cmyk = false;
        m_zintSymbol = NULL;
        m_error = 0;
        m_input_mode = UNICODE_MODE;
        m_scale = 1.0f;
        m_option_3 = 0;
        m_show_hrt = 1;
        m_eci = 0;
        m_dotty = false;
        m_dot_size = 4.0f / 5.0f;
        target_size_horiz = 0;
        target_size_vert = 0;
        m_option_2 = 0;
        m_whitespace = 0;
        m_vwhitespace = 0;
        m_gs1parens = false;
        m_gs1nocheck = false;
        m_gssep = false;
        m_reader_init = false;
        m_rotate_angle = 0;
        m_debug = false;
    }

    QZint::~QZint() {
        if (m_zintSymbol)
            ZBarcode_Delete(m_zintSymbol);
    }

    void QZint::resetSymbol() {
        if (m_zintSymbol)
            ZBarcode_Delete(m_zintSymbol);

        m_lastError.clear();
        m_zintSymbol = ZBarcode_Create();
        m_zintSymbol->output_options |= m_borderType | m_fontSetting;
        m_zintSymbol->symbology = m_symbol;
        m_zintSymbol->height = m_height;
        m_zintSymbol->whitespace_width = m_whitespace;
        m_zintSymbol->whitespace_height = m_vwhitespace;
        m_zintSymbol->border_width = m_borderWidth;
        m_zintSymbol->option_1 = m_option_1;
        m_zintSymbol->input_mode = m_input_mode;
        m_zintSymbol->option_2 = m_option_2;
        if (m_dotty) {
            m_zintSymbol->output_options |= BARCODE_DOTTY_MODE;
        }
        m_zintSymbol->dot_size = m_dot_size;
        m_zintSymbol->show_hrt = m_show_hrt ? 1 : 0;
        m_zintSymbol->eci = m_eci;
        m_zintSymbol->option_3 = m_option_3;
        m_zintSymbol->scale = m_scale;
        if (m_gs1parens) {
            m_zintSymbol->input_mode |= GS1PARENS_MODE;
        }
        if (m_gs1nocheck) {
            m_zintSymbol->input_mode |= GS1NOCHECK_MODE;
        }
        if (m_gssep) {
            m_zintSymbol->output_options |= GS1_GS_SEPARATOR;
        }
        if (m_reader_init) {
            m_zintSymbol->output_options |= READER_INIT;
        }
        if (m_debug) {
            m_zintSymbol->debug |= ZINT_DEBUG_PRINT;
        }

        strcpy(m_zintSymbol->fgcolour, m_fgColor.name().toLatin1().right(6));
        if (m_fgColor.alpha() != 0xff) {
            strcat(m_zintSymbol->fgcolour, m_fgColor.name(QColor::HexArgb).toLatin1().mid(1,2));
        }
        strcpy(m_zintSymbol->bgcolour, m_bgColor.name().toLatin1().right(6));
        if (m_bgColor.alpha() != 0xff) {
            strcat(m_zintSymbol->bgcolour, m_bgColor.name(QColor::HexArgb).toLatin1().mid(1,2));
        }
        if (m_cmyk) {
            m_zintSymbol->output_options |= CMYK_COLOUR;
        }
        strcpy(m_zintSymbol->primary, m_primaryMessage.toLatin1().left(127));
    }

    void QZint::encode() {
        resetSymbol();
        QByteArray bstr = m_text.toUtf8();
        m_error = ZBarcode_Encode_and_Buffer_Vector(m_zintSymbol, (unsigned char *) bstr.data(), bstr.length(), 0); /* Note do our own rotation */
        m_lastError = m_zintSymbol->errtxt;

        if (m_error < ZINT_ERROR) {
            m_borderType = m_zintSymbol->output_options & (BARCODE_BIND | BARCODE_BOX);
            m_height = m_zintSymbol->height;
            m_borderWidth = m_zintSymbol->border_width;
            m_whitespace = m_zintSymbol->whitespace_width;
            m_vwhitespace = m_zintSymbol->whitespace_height;
            emit encoded();
        }
    }

    int QZint::symbol() const {
        return m_symbol;
    }

    void QZint::setSymbol(int symbol) {
        m_symbol = symbol;
    }

    int QZint::inputMode() const {
        return m_input_mode;
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

    QString QZint::primaryMessage() const {
        return m_primaryMessage;
    }

    void QZint::setPrimaryMessage(const QString & primaryMessage) {
        m_primaryMessage = primaryMessage;
    }

    float QZint::height() const {
        return m_height;
    }

    void QZint::setHeight(float height) {
        m_height = height;
    }

    int QZint::option2() const {
        return m_option_2;
    }

    void QZint::setOption2(int option) {
        m_option_2 = option;
    }

    int QZint::option3() const {
        return m_option_3;
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

    bool QZint::dotty() const {
        return m_dotty;
    }

    void QZint::setDotty(bool dotty) {
        m_dotty = dotty;
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

    void QZint::setCMYK(bool cmyk) {
        m_cmyk = cmyk;
    }

    int QZint::borderType() const {
        return m_borderType;
    }

    void QZint::setBorderType(int borderTypeIndex) {
        if (borderTypeIndex == 1) {
            m_borderType = BARCODE_BIND;
        } else if (borderTypeIndex == 2) {
            m_borderType = BARCODE_BOX;
        } else {
            m_borderType = 0;
        }
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

    void QZint::setVWhitespace(int vwhitespace) {
        m_vwhitespace = vwhitespace;
    }

    int QZint::option1() const {
        return m_option_1;
    }

    void QZint::setOption1(int option_1) {
        m_option_1 = option_1;
    }

    void QZint::setFontSetting(int fontSettingIndex) {
        if (fontSettingIndex == 1) {
            m_fontSetting = BOLD_TEXT;
        } else if (fontSettingIndex == 2) {
            m_fontSetting = SMALL_TEXT;
        } else if (fontSettingIndex == 3) {
            m_fontSetting = SMALL_TEXT | BOLD_TEXT;
        } else {
            m_fontSetting = 0;
        }
    }

    void QZint::setShowText(bool show) {
        m_show_hrt = show;
    }

    void QZint::setTargetSize(int width, int height) {
        target_size_horiz = width;
        target_size_vert = height;
    }

    void QZint::setGSSep(bool gssep) {
        m_gssep = gssep;
    }

    int QZint::rotateAngle() const {
        return m_rotate_angle;
    }

    void QZint::setRotateAngle(int rotateIndex) {
        if (rotateIndex == 1) {
            m_rotate_angle = 90;
        } else if (rotateIndex == 2) {
            m_rotate_angle = 180;
        } else if (rotateIndex == 3) {
            m_rotate_angle = 270;
        } else {
            m_rotate_angle = 0;
        }
    }

    void QZint::setECI(int ECIIndex) {
        if (ECIIndex >= 1 && ECIIndex <= 11) {
            m_eci = ECIIndex + 2;
        } else if (ECIIndex >= 12 && ECIIndex <= 15) {
            m_eci = ECIIndex + 3;
        } else if (ECIIndex >= 16 && ECIIndex <= 26) {
            m_eci = ECIIndex + 4;
        } else if (ECIIndex == 27) {
            m_eci = 899; /* 8-bit binary data */
        } else {
            m_eci = 0;
        }
    }

    void QZint::setGS1Parens(bool gs1parens) {
        m_gs1parens = gs1parens;
    }

    void QZint::setGS1NoCheck(bool gs1nocheck) {
        m_gs1nocheck = gs1nocheck;
    }

    void QZint::setReaderInit(bool reader_init) {
        m_reader_init = reader_init;
    }

    void QZint::setDebug(bool debug) {
        m_debug = debug;
    }

    /* Legacy */
    void QZint::setWidth(int width) { setOption1(width); }
    int QZint::width() const { return m_option_1; }
    void QZint::setSecurityLevel(int securityLevel) { setOption2(securityLevel); }
    int QZint::securityLevel() const { return m_option_2; }
    void QZint::setPdf417CodeWords(int /*pdf417CodeWords*/) {}
    int QZint::pdf417CodeWords() const { return 0; }
    void QZint::setHideText(bool hide) { setShowText(!hide); }

    bool QZint::hasHRT(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_HRT);
    }

    bool QZint::isExtendable(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_EXTENDABLE);
    }

    bool QZint::supportsECI(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_ECI);
    }

    bool QZint::supportsGS1(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_GS1);
    }

    bool QZint::isFixedRatio(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_FIXED_RATIO);
    }

    bool QZint::isDotty(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_DOTTY);
    }

    bool QZint::supportsReaderInit(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_READER_INIT);
    }

    int QZint::getError() const {
        return m_error;
    }

    QString QZint::error_message() const {
        return m_lastError;
    }

    const QString & QZint::lastError() const {
        return m_lastError;
    }

    bool QZint::hasErrors() const {
        return m_lastError.length();
    }
    
    int QZint::getVersion() const {
        return ZBarcode_Version();
    }

    bool QZint::save_to_file(QString filename) {
        resetSymbol();
        strcpy(m_zintSymbol->outfile, filename.toLatin1().left(255));
        QByteArray bstr = m_text.toUtf8();
        m_error = ZBarcode_Encode_and_Print(m_zintSymbol, (unsigned char *) bstr.data(), bstr.length(), m_rotate_angle);
        if (m_error >= ZINT_ERROR) {
            m_lastError = m_zintSymbol->errtxt;
            return false;
        } else {
            return true;
        }
    }

    Qt::GlobalColor QZint::colourToQtColor(int colour) {
        switch (colour) {
            case 1: // Cyan
                return Qt::cyan;
                break;
            case 2: // Blue
                return Qt::blue;
                break;
            case 3: // Magenta
                return Qt::magenta;
                break;
            case 4: // Red
                return Qt::red;
                break;
            case 5: // Yellow
                return Qt::yellow;
                break;
            case 6: // Green
                return Qt::green;
                break;
            case 8: // White
                return Qt::white;
                break;
            default:
                return Qt::black;
                break;
        }
    }

    void QZint::render(QPainter & painter, const QRectF & paintRect, AspectRatioMode mode) {
        struct zint_vector_rect *rect;
        struct zint_vector_hexagon *hex;
        struct zint_vector_circle *circle;
        struct zint_vector_string *string;

        (void)mode; /* Not currently used */

        encode();

        painter.save();

        if (m_error >= ZINT_ERROR) {
            painter.setRenderHint(QPainter::Antialiasing);
            QFont font(fontStyleError, fontSizeError);
            painter.setFont(font);
            painter.drawText(paintRect, Qt::AlignCenter | Qt::TextWordWrap, m_lastError);
            painter.restore();
            return;
        }

        painter.setClipRect(paintRect, Qt::IntersectClip);

        qreal xtr = paintRect.x();
        qreal ytr = paintRect.y();
        qreal scale;

        qreal gwidth = m_zintSymbol->vector->width;
        qreal gheight = m_zintSymbol->vector->height;

        if (m_rotate_angle == 90 || m_rotate_angle == 270) {
            if (paintRect.width() / gheight < paintRect.height() / gwidth) {
                scale = paintRect.width() / gheight;
            } else {
                scale = paintRect.height() / gwidth;
            }
        } else {
            if (paintRect.width() / gwidth < paintRect.height() / gheight) {
                scale = paintRect.width() / gwidth;
            } else {
                scale = paintRect.height() / gheight;
            }
        }

        xtr += (qreal) (paintRect.width() - gwidth * scale) / 2.0;
        ytr += (qreal) (paintRect.height() - gheight * scale) / 2.0;

        if (m_rotate_angle) {
            painter.translate(paintRect.width() / 2.0, paintRect.height() / 2.0); // Need to rotate around centre
            painter.rotate(m_rotate_angle);
            painter.translate(-paintRect.width() / 2.0, -paintRect.height() / 2.0); // Undo
        }

        painter.translate(xtr, ytr);
        painter.scale(scale, scale);

        QBrush bgBrush(m_bgColor);
        painter.fillRect(QRectF(0, 0, gwidth, gheight), bgBrush);

        //Red square for diagnostics
        //painter.fillRect(QRect(0, 0, m_zintSymbol->vector->width, m_zintSymbol->vector->height), QBrush(QColor(255,0,0,255)));

        // Plot rectangles
        rect = m_zintSymbol->vector->rectangles;
        if (rect) {
            QBrush brush(Qt::SolidPattern);
            while (rect) {
                if (rect->colour == -1) {
                    brush.setColor(m_fgColor);
                } else {
                    brush.setColor(colourToQtColor(rect->colour));
                }
                painter.fillRect(QRectF(rect->x, rect->y, rect->width, rect->height), brush);
                rect = rect->next;
            }
        }

        // Plot hexagons
        hex = m_zintSymbol->vector->hexagons;
        if (hex) {
            painter.setRenderHint(QPainter::Antialiasing);
            QBrush fgBrush(m_fgColor);
            qreal previous_diameter = 0.0, radius = 0.0, half_radius = 0.0, half_sqrt3_radius = 0.0;
            while (hex) {
                if (previous_diameter != hex->diameter) {
                    previous_diameter = hex->diameter;
                    radius = 0.5 * previous_diameter;
                    half_radius = 0.25 * previous_diameter;
                    half_sqrt3_radius = 0.43301270189221932338 * previous_diameter;
                }

                QPainterPath pt;
                pt.moveTo(hex->x, hex->y + radius);
                pt.lineTo(hex->x + half_sqrt3_radius, hex->y + half_radius);
                pt.lineTo(hex->x + half_sqrt3_radius, hex->y - half_radius);
                pt.lineTo(hex->x, hex->y - radius);
                pt.lineTo(hex->x - half_sqrt3_radius, hex->y - half_radius);
                pt.lineTo(hex->x - half_sqrt3_radius, hex->y + half_radius);
                pt.lineTo(hex->x, hex->y + radius);
                painter.fillPath(pt, fgBrush);

                hex = hex->next;
            }
        }

        // Plot dots (circles)
        circle = m_zintSymbol->vector->circles;
        if (circle) {
            painter.setRenderHint(QPainter::Antialiasing);
            QPen p;
            QBrush fgBrush(m_fgColor);
            qreal previous_diameter = 0.0, radius = 0.0;
            while (circle) {
                if (previous_diameter != circle->diameter) {
                    previous_diameter = circle->diameter;
                    radius = 0.5 * previous_diameter;
                }
                if (circle->colour) { // Set means use background colour
                    p.setColor(m_bgColor);
                    p.setWidth(0);
                    painter.setPen(p);
                    painter.setBrush(bgBrush);
                } else {
                    p.setColor(m_fgColor);
                    p.setWidth(0);
                    painter.setPen(p);
                    painter.setBrush(fgBrush);
                }
                painter.drawEllipse(QPointF(circle->x, circle->y), radius, radius);
                circle = circle->next;
            }
        }

        // Plot text
        string = m_zintSymbol->vector->strings;
        if (string) {
            painter.setRenderHint(QPainter::Antialiasing);
            QPen p;
            p.setColor(m_fgColor);
            painter.setPen(p);
            bool bold = (m_zintSymbol->output_options & BOLD_TEXT) && (!isExtendable() || (m_zintSymbol->output_options & SMALL_TEXT));
            QFont font(fontStyle, -1 /*pointSize*/, bold ? QFont::Bold : -1);
            while (string) {
                font.setPixelSize(string->fsize);
                painter.setFont(font);
                QString content = QString::fromUtf8((const char *) string->text);
                /* string->y is baseline of font */
                if (string->halign == 1) { /* Left align */
                    painter.drawText(QPointF(string->x, string->y), content);
                } else {
                    QFontMetrics fm(painter.fontMetrics());
                    int width = fm.boundingRect(content).width();
                    if (string->halign == 2) { /* Right align */
                        painter.drawText(QPointF(string->x - width, string->y), content);
                    } else { /* Centre align */
                        painter.drawText(QPointF(string->x - (width / 2.0), string->y), content);
                    }
                }
                string = string->next;
            }
        }

        painter.restore();
    }
}
