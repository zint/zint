/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bogdan@licentia.eu                                                    *
 *   Copyright (C) 2010-2024 Robin Stuart                                  *
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

#ifdef _MSC_VER
#if _MSC_VER >= 1900 /* MSVC 2015 */
#pragma warning(disable: 4996) /* function or variable may be unsafe */
#endif
#endif

//#include <QDebug>
#include <QFontDatabase>
#include <QFontMetrics>
/* The following include is necessary to compile with Qt 5.15 on Windows; Qt 5.7 did not require it */
#include <QPainterPath>
#include <QRegularExpression>

#include <math.h>
#include <stdio.h>
#include "qzint.h"
#include "../backend/fonts/normal_ttf.h" /* Arimo */
#include "../backend/fonts/upcean_ttf.h" /* OCR-B subset (digits, "<", ">") */

// Shorthand
#define QSL     QStringLiteral
#define QSEmpty QLatin1String("")

namespace Zint {
    static const int maxSegs = 256;
    static const int maxCLISegs = 10; /* CLI restricted to 10 segments (including main data) */

    /* Matches RGB(A) hex string or CMYK decimal "C,M,Y,K" percentage string */
    static const QString colorREstr(
                                QSL("^([0-9A-Fa-f]{6}([0-9A-Fa-f]{2})?)|(((100|[0-9]{0,2}),){3}(100|[0-9]{0,2}))$"));
    Q_GLOBAL_STATIC_WITH_ARGS(QRegularExpression, colorRE, (colorREstr))

    static const QString normalFontFamily = QSL("Arimo"); /* Sans-serif metrically compatible with Arial */
    static const QString upceanFontFamily = QSL("OCRB"); /* Monospace OCR-B */
    static const QString fontFamilyError = QSL("Arimo");
    static const int fontSizeError = 14; /* Point size */

    static int normalFontID = -2; /* Use -2 as `addApplicationFontFromData()` returns -1 on error */

    /* Load Arimo from static array */
    static int loadNormalFont() {
        static const QByteArray normalFontArray
                                            = QByteArray::fromRawData((const char *) normal_ttf, sizeof(normal_ttf));

        normalFontID = QFontDatabase::addApplicationFontFromData(normalFontArray);
        return normalFontID;
    }

    static int upceanFontID = -2; /* Use -2 as `addApplicationFontFromData()` returns -1 on error */

    /* Load OCR-B EAN/UPC subset from static array */
    static int loadUpceanFont() {
        static const QByteArray upceanFontArray
                                            = QByteArray::fromRawData((const char *) upcean_ttf, sizeof(upcean_ttf));

        upceanFontID = QFontDatabase::addApplicationFontFromData(upceanFontArray);
        return upceanFontID;
    }

    /* Helper to convert QColor to RGB(A) hex string */
    static QString qcolor_to_str(const QColor &color) {
        if (color.alpha() == 0xFF) {
            return QString::asprintf("%02X%02X%02X", color.red(), color.green(), color.blue());
        }
        return QString::asprintf("%02X%02X%02X%02X", color.red(), color.green(), color.blue(), color.alpha());
    }

    /* Helper to convert RGB(A) hex string or CMYK decimal "C,M,Y,K" percentage string) to QColor */
    static QColor str_to_qcolor(const QString &text) {
        QColor color;
        int r, g, b, a;
        if (text.contains(',')) {
            int comma1 = text.indexOf(',');
            int comma2 = text.indexOf(',', comma1 + 1);
            int comma3 = text.indexOf(',', comma2 + 1);
            int black = 100 - text.mid(comma3 + 1).toInt();
            int val = 100 - text.mid(0, comma1).toInt();
            r = (int) roundf((0xFF * val * black) / 10000.0f);
            val = 100 - text.mid(comma1 + 1, comma2 - comma1 - 1).toInt();
            g = (int) roundf((0xFF * val * black) / 10000.0f);
            val = 100 - text.mid(comma2 + 1, comma3 - comma2 - 1).toInt();
            b = (int) roundf((0xFF * val * black) / 10000.0f);
            a = 0xFF;
        } else {
            r = text.mid(0, 2).toInt(nullptr, 16);
            g = text.mid(2, 2).toInt(nullptr, 16);
            b = text.mid(4, 2).toInt(nullptr, 16);
            a = text.length() == 8 ? text.mid(6, 2).toInt(nullptr, 16) : 0xFF;
        }
        color.setRgb(r, g, b, a);
        return color;
    }

    /* Helper to convert ECI combo index to ECI value */
    static int ECIIndexToECI(const int ECIIndex) {
        int ret;
        if (ECIIndex >= 1 && ECIIndex <= 11) {
            ret = ECIIndex + 2;
        } else if (ECIIndex >= 12 && ECIIndex <= 15) {
            ret = ECIIndex + 3;
        } else if (ECIIndex >= 16 && ECIIndex <= 31) {
            ret = ECIIndex + 4;
        } else if (ECIIndex == 32) {
            ret = 170; /* ISO 646 Invariant */
        } else if (ECIIndex == 33) {
            ret = 899; /* 8-bit binary data */
        } else {
            ret = 0;
        }
        return ret;
    }

    /* Helper to calculate max right and bottom of elements for fudging `render()` */
    static void getMaxRectsRightBottom(struct zint_vector *vector, int &maxRight, int &maxBottom) {
        struct zint_vector_rect *rect;
        struct zint_vector_hexagon *hex;
        struct zint_vector_circle *circle;

        maxRight = maxBottom = -1;

        for (rect = vector->rectangles; rect; rect = rect->next) {
            if (rect->x + rect->width > maxRight) {
                maxRight = rect->x + rect->width;
            }
            if (rect->y + rect->height > maxBottom) {
                maxBottom = rect->y + rect->height;
            }
        }

        for (hex = vector->hexagons; hex; hex = hex->next) {
            if (hex->x + hex->diameter > maxRight) {
                maxRight = hex->x + hex->diameter;
            }
            if (hex->y + hex->diameter > maxBottom) {
                maxBottom = hex->y + hex->diameter;
            }
        }

        for (circle = vector->circles; circle; circle = circle->next) {
            if (circle->x + circle->diameter + circle->width > maxRight) {
                maxRight = circle->x + circle->diameter + circle->width;
            }
            if (circle->y + circle->diameter + circle->width > maxBottom) {
                maxBottom = circle->y + circle->diameter + circle->width;
            }
        }

        // TODO: Strings?
    }

    /* Segment constructors */
    QZintSeg::QZintSeg() : m_eci(0) {}
    QZintSeg::QZintSeg(const QString& text, const int ECIIndex) : m_text(text), m_eci(ECIIndexToECI(ECIIndex)) {}

