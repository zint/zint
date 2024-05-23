/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bogdan@licentia.eu                                                    *
 *   Copyright (C) 2010-2023 Robin Stuart                                  *
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

/*
 * For version, see "../backend/zintconfig.h"
 * For documentation, see "../docs/manual.txt"
 */

#ifndef QZINT_H
#define QZINT_H

#include <QObject>
#include <QColor>
#include <QPainter>
#include "zint.h"

namespace Zint
{

/* QString version of `struct zint_seg` */
class QZintSeg {
public:
    QString m_text; // `seg->source` and `seg->length`
    int m_eci; // `seg->eci`

    QZintSeg();
    QZintSeg(const QString& text, const int ECIIndex = 0); // `ECIIndex` is comboBox index (not ECI value)
};

struct QZintXdimDpVars; // Forward reference to Printing Scale settings, see end

/* Interface */
class QZint : public QObject
{
    Q_OBJECT

public:
     /* Legacy - not used */
     enum AspectRatioMode{ IgnoreAspectRatio = 0, KeepAspectRatio = 1, CenterBarCode = 2 };

public:
    QZint();
    ~QZint();

    /* Symbology to use (see BARCODE_XXX) */
    int symbol() const; // `symbol->symbology`
    void setSymbol(int symbol);

    /* Input data encoding. Default UNICODE_MODE */
    int inputMode() const; // `symbol->input_mode`
    void setInputMode(int input_mode);

    /* Note text/eci and segs are mutally exclusive */

    /* Input data (segment 0 text) */
    QString text() const;
    /* Set input data. Note: clears segs */
    void setText(const QString& text);

    /* Input segments. */
    std::vector<QZintSeg> segs() const;
    /* Set segments. Note: clears text and sets eci */
    void setSegs(const std::vector<QZintSeg>& segs);

    /* Primary message (Maxicode, Composite) */
    QString primaryMessage() const; // `symbol->primary`
    void setPrimaryMessage(const QString& primaryMessage);

    /* Symbol height in X-dimensions */
    float height() const; // `symbol->height`
    void setHeight(float height);

    /* Symbol-specific options (see "../docs/manual.txt") */
    int option1() const; // `symbol->option_1`
    void setOption1(int option_1);

    /* Symbol-specific options */
    int option2() const; // `symbol->option_2`
    void setOption2(int option);

    /* Symbol-specific options */
    int option3() const; // `symbol->option_3`
    void setOption3(int option);

    /* Scale factor when printing barcode, i.e. adjusts X-dimension */
    float scale() const; // `symbol->scale`
    void setScale(float scale);

    /* Resolution of output in dots per mm (BMP/EMF/PCX/PNG/TIF only) */
    float dpmm() const; // `symbol->dpmm`
    void setDPMM(float dpmm);

    /* Dotty mode */
    bool dotty() const; // `symbol->input_mode | BARCODE_DOTTY_MODE`
    void setDotty(bool botty);

    /* Size of dots used in BARCODE_DOTTY_MODE */
    float dotSize() const; // `symbol->dot_size`
    void setDotSize(float dot_size);

    /* Height in X-dimensions that EAN/UPC guard bars descend */
    float guardDescent() const; // `symbol->guard_descent`
    void setGuardDescent(float guardDescent);

    /* Structured Append info */
    int structAppCount() const; // `symbol->structapp.count`
    int structAppIndex() const; // `symbol->structapp.index`
    QString structAppID() const; // `symbol->structapp.id`
    void setStructApp(const int count, const int index, const QString& id);
    void clearStructApp();

    /* Foreground colour (may be RGB(A) hex string or CMYK decimal "C,M,Y,K" percentage string) */
    QString fgStr() const; // `symbol->fgcolour`
    bool setFgStr(const QString& fgStr); // Returns false if not valid colour string

    /* Foreground colour as QColor */
    QColor fgColor() const; // `symbol->fgcolour`
    void setFgColor(const QColor& fgColor);

