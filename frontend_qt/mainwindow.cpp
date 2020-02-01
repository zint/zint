/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra <bogdan@licentia.eu>               *
 *   Copyright (C) 2009-2019 by Robin Stuart <rstuart114@gmail.com>        *
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

#include <QDebug>
#include <QGraphicsScene>
#include <QImage>
#include <QColorDialog>
#include <QUiLoader>
#include <QFile>
#include <QRadioButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QClipboard>
#include <QMimeData>
#include <QColor>

#include "mainwindow.h"
#include "datawindow.h"
#include "sequencewindow.h"
#include <stdio.h>

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags fl)
        : QWidget(parent, fl),m_optionWidget(0)
{

    QCoreApplication::setOrganizationName("zint");
    QCoreApplication::setOrganizationDomain("zint.org.uk");
    QCoreApplication::setApplicationName("Barcode Studio");

    QSettings settings;

    char bstyle_text[][50] = {
        "Australia Post Redirect Code",
        "Australia Post Reply-Paid",
        "Australia Post Routing Code",
        "Australia Post Standard Customer",
        "Aztec Code (ISO 24778)",
        "Aztec Runes",
        "Channel Code",
        "Codabar",
        "Codablock-F",
        "Code 11",
        "Code 128 (ISO 15417)",
        "Code 16k",
        "Code 2 of 5 Data Logic",
        "Code 2 of 5 IATA",
        "Code 2 of 5 Industrial",
        "Code 2 of 5 Interleaved",
        "Code 2 of 5 Matrix",
        "Code 32 (Italian Pharmacode)",
        "Code 39 (ISO 16388)",
        "Code 39 Extended",
        "Code 49",
        "Code 93",
        "Code One",
        "Data Matrix (ISO 16022)",
        "Deutsche Post Identcode",
        "Deutsche Post Leitcode",
        "DotCode",
        "Dutch Post KIX",
        "EAN-14",
        "European Article Number (EAN)",
        "Facing Identification Mark (FIM)",
        "Flattermarken",
        "Grid Matrix",
        "GS1 DataBar Expanded Omnidirectional",
        "GS1 DataBar Expanded Stacked Omnidirectional",
        "GS1 DataBar Limited",
        "GS1 DataBar Omnidirectional",
        "GS1 DataBar Stacked",
        "GS1 DataBar Stacked Omnidirectional",
        "Han Xin (Chinese Sensible) Code",
        "ITF-14",
        "International Standard Book Number (ISBN)",
        "Japanese Postal Barcode",
        "Korean Postal Barcode",
        "LOGMARS",
        "Maxicode (ISO 16023)",
        "MicroPDF417 (ISO 24728)",
        "Micro QR Code",
        "MSI Plessey",
        "NVE-18",
        "PDF417 (ISO 15438)",
        "Pharmacode",
        "Pharmacode 2-track",
        "Pharma Zentralnummer (PZN)",
        "PLANET",
        "Postnet",
        "QR Code (ISO 18004)",
        "Rectangular Micro QR (rMQR)",
        "Royal Mail 4-state Barcode",
        "Royal Mail 4-state Mailmark",
        "Telepen",
        "Telepen Numeric",
        "UK Plessey",
        "UPNQR",
        "Universal Product Code (UPC-A)",
        "Universal Product Code (UPC-E)",
        "USPS Intelligent Mail"
    };

    scene = new QGraphicsScene(this);

    setupUi(this);
    view->setScene(scene);

    m_fgcolor=qRgb(settings.value("studio/ink/red", 0).toInt(),
                settings.value("studio/ink/green", 0).toInt(),
                settings.value("studio/ink/blue", 0).toInt());
    m_bgcolor=qRgb(settings.value("studio/paper/red", 0xff).toInt(),
                settings.value("studio/paper/green", 0xff).toInt(),
                settings.value("studio/paper/blue", 0xff).toInt());
    for (int i=0;i<metaObject()->enumerator(0).keyCount();i++) {
        bstyle->addItem(metaObject()->enumerator(0).key(i));
        bstyle->setItemText(i,bstyle_text[i]);
    }
    bstyle->setCurrentIndex(settings.value("studio/symbology", 10).toInt());
        txtData->setText(settings.value("studio/data", "Your Data Here!").toString());
        txtComposite->setText(settings.value("studio/composite_text", "Your Data Here!").toString());
        heightb->setValue(settings.value("studio/appearance/height", 50).toInt());
        bwidth->setValue(settings.value("studio/appearance/border", 50).toInt());
        spnWhitespace->setValue(settings.value("studio/appearance/whitespace", 0).toInt());
        spnScale->setValue(settings.value("studio/appearance/scale", 1.0).toFloat());
        btype->setCurrentIndex(settings.value("studio/appearance/border_type", 0).toInt());
    change_options();
        scene->addItem(&m_bc);
    update_preview();
    connect(bstyle, SIGNAL(currentIndexChanged( int )), SLOT(change_options()));
    connect(bstyle, SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
    connect(heightb, SIGNAL(valueChanged( int )), SLOT(update_preview()));
    connect(bwidth,  SIGNAL(valueChanged( int )), SLOT(update_preview()));
    connect(btype, SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
    connect(txtData, SIGNAL(textChanged( const QString& )), SLOT(update_preview()));
    connect(txtComposite, SIGNAL(textChanged()), SLOT(update_preview()));
    connect(chkComposite, SIGNAL(stateChanged( int )), SLOT(composite_enable()));
    connect(chkComposite, SIGNAL(stateChanged( int )), SLOT(update_preview()));
    connect(cmbCompType, SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
    connect(spnWhitespace, SIGNAL(valueChanged( int )), SLOT(update_preview()));
    connect(btnAbout, SIGNAL(clicked( bool )), SLOT(about()));
    connect(btnSave, SIGNAL(clicked( bool )), SLOT(save()));
    connect(spnScale, SIGNAL(valueChanged( double )), SLOT(change_print_scale()));
    connect(btnExit, SIGNAL(clicked( bool )), SLOT(quit_now()));
    connect(btnReset, SIGNAL(clicked( bool )), SLOT(reset_view()));
    connect(btnMoreData, SIGNAL(clicked( bool )), SLOT(open_data_dialog()));
    connect(btnSequence, SIGNAL(clicked( bool )), SLOT(open_sequence_dialog()));
    connect(chkHRTHide, SIGNAL(stateChanged( int )), SLOT(update_preview()));
    connect(chkAutoHeight, SIGNAL(stateChanged( int )), SLOT(autoheight_clicked()));
    connect(btnCopySVG, SIGNAL(clicked( bool )), SLOT(copy_to_clipboard_svg()));
    connect(btnCopyBMP, SIGNAL(clicked( bool )), SLOT(copy_to_clipboard_bmp()));
}

MainWindow::~MainWindow()
{
    QSettings settings;

    settings.setValue("studio/symbology", bstyle->currentIndex());
    settings.setValue("studio/ink/red", m_fgcolor.red());
    settings.setValue("studio/ink/green", m_fgcolor.green());
    settings.setValue("studio/ink/blue", m_fgcolor.blue());
    settings.setValue("studio/paper/red", m_bgcolor.red());
    settings.setValue("studio/paper/green", m_bgcolor.green());
    settings.setValue("studio/paper/blue", m_bgcolor.blue());
    settings.setValue("studio/data", txtData->text());
    settings.setValue("studio/composite_text", txtComposite->toPlainText());
    settings.setValue("studio/appearance/height", heightb->value());
    settings.setValue("studio/appearance/border", bwidth->value());
    settings.setValue("studio/appearance/whitespace", spnWhitespace->value());
    settings.setValue("studio/appearance/scale", spnScale->value());
    settings.setValue("studio/appearance/border_type", btype->currentIndex());
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update_preview();
}

void MainWindow::reset_view()
{
    m_fgcolor=qRgb(0,0,0);
    m_bgcolor=qRgb(0xff,0xff,0xff);
    update_preview();
}

bool MainWindow::save()
{
    QSettings settings;
    QFileDialog save_dialog;
    QString filename;
    QString suffix;

    save_dialog.setAcceptMode(QFileDialog::AcceptSave);
    save_dialog.setWindowTitle("Save Barcode Image");
    save_dialog.setDirectory(settings.value("studio/default_dir", QDir::toNativeSeparators(QDir::homePath())).toString());

#ifdef NO_PNG
    suffix = settings.value("studio/default_suffix", "gif").toString();
    save_dialog.setNameFilter(tr("Encapsulated Post Script (*.eps);;Graphics Interchange Format (*.gif);;Scalable Vector Graphic (*.svg);;Windows Bitmap (*.bmp);;ZSoft PC Painter Image (*.pcx);;Extended Metafile (*.emf);;Tagged Image File Format (*.tif)"));
#else
    suffix = settings.value("studio/default_suffix", "png").toString();
    save_dialog.setNameFilter(tr("Portable Network Graphic (*.png);;Encapsulated Post Script (*.eps);;Graphics Interchange Format (*.gif);;Scalable Vector Graphic (*.svg);;Windows Bitmap (*.bmp);;ZSoft PC Painter Image (*.pcx);;Extended Metafile (*.emf);;Tagged Image File Format (*.tif)"));
#endif

    if (QString::compare(suffix, "png", Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter("Portable Network Graphic (*.png)");
    if (QString::compare(suffix, "eps", Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter("Encapsulated Post Script (*.eps)");
    if (QString::compare(suffix, "gif", Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter("Graphics Interchange Format (*.gif)");
    if (QString::compare(suffix, "svg", Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter("Scalable Vector Graphic (*.svg)");
    if (QString::compare(suffix, "bmp", Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter("Windows Bitmap (*.bmp)");
    if (QString::compare(suffix, "pcx", Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter("ZSoft PC Painter Image (*.pcx)");
    if (QString::compare(suffix, "emf", Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter("Extended Metafile (*.emf)");
    if (QString::compare(suffix, "tif", Qt::CaseInsensitive) == 0)
        save_dialog.selectNameFilter("Tagged Image File Format (*.tif)");

    if (save_dialog.exec()) {
        filename = save_dialog.selectedFiles().at(0);
        if ((filename.lastIndexOf(".") == -1) || (filename.lastIndexOf(".") < (filename.length() - 5))) {
            suffix = save_dialog.selectedNameFilter();
            suffix = suffix.mid((suffix.lastIndexOf(".") + 1), 3);
            filename.append(".");
            filename.append(suffix);
        } else {
            suffix = filename.right(filename.length() - (filename.lastIndexOf('.') + 1));
        }
    } else {
        return false;
    }

    if(m_bc.bc.save_to_file(filename) == false) {
            if (m_bc.bc.getError() > 4) {
        QMessageBox::critical(this,tr("Save Error"),m_bc.bc.error_message());
                return false;
            } else {
                QMessageBox::warning(this, tr("Save Warning"),m_bc.bc.error_message());
                return true;
            }
    }

        settings.setValue("studio/default_dir", filename.mid(0, filename.lastIndexOf(QDir::separator())));
        settings.setValue("studio/default_suffix", suffix);
    return true;
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Zint"),
       tr("<h2>Zint Barcode Studio 2.7.1</h2>"
           "<p>A free barcode generator"
           "<p>Instruction manual is available at the project homepage:<br>"
           "<a href=\"http://www.zint.org.uk\">http://www.zint.org.uk</a>"
           "<p>Copyright &copy; 2006-2018 Robin Stuart and others.<br>"
           "Qt back end by BogDan Vatra<br>"
           "Windows port by Harald Oehlmann</p>"
           "<p>Qt version " QT_VERSION_STR
           "<p>With thanks to Norbert Szab&oacute;, Robert Elliott, "
               "Milton Neal and many others at Sourceforge."
           "<p>Released under the GNU General Public License ver. 3 or later.<br>"
           "\"QR Code\" is a Registered Trademark of Denso Corp.<br>"
           "\"Telepen\" is a Registered Trademark of SB Electronics."
           "<p><table border=1><tr><td><small>Currently supported standards include:<br>"
           "EN 797:1996, EN 798:1996, EN 12323:2005, ISO/IEC 15417:2007,<br>"
           "ISO/IEC 15438:2015, ISO/IEC 16022:2006, ISO/IEC 16023:2000,<br>"
           "ISO/IEC 16388:2007, ISO/IEC 18004:2015, ISO/IEC 24723:2010,<br>"
           "ISO/IEC 24724:2011, ISO/IEC 24728:2006, ISO/IEC 24778:2008,<br>"
           "ISO/IEC 21471:2019, ANSI-HIBC 2.3-2009, ANSI/AIM BC6-2000,<br>"
           "ANSI/AIM BC12-1998, AIMD014 (v 1.63), USPS-B-3200"
           "</small></td></tr></table>"));
}

int MainWindow::open_data_dialog()
{
    int retval;
    DataWindow dlg(txtData->text());
    retval = dlg.exec();
    if (dlg.Valid == 1)
        txtData->setText(dlg.DataOutput);
    return retval;
}

int MainWindow::open_sequence_dialog()
{
    SequenceWindow dlg;
    dlg.barcode = &m_bc;
    return dlg.exec();
}

void MainWindow::on_fgcolor_clicked()
{
    QColor temp = m_fgcolor;
    m_fgcolor=QColorDialog::getColor(m_fgcolor,this,"Set foreground colour");
    if (m_fgcolor.isValid()) {
        update_preview();
    } else {
        m_fgcolor = temp;
    }
}

void MainWindow::on_bgcolor_clicked()
{
    QColor temp = m_bgcolor;
    m_bgcolor=QColorDialog::getColor(m_bgcolor,this,"Set background colour");
    if (m_bgcolor.isValid()) {
        update_preview();
    } else {
        m_bgcolor = temp;
    }
}

void MainWindow::autoheight_clicked()
{
    if (chkAutoHeight->isChecked()) {
        lblHeight->setEnabled(false);
        heightb->setEnabled(false);
    } else {
        lblHeight->setEnabled(true);
        heightb->setEnabled(true);
    }
    update_preview();
}

void MainWindow::change_print_scale()
{
    /* This value is only used when printing (saving) to file */
    m_bc.bc.setScale((float)spnScale->value());
}

void MainWindow::quit_now()
{
    close();
}

void MainWindow::copy_to_clipboard_svg()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QMimeData *data = new QMimeData;
    QString filename = ".zint.svg";
    double scale = spnScale->value();

    spnScale->setValue(5);

    if (!m_bc.bc.save_to_file(filename)) {
        return;
    }

    data->setImageData(QImage(filename));
    clipboard->setMimeData(data, QClipboard::Clipboard);

    QFile::remove(filename);

    spnScale->setValue(scale);
}

void MainWindow::copy_to_clipboard_bmp()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QMimeData *data = new QMimeData;
    QString filename = ".zint.bmp";

    if (!m_bc.bc.save_to_file(filename)) {
        return;
    }

    data->setImageData(QImage(filename));
    clipboard->setMimeData(data, QClipboard::Clipboard);

    QFile::remove(filename);
}

void MainWindow::change_options()
{
    QUiLoader uiload;

    if (tabMain->count()==3)
        tabMain->removeTab(1);

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_CODE128)
    {
        QFile file(":/grpC128.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Code 128"));
        chkComposite->setText(tr("Add 2D Component (GS1-128 only)"));
        connect(m_optionWidget->findChild<QObject*>("radC128EAN"), SIGNAL(toggled( bool )), SLOT(composite_ean_check()));
        connect(m_optionWidget->findChild<QObject*>("radC128Stand"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radC128CSup"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radC128EAN"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radC128HIBC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
    }
    else
        chkComposite->setText(tr("Add 2D Component"));

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_PDF417)
    {
        QFile file(":/grpPDF417.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("PDF417"));
        connect(m_optionWidget->findChild<QObject*>("cmbPDFECC"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbPDFCols"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radPDFTruncated"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radPDFStand"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radPDFHIBC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_MICROPDF417)
    {
        QFile file(":/grpMicroPDF.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Micro PDF417"));
        connect(m_optionWidget->findChild<QObject*>("cmbMPDFCols"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radMPDFStand"), SIGNAL(toggled( bool )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_DOTCODE)
    {
        QFile file(":/grpDotCode.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("DotCode"));
        connect(m_optionWidget->findChild<QObject*>("cmbDotCols"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radDotStan"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radDotGs1"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("txtDotSize"), SIGNAL(textChanged( QString )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_AZTEC)
    {
        QFile file(":/grpAztec.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Aztec Code"));
        connect(m_optionWidget->findChild<QObject*>("radAztecAuto"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radAztecSize"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radAztecECC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbAztecSize"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbAztecECC"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radAztecStand"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radAztecGS1"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radAztecHIBC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_MSI_PLESSEY)
    {
        QFile file(":/grpMSICheck.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("MSI Plessey"));
        connect(m_optionWidget->findChild<QObject*>("cmbMSICheck"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
    }

    if((metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_CODE39) ||
        (metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_EXCODE39))
    {
        QFile file(":/grpC39.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Code 39"));
        connect(m_optionWidget->findChild<QObject*>("radC39Stand"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radC39Check"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radC39HIBC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_EXCODE39)
        {
            if(m_optionWidget->findChild<QRadioButton*>("radC39HIBC")->isChecked() == true)
            {
                m_optionWidget->findChild<QRadioButton*>("radC39HIBC")->setChecked(false);
                m_optionWidget->findChild<QRadioButton*>("radC39Stand")->setChecked(true);
            }
            m_optionWidget->findChild<QRadioButton*>("radC39HIBC")->setEnabled(false);
        }
        else
            m_optionWidget->findChild<QRadioButton*>("radC39HIBC")->setEnabled(true);
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_CODE16K)
    {
        QFile file(":/grpC16k.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Code 16K"));
        connect(m_optionWidget->findChild<QObject*>("radC16kStand"), SIGNAL(toggled( bool )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_CODABLOCKF)
    {
        QFile file (":/grpCodablockF.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Codablock-F"));
        connect(m_optionWidget->findChild<QObject*>("radCbfAutosize"), SIGNAL(toggled( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radCbfSetWidth"), SIGNAL(toggled( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radCbfSetHeight"), SIGNAL(toggled( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbCbfWidth"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbCbfHeight"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_DATAMATRIX)
    {
        QFile file(":/grpDM.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Data Matrix"));
        connect(m_optionWidget->findChild<QObject*>("radDM200Stand"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radDM200GS1"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radDM200HIBC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbDM200Size"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("chkDMRectangle"), SIGNAL(stateChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("chkDMRE"), SIGNAL(stateChanged( int )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_QRCODE)
    {
        QFile file(":/grpQR.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("QR Code"));
        connect(m_optionWidget->findChild<QObject*>("radQRAuto"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radQRSize"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radQRECC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbQRSize"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbQRECC"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radQRStand"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radQRGS1"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radQRHIBC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_RMQR)
    {
        QFile file(":/grpRMQR.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("rMQR Code"));
        connect(m_optionWidget->findChild<QObject*>("radRMQRAuto"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radRMQRSize"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radRMQRECC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbRMQRSize"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbRMQRECC"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radRMQRStand"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radRMQRGS1"), SIGNAL(clicked( bool )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_HANXIN)
    {
        QFile file (":/grpHX.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Han Xin Code"));
        connect(m_optionWidget->findChild<QObject*>("radHXAuto"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radHXSize"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radHXECC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbHXSize"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbHXECC"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_MICROQR)
    {
        QFile file(":/grpMQR.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Micro QR Code"));
        connect(m_optionWidget->findChild<QObject*>("radMQRAuto"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radMQRSize"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radMQRECC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbMQRSize"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbMQRECC"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_GRIDMATRIX)
    {
        QFile file(":/grpGrid.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Grid Matrix"));
        connect(m_optionWidget->findChild<QObject*>("radGridAuto"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radGridSize"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radGridECC"), SIGNAL(clicked( bool )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbGridSize"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbGridECC"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_MAXICODE)
    {
        QFile file(":/grpMaxicode.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Maxicode"));
        connect(m_optionWidget->findChild<QObject*>("cmbMaxiMode"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("cmbMaxiMode"), SIGNAL(currentIndexChanged( int )), SLOT(maxi_primary()));
        connect(m_optionWidget->findChild<QObject*>("txtMaxiPrimary"), SIGNAL(textChanged( const QString& )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_CHANNEL)
    {
        QFile file(":/grpChannel.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Channel Code"));
        connect(m_optionWidget->findChild<QObject*>("cmbChannel"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_CODEONE)
    {
        QFile file(":/grpCodeOne.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Code One"));
        connect(m_optionWidget->findChild<QObject*>("cmbC1Size"), SIGNAL(currentIndexChanged( int )), SLOT(update_preview()));
        connect(m_optionWidget->findChild<QObject*>("radC1GS1"), SIGNAL(toggled( bool )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_CODE49)
    {
        QFile file(":/grpC49.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("Code 49"));
        connect(m_optionWidget->findChild<QObject*>("radC49GS1"), SIGNAL(toggled( bool )), SLOT(update_preview()));
    }

    if(metaObject()->enumerator(0).value(bstyle->currentIndex()) == BARCODE_RSS_EXPSTACK)
    {
        QFile file(":/grpDBExtend.ui");
        if (!file.open(QIODevice::ReadOnly))
            return;
        m_optionWidget=uiload.load(&file);
        file.close();
        tabMain->insertTab(1,m_optionWidget,tr("GS1 DataBar Stacked"));
        connect(m_optionWidget->findChild<QObject*>("cmbCols"), SIGNAL(currentIndexChanged ( int )), SLOT(update_preview()));
    }

    switch(metaObject()->enumerator(0).value(bstyle->currentIndex()))
    {
        case BARCODE_CODE128:
        case BARCODE_EANX:
        case BARCODE_UPCA:
        case BARCODE_UPCE:
        case BARCODE_RSS14:
        case BARCODE_RSS_LTD:
        case BARCODE_RSS_EXP:
        case BARCODE_RSS14STACK:
        case BARCODE_RSS14STACK_OMNI:
        case BARCODE_RSS_EXPSTACK:
            grpComposite->show();
            break;
        default:
            chkComposite->setChecked(false);
            grpComposite->hide();
            break;
    }
    tabMain->setCurrentIndex(0);
}

void MainWindow::composite_enable()
{
    if(chkComposite->isChecked() == true)
    {
        lblCompType->setEnabled(true);
        cmbCompType->setEnabled(true);
        lblComposite->setEnabled(true);
        txtComposite->setEnabled(true);
        if (metaObject()->enumerator(0).value(bstyle->currentIndex())==BARCODE_CODE128)
            m_optionWidget->findChild<QRadioButton*>("radC128EAN")->setChecked(true);
    }
    else
    {
        lblCompType->setEnabled(false);
        cmbCompType->setEnabled(false);
        lblComposite->setEnabled(false);
        txtComposite->setEnabled(false);
    }
}

void MainWindow::composite_ean_check()
{
    if (metaObject()->enumerator(0).value(bstyle->currentIndex())!=BARCODE_CODE128)
        return;
    if(!m_optionWidget->findChild<QRadioButton*>("radC128EAN")->isChecked())
        chkComposite->setChecked(false);
}

void MainWindow::maxi_primary()
{
    if (metaObject()->enumerator(0).value(bstyle->currentIndex())!=BARCODE_MAXICODE)
        return;
    if(m_optionWidget->findChild<QComboBox*>("cmbMaxiMode")->currentIndex() == 0) {
        m_optionWidget->findChild<QLabel*>("lblMaxiPrimary")->setEnabled(true);
        m_optionWidget->findChild<QLineEdit*>("txtMaxiPrimary")->setEnabled(true);
    } else {
        m_optionWidget->findChild<QLabel*>("lblMaxiPrimary")->setEnabled(false);
        m_optionWidget->findChild<QLineEdit*>("txtMaxiPrimary")->setEnabled(false);
    }
}

void MainWindow::update_preview()
{
    int width = view->geometry().width();
    int height = view->geometry().height();

    //m_bc.ar=(Zint::QZint::AspectRatioMode)1;
    if(chkComposite->isChecked() == true) {
        m_bc.bc.setPrimaryMessage(txtData->text());
        m_bc.bc.setText(txtComposite->toPlainText());
    } else {
        m_bc.bc.setText(txtData->text());
    }
    m_bc.bc.setSecurityLevel(0);
    m_bc.bc.setWidth(0);
    m_bc.bc.setInputMode(UNICODE_MODE);
    m_bc.bc.setHideText(0);
    if(chkHRTHide->isChecked() == false) {
        m_bc.bc.setHideText(1);
    }
    switch(metaObject()->enumerator(0).value(bstyle->currentIndex()))
    {
        case BARCODE_CODE128:
            if(m_optionWidget->findChild<QRadioButton*>("radC128Stand")->isChecked())
                m_bc.bc.setSymbol(BARCODE_CODE128);

            if(m_optionWidget->findChild<QRadioButton*>("radC128CSup")->isChecked())
                m_bc.bc.setSymbol(BARCODE_CODE128B);

            if(m_optionWidget->findChild<QRadioButton*>("radC128EAN")->isChecked())
            {
                if(chkComposite->isChecked())
                    m_bc.bc.setSymbol(BARCODE_EAN128_CC);
                else
                    m_bc.bc.setSymbol(BARCODE_EAN128);
            }

            if(m_optionWidget->findChild<QRadioButton*>("radC128HIBC")->isChecked())
                m_bc.bc.setSymbol(BARCODE_HIBC_128);
            break;

        case BARCODE_EANX:
            if(chkComposite->isChecked())
                m_bc.bc.setSymbol(BARCODE_EANX_CC);
            else
                m_bc.bc.setSymbol(BARCODE_EANX);
            break;

        case BARCODE_UPCA:
            if(chkComposite->isChecked())
                m_bc.bc.setSymbol(BARCODE_UPCA_CC);
            else
                m_bc.bc.setSymbol(BARCODE_UPCA);
            break;

        case BARCODE_UPCE:
            if(chkComposite->isChecked())
                m_bc.bc.setSymbol(BARCODE_UPCE_CC);
            else
                m_bc.bc.setSymbol(BARCODE_UPCE);
            break;

        case BARCODE_RSS14:
            if(chkComposite->isChecked())
                m_bc.bc.setSymbol(BARCODE_RSS14_CC);
            else
                m_bc.bc.setSymbol(BARCODE_RSS14);
            break;

        case BARCODE_RSS_LTD:
            if(chkComposite->isChecked())
                m_bc.bc.setSymbol(BARCODE_RSS_LTD_CC);
            else
                m_bc.bc.setSymbol(BARCODE_RSS_LTD);
            break;

        case BARCODE_RSS_EXP:
            if(chkComposite->isChecked())
                m_bc.bc.setSymbol(BARCODE_RSS_EXP_CC);
            else
                m_bc.bc.setSymbol(BARCODE_RSS_EXP);
            break;

        case BARCODE_RSS14STACK:
            if(chkComposite->isChecked())
                m_bc.bc.setSymbol(BARCODE_RSS14STACK_CC);
            else
                m_bc.bc.setSymbol(BARCODE_RSS14STACK);
            break;

        case BARCODE_RSS14STACK_OMNI:
            if(chkComposite->isChecked())
                m_bc.bc.setSymbol(BARCODE_RSS14_OMNI_CC);
            else
                m_bc.bc.setSymbol(BARCODE_RSS14STACK_OMNI);
            break;

        case BARCODE_RSS_EXPSTACK:
            if(chkComposite->isChecked())
                m_bc.bc.setSymbol(BARCODE_RSS_EXPSTACK_CC);
            else
                m_bc.bc.setSymbol(BARCODE_RSS_EXPSTACK);

            if(m_optionWidget->findChild<QComboBox*>("cmbCols")->currentIndex() != 0)
                m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbCols")->currentIndex());
            break;

        case BARCODE_PDF417:
            m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbPDFCols")->currentIndex());
            m_bc.bc.setSecurityLevel(m_optionWidget->findChild<QComboBox*>("cmbPDFECC")->currentIndex()-1);
            if(m_optionWidget->findChild<QRadioButton*>("radPDFStand")->isChecked())
                m_bc.bc.setSymbol(BARCODE_PDF417);

            if(m_optionWidget->findChild<QRadioButton*>("radPDFTruncated")->isChecked())
                m_bc.bc.setSymbol(BARCODE_PDF417TRUNC);

            if(m_optionWidget->findChild<QRadioButton*>("radPDFHIBC")->isChecked())
                m_bc.bc.setSymbol(BARCODE_HIBC_PDF);
            break;

        case BARCODE_MICROPDF417:
            m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbMPDFCols")->currentIndex());
            if(m_optionWidget->findChild<QRadioButton*>("radMPDFStand")->isChecked())
                m_bc.bc.setSymbol(BARCODE_MICROPDF417);

            if(m_optionWidget->findChild<QRadioButton*>("radMPDFHIBC")->isChecked())
                m_bc.bc.setSymbol(BARCODE_HIBC_MICPDF);
            break;

        case BARCODE_DOTCODE:
            m_bc.bc.setSymbol(BARCODE_DOTCODE);
            m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbDotCols")->currentIndex());
            if(m_optionWidget->findChild<QRadioButton*>("radDotGs1")->isChecked())
                m_bc.bc.setInputMode(GS1_MODE);
            m_bc.bc.setDotSize(m_optionWidget->findChild<QLineEdit*>("txtDotSize")->text().toFloat());
            break;

        case BARCODE_AZTEC:
            m_bc.bc.setSymbol(BARCODE_AZTEC);
            if(m_optionWidget->findChild<QRadioButton*>("radAztecSize")->isChecked())
                m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbAztecSize")->currentIndex() + 1);

            if(m_optionWidget->findChild<QRadioButton*>("radAztecECC")->isChecked())
                m_bc.bc.setSecurityLevel(m_optionWidget->findChild<QComboBox*>("cmbAztecECC")->currentIndex() + 1);

            if(m_optionWidget->findChild<QRadioButton*>("radAztecGS1")->isChecked())
                m_bc.bc.setInputMode(GS1_MODE);
            if(m_optionWidget->findChild<QRadioButton*>("radAztecHIBC")->isChecked())
                m_bc.bc.setSymbol(BARCODE_HIBC_AZTEC);
            break;

        case BARCODE_MSI_PLESSEY:
            m_bc.bc.setSymbol(BARCODE_MSI_PLESSEY);
            m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbMSICheck")->currentIndex());
            break;

        case BARCODE_CODE39:
            if(m_optionWidget->findChild<QRadioButton*>("radC39HIBC")->isChecked())
                m_bc.bc.setSymbol(BARCODE_HIBC_39);
            else
            {
                m_bc.bc.setSymbol(BARCODE_CODE39);
                if(m_optionWidget->findChild<QRadioButton*>("radC39Check")->isChecked())
                    m_bc.bc.setWidth(1);
            }
            break;

        case BARCODE_EXCODE39:
            m_bc.bc.setSymbol(BARCODE_EXCODE39);
            if(m_optionWidget->findChild<QRadioButton*>("radC39Check")->isChecked())
                m_bc.bc.setWidth(1);

            break;
        case BARCODE_CODE16K:
            m_bc.bc.setSymbol(BARCODE_CODE16K);
            if(m_optionWidget->findChild<QRadioButton*>("radC16kStand")->isChecked())
                m_bc.bc.setInputMode(UNICODE_MODE);
            else
                m_bc.bc.setInputMode(GS1_MODE);
            break;

            case BARCODE_CODABLOCKF:
                m_bc.bc.setSymbol(BARCODE_CODABLOCKF);
                if(m_optionWidget->findChild<QRadioButton*>("radCbfSetWidth")->isChecked())
                    m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbCbfWidth")->currentIndex() + 6);
                // Height selection uses option 1 in zint_symbol
                if(m_optionWidget->findChild<QRadioButton*>("radCbfSetHeight")->isChecked())
                    m_bc.bc.setSecurityLevel(m_optionWidget->findChild<QComboBox*>("cmbCbfHeight")->currentIndex() + 1);
                break;

        case BARCODE_DATAMATRIX:
            m_bc.bc.setSecurityLevel(1);
            if(m_optionWidget->findChild<QRadioButton*>("radDM200HIBC")->isChecked())
                m_bc.bc.setSymbol(BARCODE_HIBC_DM);
            else
                m_bc.bc.setSymbol(BARCODE_DATAMATRIX);

            if(m_optionWidget->findChild<QRadioButton*>("radDM200GS1")->isChecked())
                m_bc.bc.setInputMode(GS1_MODE);

            m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbDM200Size")->currentIndex());

            if (m_optionWidget->findChild<QComboBox*>("cmbDM200Size")->currentIndex() == 0) {
                // Supressing rectangles or allowing DMRE only makes sense if in automatic size mode
                findChild<QCheckBox*>("chkDMRectangle")->setEnabled(true);
                findChild<QCheckBox*>("chkDMRE")->setEnabled(true);
                if(m_optionWidget->findChild<QCheckBox*>("chkDMRectangle")->isChecked())
                        m_bc.bc.setOption3(DM_SQUARE);
                else {
                    if(m_optionWidget->findChild<QCheckBox*>("chkDMRE")->isChecked())
                        m_bc.bc.setOption3(DM_DMRE);
                    else
                        m_bc.bc.setOption3(0);
                }
            } else {
                findChild<QCheckBox*>("chkDMRectangle")->setEnabled(false);
                findChild<QCheckBox*>("chkDMRE")->setEnabled(false);
                m_bc.bc.setOption3(0);
            }
            break;

        case BARCODE_QRCODE:
            if(m_optionWidget->findChild<QRadioButton*>("radQRHIBC")->isChecked())
                m_bc.bc.setSymbol(BARCODE_HIBC_QR);
            else
                m_bc.bc.setSymbol(BARCODE_QRCODE);

            if(m_optionWidget->findChild<QRadioButton*>("radQRGS1")->isChecked())
                m_bc.bc.setInputMode(GS1_MODE);

            if(m_optionWidget->findChild<QRadioButton*>("radQRSize")->isChecked())
                m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbQRSize")->currentIndex() + 1);

            if(m_optionWidget->findChild<QRadioButton*>("radQRECC")->isChecked())
                m_bc.bc.setSecurityLevel(m_optionWidget->findChild<QComboBox*>("cmbQRECC")->currentIndex() + 1);
            break;

        case BARCODE_MICROQR:
            m_bc.bc.setSymbol(BARCODE_MICROQR);
            if(m_optionWidget->findChild<QRadioButton*>("radMQRSize")->isChecked())
                m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbMQRSize")->currentIndex() + 1);

            if(m_optionWidget->findChild<QRadioButton*>("radMQRECC")->isChecked())
                m_bc.bc.setSecurityLevel(m_optionWidget->findChild<QComboBox*>("cmbMQRECC")->currentIndex() + 1);
            break;

        case BARCODE_RMQR:
            m_bc.bc.setSymbol(BARCODE_RMQR);

            if(m_optionWidget->findChild<QRadioButton*>("radRMQRGS1")->isChecked())
                m_bc.bc.setInputMode(GS1_MODE);

            if(m_optionWidget->findChild<QRadioButton*>("radRMQRSize")->isChecked())
                m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbRMQRSize")->currentIndex() + 1);

            if(m_optionWidget->findChild<QRadioButton*>("radRMQRECC")->isChecked())
                m_bc.bc.setSecurityLevel(m_optionWidget->findChild<QComboBox*>("cmbRMQRECC")->currentIndex() * 2 + 2);
            break;

        case BARCODE_GRIDMATRIX:
            m_bc.bc.setSymbol(BARCODE_GRIDMATRIX);
            if(m_optionWidget->findChild<QRadioButton*>("radGridSize")->isChecked())
                m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbGridSize")->currentIndex() + 1);

            if(m_optionWidget->findChild<QRadioButton*>("radGridECC")->isChecked())
                m_bc.bc.setSecurityLevel(m_optionWidget->findChild<QComboBox*>("cmbGridECC")->currentIndex() + 1);
            break;

        case BARCODE_MAXICODE:
            m_bc.bc.setSymbol(BARCODE_MAXICODE);
            if(m_optionWidget->findChild<QComboBox*>("cmbMaxiMode")->currentIndex() == 0)
            {
                m_bc.bc.setSecurityLevel(2);
                m_bc.bc.setPrimaryMessage(m_optionWidget->findChild<QLineEdit*>("txtMaxiPrimary")->text());
            }
            else
                m_bc.bc.setSecurityLevel(m_optionWidget->findChild<QComboBox*>("cmbMaxiMode")->currentIndex() + 3);
            break;

        case BARCODE_CHANNEL:
            m_bc.bc.setSymbol(BARCODE_CHANNEL);
            if(m_optionWidget->findChild<QComboBox*>("cmbChannel")->currentIndex() == 0)
                m_bc.bc.setWidth(0);
            else
                m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbChannel")->currentIndex() + 2);
            break;

        case BARCODE_CODEONE:
            m_bc.bc.setSymbol(BARCODE_CODEONE);
            if(m_optionWidget->findChild<QRadioButton*>("radC1GS1")->isChecked())
                m_bc.bc.setInputMode(GS1_MODE);
            m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbC1Size")->currentIndex());
            break;

        case BARCODE_CODE49:
            m_bc.bc.setSymbol(BARCODE_CODE49);
            if(m_optionWidget->findChild<QRadioButton*>("radC49GS1")->isChecked())
                m_bc.bc.setInputMode(GS1_MODE);
            break;

        case BARCODE_HANXIN:
            m_bc.bc.setSymbol(BARCODE_HANXIN);
            if(m_optionWidget->findChild<QRadioButton*>("radHXSize")->isChecked())
                m_bc.bc.setWidth(m_optionWidget->findChild<QComboBox*>("cmbHXSize")->currentIndex() + 1);

            if(m_optionWidget->findChild<QRadioButton*>("radHXECC")->isChecked())
                m_bc.bc.setSecurityLevel(m_optionWidget->findChild<QComboBox*>("cmbHXECC")->currentIndex() + 1);
            break;

        default:
            m_bc.bc.setSymbol(metaObject()->enumerator(0).value(bstyle->currentIndex()));
            break;
    }

    if(chkComposite->isChecked())
        m_bc.bc.setSecurityLevel(cmbCompType->currentIndex());

    m_bc.bc.setBorderType((Zint::QZint::BorderType)(btype->currentIndex()*2));
    m_bc.bc.setBorderWidth(bwidth->value());
    if(chkAutoHeight->isChecked()) {
        m_bc.bc.setHeight(0);
    } else {
        m_bc.bc.setHeight(heightb->value());
    }
    m_bc.bc.setWhitespace(spnWhitespace->value());
    m_bc.bc.setFgColor(m_fgcolor);
    m_bc.bc.setBgColor(m_bgcolor);
    m_bc.setSize(width - 10, height - 10);
    m_bc.update();
    scene->setSceneRect(0, 0, width - 10, height - 10);
    scene->update();
}
