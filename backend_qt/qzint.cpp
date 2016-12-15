/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bogdan@licentia.eu                                                    *
 *   Copyright (C) 2010-2016 Robin Stuart                                  *
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

namespace Zint {

    static const qreal maxi_diagonal = 11;
    static const qreal maxi_width = 1.73205807568877 * maxi_diagonal / 2;
    static const char* fontstyle = "Arial";
    static const int fontPixelSizeSmall = 6;
    static const int fontPixelSizeLarge = 8;

    QZint::QZint() {
        m_symbol = BARCODE_CODE128;
        m_height = 50;
        m_border = NO_BORDER;
        m_borderWidth = 1;
        m_securityLevel = -1;
        m_pdf417CodeWords = 928;
        m_fgColor = Qt::black;
        m_bgColor = Qt::white;
        m_zintSymbol = 0;
        m_error = 0;
        m_input_mode = UNICODE_MODE;
        m_scale = 1.0;
        m_option_3 = 0;
        m_hidetext = 0;
        m_dot_size = 4.0 / 5.0;
        target_size_horiz = 0;
        target_size_vert = 0;
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
        if (m_symbol == BARCODE_PDF417) {
            m_zintSymbol->option_3 = m_pdf417CodeWords;
        } else {
            m_zintSymbol->option_3 = m_option_3;
        }
        QByteArray bstr = m_text.toUtf8();
        QByteArray pstr = m_primaryMessage.left(99).toLatin1();
        strcpy(m_zintSymbol->primary, pstr.data());
        int error = ZBarcode_Encode(m_zintSymbol, (unsigned char*) bstr.data(), bstr.length());
        if (error > ZINT_WARN_INVALID_OPTION)
            m_lastError = m_zintSymbol->errtxt;

        if (m_zintSymbol->symbology == BARCODE_MAXICODE)
            m_zintSymbol->height = 33;

        switch (m_zintSymbol->output_options) {
            case 0: m_border = NO_BORDER;
                break;
            case 2: m_border = BIND;
                break;
            case 4: m_border = BOX;
                break;
        }
        m_borderWidth = (BorderType) m_zintSymbol->border_width;
        m_whitespace = m_zintSymbol->whitespace_width;
    }

    int QZint::symbol() {
        return m_symbol;
    }

    void QZint::setSymbol(int symbol) {
        m_symbol = symbol;
    }

    void QZint::setInputMode(int input_mode) {
        m_input_mode = input_mode;
    }

    QString QZint::text() {
        return m_text;
    }

    void QZint::setText(const QString & text) {
        m_text = text;
    }
    
    void QZint::setTargetSize(int width, int height) {
        target_size_horiz = width;
        target_size_vert = height;
    }

    QString QZint::primaryMessage() {
        return m_primaryMessage;
    }

    void QZint::setPrimaryMessage(const QString & primaryMessage) {
        m_primaryMessage = primaryMessage;
    }