    /* Background colour (may be RGB(A) hex string or CMYK decimal "C,M,Y,K" percentage string) */
    QString bgStr() const; // `symbol->bgcolour`
    bool setBgStr(const QString& bgStr); // Returns false if not valid colour string

    /* Background colour as QColor */
    QColor bgColor() const; // `symbol->bgcolour`
    void setBgColor(const QColor& bgColor);

    /* Use CMYK colour space (Encapsulated PostScript and TIF) */
    bool cmyk() const; // `symbol->output_options | CMYK_COLOUR`
    void setCMYK(bool cmyk);

    /* Type of border above/below/around barcode */
    int borderType() const; // `symbol->output_options | BARCODE_BIND | BARCODE_BOX | BARCODE_BIND_TOP`
    void setBorderType(int borderTypeIndex);

    /* Size of border in X-dimensions */
    int borderWidth() const; // `symbol->border_width`
    void setBorderWidth(int borderWidth);

    /* Width in X-dimensions of whitespace to left & right of barcode */
    int whitespace() const; // `symbol->whitespace_width`
    void setWhitespace(int whitespace);

    /* Height in X-dimensions of whitespace above & below the barcode */
    int vWhitespace() const; // `symbol->whitespace_height`
    void setVWhitespace(int vWhitespace);

    /* Type of font to use i.e. normal, small, bold or (vector only) small bold */
    int fontSetting() const; // `symbol->output_options | SMALL_TEXT | BOLD_TEXT`
    void setFontSetting(int fontSettingIndex); // Sets from comboBox index
    void setFontSettingValue(int fontSetting); // Sets literal value

    /* Text gap */
    float textGap() const; // `symbol->text_gap`
    void setTextGap(float textGap);

    /* Show (true) or hide (false) Human Readable Text (HRT) */
    bool showText() const; // `symbol->show_hrt`
    void setShowText(bool showText);

    /* Set to true to use GS (Group Separator) instead of FNC1 as GS1 separator (Data Matrix) */
    bool gsSep() const; // `symbol->output_options | GS1_GS_SEPARATOR`
    void setGSSep(bool gsSep);

    /* Add compliant quiet zones (additional to any specified whitespace)
       Note: CODE16K, CODE49, CODABLOCKF, ITF14, EAN/UPC have default quiet zones */
    bool quietZones() const; // `symbol->output_options | BARCODE_QUIET_ZONES`
    void setQuietZones(bool quietZones);

    /* Disable quiet zones, notably those with defaults as listed above */
    bool noQuietZones() const; // `symbol->output_options | BARCODE_NO_QUIET_ZONES`
    void setNoQuietZones(bool noQuietZones);

    /* Warn if height not compliant and use standard height (if any) as default */
    bool compliantHeight() const; // `symbol->output_options | COMPLIANT_HEIGHT`
    void setCompliantHeight(bool compliantHeight);

    /* Rotate barcode by angle (degrees 0, 90, 180 and 270) */
    int rotateAngle() const;
    void setRotateAngle(int rotateIndex); // Sets from comboBox index
    void setRotateAngleValue(int rotateAngle); // Sets literal value

    /* Extended Channel Interpretation (segment 0 eci) */
    int eci() const; // `symbol->eci`
    void setECI(int ECIIndex); // Sets from comboBox index
    void setECIValue(int eci); // Sets literal value

    /* Process parentheses as GS1 AI delimiters (instead of square brackets) */
    bool gs1Parens() const; // `symbol->input_mode | GS1PARENS_MODE`
    void setGS1Parens(bool gs1Parens);

    /* Do not check validity of GS1 data (except that printable ASCII only) */
    bool gs1NoCheck() const; // `symbol->input_mode | GS1NOCHECK_MODE`
    void setGS1NoCheck(bool gs1NoCheck);

    /* Reader Initialisation (Programming) */
    bool readerInit() const; // `symbol->output_options | READER_INIT`
    void setReaderInit(bool readerInit);

    /* Whether to add quiet zone indicators ("<", ">") to HRT (EAN/UPC) */
    bool guardWhitespace() const; // `symbol->output_options | EANUPC_GUARD_WHITESPACE`
    void setGuardWhitespace(bool guardWhitespace);