    QZint::QZint()
        : m_zintSymbol(nullptr), m_symbol(BARCODE_CODE128), m_input_mode(UNICODE_MODE),
            m_height(0.0f),
            m_option_1(-1), m_option_2(0), m_option_3(0),
            m_dpmm(0.0f),
            m_scale(1.0f),
            m_dotty(false), m_dot_size(4.0f / 5.0f),
            m_guardDescent(5.0f),
            m_textGap(1.0f),
            m_fgStr(QSL("000000")), m_bgStr(QSL("FFFFFF")), m_cmyk(false),
            m_borderType(0), m_borderWidth(0),
            m_whitespace(0), m_vwhitespace(0),
            m_fontSetting(0),
            m_show_hrt(true),
            m_gssep(false),
            m_quiet_zones(false), m_no_quiet_zones(false),
            m_compliant_height(false),
            m_rotate_angle(0),
            m_eci(0),
            m_gs1parens(false), m_gs1nocheck(false),
            m_reader_init(false),
            m_guard_whitespace(false),
            m_embed_vector_font(false),
            m_warn_level(WARN_DEFAULT), m_debug(false),
            m_encodedWidth(0), m_encodedRows(0), m_encodedHeight(0.0f),
            m_vectorWidth(0.0f), m_vectorHeight(0.0f),
            m_error(0),
            target_size_horiz(0), target_size_vert(0) // Legacy
    {
        memset(&m_structapp, 0, sizeof(m_structapp));
    }

    QZint::~QZint() {
        if (m_zintSymbol)
            ZBarcode_Delete(m_zintSymbol);
    }

    bool QZint::resetSymbol() {
        m_error = 0;
        m_lastError.clear();

        if (m_zintSymbol) {
            ZBarcode_Reset(m_zintSymbol);
        } else if (!(m_zintSymbol = ZBarcode_Create())) {
            m_error = ZINT_ERROR_MEMORY;
            m_lastError = QSL("Insufficient memory for Zint structure");
            return false;
        }

        m_zintSymbol->symbology = m_symbol;
        m_zintSymbol->height = m_height;
        m_zintSymbol->scale = m_scale;
        m_zintSymbol->whitespace_width = m_whitespace;
        m_zintSymbol->whitespace_height = m_vwhitespace;
        m_zintSymbol->border_width = m_borderWidth;
        m_zintSymbol->output_options = m_borderType | m_fontSetting;
        if (m_dotty) {
            m_zintSymbol->output_options |= BARCODE_DOTTY_MODE;
        }
        if (m_cmyk) {
            m_zintSymbol->output_options |= CMYK_COLOUR;
        }
        if (m_gssep) {
            m_zintSymbol->output_options |= GS1_GS_SEPARATOR;
        }
        if (m_quiet_zones) {
            m_zintSymbol->output_options |= BARCODE_QUIET_ZONES;
        }
        if (m_no_quiet_zones) {
            m_zintSymbol->output_options |= BARCODE_NO_QUIET_ZONES;
        }
        if (m_compliant_height) {
            m_zintSymbol->output_options |= COMPLIANT_HEIGHT;
        }
        if (m_reader_init) {
            m_zintSymbol->output_options |= READER_INIT;
        }
        if (m_guard_whitespace) {
            m_zintSymbol->output_options |= EANUPC_GUARD_WHITESPACE;
        }
        if (m_embed_vector_font) {
            m_zintSymbol->output_options |= EMBED_VECTOR_FONT;
        }
        strcpy(m_zintSymbol->fgcolour, m_fgStr.toLatin1().left(15));
        strcpy(m_zintSymbol->bgcolour, m_bgStr.toLatin1().left(15));
        strcpy(m_zintSymbol->primary, m_primaryMessage.toLatin1().left(127));
        m_zintSymbol->option_1 = m_option_1;
        m_zintSymbol->option_2 = m_option_2;
        m_zintSymbol->option_3 = m_option_3;
        m_zintSymbol->show_hrt = m_show_hrt ? 1 : 0;
        m_zintSymbol->input_mode = m_input_mode;
        if (m_gs1parens) {
            m_zintSymbol->input_mode |= GS1PARENS_MODE;
        }
        if (m_gs1nocheck) {
            m_zintSymbol->input_mode |= GS1NOCHECK_MODE;
        }
        m_zintSymbol->eci = m_eci;
        m_zintSymbol->dpmm = m_dpmm;
        m_zintSymbol->dot_size = m_dot_size;
        m_zintSymbol->guard_descent = m_guardDescent;
        m_zintSymbol->text_gap = m_textGap;
        m_zintSymbol->structapp = m_structapp;
        m_zintSymbol->warn_level = m_warn_level;
        m_zintSymbol->debug = m_debug ? ZINT_DEBUG_PRINT : 0;

        return true;
    }

    void QZint::encode() {
        if (resetSymbol()) {
            if (m_segs.empty()) {
                QByteArray bstr = m_text.toUtf8();
                /* Note do our own rotation */
                m_error = ZBarcode_Encode_and_Buffer_Vector(m_zintSymbol, (unsigned char *) bstr.data(),
                            bstr.length(), 0);
            } else {
                struct zint_seg segs[maxSegs];
                std::vector<QByteArray> bstrs;
                int seg_count = convertSegs(segs, bstrs);
                /* Note do our own rotation */
                m_error = ZBarcode_Encode_Segs_and_Buffer_Vector(m_zintSymbol, segs, seg_count, 0);
            }
            m_lastError = m_zintSymbol->errtxt;
        }

        if (m_error < ZINT_ERROR) {
            m_borderType = m_zintSymbol->output_options & (BARCODE_BIND | BARCODE_BOX | BARCODE_BIND_TOP);
            m_height = m_zintSymbol->height;
            m_borderWidth = m_zintSymbol->border_width;
            m_whitespace = m_zintSymbol->whitespace_width;
            m_vwhitespace = m_zintSymbol->whitespace_height;
            m_encodedWidth = m_zintSymbol->width;
            m_encodedRows = m_zintSymbol->rows;
            m_encodedHeight = m_zintSymbol->height;
            m_vectorWidth = m_zintSymbol->vector->width;
            m_vectorHeight = m_zintSymbol->vector->height;
            emit encoded();
        } else {
            m_encodedWidth = m_encodedRows = 0;
            m_encodedHeight = m_vectorWidth = m_vectorHeight = 0.0f;
            emit errored();
        }
    }

    /* Symbology to use (see BARCODE_XXX) */
    int QZint::symbol() const {
        return m_symbol;
    }

    void QZint::setSymbol(int symbol) {
        m_symbol = symbol;
    }