    int QZint::height() {
        encode();
        return (m_zintSymbol->height + (m_border != NO_BORDER) ? m_borderWidth * 2 : 0)*(m_zintSymbol->symbology == BARCODE_MAXICODE ? (maxi_width + 1) : 1);
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

    int QZint::width() {
        encode();
        return (m_zintSymbol->width + (m_border == BOX) ? m_borderWidth * 2 : 0)*(m_zintSymbol->symbology == BARCODE_MAXICODE ? (maxi_width + 1) : 1);
    }

    float QZint::scale() {
        return m_scale;
    }

    void QZint::setScale(float scale) {
        m_scale = scale;
    }
    
    void QZint::setDotSize(float dot_size) {
        m_dot_size = dot_size;
    }

    QColor QZint::fgColor() {
        return m_fgColor;
    }

    void QZint::setFgColor(const QColor & fgColor) {
        m_fgColor = fgColor;
    }

    QColor QZint::bgColor() {
        return m_bgColor;
    }

    void QZint::setBgColor(const QColor & bgColor) {
        m_bgColor = bgColor;
    }

    QZint::BorderType QZint::borderType() {
        return m_border;
    }

    void QZint::setBorderType(BorderType border) {
        m_border = border;
    }

    int QZint::borderWidth() {
        return m_borderWidth;
    }

    void QZint::setBorderWidth(int boderWidth) {
        if (boderWidth < 1 || boderWidth > 16)
            boderWidth = 1;
        m_borderWidth = boderWidth;
    }

    void QZint::setWhitespace(int whitespace) {
        m_whitespace = whitespace;
    }

    int QZint::pdf417CodeWords() {
        return m_pdf417CodeWords;
    }

    void QZint::setPdf417CodeWords(int pdf417CodeWords) {
        m_pdf417CodeWords = pdf417CodeWords;
    }

    int QZint::securityLevel() {
        return m_securityLevel;
    }

    void QZint::setSecurityLevel(int securityLevel) {
        m_securityLevel = securityLevel;
    }

    QString QZint::error_message() {
        return m_lastError;
    }

    int QZint::mode() {
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
        if (m_symbol == BARCODE_PDF417) {
            m_zintSymbol->option_3 = m_pdf417CodeWords;
        } else {
            m_zintSymbol->option_3 = m_option_3;
        }
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
        int error = ZBarcode_Encode(m_zintSymbol, (unsigned char*) bstr.data(), bstr.length());
        if (error > ZINT_WARN_INVALID_OPTION)
            m_lastError = m_zintSymbol->errtxt;
        error = ZBarcode_Print(m_zintSymbol, 0);
        if (error > ZINT_WARN_INVALID_OPTION)
            m_lastError = m_zintSymbol->errtxt;
        if (error == 0) {
            return true;
        } else {
            return false;
        }
    }

    int QZint::module_set(int y_coord, int x_coord) {
        int x_char, x_sub, result;

        x_char = x_coord / 7;
        x_sub = x_coord % 7;
        result = 0;

        if (m_zintSymbol->encoded_data[y_coord][x_char] & (0x01 << x_sub)) {
            result = 1;
        }

        return result;
    }

    void QZint::render(QPainter & painter, const QRectF & paintRect, AspectRatioMode mode) {
        bool textdone;
        int comp_offset = 0;
        int xoffset = m_whitespace;
        int j, main_width = 0, addon_text_height = 0;
        int yoffset = 0;
        
        encode();
        
        QString caption = QString::fromUtf8((const char *) m_zintSymbol->text, -1);
        QFont fontSmall(fontstyle);
        fontSmall.setPixelSize(fontPixelSizeSmall);
        QFont fontLarge(fontstyle);
        fontLarge.setPixelSize(fontPixelSizeLarge);
        
        if (m_lastError.length()) {
            fontLarge.setPointSize(14);
            painter.setFont(fontLarge);
            painter.drawText(paintRect, Qt::AlignCenter, m_lastError);
            return;
        }

        painter.save();
        painter.setClipRect(paintRect, Qt::IntersectClip);
        
        qreal xtr = paintRect.x();
        qreal ytr = paintRect.y();

        int zrow_height = m_zintSymbol->height;
        int zrows = 0;
        for (int i = 0; i < m_zintSymbol->rows; i++) {
            zrow_height -= m_zintSymbol->row_height[i];
            if (!m_zintSymbol->row_height[i])
                zrows++;
        }
        if (zrows) {
            zrow_height /= zrows;
            for (int i = 0; i < m_zintSymbol->rows; i++)
                if (!m_zintSymbol->row_height[i])
                    m_zintSymbol->row_height[i] = zrow_height;
        } else
            m_zintSymbol->height -= zrow_height;


        qreal gwidth = m_zintSymbol->width;
        qreal gheight = m_zintSymbol->height;
        if (m_zintSymbol->symbology == BARCODE_MAXICODE) {
            gwidth = (33.0 * maxi_width) + xoffset + xoffset;
            gheight = (32.0 * maxi_width) + yoffset + yoffset;
        }

        if (m_zintSymbol->output_options & BARCODE_DOTTY_MODE) {
            gwidth += 2.0;
            gheight += 2.0;
        }

        qreal xsf = 1;
        qreal ysf = 1;
        qreal textoffset = 0;

        gwidth += ((m_border == BOX) ? m_borderWidth * 2 : 0);
        gheight += ((m_border != NO_BORDER) ? m_borderWidth * 2 : 0);
        if (QString((const char*) m_zintSymbol->text).isEmpty() == false) {
            textoffset = 9;
            gheight += textoffset;
        } else {
            textoffset = 0;
        }
        gwidth += m_zintSymbol->whitespace_width * 2;
        
        if (paintRect.width() / gwidth < paintRect.height() / gheight) {
            ysf = xsf = (qreal) paintRect.width() / gwidth;
            ytr += (qreal) (paintRect.height() - gheight * ysf) / 2;
        } else {
            ysf = xsf = (qreal) paintRect.height() / gheight;
            xtr += (qreal) (paintRect.width() - gwidth * xsf) / 2;
        }

        painter.setBackground(QBrush(m_bgColor));
        painter.fillRect(paintRect, QBrush(m_bgColor));
        painter.translate(xtr, ytr);
        painter.scale(xsf, ysf);

        QPen p;
        
        p.setColor(m_fgColor);
        p.setWidth(m_borderWidth);
        painter.setPen(p);

        if (m_zintSymbol->symbology != BARCODE_MAXICODE) {
            /* Draw boundary bars or boxes around the symbol */
            switch (m_border) {
                case BOX:
                    painter.fillRect(0, m_borderWidth, m_borderWidth, m_zintSymbol->height, QBrush(m_fgColor));
                    painter.fillRect(m_zintSymbol->width + xoffset + xoffset + m_borderWidth, m_borderWidth, m_borderWidth, m_zintSymbol->height, QBrush(m_fgColor));
                    painter.fillRect(0, 0, m_zintSymbol->width + xoffset + xoffset + m_borderWidth + m_borderWidth, m_borderWidth, QBrush(m_fgColor));
                    painter.fillRect(0, m_zintSymbol->height + m_borderWidth, m_zintSymbol->width + xoffset + xoffset + m_borderWidth + m_borderWidth, m_borderWidth, QBrush(m_fgColor));
                    painter.translate(m_borderWidth + m_zintSymbol->whitespace_width, m_borderWidth);
                    yoffset = m_borderWidth;
                    break;
                case BIND:
                    if (m_zintSymbol->symbology != BARCODE_CODABLOCKF) {
                        painter.fillRect(0, 0, m_zintSymbol->width + xoffset + xoffset, m_borderWidth, QBrush(m_fgColor));
                        painter.fillRect(0, m_zintSymbol->height, m_zintSymbol->width + xoffset + xoffset, m_borderWidth, QBrush(m_fgColor));
                    } else {
                        painter.fillRect(xoffset, 0, m_zintSymbol->width, m_borderWidth, QBrush(m_fgColor));
                        painter.fillRect(xoffset, m_zintSymbol->height, m_zintSymbol->width, m_borderWidth, QBrush(m_fgColor));
                    }
                    painter.translate(m_zintSymbol->whitespace_width, m_borderWidth);
                    yoffset = m_borderWidth;
                    break;

                default:
                    painter.translate(m_zintSymbol->whitespace_width, 0);
                    break;
                    ;
            }
        }

        while (!(module_set(m_zintSymbol->rows - 1, comp_offset))) {
            comp_offset++;
        }
        xoffset = comp_offset;

        /* Set up some values for displaying EAN and UPC symbols correctly */
        main_width = m_zintSymbol->width;
        if ((((m_zintSymbol->symbology == BARCODE_EANX) && (m_zintSymbol->rows == 1)) || (m_zintSymbol->symbology == BARCODE_EANX_CC))
                || (m_zintSymbol->symbology == BARCODE_ISBNX)) {
            switch (caption.size()) {
                case 13: /* EAN 13 */
                case 16:
                case 19:
                    if (m_zintSymbol->whitespace_width == 0) {
                        m_zintSymbol->whitespace_width = 10;
                    }
                    main_width = 96 + comp_offset;
                    break;
                default:
                    main_width = 68 + comp_offset;
                    break;
            }
        }

        if (((m_zintSymbol->symbology == BARCODE_UPCA) && (m_zintSymbol->rows == 1)) || (m_zintSymbol->symbology == BARCODE_UPCA_CC)) {
            if (m_zintSymbol->whitespace_width == 0) {
                m_zintSymbol->whitespace_width = 10;
            }
            main_width = 96 + comp_offset;
        }

        if (((m_zintSymbol->symbology == BARCODE_UPCE) && (m_zintSymbol->rows == 1)) || (m_zintSymbol->symbology == BARCODE_UPCE_CC)) {
            if (m_zintSymbol->whitespace_width == 0) {
                m_zintSymbol->whitespace_width = 10;
            }
            main_width = 51 + comp_offset;
        }

        p.setWidth(1);
        painter.setPen(p);

        if (m_zintSymbol->symbology == BARCODE_MAXICODE) {
            /* Draw Maxicode with hexagons */
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            for (int r = 0; r < m_zintSymbol->rows; r++) {
                for (int c = 0; c < m_zintSymbol->width; c++) {
                    if (module_set(r, c)) {
                        qreal col = (qreal) c * (maxi_width + 1)+(r % 2)*((maxi_width + 1) / 2);
                        qreal row = (qreal) r * (maxi_width + 1)*0.868;
                        QPainterPath pt;
                        pt.moveTo(col + maxi_width / 2, row);
                        pt.lineTo(col + maxi_width, row + maxi_diagonal / 4);
                        pt.lineTo(col + maxi_width, row + (maxi_diagonal - maxi_diagonal / 4));
                        pt.lineTo(col + maxi_width / 2, row + maxi_diagonal);
                        pt.lineTo(col, row + (maxi_diagonal - maxi_diagonal / 4));
                        pt.lineTo(col, row + maxi_diagonal / 4);
                        pt.lineTo(col + maxi_width / 2, row);
                        painter.fillPath(pt, QBrush(m_fgColor));
                    }
                }
            }
            p.setWidth(maxi_width);
            painter.setPen(p);
            const qreal w = maxi_width + 1;
            painter.drawEllipse(QPointF(14.5 * w, 16.5 * w * 0.868), w, w);
            painter.drawEllipse(QPointF(14.5 * w, 16.5 * w * 0.868), w + w * 1.5, w + w * 1.5);
            painter.drawEllipse(QPointF(14.5 * w, 16.5 * w * 0.868), w + w * 3, w + w * 3);
            painter.restore();
        } else if (m_zintSymbol->output_options & BARCODE_DOTTY_MODE) {
            /* Draw with dots (circles) */

            p.setColor(m_fgColor);
            p.setWidth(0);
            painter.setPen(p);
            painter.setBrush(QBrush(m_fgColor));
            painter.setRenderHint(QPainter::Antialiasing);
            for (int r = 0; r < m_zintSymbol->rows; r++) {
                for (int c = 0; c < m_zintSymbol->width; c++) {
                    if (module_set(r, c)) {

                        painter.drawEllipse(QPointF((c + 1.0), (r + 1.0)), m_dot_size / 2.0, m_dot_size / 2.0);
                    }
                }
            }
        } else {
            /* Draw all other symbols with rectangles */
            int y = 0;
            for (int row = 0; row < m_zintSymbol->rows; row++) {
                for (int i = 0; i < m_zintSymbol->width; i++) {
                    if (module_set(row, i)) {
                        int ed = module_set(row, i);
                        int linewidth = 0;
                        for (int j = i; j < m_zintSymbol->width; j++, linewidth++)
                            if (ed != module_set(row, j))
                                break;
                        QColor color;
                        color = m_fgColor;

                        if (!((i > main_width) && (row == m_zintSymbol->rows - 1))) {
                            painter.fillRect(i, y, linewidth, m_zintSymbol->row_height[row], QBrush(color));
                        } else {
                            painter.fillRect(i, y + 8, linewidth, m_zintSymbol->row_height[row] - 3, QBrush(color));
                            addon_text_height = y;
                        }
                    }
                }
                /* Add row binding */
                if (((m_zintSymbol->symbology == BARCODE_CODE16K) 
                        || (m_zintSymbol->symbology == BARCODE_CODE49)) && (row != 0)) {
                    painter.fillRect(0, y - 1, m_zintSymbol->width, 2, QBrush(m_fgColor));
                }
                if ((m_zintSymbol->symbology == BARCODE_CODABLOCKF) && (row != 0)) {
                    painter.fillRect(11, y - 1, m_zintSymbol->width - 25, 2, QBrush(m_fgColor));
                }
                y += m_zintSymbol->row_height[row];
            }
        }

        textdone = false;

        if (m_hidetext == false) {
            painter.setFont(fontSmall);
            if (((m_zintSymbol->symbology == BARCODE_EANX) || (m_zintSymbol->symbology == BARCODE_EANX_CC)) ||
                    (m_zintSymbol->symbology == BARCODE_ISBNX)) {
                /* Add bridge and format text for EAN */
                switch (caption.size()) {
                    case 8:
                    case 11:
                    case 14:
                        painter.fillRect(0 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.fillRect(2 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.fillRect(32 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.fillRect(34 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.fillRect(64 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.fillRect(66 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.setFont(fontLarge);
                        painter.drawText(3 + xoffset, m_zintSymbol->height + yoffset, 29, 9, Qt::AlignCenter, caption.mid(0, 4));
                        painter.drawText(35 + xoffset, m_zintSymbol->height + yoffset, 29, 9, Qt::AlignCenter, caption.mid(4, 4));
                        if (caption.size() == 11) {
                            /* EAN-2 */ painter.drawText(76 + xoffset, addon_text_height, 20, 9, Qt::AlignCenter, caption.mid(9, 2));
                        };
                        if (caption.size() == 14) {
                            /* EAN-5 */ painter.drawText(76 + xoffset, addon_text_height, 47, 9, Qt::AlignCenter, caption.mid(9, 5));
                        };
                        painter.setFont(fontSmall);
                        textdone = true;
                        break;
                    case 13:
                    case 16:
                    case 19:
                        painter.fillRect(0 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.fillRect(2 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.fillRect(46 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.fillRect(48 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.fillRect(92 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.fillRect(94 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                        painter.setFont(fontLarge);
                        painter.drawText(xoffset - 7, m_zintSymbol->height + yoffset, 7, 9, Qt::AlignCenter, caption.mid(0, 1));
                        painter.drawText(3 + xoffset, m_zintSymbol->height + yoffset, 43, 9, Qt::AlignCenter, caption.mid(1, 6));
                        painter.drawText(49 + xoffset, m_zintSymbol->height + yoffset, 43, 9, Qt::AlignCenter, caption.mid(7, 6));
                        if (caption.size() == 16) {
                            /* EAN-2 */ painter.drawText(104 + xoffset, addon_text_height, 20, 9, Qt::AlignCenter, caption.mid(14, 2));
                        };
                        if (caption.size() == 19) {
                            /* EAN-5 */ painter.drawText(104 + xoffset, addon_text_height, 47, 9, Qt::AlignCenter, caption.mid(14, 5));
                        };
                        painter.setFont(fontSmall);
                        textdone = true;
                        break;
                }
                if (textdone == false) {
                    painter.setFont(fontLarge);
                    painter.drawText(0, m_zintSymbol->height, m_zintSymbol->width, 9, Qt::AlignCenter, caption);
                    painter.setFont(fontSmall);
                    textdone = true;
                }
            }

            if ((m_zintSymbol->symbology == BARCODE_UPCA) || (m_zintSymbol->symbology == BARCODE_UPCA_CC)) {
                /* Add bridge and format text for UPC-A */
                int block_width;
                bool latch = true;

                j = 0 + comp_offset;
                do {
                    block_width = 0;
                    do {
                        block_width++;
                    } while (module_set(m_zintSymbol->rows - 1, j + block_width) == module_set(m_zintSymbol->rows - 1, j));
                    if (latch == true) {
                        /* a bar */
                        painter.fillRect(j + xoffset - comp_offset, m_zintSymbol->height, block_width, 5, QBrush(m_fgColor));
                        latch = false;
                    } else {
                        /* a space */
                        latch = true;
                    }
                    j += block_width;
                } while (j < 11 + comp_offset);
                painter.fillRect(46 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                painter.fillRect(48 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                latch = true;
                j = 85 + comp_offset;
                do {
                    block_width = 0;
                    do {
                        block_width++;
                    } while (module_set(m_zintSymbol->rows - 1, j + block_width) == module_set(m_zintSymbol->rows - 1, j));
                    if (latch == true) {
                        /* a bar */
                        painter.fillRect(j + xoffset - comp_offset, m_zintSymbol->height, block_width, 5, QBrush(m_fgColor));
                        latch = false;
                    } else {
                        /* a space */
                        latch = true;
                    }
                    j += block_width;
                } while (j < 96 + comp_offset);
                painter.drawText(xoffset - 7, m_zintSymbol->height + yoffset + 2, 7, 7, Qt::AlignCenter, caption.mid(0, 1));
                painter.drawText(96 + xoffset, m_zintSymbol->height + yoffset + 2, 7, 7, Qt::AlignCenter, caption.mid(11, 1));
                painter.setFont(fontLarge);
                painter.drawText(11 + xoffset, m_zintSymbol->height + yoffset, 35, 9, Qt::AlignCenter, caption.mid(1, 5));
                painter.drawText(49 + xoffset, m_zintSymbol->height + yoffset, 35, 9, Qt::AlignCenter, caption.mid(6, 5));
                if (caption.size() == 15) {
                    /* EAN-2 */ painter.drawText(104 + xoffset, addon_text_height, 20, 9, Qt::AlignCenter, caption.mid(13, 2));
                };
                if (caption.size() == 18) {
                    /* EAN-5 */ painter.drawText(104 + xoffset, addon_text_height, 47, 9, Qt::AlignCenter, caption.mid(13, 5));
                };
                painter.setFont(fontSmall);
                textdone = true;
            }

            if ((m_zintSymbol->symbology == BARCODE_UPCE) || (m_zintSymbol->symbology == BARCODE_UPCE_CC)) {
                /* Add bridge and format text for UPC-E */
                painter.fillRect(0 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                painter.fillRect(2 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                painter.fillRect(46 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                painter.fillRect(48 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                painter.fillRect(50 + xoffset, m_zintSymbol->height, 1, 5, QBrush(m_fgColor));
                painter.drawText(xoffset - 7, m_zintSymbol->height + yoffset + 2, 7, 7, Qt::AlignCenter, caption.mid(0, 1));
                painter.drawText(51 + xoffset, m_zintSymbol->height + yoffset + 2, 7, 7, Qt::AlignCenter, caption.mid(7, 1));
                painter.setFont(fontLarge);
                painter.drawText(3 + xoffset, m_zintSymbol->height + yoffset, 43, 9, Qt::AlignCenter, caption.mid(1, 6));
                if (caption.size() == 11) {
                    /* EAN-2 */ painter.drawText(60 + xoffset, addon_text_height, 20, 9, Qt::AlignCenter, caption.mid(9, 2));
                };
                if (caption.size() == 14) {
                    /* EAN-2 */ painter.drawText(60 + xoffset, addon_text_height, 47, 9, Qt::AlignCenter, caption.mid(9, 5));
                };
                painter.setFont(fontSmall);
                textdone = true;
            }
        } /* if (m_hidetext == false) */

        if ((m_hidetext == false) && (textdone == false)) {
            /* Add text to any other symbol */
            painter.drawText(0, m_zintSymbol->height + yoffset, m_zintSymbol->width, 7, Qt::AlignCenter, caption);
        }
        painter.restore();
    }

    const QString & QZint::lastError() {
        return m_lastError;
    }

    bool QZint::hasErrors() {
        return m_lastError.length();
    }

}