    /* Whether to embed the font in vector output - currently only for SVG output of EAN/UPC */
    bool embedVectorFont() const; // `symbol->output_options | EANUPC_GUARD_WHITESPACE`
    void setEmbedVectorFont(bool embedVectorFont);

    /* Affects error/warning value returned by Zint API (see `getError()` below) */
    int warnLevel() const; // `symbol->warn_level`
    void setWarnLevel(int warnLevel);

    /* Debugging flags */
    bool debug() const; // `symbol->debug`
    void setDebug(bool debug);


    /* Symbol output info set by Zint on successful `render()` */
    int encodedWidth() const; // Read-only, encoded width (no. of modules encoded)
    int encodedRows() const; // Read-only, no. of rows encoded
    float encodedHeight() const; // Read-only, in X-dimensions
    float vectorWidth() const; // Read-only, scaled width
    float vectorHeight() const; // Read-only, scaled height


    /* Legacy property getters/setters */
    void setWidth(int width); /* `symbol->option_2` */
    int width() const;
    void setSecurityLevel(int securityLevel); /* `symbol->option_1` */
    int securityLevel() const;
    void setPdf417CodeWords(int pdf417CodeWords); /* No-op */
    int pdf417CodeWords() const;
    void setHideText(bool hide); /* `setShowText(!hide)` */
    void setTargetSize(int width, int height); /* No-op */
    QString error_message() const; /* Same as `lastError()` */


    /* Test capabilities - `ZBarcode_Cap()` */
    bool hasHRT(int symbology = 0) const;
    bool isStackable(int symbology = 0) const;
    bool isEANUPC(int symbology = 0) const;
    bool isExtendable(int symbology = 0) const; /* Legacy - same as `isEANUPC()` */
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

    /* Whether takes GS1 AI-delimited data */
    bool takesGS1AIData(int symbology = 0) const;


    /* Error or warning returned by Zint on `render()` or `save_to_file()` */
    int getError() const;

    /* Error message returned by Zint on `render()` or `save_to_file()` */
    const QString& lastError() const; // `symbol->errtxt`

    /* Whether `lastError()` set */
    bool hasErrors() const; // `symbol->errtxt`


    /* Encode and print barcode to file `filename`. Only sets `getError()` on error, not on warning */
    bool save_to_file(const QString& filename); // `ZBarcode_Print()`

    /* Encode and display barcode in `paintRect` using `painter`.
       Note: legacy argument `mode` is not used */
    void render(QPainter& painter, const QRectF& paintRect, AspectRatioMode mode = IgnoreAspectRatio);


    /* Returns the default X-dimension (`ZBarcode_Default_Xdim()`).
       If `symbology` non-zero then used instead of `symbol()` */
    float defaultXdim(int symbology = 0) const;

    /* Returns the scale to use for X-dimension `x_dim_mm` at `dpmm` for `filetype`.
       If `symbology` non-zero then used instead of `symbol()` */
    float getScaleFromXdimDp(float x_dim_mm, float dpmm, const QString& fileType, int symbology = 0) const;
    // `ZBarcode_Scale_Xdim()`

    /* Reverse of `getScaleFromXdimDp()` above, returning the X-dimension or dot density given the scale `scale`.
       If `symbology` non-zero then used instead of `symbol()` */
    float getXdimDpFromScale(float scale, float x_dim_mm_or_dpmm, const QString& fileType, int symbology = 0) const;
    // `ZBarcode_XdimDp_From_Scale()`

    /* Set `width_x_dim` and `height_x_dim` with estimated size of barcode based on X-dimension `x_dim`. To be called
       after a successful `render()`. Returns false if `scale()` zero or render is in error, otherwise true */
    bool getWidthHeightXdim(float x_dim, float &width_x_dim, float &height_x_dim) const;


    /* Return the BARCODE_XXX name of `symbology` */
    static QString barcodeName(const int symbology); // `ZBarcode_BarcodeName()`

    /* Whether Zint library "libzint" built with PNG support or not */
    static bool noPng(); // `ZBarcode_NoPng()`