    /* Input data encoding. Default UNICODE_MODE */
    int QZint::inputMode() const {
        return m_input_mode;
    }

    void QZint::setInputMode(int input_mode) {
        m_input_mode = input_mode;
    }

    /* Input data (segment 0 text) */
    QString QZint::text() const {
        return m_text;
    }

    /* Set input data. Note: clears segs */
    void QZint::setText(const QString& text) {
        m_text = text;
        m_segs.clear();
    }

    /* Input segments. */
    std::vector<QZintSeg> QZint::segs() const {
        return m_segs;
    }

    /* Set segments. Note: clears text and sets eci */
    void QZint::setSegs(const std::vector<QZintSeg>& segs) {
        m_segs = segs;
        m_text.clear();
        if (m_segs.size()) { /* Make sure `symbol->eci` synced */
            m_eci = m_segs[0].m_eci;
        }
    }

    /* Primary message (Maxicode, Composite) */
    QString QZint::primaryMessage() const {
        return m_primaryMessage;
    }

    void QZint::setPrimaryMessage(const QString& primaryMessage) {
        m_primaryMessage = primaryMessage;
    }

    /* Symbol height in X-dimensions */
    float QZint::height() const {
        return m_height;
    }

    void QZint::setHeight(float height) {
        m_height = height;
    }

    /* Symbol-specific options (see "../docs/manual.txt") */
    int QZint::option1() const {
        return m_option_1;
    }

    void QZint::setOption1(int option_1) {
        m_option_1 = option_1;
    }

    /* Symbol-specific options */
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

    /* Scale factor when printing barcode, i.e. adjusts X-dimension */
    float QZint::scale() const {
        return m_scale;
    }

    void QZint::setScale(float scale) {
        m_scale = scale;
    }

    /* Resolution of output in dots per mm (BMP/EMF/PCX/PNG/TIF only) */
    float QZint::dpmm() const {
        return m_dpmm;
    }

    void QZint::setDPMM(float dpmm) {
        m_dpmm = dpmm;
    }

    /* Dotty mode */
    bool QZint::dotty() const {
        return m_dotty;
    }

    void QZint::setDotty(bool dotty) {
        m_dotty = dotty;
    }

    /* Size of dots used in BARCODE_DOTTY_MODE */
    float QZint::dotSize() const {
        return m_dot_size;
    }

    void QZint::setDotSize(float dotSize) {
        m_dot_size = dotSize;
    }

    /* Height in X-dimensions that EAN/UPC guard bars descend */
    float QZint::guardDescent() const {
        return m_guardDescent;
    }

    void QZint::setGuardDescent(float guardDescent) {
        m_guardDescent = guardDescent;
    }

    /* Structured Append info */
    int QZint::structAppCount() const {
        return m_structapp.count;
    }

    int QZint::structAppIndex() const {
        return m_structapp.index;
    }

    QString QZint::structAppID() const {
        return m_structapp.id;
    }

    void QZint::setStructApp(const int count, const int index, const QString& id) {
        if (count) {
            m_structapp.count = count;
            m_structapp.index = index;
            memset(m_structapp.id, 0, sizeof(m_structapp.id));
            if (!id.isEmpty()) {
                QByteArray idArr = id.toLatin1();
#if defined(__GNUC__) && __GNUC__ >= 8 && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
                strncpy(m_structapp.id, idArr, sizeof(m_structapp.id));
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
            }
        } else {
            clearStructApp();
        }
    }

    void QZint::clearStructApp() {
        memset(&m_structapp, 0, sizeof(m_structapp));
    }

    /* Foreground colour (may be RGB(A) hex string or CMYK decimal "C,M,Y,K" percentage string) */
    QString QZint::fgStr() const {
        return m_fgStr;
    }

    bool QZint::setFgStr(const QString& fgStr) {
        if (fgStr.indexOf(*colorRE) == 0) {
            m_fgStr = fgStr;
            return true;
        }
        return false;
    }

    /* Foreground colour as QColor */
    QColor QZint::fgColor() const {
        return str_to_qcolor(m_fgStr);
    }

    void QZint::setFgColor(const QColor& fgColor) {
        m_fgStr = qcolor_to_str(fgColor);
    }

    /* Background colour (may be RGB(A) hex string or CMYK decimal "C,M,Y,K" percentage string) */
    QString QZint::bgStr() const {
        return m_bgStr;
    }

    bool QZint::setBgStr(const QString& bgStr) {
        if (bgStr.indexOf(*colorRE) == 0) {
            m_bgStr = bgStr;
            return true;
        }
        return false;
    }

    /* Background colour as QColor */
    QColor QZint::bgColor() const {
        return str_to_qcolor(m_bgStr);
    }

    void QZint::setBgColor(const QColor& bgColor) {
        m_bgStr = qcolor_to_str(bgColor);
    }

    /* Use CMYK colour space (Encapsulated PostScript and TIF) */
    bool QZint::cmyk() const {
        return m_cmyk;
    }

    void QZint::setCMYK(bool cmyk) {
        m_cmyk = cmyk;
    }

    /* Type of border above/below/around barcode */
    int QZint::borderType() const {
        return m_borderType;
    }

    void QZint::setBorderType(int borderTypeIndex) {
        if (borderTypeIndex == 1) {
            m_borderType = BARCODE_BIND;
        } else if (borderTypeIndex == 2) {
            m_borderType = BARCODE_BOX;
        } else if (borderTypeIndex == 3) {
            m_borderType = BARCODE_BIND_TOP;
        } else {
            m_borderType = 0;
        }
    }

    /* Size of border in X-dimensions */
    int QZint::borderWidth() const {
        return m_borderWidth;
    }

    void QZint::setBorderWidth(int borderWidth) {
        if (borderWidth < 0 || borderWidth > 16)
            borderWidth = 0;
        m_borderWidth = borderWidth;
    }

    /* Width in X-dimensions of whitespace to left & right of barcode */
    int QZint::whitespace() const {
        return m_whitespace;
    }

    void QZint::setWhitespace(int whitespace) {
        m_whitespace = whitespace;
    }

    /* Height in X-dimensions of whitespace above & below the barcode */
    int QZint::vWhitespace() const {
        return m_vwhitespace;
    }

    void QZint::setVWhitespace(int vWhitespace) {
        m_vwhitespace = vWhitespace;
    }

    /* Type of font to use i.e. normal, small, bold or (vector only) small bold */
    int QZint::fontSetting() const {
        return m_fontSetting;
    }

