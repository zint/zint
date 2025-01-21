/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra <bogdan@licentia.eu>               *
 *   Copyright (C) 2009-2024 by Robin Stuart <rstuart114@gmail.com>        *
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

//#include <QDebug>
#include <QAction>
#include <QClipboard>
#include <QColor>
#include <QColorDialog>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QImage>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QRadioButton>
#include <QScreen>
#include <QSettings>
#include <QShortcut>
#include <QStandardItemModel>
#include <QTextStream>
#include <QUiLoader>

#include <math.h>
#include "mainwindow.h"
#include "cliwindow.h"
#include "datawindow.h"
#include "scalewindow.h"
#include "sequencewindow.h"

// Shorthand
#define QSL     QStringLiteral
#define QSEmpty QLatin1String("")

static const int tempMessageTimeout = 2000;

// Use on Windows also (i.e. not using QKeySequence::Quit)
Q_GLOBAL_STATIC_WITH_ARGS(QKeySequence, quitKeySeq, (Qt::CTRL | Qt::Key_Q))

Q_GLOBAL_STATIC_WITH_ARGS(QKeySequence, openCLISeq, (Qt::SHIFT | Qt::CTRL | Qt::Key_C))

Q_GLOBAL_STATIC_WITH_ARGS(QKeySequence, copyBMPSeq, (Qt::SHIFT | Qt::CTRL | Qt::Key_B))
Q_GLOBAL_STATIC_WITH_ARGS(QKeySequence, copyEMFSeq, (Qt::SHIFT | Qt::CTRL | Qt::Key_E))
Q_GLOBAL_STATIC_WITH_ARGS(QKeySequence, copyGIFSeq, (Qt::SHIFT | Qt::CTRL | Qt::Key_G))
Q_GLOBAL_STATIC_WITH_ARGS(QKeySequence, copyPNGSeq, (Qt::SHIFT | Qt::CTRL | Qt::Key_P))
Q_GLOBAL_STATIC_WITH_ARGS(QKeySequence, copySVGSeq, (Qt::SHIFT | Qt::CTRL | Qt::Key_S))
Q_GLOBAL_STATIC_WITH_ARGS(QKeySequence, copyTIFSeq, (Qt::SHIFT | Qt::CTRL | Qt::Key_T))

Q_GLOBAL_STATIC_WITH_ARGS(QKeySequence, factoryResetSeq, (Qt::SHIFT | Qt::CTRL | Qt::Key_R))

// RGB hexadecimal 6 or 8 in length or CMYK comma-separated decimal percentages "C,M,Y,K"
static const QString colorREstr(QSL("^([0-9A-Fa-f]{6}([0-9A-Fa-f]{2})?)|(((100|[0-9]{0,2}),){3}(100|[0-9]{0,2}))$"));
Q_GLOBAL_STATIC_WITH_ARGS(QRegularExpression, colorRE, (colorREstr))

static const QString fgDefault(QSL("000000"));
static const QString bgDefault(QSL("FFFFFF"));
static const QString fgDefaultAlpha(QSL("000000FF"));
static const QString bgDefaultAlpha(QSL("FFFFFFFF"));

static QString qcolor_to_str(const QColor &color)
{
    if (color.alpha() == 0xFF) {
        return QString::asprintf("%02X%02X%02X", color.red(), color.green(), color.blue());
    }
    return QString::asprintf("%02X%02X%02X%02X", color.red(), color.green(), color.blue(), color.alpha());
}

static QColor str_to_qcolor(const QString &str)
{
    QColor color;
    int r, g, b, a;
    if (str.contains(',')) {
        int comma1 = str.indexOf(',');
        int comma2 = str.indexOf(',', comma1 + 1);
        int comma3 = str.indexOf(',', comma2 + 1);
        int black = 100 - str.mid(comma3 + 1).toInt();
        int val = 100 - str.mid(0, comma1).toInt();
        r = (int) roundf((0xFF * val * black) / 10000.0f);
        val = 100 - str.mid(comma1 + 1, comma2 - comma1 - 1).toInt();
        g = (int) roundf((0xFF * val * black) / 10000.0f);
        val = 100 - str.mid(comma2 + 1, comma3 - comma2 - 1).toInt();
        b = (int) roundf((0xFF * val * black) / 10000.0f);
        a = 0xFF;
    } else {
        r = str.mid(0, 2).toInt(nullptr, 16);
        g = str.mid(2, 2).toInt(nullptr, 16);
        b = str.mid(4, 2).toInt(nullptr, 16);
        a = str.length() == 8 ? str.mid(6, 2).toInt(nullptr, 16) : 0xFF;
    }
    color.setRgb(r, g, b, a);
    return color;
}

struct bstyle_item {
    const QString text;
    int symbology;
};

static const struct bstyle_item bstyle_items[] = {
    { QSL("Australia Post Redirect Code"), BARCODE_AUSREDIRECT },
    { QSL("Australia Post Reply-Paid"), BARCODE_AUSREPLY },
    { QSL("Australia Post Routing Code"), BARCODE_AUSROUTE },
    { QSL("Australia Post Standard Customer"), BARCODE_AUSPOST },
    { QSL("Aztec Code (ISO 24778) (and HIBC)"), BARCODE_AZTEC },
    { QSL("Aztec Runes (ISO 24778)"), BARCODE_AZRUNE },
    { QSL("BC412 (SEMI T1-95)"), BARCODE_BC412 },
    { QSL("Brazilian Postal Code (CEPNet)"), BARCODE_CEPNET },
    { QSL("Channel Code"), BARCODE_CHANNEL },
    { QSL("Codabar (EN 798)"), BARCODE_CODABAR },
    { QSL("Codablock-F (and HIBC)"), BARCODE_CODABLOCKF },
    { QSL("Code 11"), BARCODE_CODE11 },
    { QSL("Code 128 (ISO 15417) (and GS1-128 and HIBC)"), BARCODE_CODE128 },
    { QSL("Code 16K (EN 12323)"), BARCODE_CODE16K },
    { QSL("Code 2 of 5 Data Logic"), BARCODE_C25LOGIC },
    { QSL("Code 2 of 5 IATA"), BARCODE_C25IATA },
    { QSL("Code 2 of 5 Industrial"), BARCODE_C25IND },
    { QSL("Code 2 of 5 Interleaved (ISO 16390)"), BARCODE_C25INTER },
    { QSL("Code 2 of 5 Standard (Matrix)"), BARCODE_C25STANDARD },
    { QSL("Code 32 (Italian Pharmacode)"), BARCODE_CODE32 },
    { QSL("Code 39 (ISO 16388) (and HIBC)"), BARCODE_CODE39 },
    { QSL("Code 39 Extended"), BARCODE_EXCODE39 },
    { QSL("Code 49"), BARCODE_CODE49 },
    { QSL("Code 93"), BARCODE_CODE93 },
    { QSL("Code One"), BARCODE_CODEONE },
    { QSL("DAFT Code"), BARCODE_DAFT },
    { QSL("Data Matrix (ISO 16022) (and HIBC)"), BARCODE_DATAMATRIX },
    { QSL("Deutsche Post Identcode"), BARCODE_DPIDENT },
    { QSL("Deutsche Post Leitcode"), BARCODE_DPLEIT },
    { QSL("DotCode"), BARCODE_DOTCODE },
    { QSL("DPD Code"), BARCODE_DPD },
    { QSL("Dutch Post KIX"), BARCODE_KIX },
    { QSL("DX Film Edge"), BARCODE_DXFILMEDGE },
    { QSL("EAN (EAN-2, EAN-5, EAN-8 and EAN-13) (ISO 15420)"), BARCODE_EANX },
    { QSL("EAN-14"), BARCODE_EAN14 },
    { QSL("FIM (Facing Identification Mark)"), BARCODE_FIM },
    { QSL("Flattermarken"), BARCODE_FLAT },
    { QSL("Grid Matrix"), BARCODE_GRIDMATRIX },
    { QSL("GS1 DataBar Expanded (ISO 24724)"), BARCODE_DBAR_EXP },
    { QSL("GS1 DataBar Expanded Stacked (ISO 24724)"), BARCODE_DBAR_EXPSTK },
    { QSL("GS1 DataBar Limited (ISO 24724)"), BARCODE_DBAR_LTD },
    { QSL("GS1 DataBar Omnidirectional (and Truncated) (ISO 24724)"), BARCODE_DBAR_OMN },
    { QSL("GS1 DataBar Stacked (ISO 24724)"), BARCODE_DBAR_STK },
    { QSL("GS1 DataBar Stacked Omnidirectional (ISO 24724)"), BARCODE_DBAR_OMNSTK },
    { QSL("Han Xin (Chinese Sensible) Code (ISO 20830)"), BARCODE_HANXIN },
    { QSL("ISBN (International Standard Book Number)"), BARCODE_ISBNX },
    { QSL("ITF-14"), BARCODE_ITF14 },
    { QSL("Japanese Postal Barcode"), BARCODE_JAPANPOST },
    { QSL("Korean Postal Barcode"), BARCODE_KOREAPOST },
    { QSL("LOGMARS"), BARCODE_LOGMARS },
    { QSL("MaxiCode (ISO 16023)"), BARCODE_MAXICODE },
    { QSL("MicroPDF417 (ISO 24728) (and HIBC)"), BARCODE_MICROPDF417 },
    { QSL("Micro QR Code (ISO 18004)"), BARCODE_MICROQR },
    { QSL("MSI Plessey"), BARCODE_MSI_PLESSEY },
    { QSL("NVE-18 (SSCC-18)"), BARCODE_NVE18 },
    { QSL("PDF417 (ISO 15438) (and Compact and HIBC)"), BARCODE_PDF417 },
    { QSL("Pharmacode"), BARCODE_PHARMA },
    { QSL("Pharmacode 2-track"), BARCODE_PHARMA_TWO },
    { QSL("Pharma Zentralnummer (PZN)"), BARCODE_PZN },
    { QSL("PLANET"), BARCODE_PLANET },
    { QSL("POSTNET"), BARCODE_POSTNET },
    { QSL("QR Code (ISO 18004) (and HIBC)"), BARCODE_QRCODE },
    { QSL("Rectangular Micro QR (rMQR) (ISO 23941)"), BARCODE_RMQR },
    { QSL("Royal Mail 2D Mailmark (CMDM) (Data Matrix)"), BARCODE_MAILMARK_2D },
    { QSL("Royal Mail 4-state Customer Code (RM4SCC)"), BARCODE_RM4SCC },
    { QSL("Royal Mail 4-state Mailmark"), BARCODE_MAILMARK_4S },
    { QSL("Telepen"), BARCODE_TELEPEN },
    { QSL("Telepen Numeric"), BARCODE_TELEPEN_NUM },
    { QSL("UK Plessey"), BARCODE_PLESSEY },
    { QSL("Ultracode"), BARCODE_ULTRA },
    { QSL("UPC-A (ISO 15420)"), BARCODE_UPCA },
    { QSL("UPC-E (ISO 15420)"), BARCODE_UPCE },
    { QSL("UPNQR"), BARCODE_UPNQR },
    { QSL("UPU S10"), BARCODE_UPU_S10 },
    { QSL("USPS Intelligent Mail (OneCode)"), BARCODE_USPS_IMAIL },
    { QSL("VIN (Vehicle Identification Number)"), BARCODE_VIN },
};

#ifdef Q_OS_MACOS

/* Helper to make data tab vertical layouts look ok on macOS */
void MainWindow::mac_hack_vLayouts(QWidget *win)
{
    QList<QVBoxLayout *> vlayouts = win->findChildren<QVBoxLayout *>();
    for (int i = 0, cnt = vlayouts.size(); i < cnt; i++) {
        if (vlayouts[i]->objectName() == "vLayoutData" || vlayouts[i]->objectName() == "vLayoutComposite"
                || vlayouts[i]->objectName() == "vLayoutSegs") {
            vlayouts[i]->setSpacing(2);
            // If set spacing on QVBoxLayout then it seems its QHBoxLayout children inherit this so undo
            QList<QHBoxLayout *> hlayouts = vlayouts[i]->findChildren<QHBoxLayout *>();
            for (int j = 0, cnt = hlayouts.size(); j < cnt; j++) {
                hlayouts[j]->setSpacing(8);
            }
        }
    }
}