    /* Version of Zint library "libzint" linked to */
    static int getVersion(); // `ZBarcode_Version()`


    /* Translate settings into Command Line equivalent. Set `win` to use Windows escaping of data.
       If `autoHeight` set then `--height=` option will not be emitted.
       If HEIGHTPERROW_MODE set and non-zero `heightPerRow` given then use that for height instead of internal
       height */
    QString getAsCLI(const bool win, const bool longOptOnly = false, const bool barcodeNames = false,
                const bool noEXE = false, const bool autoHeight = false, const float heightPerRow = 0.0f,
                const QString& outfile = "", const QZintXdimDpVars *xdimdpVars = nullptr) const;

signals:
    void encoded(); // Emitted on successful `render()`
    void errored(); // Emitted if an error (not warning) occurred on `render()`

private:
    bool resetSymbol(); // Reset the symbol structure for encoding using member fields
    void encode(); // `ZBarcode_Encode_and_Buffer_Vector()` or `ZBarcode_Encode_Segs_and_Buffer_Vector()`

    /* Helper to convert `m_segs` to `struct zint_seg[]` */
    int convertSegs(struct zint_seg segs[], std::vector<QByteArray>& bstrs);

    /* Convert `zint_vector_rect->colour` to Qt color */
    static Qt::GlobalColor colourToQtColor(int colour);

    /* `getAsCLI()` helpers */
    static void arg_str(QString& cmd, const char *const opt, const QString& val);
    static void arg_int(QString& cmd, const char *const opt, const int val, const bool allowZero = false);
    static void arg_bool(QString& cmd, const char *const opt, const bool val);
    static void arg_data(QString& cmd, const char *const opt, const QString& val, const bool win);
    static void arg_seg(QString& cmd, const int seg_no, const QZintSeg& val, const bool win);
    static void arg_data_esc(QString& cmd, const char *const opt, QString& text, const bool win);
    static void arg_float(QString& cmd, const char *const opt, const float val, const bool allowZero = false);
    static void arg_structapp(QString& cmd, const char *const opt, const int count, const int index,
                                const QString& id, const bool win);
    static void arg_scalexdimdp(QString& cmd, const char *const opt, const float scale, const float dpmm,
                                const int symbol, const QZintXdimDpVars *xdimdpVars);

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
    float m_dpmm;
    float m_scale;
    bool m_dotty;
    float m_dot_size;
    float m_guardDescent;
    float m_textGap;
    struct zint_structapp m_structapp;
    QString m_fgStr;
    QString m_bgStr;
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
    bool m_guard_whitespace;
    bool m_embed_vector_font;
    int m_warn_level;
    bool m_debug;
    int m_encodedWidth;
    int m_encodedRows;
    float m_encodedHeight;
    float m_vectorWidth;
    float m_vectorHeight;
    QString m_lastError;
    int m_error;

    int target_size_horiz; /* Legacy */
    int target_size_vert; /* Legacy */
};

/* Printing Scale settings */
struct QZintXdimDpVars {
    double x_dim = 0.0; // X-dimension in `x_dim_units`
    int x_dim_units = 0; // 0 for mm, 1 for inches
    int resolution = 0; // Dot density in `resolution_units`
    int resolution_units = 0; // 0 for dpmm, 1 for dpi
    int filetype = 0; // For non-MaxiCode, 0 for GIF (raster), 1 for SVG (vector)
    int filetype_maxicode = 0; // For MaxiCode only, 0 for GIF (raster), 1 for SVG (vector), 2 for EMF
    int set = 0; // 1 if explicitly set, 0 if just defaults (in which case the struct isn't applicable to `dpmm()`)

    /* Helper to return "GIF"/"SVG"(/"EMF") if `msg` false, "raster"/"vector"(/"EMF") otherwise
       (EMF only if `symbol` is MaxiCode) */
    static const char *getFileType(int symbol, const struct QZintXdimDpVars *vars, bool msg = false);
};

} /* namespace Zint */

/* vim: set ts=4 sw=4 et : */
#endif /* QZINT_H */