    void QZint::setFontSetting(int fontSettingIndex) { // Sets from comboBox index
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

    void QZint::setFontSettingValue(int fontSetting) { // Sets literal value
        if ((fontSetting & (BOLD_TEXT | SMALL_TEXT)) == fontSetting) {
            m_fontSetting = fontSetting;
        } else {
            m_fontSetting = 0;
        }
    }

    /* Text gap */
    float QZint::textGap() const {
        return m_textGap;
    }

    void QZint::setTextGap(float textGap) {
        m_textGap = textGap;
    }

    /* Show (true) or hide (false) Human Readable Text (HRT) */
    bool QZint::showText() const {
        return m_show_hrt;
    }

    void QZint::setShowText(bool showText) {
        m_show_hrt = showText;
    }

    /* Set to true to use GS (Group Separator) instead of FNC1 as GS1 separator (Data Matrix) */
    bool QZint::gsSep() const {
        return m_gssep;
    }

    void QZint::setGSSep(bool gsSep) {
        m_gssep = gsSep;
    }

    /* Add compliant quiet zones (additional to any specified whitespace)
       Note: CODE16K, CODE49, CODABLOCKF, ITF14, EAN/UPC have default quiet zones */
    bool QZint::quietZones() const {
        return m_quiet_zones;
    }

    void QZint::setQuietZones(bool quietZones) {
        m_quiet_zones = quietZones;
    }

    /* Disable quiet zones, notably those with defaults as listed above */
    bool QZint::noQuietZones() const {
        return m_no_quiet_zones;
    }

    void QZint::setNoQuietZones(bool noQuietZones) {
        m_no_quiet_zones = noQuietZones;
    }

    /* Warn if height not compliant and use standard height (if any) as default */
    bool QZint::compliantHeight() const {
        return m_compliant_height;
    }

    void QZint::setCompliantHeight(bool compliantHeight) {
        m_compliant_height = compliantHeight;
    }

    /* Rotate barcode by angle (degrees 0, 90, 180 and 270) */
    int QZint::rotateAngle() const {
        return m_rotate_angle;
    }

    void QZint::setRotateAngle(int rotateIndex) { // Sets from comboBox index
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

    void QZint::setRotateAngleValue(int rotateAngle) { // Sets literal value
        if (rotateAngle == 90) {
            m_rotate_angle = 90;
        } else if (rotateAngle == 180) {
            m_rotate_angle = 180;
        } else if (rotateAngle == 270) {
            m_rotate_angle = 270;
        } else {
            m_rotate_angle = 0;
        }
    }

    /* Extended Channel Interpretation (segment 0 eci) */
    int QZint::eci() const {
        return m_eci;
    }

    void QZint::setECI(int ECIIndex) { // Sets from comboBox index
        m_eci = ECIIndexToECI(ECIIndex);
    }

    void QZint::setECIValue(int eci) { // Sets literal value
        if (eci < 3 || (eci > 35 && eci != 170 && eci != 899) || eci == 14 || eci == 19) {
            m_eci = 0;
        } else {
            m_eci = eci;
        }
    }

    /* Process parentheses as GS1 AI delimiters (instead of square brackets) */
    bool QZint::gs1Parens() const {
        return m_gs1parens;
    }

    void QZint::setGS1Parens(bool gs1Parens) {
        m_gs1parens = gs1Parens;
    }

    /* Do not check validity of GS1 data (except that printable ASCII only) */
    bool QZint::gs1NoCheck() const {
        return m_gs1nocheck;
    }

    void QZint::setGS1NoCheck(bool gs1NoCheck) {
        m_gs1nocheck = gs1NoCheck;
    }

    /* Reader Initialisation (Programming) */
    bool QZint::readerInit() const {
        return m_reader_init;
    }

    void QZint::setReaderInit(bool readerInit) {
        m_reader_init = readerInit;
    }

    /* Whether to add quiet zone indicators ("<", ">") to HRT (EAN/UPC) */
    bool QZint::guardWhitespace() const {
        return m_guard_whitespace;
    }

    void QZint::setGuardWhitespace(bool guardWhitespace) {
        m_guard_whitespace = guardWhitespace;
    }

    /* Whether to embed the font in vector output - currently only for SVG output of EAN/UPC */
    bool QZint::embedVectorFont() const {
        return m_embed_vector_font;
    }

    void QZint::setEmbedVectorFont(bool embedVectorFont) {
        m_embed_vector_font = embedVectorFont;
    }

    /* Affects error/warning value returned by Zint API (see `getError()` below) */
    int QZint::warnLevel() const {
        return m_warn_level;
    }

    void QZint::setWarnLevel(int warnLevel) {
        m_warn_level = warnLevel;
    }

    /* Debugging flags */
    bool QZint::debug() const {
        return m_debug;
    }

    void QZint::setDebug(bool debug) {
        m_debug = debug;
    }

    /* Symbol output info set by Zint on successful `render()` */
    int QZint::encodedWidth() const { // Read-only, encoded width (no. of modules encoded)
        return m_encodedWidth;
    }

    int QZint::encodedRows() const { // Read-only, no. of rows encoded
        return m_encodedRows;
    }

    float QZint::encodedHeight() const { // Read-only, in X-dimensions
        if (m_symbol == BARCODE_MAXICODE) { // Maxicode encoded height is meaningless, so return fixed value
            return 33 * 0.866f; // √3 / 2
        }
        return m_encodedHeight;
    }

    float QZint::vectorWidth() const { // Read-only, scaled width
        return m_vectorWidth;
    }

    float QZint::vectorHeight() const { // Read-only, scaled height
        return m_vectorHeight;
    }

    /* Legacy property getters/setters */
    void QZint::setWidth(int width) { setOption2(width); }
    int QZint::width() const { return m_option_2; }
    void QZint::setSecurityLevel(int securityLevel) { setOption1(securityLevel); }
    int QZint::securityLevel() const { return m_option_1; }
    void QZint::setPdf417CodeWords(int /*pdf417CodeWords*/) {}
    int QZint::pdf417CodeWords() const { return 0; }
    void QZint::setHideText(bool hide) { setShowText(!hide); }
    void QZint::setTargetSize(int width, int height) {
        target_size_horiz = width;
        target_size_vert = height;
    }
    QString QZint::error_message() const { return m_lastError; } /* Same as lastError() */

    /* Test capabilities - `ZBarcode_Cap()` */
    bool QZint::hasHRT(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_HRT);
    }

    bool QZint::isStackable(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_STACKABLE);
    }