/* Helper to make status bars look ok on macOS */
void MainWindow::mac_hack_statusBars(QWidget *win, const char* name)
{
    QList<QStatusBar *> sbars = name ? win->findChildren<QStatusBar *>(name) : win->findChildren<QStatusBar *>();
    QColor bgColor = QGuiApplication::palette().window().color();
    QString sbarSS = QSL("QStatusBar {background-color:") + bgColor.name() + QSL(";}");
    for (int i = 0, cnt = sbars.size(); i < cnt; i++) {
        sbars[i]->setStyleSheet(sbarSS);
    }
}
#endif

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags fl)
        : QWidget(parent, fl), m_previewBgColor(0xF4, 0xF4, 0xF4), m_optionWidget(nullptr), m_symbology(0),
          m_menu(nullptr),
          m_lblHeightPerRow(nullptr), m_spnHeightPerRow(nullptr),
          m_btnHeightPerRowDisable(nullptr), m_btnHeightPerRowDefault(nullptr),
          m_scaleWindow(nullptr)
{
    // Undocumented command line debug flag
    m_bc.bc.setDebug(QCoreApplication::arguments().contains(QSL("--verbose")));

    QCoreApplication::setOrganizationName(QSL("zint"));
    QCoreApplication::setOrganizationDomain(QSL("zint.org.uk"));
    QCoreApplication::setApplicationName(QSL("Barcode Studio"));

    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif

    scene = new QGraphicsScene(this);

    setupUi(this);
    view->setScene(scene);

    QVariant saved_geometry = settings.value(QSL("studio/window_geometry"));

#ifdef Q_OS_MACOS
    // Standard width 360 too narrow
    if (saved_geometry.isNull()) {
        // Seems this is necessary on macOS to get a reasonable initial height
        setMinimumSize(QSize(460, (int) (QApplication::primaryScreen()->availableSize().height() * 0.9f)));
    } else {
        setMinimumSize(QSize(460, 0));
    }
    mac_hack_vLayouts(this);
    mac_hack_statusBars(this, "statusBar");
    vLayoutTabData->setContentsMargins(QMargins(20, 0, 20, 0));
    tabMain->setMinimumSize(QSize(0, 380));
    tabMain->setMaximumSize(QSize(16777215, 380));
#endif
#ifdef _WIN32
    tabMain->setMinimumSize(QSize(0, 316));
    tabMain->setMaximumSize(QSize(16777215, 316));
#endif

    restoreGeometry(saved_geometry.toByteArray());

    m_fgcolor_geometry = settings.value(QSL("studio/fgcolor_geometry")).toByteArray();
    m_bgcolor_geometry = settings.value(QSL("studio/bgcolor_geometry")).toByteArray();
    m_previewbgcolor_geometry = settings.value(QSL("studio/previewbgcolor_geometry")).toByteArray();

    btnScale->setIcon(QIcon(QSL(":res/scaling.svg")));
    fgcolor->setIcon(QIcon(QSL(":res/black-eye.svg")));
    bgcolor->setIcon(QIcon(QSL(":res/white-eye.svg")));
    btnReverse->setIcon(QIcon(QSL(":res/shuffle.svg")));

    QRegularExpressionValidator *colorValidator = new QRegularExpressionValidator(*colorRE, this);
    txt_fgcolor->setValidator(colorValidator);
    txt_bgcolor->setValidator(colorValidator);

    connect(txt_fgcolor, SIGNAL(textEdited(QString)), this, SLOT(fgcolor_edited()));
    connect(txt_bgcolor, SIGNAL(textEdited(QString)), this, SLOT(bgcolor_edited()));

    const int cnt = (int) (sizeof(bstyle_items) / sizeof(bstyle_items[0]));
    for (int i = 0; i < cnt; i++) {
        bstyle->addItem(bstyle_items[i].text);
    }
#ifdef _WIN32
    bstyle->setMaxVisibleItems(cnt); /* Apart from increasing combo size, seems to be needed for filter to work */
#endif
#if QT_VERSION < 0x50A00
    /* Prior to Qt 5.10 comboboxes have display issues when filtered (scrollers not accounted for), so disable */
    filter_bstyle->hide();
#endif
    bstyle->setCurrentIndex(settings.value(QSL("studio/symbology"), 12).toInt());

    load_settings(settings);

    // Set background of preview - allows whitespace and quiet zones to be more easily seen
    m_bc.setColor(m_previewBgColor);

    QIcon clearIcon(QSL(":res/delete.svg"));
    btnClearData->setIcon(clearIcon);
    btnClearDataSeg1->setIcon(clearIcon);
    btnClearDataSeg2->setIcon(clearIcon);
    btnClearDataSeg3->setIcon(clearIcon);
    btnClearComposite->setIcon(clearIcon);
    btnClearTextGap->setIcon(clearIcon);
    btnZap->setIcon(QIcon(QSL(":res/zap.svg")));

    change_options();

    scene->addItem(&m_bc);

    connect(bstyle, SIGNAL(currentIndexChanged(int)), SLOT(change_options()));
    connect(bstyle, SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
    connect(filter_bstyle, SIGNAL(textChanged(QString)), SLOT(filter_symbologies()));
    connect(heightb, SIGNAL(valueChanged(double)), SLOT(update_preview()));
    connect(bwidth,  SIGNAL(valueChanged(int)), SLOT(update_preview()));
    connect(btype, SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
    connect(cmbFontSetting, SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
    connect(spnTextGap, SIGNAL(valueChanged(double)), SLOT(text_gap_ui_set()));
    connect(spnTextGap, SIGNAL(valueChanged(double)), SLOT(update_preview()));
    connect(btnClearTextGap, SIGNAL(clicked(bool)), SLOT(clear_text_gap()));
    connect(txtData, SIGNAL(textChanged(QString)), SLOT(data_ui_set()));
    connect(txtData, SIGNAL(textChanged(QString)), SLOT(upcae_no_quiet_zones_ui_set()));
    connect(txtData, SIGNAL(textChanged(QString)), SLOT(update_preview()));
    connect(txtDataSeg1, SIGNAL(textChanged(QString)), SLOT(data_ui_set()));
    connect(txtDataSeg1, SIGNAL(textChanged(QString)), SLOT(update_preview()));
    connect(txtDataSeg2, SIGNAL(textChanged(QString)), SLOT(data_ui_set()));
    connect(txtDataSeg2, SIGNAL(textChanged(QString)), SLOT(update_preview()));
    connect(txtDataSeg3, SIGNAL(textChanged(QString)), SLOT(data_ui_set()));
    connect(txtDataSeg3, SIGNAL(textChanged(QString)), SLOT(update_preview()));
    connect(txtComposite, SIGNAL(textChanged()), SLOT(update_preview()));
    connect(chkComposite, SIGNAL(toggled(bool)), SLOT(composite_ui_set()));
    connect(chkComposite, SIGNAL(toggled(bool)), SLOT(update_preview()));
    connect(cmbCompType, SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
    connect(btnClearComposite, SIGNAL(clicked(bool)), SLOT(clear_composite()));
    connect(cmbECI, SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
    connect(cmbECISeg1, SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
    connect(cmbECISeg2, SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
    connect(cmbECISeg3, SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
    connect(chkEscape, SIGNAL(toggled(bool)), SLOT(update_preview()));
    connect(chkData, SIGNAL(toggled(bool)), SLOT(update_preview()));
    connect(chkRInit, SIGNAL(toggled(bool)), SLOT(update_preview()));
    connect(chkGS1Parens, SIGNAL(toggled(bool)), SLOT(update_preview()));
    connect(chkGS1NoCheck, SIGNAL(toggled(bool)), SLOT(update_preview()));
    connect(spnWhitespace, SIGNAL(valueChanged(int)), SLOT(update_preview()));
    connect(spnVWhitespace, SIGNAL(valueChanged(int)), SLOT(update_preview()));
    connect(btnMenu, SIGNAL(clicked(bool)), SLOT(menu()));
    connect(btnSave, SIGNAL(clicked(bool)), SLOT(save()));
    connect(spnScale, SIGNAL(valueChanged(double)), SLOT(update_preview()));
    connect(btnExit, SIGNAL(clicked(bool)), SLOT(quit_now()));
    connect(fgcolor, SIGNAL(clicked(bool)), SLOT(fgcolor_clicked()));
    connect(bgcolor, SIGNAL(clicked(bool)), SLOT(bgcolor_clicked()));
    connect(btnReset, SIGNAL(clicked(bool)), SLOT(reset_colours()));
    connect(btnReverse, SIGNAL(clicked(bool)), SLOT(reverse_colours()));
    connect(btnMoreData, SIGNAL(clicked(bool)), SLOT(open_data_dialog()));
    connect(btnMoreDataSeg1, SIGNAL(clicked(bool)), SLOT(open_data_dialog_seg1()));
    connect(btnMoreDataSeg2, SIGNAL(clicked(bool)), SLOT(open_data_dialog_seg2()));
    connect(btnMoreDataSeg3, SIGNAL(clicked(bool)), SLOT(open_data_dialog_seg3()));
    connect(btnClearData, SIGNAL(clicked(bool)), SLOT(clear_data()));
    connect(btnClearDataSeg1, SIGNAL(clicked(bool)), SLOT(clear_data_seg1()));
    connect(btnClearDataSeg2, SIGNAL(clicked(bool)), SLOT(clear_data_seg2()));
    connect(btnClearDataSeg3, SIGNAL(clicked(bool)), SLOT(clear_data_seg3()));
    connect(btnSequence, SIGNAL(clicked(bool)), SLOT(open_sequence_dialog()));
    connect(btnZap, SIGNAL(clicked(bool)), SLOT(zap()));
    connect(chkAutoHeight, SIGNAL(toggled(bool)), SLOT(autoheight_ui_set()));
    connect(chkAutoHeight, SIGNAL(toggled(bool)), SLOT(update_preview()));
    connect(chkCompliantHeight, SIGNAL(toggled(bool)), SLOT(update_preview()));
    connect(btnScale, SIGNAL(clicked(bool)), SLOT(open_scale_dialog()));
    connect(chkHRTShow, SIGNAL(toggled(bool)), SLOT(HRTShow_ui_set()));
    connect(chkHRTShow, SIGNAL(toggled(bool)), SLOT(update_preview()));
    connect(chkCMYK, SIGNAL(toggled(bool)), SLOT(change_cmyk()));
    connect(chkQuietZones, SIGNAL(toggled(bool)), SLOT(update_preview()));
    connect(cmbRotate, SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
    connect(chkDotty, SIGNAL(toggled(bool)), SLOT(dotty_ui_set()));
    connect(chkDotty, SIGNAL(toggled(bool)), SLOT(update_preview()));
    connect(spnDotSize, SIGNAL(valueChanged(double)), SLOT(update_preview()));
    connect(btnCopySVG, SIGNAL(clicked(bool)), SLOT(copy_to_clipboard_svg()));
    connect(btnCopyBMP, SIGNAL(clicked(bool)), SLOT(copy_to_clipboard_bmp()));

    connect(&m_bc.bc, SIGNAL(encoded()), SLOT(on_encoded()));
    connect(&m_bc.bc, SIGNAL(errored()), SLOT(on_errored()));

    connect(view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(view_context_menu(QPoint)));
    connect(errtxtBar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(errtxtBar_context_menu(QPoint)));

    // Will enable/disable these on error
    m_saveAsShortcut = new QShortcut(QKeySequence::Save, this);
    connect(m_saveAsShortcut, SIGNAL(activated()), SLOT(save()));
    m_openCLIShortcut = new QShortcut(*openCLISeq, this);
    connect(m_openCLIShortcut, SIGNAL(activated()), SLOT(open_cli_dialog()));
    m_copyBMPShortcut = new QShortcut(*copyBMPSeq, this);
    connect(m_copyBMPShortcut, SIGNAL(activated()), SLOT(copy_to_clipboard_bmp()));
    m_copyEMFShortcut = new QShortcut(*copyEMFSeq, this);
    connect(m_copyEMFShortcut, SIGNAL(activated()), SLOT(copy_to_clipboard_emf()));
    m_copyGIFShortcut = new QShortcut(*copyGIFSeq, this);
    connect(m_copyGIFShortcut, SIGNAL(activated()), SLOT(copy_to_clipboard_gif()));
    if (!m_bc.bc.noPng()) {
        m_copyPNGShortcut = new QShortcut(*copyPNGSeq, this);
        connect(m_copyPNGShortcut, SIGNAL(activated()), SLOT(copy_to_clipboard_png()));
    }
    m_copySVGShortcut = new QShortcut(*copySVGSeq, this);
    connect(m_copySVGShortcut, SIGNAL(activated()), SLOT(copy_to_clipboard_svg()));
    m_copyTIFShortcut = new QShortcut(*copyTIFSeq, this);
    connect(m_copyTIFShortcut, SIGNAL(activated()), SLOT(copy_to_clipboard_tif()));

    m_factoryResetShortcut = new QShortcut(*factoryResetSeq, this);
    connect(m_factoryResetShortcut, SIGNAL(activated()), SLOT(factory_reset()));

    QShortcut *helpShortcut = new QShortcut(QKeySequence::HelpContents, this);
    connect(helpShortcut, SIGNAL(activated()), SLOT(help()));
    QShortcut *quitShortcut = new QShortcut(*quitKeySeq, this);
    connect(quitShortcut, SIGNAL(activated()), SLOT(quit_now()));

    createActions();
    createMenu();
    bstyle->setFocus();

    tabMain->installEventFilter(this);
    txt_fgcolor->installEventFilter(this);
    txt_bgcolor->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif

    settings.setValue(QSL("studio/window_geometry"), saveGeometry());
    settings.setValue(QSL("studio/fgcolor_geometry"), m_fgcolor_geometry);
    settings.setValue(QSL("studio/bgcolor_geometry"), m_bgcolor_geometry);
    settings.setValue(QSL("studio/previewbgcolor_geometry"), m_previewbgcolor_geometry);
    settings.setValue(QSL("studio/tab_index"), tabMain->currentIndex());
    settings.setValue(QSL("studio/symbology"), bstyle->currentIndex());
    settings.setValue(QSL("studio/ink/text"), m_fgstr);
    settings.setValue(QSL("studio/paper/text"), m_bgstr);
    if (m_previewBgColor.isValid()) {
        settings.setValue(QSL("studio/preview_bg_color"), m_previewBgColor.name());
    }
    settings.setValue(QSL("studio/data"), txtData->text());
    /* Seg data not saved so don't restore */
    settings.setValue(QSL("studio/composite_text"), txtComposite->toPlainText());
    settings.setValue(QSL("studio/chk_composite"), chkComposite->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/comp_type"), cmbCompType->currentIndex());
    settings.setValue(QSL("studio/eci"), cmbECI->currentIndex());
    /* Seg ECIs not saved so don't restore */
    settings.setValue(QSL("studio/chk_escape"), chkEscape->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/chk_data"), chkData->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/chk_rinit"), chkRInit->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/chk_gs1parens"), chkGS1Parens->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/chk_gs1nocheck"), chkGS1NoCheck->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/appearance/autoheight"), chkAutoHeight->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/appearance/compliantheight"), chkCompliantHeight->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/appearance/height"), heightb->value());
    settings.setValue(QSL("studio/appearance/border"), bwidth->value());
    settings.setValue(QSL("studio/appearance/whitespace"), spnWhitespace->value());
    settings.setValue(QSL("studio/appearance/vwhitespace"), spnVWhitespace->value());
    settings.setValue(QSL("studio/appearance/scale"), spnScale->value());
    settings.setValue(QSL("studio/appearance/border_type"), btype->currentIndex());
    settings.setValue(QSL("studio/appearance/font_setting"), cmbFontSetting->currentIndex());
    settings.setValue(QSL("studio/appearance/text_gap"), spnTextGap->value());
    settings.setValue(QSL("studio/appearance/chk_hrt_show"), chkHRTShow->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/appearance/chk_cmyk"), chkCMYK->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/appearance/chk_quiet_zones"), chkQuietZones->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/appearance/rotate"), cmbRotate->currentIndex());
    settings.setValue(QSL("studio/appearance/chk_embed_vector_font"), chkEmbedVectorFont->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/appearance/chk_dotty"), chkDotty->isChecked() ? 1 : 0);
    settings.setValue(QSL("studio/appearance/dot_size"), spnDotSize->value());
    // These are "system-wide"
    settings.setValue(QSL("studio/xdimdpvars/resolution"), m_xdimdpVars.resolution);
    settings.setValue(QSL("studio/xdimdpvars/resolution_units"), m_xdimdpVars.resolution_units);
    settings.setValue(QSL("studio/xdimdpvars/filetype"), m_xdimdpVars.filetype);
    settings.setValue(QSL("studio/xdimdpvars/filetype_maxicode"), m_xdimdpVars.filetype_maxicode);

    save_sub_settings(settings, m_bc.bc.symbol());
}

void MainWindow::load_settings(QSettings &settings)
{
    bool initial_load = m_symbology == 0;
    QString initialData(initial_load ? tr("Your Data Here!") : "");

    m_fgstr = settings.value(QSL("studio/ink/text"), QSEmpty).toString();
    if (m_fgstr.isEmpty()) {
        QColor color(settings.value(QSL("studio/ink/red"), 0).toInt(),
                    settings.value(QSL("studio/ink/green"), 0).toInt(),
                    settings.value(QSL("studio/ink/blue"), 0).toInt(),
                    settings.value(QSL("studio/ink/alpha"), 0xff).toInt());
        m_fgstr = qcolor_to_str(color);
    }
    if (m_fgstr.indexOf(*colorRE) != 0) {
        m_fgstr = fgDefault;
    }
    m_bgstr = settings.value(QSL("studio/paper/text"), QSEmpty).toString();
    if (m_bgstr.isEmpty()) {
        QColor color(settings.value(QSL("studio/paper/red"), 0).toInt(),
                    settings.value(QSL("studio/paper/green"), 0).toInt(),
                    settings.value(QSL("studio/paper/blue"), 0).toInt(),
                    settings.value(QSL("studio/paper/alpha"), 0xff).toInt());
        m_bgstr = qcolor_to_str(color);
    }
    if (m_bgstr.indexOf(*colorRE) != 0) {
        m_bgstr = bgDefault;
    }

    m_previewBgColor = QColor(settings.value(QSL("studio/preview_bg_color"), QSL("#F4F4F4")).toString());
    if (!m_previewBgColor.isValid()) {
        m_previewBgColor = QColor(0xF4, 0xF4, 0xF4);
    }

    txtData->setText(settings.value(QSL("studio/data"), initialData).toString());
    /* Don't save seg data */
    txtComposite->setText(settings.value(QSL("studio/composite_text"), initialData).toString());
    chkComposite->setChecked(settings.value(QSL("studio/chk_composite")).toInt() ? true : false);
    cmbCompType->setCurrentIndex(settings.value(QSL("studio/comp_type"), 0).toInt());
    cmbECI->setCurrentIndex(settings.value(QSL("studio/eci"), 0).toInt());
    /* Don't save seg ECIs */
    chkEscape->setChecked(settings.value(QSL("studio/chk_escape")).toInt() ? true : false);
    chkData->setChecked(settings.value(QSL("studio/chk_data")).toInt() ? true : false);
    chkRInit->setChecked(settings.value(QSL("studio/chk_rinit")).toInt() ? true : false);
    chkGS1Parens->setChecked(settings.value(QSL("studio/chk_gs1parens")).toInt() ? true : false);
    chkGS1NoCheck->setChecked(settings.value(QSL("studio/chk_gs1nocheck")).toInt() ? true : false);
    chkAutoHeight->setChecked(settings.value(QSL("studio/appearance/autoheight"), 1).toInt() ? true : false);
    chkCompliantHeight->setChecked(
        settings.value(QSL("studio/appearance/compliantheight"), 1).toInt() ? true : false);
    heightb->setValue(settings.value(QSL("studio/appearance/height"), 50.0f).toFloat());
    bwidth->setValue(settings.value(QSL("studio/appearance/border"), 0).toInt());
    spnWhitespace->setValue(settings.value(QSL("studio/appearance/whitespace"), 0).toInt());
    spnVWhitespace->setValue(settings.value(QSL("studio/appearance/vwhitespace"), 0).toInt());
    spnScale->setValue(settings.value(QSL("studio/appearance/scale"), 1.0).toFloat());
    btype->setCurrentIndex(settings.value(QSL("studio/appearance/border_type"), 0).toInt());
    cmbFontSetting->setCurrentIndex(settings.value(QSL("studio/appearance/font_setting"), 0).toInt());
    spnTextGap->setValue(settings.value(QSL("studio/appearance/text_gap"), 1.0).toFloat());
    chkHRTShow->setChecked(settings.value(QSL("studio/appearance/chk_hrt_show"), 1).toInt() ? true : false);
    chkCMYK->setChecked(settings.value(QSL("studio/appearance/chk_cmyk"), 0).toInt() ? true : false);
    chkQuietZones->setChecked(settings.value(QSL("studio/appearance/chk_quiet_zones"), 0).toInt() ? true : false);
    cmbRotate->setCurrentIndex(settings.value(QSL("studio/appearance/rotate"), 0).toInt());
    chkEmbedVectorFont->setChecked(settings.value(QSL("studio/appearance/chk_embed_vector_font"), 0).toInt()
                                                    ? true : false);
    chkDotty->setChecked(settings.value(QSL("studio/appearance/chk_dotty"), 0).toInt() ? true : false);
    spnDotSize->setValue(settings.value(QSL("studio/appearance/dot_size"), 4.0 / 5.0).toFloat());
    // These are "system-wide"
    m_xdimdpVars.resolution_units = std::max(std::min(settings.value(QSL("studio/xdimdpvars/resolution_units"),
                                                        0).toInt(), 1), 0);
    const int defaultResolution = m_xdimdpVars.resolution_units == 1 ? 300 : 12; // 300dpi or 12dpmm
    const int maxRes = m_xdimdpVars.resolution_units == 1 ? 25400 : 1000;
    m_xdimdpVars.resolution = std::max(std::min(settings.value(QSL("studio/xdimdpvars/resolution"),
                                                        defaultResolution).toInt(), maxRes), 1);
    m_xdimdpVars.filetype = std::max(std::min(settings.value(QSL("studio/xdimdpvars/filetype"), 0).toInt(), 1), 0);
    m_xdimdpVars.filetype_maxicode = std::max(std::min(settings.value(QSL("studio/xdimdpvars/filetype_maxicode"),
                                                        0).toInt(), 2), 0);
}

QString MainWindow::get_zint_version(void)
{
    QString zint_version;

    int lib_version = Zint::QZint::getVersion();
    int version_major = lib_version / 10000;
    int version_minor = (lib_version % 10000) / 100;
    int version_release = lib_version % 100;
    int version_build;

    if (version_release >= 9) {
        /* This is a test release */
        version_release = version_release / 10;
        version_build = lib_version % 10;
        QTextStream(&zint_version) << version_major << '.' << version_minor << '.' << version_release
                                    << '.' << version_build << QSL(" (dev)");
    } else {
        /* This is a stable release */
        QTextStream(&zint_version) << version_major << '.' << version_minor << '.' << version_release;
    }

    return zint_version;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update_preview();
}

bool MainWindow::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::StatusTip:
            statusBar->showMessage(static_cast<QStatusTipEvent*>(event)->tip());
            break;
        default:
            break;
    }

    return QWidget::event(event);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->modifiers().testFlag(Qt::AltModifier) && keyEvent->key() == 'O') {
            event->ignore();
            txtData->setFocus();
            return true;
        }
    }

    if ((watched == txt_fgcolor || watched == txt_bgcolor) && event->type() == QEvent::FocusOut) {
        // Exclude right-click context menu pop-up (Undo/Redo/Cut/Copy/Paste etc.)
        QFocusEvent *focusEvent = static_cast<QFocusEvent *>(event);
        if (focusEvent->reason() != Qt::PopupFocusReason) {
            if (watched == txt_fgcolor) {
                setColorTxtBtn(m_fgstr, txt_fgcolor, fgcolor);
            } else {
                setColorTxtBtn(m_bgstr, txt_bgcolor, bgcolor);
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void MainWindow::reset_colours()
{
    m_fgstr = fgDefault;
    m_bgstr = bgDefault;
    setColorTxtBtn(m_fgstr, txt_fgcolor, fgcolor);
    setColorTxtBtn(m_bgstr, txt_bgcolor, bgcolor);
    update_preview();
}

void MainWindow::reverse_colours()
{
    QString temp = m_fgstr;
    m_fgstr = m_bgstr;
    m_bgstr = temp;
    setColorTxtBtn(m_fgstr, txt_fgcolor, fgcolor);
    setColorTxtBtn(m_bgstr, txt_bgcolor, bgcolor);
    update_preview();
}

void MainWindow::setColorTxtBtn(const QString &colorStr, QLineEdit *txt, QToolButton* btn) {
    if (colorStr != txt->text()) {
        int cursorPos = txt->cursorPosition();
        txt->setText(colorStr);
        txt->setCursorPosition(cursorPos);
    }
    btn->setStyleSheet(QSL("QToolButton {background-color:") + str_to_qcolor(colorStr).name() + QSL(";}"));
}

bool MainWindow::save()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    QFileDialog save_dialog;
    QString pathname;
    QString suffix;
    QStringList suffixes;

    save_dialog.setAcceptMode(QFileDialog::AcceptSave);
    save_dialog.setWindowTitle(tr("Save Barcode Image"));
    save_dialog.setDirectory(settings.value(QSL("studio/default_dir"),
                QDir::toNativeSeparators(QDir::homePath())).toString());

    suffixes << QSL("eps") << QSL("gif") << QSL("svg") << QSL("bmp") << QSL("pcx") << QSL("emf") << QSL("tif");
    if (m_bc.bc.noPng()) {
        suffix = settings.value(QSL("studio/default_suffix"), QSL("gif")).toString();
        save_dialog.setNameFilter(tr(
            "Encapsulated PostScript (*.eps);;Graphics Interchange Format (*.gif)"
            ";;Scalable Vector Graphic (*.svg);;Windows Bitmap (*.bmp);;ZSoft PC Painter Image (*.pcx)"
            ";;Enhanced Metafile (*.emf);;Tagged Image File Format (*.tif)"));
    } else {
        suffix = settings.value(QSL("studio/default_suffix"), QSL("png")).toString();
        save_dialog.setNameFilter(tr(
            "Portable Network Graphic (*.png);;Encapsulated PostScript (*.eps);;Graphics Interchange Format (*.gif)"
            ";;Scalable Vector Graphic (*.svg);;Windows Bitmap (*.bmp);;ZSoft PC Painter Image (*.pcx)"
            ";;Enhanced Metafile (*.emf);;Tagged Image File Format (*.tif)"));
        suffixes << QSL("png");
    }

    if (QString::compare(suffix, QSL("png"), Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter(tr("Portable Network Graphic (*.png)"));
    else if (QString::compare(suffix, QSL("eps"), Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter(tr("Encapsulated PostScript (*.eps)"));
    else if (QString::compare(suffix, QSL("gif"), Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter(tr("Graphics Interchange Format (*.gif)"));
    else if (QString::compare(suffix, QSL("svg"), Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter(tr("Scalable Vector Graphic (*.svg)"));
    else if (QString::compare(suffix, QSL("bmp"), Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter(tr("Windows Bitmap (*.bmp)"));
    else if (QString::compare(suffix, QSL("pcx"), Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter(tr("ZSoft PC Painter Image (*.pcx)"));
    else if (QString::compare(suffix, QSL("emf"), Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter(tr("Enhanced Metafile (*.emf)"));
    else if (QString::compare(suffix, QSL("tif"), Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter(tr("Tagged Image File Format (*.tif)"));

    if (save_dialog.exec()) {
        pathname = save_dialog.selectedFiles().at(0);
        if ((pathname.lastIndexOf('.') == -1) || (pathname.lastIndexOf('.') < (pathname.length() - 5))) {
            suffix = save_dialog.selectedNameFilter();
            suffix = suffix.mid((suffix.lastIndexOf('.') + 1), 3);
            pathname.append('.');
            pathname.append(suffix);
        } else {
            suffix = pathname.right(pathname.length() - (pathname.lastIndexOf('.') + 1));
            if (!suffixes.contains(suffix, Qt::CaseInsensitive)) {
                /*: %1 is suffix of filename */
                QMessageBox::critical(this, tr("Save Error"), tr("Unknown output format \"%1\"").arg(suffix));
                return false;
            }
        }
    } else {
        return false;
    }

    if (m_bc.bc.save_to_file(pathname) == false) {
        if (m_bc.bc.getError() >= ZINT_ERROR) {
            QMessageBox::critical(this, tr("Save Error"), m_bc.bc.error_message());
            return false;
        }
        QMessageBox::warning(this, tr("Save Warning"), m_bc.bc.error_message());
    }

    QString nativePathname = QDir::toNativeSeparators(pathname);
    int lastSeparator = nativePathname.lastIndexOf(QDir::separator());
    QString dirname = nativePathname.mid(0, lastSeparator);
    if (dirname.isEmpty()) {
        /*: %1 is path saved to */
        statusBar->showMessage(tr("Saved as \"%1\"").arg(nativePathname), 0 /*No timeout*/);
    } else {
        QString filename = nativePathname.right(nativePathname.length() - (lastSeparator + 1));
        /*: %1 is base filename saved to, %2 is directory saved in */
        statusBar->showMessage(tr("Saved as \"%1\" in \"%2\"").arg(filename, dirname), 0 /*No timeout*/);
    }

    settings.setValue(QSL("studio/default_dir"), dirname);
    settings.setValue(QSL("studio/default_suffix"), suffix);
    return true;
}

void MainWindow::factory_reset()
{
    QMessageBox msgBox(QMessageBox::Question, tr("Factory Reset"),
        tr("This will clear all saved data and reset all settings for all symbologies to defaults."),
        QMessageBox::Yes | QMessageBox::No, this);
    msgBox.setInformativeText(tr("Do you wish to continue?"));
    msgBox.setDefaultButton(QMessageBox::Yes);
    if (msgBox.exec() == QMessageBox::No) {
        return;
    }

    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    settings.clear();

    int symbology = bstyle_items[bstyle->currentIndex()].symbology;

    load_settings(settings);

    load_sub_settings(settings, symbology);

    settings.sync();

    setColorTxtBtn(m_fgstr, txt_fgcolor, fgcolor);
    setColorTxtBtn(m_bgstr, txt_bgcolor, bgcolor);

    m_previewBgColor = QColor(0xF4, 0xF4, 0xF4);
    m_bc.setColor(m_previewBgColor);

    txtData->setFocus(Qt::OtherFocusReason);
    update_preview();
}

void MainWindow::about()
{
    QString zint_version = get_zint_version();
    QSettings settings;

    QMessageBox::about(this, tr("About Zint"),
        /*: %1 is Zint version, %2 is Qt version, %3 is QSettings file/registry path */
        tr(
#ifdef Q_OS_MACOS
            "<style>h2, p { font-size:11px; font-weight:normal; }</style>"
#endif
            "<h2>Zint Barcode Studio %1</h2>"
            "<p>A free barcode generator</p>"
            "<p>Instruction manual is available at the project homepage:<br>"
            "<a href=\"http://www.zint.org.uk\">http://www.zint.org.uk</a>.</p>"
            "<p>Copyright &copy; 2006-2024 Robin Stuart and others.<br>"
            "Qt backend by BogDan Vatra.<br>"
            "Released under GNU GPL 3.0 or later.</p>"
            "<p>Qt version %2<br>%3</p>"
            "<p>\"Mailmark\" is a Registered Trademark of Royal Mail.<br>"
            "\"QR Code\" is a Registered Trademark of Denso Corp.<br>"
            "\"Telepen\" is a Registered Trademark of SB Electronics.</p>"
            "<p>With thanks to Harald Oehlmann, Norbert Szab&oacute;, Robert Elliott, Milton Neal, "
                "Git Lost, Alonso Schaich, Andre Maute and many others at "
                "<a href=\"https://sourceforge.net/projects/zint/\">SourceForge</a>.</p>"
        ).arg(zint_version, QT_VERSION_STR, settings.fileName()));
}

void MainWindow::help()
{
    QDesktopServices::openUrl(QSL("https://zint.org.uk/manual")); // TODO: manual.md
}

void MainWindow::preview_bg()
{
    QColorDialog color_dialog(nullptr /*parent*/);
    color_dialog.setWindowTitle(tr("Set preview background colour"));
    color_dialog.setOptions(QColorDialog::DontUseNativeDialog);
    color_dialog.setCurrentColor(m_previewBgColor);
    color_dialog.restoreGeometry(m_previewbgcolor_geometry);
    connect(&color_dialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(previewbgcolor_changed(QColor)));
    if (color_dialog.exec() && color_dialog.selectedColor().isValid()) {
        m_previewBgColor = color_dialog.selectedColor();
    }
    m_previewbgcolor_geometry = color_dialog.saveGeometry();
    disconnect(&color_dialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(previewbgcolor_changed(QColor)));

    m_bc.setColor(m_previewBgColor);
    update_preview();
}

void MainWindow::previewbgcolor_changed(const QColor& color)
{
    if (color.isValid()) {
        m_bc.setColor(color);
        update_preview();
    }
}

QLineEdit *MainWindow::get_seg_textbox(int seg_no)
{
    static QLineEdit *textboxes[4] = {
        txtData, txtDataSeg1, txtDataSeg2, txtDataSeg3
    };
    return textboxes[seg_no];
}

QComboBox *MainWindow::get_seg_eci(int seg_no)
{
    static QComboBox *ecis[4] = {
        cmbECI, cmbECISeg1, cmbECISeg2, cmbECISeg3
    };
    return ecis[seg_no];
}

void MainWindow::clear_data()
{
    if (clear_data_eci_seg(0)) {
        update_preview();
    }
}

void MainWindow::clear_data_seg1()
{
    if (clear_data_eci_seg(1)) {
        update_preview();
    }
}

void MainWindow::clear_data_seg2()
{
    if (clear_data_eci_seg(2)) {
        update_preview();
    }
}

void MainWindow::clear_data_seg3()
{
    if (clear_data_eci_seg(3)) {
        update_preview();
    }
}

bool MainWindow::clear_data_eci_seg(int seg_no)
{
    QLineEdit *txt = get_seg_textbox(seg_no);
    QComboBox *cmb = get_seg_eci(seg_no);
    if (!txt->text().isEmpty() || cmb->currentIndex() != 0) {
        txt->clear();
        cmb->setCurrentIndex(0);
        txt->setFocus(Qt::OtherFocusReason);
        return true;
    }
    return false;
}

void MainWindow::clear_composite()
{
    if (!txtComposite->toPlainText().isEmpty()) {
        txtComposite->clear();
        update_preview();
    }
}

void MainWindow::open_data_dialog_seg(const int seg_no)
{
    if (seg_no < 0 || seg_no > 3) {
        return;
    }
    QLineEdit *seg_textbox = get_seg_textbox(seg_no);
    QString originalText = seg_textbox->text();
    bool originalChkEscape = chkEscape->isChecked();
    DataWindow dlg(originalText, originalChkEscape, seg_no);

#ifdef Q_OS_MACOS
    mac_hack_statusBars(&dlg);
#endif

    connect(&dlg, SIGNAL(dataChanged(QString,bool,int)), this, SLOT(on_dataChanged(QString,bool,int)));
    (void) dlg.exec();
    if (dlg.Valid) {
        const bool updated = originalText != dlg.DataOutput;
        seg_textbox->setText(dlg.DataOutput);
        if (updated) {
            static const QString updatedEscTxts[4] = {
                tr("Set \"Parse Escapes\", updated data"),
                tr("Set \"Parse Escapes\", updated segment 1 data"),
                tr("Set \"Parse Escapes\", updated segment 2 data"),
                tr("Set \"Parse Escapes\", updated segment 3 data"),
            };
            static const QString updatedTxts[4] = {
                tr("Updated data"),
                tr("Updated segment 1 data"),
                tr("Updated segment 2 data"),
                tr("Updated segment 3 data"),
            };
            if (dlg.Escaped && !originalChkEscape) {
                chkEscape->setChecked(true);
                statusBar->showMessage(updatedEscTxts[seg_no], tempMessageTimeout);
            } else {
                chkEscape->setChecked(originalChkEscape);
                statusBar->showMessage(updatedTxts[seg_no], tempMessageTimeout);
            }
        }
    } else {
        seg_textbox->setText(originalText); // Restore
        chkEscape->setChecked(originalChkEscape);
    }
    disconnect(&dlg, SIGNAL(dataChanged(QString,bool,int)), this, SLOT(on_dataChanged(QString,bool,int)));
}

void MainWindow::open_data_dialog()
{
    open_data_dialog_seg(0);
}

void MainWindow::open_data_dialog_seg1()
{
    open_data_dialog_seg(1);
}

void MainWindow::open_data_dialog_seg2()
{
    open_data_dialog_seg(2);
}

void MainWindow::open_data_dialog_seg3()
{
    open_data_dialog_seg(3);
}

void MainWindow::open_sequence_dialog()
{
    SequenceWindow dlg(&m_bc);
    (void) dlg.exec();
#ifdef _WIN32
    // Windows causes BarcodeItem to paint on closing dialog so need to re-paint with our (non-Export) values
    update_preview();
#endif
}

void MainWindow::zap()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif

    int symbology = bstyle_items[bstyle->currentIndex()].symbology;
    QString name = get_setting_name(symbology);
    settings.remove(QSL("studio/bc/%1").arg(name));
    settings.remove(QSL("studio/data"));
    settings.remove(QSL("studio/eci"));

    load_settings(settings);

    m_xdimdpVars.x_dim = 0.0f;
    m_xdimdpVars.x_dim_units = 0;
    m_xdimdpVars.set = 0;

    load_sub_settings(settings, symbology);

    setColorTxtBtn(m_fgstr, txt_fgcolor, fgcolor);
    setColorTxtBtn(m_bgstr, txt_bgcolor, bgcolor);

    txtData->setFocus(Qt::OtherFocusReason);
    update_preview();
}

void MainWindow::on_dataChanged(const QString& text, bool escaped, int seg_no)
{
    QLineEdit *seg_textbox = get_seg_textbox(seg_no);

    chkEscape->setChecked(escaped);
    seg_textbox->setText(text);
    update_preview();
}

void MainWindow::on_scaleChanged(double scale)
{
    spnScale->setValue(scale);
    size_msg_ui_set();
}

void MainWindow::open_cli_dialog()
{
    CLIWindow dlg(&m_bc, chkAutoHeight->isEnabled() && chkAutoHeight->isChecked(),
                    m_spnHeightPerRow && m_spnHeightPerRow->isEnabled() ? m_spnHeightPerRow->value() : 0.0,
                    &m_xdimdpVars);

#ifdef Q_OS_MACOS
    mac_hack_statusBars(&dlg);
#endif

    (void) dlg.exec();
}

void MainWindow::open_scale_dialog()
{
    double originalScale = spnScale->value();
    QString originalSizeMsg = lblSizeMsg->text();
    ScaleWindow dlg(&m_bc, &m_xdimdpVars, originalScale);
    m_scaleWindow = &dlg;
    connect(&dlg, SIGNAL(scaleChanged(double)), this, SLOT(on_scaleChanged(double)));
    (void) dlg.exec();
    disconnect(&dlg, SIGNAL(scaleChanged(double)), this, SLOT(on_scaleChanged(double)));
    if (dlg.Valid) {
        m_xdimdpVars = dlg.m_vars;
        update_preview();
    } else { // Restore
        spnScale->setValue(originalScale);
        lblSizeMsg->setText(originalSizeMsg);
    }
    m_scaleWindow = nullptr;
}

void MainWindow::fgcolor_clicked()
{
    color_clicked(m_fgstr, txt_fgcolor, fgcolor, tr("Set foreground colour"), m_fgcolor_geometry,
                    SLOT(fgcolor_changed(const QColor&)));
}

void MainWindow::bgcolor_clicked()
{
    color_clicked(m_bgstr, txt_bgcolor, bgcolor, tr("Set background colour"), m_bgcolor_geometry,
                    SLOT(bgcolor_changed(const QColor&)));
}

void MainWindow::color_clicked(QString &colorStr, QLineEdit *txt, QToolButton *btn, const QString& title,
                                QByteArray& geometry, const char *color_changed)
{
    QString original = colorStr;

    QColorDialog color_dialog(nullptr /*parent*/);
    color_dialog.setWindowTitle(title);
    color_dialog.setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);
    color_dialog.setCurrentColor(str_to_qcolor(colorStr));
    color_dialog.restoreGeometry(geometry);
    connect(&color_dialog, SIGNAL(currentColorChanged(QColor)), this, color_changed);

    if (color_dialog.exec() && color_dialog.selectedColor().isValid()) {
        colorStr = qcolor_to_str(color_dialog.selectedColor());
    } else {
        colorStr = original;
    }
    geometry = color_dialog.saveGeometry();
    disconnect(&color_dialog, SIGNAL(currentColorChanged(QColor)), this, color_changed);

    setColorTxtBtn(colorStr, txt, btn);
    update_preview();
}

void MainWindow::fgcolor_changed(const QColor& color)
{
    if (color.isValid()) {
        m_fgstr = qcolor_to_str(color);
        setColorTxtBtn(m_fgstr, txt_fgcolor, fgcolor);
        update_preview();
    }
}

void MainWindow::bgcolor_changed(const QColor& color)
{
    if (color.isValid()) {
        m_bgstr = qcolor_to_str(color);
        setColorTxtBtn(m_bgstr, txt_bgcolor, bgcolor);
        update_preview();
    }
}

void MainWindow::fgcolor_edited()
{
    color_edited(m_fgstr, txt_fgcolor, fgcolor);
}

void MainWindow::bgcolor_edited()
{
    color_edited(m_bgstr, txt_bgcolor, bgcolor);
}

void MainWindow::color_edited(QString &colorStr, QLineEdit *txt, QToolButton *btn)
{
    QString new_str = txt->text().trimmed();
    if (new_str.indexOf(*colorRE) != 0) {
        return;
    }
    colorStr = new_str;
    setColorTxtBtn(colorStr, txt, btn);
    update_preview();
}

void MainWindow::autoheight_ui_set()
{
    bool enabled = chkAutoHeight->isEnabled() && !chkAutoHeight->isChecked();
    lblHeight->setEnabled(enabled);
    heightb->setEnabled(enabled);

    if (m_lblHeightPerRow && m_spnHeightPerRow) {
        m_lblHeightPerRow->setEnabled(enabled);
        m_spnHeightPerRow->setEnabled(enabled);
        if (enabled && m_spnHeightPerRow->value()) {
            lblHeight->setEnabled(!enabled);
            heightb->setEnabled(!enabled);
            statusBar->showMessage(tr("Using \"Row Height\""), 0 /*No timeout*/);
        } else {
            statusBar->clearMessage();
        }
        if (m_btnHeightPerRowDisable) {
            m_btnHeightPerRowDisable->setEnabled(enabled && m_spnHeightPerRow->value());
        }
        if (m_btnHeightPerRowDefault) {
            if (enabled && m_spnHeightPerRow->value() == get_height_per_row_default()) {
                enabled = false;
            }
            m_btnHeightPerRowDefault->setEnabled(enabled);
        }
    }
}

void MainWindow::HRTShow_ui_set()
{
    bool enabled = chkHRTShow->isEnabled() && chkHRTShow->isChecked();
    lblFontSetting->setEnabled(enabled);
    cmbFontSetting->setEnabled(enabled);
    lblTextGap->setEnabled(enabled);
    spnTextGap->setEnabled(enabled);
    chkEmbedVectorFont->setEnabled(enabled);
    text_gap_ui_set();
    upcean_no_quiet_zones_ui_set();
    upcae_no_quiet_zones_ui_set();
}

void MainWindow::text_gap_ui_set()
{
    bool hrtEnabled = chkHRTShow->isEnabled() && chkHRTShow->isChecked();
    btnClearTextGap->setEnabled(hrtEnabled && spnTextGap->value() != 1.0);
}

void MainWindow::dotty_ui_set()
{
    int symbology = bstyle_items[bstyle->currentIndex()].symbology;

    if (symbology == BARCODE_DOTCODE) {
        chkDotty->setEnabled(false);
        lblDotSize->setEnabled(true);
        spnDotSize->setEnabled(true);
    } else {
        bool enabled = chkDotty->isEnabled() && chkDotty->isChecked();
        lblDotSize->setEnabled(enabled);
        spnDotSize->setEnabled(enabled);
    }
}

void MainWindow::codeone_ui_set()
{
    int symbology = bstyle_items[bstyle->currentIndex()].symbology;
    if (symbology != BARCODE_CODEONE)
        return;

    QGroupBox *groupBox = m_optionWidget->findChild<QGroupBox*>(QSL("groupBoxC1StructApp"));
    if (groupBox) {
        bool enabled = get_cmb_index(QSL("cmbC1Size")) != 9; // Not Version S
        groupBox->setEnabled(enabled);
    }
}

void MainWindow::upcean_no_quiet_zones_ui_set()
{
    int symbology = bstyle_items[bstyle->currentIndex()].symbology;
    if (!m_bc.bc.isEANUPC(symbology) || symbology == BARCODE_UPCA || symbology == BARCODE_UPCA_CHK
            || symbology == BARCODE_UPCA_CC)
        return;

    bool showHRT = chkHRTShow->isEnabled() && chkHRTShow->isChecked();
    QCheckBox *noQZs, *guardWS;
    noQZs = m_optionWidget ? m_optionWidget->findChild<QCheckBox*>(QSL("chkUPCEANNoQuietZones")) : nullptr;
    guardWS = m_optionWidget ? m_optionWidget->findChild<QCheckBox*>(QSL("chkUPCEANGuardWhitespace")) : nullptr;

    if (noQZs && guardWS) {
        guardWS->setEnabled(!noQZs->isChecked() && showHRT);
    }
}

void MainWindow::upcae_no_quiet_zones_ui_set()
{
    const int symbology = bstyle_items[bstyle->currentIndex()].symbology;
    const bool is_upca = symbology == BARCODE_UPCA || symbology == BARCODE_UPCA_CHK || symbology == BARCODE_UPCA_CC;
    const bool is_upce = symbology == BARCODE_UPCE || symbology == BARCODE_UPCE_CHK || symbology == BARCODE_UPCE_CC;
    if (!is_upca && !is_upce)
        return;

    bool showHRT = chkHRTShow->isEnabled() && chkHRTShow->isChecked();
    QCheckBox *noQZs, *guardWS;
    if (is_upca) {
        noQZs = m_optionWidget ? m_optionWidget->findChild<QCheckBox*>(QSL("chkUPCANoQuietZones")) : nullptr;
        guardWS = m_optionWidget ? m_optionWidget->findChild<QCheckBox*>(QSL("chkUPCAGuardWhitespace")) : nullptr;
    } else {
        noQZs = m_optionWidget ? m_optionWidget->findChild<QCheckBox*>(QSL("chkUPCEANNoQuietZones")) : nullptr;
        guardWS = m_optionWidget ? m_optionWidget->findChild<QCheckBox*>(QSL("chkUPCEANGuardWhitespace")) : nullptr;
    }

    if (noQZs && guardWS) {
        if (have_addon()) {
            noQZs->setEnabled(true);
            guardWS->setEnabled(!noQZs->isChecked() && showHRT);
        } else {
            noQZs->setEnabled(!showHRT);
            guardWS->setEnabled(false);
        }
    }
}

void MainWindow::structapp_ui_set()
{
    int symbology = bstyle_items[bstyle->currentIndex()].symbology;
    QString name;
    bool enabled = false;
    QWidget *widgetCount = nullptr, *widgetIndex = nullptr;
    QLabel *lblID2 = nullptr;
    QWidget *widgetID = nullptr, *widgetID2 = nullptr;

    if (symbology == BARCODE_AZTEC) {
        name = QSL("Aztec");
        widgetID = get_widget(QSL("txt") + name + QSL("StructAppID"));
    } else if (symbology == BARCODE_CODEONE) {
        name = QSL("C1");
        QSpinBox *spnCount = m_optionWidget->findChild<QSpinBox*>(QSL("spn") + name + QSL("StructAppCount"));
        enabled = spnCount ? spnCount->value() > 1 : false;
        widgetCount = spnCount;
        widgetIndex = get_widget(QSL("spn") + name + QSL("StructAppIndex"));
    } else if (symbology == BARCODE_DATAMATRIX) {
        name = QSL("DM");
        widgetID = get_widget(QSL("spn") + name + QSL("StructAppID"));
        widgetID2 = get_widget(QSL("spn") + name + QSL("StructAppID2"));
    } else if (symbology == BARCODE_DOTCODE) {
        name = QSL("Dot");
    } else if (symbology == BARCODE_MAXICODE) {
        name = QSL("Maxi");
    } else if (symbology == BARCODE_PDF417 || symbology == BARCODE_MICROPDF417) {
        name = symbology == BARCODE_PDF417 ? QSL("PDF") : QSL("MPDF");
        QSpinBox *spnCount = m_optionWidget->findChild<QSpinBox*>(QSL("spn") + name + QSL("StructAppCount"));
        enabled = spnCount ? spnCount->value() > 1 : false;
        widgetCount = spnCount;
        widgetIndex = get_widget(QSL("spn") + name + QSL("StructAppIndex"));
        widgetID = get_widget(QSL("txt") + name + QSL("StructAppID"));
    } else if (symbology == BARCODE_QRCODE) {
        name = QSL("QR");
        widgetID = get_widget(QSL("spn") + name + QSL("StructAppID"));
    } else if (symbology == BARCODE_GRIDMATRIX) {
        name = QSL("Grid");
        widgetID = get_widget(QSL("spn") + name + QSL("StructAppID"));
    } else if (symbology == BARCODE_ULTRA) {
        name = QSL("Ultra");
        widgetID = get_widget(QSL("spn") + name + QSL("StructAppID"));
    }
    if (!name.isEmpty()) {
        QLabel *lblIndex = m_optionWidget->findChild<QLabel*>(QSL("lbl") + name + QSL("StructAppIndex"));
        if (!widgetCount) {
            QComboBox *cmbCount = m_optionWidget->findChild<QComboBox*>(QSL("cmb") + name + QSL("StructAppCount"));
            enabled = cmbCount ? cmbCount->currentIndex() != 0 : false;
            widgetCount = cmbCount;
        }
        if (!widgetIndex) {
            widgetIndex = get_widget(QSL("cmb") + name + QSL("StructAppIndex"));
        }
        if (lblIndex && widgetCount && widgetIndex) {
            lblIndex->setEnabled(enabled);
            widgetIndex->setEnabled(enabled);
            QLabel *lblID = m_optionWidget->findChild<QLabel*>(QSL("lbl") + name + QSL("StructAppID"));
            if (lblID) {
                lblID->setEnabled(enabled);
                if (lblID2) {
                    lblID2->setEnabled(enabled);
                }
            }
            if (widgetID) {
                widgetID->setEnabled(enabled);
                if (widgetID2) {
                    widgetID2->setEnabled(enabled);
                }
            }
        }
    }
}

void MainWindow::clear_text_gap()
{
    spnTextGap->setValue(1.0);
    spnTextGap->setFocus();
    update_preview();
}

void MainWindow::on_encoded()
{
    // Protect against encode in Sequence Export popup dialog
    QWidget *activeModalWidget = QApplication::activeModalWidget();
    if (activeModalWidget != nullptr && activeModalWidget->objectName() == "ExportDialog") {
        return;
    }
    enableActions();
    errtxtBar_set();

    if (!chkAutoHeight->isEnabled() || chkAutoHeight->isChecked() || !heightb->isEnabled()) {
        /* setValue() rounds up/down to precision (decimals 3), we want round up only */
        float height = (float) (ceil(m_bc.bc.height() * 1000.0f) / 1000.0f);
        heightb->setValue(height); // This can cause a double-encode unfortunately
    }
    size_msg_ui_set();

    if (m_optionWidget) {
        automatic_info_set();
    }
}

void MainWindow::on_errored()
{
    // Protect against error in Sequence Export popup dialog
    QWidget *activeModalWidget = QApplication::activeModalWidget();
    if (activeModalWidget != nullptr && activeModalWidget->objectName() == "ExportDialog") {
        return;
    }
    enableActions();
    errtxtBar_set();
    size_msg_ui_set();
    if (m_optionWidget) {
        automatic_info_set();
    }
}

void MainWindow::filter_symbologies()
{
    // `simplified()` trims and reduces inner whitespace to a single space - nice!
    QString filter = filter_bstyle->text().simplified();
    QListView *lview = qobject_cast<QListView *>(bstyle->view());
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(bstyle->model());
    QStandardItem *item;

    if (!lview || !model) {
        return;
    }

    /* QString::split() only introduced Qt 5.14, so too new for us to use */
    QStringList filter_list;
    if (!filter.isEmpty()) {
        int i, j;
        for (i = 0; (j = filter.indexOf(' ', i)) != -1; i = j + 1) {
            filter_list << filter.mid(i, j - i);
        }
        filter_list << filter.mid(i);
    }
    int filter_cnt = filter_list.size();
    int cnt = (int) (sizeof(bstyle_items) / sizeof(bstyle_items[0]));

    if (filter_cnt) {
        for (int i = 0; i < cnt; i++) {
            bool hidden = lview->isRowHidden(i);
            bool hide = true;
            for (int j = 0; j < filter_cnt; j++) {
                if (bstyle->itemText(i).contains(filter_list[j], Qt::CaseInsensitive)) {
                    hide = false;
                    break;
                }
            }
            if ((hide && !hidden) || (!hide && hidden)) {
                // https://stackoverflow.com/questions/25172220
                // /how-to-hide-qcombobox-items-instead-of-clearing-them-out
                item = model->item(i);
                item->setFlags(hide ? item->flags() & ~Qt::ItemIsEnabled : item->flags() | Qt::ItemIsEnabled);
                lview->setRowHidden(i, hide);
            }
        }
    } else {
        for (int i = 0; i < cnt; i++) {
            if (lview->isRowHidden(i)) {
                item = model->item(i);
                item->setFlags(item->flags() | Qt::ItemIsEnabled);
                lview->setRowHidden(i, false);
            }
        }
    }
}

void MainWindow::size_msg_ui_set()
{
    if (m_bc.bc.getError() < ZINT_ERROR) {
        float scale = (float) spnScale->value();
        struct Zint::QZintXdimDpVars *vars = m_scaleWindow ? &m_scaleWindow->m_vars : &m_xdimdpVars;
        if (vars->x_dim == 0.0) {
            vars->x_dim_units = 0;
            vars->x_dim = std::min(m_bc.bc.getXdimDpFromScale(scale, get_dpmm(vars), getFileType(vars)), 10.0f);
        } else {
            // Scale trumps stored X-dimension
            double x_dim_mm = vars->x_dim_units == 1 ? vars->x_dim * 25.4 : vars->x_dim;
            if (m_bc.bc.getScaleFromXdimDp((float) x_dim_mm, get_dpmm(vars), getFileType(vars)) != scale) {
                x_dim_mm = std::min(m_bc.bc.getXdimDpFromScale(scale, get_dpmm(vars), getFileType(vars)), 10.0f);
                vars->x_dim = vars->x_dim_units == 1 ? x_dim_mm / 25.4 : x_dim_mm;
            }
        }
        float width_x_dim, height_x_dim;
        if (m_bc.bc.getWidthHeightXdim((float) vars->x_dim, width_x_dim, height_x_dim)) {
            const char *fmt = vars->x_dim_units == 1 ? "%.3f x %.3f in @ %d %s (%s)" : "%.2f x %.2f mm @ %d %s (%s)";
            const char *resolution_units_str = vars->resolution_units == 1 ? "dpi" : "dpmm";
            lblSizeMsg->setText(QString::asprintf(fmt, width_x_dim, height_x_dim, vars->resolution,
                                resolution_units_str, getFileType(vars, true /*msg*/)));
        } else {
            lblSizeMsg->clear();
        }
    } else {
        lblSizeMsg->clear();
    }
    if (m_scaleWindow) {
        m_scaleWindow->size_msg_ui_set();
    }
}

void MainWindow::change_cmyk()
{
    /* This value is only used when printing (saving) to file */
    m_bc.bc.setCMYK(chkCMYK->isChecked());
}

void MainWindow::quit_now()
{
    close();
}

void MainWindow::menu()
{
    QSize size = m_menu->sizeHint();
    m_menu->exec(btnMenu->mapToGlobal(QPoint(0, -size.height())));
}

void MainWindow::copy_to_clipboard_bmp()
{
    copy_to_clipboard(QSL(".zint.bmp"), QSL("BMP"));
}

void MainWindow::copy_to_clipboard_emf()
{
    copy_to_clipboard(QSL(".zint.emf"), QSL("EMF"), "image/x-emf");
}

void MainWindow::copy_to_clipboard_eps()
{
    // TODO: try other possibles application/eps, application/x-eps, image/eps, image/x-eps
    copy_to_clipboard(QSL(".zint.eps"), QSL("EPS"), "application/postscript");
}

void MainWindow::copy_to_clipboard_gif()
{
    copy_to_clipboard(QSL(".zint.gif"), QSL("GIF"));
}

void MainWindow::copy_to_clipboard_pcx()
{
    // TODO: try other mime types in various apps
    copy_to_clipboard(QSL(".zint.pcx"), QSL("PCX"), "image/x-pcx");
}

void MainWindow::copy_to_clipboard_png()
{
    if (!m_bc.bc.noPng()) {
        copy_to_clipboard(QSL(".zint.png"), QSL("PNG"));
    }
}

void MainWindow::copy_to_clipboard_svg()
{
    copy_to_clipboard(QSL(".zint.svg"), QSL("SVG"));
}

void MainWindow::copy_to_clipboard_tif()
{
    copy_to_clipboard(QSL(".zint.tif"), QSL("TIF"));
}

void MainWindow::copy_to_clipboard_errtxt()
{
    if (m_bc.bc.hasErrors()) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        QMimeData *mdata = new QMimeData;
        mdata->setText(m_bc.bc.lastError());
        clipboard->setMimeData(mdata, QClipboard::Clipboard);
        statusBar->showMessage(tr("Copied message to clipboard"), 0 /*No timeout*/);
    }
}

void MainWindow::height_per_row_disable()
{
    if (m_spnHeightPerRow) {
        m_spnHeightPerRow->setValue(0.0);
    }
}

double MainWindow::get_height_per_row_default()
{
    const QString &name = m_btnHeightPerRowDefault->objectName();
    double val = 0.0;
    if (name == QSL("btnPDFHeightPerRowDefault")) {
        val = 3.0;
    } else if (name == QSL("btnMPDFHeightPerRowDefault")) {
        val = 2.0;
    } else if (name == QSL("btnC16kHeightPerRowDefault")) {
        if (chkCompliantHeight->isEnabled() && chkCompliantHeight->isChecked()) {
            const int rows = m_bc.bc.encodedRows();
            val = 10.0 + (double)((rows - 1) * (get_cmb_index(QSL("cmbC16kRowSepHeight")) + 1)) / rows;
        } else {
            val = 10.0;
        }
    } else if (name == QSL("btnCbfHeightPerRowDefault")) {
        // Depends on no. of data cols
        const int cols = (m_bc.bc.encodedWidth() - 57) / 11; // 57 = 4 * 11 (start/subset/checks) + 13 (stop char)
        val = 0.55 * cols + 3;
        if (val < 10.0) {
            val = 10.0;
        }
    } else if (name == QSL("btnC49HeightPerRowDefault")) {
        if (chkCompliantHeight->isEnabled() && chkCompliantHeight->isChecked()) {
            const int rows = m_bc.bc.encodedRows();
            val = 10.0 + (double)((rows - 1) * (get_cmb_index(QSL("cmbC49RowSepHeight")) + 1)) / rows;
        } else {
            val = 10.0;
        }
    } else if (name == QSL("btnDBESHeightPerRowDefault")) {
        val = 34.0;
    }
    return val;
}

void MainWindow::height_per_row_default()
{
    if (m_spnHeightPerRow && m_btnHeightPerRowDefault) {
        double val = get_height_per_row_default();
        if (val) {
            m_spnHeightPerRow->setValue(val);
        }
    }
}

bool MainWindow::have_addon()
{
    const QRegularExpression addonRE(QSL("^[0-9X]+[+][0-9]+$"));
    return txtData->text().contains(addonRE);
}

void MainWindow::guard_default_upcean()
{
    guard_default(QSL("spnUPCEANGuardDescent"));
}

void MainWindow::guard_default_upca()
{
    guard_default(QSL("spnUPCAGuardDescent"));
}

void MainWindow::view_context_menu(const QPoint &pos)
{
    QMenu menu(tr("View Menu"), view);

    if (m_bc.bc.getError() >= ZINT_ERROR) {
        menu.addAction(m_copyErrtxtAct);

        menu.exec(get_context_menu_pos(pos, view));
    } else {
        menu.addAction(m_copyBMPAct);
        menu.addAction(m_copyEMFAct);
#ifdef MAINWINDOW_COPY_EPS
        menu.addAction(m_copyEPSAct);
#endif
        menu.addAction(m_copyGIFAct);
#ifdef MAINWINDOW_COPY_PCX
        menu.addAction(m_copyPCXAct);
#endif
        if (!m_bc.bc.noPng()) {
            menu.addAction(m_copyPNGAct);
        }
        menu.addAction(m_copySVGAct);
        menu.addAction(m_copyTIFAct);
        menu.addSeparator();
        menu.addAction(m_openCLIAct);
        menu.addSeparator();
        menu.addAction(m_saveAsAct);
        menu.addSeparator();
        menu.addAction(m_previewBgColorAct);

        menu.exec(get_context_menu_pos(pos, view));
    }
}

void MainWindow::errtxtBar_context_menu(const QPoint &pos)
{
    QMenu menu(tr("Message Menu"), errtxtBar);

    menu.addAction(m_copyErrtxtAct);

    menu.exec(get_context_menu_pos(pos, errtxtBar));
}

void MainWindow::change_options()
{
    QUiLoader uiload;
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif

    bool initial_load = m_symbology == 0;
    int original_tab_count = tabMain->count();
    int original_tab_index = tabMain->currentIndex();
    int symbology = bstyle_items[bstyle->currentIndex()].symbology;

    if (m_symbology) {
        save_sub_settings(settings, m_symbology);
    }
    statusBar->clearMessage();

    grpSpecific->hide();
    if (m_optionWidget) {
        if (tabMain->count() == 3) {
            tabMain->removeTab(1);
        } else {
            vLayoutSpecific->removeWidget(m_optionWidget);
        }
        delete m_optionWidget;
        m_optionWidget = nullptr;
    }
    chkComposite->setText(tr("Add &2D Component"));
    combobox_item_enabled(cmbCompType, 3, false); // CC-C
    btype->setItemText(0, tr("No border"));
    combobox_item_enabled(cmbFontSetting, 1, true); // Reset bold options
    combobox_item_enabled(cmbFontSetting, 3, true);
    m_lblHeightPerRow = nullptr;
    m_spnHeightPerRow = nullptr;
    m_btnHeightPerRowDisable = nullptr;
    m_btnHeightPerRowDefault = nullptr;

    setColorTxtBtn(m_fgstr, txt_fgcolor, fgcolor);
    setColorTxtBtn(m_bgstr, txt_bgcolor, bgcolor);

    m_xdimdpVars.x_dim = 0.0f;
    m_xdimdpVars.x_dim_units = 0;
    m_xdimdpVars.set = 0;

    if (symbology == BARCODE_CODE128) {
        QFile file(QSL(":/grpC128.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        tabMain->insertTab(1, m_optionWidget, tr("Cod&e 128"));
        chkComposite->setText(tr("Add &2D Component (GS1-128 only)"));
        combobox_item_enabled(cmbCompType, 3, true); // CC-C
        set_smaller_font(QSL("noteC128CompositeEAN"));
        connect(get_widget(QSL("radC128Stand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radC128CSup")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radC128EAN")), SIGNAL(toggled(bool)), SLOT(composite_ean_check()));
        connect(get_widget(QSL("radC128EAN")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radC128HIBC")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radC128ExtraEsc")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_PDF417) {
        QFile file(QSL(":/grpPDF417.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        structapp_ui_set();
        tabMain->insertTab(1, m_optionWidget, tr("PDF41&7"));
        connect(get_widget(QSL("cmbPDFECC")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbPDFCols")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbPDFRows")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        m_lblHeightPerRow = m_optionWidget->findChild<QLabel*>(QSL("lblPDFHeightPerRow"));
        m_spnHeightPerRow = m_optionWidget->findChild<QDoubleSpinBox*>(QSL("spnPDFHeightPerRow"));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(autoheight_ui_set()));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(update_preview()));
        m_btnHeightPerRowDisable = m_optionWidget->findChild<QPushButton*>(QSL("btnPDFHeightPerRowDisable"));
        m_btnHeightPerRowDefault = m_optionWidget->findChild<QPushButton*>(QSL("btnPDFHeightPerRowDefault"));
        connect(m_btnHeightPerRowDisable, SIGNAL(clicked(bool)), SLOT(height_per_row_disable()));
        connect(m_btnHeightPerRowDefault, SIGNAL(clicked(bool)), SLOT(height_per_row_default()));
        connect(get_widget(QSL("radPDFTruncated")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radPDFStand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radPDFHIBC")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkPDFFast")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("spnPDFStructAppCount")), SIGNAL(valueChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("spnPDFStructAppCount")), SIGNAL(valueChanged(int)), SLOT(structapp_ui_set()));
        connect(get_widget(QSL("spnPDFStructAppIndex")), SIGNAL(valueChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("txtPDFStructAppID")), SIGNAL(textChanged(QString)), SLOT(update_preview()));

    } else if (symbology == BARCODE_MICROPDF417) {
        QFile file(QSL(":/grpMicroPDF.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        structapp_ui_set();
        tabMain->insertTab(1, m_optionWidget, tr("Micro PDF41&7"));
        connect(get_widget(QSL("cmbMPDFCols")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        m_lblHeightPerRow = m_optionWidget->findChild<QLabel*>(QSL("lblMPDFHeightPerRow"));
        m_spnHeightPerRow = m_optionWidget->findChild<QDoubleSpinBox*>(QSL("spnMPDFHeightPerRow"));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(autoheight_ui_set()));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(update_preview()));
        m_btnHeightPerRowDisable = m_optionWidget->findChild<QPushButton*>(QSL("btnMPDFHeightPerRowDisable"));
        m_btnHeightPerRowDefault = m_optionWidget->findChild<QPushButton*>(QSL("btnMPDFHeightPerRowDefault"));
        connect(m_btnHeightPerRowDisable, SIGNAL(clicked(bool)), SLOT(height_per_row_disable()));
        connect(m_btnHeightPerRowDefault, SIGNAL(clicked(bool)), SLOT(height_per_row_default()));
        connect(get_widget(QSL("radMPDFStand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkMPDFFast")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("spnMPDFStructAppCount")), SIGNAL(valueChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("spnMPDFStructAppCount")), SIGNAL(valueChanged(int)), SLOT(structapp_ui_set()));
        connect(get_widget(QSL("spnMPDFStructAppIndex")), SIGNAL(valueChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("txtMPDFStructAppID")), SIGNAL(textChanged(QString)), SLOT(update_preview()));

    } else if (symbology == BARCODE_DOTCODE) {
        QFile file(QSL(":/grpDotCode.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        structapp_ui_set();
        tabMain->insertTab(1, m_optionWidget, tr("DotCod&e"));
        connect(get_widget(QSL("cmbDotCols")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbDotMask")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("radDotStand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radDotGS1")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbDotStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbDotStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(structapp_ui_set()));
        connect(get_widget(QSL("cmbDotStructAppIndex")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));

    } else if (symbology == BARCODE_AZTEC) {
        QFile file(QSL(":/grpAztec.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        structapp_ui_set();
        tabMain->insertTab(1, m_optionWidget, tr("Aztec Cod&e"));
        connect(get_widget(QSL("radAztecAuto")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radAztecSize")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radAztecECC")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbAztecSize")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbAztecECC")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("radAztecStand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radAztecGS1")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radAztecHIBC")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbAztecStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbAztecStructAppCount")), SIGNAL(currentIndexChanged(int)),
                SLOT(structapp_ui_set()));
        connect(get_widget(QSL("cmbAztecStructAppIndex")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("txtAztecStructAppID")), SIGNAL(textChanged(QString)), SLOT(update_preview()));

    } else if (symbology == BARCODE_MSI_PLESSEY) {
        QFile file(QSL(":/grpMSICheck.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        vLayoutSpecific->addWidget(m_optionWidget);
        grpSpecific->show();
        connect(get_widget(QSL("cmbMSICheck")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbMSICheck")), SIGNAL(currentIndexChanged(int)), SLOT(msi_plessey_ui_set()));
        connect(get_widget(QSL("chkMSICheckText")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_CODE11) {
        QFile file(QSL(":/grpC11.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        vLayoutSpecific->addWidget(m_optionWidget);
        grpSpecific->show();
        connect(get_widget(QSL("radC11TwoCheckDigits")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radC11OneCheckDigit")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radC11NoCheckDigits")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_C25STANDARD || symbology == BARCODE_C25INTER || symbology == BARCODE_C25IATA
            || symbology == BARCODE_C25LOGIC || symbology == BARCODE_C25IND) {
        QFile file(QSL(":/grpC25.ui"));
        if (file.open(QIODevice::ReadOnly)) {
            m_optionWidget = uiload.load(&file);
            file.close();
            load_sub_settings(settings, symbology);
            vLayoutSpecific->addWidget(m_optionWidget);
            grpSpecific->show();
            connect(get_widget(QSL("radC25Stand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
            connect(get_widget(QSL("radC25Check")), SIGNAL(toggled(bool)), SLOT(update_preview()));
            connect(get_widget(QSL("radC25CheckHide")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        }

    } else if (symbology == BARCODE_CODE39 || symbology == BARCODE_EXCODE39 || symbology == BARCODE_LOGMARS) {
        QFile file(QSL(":/grpC39.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        vLayoutSpecific->addWidget(m_optionWidget);
        grpSpecific->show();
        connect(get_widget(QSL("radC39Stand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radC39Check")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radC39CheckHide")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        QRadioButton *radC39HIBC = m_optionWidget->findChild<QRadioButton*>(QSL("radC39HIBC"));
        if (symbology == BARCODE_EXCODE39 || symbology == BARCODE_LOGMARS) {
            if (radC39HIBC->isChecked()) {
                radC39HIBC->setChecked(false);
                m_optionWidget->findChild<QRadioButton*>(QSL("radC39Stand"))->setChecked(true);
            }
            radC39HIBC->setEnabled(false);
            radC39HIBC->hide();
        } else {
            connect(get_widget(QSL("radC39HIBC")), SIGNAL(toggled(bool)), SLOT(update_preview()));
            radC39HIBC->setEnabled(true);
            radC39HIBC->show();
        }

    } else if (symbology == BARCODE_CODE16K) {
        QFile file(QSL(":/grpC16k.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        tabMain->insertTab(1, m_optionWidget, tr("Cod&e 16K"));
        btype->setItemText(0, tr("Default (bind)"));
        connect(get_widget(QSL("cmbC16kRows")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        m_lblHeightPerRow = m_optionWidget->findChild<QLabel*>(QSL("lblC16kHeightPerRow"));
        m_spnHeightPerRow = m_optionWidget->findChild<QDoubleSpinBox*>(QSL("spnC16kHeightPerRow"));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(autoheight_ui_set()));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(update_preview()));
        m_btnHeightPerRowDisable = m_optionWidget->findChild<QPushButton*>(QSL("btnC16kHeightPerRowDisable"));
        m_btnHeightPerRowDefault = m_optionWidget->findChild<QPushButton*>(QSL("btnC16kHeightPerRowDefault"));
        connect(m_btnHeightPerRowDisable, SIGNAL(clicked(bool)), SLOT(height_per_row_disable()));
        connect(m_btnHeightPerRowDefault, SIGNAL(clicked(bool)), SLOT(height_per_row_default()));
        connect(get_widget(QSL("cmbC16kRowSepHeight")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("radC16kStand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkC16kNoQuietZones")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_CODABAR) {
        QFile file(QSL(":/grpCodabar.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        vLayoutSpecific->addWidget(m_optionWidget);
        grpSpecific->show();
        connect(get_widget(QSL("radCodabarStand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radCodabarCheckHide")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radCodabarCheck")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_CODABLOCKF) {
        QFile file (QSL(":/grpCodablockF.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        tabMain->insertTab(1, m_optionWidget, tr("Codablock&-F"));
        btype->setItemText(0, tr("Default (bind)"));
        connect(get_widget(QSL("cmbCbfWidth")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbCbfHeight")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        m_lblHeightPerRow = m_optionWidget->findChild<QLabel*>(QSL("lblCbfHeightPerRow"));
        m_spnHeightPerRow = m_optionWidget->findChild<QDoubleSpinBox*>(QSL("spnCbfHeightPerRow"));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(autoheight_ui_set()));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(update_preview()));
        m_btnHeightPerRowDisable = m_optionWidget->findChild<QPushButton*>(QSL("btnCbfHeightPerRowDisable"));
        m_btnHeightPerRowDefault = m_optionWidget->findChild<QPushButton*>(QSL("btnCbfHeightPerRowDefault"));
        connect(m_btnHeightPerRowDisable, SIGNAL(clicked(bool)), SLOT(height_per_row_disable()));
        connect(m_btnHeightPerRowDefault, SIGNAL(clicked(bool)), SLOT(height_per_row_default()));
        connect(get_widget(QSL("cmbCbfRowSepHeight")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("radCbfStand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radCbfHIBC")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkCbfNoQuietZones")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_DAFT) {
        QFile file(QSL(":/grpDAFT.ui"));
        if (file.open(QIODevice::ReadOnly)) {
            m_optionWidget = uiload.load(&file);
            file.close();
            load_sub_settings(settings, symbology);
            vLayoutSpecific->addWidget(m_optionWidget);
            grpSpecific->show();
            set_smaller_font(QSL("noteDAFTTrackerRatios"));
            connect(get_widget(QSL("spnDAFTTrackerRatio")), SIGNAL(valueChanged(double)), SLOT(daft_ui_set()));
            connect(get_widget(QSL("spnDAFTTrackerRatio")), SIGNAL(valueChanged(double)), SLOT(update_preview()));
            connect(get_widget(QSL("btnDAFTTrackerDefault")), SIGNAL(clicked(bool)), SLOT(daft_tracker_default()));
            daft_ui_set();
        }

    } else if (symbology == BARCODE_DPD) {
        btype->setItemText(0, tr("Default (bind top, 3X width)"));
        QFile file(QSL(":/grpDPD.ui"));
        if (file.open(QIODevice::ReadOnly)) {
            m_optionWidget = uiload.load(&file);
            file.close();
            load_sub_settings(settings, symbology);
            vLayoutSpecific->addWidget(m_optionWidget);
            grpSpecific->show();
            connect(get_widget(QSL("chkDPDRelabel")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        }

    } else if (symbology == BARCODE_DATAMATRIX) {
        QFile file(QSL(":/grpDM.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        structapp_ui_set();
        tabMain->insertTab(1, m_optionWidget, tr("D&ata Matrix"));
        connect(get_widget(QSL("radDM200Stand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radDM200GS1")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radDM200HIBC")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbDM200Size")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("chkDMRectangle")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkDMRE")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkDMGSSep")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkDMISO144")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkDMFast")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbDMStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbDMStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(structapp_ui_set()));
        connect(get_widget(QSL("cmbDMStructAppIndex")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("spnDMStructAppID")), SIGNAL(valueChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("spnDMStructAppID2")), SIGNAL(valueChanged(int)), SLOT(update_preview()));

    } else if (symbology == BARCODE_MAILMARK_2D) {
        QFile file(QSL(":/grpMailmark2D.ui"));
        if (file.open(QIODevice::ReadOnly)) {
            m_optionWidget = uiload.load(&file);
            file.close();
            load_sub_settings(settings, symbology);
            vLayoutSpecific->addWidget(m_optionWidget);
            grpSpecific->show();
            connect(get_widget(QSL("cmbMailmark2DSize")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
            connect(get_widget(QSL("chkMailmark2DRectangle")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        }

    } else if (symbology == BARCODE_ITF14) {
        btype->setItemText(0, tr("Default (box, 5X width)"));
        QFile file(QSL(":/grpITF14.ui"));
        if (file.open(QIODevice::ReadOnly)) {
            m_optionWidget = uiload.load(&file);
            file.close();
            load_sub_settings(settings, symbology);
            vLayoutSpecific->addWidget(m_optionWidget);
            grpSpecific->show();
            connect(get_widget(QSL("chkITF14NoQuietZones")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        }

    } else if (symbology == BARCODE_PZN) {
        QFile file(QSL(":/grpPZN.ui"));
        if (file.open(QIODevice::ReadOnly)) {
            m_optionWidget = uiload.load(&file);
            file.close();
            load_sub_settings(settings, symbology);
            vLayoutSpecific->addWidget(m_optionWidget);
            grpSpecific->show();
            connect(get_widget(QSL("chkPZN7")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        }

    } else if (symbology == BARCODE_QRCODE) {
        QFile file(QSL(":/grpQR.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        structapp_ui_set();
        tabMain->insertTab(1, m_optionWidget, tr("QR Cod&e"));
        connect(get_widget(QSL("cmbQRSize")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbQRECC")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbQRMask")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("radQRStand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radQRGS1")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radQRHIBC")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkQRFullMultibyte")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkQRFast")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbQRStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbQRStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(structapp_ui_set()));
        connect(get_widget(QSL("cmbQRStructAppIndex")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("spnQRStructAppID")), SIGNAL(valueChanged(int)), SLOT(update_preview()));

    } else if (symbology == BARCODE_UPNQR) {
        QFile file(QSL(":/grpUPNQR.ui"));
        if (file.open(QIODevice::ReadOnly)) {
            m_optionWidget = uiload.load(&file);
            file.close();
            load_sub_settings(settings, symbology);
            vLayoutSpecific->addWidget(m_optionWidget);
            grpSpecific->show();
            connect(get_widget(QSL("cmbUPNQRMask")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
            connect(get_widget(QSL("chkUPNQRFast")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        }

    } else if (symbology == BARCODE_RMQR) {
        QFile file(QSL(":/grpRMQR.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        tabMain->insertTab(1, m_optionWidget, tr("rMQR Cod&e"));
        connect(get_widget(QSL("cmbRMQRSize")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbRMQRECC")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("radRMQRStand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radRMQRGS1")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkRMQRFullMultibyte")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_HANXIN) {
        QFile file(QSL(":/grpHX.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        tabMain->insertTab(1, m_optionWidget, tr("Han Xin Cod&e"));
        connect(get_widget(QSL("cmbHXSize")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbHXECC")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbHXMask")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("chkHXFullMultibyte")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_MICROQR) {
        QFile file(QSL(":/grpMQR.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        tabMain->insertTab(1, m_optionWidget, tr("Micro QR Cod&e"));
        connect(get_widget(QSL("cmbMQRSize")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbMQRECC")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbMQRMask")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("chkMQRFullMultibyte")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_GRIDMATRIX) {
        QFile file(QSL(":/grpGrid.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        structapp_ui_set();
        tabMain->insertTab(1, m_optionWidget, tr("Grid M&atrix"));
        set_smaller_font(QSL("noteGridECC"));
        connect(get_widget(QSL("cmbGridSize")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbGridECC")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("chkGridFullMultibyte")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbGridStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbGridStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(structapp_ui_set()));
        connect(get_widget(QSL("cmbGridStructAppIndex")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("spnGridStructAppID")), SIGNAL(valueChanged(int)), SLOT(update_preview()));

    } else if (symbology == BARCODE_MAXICODE) {
        QFile file(QSL(":/grpMaxicode.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        structapp_ui_set();
        tabMain->insertTab(1, m_optionWidget, tr("MaxiCod&e"));
        connect(get_widget(QSL("cmbMaxiMode")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbMaxiMode")), SIGNAL(currentIndexChanged(int)), SLOT(maxi_scm_ui_set()));
        connect(get_widget(QSL("txtMaxiSCMPostcode")), SIGNAL(textChanged(QString)), SLOT(update_preview()));
        connect(get_widget(QSL("spnMaxiSCMCountry")), SIGNAL(valueChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("spnMaxiSCMService")), SIGNAL(valueChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("chkMaxiSCMVV")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkMaxiSCMVV")), SIGNAL(toggled(bool)), SLOT(maxi_scm_ui_set()));
        connect(get_widget(QSL("spnMaxiSCMVV")), SIGNAL(valueChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbMaxiStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbMaxiStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(structapp_ui_set()));
        connect(get_widget(QSL("cmbMaxiStructAppIndex")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        maxi_scm_ui_set();

    } else if (symbology == BARCODE_CHANNEL) {
        QFile file(QSL(":/grpChannel.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        vLayoutSpecific->addWidget(m_optionWidget);
        grpSpecific->show();
        connect(get_widget(QSL("cmbChannel")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));

    } else if (symbology == BARCODE_CODEONE) {
        QFile file(QSL(":/grpCodeOne.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        codeone_ui_set();
        structapp_ui_set();
        tabMain->insertTab(1, m_optionWidget, tr("Code On&e"));
        connect(get_widget(QSL("cmbC1Size")), SIGNAL(currentIndexChanged(int)), SLOT(codeone_ui_set()));
        connect(get_widget(QSL("cmbC1Size")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("radC1GS1")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("spnC1StructAppCount")), SIGNAL(valueChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("spnC1StructAppCount")), SIGNAL(valueChanged(int)), SLOT(structapp_ui_set()));
        connect(get_widget(QSL("spnC1StructAppIndex")), SIGNAL(valueChanged(int)), SLOT(update_preview()));

    } else if (symbology == BARCODE_CODE49) {
        QFile file(QSL(":/grpC49.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        tabMain->insertTab(1, m_optionWidget, tr("Cod&e 49"));
        btype->setItemText(0, tr("Default (bind)"));
        connect(get_widget(QSL("cmbC49Rows")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        m_lblHeightPerRow = m_optionWidget->findChild<QLabel*>(QSL("lblC49HeightPerRow"));
        m_spnHeightPerRow = m_optionWidget->findChild<QDoubleSpinBox*>(QSL("spnC49HeightPerRow"));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(autoheight_ui_set()));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(update_preview()));
        m_btnHeightPerRowDisable = m_optionWidget->findChild<QPushButton*>(QSL("btnC49HeightPerRowDisable"));
        m_btnHeightPerRowDefault = m_optionWidget->findChild<QPushButton*>(QSL("btnC49HeightPerRowDefault"));
        connect(m_btnHeightPerRowDisable, SIGNAL(clicked(bool)), SLOT(height_per_row_disable()));
        connect(m_btnHeightPerRowDefault, SIGNAL(clicked(bool)), SLOT(height_per_row_default()));
        connect(get_widget(QSL("cmbC49RowSepHeight")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("radC49GS1")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkC49NoQuietZones")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_CODE93) {
        QFile file(QSL(":/grpC93.ui"));
        if (file.open(QIODevice::ReadOnly)) {
            m_optionWidget = uiload.load(&file);
            file.close();
            load_sub_settings(settings, symbology);
            vLayoutSpecific->addWidget(m_optionWidget);
            grpSpecific->show();
            connect(get_widget(QSL("chkC93ShowChecks")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        }

    } else if (symbology == BARCODE_DBAR_EXPSTK) {
        QFile file(QSL(":/grpDBExtend.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        tabMain->insertTab(1, m_optionWidget, tr("GS1 D&ataBar Exp Stack"));
        connect(get_widget(QSL("radDBESCols")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radDBESRows")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbDBESCols")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbDBESRows")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        m_lblHeightPerRow = m_optionWidget->findChild<QLabel*>(QSL("lblDBESHeightPerRow"));
        m_spnHeightPerRow = m_optionWidget->findChild<QDoubleSpinBox*>(QSL("spnDBESHeightPerRow"));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(autoheight_ui_set()));
        connect(m_spnHeightPerRow, SIGNAL(valueChanged(double)), SLOT(update_preview()));
        m_btnHeightPerRowDisable = m_optionWidget->findChild<QPushButton*>(QSL("btnDBESHeightPerRowDisable"));
        m_btnHeightPerRowDefault = m_optionWidget->findChild<QPushButton*>(QSL("btnDBESHeightPerRowDefault"));
        connect(m_btnHeightPerRowDisable, SIGNAL(clicked(bool)), SLOT(height_per_row_disable()));
        connect(m_btnHeightPerRowDefault, SIGNAL(clicked(bool)), SLOT(height_per_row_default()));

    } else if (symbology == BARCODE_ULTRA) {
        QFile file(QSL(":/grpUltra.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        structapp_ui_set();
        tabMain->insertTab(1, m_optionWidget, tr("Ultracod&e"));
        connect(get_widget(QSL("radUltraAuto")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radUltraEcc")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbUltraEcc")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbUltraRevision")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("radUltraStand")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("radUltraGS1")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbUltraStructAppCount")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("cmbUltraStructAppCount")), SIGNAL(currentIndexChanged(int)),
                SLOT(structapp_ui_set()));
        connect(get_widget(QSL("cmbUltraStructAppIndex")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("spnUltraStructAppID")), SIGNAL(valueChanged(int)), SLOT(update_preview()));

    } else if (symbology == BARCODE_UPCA || symbology == BARCODE_UPCA_CHK || symbology == BARCODE_UPCA_CC) {
        QFile file(QSL(":/grpUPCA.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        upcae_no_quiet_zones_ui_set();
        tabMain->insertTab(1, m_optionWidget, tr("UPC-&A"));
        combobox_item_enabled(cmbFontSetting, 1, false); // Disable bold options
        combobox_item_enabled(cmbFontSetting, 3, false);
        if (cmbFontSetting->currentIndex() == 1 || cmbFontSetting->currentIndex() == 3) {
            cmbFontSetting->setCurrentIndex(cmbFontSetting->currentIndex() - 1);
        }
        connect(get_widget(QSL("cmbUPCAAddonGap")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("spnUPCAGuardDescent")), SIGNAL(valueChanged(double)), SLOT(update_preview()));
        connect(get_widget(QSL("btnUPCAGuardDefault")), SIGNAL(clicked(bool)), SLOT(guard_default_upca()));
        connect(get_widget(QSL("chkUPCANoQuietZones")), SIGNAL(toggled(bool)), SLOT(upcae_no_quiet_zones_ui_set()));
        connect(get_widget(QSL("chkUPCANoQuietZones")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkUPCAGuardWhitespace")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_EANX || symbology == BARCODE_EANX_CHK || symbology == BARCODE_EANX_CC
            || symbology == BARCODE_UPCE || symbology == BARCODE_UPCE_CHK || symbology == BARCODE_UPCE_CC
            || symbology == BARCODE_ISBNX) {
        QFile file(QSL(":/grpUPCEAN.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        const bool is_upce = symbology == BARCODE_UPCE || symbology == BARCODE_UPCE_CHK
                                || symbology == BARCODE_UPCE_CC;
        if (is_upce) {
            tabMain->insertTab(1, m_optionWidget, tr("UPC-&E"));
            upcae_no_quiet_zones_ui_set();
        } else if (symbology == BARCODE_ISBNX) {
            tabMain->insertTab(1, m_optionWidget, tr("ISBN"));
            upcean_no_quiet_zones_ui_set();
        } else {
            tabMain->insertTab(1, m_optionWidget, tr("&EAN"));
            upcean_no_quiet_zones_ui_set();
        }
        combobox_item_enabled(cmbFontSetting, 1, false); // Disable bold options
        combobox_item_enabled(cmbFontSetting, 3, false);
        if (cmbFontSetting->currentIndex() == 1 || cmbFontSetting->currentIndex() == 3) {
            cmbFontSetting->setCurrentIndex(cmbFontSetting->currentIndex() - 1);
        }
        connect(get_widget(QSL("cmbUPCEANAddonGap")), SIGNAL(currentIndexChanged(int)), SLOT(update_preview()));
        connect(get_widget(QSL("spnUPCEANGuardDescent")), SIGNAL(valueChanged(double)), SLOT(update_preview()));
        connect(get_widget(QSL("btnUPCEANGuardDefault")), SIGNAL(clicked(bool)), SLOT(guard_default_upcean()));
        if (is_upce) {
            connect(get_widget(QSL("chkUPCEANNoQuietZones")), SIGNAL(toggled(bool)),
                    SLOT(upcae_no_quiet_zones_ui_set()));
        } else {
            connect(get_widget(QSL("chkUPCEANNoQuietZones")), SIGNAL(toggled(bool)),
                    SLOT(upcean_no_quiet_zones_ui_set()));
        }
        connect(get_widget(QSL("chkUPCEANNoQuietZones")), SIGNAL(toggled(bool)), SLOT(update_preview()));
        connect(get_widget(QSL("chkUPCEANGuardWhitespace")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else if (symbology == BARCODE_VIN) {
        QFile file(QSL(":/grpVIN.ui"));
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget = uiload.load(&file);
        file.close();
        load_sub_settings(settings, symbology);
        vLayoutSpecific->addWidget(m_optionWidget);
        grpSpecific->show();
        connect(get_widget(QSL("chkVINImportChar")), SIGNAL(toggled(bool)), SLOT(update_preview()));

    } else {
        m_optionWidget = nullptr;
        load_sub_settings(settings, symbology);
    }

    switch (symbology) {
        case BARCODE_CODE128:
        case BARCODE_EANX:
        case BARCODE_UPCA:
        case BARCODE_UPCE:
        case BARCODE_DBAR_OMN:
        case BARCODE_DBAR_LTD:
        case BARCODE_DBAR_EXP:
        case BARCODE_DBAR_STK:
        case BARCODE_DBAR_OMNSTK:
        case BARCODE_DBAR_EXPSTK:
            grpComposite->show();
            grpSegs->hide();
            break;
        case BARCODE_AZTEC:
        case BARCODE_DATAMATRIX:
        case BARCODE_MAXICODE:
        case BARCODE_MICROPDF417:
        case BARCODE_PDF417:
        case BARCODE_PDF417COMP:
        case BARCODE_QRCODE:
        case BARCODE_DOTCODE:
        case BARCODE_CODEONE:
        case BARCODE_GRIDMATRIX:
        case BARCODE_HANXIN:
        case BARCODE_ULTRA:
        case BARCODE_RMQR:
            grpComposite->hide();
            grpSegs->show();
            break;
        default:
            grpComposite->hide();
            grpSegs->hide();
            break;
    }

    // ECI, GS1Parens, GS1NoCheck, RInit, CompliantHeight will be checked in update_preview() as
    // encoding mode dependent (HIBC and/or GS1)
    chkAutoHeight->setEnabled(!m_bc.bc.isFixedRatio(symbology));
    chkHRTShow->setEnabled(m_bc.bc.hasHRT(symbology));
    chkQuietZones->setEnabled(!m_bc.bc.hasDefaultQuietZones(symbology));
    chkDotty->setEnabled(m_bc.bc.isDotty(symbology));

    setColorTxtBtn(m_fgstr, txt_fgcolor, fgcolor);
    setColorTxtBtn(m_bgstr, txt_bgcolor, bgcolor);

    data_ui_set();
    composite_ui_set();
    autoheight_ui_set();
    HRTShow_ui_set();
    dotty_ui_set();

    if (initial_load) {
        tabMain->setCurrentIndex(settings.value(QSL("studio/tab_index"), 0).toInt());
    } else if (original_tab_count == tabMain->count()) {
        tabMain->setCurrentIndex(original_tab_index);
    } else if (original_tab_count > tabMain->count()) {
        tabMain->setCurrentIndex(original_tab_index == 2 ? 1 : 0);
    } else {
        tabMain->setCurrentIndex(original_tab_index == 1 ? 2 : 0);
    }
}

void MainWindow::data_ui_set()
{
    if (grpSegs->isHidden()) {
        return;
    }

    if (txtData->text().isEmpty()) {
        lblSeg1->setEnabled(false);
        cmbECISeg1->setEnabled(false);
        txtDataSeg1->setEnabled(false);
        btnMoreDataSeg1->setEnabled(false);
        btnClearDataSeg1->setEnabled(false);

        lblSeg2->setEnabled(false);
        cmbECISeg2->setEnabled(false);
        txtDataSeg2->setEnabled(false);
        btnMoreDataSeg2->setEnabled(false);
        btnClearDataSeg2->setEnabled(false);

        lblSeg3->setEnabled(false);
        cmbECISeg3->setEnabled(false);
        txtDataSeg3->setEnabled(false);
        btnMoreDataSeg3->setEnabled(false);
        btnClearDataSeg3->setEnabled(false);
        return;
    }

    lblSeg1->setEnabled(true);
    txtDataSeg1->setEnabled(true);
    btnMoreDataSeg1->setEnabled(true);
    if (txtDataSeg1->text().isEmpty()) {
        cmbECISeg1->setEnabled(false);
        btnClearDataSeg1->setEnabled(false);

        lblSeg2->setEnabled(false);
        cmbECISeg2->setEnabled(false);
        txtDataSeg2->setEnabled(false);
        btnMoreDataSeg2->setEnabled(false);
        btnClearDataSeg2->setEnabled(false);

        lblSeg3->setEnabled(false);
        cmbECISeg3->setEnabled(false);
        txtDataSeg3->setEnabled(false);
        btnMoreDataSeg3->setEnabled(false);
        btnClearDataSeg3->setEnabled(false);
    } else {
        cmbECISeg1->setEnabled(true);
        btnClearDataSeg1->setEnabled(true);

        lblSeg2->setEnabled(true);
        txtDataSeg2->setEnabled(true);
        btnMoreDataSeg2->setEnabled(true);
        if (txtDataSeg2->text().isEmpty()) {
            cmbECISeg2->setEnabled(false);
            btnClearDataSeg2->setEnabled(false);

            lblSeg3->setEnabled(false);
            cmbECISeg3->setEnabled(false);
            txtDataSeg3->setEnabled(false);
            btnMoreDataSeg3->setEnabled(false);
            btnClearDataSeg3->setEnabled(false);
        } else {
            cmbECISeg2->setEnabled(true);
            btnClearDataSeg2->setEnabled(true);

            bool data_seg3_empty = txtDataSeg3->text().isEmpty();
            lblSeg3->setEnabled(true);
            cmbECISeg3->setEnabled(!data_seg3_empty);
            txtDataSeg3->setEnabled(true);
            btnMoreDataSeg3->setEnabled(true);
            btnClearDataSeg3->setEnabled(!data_seg3_empty);
        }
    }
}

void MainWindow::composite_ui_set()
{
    bool enabled = !grpComposite->isHidden() && chkComposite->isChecked();

    lblCompType->setEnabled(enabled);
    cmbCompType->setEnabled(enabled);
    lblComposite->setEnabled(enabled);
    btnClearComposite->setEnabled(enabled);
    txtComposite->setEnabled(enabled);

    if (enabled) {
        if (bstyle_items[bstyle->currentIndex()].symbology == BARCODE_CODE128) {
            QRadioButton *radioButton = m_optionWidget->findChild<QRadioButton*>(QSL("radC128EAN"));
            if (radioButton) {
                radioButton->setChecked(true);
            }
        }
    }
}

void MainWindow::composite_ean_check()
{
    if (bstyle_items[bstyle->currentIndex()].symbology != BARCODE_CODE128)
        return;
    QRadioButton *radioButton = m_optionWidget->findChild<QRadioButton*>(QSL("radC128EAN"));
    if (radioButton && !radioButton->isChecked())
        chkComposite->setChecked(false);
}

void MainWindow::maxi_scm_ui_set()
{
    if (bstyle_items[bstyle->currentIndex()].symbology != BARCODE_MAXICODE)
        return;
    QCheckBox *chkMaxiSCMVV = m_optionWidget ? m_optionWidget->findChild<QCheckBox*>(QSL("chkMaxiSCMVV")) : nullptr;
    if (!chkMaxiSCMVV)
        return;

    bool isMode2or3 = get_cmb_index(QSL("cmbMaxiMode")) == 0;

    m_optionWidget->findChild<QLabel*>(QSL("lblMaxiSCMPostcode"))->setEnabled(isMode2or3);
    m_optionWidget->findChild<QLineEdit*>(QSL("txtMaxiSCMPostcode"))->setEnabled(isMode2or3);
    m_optionWidget->findChild<QLabel*>(QSL("lblMaxiSCMCountry"))->setEnabled(isMode2or3);
    m_optionWidget->findChild<QSpinBox*>(QSL("spnMaxiSCMCountry"))->setEnabled(isMode2or3);
    m_optionWidget->findChild<QLabel*>(QSL("lblMaxiSCMService"))->setEnabled(isMode2or3);
    m_optionWidget->findChild<QSpinBox*>(QSL("spnMaxiSCMService"))->setEnabled(isMode2or3);
    chkMaxiSCMVV->setEnabled(isMode2or3);
    m_optionWidget->findChild<QLabel*>(QSL("lblMaxiSCMVV"))->setEnabled(isMode2or3 && chkMaxiSCMVV->isChecked());
    m_optionWidget->findChild<QSpinBox*>(QSL("spnMaxiSCMVV"))->setEnabled(isMode2or3 && chkMaxiSCMVV->isChecked());
}

void MainWindow::msi_plessey_ui_set()
{
    if (bstyle_items[bstyle->currentIndex()].symbology != BARCODE_MSI_PLESSEY)
        return;
    QCheckBox *checkBox = m_optionWidget ? m_optionWidget->findChild<QCheckBox*>(QSL("chkMSICheckText")) : nullptr;
    if (checkBox) {
        checkBox->setEnabled(get_cmb_index(QSL("cmbMSICheck")) > 0);
    }
}

// Taken from https://stackoverflow.com/questions/38915001/disable-specific-items-in-qcombobox
void MainWindow::combobox_item_enabled(QComboBox *comboBox, int index, bool enabled)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(comboBox->model());
    if (model) {
        QStandardItem *item = model->item(index);
        if (item) {
            item->setEnabled(enabled);
        }
    }
}

bool MainWindow::upcean_addon_gap(const QString &comboBoxName, const QString &labelName, int base)
{
    QComboBox *comboBox = m_optionWidget->findChild<QComboBox*>(comboBoxName);
    QLabel *label = m_optionWidget->findChild<QLabel*>(labelName);

    bool enabled = have_addon();
    if (comboBox) {
        comboBox->setEnabled(enabled);
    }
    if (label) {
        label->setEnabled(enabled);
    }
    if (enabled && comboBox) {
        int item_val = comboBox->currentIndex();
        if (item_val) {
            m_bc.bc.setOption2(item_val + base);
        }
    }
    return enabled;
}

void MainWindow::upcean_guard_descent(const QString &spnBoxName, const QString &labelName,
        const QString &btnDefaultName, bool enabled)
{
    QDoubleSpinBox *spnBox = m_optionWidget->findChild<QDoubleSpinBox*>(spnBoxName);
    QLabel *label = m_optionWidget->findChild<QLabel*>(labelName);
    QPushButton *btnDefault = m_optionWidget->findChild<QPushButton*>(btnDefaultName);

    if (spnBox) {
        spnBox->setEnabled(enabled);
    }
    if (label) {
        label->setEnabled(enabled);
    }
    if (btnDefault) {
        btnDefault->setEnabled(enabled);
    }
    if (enabled && spnBox) {
        m_bc.bc.setGuardDescent(spnBox->value());
        if (btnDefault && spnBox->value() == 5.0) {
            QWidget *focus = QApplication::focusWidget();
            btnDefault->setEnabled(false);
            if (focus == btnDefault) {
                spnBox->setFocus();
            }
        }
    }
}

void MainWindow::guard_default(const QString &spnBoxName)
{
    QDoubleSpinBox *spnBox = m_optionWidget->findChild<QDoubleSpinBox*>(spnBoxName);
    if (spnBox && spnBox->value() != 5.0) {
        spnBox->setValue(5.0);
        update_preview();
    }
}

void MainWindow::daft_ui_set()
{
    QDoubleSpinBox *spnBox = m_optionWidget->findChild<QDoubleSpinBox*>(QSL("spnDAFTTrackerRatio"));
    QPushButton *btnDefault = m_optionWidget->findChild<QPushButton*>(QSL("btnDAFTTrackerDefault"));
    if (spnBox && spnBox->value() == 25.0) {
        if (btnDefault) {
            QWidget *focus = QApplication::focusWidget();
            btnDefault->setEnabled(false);
            if (focus == btnDefault) {
                spnBox->setFocus();
            }
        }
    } else if (btnDefault && !btnDefault->isEnabled()) {
        btnDefault->setEnabled(true);
    }
}

void MainWindow::daft_tracker_default()
{
    QDoubleSpinBox *spnBox = m_optionWidget->findChild<QDoubleSpinBox*>(QSL("spnDAFTTrackerRatio"));
    if (spnBox && spnBox->value() != 25.0) {
        spnBox->setValue(25.0);
        update_preview();
    }
}

void MainWindow::set_gs1_mode(bool gs1_mode)
{
    if (gs1_mode) {
        m_bc.bc.setInputMode(GS1_MODE | (m_bc.bc.inputMode() & ~0x07)); // Keep upper bits
        chkData->setEnabled(false);
    } else {
        chkData->setEnabled(true);
    }
}

void MainWindow::set_smaller_font(const QString &labelName)
{
    QLabel *label = m_optionWidget ? m_optionWidget->findChild<QLabel*>(labelName) : nullptr;
    if (label) {
        const QFont &appFont = QApplication::font();
        qreal pointSize = appFont.pointSizeF();
        if (pointSize != -1.0) {
            QFont font = label->font();
            pointSize *= 0.9;
            font.setPointSizeF(pointSize);
            label->setFont(font);
        } else {
            int pixelSize = appFont.pixelSize();
            if (pixelSize > 1) {
                QFont font = label->font();
                font.setPixelSize(pixelSize - 1);
                label->setFont(font);
            }
        }
    }
}

void MainWindow::update_preview()
{
    int symbology = bstyle_items[bstyle->currentIndex()].symbology;
    int eci_not_set = true;
    int width = view->geometry().width();
    int height = view->geometry().height();
    int item_val;
    QCheckBox *checkBox;

    if (!grpComposite->isHidden() && chkComposite->isChecked()) {
        m_bc.bc.setPrimaryMessage(txtData->text());
        m_bc.bc.setText(txtComposite->toPlainText());
        btnClearComposite->setEnabled(!txtComposite->toPlainText().isEmpty());
    } else {
        btnClearComposite->setEnabled(false);
        if (!grpSegs->isHidden() && !txtDataSeg1->text().isEmpty()) {
            std::vector<Zint::QZintSeg> segs;
            segs.push_back(Zint::QZintSeg(txtData->text(), cmbECI->currentIndex()));
            segs.push_back(Zint::QZintSeg(txtDataSeg1->text(), cmbECISeg1->currentIndex()));
            if (!txtDataSeg2->text().isEmpty()) {
                segs.push_back(Zint::QZintSeg(txtDataSeg2->text(), cmbECISeg2->currentIndex()));
                if (!txtDataSeg3->text().isEmpty()) {
                    segs.push_back(Zint::QZintSeg(txtDataSeg3->text(), cmbECISeg3->currentIndex()));
                }
            }
            m_bc.bc.setSegs(segs);
        } else {
            m_bc.bc.setText(txtData->text());
        }
    }
    btnReset->setEnabled((m_fgstr != fgDefault && m_fgstr != fgDefaultAlpha)
                        || (m_bgstr != bgDefault && m_bgstr != bgDefaultAlpha));
    m_bc.bc.setOption1(-1);
    m_bc.bc.setOption2(0);
    m_bc.bc.setOption3(0);
    m_bc.bc.setDPMM(m_xdimdpVars.set ? get_dpmm(&m_xdimdpVars) : 0.0f);
    chkData->setEnabled(true);
    if (chkData->isChecked()) {
        m_bc.bc.setInputMode(DATA_MODE);
    } else {
        m_bc.bc.setInputMode(UNICODE_MODE);
    }
    if (chkEscape->isChecked()) {
        m_bc.bc.setInputMode(m_bc.bc.inputMode() | ESCAPE_MODE);
    }
    m_bc.bc.setGSSep(false);
    m_bc.bc.setNoQuietZones(false);
    m_bc.bc.setGuardWhitespace(false);
    m_bc.bc.setDotSize(0.4f / 0.5f);
    m_bc.bc.setGuardDescent(5.0f);
    m_bc.bc.setTextGap(1.0f);
    m_bc.bc.clearStructApp();

    switch (symbology) {

        case BARCODE_CODE128:
            if (get_rad_val(QSL("radC128CSup"))) {
                m_bc.bc.setSymbol(BARCODE_CODE128AB);
            } else if (get_rad_val(QSL("radC128EAN"))) {
                m_bc.bc.setSymbol(chkComposite->isChecked() ? BARCODE_GS1_128_CC : BARCODE_GS1_128);
            } else if (get_rad_val(QSL("radC128HIBC"))) {
                m_bc.bc.setSymbol(BARCODE_HIBC_128);
            } else if (get_rad_val(QSL("radC128ExtraEsc"))) {
                m_bc.bc.setSymbol(BARCODE_CODE128);
                m_bc.bc.setInputMode(m_bc.bc.inputMode() | EXTRA_ESCAPE_MODE);
            } else {
                m_bc.bc.setSymbol(BARCODE_CODE128);
            }
            break;

        case BARCODE_EANX:
            m_bc.bc.setSymbol(chkComposite->isChecked() ? BARCODE_EANX_CC : BARCODE_EANX);
            {
                bool have_addon = upcean_addon_gap(QSL("cmbUPCEANAddonGap"), QSL("lblUPCEANAddonGap"), 7 /*base*/);
                bool enable_guard = have_addon || txtData->text().length() > 5;
                upcean_guard_descent(QSL("spnUPCEANGuardDescent"), QSL("lblUPCEANGuardDescent"),
                                        QSL("btnUPCEANGuardDefault"), enable_guard);
            }
            if (get_chk_val(QSL("chkUPCEANNoQuietZones"))) {
                m_bc.bc.setNoQuietZones(true);
            } else if (get_chk_val(QSL("chkUPCEANGuardWhitespace"))) {
                m_bc.bc.setGuardWhitespace(true);
            }
            break;

        case BARCODE_ISBNX:
            m_bc.bc.setSymbol(symbology);
            upcean_addon_gap(QSL("cmbUPCEANAddonGap"), QSL("lblUPCEANAddonGap"), 7 /*base*/);
            upcean_guard_descent(QSL("spnUPCEANGuardDescent"), QSL("lblUPCEANGuardDescent"),
                                    QSL("btnUPCEANGuardDefault"));
            if (get_chk_val(QSL("chkUPCEANNoQuietZones"))) {
                m_bc.bc.setNoQuietZones(true);
            } else if (get_chk_val(QSL("chkUPCEANGuardWhitespace"))) {
                m_bc.bc.setGuardWhitespace(true);
            }
            break;

        case BARCODE_UPCA:
            m_bc.bc.setSymbol(chkComposite->isChecked() ? BARCODE_UPCA_CC : BARCODE_UPCA);
            upcean_addon_gap(QSL("cmbUPCAAddonGap"), QSL("lblUPCAAddonGap"), 9 /*base*/);
            upcean_guard_descent(QSL("spnUPCAGuardDescent"), QSL("lblUPCAGuardDescent"), QSL("btnUPCAGuardDefault"));
            if (get_chk_val(QSL("chkUPCANoQuietZones"))) {
                m_bc.bc.setNoQuietZones(true);
            } else if (get_chk_val(QSL("chkUPCAGuardWhitespace"))) {
                m_bc.bc.setGuardWhitespace(true);
            }
            break;

        case BARCODE_UPCE:
            m_bc.bc.setSymbol(chkComposite->isChecked() ? BARCODE_UPCE_CC : BARCODE_UPCE);
            upcean_addon_gap(QSL("cmbUPCEANAddonGap"), QSL("lblUPCEANAddonGap"), 7 /*base*/);
            upcean_guard_descent(QSL("spnUPCEANGuardDescent"), QSL("lblUPCEANGuardDescent"),
                                    QSL("btnUPCEANGuardDefault"));
            if (get_chk_val(QSL("chkUPCEANNoQuietZones"))) {
                m_bc.bc.setNoQuietZones(true);
            } else if (get_chk_val(QSL("chkUPCEANGuardWhitespace"))) {
                m_bc.bc.setGuardWhitespace(true);
            }
            break;

        case BARCODE_DBAR_OMN:
            m_bc.bc.setSymbol(chkComposite->isChecked() ? BARCODE_DBAR_OMN_CC : BARCODE_DBAR_OMN);
            break;
        case BARCODE_DBAR_LTD:
            m_bc.bc.setSymbol(chkComposite->isChecked() ? BARCODE_DBAR_LTD_CC : BARCODE_DBAR_LTD);
            break;
        case BARCODE_DBAR_EXP:
            m_bc.bc.setSymbol(chkComposite->isChecked() ? BARCODE_DBAR_EXP_CC : BARCODE_DBAR_EXP);
            break;
        case BARCODE_DBAR_STK:
            m_bc.bc.setSymbol(chkComposite->isChecked() ? BARCODE_DBAR_STK_CC : BARCODE_DBAR_STK);
            break;
        case BARCODE_DBAR_OMNSTK:
            m_bc.bc.setSymbol(chkComposite->isChecked() ? BARCODE_DBAR_OMNSTK_CC : BARCODE_DBAR_OMNSTK);
            break;
        case BARCODE_DBAR_EXPSTK:
            m_bc.bc.setSymbol(chkComposite->isChecked() ? BARCODE_DBAR_EXPSTK_CC : BARCODE_DBAR_EXPSTK);
            if (get_rad_val(QSL("radDBESCols"))) {
                if ((item_val = get_cmb_index(QSL("cmbDBESCols"))) != 0) {
                    m_bc.bc.setOption2(item_val);
                }
            } else if (get_rad_val(QSL("radDBESRows"))) {
                if ((item_val = get_cmb_index(QSL("cmbDBESRows"))) != 0) {
                    m_bc.bc.setOption3(item_val + 1); // Begins at 2
                }
            }
            break;

        case BARCODE_PDF417:
            if (get_rad_val(QSL("radPDFTruncated")))
                m_bc.bc.setSymbol(BARCODE_PDF417COMP);
            else if (get_rad_val(QSL("radPDFHIBC")))
                m_bc.bc.setSymbol(BARCODE_HIBC_PDF);
            else
                m_bc.bc.setSymbol(BARCODE_PDF417);

            m_bc.bc.setOption2(get_cmb_index(QSL("cmbPDFCols")));
            if ((item_val = get_cmb_index(QSL("cmbPDFRows"))) != 0) {
                m_bc.bc.setOption3(item_val + 2); // Starts at 3 rows
            }
            m_bc.bc.setOption1(get_cmb_index(QSL("cmbPDFECC")) - 1);

            if (get_chk_val(QSL("chkPDFFast"))) {
                m_bc.bc.setInputMode(FAST_MODE | m_bc.bc.inputMode());
            }

            if ((item_val = get_spn_val(QSL("spnPDFStructAppCount"))) > 1) {
                m_bc.bc.setStructApp(item_val, get_spn_val(QSL("spnPDFStructAppIndex")),
                            get_txt_val(QSL("txtPDFStructAppID")));
            }
            break;

        case BARCODE_MICROPDF417:
            if (get_rad_val(QSL("radMPDFHIBC")))
                m_bc.bc.setSymbol(BARCODE_HIBC_MICPDF);
            else
                m_bc.bc.setSymbol(BARCODE_MICROPDF417);

            m_bc.bc.setOption2(get_cmb_index(QSL("cmbMPDFCols")));

            if (get_chk_val(QSL("chkMPDFFast"))) {
                m_bc.bc.setInputMode(FAST_MODE | m_bc.bc.inputMode());
            }

            if ((item_val = get_spn_val(QSL("spnMPDFStructAppCount"))) > 1) {
                m_bc.bc.setStructApp(item_val, get_spn_val(QSL("spnMPDFStructAppIndex")),
                            get_txt_val(QSL("txtMPDFStructAppID")));
            }
            break;

        case BARCODE_DOTCODE:
            m_bc.bc.setSymbol(BARCODE_DOTCODE);
            if ((item_val = get_cmb_index(QSL("cmbDotCols"))) != 0) {
                m_bc.bc.setOption2(item_val + 4); // Cols 1-4 not listed
            }
            if ((item_val = get_cmb_index(QSL("cmbDotMask"))) != 0) {
                m_bc.bc.setOption3((item_val << 8) | m_bc.bc.option3());
            }
            set_gs1_mode(get_rad_val(QSL("radDotGS1")));

            if ((item_val = get_cmb_index(QSL("cmbDotStructAppCount"))) != 0) {
                QString id; // Dummy ID
                m_bc.bc.setStructApp(item_val + 1, get_cmb_index(QSL("cmbDotStructAppIndex")) + 1, id);
            }
            break;

        case BARCODE_AZTEC:
            if (get_rad_val(QSL("radAztecHIBC")))
                m_bc.bc.setSymbol(BARCODE_HIBC_AZTEC);
            else
                m_bc.bc.setSymbol(BARCODE_AZTEC);

            if (get_rad_val(QSL("radAztecSize")))
                m_bc.bc.setOption2(get_cmb_index(QSL("cmbAztecSize")) + 1);

            if (get_rad_val(QSL("radAztecECC")))
                m_bc.bc.setOption1(get_cmb_index(QSL("cmbAztecECC")) + 1);

            set_gs1_mode(get_rad_val(QSL("radAztecGS1")));

            if ((item_val = get_cmb_index(QSL("cmbAztecStructAppCount"))) != 0) {
                m_bc.bc.setStructApp(item_val + 1, get_cmb_index(QSL("cmbAztecStructAppIndex")) + 1,
                            get_txt_val(QSL("txtAztecStructAppID")));
            }
            break;

        case BARCODE_MSI_PLESSEY:
            m_bc.bc.setSymbol(BARCODE_MSI_PLESSEY);
            item_val = get_cmb_index(QSL("cmbMSICheck"));
            if (item_val && get_chk_val(QSL("chkMSICheckText"))) {
                item_val += 10;
            }
            m_bc.bc.setOption2(item_val);
            break;

        case BARCODE_CODE11:
            m_bc.bc.setSymbol(BARCODE_CODE11);
            if (get_rad_val(QSL("radC11OneCheckDigit"))) {
                m_bc.bc.setOption2(1);
            } else if (get_rad_val(QSL("radC11NoCheckDigits"))) {
                m_bc.bc.setOption2(2);
            }
            break;

        case BARCODE_C25STANDARD:
        case BARCODE_C25INTER:
        case BARCODE_C25IATA:
        case BARCODE_C25LOGIC:
        case BARCODE_C25IND:
            m_bc.bc.setSymbol(symbology);
            m_bc.bc.setOption2(get_rad_grp_index(
                QStringList() << QSL("radC25Stand") << QSL("radC25Check") << QSL("radC25CheckHide")));
            break;

        case BARCODE_CODE39:
            if (get_rad_val(QSL("radC39HIBC"))) {
                m_bc.bc.setSymbol(BARCODE_HIBC_39);
            } else {
                m_bc.bc.setSymbol(BARCODE_CODE39);
                if (get_rad_val(QSL("radC39Check"))) {
                    m_bc.bc.setOption2(1);
                } else if (get_rad_val(QSL("radC39CheckHide"))) {
                    m_bc.bc.setOption2(2);
                }
            }
            break;
        case BARCODE_EXCODE39:
            m_bc.bc.setSymbol(BARCODE_EXCODE39);
            if (get_rad_val(QSL("radC39Check"))) {
                m_bc.bc.setOption2(1);
            } else if (get_rad_val(QSL("radC39CheckHide"))) {
                m_bc.bc.setOption2(2);
            }
            break;
        case BARCODE_LOGMARS:
            m_bc.bc.setSymbol(BARCODE_LOGMARS);
            if (get_rad_val(QSL("radC39Check"))) {
                m_bc.bc.setOption2(1);
            } else if (get_rad_val(QSL("radC39CheckHide"))) {
                m_bc.bc.setOption2(2);
            }
            break;

        case BARCODE_CODE16K:
            m_bc.bc.setSymbol(BARCODE_CODE16K);
            set_gs1_mode(get_rad_val(QSL("radC16kGS1")));
            if ((item_val = get_cmb_index(QSL("cmbC16kRows"))) != 0) {
                m_bc.bc.setOption1(item_val + 2); // Starts at 3
            }
            // Row separator height selection uses option 3 in zint_symbol
            if ((item_val = get_cmb_index(QSL("cmbC16kRowSepHeight"))) != 0) {
                m_bc.bc.setOption3(item_val + 1); // Zero-based
            }
            if (get_chk_val(QSL("chkC16kNoQuietZones"))) {
                m_bc.bc.setNoQuietZones(true);
            }
            break;

        case BARCODE_CODABAR:
            m_bc.bc.setSymbol(BARCODE_CODABAR);
            m_bc.bc.setOption2(get_rad_grp_index(
                QStringList() << QSL("radCodabarStand") << QSL("radCodabarCheckHide") << QSL("radCodabarCheck")));
            break;

        case BARCODE_CODABLOCKF:
            if (get_rad_val(QSL("radCbfHIBC"))) {
                m_bc.bc.setSymbol(BARCODE_HIBC_BLOCKF);
            } else {
                m_bc.bc.setSymbol(BARCODE_CODABLOCKF);
            }
            if ((item_val = get_cmb_index(QSL("cmbCbfWidth"))) != 0) {
                m_bc.bc.setOption2(item_val - 1 + 9);
            }
            // Height selection uses option 1 in zint_symbol
            if ((item_val = get_cmb_index(QSL("cmbCbfHeight"))) != 0) {
                m_bc.bc.setOption1(item_val);
            }
            // Row separator height selection uses option 3 in zint_symbol
            if ((item_val = get_cmb_index(QSL("cmbCbfRowSepHeight"))) != 0) {
                m_bc.bc.setOption3(item_val + 1); // Zero-based
            }
            if (get_chk_val(QSL("chkCbfNoQuietZones"))) {
                m_bc.bc.setNoQuietZones(true);
            }
            break;

        case BARCODE_DAFT:
            m_bc.bc.setSymbol(BARCODE_DAFT);
            // Kept as percentage, convert to thousandths
            m_bc.bc.setOption2((int) (get_dspn_val(QSL("spnDAFTTrackerRatio")) * 10));
            break;

        case BARCODE_DPD:
            m_bc.bc.setSymbol(BARCODE_DPD);
            if (get_chk_val(QSL("chkDPDRelabel"))) {
                m_bc.bc.setOption2(1);
            }
            break;

        case BARCODE_DATAMATRIX:
            if (get_rad_val(QSL("radDM200HIBC")))
                m_bc.bc.setSymbol(BARCODE_HIBC_DM);
            else
                m_bc.bc.setSymbol(BARCODE_DATAMATRIX);

            checkBox = m_optionWidget->findChild<QCheckBox*>(QSL("chkDMGSSep"));
            if (get_rad_val(QSL("radDM200GS1"))) {
                set_gs1_mode(true);
                checkBox->setEnabled(true);
                if (checkBox->isChecked()) {
                    m_bc.bc.setGSSep(true);
                }
            } else {
                set_gs1_mode(false);
                checkBox->setEnabled(false);
            }

            m_bc.bc.setOption2(get_cmb_index(QSL("cmbDM200Size")));

            if (get_cmb_index(QSL("cmbDM200Size")) == 0) {
                // Suppressing rectangles or allowing DMRE only makes sense if in automatic size mode
                m_optionWidget->findChild<QLabel*>(QSL("lblDMAutoSize"))->setEnabled(true);
                m_optionWidget->findChild<QCheckBox*>(QSL("chkDMRectangle"))->setEnabled(true);
                if (m_optionWidget->findChild<QCheckBox*>(QSL("chkDMRectangle"))->isChecked()) {
                    m_bc.bc.setOption3(DM_SQUARE);
                    m_optionWidget->findChild<QCheckBox*>(QSL("chkDMRE"))->setEnabled(false);
                } else {
                    m_optionWidget->findChild<QCheckBox*>(QSL("chkDMRE"))->setEnabled(true);
                    if (m_optionWidget->findChild<QCheckBox*>(QSL("chkDMRE"))->isChecked())
                        m_bc.bc.setOption3(DM_DMRE);
                    else
                        m_bc.bc.setOption3(0);
                }
            } else {
                m_optionWidget->findChild<QLabel*>(QSL("lblDMAutoSize"))->setEnabled(false);
                m_optionWidget->findChild<QCheckBox*>(QSL("chkDMRectangle"))->setEnabled(false);
                m_optionWidget->findChild<QCheckBox*>(QSL("chkDMRE"))->setEnabled(false);
                m_bc.bc.setOption3(0);
            }

            if (get_chk_val(QSL("chkDMISO144"))) {
                m_bc.bc.setOption3(m_bc.bc.option3() | DM_ISO_144);
            }

            if (get_chk_val(QSL("chkDMFast"))) {
                m_bc.bc.setInputMode(FAST_MODE | m_bc.bc.inputMode());
            }

            if ((item_val = get_cmb_index(QSL("cmbDMStructAppCount"))) != 0) {
                QString id;
                int id1 = get_spn_val(QSL("spnDMStructAppID"));
                int id2 = get_spn_val(QSL("spnDMStructAppID2"));
                m_bc.bc.setStructApp(item_val + 1, get_cmb_index(QSL("cmbDMStructAppIndex")) + 1,
                            id.setNum(id1 * 1000 + id2));
            }
            break;

        case BARCODE_MAILMARK_2D:
            m_bc.bc.setSymbol(BARCODE_MAILMARK_2D);

            if ((item_val = get_cmb_index(QSL("cmbMailmark2DSize")))) {
                m_bc.bc.setOption2(item_val == 1 ? 8 : item_val == 2 ? 10 : 30);
            }

            if (!item_val) {
                // Suppressing rectangles only makes sense if in automatic size mode
                m_optionWidget->findChild<QLabel*>(QSL("lblMailmark2DAutoSize"))->setEnabled(true);
                m_optionWidget->findChild<QCheckBox*>(QSL("chkMailmark2DRectangle"))->setEnabled(true);
                if (m_optionWidget->findChild<QCheckBox*>(QSL("chkMailmark2DRectangle"))->isChecked()) {
                    m_bc.bc.setOption3(DM_SQUARE);
                }
            } else {
                m_optionWidget->findChild<QLabel*>(QSL("lblMailmark2DAutoSize"))->setEnabled(false);
                m_optionWidget->findChild<QCheckBox*>(QSL("chkMailmark2DRectangle"))->setEnabled(false);
                m_bc.bc.setOption3(0);
            }

            break;

        case BARCODE_ITF14:
            m_bc.bc.setSymbol(BARCODE_ITF14);
            if (get_chk_val(QSL("chkITF14NoQuietZones"))) {
                m_bc.bc.setNoQuietZones(true);
            }
            break;

        case BARCODE_PZN:
            m_bc.bc.setSymbol(BARCODE_PZN);
            if (get_chk_val(QSL("chkPZN7"))) {
                m_bc.bc.setOption2(1);
            }
            break;

        case BARCODE_QRCODE:
            if (get_rad_val(QSL("radQRHIBC")))
                m_bc.bc.setSymbol(BARCODE_HIBC_QR);
            else
                m_bc.bc.setSymbol(BARCODE_QRCODE);

            set_gs1_mode(get_rad_val(QSL("radQRGS1")));

            if ((item_val = get_cmb_index(QSL("cmbQRSize"))) != 0) {
                m_bc.bc.setOption2(item_val);
            }
            if ((item_val = get_cmb_index(QSL("cmbQRECC"))) != 0) {
                m_bc.bc.setOption1(item_val);
            }
            if ((item_val = get_cmb_index(QSL("cmbQRMask"))) != 0) {
                m_bc.bc.setOption3((item_val << 8) | m_bc.bc.option3());
            }
            if (get_chk_val(QSL("chkQRFullMultibyte"))) {
                m_bc.bc.setOption3(ZINT_FULL_MULTIBYTE | m_bc.bc.option3());
            }
            if (get_chk_val(QSL("chkQRFast"))) {
                m_bc.bc.setInputMode(FAST_MODE | m_bc.bc.inputMode());
            }
            if ((item_val = get_cmb_index(QSL("cmbQRStructAppCount"))) != 0) {
                QString id;
                int id_val = get_spn_val(QSL("spnQRStructAppID"));
                m_bc.bc.setStructApp(item_val + 1, get_cmb_index(QSL("cmbQRStructAppIndex")) + 1, id.setNum(id_val));
            }
            break;

        case BARCODE_MICROQR:
            m_bc.bc.setSymbol(BARCODE_MICROQR);
            if ((item_val = get_cmb_index(QSL("cmbMQRSize"))) != 0) {
                m_bc.bc.setOption2(item_val);
            }
            if ((item_val = get_cmb_index(QSL("cmbMQRECC"))) != 0) {
                m_bc.bc.setOption1(item_val);
            }
            if ((item_val = get_cmb_index(QSL("cmbMQRMask"))) != 0) {
                m_bc.bc.setOption3((item_val << 8) | m_bc.bc.option3());
            }
            if (get_chk_val(QSL("chkMQRFullMultibyte"))) {
                m_bc.bc.setOption3(ZINT_FULL_MULTIBYTE | m_bc.bc.option3());
            }
            break;

        case BARCODE_UPNQR:
            m_bc.bc.setSymbol(BARCODE_UPNQR);
            cmbECI->setCurrentIndex(2 /*ECI 4*/);
            if ((item_val = get_cmb_index(QSL("cmbUPNQRMask"))) != 0) {
                m_bc.bc.setOption3((item_val << 8) | m_bc.bc.option3());
            }
            if (get_chk_val(QSL("chkUPNQRFast"))) {
                m_bc.bc.setInputMode(FAST_MODE | m_bc.bc.inputMode());
            }
            break;

        case BARCODE_RMQR:
            m_bc.bc.setSymbol(BARCODE_RMQR);

            set_gs1_mode(get_rad_val(QSL("radRMQRGS1")));

            if ((item_val = get_cmb_index(QSL("cmbRMQRSize"))) != 0) {
                m_bc.bc.setOption2(item_val);
            }
            if ((item_val = get_cmb_index(QSL("cmbRMQRECC"))) != 0) {
                m_bc.bc.setOption1(item_val * 2); // Levels 2 (M) and 4 (H) only
            }
            if (get_chk_val(QSL("chkRMQRFullMultibyte"))) {
                m_bc.bc.setOption3(ZINT_FULL_MULTIBYTE);
            }
            break;

        case BARCODE_GRIDMATRIX:
            m_bc.bc.setSymbol(BARCODE_GRIDMATRIX);
            if ((item_val = get_cmb_index(QSL("cmbGridSize"))) != 0) {
                m_bc.bc.setOption2(item_val);
            }
            if ((item_val = get_cmb_index(QSL("cmbGridECC"))) != 0) {
                m_bc.bc.setOption1(item_val);
            }
            if (get_chk_val(QSL("chkGridFullMultibyte"))) {
                m_bc.bc.setOption3(ZINT_FULL_MULTIBYTE);
            }
            if ((item_val = get_cmb_index(QSL("cmbGridStructAppCount"))) != 0) {
                QString id;
                int id_val = get_spn_val(QSL("spnGridStructAppID"));
                m_bc.bc.setStructApp(item_val + 1, get_cmb_index(QSL("cmbGridStructAppIndex")) + 1,
                            id.setNum(id_val));
            }
            break;

        case BARCODE_MAXICODE:
            m_bc.bc.setSymbol(BARCODE_MAXICODE);
            if (get_cmb_index(QSL("cmbMaxiMode")) == 0) {
                m_bc.bc.setOption1(0); // Auto-determine mode 2 or 3 from primary message (checks that it isn't empty)
                m_bc.bc.setPrimaryMessage(QString::asprintf("%s%03d%03d",
                                    get_txt_val(QSL("txtMaxiSCMPostcode")).toUtf8().constData(),
                                    get_spn_val(QSL("spnMaxiSCMCountry")), get_spn_val(QSL("spnMaxiSCMService"))));
                QCheckBox *chkMaxiSCMVV = m_optionWidget->findChild<QCheckBox*>(QSL("chkMaxiSCMVV"));
                if (chkMaxiSCMVV && chkMaxiSCMVV->isEnabled() && chkMaxiSCMVV->isChecked()) {
                    m_bc.bc.setOption2(get_spn_val(QSL("spnMaxiSCMVV")) + 1);
                }
            } else {
                m_bc.bc.setOption1(get_cmb_index(QSL("cmbMaxiMode")) + 3);
            }

            if ((item_val = get_cmb_index(QSL("cmbMaxiStructAppCount"))) != 0) {
                QString id; // Dummy ID
                m_bc.bc.setStructApp(item_val + 1, get_cmb_index(QSL("cmbMaxiStructAppIndex")) + 1, id);
            }
            break;

        case BARCODE_CHANNEL:
            m_bc.bc.setSymbol(BARCODE_CHANNEL);
            if ((item_val = get_cmb_index(QSL("cmbChannel"))) == 0)
                m_bc.bc.setOption2(0);
            else
                m_bc.bc.setOption2(item_val + 2);
            break;

        case BARCODE_CODEONE:
            m_bc.bc.setSymbol(BARCODE_CODEONE);
            m_bc.bc.setOption2(get_cmb_index(QSL("cmbC1Size")));
            if (m_bc.bc.option2() == 9) { // Version S
                eci_not_set = false;
                cmbECI->setEnabled(false);
                lblECI->setEnabled(false);
                m_optionWidget->findChild<QRadioButton*>(QSL("radC1GS1"))->setEnabled(false);
            } else {
                m_optionWidget->findChild<QRadioButton*>(QSL("radC1GS1"))->setEnabled(true);
                set_gs1_mode(get_rad_val(QSL("radC1GS1")));
            }
            if (get_cmb_index(QSL("cmbC1Size")) != 9 && (item_val = get_spn_val(QSL("spnC1StructAppCount"))) > 1) {
                QString id; // Dummy ID
                m_bc.bc.setStructApp(item_val, get_spn_val(QSL("spnC1StructAppIndex")), id);
            }
            break;

        case BARCODE_CODE49:
            m_bc.bc.setSymbol(BARCODE_CODE49);
            set_gs1_mode(get_rad_val(QSL("radC49GS1")));
            if ((item_val = get_cmb_index(QSL("cmbC49Rows"))) != 0) {
                m_bc.bc.setOption1(item_val + 2); // Starts at 3
            }
            // Row separator height selection uses option 3 in zint_symbol
            if ((item_val = get_cmb_index(QSL("cmbC49RowSepHeight"))) != 0) {
                m_bc.bc.setOption3(item_val + 1); // Zero-based
            }
            if (get_chk_val(QSL("chkC49NoQuietZones"))) {
                m_bc.bc.setNoQuietZones(true);
            }
            break;

        case BARCODE_CODE93:
            m_bc.bc.setSymbol(BARCODE_CODE93);
            if (get_chk_val(QSL("chkC93ShowChecks"))) {
                m_bc.bc.setOption2(1);
            }
            break;

        case BARCODE_HANXIN:
            m_bc.bc.setSymbol(BARCODE_HANXIN);
            if ((item_val = get_cmb_index(QSL("cmbHXSize"))) != 0) {
                m_bc.bc.setOption2(item_val);
            }
            if ((item_val = get_cmb_index(QSL("cmbHXECC"))) != 0) {
                m_bc.bc.setOption1(item_val);
            }
            if ((item_val = get_cmb_index(QSL("cmbHXMask"))) != 0) {
                m_bc.bc.setOption3((item_val << 8) | m_bc.bc.option3());
            }
            if (get_chk_val(QSL("chkHXFullMultibyte"))) {
                m_bc.bc.setOption3(ZINT_FULL_MULTIBYTE | m_bc.bc.option3());
            }
            break;

        case BARCODE_ULTRA:
            m_bc.bc.setSymbol(BARCODE_ULTRA);
            if (get_rad_val(QSL("radUltraEcc")))
                m_bc.bc.setOption1(get_cmb_index(QSL("cmbUltraEcc")) + 1);

            set_gs1_mode(get_rad_val(QSL("radUltraGS1")));

            if ((item_val = get_cmb_index(QSL("cmbUltraRevision"))) > 0) {
                m_bc.bc.setOption2(item_val + 1); // Combobox 0-based
            }
            if ((item_val = get_cmb_index(QSL("cmbUltraStructAppCount"))) != 0) {
                QString id;
                int id_val = get_spn_val(QSL("spnUltraStructAppID"));
                m_bc.bc.setStructApp(item_val + 1, get_cmb_index(QSL("cmbUltraStructAppIndex")) + 1,
                            id.setNum(id_val));
            }
            break;

        case BARCODE_VIN:
            m_bc.bc.setSymbol(BARCODE_VIN);
            if (get_chk_val(QSL("chkVINImportChar"))) {
                m_bc.bc.setOption2(m_bc.bc.option2() | 1); // Import character 'I' prefix
            }
            break;

        default:
            m_bc.bc.setSymbol(symbology);
            break;
    }
    m_symbology = m_bc.bc.symbol();

    if (eci_not_set) {
        cmbECI->setEnabled(m_bc.bc.supportsECI());
        lblECI->setEnabled(cmbECI->isEnabled());
    }
    btnClearData->setEnabled(!txtData->text().isEmpty());
    chkGS1Parens->setEnabled(m_bc.bc.takesGS1AIData(m_symbology) || (m_bc.bc.inputMode() & 0x07) == GS1_MODE);
    chkGS1NoCheck->setEnabled(chkGS1Parens->isEnabled());
    chkRInit->setEnabled(m_bc.bc.supportsReaderInit() && (m_bc.bc.inputMode() & 0x07) != GS1_MODE);
    chkCompliantHeight->setEnabled(m_bc.bc.hasCompliantHeight());

    if (!grpComposite->isHidden() && chkComposite->isChecked())
        m_bc.bc.setOption1(cmbCompType->currentIndex());

    if (!chkAutoHeight->isEnabled() || chkAutoHeight->isChecked()) {
        m_bc.bc.setHeight(0);
    } else {
        if (m_spnHeightPerRow && m_spnHeightPerRow->isEnabled() && m_spnHeightPerRow->value()) {
            // This causes a double-encode unfortunately, as heightb gets synced
            m_bc.bc.setInputMode(m_bc.bc.inputMode() | HEIGHTPERROW_MODE);
            m_bc.bc.setHeight(m_spnHeightPerRow->value());
        } else {
            m_bc.bc.setHeight(heightb->value());
        }
    }
    m_bc.bc.setCompliantHeight(chkCompliantHeight->isEnabled() && chkCompliantHeight->isChecked());
    m_bc.bc.setECI(cmbECI->isEnabled() ? cmbECI->currentIndex() : 0);
    m_bc.bc.setGS1Parens(chkGS1Parens->isEnabled() && chkGS1Parens->isChecked());
    m_bc.bc.setGS1NoCheck(chkGS1NoCheck->isEnabled() && chkGS1NoCheck->isChecked());
    m_bc.bc.setReaderInit(chkRInit->isEnabled() && chkRInit->isChecked());
    m_bc.bc.setShowText(chkHRTShow->isEnabled() && chkHRTShow->isChecked());
    m_bc.bc.setBorderType(btype->currentIndex());
    m_bc.bc.setBorderWidth(bwidth->value());
    m_bc.bc.setWhitespace(spnWhitespace->value());
    m_bc.bc.setVWhitespace(spnVWhitespace->value());
    m_bc.bc.setQuietZones(chkQuietZones->isEnabled() && chkQuietZones->isChecked());
    m_bc.bc.setFontSetting(cmbFontSetting->currentIndex());
    m_bc.bc.setTextGap(spnTextGap->value());
    m_bc.bc.setRotateAngle(cmbRotate->currentIndex());
    m_bc.bc.setEmbedVectorFont(chkEmbedVectorFont->isEnabled() && chkEmbedVectorFont->isChecked());
    m_bc.bc.setDotty(chkDotty->isEnabled() && chkDotty->isChecked());
    if (m_symbology == BARCODE_DOTCODE || (chkDotty->isEnabled() && chkDotty->isChecked())) {
        m_bc.bc.setDotSize(spnDotSize->value());
    }
    m_bc.bc.setFgStr(m_fgstr);
    m_bc.bc.setBgStr(m_bgstr);
    m_bc.bc.setScale((float) spnScale->value());
    change_cmyk();
    m_bc.setSize(width - 10, height - 10);
    m_bc.update();
    scene->setSceneRect(m_bc.boundingRect());
    scene->update();
}

void MainWindow::createActions()
{
    // SVG icons from https://github.com/feathericons/feather
    // MIT license - see site and "frontend_qt/res/LICENSE_feathericons"
    QIcon menuIcon(QSL(":res/menu.svg"));
    QIcon copyIcon(QIcon::fromTheme(QSL("edit-copy"), QIcon(QSL(":res/copy.svg"))));
    QIcon cliIcon(QSL(":res/zint_black.ico"));
    QIcon saveIcon(QIcon::fromTheme(QSL("document-save"), QIcon(QSL(":res/download.svg"))));
    QIcon zapIcon(QSL(":res/zap.svg"));
    QIcon aboutIcon(QSL(":res/zint-qt.ico"));
    QIcon helpIcon(QIcon::fromTheme(QSL("help-contents"), QIcon(QSL(":res/help-circle.svg"))));
    QIcon previewBgIcon(QSL(":res/monitor-bg.svg"));
    QIcon quitIcon(QIcon::fromTheme(QSL("window-close"), QIcon(QSL(":res/x.svg"))));

    btnMenu->setIcon(menuIcon);
    btnCopyBMP->setIcon(copyIcon);
    btnCopySVG->setIcon(copyIcon);
    btnSave->setIcon(saveIcon); // Makes it (too) big but live with it for a while after "Save As..." -> "Save..."
    btnExit->setIcon(quitIcon);

    m_copyBMPAct = new QAction(copyIcon, tr("Copy as &BMP"), this);
    m_copyBMPAct->setStatusTip(tr("Copy to clipboard as BMP"));
    m_copyBMPAct->setShortcut(*copyBMPSeq);
    connect(m_copyBMPAct, SIGNAL(triggered()), this, SLOT(copy_to_clipboard_bmp()));

    m_copyEMFAct = new QAction(copyIcon, tr("Copy as E&MF"), this);
    m_copyEMFAct->setStatusTip(tr("Copy to clipboard as EMF"));
    m_copyEMFAct->setShortcut(*copyEMFSeq);
    connect(m_copyEMFAct, SIGNAL(triggered()), this, SLOT(copy_to_clipboard_emf()));

#ifdef MAINWINDOW_COPY_EPS /* TODO: see if can get this to work */
    m_copyEPSAct = new QAction(copyIcon, tr("Copy as &EPS"), this);
    m_copyEPSAct->setStatusTip(tr("Copy to clipboard as EPS"));
    connect(m_copyEPSAct, SIGNAL(triggered()), this, SLOT(copy_to_clipboard_eps()));
#endif

    m_copyGIFAct = new QAction(copyIcon, tr("Copy as &GIF"), this);
    m_copyGIFAct->setStatusTip(tr("Copy to clipboard as GIF"));
    m_copyGIFAct->setShortcut(*copyGIFSeq);
    connect(m_copyGIFAct, SIGNAL(triggered()), this, SLOT(copy_to_clipboard_gif()));

#ifdef MAINWINDOW_COPY_PCX /* TODO: see if can get this to work */
    m_copyPCXAct = new QAction(copyIcon, tr("Copy as P&CX"), this);
    m_copyPCXAct->setStatusTip(tr("Copy to clipboard as PCX"));
    connect(m_copyPCXAct, SIGNAL(triggered()), this, SLOT(copy_to_clipboard_pcx()));
#endif

    if (!m_bc.bc.noPng()) {
        m_copyPNGAct = new QAction(copyIcon, tr("Copy as &PNG"), this);
        m_copyPNGAct->setStatusTip(tr("Copy to clipboard as PNG"));
        m_copyPNGAct->setShortcut(*copyPNGSeq);
        connect(m_copyPNGAct, SIGNAL(triggered()), this, SLOT(copy_to_clipboard_png()));
    }

    m_copySVGAct = new QAction(copyIcon, tr("Copy as S&VG"), this);
    m_copySVGAct->setStatusTip(tr("Copy to clipboard as SVG"));
    m_copySVGAct->setShortcut(*copySVGSeq);
    connect(m_copySVGAct, SIGNAL(triggered()), this, SLOT(copy_to_clipboard_svg()));

    m_copyTIFAct = new QAction(copyIcon, tr("Copy as &TIF"), this);
    m_copyTIFAct->setStatusTip(tr("Copy to clipboard as TIF"));
    m_copyTIFAct->setShortcut(*copyTIFSeq);
    connect(m_copyTIFAct, SIGNAL(triggered()), this, SLOT(copy_to_clipboard_tif()));

    m_openCLIAct = new QAction(cliIcon, tr("C&LI Equivalent..."), this);
    m_openCLIAct->setStatusTip(tr("Generate CLI equivalent"));
    m_openCLIAct->setShortcut(*openCLISeq);
    connect(m_openCLIAct, SIGNAL(triggered()), this, SLOT(open_cli_dialog()));

    m_saveAsAct = new QAction(saveIcon, tr("&Save As..."), this);
    m_saveAsAct->setStatusTip(tr("Output image to file"));
    m_saveAsAct->setShortcut(QKeySequence::Save);
    connect(m_saveAsAct, SIGNAL(triggered()), this, SLOT(save()));

    m_factoryResetAct = new QAction(zapIcon, tr("Factory &Reset..."), this);
    m_factoryResetAct->setStatusTip(tr("Clear all data & settings"));
    m_factoryResetAct->setShortcut(*factoryResetSeq);
    connect(m_factoryResetAct, SIGNAL(triggered()), this, SLOT(factory_reset()));

    m_helpAct = new QAction(helpIcon, tr("&Help (online)"), this);
    m_helpAct->setStatusTip(tr("Online manual"));
    m_helpAct->setShortcut(QKeySequence::HelpContents);
    connect(m_helpAct, SIGNAL(triggered()), this, SLOT(help()));

    m_previewBgColorAct = new QAction(previewBgIcon, tr("Set preview bac&kground..."), this);
    m_previewBgColorAct->setStatusTip(tr("Set preview background colour"));
    connect(m_previewBgColorAct, SIGNAL(triggered()), this, SLOT(preview_bg()));

    m_aboutAct = new QAction(aboutIcon, tr("&About..."), this);
    m_aboutAct->setStatusTip(tr("About Zint Barcode Studio"));
    connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    m_quitAct = new QAction(quitIcon, tr("&Quit"), this);
    m_quitAct->setStatusTip(tr("Exit Zint Barcode Studio"));
    m_quitAct->setShortcut(*quitKeySeq);
    connect(m_quitAct, SIGNAL(triggered()), this, SLOT(quit_now()));

    m_copyErrtxtAct = new QAction(copyIcon, tr("&Copy"), this);
    m_copyErrtxtAct->setStatusTip(tr("Copy message to clipboard"));
    connect(m_copyErrtxtAct, SIGNAL(triggered()), this, SLOT(copy_to_clipboard_errtxt()));
}

void MainWindow::createMenu()
{
    m_menu = new QMenu(tr("Menu"), this);

    m_menu->addAction(m_copyBMPAct);
    m_menu->addAction(m_copyEMFAct);
#ifdef MAINWINDOW_COPY_EPS
    m_menu->addAction(m_copyEPSAct);
#endif
    m_menu->addAction(m_copyGIFAct);
#ifdef MAINWINDOW_COPY_PCX
    m_menu->addAction(m_copyPCXAct);
#endif
    if (!m_bc.bc.noPng()) {
        m_menu->addAction(m_copyPNGAct);
    }
    m_menu->addAction(m_copySVGAct);
    m_menu->addAction(m_copyTIFAct);
    m_menu->addSeparator();

    m_menu->addAction(m_openCLIAct);
    m_menu->addSeparator();
    m_menu->addAction(m_saveAsAct);
    m_menu->addSeparator();
    m_menu->addAction(m_factoryResetAct);
    m_menu->addSeparator();
    m_menu->addAction(m_helpAct);
    m_menu->addAction(m_aboutAct);
    m_menu->addSeparator();
    m_menu->addAction(m_quitAct);
}

void MainWindow::enableActions()
{
    const bool enabled = m_bc.bc.getError() < ZINT_ERROR;
    btnCopyBMP->setEnabled(enabled);
    btnCopySVG->setEnabled(enabled);
    btnSave->setEnabled(enabled);

    m_copyBMPAct->setEnabled(enabled);
    m_copyEMFAct->setEnabled(enabled);
#ifdef MAINWINDOW_COPY_EPS
    m_copyEPSAct->setEnabled(enabled);
#endif
    m_copyGIFAct->setEnabled(enabled);
#ifdef MAINWINDOW_COPY_PCX
    m_copyPCXAct->setEnabled(enabled);
#endif
    if (!m_bc.bc.noPng()) {
        m_copyPNGAct->setEnabled(enabled);
    }
    m_copySVGAct->setEnabled(enabled);
    m_copyTIFAct->setEnabled(enabled);
    m_openCLIAct->setEnabled(enabled);
    m_saveAsAct->setEnabled(enabled);

    m_saveAsShortcut->setEnabled(enabled);
    m_openCLIShortcut->setEnabled(enabled);
    m_copyBMPShortcut->setEnabled(enabled);
    m_copyEMFShortcut->setEnabled(enabled);
    m_copyGIFShortcut->setEnabled(enabled);
    if (!m_bc.bc.noPng()) {
        m_copyPNGShortcut->setEnabled(enabled);
    }
    m_copySVGShortcut->setEnabled(enabled);
    m_copyTIFShortcut->setEnabled(enabled);
}

void MainWindow::copy_to_clipboard(const QString &filename, const QString& name, const char *mimeType)
{
    QClipboard *clipboard = QGuiApplication::clipboard();

    if (!m_bc.bc.save_to_file(filename)) {
        return;
    }

    QMimeData *mdata = new QMimeData;
    if (mimeType) {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            delete mdata;
        } else {
            mdata->setData(mimeType, file.readAll());
            file.close();
            clipboard->setMimeData(mdata, QClipboard::Clipboard);
            /*: %1 is format (BMP/EMF etc) */
            statusBar->showMessage(tr("Copied to clipboard as %1").arg(name), 0 /*No timeout*/);
        }
    } else {
        mdata->setImageData(QImage(filename));
        clipboard->setMimeData(mdata, QClipboard::Clipboard);
        /*: %1 is format (BMP/EMF etc) */
        statusBar->showMessage(tr("Copied to clipboard as %1").arg(name), 0 /*No timeout*/);
    }

    QFile::remove(filename);
}

void MainWindow::errtxtBar_clear()
{
    errtxtBar->clearMessage();
    if (!errtxtBarContainer->isHidden()) {
        errtxtBarContainer->hide();
        errtxtBarContainer->update();
        update_preview(); // This causes a double-encode unfortunately
    }
}

void MainWindow::errtxtBar_set()
{
    const bool isError = m_bc.bc.getError() >= ZINT_ERROR;
    if (!m_bc.bc.hasErrors()) {
        errtxtBar_clear();
        view->setMinimumSize(QSize(0, 35));
    } else {
        view->setMinimumSize(QSize(0, 5));
        errtxtBar->showMessage(m_bc.bc.lastError());
        errtxtBar->setStyleSheet(isError
                    ? QSL("QStatusBar {background:white; color:#dd0000;}")
                    : QSL("QStatusBar {background:white; color:#ff6f00;}"));
        if (errtxtBarContainer->isHidden()) {
            errtxtBarContainer->show();
            errtxtBarContainer->update();
            update_preview(); // This causes a double-encode unfortunately
        }
    }
}

void MainWindow::automatic_info_set()
{
    if (!m_optionWidget) {
        return;
    }
    const int symbology = bstyle_items[bstyle->currentIndex()].symbology;
    const bool isError = m_bc.bc.getError() >= ZINT_ERROR;
    QLineEdit *txt;
    QComboBox *cmb;

    if (symbology == BARCODE_AZTEC || symbology == BARCODE_HIBC_AZTEC) {
        if ((txt = m_optionWidget->findChild<QLineEdit*>(QSL("txtAztecAutoInfo")))) {
            if (!isError && !get_rad_val(QSL("radAztecSize"))) {
                const int w = m_bc.bc.encodedWidth();
                if (w <= 27) { // Note Zint always favours Compact on automatic
                    txt->setText(QString::asprintf("(%d X %d Compact)", w, w));
                } else {
                    int layers;
                    if (w <= 95) {
                        layers = (w - 16 + (w <= 61)) / 4;
                    } else {
                        layers = (w - 20 + (w <= 125) * 2) / 4;
                    }
                    txt->setText(QString::asprintf("(%d X %d (%d Layers))", w, w, layers));
                }
            } else {
                txt->setText(QSEmpty);
            }
        }

    } else if (symbology == BARCODE_CHANNEL) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbChannel")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int channels = (m_bc.bc.encodedWidth() - 7) / 4;
                cmb->setItemText(0, QString::asprintf("Automatic (%d)", channels));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_CODABLOCKF || symbology == BARCODE_HIBC_BLOCKF) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbCbfWidth")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int data_w = (m_bc.bc.encodedWidth() - 57) / 11;
                cmb->setItemText(0, QString::asprintf("Automatic (%d (%d data))", data_w + 5, data_w));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbCbfHeight")))) {
            if (!isError && cmb->currentIndex() == 0) {
                cmb->setItemText(0, QString::asprintf("Automatic (%d)", m_bc.bc.encodedRows()));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_CODE16K) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbC16kRows")))) {
            if (!isError && cmb->currentIndex() == 0) {
                cmb->setItemText(0, QString::asprintf("Automatic (%d)", m_bc.bc.encodedRows()));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_CODE49) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbC49Rows")))) {
            if (!isError && cmb->currentIndex() == 0) {
                cmb->setItemText(0, QString::asprintf("Automatic (%d)", m_bc.bc.encodedRows()));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_CODEONE) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbC1Size")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int r = m_bc.bc.encodedRows();
                const int w = m_bc.bc.encodedWidth();
                // Note Versions S & T not used by Zint in automatic mode
                static const char vers[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
                int idx;
                if (r <= 40) {
                    idx = (r == 22) + (r == 28) * 2 + (r == 40) * 3;
                } else {
                    idx = (r == 70) + (r == 104) * 2 + (r == 148) * 3 + 4;
                }
                cmb->setItemText(0, QString::asprintf("Automatic (%d X %d (Version %c))", r, w, vers[idx]));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_DATAMATRIX || symbology == BARCODE_HIBC_DM) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbDM200Size")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int r = m_bc.bc.encodedRows();
                const int w = m_bc.bc.encodedWidth();
                int z = 0;
                if (r == w) {
                    if (r <= 26) {
                        z = (r - 8) / 2;
                    } else if (r <= 52) {
                        z = 10 + (r - 32) / 4;
                    } else if (r <= 104) {
                        z = 16 + (r - 64) / 8;
                    } else {
                        z = 22 + (r - 120) / 12;
                    }
                    cmb->setItemText(0, QString::asprintf("Automatic (%d x %d (Zint %d))", r, w, z));
                } else if ((r == 8 && (w == 18 || w == 32)) || (r == 12 && (w == 26 || w == 36))
                            || (r == 16 && (w == 36 || w == 48))) {
                    z = 25 + (w == 32) + (w == 26) * 2 + (r == 12 && w == 36) * 3
                            + (r == 16 && w == 36) * 4 + (w == 48) * 5;
                    cmb->setItemText(0, QString::asprintf("Automatic (%d x %d (Zint %d))", r, w, z));
                } else { // DMRE
                    if (r == 8) {
                        z = 31 + (w == 64) + (w == 80) * 2 + (w == 96) * 3 + (w == 120) * 4 + (w == 144) * 5;
                    } else if (r == 12) {
                        z = 37 + (w == 88);
                    } else if (r == 16) {
                        z = 39;
                    } else if (r == 20) {
                        z = 40 + (w == 44) + (w == 64) * 2;
                    } else if (r == 22) {
                        z = 43;
                    } else if (r == 24) {
                        z = 44 + (w == 64);
                    } else { /* if (r == 26) */
                        z = 46 + (w == 48) + (w == 64) * 2;
                    }
                    cmb->setItemText(0, QString::asprintf("Automatic (%d x %d (DMRE) (Zint %d))", r, w, z));
                }
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_MAILMARK_2D) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbMailmark2DSize")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int r = m_bc.bc.encodedRows();
                const int w = m_bc.bc.encodedWidth();
                int z;
                if (r == w) {
                    z = r <= 26 ? 8 : 10;
                    cmb->setItemText(0, QString::asprintf("Automatic (%d x %d (Zint %d) - Type %d)", r, w, z, z - 1));
                } else {
                    z = 30;
                    cmb->setItemText(0, QString::asprintf("Automatic (%d x %d (Zint %d) - Type %d)", r, w, z, z - 1));
                }
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_DOTCODE) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbDotCols")))) {
            if (!isError && cmb->currentIndex() == 0) {
                cmb->setItemText(0, QString::asprintf("Automatic (%d)", m_bc.bc.encodedWidth()));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_GRIDMATRIX) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbGridSize")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int r = m_bc.bc.encodedRows();
                cmb->setItemText(0, QString::asprintf("Automatic (%d x %d (Version %d))", r, r, (r - 6) / 12));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_HANXIN) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbHXSize")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int r = m_bc.bc.encodedRows();
                cmb->setItemText(0, QString::asprintf("Automatic (%d x %d (Version %d))", r, r, (r - 21) / 2));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_MICROPDF417 || symbology == BARCODE_HIBC_MICPDF) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbMPDFCols")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int w = m_bc.bc.encodedWidth();
                int cols;
                if (w == 38) {
                    cols = 1;
                } else if (w == 55) {
                    cols = 2;
                } else if (w == 82) {
                    cols = 3;
                } else { /* if (w == 99) */
                    cols = 4;
                }
                cmb->setItemText(0, QString::asprintf("Automatic (%d)", cols));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_MICROQR) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbMQRSize")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int r = m_bc.bc.encodedRows();
                cmb->setItemText(0, QString::asprintf("Automatic (%d x %d (Version M%d))", r, r, (r - 9) / 2));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_PDF417 || symbology == BARCODE_PDF417COMP || symbology == BARCODE_HIBC_PDF) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbPDFCols")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int w = m_bc.bc.encodedWidth();
                const int overhead = get_rad_val(QSL("radPDFTruncated")) || symbology == BARCODE_PDF417COMP ? 35 : 69;
                cmb->setItemText(0, QString::asprintf("Automatic (%d)", (w - overhead) / 17));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbPDFRows")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int r = m_bc.bc.encodedRows();
                cmb->setItemText(0, QString::asprintf("Automatic (%d)", r));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_QRCODE || symbology == BARCODE_HIBC_QR) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbQRSize")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int r = m_bc.bc.encodedRows();
                cmb->setItemText(0, QString::asprintf("Automatic (%d x %d (Version %d))", r, r, (r - 17) / 4));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_RMQR) {
        if ((cmb = m_optionWidget->findChild<QComboBox*>(QSL("cmbRMQRSize")))) {
            if (!isError && cmb->currentIndex() == 0) {
                const int r = m_bc.bc.encodedRows();
                const int w = m_bc.bc.encodedWidth();
                int z;
                if (r == 11 || r == 13) {
                    z = 11 + (r == 13) * 6 + (w == 43) + (w == 59) * 2 + (w == 77) * 3 + (w == 99) * 4
                            + (w == 139) * 5;
                } else {
                    z = (w == 59) + (w == 77) * 2 + (w == 99) * 3 + (w == 139) * 4;
                    if (r == 7) {
                        z += 1;
                    } else if (r == 9) {
                        z += 6;
                    } else if (r == 15) {
                        z += 23;
                    } else { /* if (r == 17) */
                        z += 28;
                    }
                }
                cmb->setItemText(0, QString::asprintf("Automatic (R%dx%d (Zint %d))", r, w, z));
            } else {
                cmb->setItemText(0, QSL("Automatic"));
            }
        }

    } else if (symbology == BARCODE_ULTRA) {
        if ((txt = m_optionWidget->findChild<QLineEdit*>(QSL("txtUltraAutoInfo")))) {
            if (!isError) {
                const int w = m_bc.bc.encodedWidth();
                const int r = m_bc.bc.encodedRows();
                txt->setText(QString::asprintf("(%d X %d)", w, r));
            } else {
                txt->setText(QSEmpty);
            }
        }
    }
}

QPoint MainWindow::get_context_menu_pos(const QPoint &pos, QWidget *widget)
{
    QPoint menuPos(pos);
    if (menuPos.x() == 0 && menuPos.y() == 0) { // May have been invoked by menu key
        QPoint mousePos(widget->mapFromGlobal(QCursor::pos()));
        if (widget->rect().contains(mousePos)) {
            menuPos = mousePos;
        }
    }

    return widget->mapToGlobal(menuPos);
}

/* Shorthand to find widget child as generic QWidget */
QWidget *MainWindow::get_widget(const QString &name)
{
    return m_optionWidget ? m_optionWidget->findChild<QWidget*>(name) : nullptr;
}

/* Return settings subsection name for a symbol */
QString MainWindow::get_setting_name(int symbology)
{
    switch (symbology) {
        case BARCODE_CODE128AB:
        case BARCODE_GS1_128:
        case BARCODE_GS1_128_CC:
        case BARCODE_HIBC_128:
            symbology = BARCODE_CODE128;
            break;
        case BARCODE_PDF417COMP:
        case BARCODE_HIBC_PDF:
            symbology = BARCODE_PDF417;
            break;
        case BARCODE_HIBC_MICPDF:
            symbology = BARCODE_MICROPDF417;
            break;
        case BARCODE_HIBC_AZTEC:
            symbology = BARCODE_AZTEC;
            break;
        case BARCODE_HIBC_39:
            symbology = BARCODE_CODE39;
            break;
        case BARCODE_HIBC_BLOCKF:
            symbology = BARCODE_CODABLOCKF;
            break;
        case BARCODE_HIBC_DM:
            symbology = BARCODE_DATAMATRIX;
            break;
        case BARCODE_HIBC_QR:
            symbology = BARCODE_QRCODE;
            break;
        case BARCODE_DBAR_OMN_CC:
            symbology = BARCODE_DBAR_OMN;
            break;
        case BARCODE_DBAR_OMNSTK_CC:
            symbology = BARCODE_DBAR_OMNSTK;
            break;
        case BARCODE_DBAR_LTD_CC:
            symbology = BARCODE_DBAR_LTD;
            break;
        case BARCODE_DBAR_STK_CC:
            symbology = BARCODE_DBAR_STK;
            break;
        case BARCODE_DBAR_EXP_CC:
            symbology = BARCODE_DBAR_EXP;
            break;
        case BARCODE_DBAR_EXPSTK_CC:
            symbology = BARCODE_DBAR_EXPSTK;
            break;
        case BARCODE_UPCA_CHK:
        case BARCODE_UPCA_CC:
            symbology = BARCODE_UPCA;
            break;
        case BARCODE_EANX_CHK:
        case BARCODE_EANX_CC:
            symbology = BARCODE_EANX;
            break;
        case BARCODE_UPCE_CHK:
        case BARCODE_UPCE_CC:
            symbology = BARCODE_UPCE;
            break;
    }
    return Zint::QZint::barcodeName(symbology).mid(8).toLower(); // Strip "BARCODE_" prefix
}

/* Helper to return index of selected radio button in group, checking for NULL */
int MainWindow::get_rad_grp_index(const QStringList &names)
{
    if (m_optionWidget) {
        QRadioButton *radioButton;
        for (int index = 0; index < names.size(); index++) {
            radioButton = m_optionWidget->findChild<QRadioButton*>(names[index]);
            if (radioButton && radioButton->isChecked()) {
                return index;
            }
        }
    }
    return 0;
}

/* Helper to set radio button in group from index in settings, checking for NULL */
void MainWindow::set_rad_from_setting(QSettings &settings, const QString &setting,
            const QStringList &names, int default_val)
{
    if (m_optionWidget) {
        int index = settings.value(setting, default_val).toInt();
        QRadioButton *radioButton;
        if (index >= 0 && index < names.size()) {
            radioButton = m_optionWidget->findChild<QRadioButton*>(names[index]);
        } else {
            radioButton = m_optionWidget->findChild<QRadioButton*>(names[0]);
        }
        if (radioButton) {
            radioButton->setChecked(true);
        }
    }
}

/* Helper to see if radio button checked, checking for NULL and whether enabled */
bool MainWindow::get_rad_val(const QString &name)
{
    QRadioButton *radioButton = m_optionWidget ? m_optionWidget->findChild<QRadioButton*>(name) : nullptr;
    return radioButton && radioButton->isEnabled() && radioButton->isChecked();
}

/* Helper to return index of selected item in combobox, checking for NULL */
int MainWindow::get_cmb_index(const QString &name)
{
    QComboBox *comboBox = m_optionWidget ? m_optionWidget->findChild<QComboBox*>(name) : nullptr;
    return comboBox ? comboBox->currentIndex() : 0;
}

/* Helper to set item in combobox from index in settings, checking for NULL */
void MainWindow::set_cmb_from_setting(QSettings &settings, const QString &setting, const QString &name,
            int default_val)
{
    QComboBox *comboBox = m_optionWidget ? m_optionWidget->findChild<QComboBox*>(name) : nullptr;
    if (comboBox) {
        comboBox->setCurrentIndex(settings.value(setting, default_val).toInt());
    }
}

/* Helper to return if checkbox checked, checking for NULL */
int MainWindow::get_chk_val(const QString &name)
{
    QCheckBox *checkBox = m_optionWidget ? m_optionWidget->findChild<QCheckBox*>(name) : nullptr;
    return checkBox && checkBox->isChecked() ? 1 : 0;
}

/* Helper to set checkbox from settings, checking for NULL */
void MainWindow::set_chk_from_setting(QSettings &settings, const QString &setting, const QString &name,
            int default_val)
{
    QCheckBox *checkBox = m_optionWidget ? m_optionWidget->findChild<QCheckBox*>(name) : nullptr;
    if (checkBox) {
        checkBox->setChecked(settings.value(setting, default_val).toInt() ? true : false);
    }
}

/* Helper to return value of double spinner, checking for NULL */
double MainWindow::get_dspn_val(const QString &name)
{
    QDoubleSpinBox *spinBox = m_optionWidget->findChild<QDoubleSpinBox*>(name);
    return spinBox ? spinBox->value() : 0.0;
}

/* Helper to set double spinner from settings, checking for NULL */
void MainWindow::set_dspn_from_setting(QSettings &settings, const QString &setting, const QString &name,
            float default_val)
{
    QDoubleSpinBox *spinBox = m_optionWidget->findChild<QDoubleSpinBox*>(name);
    if (spinBox) {
        spinBox->setValue(settings.value(setting, default_val).toFloat());
    }
}

/* Helper to return text of line edit, checking for NULL */
QString MainWindow::get_txt_val(const QString &name)
{
    QLineEdit *lineEdit = m_optionWidget ? m_optionWidget->findChild<QLineEdit*>(name) : nullptr;
    return lineEdit ? lineEdit->text() : QSEmpty;
}

/* Helper to set line edit from settings, checking for NULL */
void MainWindow::set_txt_from_setting(QSettings &settings, const QString &setting, const QString &name,
            const QString &default_val)
{
    QLineEdit *lineEdit = m_optionWidget ? m_optionWidget->findChild<QLineEdit*>(name) : nullptr;
    if (lineEdit) {
        lineEdit->setText(settings.value(setting, default_val).toString());
    }
}

/* Helper to return value of spin box, checking for NULL */
int MainWindow::get_spn_val(const QString &name)
{
    QSpinBox *spinBox = m_optionWidget ? m_optionWidget->findChild<QSpinBox*>(name) : nullptr;
    return spinBox ? spinBox->value() : 0;
}

/* Helper to set spin box from settings, checking for NULL */
void MainWindow::set_spn_from_setting(QSettings &settings, const QString &setting, const QString &name,
            int default_val)
{
    QSpinBox *spinBox = m_optionWidget ? m_optionWidget->findChild<QSpinBox*>(name) : nullptr;
    if (spinBox) {
        spinBox->setValue(settings.value(setting, default_val).toInt());
    }
}

/* Save settings for an individual symbol */
void MainWindow::save_sub_settings(QSettings &settings, int symbology)
{
    QString name = get_setting_name(symbology);
    if (!name.isEmpty()) { // Should never be empty
        settings.setValue(QSL("studio/bc/%1/data").arg(name), txtData->text());
        if (!grpSegs->isHidden()) {
            settings.setValue(QSL("studio/bc/%1/data_seg1").arg(name), txtDataSeg1->text());
            settings.setValue(QSL("studio/bc/%1/data_seg2").arg(name), txtDataSeg2->text());
            settings.setValue(QSL("studio/bc/%1/data_seg3").arg(name), txtDataSeg3->text());
        }
        if (!grpComposite->isHidden()) {
            settings.setValue(QSL("studio/bc/%1/composite_text").arg(name), txtComposite->toPlainText());
            settings.setValue(QSL("studio/bc/%1/chk_composite").arg(name), chkComposite->isChecked() ? 1 : 0);
            settings.setValue(QSL("studio/bc/%1/comp_type").arg(name), cmbCompType->currentIndex());
        }
        if (cmbECI->isEnabled()) {
            settings.setValue(QSL("studio/bc/%1/eci").arg(name), cmbECI->currentIndex());
            settings.setValue(QSL("studio/bc/%1/eci_seg1").arg(name), cmbECISeg1->currentIndex());
            settings.setValue(QSL("studio/bc/%1/eci_seg2").arg(name), cmbECISeg2->currentIndex());
            settings.setValue(QSL("studio/bc/%1/eci_seg3").arg(name), cmbECISeg3->currentIndex());
        }
        settings.setValue(QSL("studio/bc/%1/chk_escape").arg(name), chkEscape->isChecked() ? 1 : 0);
        settings.setValue(QSL("studio/bc/%1/chk_data").arg(name), chkData->isChecked() ? 1 : 0);
        if (chkRInit->isEnabled()) {
            settings.setValue(QSL("studio/bc/%1/chk_rinit").arg(name), chkRInit->isChecked() ? 1 : 0);
        }
        settings.setValue(QSL("studio/bc/%1/chk_gs1parens").arg(name), chkGS1Parens->isChecked() ? 1 : 0);
        settings.setValue(QSL("studio/bc/%1/chk_gs1nocheck").arg(name), chkGS1NoCheck->isChecked() ? 1 : 0);
        if (chkAutoHeight->isEnabled()) {
            settings.setValue(
                QSL("studio/bc/%1/appearance/autoheight").arg(name), chkAutoHeight->isChecked() ? 1 : 0);
            settings.setValue(QSL("studio/bc/%1/appearance/height").arg(name), heightb->value());
        }
        if (chkCompliantHeight->isEnabled()) {
            settings.setValue(
                QSL("studio/bc/%1/appearance/compliantheight").arg(name), chkCompliantHeight->isChecked() ? 1 : 0);
        }
        settings.setValue(QSL("studio/bc/%1/appearance/border").arg(name), bwidth->value());
        settings.setValue(QSL("studio/bc/%1/appearance/whitespace").arg(name), spnWhitespace->value());
        settings.setValue(QSL("studio/bc/%1/appearance/vwhitespace").arg(name), spnVWhitespace->value());
        settings.setValue(QSL("studio/bc/%1/appearance/scale").arg(name), spnScale->value());
        settings.setValue(QSL("studio/bc/%1/appearance/border_type").arg(name), btype->currentIndex());
        if (chkHRTShow->isEnabled()) {
            settings.setValue(QSL("studio/bc/%1/appearance/font_setting").arg(name), cmbFontSetting->currentIndex());
            settings.setValue(QSL("studio/bc/%1/appearance/text_gap").arg(name), spnTextGap->value());
            settings.setValue(QSL("studio/bc/%1/appearance/chk_embed_vector_font").arg(name),
                chkEmbedVectorFont->isChecked() ? 1 : 0);
            settings.setValue(QSL("studio/bc/%1/appearance/chk_hrt_show").arg(name), chkHRTShow->isChecked() ? 1 : 0);
        }
        settings.setValue(QSL("studio/bc/%1/appearance/chk_cmyk").arg(name), chkCMYK->isChecked() ? 1 : 0);
        settings.setValue(
            QSL("studio/bc/%1/appearance/chk_quietzones").arg(name), chkQuietZones->isChecked() ? 1 : 0);
        settings.setValue(QSL("studio/bc/%1/appearance/rotate").arg(name), cmbRotate->currentIndex());
        if (symbology == BARCODE_DOTCODE || chkDotty->isEnabled()) {
            settings.setValue(QSL("studio/bc/%1/appearance/chk_dotty").arg(name), chkDotty->isChecked() ? 1 : 0);
            settings.setValue(QSL("studio/bc/%1/appearance/dot_size").arg(name), spnDotSize->value());
        }
        settings.setValue(QSL("studio/bc/%1/ink/text").arg(name), m_fgstr);
        settings.setValue(QSL("studio/bc/%1/paper/text").arg(name), m_bgstr);
        settings.setValue(QSL("studio/bc/%1/xdimdpvars/x_dim").arg(name), m_xdimdpVars.x_dim);
        settings.setValue(QSL("studio/bc/%1/xdimdpvars/x_dim_units").arg(name), m_xdimdpVars.x_dim_units);
        settings.setValue(QSL("studio/bc/%1/xdimdpvars/set").arg(name), m_xdimdpVars.set);
    }

    switch (symbology) {
        case BARCODE_CODE128:
        case BARCODE_CODE128AB:
        case BARCODE_GS1_128:
        case BARCODE_GS1_128_CC:
        case BARCODE_HIBC_128:
            settings.setValue(QSL("studio/bc/code128/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radC128Stand") << QSL("radC128EAN") << QSL("radC128CSup")
                                << QSL("radC128HIBC") << QSL("radC128ExtraEsc")));
            break;

        case BARCODE_PDF417:
        case BARCODE_PDF417COMP:
        case BARCODE_HIBC_PDF:
            settings.setValue(QSL("studio/bc/pdf417/cols"), get_cmb_index(QSL("cmbPDFCols")));
            settings.setValue(QSL("studio/bc/pdf417/rows"), get_cmb_index(QSL("cmbPDFRows")));
            settings.setValue(QSL("studio/bc/pdf417/height_per_row"), get_dspn_val(QSL("spnPDFHeightPerRow")));
            settings.setValue(QSL("studio/bc/pdf417/ecc"), get_cmb_index(QSL("cmbPDFECC")));
            settings.setValue(QSL("studio/bc/pdf417/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radPDFStand") << QSL("radPDFTruncated") << QSL("radPDFHIBC")));
            settings.setValue(QSL("studio/bc/pdf417/chk_fast"), get_chk_val(QSL("chkPDFFast")));
            settings.setValue(QSL("studio/bc/pdf417/structapp_count"), get_spn_val(QSL("spnPDFStructAppCount")));
            settings.setValue(QSL("studio/bc/pdf417/structapp_index"), get_spn_val(QSL("spnPDFStructAppIndex")));
            settings.setValue(QSL("studio/bc/pdf417/structapp_id"), get_txt_val(QSL("txtPDFStructAppID")));
            break;

        case BARCODE_MICROPDF417:
        case BARCODE_HIBC_MICPDF:
            settings.setValue(QSL("studio/bc/micropdf417/cols"), get_cmb_index(QSL("cmbMPDFCols")));
            settings.setValue(QSL("studio/bc/micropdf417/height_per_row"), get_dspn_val(QSL("spnMPDFHeightPerRow")));
            settings.setValue(QSL("studio/bc/micropdf417/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radMPDFStand") << QSL("radMPDFHIBC")));
            settings.setValue(QSL("studio/bc/micropdf417/chk_fast"), get_chk_val(QSL("chkMPDFFast")));
            settings.setValue(QSL("studio/bc/micropdf417/structapp_count"),
                get_spn_val(QSL("spnMPDFStructAppCount")));
            settings.setValue(QSL("studio/bc/micropdf417/structapp_index"),
                get_spn_val(QSL("spnMPDFStructAppIndex")));
            settings.setValue(QSL("studio/bc/micropdf417/structapp_id"), get_txt_val(QSL("txtMPDFStructAppID")));
            break;

        case BARCODE_DOTCODE:
            settings.setValue(QSL("studio/bc/dotcode/cols"), get_cmb_index(QSL("cmbDotCols")));
            settings.setValue(QSL("studio/bc/dotcode/mask"), get_cmb_index(QSL("cmbDotMask")));
            settings.setValue(QSL("studio/bc/dotcode/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radDotStand") << QSL("radDotGS1")));
            settings.setValue(QSL("studio/bc/dotcode/structapp_count"), get_cmb_index(QSL("cmbDotStructAppCount")));
            settings.setValue(QSL("studio/bc/dotcode/structapp_index"), get_cmb_index(QSL("cmbDotStructAppIndex")));
            break;

        case BARCODE_AZTEC:
        case BARCODE_HIBC_AZTEC:
            settings.setValue(QSL("studio/bc/aztec/autoresizing"), get_rad_grp_index(
                QStringList() << QSL("radAztecAuto") << QSL("radAztecSize") << QSL("radAztecECC")));
            settings.setValue(QSL("studio/bc/aztec/size"), get_cmb_index(QSL("cmbAztecSize")));
            settings.setValue(QSL("studio/bc/aztec/ecc"), get_cmb_index(QSL("cmbAztecECC")));
            settings.setValue(QSL("studio/bc/aztec/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radAztecStand") << QSL("radAztecGS1") << QSL("radAztecHIBC")));
            settings.setValue(QSL("studio/bc/aztec/structapp_count"), get_cmb_index(QSL("cmbAztecStructAppCount")));
            settings.setValue(QSL("studio/bc/aztec/structapp_index"), get_cmb_index(QSL("cmbAztecStructAppIndex")));
            settings.setValue(QSL("studio/bc/aztec/structapp_id"), get_txt_val(QSL("txtAztecStructAppID")));
            break;

        case BARCODE_MSI_PLESSEY:
            settings.setValue(QSL("studio/bc/msi_plessey/check_digit"), get_cmb_index(QSL("cmbMSICheck")));
            settings.setValue(QSL("studio/bc/msi_plessey/check_text"), get_chk_val(QSL("chkMSICheckText")));
            break;

        case BARCODE_CODE11:
            settings.setValue(QSL("studio/bc/code11/check_digit"), get_rad_grp_index(
                QStringList() << QSL("radC11TwoCheckDigits") << QSL("radC11OneCheckDigit")
                                << QSL("radC11NoCheckDigits")));
            break;

        case BARCODE_C25STANDARD:
            settings.setValue(QSL("studio/bc/c25standard/check_digit"), get_rad_grp_index(
                QStringList() << QSL("radC25Stand") << QSL("radC25Check") << QSL("radC25CheckHide")));
            break;
        case BARCODE_C25INTER:
            settings.setValue(QSL("studio/bc/c25inter/check_digit"), get_rad_grp_index(
                QStringList() << QSL("radC25Stand") << QSL("radC25Check") << QSL("radC25CheckHide")));
            break;
        case BARCODE_C25IATA:
            settings.setValue(QSL("studio/bc/c25iata/check_digit"), get_rad_grp_index(
                QStringList() << QSL("radC25Stand") << QSL("radC25Check") << QSL("radC25CheckHide")));
            break;
        case BARCODE_C25LOGIC:
            settings.setValue(QSL("studio/bc/c25logic/check_digit"), get_rad_grp_index(
                QStringList() << QSL("radC25Stand") << QSL("radC25Check") << QSL("radC25CheckHide")));
            break;
        case BARCODE_C25IND:
            settings.setValue(QSL("studio/bc/c25ind/check_digit"), get_rad_grp_index(
                QStringList() << QSL("radC25Stand") << QSL("radC25Check") << QSL("radC25CheckHide")));
            break;

        case BARCODE_CODE39:
        case BARCODE_HIBC_39:
            settings.setValue(QSL("studio/bc/code39/check_digit"), get_rad_grp_index(
                QStringList() << QSL("radC39Stand") << QSL("radC39Check") << QSL("radC39HIBC")
                                << QSL("radC39CheckHide")));
            break;

        case BARCODE_EXCODE39:
            settings.setValue(QSL("studio/bc/excode39/check_digit"), get_rad_grp_index(
                QStringList() << QSL("radC39Stand") << QSL("radC39Check") << QSL("radC39CheckHide")));
            break;
        case BARCODE_LOGMARS:
            settings.setValue(QSL("studio/bc/logmars/check_digit"), get_rad_grp_index(
                QStringList() << QSL("radC39Stand") << QSL("radC39Check") << QSL("radC39CheckHide")));
            break;

        case BARCODE_CODE16K:
            settings.setValue(QSL("studio/bc/code16k/rows"), get_cmb_index(QSL("cmbC16kRows")));
            settings.setValue(QSL("studio/bc/code16k/height_per_row"), get_dspn_val(QSL("spnC16kHeightPerRow")));
            settings.setValue(QSL("studio/bc/code16k/row_sep_height"), get_cmb_index(QSL("cmbC16kRowSepHeight")));
            settings.setValue(QSL("studio/bc/code16k/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radC16kStand") << QSL("radC16kGS1")));
            settings.setValue(QSL("studio/bc/code16k/chk_no_quiet_zones"), get_chk_val(QSL("chkC16kNoQuietZones")));
            break;

        case BARCODE_CODABAR:
            settings.setValue(QSL("studio/bc/codabar/check_digit"), get_rad_grp_index(
                QStringList() << QSL("radCodabarStand") << QSL("radCodabarCheckHide") << QSL("radCodabarCheck")));
            break;

        case BARCODE_CODABLOCKF:
        case BARCODE_HIBC_BLOCKF:
            settings.setValue(QSL("studio/bc/codablockf/width"), get_cmb_index(QSL("cmbCbfWidth")));
            settings.setValue(QSL("studio/bc/codablockf/height"), get_cmb_index(QSL("cmbCbfHeight")));
            settings.setValue(QSL("studio/bc/codablockf/height_per_row"), get_dspn_val(QSL("spnCbfHeightPerRow")));
            settings.setValue(QSL("studio/bc/codablockf/row_sep_height"), get_cmb_index(QSL("cmbCbfRowSepHeight")));
            settings.setValue(QSL("studio/bc/codablockf/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radCbfStand") << QSL("radCbfHIBC")));
            settings.setValue(QSL("studio/bc/codablockf/chk_no_quiet_zones"), get_chk_val(QSL("chkCbfNoQuietZones")));
            break;

        case BARCODE_DAFT:
            settings.setValue(QSL("studio/bc/daft/tracker_ratio"),
                QString::number(get_dspn_val(QSL("spnDAFTTrackerRatio")), 'f', 1 /*precision*/));
            break;

        case BARCODE_DPD:
            settings.setValue(QSL("studio/bc/dpd/chk_relabel"), get_chk_val(QSL("chkDPDRelabel")));
            break;

        case BARCODE_DATAMATRIX:
        case BARCODE_HIBC_DM:
            settings.setValue(QSL("studio/bc/datamatrix/size"), get_cmb_index(QSL("cmbDM200Size")));
            settings.setValue(QSL("studio/bc/datamatrix/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radDM200Stand") << QSL("radDM200GS1") << QSL("radDM200HIBC")));
            settings.setValue(QSL("studio/bc/datamatrix/chk_suppress_rect"), get_chk_val(QSL("chkDMRectangle")));
            settings.setValue(QSL("studio/bc/datamatrix/chk_allow_dmre"), get_chk_val(QSL("chkDMRE")));
            settings.setValue(QSL("studio/bc/datamatrix/chk_gs_sep"), get_chk_val(QSL("chkDMGSSep")));
            settings.setValue(QSL("studio/bc/datamatrix/iso_144"), get_chk_val(QSL("chkDMISO144")));
            settings.setValue(QSL("studio/bc/datamatrix/chk_fast"), get_chk_val(QSL("chkDMFast")));
            settings.setValue(QSL("studio/bc/datamatrix/structapp_count"), get_cmb_index(QSL("cmbDMStructAppCount")));
            settings.setValue(QSL("studio/bc/datamatrix/structapp_index"), get_cmb_index(QSL("cmbDMStructAppIndex")));
            settings.setValue(QSL("studio/bc/datamatrix/structapp_id"), get_spn_val(QSL("spnDMStructAppID")));
            settings.setValue(QSL("studio/bc/datamatrix/structapp_id2"), get_spn_val(QSL("spnDMStructAppID2")));
            break;

        case BARCODE_MAILMARK_2D:
            settings.setValue(QSL("studio/bc/mailmark2d/size"), get_cmb_index(QSL("cmbMailmark2DSize")));
            settings.setValue(
                        QSL("studio/bc/mailmark2d/chk_suppress_rect"), get_chk_val(QSL("chkMailmark2DRectangle")));
            break;

        case BARCODE_ITF14:
            settings.setValue(QSL("studio/bc/itf14/chk_no_quiet_zones"), get_chk_val(QSL("chkITF14NoQuietZones")));
            break;

        case BARCODE_PZN:
            settings.setValue(QSL("studio/bc/pzn/chk_pzn7"), get_chk_val(QSL("chkPZN7")));
            break;

        case BARCODE_QRCODE:
        case BARCODE_HIBC_QR:
            settings.setValue(QSL("studio/bc/qrcode/size"), get_cmb_index(QSL("cmbQRSize")));
            settings.setValue(QSL("studio/bc/qrcode/ecc"), get_cmb_index(QSL("cmbQRECC")));
            settings.setValue(QSL("studio/bc/qrcode/mask"), get_cmb_index(QSL("cmbQRMask")));
            settings.setValue(QSL("studio/bc/qrcode/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radDM200Stand") << QSL("radQRGS1") << QSL("radQRHIBC")));
            settings.setValue(QSL("studio/bc/qrcode/chk_full_multibyte"), get_chk_val(QSL("chkQRFullMultibyte")));
            settings.setValue(QSL("studio/bc/qrcode/chk_fast_mode"), get_chk_val(QSL("chkQRFast")));
            settings.setValue(QSL("studio/bc/qrcode/structapp_count"), get_cmb_index(QSL("cmbQRStructAppCount")));
            settings.setValue(QSL("studio/bc/qrcode/structapp_index"), get_cmb_index(QSL("cmbQRStructAppIndex")));
            settings.setValue(QSL("studio/bc/qrcode/structapp_id"), get_spn_val(QSL("spnQRStructAppID")));
            break;

        case BARCODE_UPNQR:
            settings.setValue(QSL("studio/bc/upnqr/mask"), get_cmb_index(QSL("cmbUPNQRMask")));
            settings.setValue(QSL("studio/bc/upnqr/chk_fast_mode"), get_chk_val(QSL("chkUPNQRFast")));
            break;

        case BARCODE_RMQR:
            settings.setValue(QSL("studio/bc/rmqr/size"), get_cmb_index(QSL("cmbRMQRSize")));
            settings.setValue(QSL("studio/bc/rmqr/ecc"), get_cmb_index(QSL("cmbRMQRECC")));
            settings.setValue(QSL("studio/bc/rmqr/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radQRStand") << QSL("radRMQRGS1")));
            settings.setValue(QSL("studio/bc/rmqr/chk_full_multibyte"), get_chk_val(QSL("chkRMQRFullMultibyte")));
            break;

        case BARCODE_HANXIN:
            settings.setValue(QSL("studio/bc/hanxin/size"), get_cmb_index(QSL("cmbHXSize")));
            settings.setValue(QSL("studio/bc/hanxin/ecc"), get_cmb_index(QSL("cmbHXECC")));
            settings.setValue(QSL("studio/bc/hanxin/mask"), get_cmb_index(QSL("cmbHXMask")));
            settings.setValue(QSL("studio/bc/hanxin/chk_full_multibyte"), get_chk_val(QSL("chkHXFullMultibyte")));
            break;

        case BARCODE_MICROQR:
            settings.setValue(QSL("studio/bc/microqr/size"), get_cmb_index(QSL("cmbMQRSize")));
            settings.setValue(QSL("studio/bc/microqr/ecc"), get_cmb_index(QSL("cmbMQRECC")));
            settings.setValue(QSL("studio/bc/microqr/mask"), get_cmb_index(QSL("cmbMQRMask")));
            settings.setValue(QSL("studio/bc/microqr/chk_full_multibyte"), get_chk_val(QSL("chkMQRFullMultibyte")));
            break;

        case BARCODE_GRIDMATRIX:
            settings.setValue(QSL("studio/bc/gridmatrix/size"), get_cmb_index(QSL("cmbGridSize")));
            settings.setValue(QSL("studio/bc/gridmatrix/ecc"), get_cmb_index(QSL("cmbGridECC")));
            settings.setValue(QSL("studio/bc/gridmatrix/chk_full_multibyte"),
                get_chk_val(QSL("chkGridFullMultibyte")));
            settings.setValue(QSL("studio/bc/gridmatrix/structapp_count"),
                get_cmb_index(QSL("cmbGridStructAppCount")));
            settings.setValue(QSL("studio/bc/gridmatrix/structapp_index"),
                get_cmb_index(QSL("cmbGridStructAppIndex")));
            settings.setValue(QSL("studio/bc/gridmatrix/structapp_id"), get_spn_val(QSL("spnGridStructAppID")));
            break;

        case BARCODE_MAXICODE:
            settings.setValue(QSL("studio/bc/maxicode/mode"), get_cmb_index(QSL("cmbMaxiMode")));
            settings.setValue(QSL("studio/bc/maxicode/scm_postcode"), get_txt_val(QSL("txtMaxiSCMPostcode")));
            settings.setValue(QSL("studio/bc/maxicode/scm_country"), get_spn_val(QSL("spnMaxiSCMCountry")));
            settings.setValue(QSL("studio/bc/maxicode/scm_service"), get_spn_val(QSL("spnMaxiSCMService")));
            settings.setValue(QSL("studio/bc/maxicode/chk_scm_vv"), get_chk_val(QSL("chkMaxiSCMVV")));
            settings.setValue(QSL("studio/bc/maxicode/spn_scm_vv"), get_spn_val(QSL("spnMaxiSCMVV")));
            settings.setValue(QSL("studio/bc/maxicode/structapp_count"), get_cmb_index(QSL("cmbMaxiStructAppCount")));
            settings.setValue(QSL("studio/bc/maxicode/structapp_index"), get_cmb_index(QSL("cmbMaxiStructAppIndex")));
            break;

        case BARCODE_CHANNEL:
            settings.setValue(QSL("studio/bc/channel/channel"), get_cmb_index(QSL("cmbChannel")));
            break;

        case BARCODE_CODEONE:
            settings.setValue(QSL("studio/bc/codeone/size"), get_cmb_index(QSL("cmbC1Size")));
            settings.setValue(QSL("studio/bc/codeone/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radC1Stand") << QSL("radC1GS1")));
            settings.setValue(QSL("studio/bc/codeone/structapp_count"), get_spn_val(QSL("spnC1StructAppCount")));
            settings.setValue(QSL("studio/bc/codeone/structapp_index"), get_spn_val(QSL("spnC1StructAppIndex")));
            break;

        case BARCODE_CODE49:
            settings.setValue(QSL("studio/bc/code49/rows"), get_cmb_index(QSL("cmbC49Rows")));
            settings.setValue(QSL("studio/bc/code49/height_per_row"), get_dspn_val(QSL("spnC49HeightPerRow")));
            settings.setValue(QSL("studio/bc/code49/row_sep_height"), get_cmb_index(QSL("cmbC49RowSepHeight")));
            settings.setValue(QSL("studio/bc/code49/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radC49Stand") << QSL("radC49GS1")));
            settings.setValue(QSL("studio/bc/code49/chk_no_quiet_zones"), get_chk_val(QSL("chkC49NoQuietZones")));
            break;

        case BARCODE_CODE93:
            settings.setValue(QSL("studio/bc/code93/chk_show_checks"), get_chk_val(QSL("chkC93ShowChecks")));
            break;

        case BARCODE_DBAR_EXPSTK:
        case BARCODE_DBAR_EXPSTK_CC:
            settings.setValue(QSL("studio/bc/dbar_expstk/colsrows"), get_rad_grp_index(
                QStringList() << QSL("radDBESCols") << QSL("radDBESRows")));
            settings.setValue(QSL("studio/bc/dbar_expstk/cols"), get_cmb_index(QSL("cmbDBESCols")));
            settings.setValue(QSL("studio/bc/dbar_expstk/rows"), get_cmb_index(QSL("cmbDBESRows")));
            settings.setValue(QSL("studio/bc/dbar_expstk/height_per_row"), get_dspn_val(QSL("spnDBESHeightPerRow")));
            break;

        case BARCODE_ULTRA:
            settings.setValue(QSL("studio/bc/ultra/autoresizing"), get_rad_grp_index(
                QStringList() << QSL("radUltraAuto") << QSL("radUltraEcc")));
            settings.setValue(QSL("studio/bc/ultra/ecc"), get_cmb_index(QSL("cmbUltraEcc")));
            settings.setValue(QSL("studio/bc/ultra/revision"), get_cmb_index(QSL("cmbUltraRevision")));
            settings.setValue(QSL("studio/bc/ultra/encoding_mode"), get_rad_grp_index(
                QStringList() << QSL("radUltraStand") << QSL("radUltraGS1")));
            settings.setValue(QSL("studio/bc/ultra/structapp_count"), get_cmb_index(QSL("cmbUltraStructAppCount")));
            settings.setValue(QSL("studio/bc/ultra/structapp_index"), get_cmb_index(QSL("cmbUltraStructAppIndex")));
            settings.setValue(QSL("studio/bc/ultra/structapp_id"), get_spn_val(QSL("spnUltraStructAppID")));
            break;

        case BARCODE_UPCA:
        case BARCODE_UPCA_CHK:
        case BARCODE_UPCA_CC:
            settings.setValue(QSL("studio/bc/upca/addongap"), get_cmb_index(QSL("cmbUPCAAddonGap")));
            settings.setValue(QSL("studio/bc/upca/guard_descent"),
                QString::number(get_dspn_val(QSL("spnUPCAGuardDescent")), 'f', 3 /*precision*/));
            settings.setValue(QSL("studio/bc/upca/chk_no_quiet_zones"), get_chk_val(QSL("chkUPCANoQuietZones")));
            settings.setValue(QSL("studio/bc/upca/chk_guard_whitespace"), get_chk_val(QSL("chkUPCAGuardWhitespace")));
            break;

        case BARCODE_EANX:
        case BARCODE_EANX_CHK:
        case BARCODE_EANX_CC:
            settings.setValue(QSL("studio/bc/eanx/addongap"), get_cmb_index(QSL("cmbUPCEANAddonGap")));
            settings.setValue(QSL("studio/bc/eanx/guard_descent"),
                QString::number(get_dspn_val(QSL("spnUPCEANGuardDescent")), 'f', 3 /*precision*/));
            settings.setValue(QSL("studio/bc/eanx/chk_no_quiet_zones"), get_chk_val(QSL("chkUPCEANNoQuietZones")));
            settings.setValue(QSL("studio/bc/eanx/chk_guard_whitespace"),
                get_chk_val(QSL("chkUPCEANGuardWhitespace")));
            break;

        case BARCODE_UPCE:
        case BARCODE_UPCE_CHK:
        case BARCODE_UPCE_CC:
            settings.setValue(QSL("studio/bc/upce/addongap"), get_cmb_index(QSL("cmbUPCEANAddonGap")));
            settings.setValue(QSL("studio/bc/upce/guard_descent"),
                QString::number(get_dspn_val(QSL("spnUPCEANGuardDescent")), 'f', 3 /*precision*/));
            settings.setValue(QSL("studio/bc/upce/chk_no_quiet_zones"), get_chk_val(QSL("chkUPCEANNoQuietZones")));
            settings.setValue(QSL("studio/bc/upce/chk_guard_whitespace"),
                get_chk_val(QSL("chkUPCEANGuardWhitespace")));
            break;

        case BARCODE_ISBNX:
            settings.setValue(QSL("studio/bc/isnbx/addongap"), get_cmb_index(QSL("cmbUPCEANAddonGap")));
            settings.setValue(QSL("studio/bc/isnbx/guard_descent"),
                QString::number(get_dspn_val(QSL("spnUPCEANGuardDescent")), 'f', 3 /*precision*/));
            settings.setValue(QSL("studio/bc/isnbx/chk_no_quiet_zones"), get_chk_val(QSL("chkUPCEANNoQuietZones")));
            settings.setValue(QSL("studio/bc/isnbx/chk_guard_whitespace"),
                get_chk_val(QSL("chkUPCEANGuardWhitespace")));
            break;

        case BARCODE_VIN:
            settings.setValue(QSL("studio/bc/vin/chk_import_char_prefix"), get_chk_val(QSL("chkVINImportChar")));
            break;
    }
}

/* Load settings for an individual symbol */
void MainWindow::load_sub_settings(QSettings &settings, int symbology)
{
    QString name = get_setting_name(symbology);
    if (!name.isEmpty()) { // Should never be empty
        const QString &tdata = settings.value(QSL("studio/bc/%1/data").arg(name), QSEmpty).toString();
        if (!tdata.isEmpty()) {
            txtData->setText(tdata);
        }
        if (!grpSegs->isHidden()) {
            txtDataSeg1->setText(settings.value(QSL("studio/bc/%1/data_seg1").arg(name), QSEmpty).toString());
            txtDataSeg2->setText(settings.value(QSL("studio/bc/%1/data_seg2").arg(name), QSEmpty).toString());
            txtDataSeg3->setText(settings.value(QSL("studio/bc/%1/data_seg3").arg(name), QSEmpty).toString());
        }
        if (!grpComposite->isHidden()) {
            const QString &composite_text = settings.value(
                                                QSL("studio/bc/%1/composite_text").arg(name), QSEmpty).toString();
            if (!composite_text.isEmpty()) {
                txtComposite->setText(composite_text);
            }
            chkComposite->setChecked(settings.value(
                QSL("studio/bc/%1/chk_composite").arg(name), 0).toInt() ? true : false);
            cmbCompType->setCurrentIndex(settings.value(QSL("studio/bc/%1/comp_type").arg(name), 0).toInt());
        }
        if (cmbECI->isEnabled()) {
            cmbECI->setCurrentIndex(settings.value(QSL("studio/bc/%1/eci").arg(name), 0).toInt());
            cmbECISeg1->setCurrentIndex(settings.value(QSL("studio/bc/%1/eci_seg1").arg(name), 0).toInt());
            cmbECISeg2->setCurrentIndex(settings.value(QSL("studio/bc/%1/eci_seg2").arg(name), 0).toInt());
            cmbECISeg3->setCurrentIndex(settings.value(QSL("studio/bc/%1/eci_seg3").arg(name), 0).toInt());
        }
        chkEscape->setChecked(settings.value(QSL("studio/bc/%1/chk_escape").arg(name)).toInt() ? true : false);
        chkData->setChecked(settings.value(QSL("studio/bc/%1/chk_data").arg(name)).toInt() ? true : false);
        if (chkRInit->isEnabled()) {
            chkRInit->setChecked(settings.value(QSL("studio/bc/%1/chk_rinit").arg(name)).toInt() ? true : false);
        }
        chkGS1Parens->setChecked(settings.value(QSL("studio/bc/%1/chk_gs1parens").arg(name)).toInt() ? true : false);
        chkGS1NoCheck->setChecked(settings.value(
            QSL("studio/bc/%1/chk_gs1nocheck").arg(name)).toInt() ? true : false);
        if (chkAutoHeight->isEnabled()) {
            chkAutoHeight->setChecked(settings.value(
                QSL("studio/bc/%1/appearance/autoheight").arg(name), 1).toInt() ? true : false);
            heightb->setValue(settings.value(QSL("studio/bc/%1/appearance/height").arg(name), 50.0f).toFloat());
        }
        if (chkCompliantHeight->isEnabled()) {
            chkCompliantHeight->setChecked(settings.value(
                QSL("studio/bc/%1/appearance/compliantheight").arg(name), 1).toInt() ? true : false);
        }
        bwidth->setValue(settings.value(QSL("studio/bc/%1/appearance/border").arg(name), 0).toInt());
        spnWhitespace->setValue(settings.value(QSL("studio/bc/%1/appearance/whitespace").arg(name), 0).toInt());
        spnVWhitespace->setValue(settings.value(QSL("studio/bc/%1/appearance/vwhitespace").arg(name), 0).toInt());
        spnScale->setValue(settings.value(QSL("studio/bc/%1/appearance/scale").arg(name), 1.0).toFloat());
        btype->setCurrentIndex(settings.value(QSL("studio/bc/%1/appearance/border_type").arg(name), 0).toInt());
        if (chkHRTShow->isEnabled()) {
            cmbFontSetting->setCurrentIndex(settings.value(
                QSL("studio/bc/%1/appearance/font_setting").arg(name), 0).toInt());
            spnTextGap->setValue(settings.value(QSL("studio/bc/%1/appearance/text_gap").arg(name), 1.0).toFloat());
            chkEmbedVectorFont->setChecked(settings.value(
                QSL("studio/bc/%1/appearance/chk_embed_vector_font").arg(name), 0).toInt() ? true : false);
            chkHRTShow->setChecked(settings.value(
                QSL("studio/bc/%1/appearance/chk_hrt_show").arg(name), 1).toInt() ? true : false);
        }
        chkCMYK->setChecked(settings.value(
            QSL("studio/bc/%1/appearance/chk_cmyk").arg(name), 0).toInt() ? true : false);
        chkQuietZones->setChecked(settings.value(
            QSL("studio/bc/%1/appearance/chk_quietzones").arg(name), 0).toInt() ? true : false);
        cmbRotate->setCurrentIndex(settings.value(QSL("studio/bc/%1/appearance/rotate").arg(name), 0).toInt());
        if (symbology == BARCODE_DOTCODE || chkDotty->isEnabled()) {
            chkDotty->setChecked(settings.value(
                QSL("studio/bc/%1/appearance/chk_dotty").arg(name), 0).toInt() ? true : false);
            spnDotSize->setValue(settings.value(
                QSL("studio/bc/%1/appearance/dot_size").arg(name), 0.4f / 0.5f).toFloat());
        }
        m_fgstr = settings.value(QSL("studio/bc/%1/ink/text").arg(name), QSEmpty).toString();
        if (m_fgstr.isEmpty()) {
            QColor color(settings.value(QSL("studio/bc/%1/ink/red").arg(name), 0).toInt(),
                        settings.value(QSL("studio/bc/%1/ink/green").arg(name), 0).toInt(),
                        settings.value(QSL("studio/bc/%1/ink/blue").arg(name), 0).toInt(),
                        settings.value(QSL("studio/bc/%1/ink/alpha").arg(name), 0xff).toInt());
            m_fgstr = qcolor_to_str(color);
        }
        if (m_fgstr.indexOf(*colorRE) != 0) {
            m_fgstr = fgDefault;
        }
        m_bgstr = settings.value(QSL("studio/bc/%1/paper/text").arg(name), QSEmpty).toString();
        if (m_bgstr.isEmpty()) {
            QColor color(settings.value(QSL("studio/bc/%1/paper/red").arg(name), 0xff).toInt(),
                        settings.value(QSL("studio/bc/%1/paper/green").arg(name), 0xff).toInt(),
                        settings.value(QSL("studio/bc/%1/paper/blue").arg(name), 0xff).toInt(),
                        settings.value(QSL("studio/bc/%1/paper/alpha").arg(name), 0xff).toInt());
            m_bgstr = qcolor_to_str(color);
        }
        if (m_bgstr.indexOf(*colorRE) != 0) {
            m_bgstr = bgDefault;
        }
        m_xdimdpVars.x_dim = settings.value(QSL("studio/bc/%1/xdimdpvars/x_dim").arg(name), 0.0).toFloat();
        m_xdimdpVars.x_dim_units = settings.value(QSL("studio/bc/%1/xdimdpvars/x_dim_units").arg(name), 0).toInt();
        m_xdimdpVars.set = settings.value(QSL("studio/bc/%1/xdimdpvars/set").arg(name), 0).toInt();
    } else {
        m_xdimdpVars.x_dim = 0.0;
        m_xdimdpVars.x_dim_units = 0;
        m_xdimdpVars.set = 0;
    }

    switch (symbology) {
        case BARCODE_CODE128:
        case BARCODE_CODE128AB:
        case BARCODE_GS1_128:
        case BARCODE_GS1_128_CC:
        case BARCODE_HIBC_128:
            set_rad_from_setting(settings, QSL("studio/bc/code128/encoding_mode"),
                QStringList() << QSL("radC128Stand") << QSL("radC128EAN") << QSL("radC128CSup")
                                << QSL("radC128HIBC") << QSL("radC128ExtraEsc"));
            break;

        case BARCODE_PDF417:
        case BARCODE_PDF417COMP:
        case BARCODE_HIBC_PDF:
            set_cmb_from_setting(settings, QSL("studio/bc/pdf417/cols"), QSL("cmbPDFCols"));
            set_cmb_from_setting(settings, QSL("studio/bc/pdf417/rows"), QSL("cmbPDFRows"));
            set_dspn_from_setting(settings, QSL("studio/bc/pdf417/height_per_row"), QSL("spnPDFHeightPerRow"), 0.0f);
            set_cmb_from_setting(settings, QSL("studio/bc/pdf417/ecc"), QSL("cmbPDFECC"));
            set_rad_from_setting(settings, QSL("studio/bc/pdf417/encoding_mode"),
                QStringList() << QSL("radPDFStand") << QSL("radPDFTruncated") << QSL("radPDFHIBC"));
            set_chk_from_setting(settings, QSL("studio/bc/pdf417/chk_fast"), QSL("chkPDFFast"));
            set_spn_from_setting(settings, QSL("studio/bc/pdf417/structapp_count"), QSL("spnPDFStructAppCount"), 1);
            set_spn_from_setting(settings, QSL("studio/bc/pdf417/structapp_index"), QSL("spnPDFStructAppIndex"), 0);
            set_txt_from_setting(settings, QSL("studio/bc/pdf417/structapp_id"), QSL("txtPDFStructAppID"), QSEmpty);
            break;

        case BARCODE_MICROPDF417:
        case BARCODE_HIBC_MICPDF:
            set_cmb_from_setting(settings, QSL("studio/bc/micropdf417/cols"), QSL("cmbMPDFCols"));
            set_dspn_from_setting(settings, QSL("studio/bc/micropdf417/height_per_row"), QSL("spnMPDFHeightPerRow"),
                0.0f);
            set_rad_from_setting(settings, QSL("studio/bc/micropdf417/encoding_mode"),
                QStringList() << QSL("radMPDFStand") << QSL("radMPDFHIBC"));
            set_chk_from_setting(settings, QSL("studio/bc/micropdf417/chk_fast"), QSL("chkMPDFFast"));
            set_spn_from_setting(settings, QSL("studio/bc/micropdf417/structapp_count"),
                QSL("spnMPDFStructAppCount"), 1);
            set_spn_from_setting(settings, QSL("studio/bc/micropdf417/structapp_index"),
                QSL("spnMPDFStructAppIndex"), 0);
            set_txt_from_setting(settings, QSL("studio/bc/micropdf417/structapp_id"), QSL("txtMPDFStructAppID"),
                QSEmpty);
            break;

        case BARCODE_DOTCODE:
            set_cmb_from_setting(settings, QSL("studio/bc/dotcode/cols"), QSL("cmbDotCols"));
            set_cmb_from_setting(settings, QSL("studio/bc/dotcode/mask"), QSL("cmbDotMask"));
            set_rad_from_setting(settings, QSL("studio/bc/dotcode/encoding_mode"),
                QStringList() << QSL("radDotStand") << QSL("radDotGS1"));
            set_cmb_from_setting(settings, QSL("studio/bc/dotcode/structapp_count"), QSL("cmbDotStructAppCount"));
            set_cmb_from_setting(settings, QSL("studio/bc/dotcode/structapp_index"), QSL("cmbDotStructAppIndex"));
            break;

        case BARCODE_AZTEC:
        case BARCODE_HIBC_AZTEC:
            set_rad_from_setting(settings, QSL("studio/bc/aztec/autoresizing"),
                QStringList() << QSL("radAztecAuto") << QSL("radAztecSize") << QSL("radAztecECC"));
            set_cmb_from_setting(settings, QSL("studio/bc/aztec/size"), QSL("cmbAztecSize"));
            set_cmb_from_setting(settings, QSL("studio/bc/aztec/ecc"), QSL("cmbAztecECC"));
            set_rad_from_setting(settings, QSL("studio/bc/aztec/encoding_mode"),
                QStringList() << QSL("radAztecStand") << QSL("radAztecGS1") << QSL("radAztecHIBC"));
            set_cmb_from_setting(settings, QSL("studio/bc/aztec/structapp_count"), QSL("cmbAztecStructAppCount"));
            set_cmb_from_setting(settings, QSL("studio/bc/aztec/structapp_index"), QSL("cmbAztecStructAppIndex"));
            set_txt_from_setting(settings, QSL("studio/bc/aztec/structapp_id"), QSL("txtAztecStructAppID"), QSEmpty);
            break;

        case BARCODE_MSI_PLESSEY:
            set_cmb_from_setting(settings, QSL("studio/bc/msi_plessey/check_digit"), QSL("cmbMSICheck"));
            set_chk_from_setting(settings, QSL("studio/bc/msi_plessey/check_text"), QSL("chkMSICheckText"));
            msi_plessey_ui_set();
            break;

        case BARCODE_CODE11:
            set_rad_from_setting(settings, QSL("studio/bc/code11/check_digit"),
                QStringList() << QSL("radC11TwoCheckDigits") << QSL("radC11OneCheckDigit")
                                << QSL("radC11NoCheckDigits"));
            break;

        case BARCODE_C25STANDARD:
            set_rad_from_setting(settings, QSL("studio/bc/c25standard/check_digit"),
                QStringList() << QSL("radC25Stand") << QSL("radC25Check") << QSL("radC25CheckHide"));
            break;
        case BARCODE_C25INTER:
            set_rad_from_setting(settings, QSL("studio/bc/c25inter/check_digit"),
                QStringList() << QSL("radC25Stand") << QSL("radC25Check") << QSL("radC25CheckHide"));
            break;
        case BARCODE_C25IATA:
            set_rad_from_setting(settings, QSL("studio/bc/c25iata/check_digit"),
                QStringList() << QSL("radC25Stand") << QSL("radC25Check") << QSL("radC25CheckHide"));
            break;
        case BARCODE_C25LOGIC:
            set_rad_from_setting(settings, QSL("studio/bc/c25logic/check_digit"),
                QStringList() << QSL("radC25Stand") << QSL("radC25Check") << QSL("radC25CheckHide"));
            break;
        case BARCODE_C25IND:
            set_rad_from_setting(settings, QSL("studio/bc/c25ind/check_digit"),
                QStringList() << QSL("radC25Stand") << QSL("radC25Check") << QSL("radC25CheckHide"));
            break;

        case BARCODE_CODE39:
        case BARCODE_HIBC_39:
            set_rad_from_setting(settings, QSL("studio/bc/code39/check_digit"),
                QStringList() << QSL("radC39Stand") << QSL("radC39Check") << QSL("radC39HIBC")
                                << QSL("radC39CheckHide"));
            break;

        case BARCODE_EXCODE39:
            set_rad_from_setting(settings, QSL("studio/bc/excode39/check_digit"),
                QStringList() << QSL("radC39Stand") << QSL("radC39Check") << QSL("radC39CheckHide"));
            break;
        case BARCODE_LOGMARS:
            set_rad_from_setting(settings, QSL("studio/bc/logmars/check_digit"),
                QStringList() << QSL("radC39Stand") << QSL("radC39Check") << QSL("radC39CheckHide"));
            break;

        case BARCODE_CODE16K:
            set_cmb_from_setting(settings, QSL("studio/bc/code16k/rows"), QSL("cmbC16kRows"));
            set_dspn_from_setting(settings, QSL("studio/bc/code16k/height_per_row"), QSL("spnC16kHeightPerRow"),
                0.0f);
            set_cmb_from_setting(settings, QSL("studio/bc/code16k/row_sep_height"), QSL("cmbC16kRowSepHeight"));
            set_rad_from_setting(settings, QSL("studio/bc/code16k/encoding_mode"),
                QStringList() << QSL("radC16kStand") << QSL("radC16kGS1"));
            set_chk_from_setting(settings, QSL("studio/bc/code16k/chk_no_quiet_zones"), QSL("chkC16kNoQuietZones"));
            break;

        case BARCODE_CODABAR:
            set_rad_from_setting(settings, QSL("studio/bc/codabar/check_digit"),
                QStringList() << QSL("radCodabarStand") << QSL("radCodabarCheckHide") << QSL("radCodabarCheck"));
            break;

        case BARCODE_CODABLOCKF:
        case BARCODE_HIBC_BLOCKF:
            set_cmb_from_setting(settings, QSL("studio/bc/codablockf/width"), QSL("cmbCbfWidth"));
            set_cmb_from_setting(settings, QSL("studio/bc/codablockf/height"), QSL("cmbCbfHeight"));
            set_dspn_from_setting(settings, QSL("studio/bc/codablockf/height_per_row"), QSL("spnCbfHeightPerRow"),
                0.0f);
            set_cmb_from_setting(settings, QSL("studio/bc/codablockf/row_sep_height"),
                QSL("cmbCbfRowSepHeight"));
            set_rad_from_setting(settings, QSL("studio/bc/codablockf/encoding_mode"),
                QStringList() << QSL("radCbfStand") << QSL("radCbfHIBC"));
            set_chk_from_setting(settings, QSL("studio/bc/codablockf/chk_no_quiet_zones"), QSL("chkCbfNoQuietZones"));
            break;

        case BARCODE_DAFT:
            set_dspn_from_setting(settings, QSL("studio/bc/daft/tracker_ratio"), QSL("spnDAFTTrackerRatio"), 25.0f);
            break;

        case BARCODE_DPD:
            set_chk_from_setting(settings, QSL("studio/bc/dpd/chk_relabel"), QSL("chkDPDRelabel"));
            break;

        case BARCODE_DATAMATRIX:
        case BARCODE_HIBC_DM:
            set_cmb_from_setting(settings, QSL("studio/bc/datamatrix/size"), QSL("cmbDM200Size"));
            set_rad_from_setting(settings, QSL("studio/bc/datamatrix/encoding_mode"),
                QStringList() << QSL("radDM200Stand") << QSL("radDM200GS1") << QSL("radDM200HIBC"));
            set_chk_from_setting(settings, QSL("studio/bc/datamatrix/chk_suppress_rect"), QSL("chkDMRectangle"));
            set_chk_from_setting(settings, QSL("studio/bc/datamatrix/chk_allow_dmre"), QSL("chkDMRE"));
            set_chk_from_setting(settings, QSL("studio/bc/datamatrix/chk_gs_sep"), QSL("chkDMGSSep"));
            set_chk_from_setting(settings, QSL("studio/bc/datamatrix/iso_144"), QSL("chkDMISO144"));
            set_chk_from_setting(settings, QSL("studio/bc/datamatrix/chk_fast"), QSL("chkDMFast"));
            set_cmb_from_setting(settings, QSL("studio/bc/datamatrix/structapp_count"), QSL("cmbDMStructAppCount"));
            set_cmb_from_setting(settings, QSL("studio/bc/datamatrix/structapp_index"), QSL("cmbDMStructAppIndex"));
            set_spn_from_setting(settings, QSL("studio/bc/datamatrix/structapp_id"), QSL("spnDMStructAppID"), 1);
            set_spn_from_setting(settings, QSL("studio/bc/datamatrix/structapp_id2"), QSL("spnDMStructAppID2"), 1);
            break;

        case BARCODE_MAILMARK_2D:
            set_cmb_from_setting(settings, QSL("studio/bc/mailmark2d/size"), QSL("cmbMailmark2DSize"));
            set_chk_from_setting(settings, QSL("studio/bc/mailmark2d/chk_suppress_rect"),
                                            QSL("chkMailmark2DRectangle"));
            break;

        case BARCODE_ITF14:
            set_chk_from_setting(settings, QSL("studio/bc/itf14/chk_no_quiet_zones"), QSL("chkITF14NoQuietZones"));
            break;

        case BARCODE_PZN:
            set_chk_from_setting(settings, QSL("studio/bc/pzn/chk_pzn7"), QSL("chkPZN7"));
            break;

        case BARCODE_QRCODE:
        case BARCODE_HIBC_QR:
            set_cmb_from_setting(settings, QSL("studio/bc/qrcode/size"), QSL("cmbQRSize"));
            set_cmb_from_setting(settings, QSL("studio/bc/qrcode/ecc"), QSL("cmbQRECC"));
            set_cmb_from_setting(settings, QSL("studio/bc/qrcode/mask"), QSL("cmbQRMask"));
            set_rad_from_setting(settings, QSL("studio/bc/qrcode/encoding_mode"),
                QStringList() << QSL("radDM200Stand") << QSL("radQRGS1") << QSL("radQRHIBC"));
            set_chk_from_setting(settings, QSL("studio/bc/qrcode/chk_full_multibyte"), QSL("chkQRFullMultibyte"));
            set_chk_from_setting(settings, QSL("studio/bc/qrcode/chk_fast_mode"), QSL("chkQRFast"));
            set_cmb_from_setting(settings, QSL("studio/bc/qrcode/structapp_count"), QSL("cmbQRStructAppCount"));
            set_cmb_from_setting(settings, QSL("studio/bc/qrcode/structapp_index"), QSL("cmbQRStructAppIndex"));
            set_spn_from_setting(settings, QSL("studio/bc/qrcode/structapp_id"), QSL("spnQRStructAppID"), 0);
            break;

        case BARCODE_UPNQR:
            set_cmb_from_setting(settings, QSL("studio/bc/upnqr/mask"), QSL("cmbUPNQRMask"));
            set_chk_from_setting(settings, QSL("studio/bc/upnqr/chk_fast_mode"), QSL("chkUPNQRFast"));
            break;

        case BARCODE_RMQR:
            set_cmb_from_setting(settings, QSL("studio/bc/rmqr/size"), QSL("cmbRMQRSize"));
            set_cmb_from_setting(settings, QSL("studio/bc/rmqr/ecc"), QSL("cmbRMQRECC"));
            set_rad_from_setting(settings, QSL("studio/bc/rmqr/encoding_mode"),
                QStringList() << QSL("radQRStand") << QSL("radRMQRGS1"));
            set_chk_from_setting(settings, QSL("studio/bc/rmqr/chk_full_multibyte"), QSL("chkRMQRFullMultibyte"));
            break;

        case BARCODE_HANXIN:
            set_cmb_from_setting(settings, QSL("studio/bc/hanxin/size"), QSL("cmbHXSize"));
            set_cmb_from_setting(settings, QSL("studio/bc/hanxin/ecc"), QSL("cmbHXECC"));
            set_cmb_from_setting(settings, QSL("studio/bc/hanxin/mask"), QSL("cmbHXMask"));
            set_chk_from_setting(settings, QSL("studio/bc/hanxin/chk_full_multibyte"), QSL("chkHXFullMultibyte"));
            break;

        case BARCODE_MICROQR:
            set_cmb_from_setting(settings, QSL("studio/bc/microqr/size"), QSL("cmbMQRSize"));
            set_cmb_from_setting(settings, QSL("studio/bc/microqr/ecc"), QSL("cmbMQRECC"));
            set_cmb_from_setting(settings, QSL("studio/bc/microqr/mask"), QSL("cmbMQRMask"));
            set_chk_from_setting(settings, QSL("studio/bc/microqr/chk_full_multibyte"), QSL("chkMQRFullMultibyte"));
            break;

        case BARCODE_GRIDMATRIX:
            set_cmb_from_setting(settings, QSL("studio/bc/gridmatrix/size"), QSL("cmbGridSize"));
            set_cmb_from_setting(settings, QSL("studio/bc/gridmatrix/ecc"), QSL("cmbGridECC"));
            set_chk_from_setting(settings, QSL("studio/bc/gridmatrix/chk_full_multibyte"),
                QSL("chkGridFullMultibyte"));
            set_cmb_from_setting(settings, QSL("studio/bc/gridmatrix/structapp_count"), QSL("cmbGridStructAppCount"));
            set_cmb_from_setting(settings, QSL("studio/bc/gridmatrix/structapp_index"), QSL("cmbGridStructAppIndex"));
            set_spn_from_setting(settings, QSL("studio/bc/gridmatrix/structapp_id"), QSL("spnGridStructAppID"), 0);
            break;

        case BARCODE_MAXICODE:
            set_cmb_from_setting(settings, QSL("studio/bc/maxicode/mode"), QSL("cmbMaxiMode"), 1);
            set_txt_from_setting(settings, QSL("studio/bc/maxicode/scm_postcode"), QSL("txtMaxiSCMPostcode"),
                QSEmpty);
            set_spn_from_setting(settings, QSL("studio/bc/maxicode/scm_country"), QSL("spnMaxiSCMCountry"), 0);
            set_spn_from_setting(settings, QSL("studio/bc/maxicode/scm_service"), QSL("spnMaxiSCMService"), 0);
            set_chk_from_setting(settings, QSL("studio/bc/maxicode/chk_scm_vv"), QSL("chkMaxiSCMVV"));
            // 96 is ASC MH10/SC 8
            set_spn_from_setting(settings, QSL("studio/bc/maxicode/spn_scm_vv"), QSL("spnMaxiSCMVV"), 96);
            set_cmb_from_setting(settings, QSL("studio/bc/maxicode/structapp_count"), QSL("cmbMaxiStructAppCount"));
            set_cmb_from_setting(settings, QSL("studio/bc/maxicode/structapp_index"), QSL("cmbMaxiStructAppIndex"));
            break;

        case BARCODE_CHANNEL:
            set_cmb_from_setting(settings, QSL("studio/bc/channel/channel"), QSL("cmbChannel"));
            break;

        case BARCODE_CODEONE:
            set_cmb_from_setting(settings, QSL("studio/bc/codeone/size"), QSL("cmbC1Size"));
            set_rad_from_setting(settings, QSL("studio/bc/codeone/encoding_mode"),
                QStringList() << QSL("radC1Stand") << QSL("radC1GS1"));
            set_spn_from_setting(settings, QSL("studio/bc/codeone/structapp_count"), QSL("spnC1StructAppCount"), 1);
            set_spn_from_setting(settings, QSL("studio/bc/codeone/structapp_index"), QSL("spnC1StructAppIndex"), 0);
            break;

        case BARCODE_CODE49:
            set_cmb_from_setting(settings, QSL("studio/bc/code49/rows"), QSL("cmbC49Rows"));
            set_dspn_from_setting(settings, QSL("studio/bc/code49/height_per_row"), QSL("spnC49HeightPerRow"), 0.0f);
            set_cmb_from_setting(settings, QSL("studio/bc/code49/row_sep_height"), QSL("cmbC49RowSepHeight"));
            set_rad_from_setting(settings, QSL("studio/bc/code49/encoding_mode"),
                QStringList() << QSL("radC49Stand") << QSL("radC49GS1"));
            set_chk_from_setting(settings, QSL("studio/bc/code49/chk_no_quiet_zones"), QSL("chkC49NoQuietZones"));
            break;

        case BARCODE_CODE93:
            set_chk_from_setting(settings, QSL("studio/bc/code93/chk_show_checks"), QSL("chkC93ShowChecks"));
            break;

        case BARCODE_DBAR_EXPSTK:
            set_rad_from_setting(settings, QSL("studio/bc/dbar_expstk/colsrows"),
                QStringList() << QSL("radDBESCols") << QSL("radDBESRows"));
            set_cmb_from_setting(settings, QSL("studio/bc/dbar_expstk/cols"), QSL("cmbDBESCols"));
            set_cmb_from_setting(settings, QSL("studio/bc/dbar_expstk/rows"), QSL("cmbDBESRows"));
            set_dspn_from_setting(settings, QSL("studio/bc/dbar_expstk/height_per_row"), QSL("spnDBESHeightPerRow"),
                0.0f);
            break;

        case BARCODE_ULTRA:
            set_rad_from_setting(settings, QSL("studio/bc/ultra/autoresizing"),
                QStringList() << QSL("radUltraAuto") << QSL("radUltraEcc"));
            set_cmb_from_setting(settings, QSL("studio/bc/ultra/ecc"), QSL("cmbUltraEcc"));
            set_cmb_from_setting(settings, QSL("studio/bc/ultra/revision"), QSL("cmbUltraRevision"));
            set_rad_from_setting(settings, QSL("studio/bc/ultra/encoding_mode"),
                QStringList() << QSL("radUltraStand") << QSL("radUltraGS1"));
            set_cmb_from_setting(settings, QSL("studio/bc/ultra/structapp_count"), QSL("cmbUltraStructAppCount"));
            set_cmb_from_setting(settings, QSL("studio/bc/ultra/structapp_index"), QSL("cmbUltraStructAppIndex"));
            set_spn_from_setting(settings, QSL("studio/bc/ultra/structapp_id"), QSL("spnUltraStructAppID"), 0);
            break;

        case BARCODE_UPCA:
        case BARCODE_UPCA_CHK:
        case BARCODE_UPCA_CC:
            set_cmb_from_setting(settings, QSL("studio/bc/upca/addongap"), QSL("cmbUPCAAddonGap"));
            set_dspn_from_setting(settings, QSL("studio/bc/upca/guard_descent"), QSL("spnUPCAGuardDescent"), 5.0f);
            set_chk_from_setting(settings, QSL("studio/bc/upca/chk_no_quiet_zones"), QSL("chkUPCANoQuietZones"));
            set_chk_from_setting(settings, QSL("studio/bc/upca/chk_guard_whitespace"), QSL("chkUPCAGuardWhitespace"));
            break;

        case BARCODE_EANX:
        case BARCODE_EANX_CHK:
        case BARCODE_EANX_CC:
            set_cmb_from_setting(settings, QSL("studio/bc/eanx/addongap"), QSL("cmbUPCEANAddonGap"));
            set_dspn_from_setting(settings, QSL("studio/bc/eanx/guard_descent"), QSL("spnUPCEANGuardDescent"), 5.0f);
            set_chk_from_setting(settings, QSL("studio/bc/eanx/chk_no_quiet_zones"), QSL("chkUPCEANNoQuietZones"));
            set_chk_from_setting(settings, QSL("studio/bc/eanx/chk_guard_whitespace"),
                QSL("chkUPCEANGuardWhitespace"));
            break;

        case BARCODE_UPCE:
        case BARCODE_UPCE_CHK:
        case BARCODE_UPCE_CC:
            set_cmb_from_setting(settings, QSL("studio/bc/upce/addongap"), QSL("cmbUPCEANAddonGap"));
            set_dspn_from_setting(settings, QSL("studio/bc/upce/guard_descent"), QSL("spnUPCEANGuardDescent"), 5.0f);
            set_chk_from_setting(settings, QSL("studio/bc/upce/chk_no_quiet_zones"), QSL("chkUPCEANNoQuietZones"));
            set_chk_from_setting(settings, QSL("studio/bc/upce/chk_guard_whitespace"),
                QSL("chkUPCEANGuardWhitespace"));
            break;

        case BARCODE_ISBNX:
            set_cmb_from_setting(settings, QSL("studio/bc/isbnx/addongap"), QSL("cmbUPCEANAddonGap"));
            set_dspn_from_setting(settings, QSL("studio/bc/isbnx/guard_descent"), QSL("spnUPCEANGuardDescent"), 5.0f);
            set_chk_from_setting(settings, QSL("studio/bc/isbnx/chk_no_quiet_zones"), QSL("chkUPCEANNoQuietZones"));
            set_chk_from_setting(settings, QSL("studio/bc/isbnx/chk_guard_whitespace"),
                QSL("chkUPCEANGuardWhitespace"));
            break;

        case BARCODE_VIN:
            set_chk_from_setting(settings, QSL("studio/bc/vin/chk_import_char_prefix"), QSL("chkVINImportChar"));
            break;
    }
}

float MainWindow::get_dpmm(const struct Zint::QZintXdimDpVars* vars) const
{
    return (float) (vars->resolution_units == 1 ? vars->resolution / 25.4 : vars->resolution);
}

const char *MainWindow::getFileType(const struct Zint::QZintXdimDpVars* vars, bool msg) const
{
    return Zint::QZintXdimDpVars::getFileType(m_bc.bc.symbol(), vars, msg);
}

/* vim: set ts=4 sw=4 et : */