    bool QZint::isEANUPC(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_EANUPC);
    }

    bool QZint::isExtendable(int symbology) const { /* Legacy - same as `isEANUPC()` */
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_EANUPC);
    }

    bool QZint::isComposite(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_COMPOSITE);
    }

    bool QZint::supportsECI(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_ECI);
    }

    bool QZint::supportsGS1(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_GS1);
    }

    bool QZint::isDotty(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_DOTTY);
    }

    bool QZint::hasDefaultQuietZones(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_QUIET_ZONES);
    }

    bool QZint::isFixedRatio(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_FIXED_RATIO);
    }

    bool QZint::supportsReaderInit(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_READER_INIT);
    }

    bool QZint::supportsFullMultibyte(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_FULL_MULTIBYTE);
    }

    bool QZint::hasMask(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_MASK);
    }

    bool QZint::supportsStructApp(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_STRUCTAPP);
    }

    bool QZint::hasCompliantHeight(int symbology) const {
        return ZBarcode_Cap(symbology ? symbology : m_symbol, ZINT_CAP_COMPLIANT_HEIGHT);
    }

    /* Whether takes GS1 AI-delimited data */
    bool QZint::takesGS1AIData(int symbology) const {
        if (symbology == 0) {
            symbology = m_symbol;
        }
        switch (symbology) {
            case BARCODE_GS1_128:
            case BARCODE_DBAR_EXP:
            case BARCODE_DBAR_EXPSTK:
                return true;
                break;
            default:
                return symbology >= BARCODE_EANX_CC && symbology <= BARCODE_DBAR_EXPSTK_CC;
                break;
        }
    }

    /* Error or warning returned by Zint on `render()` or `save_to_file()` */
    int QZint::getError() const {
        return m_error;
    }

    /* Error message returned by Zint on `render()` or `save_to_file()` */
    const QString& QZint::lastError() const {
        return m_lastError;
    }

    /* Whether `lastError()` set */
    bool QZint::hasErrors() const {
        return m_lastError.length();
    }

    bool QZint::save_to_file(const QString& filename) {
        if (resetSymbol()) {
            strcpy(m_zintSymbol->outfile, filename.toUtf8().left(255));
            if (m_segs.empty()) {
                QByteArray bstr = m_text.toUtf8();
                m_error = ZBarcode_Encode_and_Print(m_zintSymbol, (unsigned char *) bstr.data(), bstr.length(),
                                                    m_rotate_angle);
            } else {
                struct zint_seg segs[maxSegs];
                std::vector<QByteArray> bstrs;
                int seg_count = convertSegs(segs, bstrs);
                m_error = ZBarcode_Encode_Segs_and_Print(m_zintSymbol, segs, seg_count, m_rotate_angle);
            }
        }
        if (m_error >= ZINT_ERROR) {
            m_lastError = m_zintSymbol->errtxt;
            m_encodedWidth = m_encodedRows = 0;
            m_encodedHeight = m_vectorWidth = m_vectorHeight = 0.0f;
            emit errored();
            return false;
        }
        return true;
    }

    /* Convert `zint_vector_rect->colour` to Qt color */
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

    /* Helper to convert `m_segs` to `struct zint_seg[]` */
    int QZint::convertSegs(struct zint_seg segs[], std::vector<QByteArray>& bstrs) {
        bstrs.reserve(m_segs.size());
        int i;
        for (i = 0; i < (int) m_segs.size() && i < maxSegs && !m_segs[i].m_text.isEmpty(); i++) {
            segs[i].eci = m_segs[i].m_eci;
            bstrs.push_back(m_segs[i].m_text.toUtf8());
            segs[i].source = (unsigned char *) bstrs.back().data();
            segs[i].length = bstrs.back().length();
        }
        return i;
    }

    /* Encode and display barcode in `paintRect` using `painter`.
       Note: legacy argument `mode` is not used */
    void QZint::render(QPainter& painter, const QRectF& paintRect, AspectRatioMode /*mode*/) {
        struct zint_vector_rect *rect;
        struct zint_vector_hexagon *hex;
        struct zint_vector_circle *circle;
        struct zint_vector_string *string;
        QColor fgColor = str_to_qcolor(m_fgStr);
        QColor bgColor = str_to_qcolor(m_bgStr);
        encode();

        painter.save();

        if (m_error >= ZINT_ERROR) {
            painter.setRenderHint(QPainter::Antialiasing);
            QFont font(fontFamilyError, fontSizeError);
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

        QBrush bgBrush(bgColor);
        if (bgColor.alpha() != 0) {
            painter.fillRect(QRectF(0, 0, gwidth, gheight), bgBrush);
        }

        // Plot rectangles
        rect = m_zintSymbol->vector->rectangles;
        if (rect) {
            int maxRight = -1, maxBottom = -1; // Used for fudging below
            getMaxRectsRightBottom(m_zintSymbol->vector, maxRight, maxBottom);
            QBrush brush(Qt::SolidPattern);
            while (rect) {
                if (rect->colour == -1) {
                    brush.setColor(fgColor);
                } else {
                    brush.setColor(colourToQtColor(rect->colour));
                }
                // Allow for rounding errors on translation/scaling TODO: proper calc
                float fudgeW = rect->x + rect->width == maxRight ? 0.1f : 0.0f;
                float fudgeH = rect->y + rect->height == maxBottom ? 0.1f : 0.0f;
                painter.fillRect(QRectF(rect->x, rect->y, rect->width + fudgeW, rect->height + fudgeH), brush);
                rect = rect->next;
            }
        }

        // Plot hexagons
        hex = m_zintSymbol->vector->hexagons;
        if (hex) {
            painter.setRenderHint(QPainter::Antialiasing);
            QBrush fgBrush(fgColor);
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
            QBrush fgBrush(fgColor);
            qreal previous_diameter = 0.0, radius = 0.0;
            while (circle) {
                if (previous_diameter != circle->diameter) {
                    previous_diameter = circle->diameter;
                    radius = 0.5 * previous_diameter;
                }
                if (circle->colour) { // Set means use background colour (legacy, no longer used)
                    p.setColor(bgColor);
                    p.setWidthF(circle->width);
                    painter.setPen(p);
                    painter.setBrush(circle->width ? Qt::NoBrush : bgBrush);
                } else {
                    p.setColor(fgColor);
                    p.setWidthF(circle->width);
                    painter.setPen(p);
                    painter.setBrush(circle->width ? Qt::NoBrush : fgBrush);
                }
                painter.drawEllipse(QPointF(circle->x, circle->y), radius, radius);
                circle = circle->next;
            }
        }

        // Plot text
        string = m_zintSymbol->vector->strings;
        if (string) {
            if (normalFontID == -2) { /* First time? */
                loadNormalFont();
            }
            if (upceanFontID == -2) { /* First time? */
                loadUpceanFont();
            }
            painter.setRenderHint(QPainter::Antialiasing);
            QPen p;
            p.setColor(fgColor);
            painter.setPen(p);
            bool bold = (m_zintSymbol->output_options & BOLD_TEXT) && !isEANUPC();
            QFont font(isEANUPC() ? upceanFontFamily : normalFontFamily, -1 /*pointSize*/,
                        bold ? QFont::Bold : -1);
            while (string) {
                font.setPixelSize(string->fsize);
                painter.setFont(font);
                QString content = QString::fromUtf8((const char *) string->text);
                /* string->y is baseline of font */
                if (string->halign == 1) { /* Left align */
                    painter.drawText(QPointF(string->x, string->y), content);
                } else {
                    QFontMetrics fm(painter.fontMetrics());
                    int width = fm.horizontalAdvance(content);
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

    /* Returns the default X-dimension (`ZBarcode_Default_Xdim()`).
       If `symbology` non-zero then used instead of `symbol()` */
    float QZint::defaultXdim(int symbology) const {
        return ZBarcode_Default_Xdim(symbology ? symbology : m_symbol);
    }

    /* Returns the scale to use for X-dimension `x_dim_mm` at `dpmm` for `filetype`.
       If `symbology` non-zero then used instead of `symbol()` */
    float QZint::getScaleFromXdimDp(float x_dim_mm, float dpmm, const QString& fileType, int symbology) const {
        return ZBarcode_Scale_From_XdimDp(symbology ? symbology : m_symbol, x_dim_mm, dpmm, fileType.toLatin1());
    }

    /* Reverse of `getScaleFromXdimDp()` above, returning the X-dimension or dot density given the scale `scale`.
       If `symbology` non-zero then used instead of `symbol()` */
    float QZint::getXdimDpFromScale(float scale, float x_dim_mm_or_dpmm, const QString& fileType,
                    int symbology) const {
        return ZBarcode_XdimDp_From_Scale(symbology ? symbology : m_symbol, scale, x_dim_mm_or_dpmm,
                                            fileType.toLatin1());
    }

    /* Set `width_x_dim` and `height_x_dim` with estimated size of barcode based on X-dimension `x_dim`. To be called
       after a successful `render()`. Returns false if `scale()` zero or render is in error, otherwise true */
    bool QZint::getWidthHeightXdim(float x_dim, float &width_x_dim, float &height_x_dim) const {

        if (m_scale == 0.0f || m_vectorWidth == 0.0f || m_vectorHeight == 0.0f) {
            width_x_dim = height_x_dim = 0.0f;
            return false;
        }

        const float scale = m_scale * 2.0f;
        const float width = m_vectorWidth / scale;
        const float height = m_vectorHeight / scale;

        if (rotateAngle() == 90 || rotateAngle() == 270) { // Sideways - swop
            width_x_dim = (height * x_dim);
            height_x_dim = (width * x_dim);
        } else {
            width_x_dim = (width * x_dim);
            height_x_dim = (height * x_dim);
        }

        return true;
    }

    /* Return the BARCODE_XXX name of `symbology` */
    QString QZint::barcodeName(const int symbology) {
        char buf[32];
        if (ZBarcode_BarcodeName(symbology, buf) == 0) {
            return QString(buf);
        }
        return QSEmpty;
    }

    /* Whether Zint library "libzint" built with PNG support or not */
    bool QZint::noPng() {
        return ZBarcode_NoPng() == 1;
    }

    /* Version of Zint library "libzint" linked to */
    int QZint::getVersion() {
        return ZBarcode_Version();
    }

    /* Translate settings into Command Line equivalent. Set `win` to use Windows escaping of data.
       If `autoHeight` set then `--height=` option will not be emitted.
       If HEIGHTPERROW_MODE set and non-zero `heightPerRow` given then use that for height instead of internal
       height */
    QString QZint::getAsCLI(const bool win, const bool longOptOnly, const bool barcodeNames, const bool noEXE,
                    const bool autoHeight, const float heightPerRow, const QString& outfile,
                    const QZintXdimDpVars *xdimdpVars) const {
        QString cmd(win && !noEXE ? QSL("zint.exe") : QSL("zint"));
        const bool nobackground = bgColor().alpha() == 0;
        const bool notext = hasHRT() && !showText();

        char name_buf[32];
        if (barcodeNames && ZBarcode_BarcodeName(m_symbol, name_buf) == 0) {
            QString name(name_buf + 8); // Strip "BARCODE_" prefix
            arg_str(cmd, longOptOnly ? "--barcode=" : "-b ", name);
        } else {
            arg_int(cmd, longOptOnly ? "--barcode=" : "-b ", m_symbol);
        }

        if (isEANUPC()) {
            arg_int(cmd, "--addongap=", option2());
        }

        if (bgStr() != QSL("FFFFFF") && !nobackground) {
            arg_str(cmd, "--bg=", bgStr());
        }

        bool default_bind = false, default_bind_top = false, default_box = false, default_border = false;
        if (m_symbol == BARCODE_ITF14) {
            if ((borderType() & BARCODE_BOX) && borderWidth() == 5) {
                default_box = default_border = true;
            }
        } else if (m_symbol == BARCODE_CODABLOCKF || m_symbol == BARCODE_HIBC_BLOCKF || m_symbol == BARCODE_CODE16K
                    || m_symbol == BARCODE_CODE49) {
            if ((borderType() & BARCODE_BIND) && borderWidth() == 1) {
                default_bind = default_border = true;
            }
        } else if (m_symbol == BARCODE_DPD) {
            if ((borderType() & BARCODE_BIND_TOP) && borderWidth() == 3) {
                default_bind_top = default_border = true;
            }
        }

        arg_bool(cmd, "--binary", (inputMode() & 0x07) == DATA_MODE);
        if (!default_bind) {
            arg_bool(cmd, "--bind", borderType() & BARCODE_BIND);
        }
        if (!default_bind_top) {
            arg_bool(cmd, "--bindtop", borderType() & BARCODE_BIND_TOP);
        }
        arg_bool(cmd, "--bold", !notext && (fontSetting() & BOLD_TEXT) && !isEANUPC());
        if (!default_border) {
            arg_int(cmd, "--border=", borderWidth());
        }
        if (!default_box) {
            arg_bool(cmd, "--box", borderType() & BARCODE_BOX);
        }
        arg_bool(cmd, "--cmyk", cmyk());

        if (m_symbol == BARCODE_DBAR_EXPSTK || m_symbol == BARCODE_DBAR_EXPSTK_CC
                || m_symbol == BARCODE_PDF417 || m_symbol == BARCODE_PDF417COMP || m_symbol == BARCODE_HIBC_PDF
                || m_symbol == BARCODE_MICROPDF417 || m_symbol == BARCODE_HIBC_MICPDF
                || m_symbol == BARCODE_DOTCODE || m_symbol == BARCODE_CODABLOCKF || m_symbol == BARCODE_HIBC_BLOCKF) {
            arg_int(cmd, "--cols=", option2());
        }

        arg_bool(cmd, "--compliantheight", hasCompliantHeight() && compliantHeight());

        if (m_segs.empty()) {
            if (supportsECI()) {
                arg_int(cmd, "--eci=", eci());
            }
            arg_data(cmd, longOptOnly ? "--data=" : "-d ", m_text, win);
        } else {
            arg_int(cmd, "--eci=", m_segs.front().m_eci);
            arg_data(cmd, longOptOnly ? "--data=" : "-d ", m_segs.front().m_text, win);
            for (int i = 1; i < (int) m_segs.size() && i < maxCLISegs && !m_segs[i].m_text.isEmpty(); i++) {
                arg_seg(cmd, i, m_segs[i], win);
            }
        }

        if (m_symbol == BARCODE_DATAMATRIX || m_symbol == BARCODE_HIBC_DM) {
            arg_bool(cmd, "--dmiso144", (option3() & DM_ISO_144) == DM_ISO_144);
            arg_bool(cmd, "--dmre", (option3() & 0x7F) == DM_DMRE);
        }

        if ((m_symbol == BARCODE_DOTCODE || (isDotty() && dotty())) && dotSize() != 0.8f) {
            arg_float(cmd, "--dotsize=", dotSize());
        }
        if (m_symbol != BARCODE_DOTCODE && isDotty() && dotty()) {
            arg_bool(cmd, "--dotty", dotty());
        }

        if (showText()) {
            arg_bool(cmd, "--embedfont", embedVectorFont());
        }
        arg_bool(cmd, "--esc", inputMode() & ESCAPE_MODE);
        arg_bool(cmd, "--extraesc", inputMode() & EXTRA_ESCAPE_MODE);
        arg_bool(cmd, "--fast", inputMode() & FAST_MODE);

        if (fgStr() != QSL("000000") && fgStr() != QSL("000000FF")) {
            arg_str(cmd, "--fg=", fgStr());
        }

        arg_bool(cmd, "--fullmultibyte", supportsFullMultibyte() && (option3() & 0xFF) == ZINT_FULL_MULTIBYTE);

        if (supportsGS1()) {
            arg_bool(cmd, "--gs1", (inputMode() & 0x07) == GS1_MODE);
            arg_bool(cmd, "--gs1parens", gs1Parens() || (inputMode() & GS1PARENS_MODE));
            arg_bool(cmd, "--gs1nocheck", gs1NoCheck() || (inputMode() & GS1NOCHECK_MODE));
            arg_bool(cmd, "--gssep", gsSep());
        }

        if (isEANUPC() && guardDescent() != 5.0f) {
            arg_float(cmd, "--guarddescent=", guardDescent(), true /*allowZero*/);
        }
        if (isEANUPC() && showText()) {
            arg_bool(cmd, "--guardwhitespace", guardWhitespace());
        }

        if (!autoHeight && !isFixedRatio()) {
            if (inputMode() & HEIGHTPERROW_MODE) {
                arg_float(cmd, "--height=", heightPerRow ? heightPerRow : height());
                arg_bool(cmd, "--heightperrow", true);
            } else {
                arg_float(cmd, "--height=", height());
            }
        }

        arg_bool(cmd, "--init", supportsReaderInit() && readerInit());

        if (hasMask()) {
            arg_int(cmd, "--mask=", (option3() >> 8) - 1, true /*allowZero*/);
        }

        if (m_symbol == BARCODE_MAXICODE || isComposite()) {
            arg_int(cmd, "--mode=", option1());
        }

        arg_bool(cmd, "--nobackground", nobackground);
        arg_bool(cmd, "--noquietzones", hasDefaultQuietZones() && noQuietZones());
        arg_bool(cmd, "--notext", notext);
        arg_data(cmd, longOptOnly ? "--output=" : "-o ", outfile, win);

        if (m_symbol == BARCODE_MAXICODE || isComposite()) {
            arg_data(cmd, "--primary=", primaryMessage(), win);
        }

        arg_bool(cmd, "--quietzones", !hasDefaultQuietZones() && quietZones());
        arg_int(cmd, "--rotate=", rotateAngle());

        if (m_symbol == BARCODE_CODE16K || m_symbol == BARCODE_CODABLOCKF || m_symbol == BARCODE_HIBC_BLOCKF
                || m_symbol == BARCODE_CODE49) {
            arg_int(cmd, "--rows=", option1());
        } else if (m_symbol == BARCODE_DBAR_EXPSTK || m_symbol == BARCODE_DBAR_EXPSTK_CC
                || m_symbol == BARCODE_PDF417 || m_symbol == BARCODE_PDF417COMP || m_symbol == BARCODE_HIBC_PDF) {
            arg_int(cmd, "--rows=", option3());
        }

        if (dpmm()) {
            arg_scalexdimdp(cmd, "--scalexdimdp", scale(), dpmm(), symbol(), xdimdpVars);
        } else if (scale() != 1.0f) {
            arg_float(cmd, "--scale=", scale());
        }

        if (m_symbol == BARCODE_MAXICODE) {
            arg_int(cmd, "--scmvv=", option2() - 1, true /*allowZero*/);
        }

        if (m_symbol == BARCODE_PDF417 || m_symbol == BARCODE_PDF417COMP || m_symbol == BARCODE_HIBC_PDF
                || m_symbol == BARCODE_AZTEC || m_symbol == BARCODE_HIBC_AZTEC
                || m_symbol == BARCODE_QRCODE || m_symbol == BARCODE_HIBC_QR || m_symbol == BARCODE_MICROQR
                || m_symbol == BARCODE_RMQR || m_symbol == BARCODE_GRIDMATRIX || m_symbol == BARCODE_HANXIN
                || m_symbol == BARCODE_ULTRA) {
            arg_int(cmd, "--secure=", option1());
        }

        if (m_symbol == BARCODE_CODE16K || m_symbol == BARCODE_CODABLOCKF || m_symbol == BARCODE_HIBC_BLOCKF
                || m_symbol == BARCODE_CODE49) {
            arg_int(cmd, "--separator=", option3());
        }

        arg_bool(cmd, "--small", !notext && (fontSetting() & SMALL_TEXT));

        if (m_symbol == BARCODE_DATAMATRIX || m_symbol == BARCODE_HIBC_DM) {
            arg_bool(cmd, "--square", (option3() & 0x7F) == DM_SQUARE);
        }

        if (supportsStructApp()) {
            arg_structapp(cmd, "--structapp=", structAppCount(), structAppIndex(), structAppID(), win);
        }

        if (!notext && textGap() != 1.0f) {
            arg_float(cmd, "--textgap=", textGap(), true /*allowZero*/);
        }

        arg_bool(cmd, "--verbose", debug());

        if (m_symbol == BARCODE_AZTEC || m_symbol == BARCODE_HIBC_AZTEC
                || m_symbol == BARCODE_MSI_PLESSEY || m_symbol == BARCODE_CODE11
                || m_symbol == BARCODE_C25STANDARD || m_symbol == BARCODE_C25INTER || m_symbol == BARCODE_C25IATA
                || m_symbol == BARCODE_C25LOGIC || m_symbol == BARCODE_C25IND
                || m_symbol == BARCODE_CODE39 || m_symbol == BARCODE_HIBC_39 || m_symbol == BARCODE_EXCODE39
                || m_symbol == BARCODE_LOGMARS || m_symbol == BARCODE_CODABAR
                || m_symbol == BARCODE_DATAMATRIX || m_symbol == BARCODE_HIBC_DM
                || m_symbol == BARCODE_QRCODE || m_symbol == BARCODE_HIBC_QR || m_symbol == BARCODE_MICROQR
                || m_symbol == BARCODE_RMQR || m_symbol == BARCODE_GRIDMATRIX || m_symbol == BARCODE_HANXIN
                || m_symbol == BARCODE_CHANNEL || m_symbol == BARCODE_CODEONE || m_symbol == BARCODE_CODE93
                || m_symbol == BARCODE_ULTRA || m_symbol == BARCODE_VIN) {
            arg_int(cmd, "--vers=", option2());
        } else if (m_symbol == BARCODE_DAFT && option2() != 250) {
            arg_int(cmd, "--vers=", option2());
        }

        arg_int(cmd, "--vwhitesp=", vWhitespace());
        arg_int(cmd, longOptOnly ? "--whitesp=" : "-w ", whitespace());
        arg_bool(cmd, "--werror", warnLevel() == WARN_FAIL_ALL);

        return cmd;
    }

    /* `getAsCLI()` helpers */
    void QZint::arg_str(QString& cmd, const char *const opt, const QString& val) {
        if (!val.isEmpty()) {
            QByteArray bstr = val.toUtf8();
            cmd += QString::asprintf(" %s%.*s", opt, (int) bstr.length(), bstr.data());
        }
    }

    void QZint::arg_int(QString& cmd, const char *const opt, const int val, const bool allowZero) {
        if (val > 0 || (val == 0 && allowZero)) {
            cmd += QString::asprintf(" %s%d", opt, val);
        }
    }

    void QZint::arg_bool(QString& cmd, const char *const opt, const bool val) {
        if (val) {
            cmd += QString::asprintf(" %s", opt);
        }
    }

    void QZint::arg_data(QString& cmd, const char *const opt, const QString& val, const bool win) {
        if (!val.isEmpty()) {
            QString text(val);
            arg_data_esc(cmd, opt, text, win);
        }
    }

    void QZint::arg_seg(QString& cmd, const int seg_no, const QZintSeg& val, const bool win) {
        QString text(val.m_text);
        QString opt = QString::asprintf("--seg%d=%d,", seg_no, val.m_eci);
        arg_data_esc(cmd, opt.toUtf8(), text, win);
    }

    void QZint::arg_data_esc(QString& cmd, const char *const opt, QString& text, const bool win) {
        const char delim = win ? '"' : '\'';
        if (win) {
            // Difficult (impossible?) to fully escape strings on Windows, e.g. "blah%PATH%" will substitute
            // env var PATH, so just doing basic escaping here
            text.replace("\\\\", "\\\\\\\\"); // Double-up backslashed backslash `\\` -> `\\\\`
            text.replace("\"", "\\\""); // Backslash quote `"` -> `\"`
            QByteArray bstr = text.toUtf8();
            cmd += QString::asprintf(" %s%c%.*s%c", opt, delim, (int) bstr.length(), bstr.data(), delim);
        } else {
            text.replace("'", "'\\''"); // Single quote `'` -> `'\''`
            QByteArray bstr = text.toUtf8();
            cmd += QString::asprintf(" %s%c%.*s%c", opt, delim, (int) bstr.length(), bstr.data(), delim);
        }
    }

    void QZint::arg_float(QString& cmd, const char *const opt, const float val, const bool allowZero) {
        if (val > 0 || (val == 0 && allowZero)) {
            cmd += QString::asprintf(" %s%g", opt, val);
        }
    }

    void QZint::arg_structapp(QString& cmd, const char *const opt, const int count, const int index,
                                const QString& id, const bool win) {
        if (count >= 2 && index >= 1) {
            if (id.isEmpty()) {
                cmd += QString::asprintf(" %s%d,%d", opt, index, count);
            } else {
                QByteArray bstr = id.toUtf8();
                arg_data(cmd, opt, QString::asprintf("%d,%d,%.*s", index, count, (int) bstr.length(), bstr.data()),
                                                        win);
            }
        }
    }

    void QZint::arg_scalexdimdp(QString& cmd, const char *const opt, const float scale, const float dpmm,
                                const int symbol, const QZintXdimDpVars *xdimdpVars) {
        if (dpmm) {
            float resolution = dpmm;
            float x_dim;
            const char *x_dim_units_str = "";
            const char *resolution_units_str = "";
            if (xdimdpVars && xdimdpVars->set) {
                x_dim = xdimdpVars->x_dim;
                resolution = xdimdpVars->resolution;
                if (xdimdpVars->x_dim_units || xdimdpVars->resolution_units) {
                    x_dim_units_str = xdimdpVars->x_dim_units ? "in" : "mm";
                    resolution_units_str = xdimdpVars->resolution_units ? "dpi" : "dpmm";
                }
            } else {
                x_dim = ZBarcode_XdimDp_From_Scale(symbol, scale, resolution, nullptr);
            }
            cmd += QString::asprintf(" %s=%g%s,%g%s",
                                        opt, x_dim, x_dim_units_str, resolution, resolution_units_str);
        }
    }

    /* Helper to return "GIF"/"SVG"(/"EMF") if `msg` false, "raster"/"vector"(/"EMF") otherwise
       (EMF only if `symbol` is MaxiCode) */
    const char *QZintXdimDpVars::getFileType(int symbol, const struct QZintXdimDpVars *vars, bool msg) {
        static const char *filetypes[3] = { "GIF", "SVG", "EMF" };
        static const char *msg_types[3] = { "raster", "vector", "EMF" };

        if (!vars) return "";

        const int idx = std::max(std::min(symbol == BARCODE_MAXICODE ? vars->filetype_maxicode
                                                                                : vars->filetype, 2), 0);
        return msg ? msg_types[idx] : filetypes[idx];
    }
} /* namespace Zint */

/* vim: set ts=4 sw=4 et : */
