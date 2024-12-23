/* zint_tcl.c TCL binding for zint */
/*
    zint - the open source tcl binding to the zint barcode library
    Copyright (C) 2014-2024 Harald Oehlmann <oehhar@users.sourceforge.net>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
/* SPDX-License-Identifier: BSD-3-Clause */
/*
 History

  2014-06-16 2.5.0 HaO
    First implementation
 2016-09-14 2.5.1 HaO
-   Added Codablock F options "-rows".
-   Adopted to new image format of zint
 2016-10-14 2.5.2 HaO
-   Include the upstream reverted image format
 2016-12-12 2.5.3 HaO
-   No changes here, take 2.5.1 framework files
 2017-05-12 2.6.0 HaO
-   No changes here, take 2.6 framework files
 2017-08-29 2.6.1 HaO
-   Framework 2.6.1 extensions
-   EAN/UPC Codes with included check digit
-   UPNQR Code
-   Misspelled symbology: AztecRunes
 2017-10-23 2.6.2 HaO
-   Framework 2.6.2 bugfixes
-   Allow dll unload
 2018-02-13 2.6.3 HaO
 -  Framework trunk update
 -  Added VIN and MailMark symbologies.
 2018-11-02 2.6.4 HaO
 -  Framework trunk update
 -  Add options -bold, -dotted, -dotsize, -dmre, -eci
 -  Implemented ECI logic
 2019-09-01 2.6.5 HaO
 -  Framework 2.6.5 update
 -  Add option -gssep
 2019-09-18 2.6.6 HaO
 -  Framework 2.6.6 update
 2019-10-07 2.6.7 HaO
 -  Framework 2.6.7 update
 2019-12-05 2.7.0 HaO
 -  Framework 2.7.0 update
 -  Add symbology rmqr
 2020-02-01 2.7.1 HaO
 -  Framework 2.7.1 update
 2020-04-06 HaO
 -  Added option -fullmultibyte
 2020-04-07 2.8.0 HaO
 - Added symbology "UltraCode".
 2020-05-19 HaO
 - Added option -separator to specify stacked symbology separator width
 - -cols maximum changed from 66 to 67
 2020-07-27 2.9.0 HaO
 - added option "-addongap"
 - Renamed symbology names:
    - Matrix2of5 -> Standard2of5
    - PDF417Trunc -> PDF417Compact
    - RSS14Stacked -> GS1DataBarStacked
    - RSS14Stacked -> GS1DataBarStacked
    - RSS14StackedOmni -> GS1DataBarSstackedOmni
    - RSS14ExpandedStacked -> GS1DataBarExpandedStacked
    - OneCode -> USPSIntelligentMail
    - EAN128-CC -> GS1-128-CC
    - RSS14-CC -> GS1DataBarOmni-CC
    - RSSLimited-CC -> GS1DataBarLimited-CC
    - RSSExpandedStacked-CC -> GS1DataBarExpanded-CC
    - RSSEXPanded-CC -> GS1DataBarExpanded-CC
    - RSS14Stacked-CC -> GS1DataBarStacked-CC
    - RSS14Omni-CC -> GS1DataBarStackedOmni-CC
    - RSSExpandedStacked-CC -> GS1DataBarExpandedStacked-CC
    *** Potential incompatibility ***
2020-08-04 2.10.0 HaO
- added symbology "DPDCode"
- Alpha channel support added:
    - added option -nobackground
    - also allow RRGGBBAA for -fg and -bg options
2021-01-05 2.9.1 HaO
- Added options -reverse, -werror, -wzpl
- Use version number from zint.h (first 3 digits). Do not use an own one.
2021-01-14 GL
- Removed TCL native encoding of ECI's and replace by zint buildin mechanism.
  The input is now UTF-8 for any ECI and zint cares about the encoding.
2021-01-14 HaO
- Added detection of presence of the Tk package and late initialization.
  This is a preparation to add a TCL only mode to the DLL.
2021-01-22 GL
- -cols maximum changed from 67 to 108 (DotCode)
2021-05-10 GL
- Added -gs1parens option
2021-05-22 GL
- Added -vwhitesp option
2021-05-28 GL
- -cols maximum changed from 108 to 200 (DotCode)
2021-07-09 GL
- Removed -wzpl, added -gs1nocheck
- Made -format position independent
- Tabs -> spaces
2021-09-21 GL
- Added -guarddescent option
- iHeight check int -> double
2021-09-24 GL
- Added -quietzones and -noquietzones options
2021-09-27 GL
- Added -structapp
- Split up -to parsing (could seg fault if given non-int for X0 or Y0)
2021-10-05 GL
- Added -compliantheight option
2021-10-30 GL
- Added PDF417 -rows
2021-11-19 GL
- Added -heightperrow option
- Added DBAR_EXPSTK, CODE16K, CODE49 -rows
2021-12-17 GL
- Added -fast option
2022-04-08 GL
- Updated ECIs to AIM ITS/04-023:2022
  Note changed names "unicode" -> "utf-16be", "euc-cn" -> "gb2312"
2022-04-24 GL
- Added -segN options
- Added "invariant" and "binary" ECIs
- Tcl_GetIndexFromObj() flags arg -> 0
2022-05-12 GL
- -vers maximum changed to 999 (DAFT)
2022-07-03 GL
- Added BC412
2022-08-20 GL
- Added CEPNet
2022-11-10 GL
- Added -bindtop option
2022-12-02 GL
- Added -scalexdimdp option
- Renamed CODE128B to CODE128AB
    *** Potential incompatibility ***
2022-12-08 GL
- Added MAILMARK_2D
- Renamed MAILMARK to MAILMARK_4S
    *** Potential incompatibility ***
2022-12-09 GL
- Added UPU_S10
2023-01-15 GL
- Added -esc and -extraesc options
2023-02-10 GL
- Added -textgap option
2023-08-11 GL
- Added -guardwhitespace option
2023-10-30 GL
- Added -dmiso144 option
2024-12-09 HaO
- TCL 9 compatibility
- support TCL buildinfo
- remove the zint command on dll unload
2024-12-23 GL
- Added DXFILMEDGE
*/

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#pragma warning(disable : 4201 4214 4514)
#define STRICT
#define WIN32_LEAN_AND_MEAN
/* TCL Defines */
#define DLL_BUILD

#include <windows.h>

/* Define ERROR_INVALID_DATA is also used by zint... */
#ifdef ERROR_INVALID_DATA
#undef ERROR_INVALID_DATA
#endif
#endif

#include <zint.h>
/* Load version defines */
#include "../backend/zintconfig.h"
#include <string.h>

#include <tcl.h>
#include <tk.h>

/* TCL 9 compatibility for TCL 8.6 compile */
#ifndef TCL_SIZE_MAX
#ifndef Tcl_Size
typedef int Tcl_Size;
#endif
# define Tcl_GetSizeIntFromObj Tcl_GetIntFromObj
# define Tcl_NewSizeIntObj Tcl_NewIntObj
# define TCL_SIZE_MAX      INT_MAX
# define TCL_SIZE_MODIFIER ""
#endif

#undef EXPORT
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif


/*----------------------------------------------------------------------------*/
/* >>>>> Helper defines */

/* Two macros are necessary to not include the define name, but the value */
#define STRING(x) #x
#define TOSTRING(x) STRING(x)

/* Define VERSION as the first 3 digits of the zint library version number */
#define VERSION TOSTRING( ZINT_VERSION_MAJOR ) \
        "." TOSTRING( ZINT_VERSION_MINOR ) \
        "." TOSTRING( ZINT_VERSION_RELEASE )

/*----------------------------------------------------------------------------*/
/* >>>> External Prototypes (exports) */
DLLEXPORT int Zint_Init (Tcl_Interp *interp);
DLLEXPORT int Zint_Unload (Tcl_Interp *Interp, int Flags);
/*----------------------------------------------------------------------------*/
/* >>>> local prototypes */
static void InterpCleanupProc(ClientData clientData, Tcl_Interp *interp);
static int CheckForTk(Tcl_Interp *interp, int *tkFlagPtr);
static int ZintCmd(ClientData unused, Tcl_Interp *interp, int objc,
    Tcl_Obj *CONST objv[]);
static int Encode(Tcl_Interp *interp, int objc,
    Tcl_Obj *CONST objv[]);
/*----------------------------------------------------------------------------*/
/* >>>> File Global Variables */

/* >> List of Codes */

static const char *s_code_list[] = {
    "Code11",
    "Standard2of5",
    "Interleaved2of5",
    "IATAC2of5",
    "Logic2of5",
    "Ind2of5",
    "Code39",
    "Code39Extended",
    "EAN",
    "EAN+Check",
    "GS1-128",
    "Codabar",
    "Code128",
    "DPLeit",
    "DPIdent",
    "Code16K",
    "Code49",
    "Code93",
    "Flat",
    "GS1DataBar",
    "GS1DataBarLimited",
    "GS1DataBarExpanded",
    "Telepen",
    "UPC-A",
    "UPC-A+Check",
    "UPC-E",
    "UPC-E+Check",
    "POSTNET",
    "MSIPlessey",
    "FIM",
    "Logmars",
    "Pharma",
    "PZN",
    "PharmaTwo",
    "CEPNet",
    "PDF417",
    "PDF417Compact",
    "MaxiCode",
    "QR",
    "Code128AB",
    "AusPost",
    "AusReply",
    "AusRoute",
    "AusRedirect",
    "ISBN",
    "RM4SCC",
    "Datamatrix",
    "EAN14",
    "VIN",
    "CodablockF",
    "NVE18",
    "JapanPost",
    "KoreaPost",
    "GS1DataBarStacked",
    "GS1DataBarSstackedOmni",
    "GS1DataBarExpandedStacked",
    "PLANET",
    "DPDCode",
    "MicroPDF417",
    "USPSIntelligentMail",
    "Plessey",
    "TelepenNum",
    "ITF14",
    "KIX",
    "Aztec",
    "DAFT",
    "MicroQR",
    "HIBC-128",
    "HIBC-39",
    "HIBC-DM",
    "HIBC-QR",
    "HIBC-PDF",
    "HIBC-MicroPDF",
    "HIBC-CodablockF",
    "HIBCAztec",
    "DotCode",
    "HanXin",
    "MailMark-2D",
    "UPU-S10",
    "MailMark-4S",
    "DXFilmEdge",
    "AztecRunes",
    "Code32",
    "EAN-CC",
    "GS1-128-CC",
    "GS1DataBarOmni-CC",
    "GS1DataBarLimited-CC",
    "GS1DataBarExpanded-CC",
    "UPCA-CC",
    "UPCE-CC",
    "GS1DataBarStacked-CC",
    "GS1DataBarStackedOmni-CC",
    "GS1DataBarExpandedStacked-CC",
    "Channel",
    "CodeOne",
    "GridMatrix",
    "UPNQR",
    "UltraCode",
    "rMQR",
    "BC412",
    NULL};

static const int s_code_number[] = {
    BARCODE_CODE11,
    BARCODE_C25STANDARD,
    BARCODE_C25INTER,
    BARCODE_C25IATA,
    BARCODE_C25LOGIC,
    BARCODE_C25IND,
    BARCODE_CODE39,
    BARCODE_EXCODE39,
    BARCODE_EANX,
    BARCODE_EANX_CHK,
    BARCODE_GS1_128,
    BARCODE_CODABAR,
    BARCODE_CODE128,
    BARCODE_DPLEIT,
    BARCODE_DPIDENT,
    BARCODE_CODE16K,
    BARCODE_CODE49,
    BARCODE_CODE93,
    BARCODE_FLAT,
    BARCODE_DBAR_OMN,
    BARCODE_DBAR_LTD,
    BARCODE_DBAR_EXP,
    BARCODE_TELEPEN,
    BARCODE_UPCA,
    BARCODE_UPCA_CHK,
    BARCODE_UPCE,
    BARCODE_UPCE_CHK,
    BARCODE_POSTNET,
    BARCODE_MSI_PLESSEY,
    BARCODE_FIM,
    BARCODE_LOGMARS,
    BARCODE_PHARMA,
    BARCODE_PZN,
    BARCODE_PHARMA_TWO,
    BARCODE_CEPNET,
    BARCODE_PDF417,
    BARCODE_PDF417COMP,
    BARCODE_MAXICODE,
    BARCODE_QRCODE,
    BARCODE_CODE128AB,
    BARCODE_AUSPOST,
    BARCODE_AUSREPLY,
    BARCODE_AUSROUTE,
    BARCODE_AUSREDIRECT,
    BARCODE_ISBNX,
    BARCODE_RM4SCC,
    BARCODE_DATAMATRIX,
    BARCODE_EAN14,
    BARCODE_VIN,
    BARCODE_CODABLOCKF,
    BARCODE_NVE18,
    BARCODE_JAPANPOST,
    BARCODE_KOREAPOST,
    BARCODE_DBAR_STK,
    BARCODE_DBAR_OMNSTK,
    BARCODE_DBAR_EXPSTK,
    BARCODE_PLANET,
    BARCODE_DPD,
    BARCODE_MICROPDF417,
    BARCODE_USPS_IMAIL,
    BARCODE_PLESSEY,
    BARCODE_TELEPEN_NUM,
    BARCODE_ITF14,
    BARCODE_KIX,
    BARCODE_AZTEC,
    BARCODE_DAFT,
    BARCODE_MICROQR,
    BARCODE_HIBC_128,
    BARCODE_HIBC_39,
    BARCODE_HIBC_DM,
    BARCODE_HIBC_QR,
    BARCODE_HIBC_PDF,
    BARCODE_HIBC_MICPDF,
    BARCODE_HIBC_BLOCKF,
    BARCODE_HIBC_AZTEC,
    BARCODE_DOTCODE,
    BARCODE_HANXIN,
    BARCODE_MAILMARK_2D,
    BARCODE_UPU_S10,
    BARCODE_MAILMARK_4S,
    BARCODE_DXFILMEDGE,
    BARCODE_AZRUNE,
    BARCODE_CODE32,
    BARCODE_EANX_CC,
    BARCODE_GS1_128_CC,
    BARCODE_DBAR_OMN_CC,
    BARCODE_DBAR_LTD_CC,
    BARCODE_DBAR_EXP_CC,
    BARCODE_UPCA_CC,
    BARCODE_UPCE_CC,
    BARCODE_DBAR_STK_CC,
    BARCODE_DBAR_OMNSTK_CC,
    BARCODE_DBAR_EXPSTK_CC,
    BARCODE_CHANNEL,
    BARCODE_CODEONE,
    BARCODE_GRIDMATRIX,
    BARCODE_UPNQR,
    BARCODE_ULTRA,
    BARCODE_RMQR,
    BARCODE_BC412,
    0};

/* ECI TCL encoding names.
 * The ECI comments are given after the name.
 * A ** indicates encodings where native data must be delivered and not utf-8
 */
static const char *s_eci_list[] = {
    "iso8859-1",    /* 3: ISO-8859-1 - Latin alphabet No. 1 (default)*/
    "iso8859-2",    /* 4: ISO-8859-2 - Latin alphabet No. 2*/
    "iso8859-3",    /* 5: ISO-8859-3 - Latin alphabet No. 3*/
    "iso8859-4",    /* 6: ISO-8859-4 - Latin alphabet No. 4*/
    "iso8859-5",    /* 7: ISO-8859-5 - Latin/Cyrillic alphabet*/
    "iso8859-6",    /* 8: ISO-8859-6 - Latin/Arabic alphabet*/
    "iso8859-7",    /* 9: ISO-8859-7 - Latin/Greek alphabet*/
    "iso8859-9",    /*10: ISO-8859-8 - Latin/Hebrew alphabet*/
    "iso8859-9",    /*11: ISO-8859-9 - Latin alphabet No. 5*/
    "iso8859-10",   /*12: ISO-8859-10 - Latin alphabet No. 6*/
    "iso8859-11",   /*13: ISO-8859-11 - Latin/Thai alphabet*/
    "iso8859-13",   /*15: ISO-8859-13 - Latin alphabet No. 7*/
    "iso8859-14",   /*16: ISO-8859-14 - Latin alphabet No. 8 (Celtic)*/
    "iso8859-15",   /*17: ISO-8859-15 - Latin alphabet No. 9*/
    "iso8859-16",   /*18: ISO-8859-16 - Latin alphabet No. 10*/
    "jis0208",      /*20: Shift JIS (JIS X 0208 and JIS X 0201)*/
    "cp1250",       /*21: Windows-1250*/
    "cp1251",       /*22: Windows-1251*/
    "cp1252",       /*23: Windows-1252*/
    "cp1256",       /*24: Windows-1256*/
    "utf-16be",     /*25: UTF-16BE (High order byte first) Unicode*/
    "utf-8",        /*26: Unicode (UTF-8)*/
    "ascii",        /*27: ISO-646:1991 7-bit character set ASCII*/
    "big5",         /*28: Big5 (Taiwan) Chinese Character Set*/
    "gb2312",       /*29: GB 2312 (PRC) Chinese Character Set*/
    "iso2022-kr",   /*30: Korean Character Set EUC-KR (KS X 1001:2002)*/
    "gbk",          /*31: GBK Chinese Character Set*/
    "gb18030",      /*32: GB 18030 Chinese Character Set*/
    "utf-16le",     /*33: UTF-16LE (Low order byte first) Unicode*/
    "utf-32be",     /*34: UTF-32BE (High order byte first) Unicode*/
    "utf-32le",     /*35: UTF-32BE (Low order byte first) Unicode*/
    "invariant",    /*170: ISO-646:1991 7-bit character set invariant*/
    "binary",       /*899: 8-bit binary*/
    NULL
};

/* The ECI numerical number to pass to ZINT */
static const int s_eci_number[] = {
    3,4,5,6,7,8,9,10,11,12,13,15,16,17,18,20,21,22,23,24,25,26,27,28,29,30,
    31,32,33,34,35,170,899
};

/* Version information */
static const char version_string[] = VERSION;
/* Help text */
static const char help_message[] = "zint tcl(stub,obj) dll\n"
    " Generate barcode in tk images\n"
    "Usage:\n"
    " zint encode data photo ?option value? ...\n"
    "  data: data to encode in the symbol\n"
    "  photo: a tcl photo image handle ('p' after 'image create photo p')\n"
    "  Available options:\n"
    "   -barcode choice: symbology, use 'zint symbology' to get a list\n"
    "   -addongap integer: (7..12, default: 9) set add-on gap in multiple of module size (EAN/UPC-CC)\n"
    "   -bg color: set background color as 6 or 8 hex rrggbbaa\n"
    /* cli option --binary internally handled */
    "   -bind bool: bars above/below the code, size set by -border\n"
    "   -bindtop bool: bar above the code, size set by -border\n"
    "   -bold bool: use bold text\n"
    "   -border integer: width of a border around the symbol. Use with -bind/-box/-bindtop 1\n"
    "   -box bool: box around bar code, size set by -border\n"
    /* cli option --cmyk not supported as no corresponding output */
    "   -cols integer: Codablock F, DotCode, PDF417: number of columns\n"
    "   -compliantheight bool: warn if height not compliant, and use standard default\n"
    /* cli option --data is standard parameter */
    "   -dmiso144 bool: Use ISO format for 144x144 Data Matrix symbols\n"
    "   -dmre bool: Allow Data Matrix Rectangular Extended\n"
    "   -dotsize number: radius ratio of dots from 0.01 to 1.0\n" 
    "   -dotty bool: use dots instead of boxes for matrix codes\n"
    /* cli option --dump not supported */
    /* cli option --ecinos not supported */
    "   -eci choice: ECI to use\n"
    /* cli option --embedfont not supported (vector output only) */
    "   -esc bool: Process escape sequences in input data\n"
    "   -extraesc bool: Process symbology-specific escape sequences (Code 128 only)\n"
    "   -fast bool: use fast encodation (Data Matrix)\n"
    "   -fg color: set foreground color as 6 or 8 hex rrggbbaa\n"
    /* replaces cli options --binary and --gs1 */
    "   -format binary|unicode|gs1: input data format. Default:unicode\n"
    "   -fullmultibyte bool: allow multibyte compaction for xQR, HanXin, Gridmatrix\n"
    /* cli option --gs1 replaced by -format */
    "   -gs1nocheck bool: for gs1, do not check validity of data (allows non-standard symbols)\n"
    "   -gs1parens bool: for gs1, AIs enclosed in parentheses instead of square brackets\n"
    "   -gssep bool: for gs1, use gs as separator instead fnc1 (Datamatrix only)\n"
    "   -guarddescent double: Height of guard bar descent in modules (EAN/UPC only)\n"
    "   -guardwhitespace bool: add quiet zone indicators (EAN/UPC only)\n"
    "   -height double: Symbol height in modules\n"
    "   -heightperrow bool: treat height as per-row\n"
    /* cli option --input not supported */
    "   -init bool: Create reader initialisation symbol (Code 128, Data Matrix)\n"
    "   -mask integer: set masking pattern to use (QR/MicroQR/HanXin/DotCode)\n"
    /* cli option --mirror not supported */
    "   -mode integer: set encoding mode (MaxiCode, Composite)\n"
    "   -nobackground bool: set background transparent\n"
    "   -noquietzones bool: disable default quiet zones\n"
    "   -notext bool: no interpretation line\n"
    /* cli option --output not supported */
    "   -primary text: Structured primary data (MaxiCode, Composite)\n"
    "   -quietzones bool: add compliant quiet zones to whitespace\n"
    "   -reverse bool: Reverse colours (white on black)\n"
    "   -rotate angle: Image rotation by 0,90 or 270 degrees\n"
    "   -rows integer: Codablock F, PDF417: number of rows\n"
    "   -scale double: Scale the image to this factor\n"
    "   -scalexdimdp {xdim ?resolution?}: Scale with X-dimension mm, resolution dpmm\n"
    "   -scmvv integer: Prefix SCM with [)>\\R01\\Gvv (vv is integer) (MaxiCode)\n"
    "   -secure integer: EC Level (Aztec, GridMatrix, HanXin, PDF417, QR, UltraCode)\n"
    "   -segN {eci data}: Set the ECI & data content for segment N where N is 1 to 9\n"
    "   -separator 0..4 (default: 1) : Stacked symbologies: separator width\n"
    /* cli option --small replaced by -smalltext */
    "   -smalltext bool: tiny interpretation line font\n"
    "   -square bool: force Data Matrix symbols to be square\n"
    "   -structapp {index count ?id?}: set Structured Append info\n"
    "   -textgap double: Gap between barcode and text\n"
    /* cli option --types not supported */
    "   -vers integer: Symbology option\n"
    /* cli option --version not supported */
    "   -vwhitesp integer: vertical quiet zone in modules\n"
    "   -whitesp integer: horizontal quiet zone in modules\n"
    "   -werror bool: Convert all warnings into errors\n"
    "   -to {x0 y0 ?width? ?height?}: place to put in photo image\n"
    "\n"
    "zint symbologies: List available symbologies\n"
    "zint eci: List available eci tables\n"
    "zint help\n"
    "zint version\n"
    ;
    
/*----------------------------------------------------------------------------*/
/* Exported symbols */
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
EXPORT BOOL WINAPI DllEntryPoint (HINSTANCE hInstance,
    DWORD seginfo, LPVOID lpCmdLine)
{
  /* Don't do anything, so just return true */
  return TRUE;
}
#endif
/*----------------------------------------------------------------------------*/
/* Initialisation Procedures */
DLLEXPORT int Zint_Init (Tcl_Interp *interp)
{
    int * tkFlagPtr;
    Tcl_CmdInfo info;
    /*------------------------------------------------------------------------*/
    /* If TCL_STUB is not defined, the following only does a version check    */
    if (Tcl_InitStubs(interp, "8.5-", 0) == NULL) {
        return TCL_ERROR;
    }
    /*------------------------------------------------------------------------*/
    /* Add build info                                                         */
    if (Tcl_GetCommandInfo(interp, "::tcl::build-info", &info)) {
	Tcl_CreateObjCommand(interp, "::zint::build-info",
		info.objProc, (void *)(
		    PACKAGE_VERSION "+" STRINGIFY(SAMPLE_VERSION_UUID)
#if defined(__clang__) && defined(__clang_major__)
			    ".clang-" STRINGIFY(__clang_major__)
#if __clang_minor__ < 10
			    "0"
#endif
			    STRINGIFY(__clang_minor__)
#endif
#if defined(__cplusplus) && !defined(__OBJC__)
			    ".cplusplus"
#endif
#ifndef NDEBUG
			    ".debug"
#endif
#if !defined(__clang__) && !defined(__INTEL_COMPILER) && defined(__GNUC__)
			    ".gcc-" STRINGIFY(__GNUC__)
#if __GNUC_MINOR__ < 10
			    "0"
#endif
			    STRINGIFY(__GNUC_MINOR__)
#endif
#ifdef __INTEL_COMPILER
			    ".icc-" STRINGIFY(__INTEL_COMPILER)
#endif
#ifdef TCL_MEM_DEBUG
			    ".memdebug"
#endif
#if defined(_MSC_VER)
			    ".msvc-" STRINGIFY(_MSC_VER)
#endif
#ifdef USE_NMAKE
			    ".nmake"
#endif
#ifndef TCL_CFG_OPTIMIZED
			    ".no-optimize"
#endif
#ifdef __OBJC__
			    ".objective-c"
#if defined(__cplusplus)
			    "plusplus"
#endif
#endif
#ifdef TCL_CFG_PROFILED
			    ".profile"
#endif
#ifdef PURIFY
			    ".purify"
#endif
#ifdef STATIC_BUILD
			    ".static"
#endif
		), NULL);
    }
    /*------------------------------------------------------------------------*/
    /* This procedure is called once per thread and any thread local data     */
    /* should be allocated and initialized here (and not in static variables) */
    
    /* Create a flag if Tk is loaded */
    tkFlagPtr = (int *)ckalloc(sizeof(int));
    *tkFlagPtr = 0;
    Tcl_CallWhenDeleted(interp, InterpCleanupProc, (ClientData)tkFlagPtr);
    /*------------------------------------------------------------------------*/
    Tcl_CreateObjCommand(interp, "zint", ZintCmd, (ClientData)tkFlagPtr,
            (Tcl_CmdDeleteProc *)NULL);
    Tcl_PkgProvide (interp, "zint", version_string);
    /*------------------------------------------------------------------------*/
    return TCL_OK;
}
/*----------------------------------------------------------------------------*/
/* >>>> Cleanup procedure */
/*----------------------------------------------------------------------------*/
/* This routine is called, if a thread is terminated */
static void InterpCleanupProc(ClientData clientData, Tcl_Interp *interp)
{
    ckfree( (char *)clientData );
}
/*----------------------------------------------------------------------------*/
/* >>>> Unload Procedures */
/*----------------------------------------------------------------------------*/
DLLEXPORT int Zint_Unload (Tcl_Interp *Interp, int Flags)
{
    /* Remove created commands */
    Tcl_DeleteCommand(Interp, "::zint::build-info");
    Tcl_DeleteCommand(Interp, "zint");
    // Allow unload
    return TCL_OK;
}
/*----------------------------------------------------------------------------*/
/* >>>>> Called routine */
/*----------------------------------------------------------------------------*/
/* Decode tcl commands */
static int ZintCmd(ClientData tkFlagPtr, Tcl_Interp *interp, int objc,
    Tcl_Obj *CONST objv[])
{
    /* Option list and indexes */
    enum iCommand {iEncode, iSymbologies, iECI, iVersion, iHelp};
    /* choice of option */
    int Index;
    /*------------------------------------------------------------------------*/
    /* > Check if option argument is given and decode it */
    if (objc > 1)
    {
        char *subCmds[] = {"encode", "symbologies", "eci", "version", "help", NULL};
        if(Tcl_GetIndexFromObj(interp, objv[1], (const char **) subCmds,
            "option", 0, &Index)
            == TCL_ERROR)
        {
            return TCL_ERROR;
        }
    } else {
        Tcl_WrongNumArgs(interp, 1, objv, "option");
        return TCL_ERROR;
    }
    /*------------------------------------------------------------------------*/
    /* > Call functions in dependency of Index */
    /*------------------------------------------------------------------------*/
    switch (Index)
    {
    case iEncode:
        if (CheckForTk(interp, (int *)tkFlagPtr) != TCL_OK) {
            return TCL_ERROR;
        }
        return Encode(interp, objc, objv);
    case iSymbologies:
        {
            Tcl_Obj *oRes;
            int posCur;
            oRes = Tcl_NewObj();
            for (posCur = 0 ; s_code_list[posCur] != NULL; posCur++) {
                if( ZBarcode_ValidID(s_code_number[posCur]) != 0) {
                    if (TCL_OK != Tcl_ListObjAppendElement(interp,
                        oRes, Tcl_NewStringObj(s_code_list[posCur],-1)))
                    {
                        return TCL_ERROR;
                    }
                }
            }
            Tcl_SetObjResult(interp,oRes);
            return TCL_OK;
        }
    case iECI:
        {
            Tcl_Obj *oRes;
            int posCur;
            oRes = Tcl_NewObj();
            for (posCur = 0 ; s_eci_list[posCur] != NULL; posCur++) {
                if (TCL_OK != Tcl_ListObjAppendElement(interp,
                    oRes, Tcl_NewStringObj(s_eci_list[posCur],-1)))
                {
                    return TCL_ERROR;
                }
            }
            Tcl_SetObjResult(interp,oRes);
            return TCL_OK;
        }
    case iVersion:
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj(version_string, -1));
        return TCL_OK;
    case iHelp:
    default:
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj(help_message, -1));
        return TCL_OK;
    }
}
/*----------------------------------------------------------------------
 * Check availability of Tk.
 *----------------------------------------------------------------------
 */
static int CheckForTk(Tcl_Interp *interp, int *tkFlagPtr)
{
    if (*tkFlagPtr > 0) {
        return TCL_OK;
    }
    if (*tkFlagPtr == 0) {
        if ( ! Tcl_PkgPresent(interp, "Tk", "8.5-", 0) ) {
            Tcl_SetResult(interp, "package Tk not loaded", TCL_STATIC);
            return TCL_ERROR;
        }
    }
#ifdef USE_TK_STUBS
    if (*tkFlagPtr < 0 || Tk_InitStubs(interp, "8.5-", 0) == NULL) {
        *tkFlagPtr = -1;
        Tcl_SetResult(interp, "error initializing Tk", TCL_STATIC);
        return TCL_ERROR;
    }
#endif
    *tkFlagPtr = 1;
    return TCL_OK;
}/*----------------------------------------------------------------------------*/
/* >>>>> Encode */
/*----------------------------------------------------------------------------*/
/* Encode image */
static int Encode(Tcl_Interp *interp, int objc,
    Tcl_Obj *CONST objv[])
{
    struct zint_symbol *my_symbol;
    Tcl_DString dsInput;
    char *pStr = NULL;
    Tcl_Size lStr;
    Tcl_Encoding hZINTEncoding;
    int rotate_angle=0;
    int fError = 0;
    Tcl_DString dString;
    int optionPos;
    int destX0 = 0;
    int destY0 = 0;
    int destWidth = 0;
    int destHeight = 0;
    int ECIIndex = 0;
    int fFullMultiByte = 0;
    int addon_gap = 0;
    int Separator = 1;
    int Mask = 0;
    int rows = 0;
    unsigned int cap;
    int seg_count = 0;
    int seg_no;
    Tcl_Obj *pSegDataObjs[10] = {0};
    Tcl_DString segInputs[10];
    struct zint_seg segs[10];
    double xdim = 0.0;
    double resolution = 0.0;
    /*------------------------------------------------------------------------*/
    /* >> Check if at least data and object is given and a pair number of */
    /* >> options */
    if ( objc < 4 || (objc % 2) != 0 )
    {
        Tcl_WrongNumArgs(interp, 2, objv, "data photo ?-switch value?...");
        return TCL_ERROR;
    }
    /*------------------------------------------------------------------------*/
    /* >>> Prepare encoding */
    hZINTEncoding = Tcl_GetEncoding(interp, "utf-8");
    if (NULL == hZINTEncoding) {
        return TCL_ERROR;
    }
    /*------------------------------------------------------------------------*/
    /* >>> Prepare zint object */
    my_symbol = ZBarcode_Create();
    my_symbol->input_mode = UNICODE_MODE;
    my_symbol->option_3 = 0;
    /*------------------------------------------------------------------------*/
    /* >> Decode options */
    for (optionPos = 4; optionPos < objc; optionPos+=2) {
        /*--------------------------------------------------------------------*/
        /* Option list and indexes */
        static const char *optionList[] = {
            "-addongap", "-barcode", "-bg", "-bind", "-bindtop", "-bold", "-border", "-box",
            "-cols", "-compliantheight", "-dmiso144", "-dmre", "-dotsize", "-dotty",
            "-eci", "-esc", "-extraesc", "-fast", "-fg", "-format", "-fullmultibyte",
            "-gs1nocheck", "-gs1parens", "-gssep", "-guarddescent", "-guardwhitespace",
            "-height", "-heightperrow", "-init", "-mask", "-mode",
            "-nobackground", "-noquietzones", "-notext", "-primary", "-quietzones",
            "-reverse", "-rotate", "-rows", "-scale", "-scalexdimdp", "-scmvv", "-secure",
            "-seg1", "-seg2", "-seg3", "-seg4", "-seg5", "-seg6", "-seg7", "-seg8", "-seg9",
            "-separator", "-smalltext", "-square", "-structapp",
            "-textgap", "-to", "-vers", "-vwhitesp", "-werror", "-whitesp",
            NULL};
        enum iOption {
            iAddonGap, iBarcode, iBG, iBind, iBindTop, iBold, iBorder, iBox,
            iCols, iCompliantHeight, iDMISO144, iDMRE, iDotSize, iDotty,
            iECI, iEsc, iExtraEsc, iFast, iFG, iFormat, iFullMultiByte,
            iGS1NoCheck, iGS1Parens, iGSSep, iGuardDescent, iGuardWhitespace,
            iHeight, iHeightPerRow, iInit, iMask, iMode,
            iNoBackground, iNoQuietZones, iNoText, iPrimary, iQuietZones,
            iReverse, iRotate, iRows, iScale, iScaleXdimDp, iSCMvv, iSecure,
            iSeg1, iSeg2, iSeg3, iSeg4, iSeg5, iSeg6, iSeg7, iSeg8, iSeg9,
            iSeparator, iSmallText, iSquare, iStructApp,
            iTextGap, iTo, iVers, iVWhiteSp, iWError, iWhiteSp
            };
        int optionIndex;
        int intValue;
        double doubleValue;
        /*--------------------------------------------------------------------*/
        if(Tcl_GetIndexFromObj(interp, objv[optionPos],
            (const char **) optionList,
            "zint option", 0, &optionIndex)
            == TCL_ERROR)
        {
            fError = 1;
            break;
        }
        /*--------------------------------------------------------------------*/
        /* >> Decode object */
        switch (optionIndex) {
        case iBind:
        case iBindTop:
        case iBold:
        case iBox:
        case iCompliantHeight:
        case iDMISO144:
        case iDMRE:
        case iDotty:
        case iEsc:
        case iExtraEsc:
        case iFast:
        case iGS1NoCheck:
        case iGS1Parens:
        case iGSSep:
        case iGuardWhitespace:
        case iHeightPerRow:
        case iInit:
        case iNoBackground:
        case iNoQuietZones:
        case iNoText:
        case iQuietZones:
        case iSmallText:
        case iSquare:
        case iFullMultiByte:
        case iReverse:
        case iWError:
            /* >> Binary options */
            if (TCL_OK != Tcl_GetBooleanFromObj(interp, objv[optionPos+1],
                    &intValue))
            {
                fError = 1;
            }
            break;
        case iFG:
        case iBG:
            /* >> Colors */
            pStr = Tcl_GetStringFromObj(objv[optionPos+1],&lStr);
            if (lStr != 6 && lStr != 8) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Color is not 6 or 8 hex",-1));
                fError = 1;
            }
            break;
        case iHeight:
        case iGuardDescent:
        case iDotSize:
        case iScale:
        case iTextGap:
            /* >> Float */
            if (TCL_OK != Tcl_GetDoubleFromObj(interp, objv[optionPos+1],
                &doubleValue))
            {
                fError = 1;
            }
            break;
        case iAddonGap:
        case iBorder:
        case iCols:
        case iMask:
        case iMode:
        case iRotate:
        case iRows:
        case iSecure:
        case iSeparator:
        case iSCMvv:
        case iVers:
        case iVWhiteSp:
        case iWhiteSp:
            /* >> Int */
            if (TCL_OK != Tcl_GetIntFromObj(interp, objv[optionPos+1],
                    &intValue))
            {
                fError = 1;
            }
            break;
        case iPrimary:
            /* > Primary String up to 90 characters */
            /* > Output filename up to 250 characters */
            Tcl_DStringInit(& dString);
            pStr = Tcl_GetStringFromObj(objv[optionPos+1], &lStr);
            Tcl_UtfToExternalDString( hZINTEncoding, pStr, lStr, &dString);
            if (Tcl_DStringLength(&dString) > (optionIndex==iPrimary?90:250)) {
                Tcl_DStringFree(&dString);
                Tcl_SetObjResult(interp,Tcl_NewStringObj("String too long", -1));
                fError = 1;
            }
            break;
        case iSeg1: case iSeg2: case iSeg3: case iSeg4: case iSeg5:
        case iSeg6: case iSeg7: case iSeg8: case iSeg9:
            seg_no = optionIndex - iSeg1 + 1;
            if (pSegDataObjs[seg_no]) {
                Tcl_SetObjResult(interp, Tcl_NewStringObj("duplicate segment", -1));
                fError = 1;
            } else {
                Tcl_Obj *poParam;
                if (TCL_OK != Tcl_ListObjLength(interp, objv[optionPos+1], &lStr)) {
                    Tcl_SetObjResult(interp, Tcl_Format(interp, "option %s not a list", 1, objv + optionPos));
                    fError = 1;
                } else if (lStr != 2) {
                    Tcl_SetObjResult(interp, Tcl_Format(interp, "option %s not a list of 2", 1, objv + optionPos));
                    fError = 1;
                } else if (TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                        0, &poParam)
                        || TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                            1, &pSegDataObjs[seg_no])) {
                    Tcl_SetObjResult(interp, Tcl_Format(interp, "option %s list format is {eci data}", 1, objv + optionPos));
                    fError = 1;
                } else if (Tcl_GetIndexFromObj(interp, poParam,
                            (const char **) s_eci_list, Tcl_GetString(objv[optionPos]), 0, &ECIIndex)
                            == TCL_ERROR) {
                    fError = 1;
                } else {
                    segs[seg_no].eci = s_eci_number[ECIIndex];
                    if (seg_no >= seg_count) {
                        seg_count = seg_no + 1;
                    }
                }
            }
            break;
        }
        if (fError) {
            break;
        }
        /*--------------------------------------------------------------------*/
        switch (optionIndex) {
        case iAddonGap:
            if (intValue < 7 || intValue > 12) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Invalid add-on gap value not within 7 to 12", -1));
                fError = 1;
            } else {
                addon_gap = intValue;
            }
            break;
        case iBind:
            if (intValue) {
                my_symbol->output_options |= BARCODE_BIND;
            } else {
                my_symbol->output_options &= ~BARCODE_BIND;
            }
            break;
        case iBindTop:
            if (intValue) {
                my_symbol->output_options |= BARCODE_BIND_TOP;
            } else {
                my_symbol->output_options &= ~BARCODE_BIND_TOP;
            }
            break;
        case iBold:
            if (intValue) {
                my_symbol->output_options |= BOLD_TEXT;
            } else {
                my_symbol->output_options &= ~BOLD_TEXT;
            }
            break;
        case iBox:
            if (intValue) {
                my_symbol->output_options |= BARCODE_BOX;
            } else {
                my_symbol->output_options &= ~BARCODE_BOX;
            }
            break;
        case iCompliantHeight:
            if (intValue) {
                my_symbol->output_options |= COMPLIANT_HEIGHT;
            } else {
                my_symbol->output_options &= ~COMPLIANT_HEIGHT;
            }
            break;
        case iDotSize:
            if (doubleValue < 0.01) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Dot size below 0.01", -1));
                fError = 1;
            } else {
                my_symbol->dot_size = (float)doubleValue;
            }
            break;
        case iDotty:
            if (intValue) {
                my_symbol->output_options |= BARCODE_DOTTY_MODE;
            } else {
                my_symbol->output_options &= ~BARCODE_DOTTY_MODE;
            }
            break;
        case iEsc:
            if (intValue) {
                my_symbol->input_mode |= ESCAPE_MODE;
            } else {
                my_symbol->input_mode &= ~ESCAPE_MODE;
            }
            break;
        case iExtraEsc:
            if (intValue) {
                my_symbol->input_mode |= EXTRA_ESCAPE_MODE;
            } else {
                my_symbol->input_mode &= ~EXTRA_ESCAPE_MODE;
            }
            break;
        case iFast:
            if (intValue) {
                my_symbol->input_mode |= FAST_MODE;
            } else {
                my_symbol->input_mode &= ~FAST_MODE;
            }
            break;
        case iGS1NoCheck:
            if (intValue) {
                my_symbol->input_mode |= GS1NOCHECK_MODE;
            } else {
                my_symbol->input_mode &= ~GS1NOCHECK_MODE;
            }
            break;
        case iGS1Parens:
            if (intValue) {
                my_symbol->input_mode |= GS1PARENS_MODE;
            } else {
                my_symbol->input_mode &= ~GS1PARENS_MODE;
            }
            break;
        case iGSSep:
            if (intValue) {
                my_symbol->output_options |= GS1_GS_SEPARATOR;
            } else {
                my_symbol->output_options &= ~GS1_GS_SEPARATOR;
            }
            break;
        case iFullMultiByte:
            fFullMultiByte = intValue;
            break;
        case iECI:
            if(Tcl_GetIndexFromObj(interp, objv[optionPos+1],
                (const char **) s_eci_list, "-eci", 0, &ECIIndex)
                == TCL_ERROR)
            {
                fError = 1;
            } else {
                my_symbol->eci = s_eci_number[ECIIndex];
            }
            break;
        case iGuardWhitespace:
            if (intValue) {
                my_symbol->output_options |= EANUPC_GUARD_WHITESPACE;
            } else {
                my_symbol->output_options &= ~EANUPC_GUARD_WHITESPACE;
            }
            break;
        case iHeightPerRow:
            if (intValue) {
                my_symbol->input_mode |= HEIGHTPERROW_MODE;
            } else {
                my_symbol->input_mode &= ~HEIGHTPERROW_MODE;
            }
            break;
        case iInit:
            if (intValue) {
                my_symbol->output_options |= READER_INIT;
            } else {
                my_symbol->output_options &= ~READER_INIT;
            }
            break;
        case iSmallText:
            if (intValue) {
                my_symbol->output_options |= SMALL_TEXT;
            } else {
                my_symbol->output_options &= ~SMALL_TEXT;
            }
            break;
        case iReverse:
            if (intValue) {
                strcpy(my_symbol->fgcolour, "ffffff");
                strcpy(my_symbol->bgcolour, "000000");
            }
            break;
        case iWError:
            if (intValue) {
                my_symbol->warn_level = WARN_FAIL_ALL;
            }
            break;
        case iFG:
            strncpy(my_symbol->fgcolour, pStr, lStr);
            my_symbol->fgcolour[lStr]='\0';
            break;
        case iBG:
            strncpy(my_symbol->bgcolour, pStr, lStr);
            my_symbol->bgcolour[lStr]='\0';
            break;
        case iNoBackground:
            if (intValue) {
                strcpy(my_symbol->bgcolour, "ffffff00");
            }
            break;
        case iNoQuietZones:
            if (intValue) {
                my_symbol->output_options |= BARCODE_NO_QUIET_ZONES;
            } else {
                my_symbol->output_options &= ~BARCODE_NO_QUIET_ZONES;
            }
            break;
        case iNoText:
            my_symbol->show_hrt = (intValue?0:1);
            break;
        case iQuietZones:
            if (intValue) {
                my_symbol->output_options |= BARCODE_QUIET_ZONES;
            } else {
                my_symbol->output_options &= ~BARCODE_QUIET_ZONES;
            }
            break;
        case iSquare:
            /* DM_SQUARE overwrites DM_DMRE */
            if (intValue)
                my_symbol->option_3 = DM_SQUARE | (my_symbol->option_3 & ~0x7F);
            break;
        case iDMRE:
            /* DM_DMRE overwrites DM_SQUARE */
            if (intValue)
                my_symbol->option_3 = DM_DMRE | (my_symbol->option_3 & ~0x7F);
            break;
        case iDMISO144:
            if (intValue)
                my_symbol->option_3 |= DM_ISO_144;
            break;
        case iScale:
            if (doubleValue < 0.01) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Scale below 0.01", -1));
                fError = 1;
            } else {
                my_symbol->scale = (float)doubleValue;
            }
            break;
        case iTextGap:
            if (doubleValue < 0.0 || doubleValue > 5.0) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Text Gap out of range", -1));
                fError = 1;
            } else {
                my_symbol->text_gap = (float)doubleValue;
            }
            break;
        case iScaleXdimDp:
            /* >> Decode the -scalexdimdp parameter as list of xdim ?resolution? */
            {
                Tcl_Obj *poParam;
                xdim = resolution = 0.0;
                if (TCL_OK != Tcl_ListObjLength(interp,
                    objv[optionPos+1], &lStr))
                {
                    fError = 1;
                } else if ( ! ( lStr == 1 || lStr == 2 ) ) {
                    Tcl_SetObjResult(interp,
                        Tcl_NewStringObj(
                        "option -scalexdimdp not a list of 1 or 2", -1));
                    fError = 1;
                } else {
                    if (TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                        0, &poParam)
                        || TCL_OK != Tcl_GetDoubleFromObj(interp, poParam, &xdim)
                        || xdim < 0.0)
                    {
                        fError = 1;
                    }
                    if (!fError && lStr == 2 && (
                        TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                            1, &poParam)
                        || TCL_OK != Tcl_GetDoubleFromObj(interp, poParam, &resolution)
                        || resolution < 0.0))
                    {
                        fError = 1;
                    }
                    if (!fError && resolution == 0.0) {
                        resolution = 12.0; /* Default 12 dpmm (~300 dpi) */
                    }
                }
            }
            break;
        case iBorder:
            if (intValue < 0 || intValue > 1000) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Border out of range", -1));
                fError = 1;
            } else {
                my_symbol->border_width = intValue;
            }
            break;
        case iGuardDescent:
            if ((float)doubleValue < 0.0f || (float)doubleValue > 50.0f) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Guard bar descent out of range", -1));
                fError = 1;
            } else {
                my_symbol->guard_descent = (float)doubleValue;
            }
            break;
        case iHeight:
            if ((float)doubleValue < 0.5f || (float)doubleValue > 2000.0f) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Height out of range", -1));
                fError = 1;
            } else {
                my_symbol->height = (float)doubleValue;
            }
            break;
        case iSeparator:
            if (intValue < 0 || intValue > 4) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Separator out of range", -1));
                fError = 1;
            } else {
                Separator = intValue;
            }
            break;
        case iMask:
            if (intValue < 0 || intValue > 7) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Mask out of range", -1));
                fError = 1;
            } else {
                Mask = intValue + 1;
            }
            break;
        case iSCMvv:
            if (intValue < 0 || intValue > 99) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("SCM version out of range", -1));
                fError = 1;
            } else {
                my_symbol->option_2 = intValue + 1;
            }
            break;
        case iCols:
        case iVers:
            /* >> Int in Option 2 */
            if (intValue < 1
                || (optionIndex==iCols && intValue > 200)
                || (optionIndex==iVers && intValue > 999))
            {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("cols/vers out of range", -1));
                fError = 1;
            } else {
                my_symbol->option_2 = intValue;
            }
            break;
        case iSecure:
        case iMode:
        case iRows:
            /* >> Int in Option 1 for Codablock, Option 3 for PDF417 */
            if ( (optionIndex==iSecure && (intValue < 1 || intValue > 8))
                || (optionIndex==iMode && (intValue < 0 || intValue > 6))
                || (optionIndex==iRows && (intValue < 0 || intValue > 90)))
            {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("secure/mode/rows out of range", -1));
                fError = 1;
            } else {
                rows = intValue;
            }
            break;
        case iPrimary:
            strcpy(my_symbol->primary, Tcl_DStringValue( &dString ) );
            Tcl_DStringFree(&dString);
            break;
        case iRotate:
            /* >> Rotate angle */
            /*----------------------------------------------------------------*/
            {
                char *rotateList[] = {"0", "90", "180", "270", NULL};
                enum iRotate { iRotate0, iRotate90, iRotate180, iRotate270 };
                /*------------------------------------------------------------*/
                if(Tcl_GetIndexFromObj(interp, objv[optionPos+1],
                    (const char **) rotateList,
                    "-rotate", 0, &intValue)
                    == TCL_ERROR)
                {
                    fError = 1;
                    break;
                }
                switch (intValue) {
                    case iRotate90: rotate_angle = 90; break;
                    case iRotate180: rotate_angle = 180; break;
                    case iRotate270: rotate_angle = 270; break;
                    default: rotate_angle = 0; break;
                }
            }
            break;
        case iBarcode:
            if(Tcl_GetIndexFromObj(interp, objv[optionPos+1],
                (const char **) s_code_list, "-barcode", 0, &intValue)
                == TCL_ERROR)
            {
                fError = 1;
            } else {
                my_symbol->symbology = s_code_number[intValue];
            }
            break;
        case iVWhiteSp:
            my_symbol->whitespace_height = intValue;
            break;
        case iWhiteSp:
            my_symbol->whitespace_width = intValue;
            break;
        case iStructApp:
            /* >> Decode the -structapp parameter as list of index count ?ID? */
            {
                Tcl_Obj *poParam;
                struct zint_structapp structapp = { 0, 0, "" };
                char *pStructAppId = NULL;
                Tcl_Size lStructAppId = 0;
                if (TCL_OK != Tcl_ListObjLength(interp,
                    objv[optionPos+1], &lStr))
                {
                    fError = 1;
                } else if ( ! ( lStr == 2 || lStr == 3 ) ) {
                    Tcl_SetObjResult(interp,
                        Tcl_NewStringObj(
                        "option -structapp not a list of 2 or 3", -1));
                    fError = 1;
                } else {
                    if (TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                        0, &poParam)
                        || TCL_OK != Tcl_GetIntFromObj(interp, poParam, &structapp.index)
                        || TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                            1, &poParam)
                        || TCL_OK != Tcl_GetIntFromObj(interp, poParam, &structapp.count))
                    {
                        fError = 1;
                    }
                    if (!fError && lStr == 3 && (
                        TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                            2, &poParam)
                        || !(pStructAppId = Tcl_GetStringFromObj(poParam, &lStructAppId))
                        || lStructAppId > 32
                        ))
                    {
                        if (lStructAppId > 32) {
                            Tcl_SetObjResult(interp,
                                Tcl_NewStringObj("Structured Append ID too long", -1));
                        }
                        fError = 1;
                    }
                    if (!fError) {
                        my_symbol->structapp = structapp;
                        if (lStr == 3 && pStructAppId && lStructAppId) {
                            strncpy(my_symbol->structapp.id, pStructAppId, lStructAppId);
                        }
                    }
                }
            }
            break;
        case iTo:
            /* >> Decode the -to parameter as list of X0 Y0 ?Width Height? */
            {
                Tcl_Obj *poParam;
                if (TCL_OK != Tcl_ListObjLength(interp,
                    objv[optionPos+1], &lStr))
                {
                    fError = 1;
                } else if ( ! ( lStr == 2 || lStr == 4 ) ) {
                    Tcl_SetObjResult(interp,
                        Tcl_NewStringObj(
                        "option -to not a list of 2 or 4", -1));
                    fError = 1;
                } else {
                    if (TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                            0, &poParam)
                        || TCL_OK != Tcl_GetIntFromObj(interp,poParam,&destX0)
                        || TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                            1, &poParam)
                        || TCL_OK != Tcl_GetIntFromObj(interp,poParam,&destY0))
                    {
                        fError = 1;
                    }
                    if (!fError && lStr == 4 && (
                        TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                            2, &poParam)
                        || TCL_OK != Tcl_GetIntFromObj(interp,poParam,
                            &destWidth)
                        || TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                            3, &poParam)
                        || TCL_OK != Tcl_GetIntFromObj(interp,poParam,
                            &destHeight)
                        ))
                    {
                        fError = 1;
                    }
                }
            }
            break;
        case iFormat:
            /* >> Format of the input data */
            /*----------------------------------------------------------------*/
            {
                char *formatList[] = {"binary", "gs1", "unicode",NULL};
                enum iFormat { iBinary, iGS1, iUnicode };
                /*------------------------------------------------------------*/
                if(Tcl_GetIndexFromObj(interp, objv[optionPos+1],
                    (const char **) formatList,
                    "-format", 0, &intValue)
                    == TCL_ERROR)
                {
                    fError = 1;
                    break;
                }
                switch (intValue) {
                    case iBinary: my_symbol->input_mode = (my_symbol->input_mode & ~0x07) | DATA_MODE; break;
                    case iGS1: my_symbol->input_mode = (my_symbol->input_mode & ~0x07) | GS1_MODE; break;
                    default: my_symbol->input_mode = (my_symbol->input_mode & ~0x07) | UNICODE_MODE; break;
                }
            }
        }
    }
    /*------------------------------------------------------------------------*/
    /* >>> Get symbology capability mask */
    cap = ZBarcode_Cap(my_symbol->symbology,
            ZINT_CAP_STACKABLE | ZINT_CAP_EXTENDABLE | ZINT_CAP_FULL_MULTIBYTE
            | ZINT_CAP_MASK);
    /*------------------------------------------------------------------------*/
    /* >>> option_3 is set by three values depending on the symbology */
    /* On wrong symbology, the option is ignored(as does the zint program)*/
    if (fFullMultiByte && (cap & ZINT_CAP_FULL_MULTIBYTE)) {
        my_symbol->option_3 = ZINT_FULL_MULTIBYTE;
    }
    if (Mask && (cap & ZINT_CAP_MASK)) {
        my_symbol->option_3 |= Mask << 8;
    }
    if (Separator && (cap & ZINT_CAP_STACKABLE)) {
        my_symbol->option_3 = Separator;
    }
    /*------------------------------------------------------------------------*/
    /* >>> option_2 is set by two values depending on the symbology */
    /* On wrong symbology, the option is ignored(as does the zint program)*/
    if (addon_gap && (cap & ZINT_CAP_EXTENDABLE)) {
        my_symbol->option_2 = addon_gap;
    }
    /*------------------------------------------------------------------------*/
    if (rows) {
        /* PDF417 and DBAR_EXPSTK use option 3 for rows */
        if (my_symbol->symbology == BARCODE_PDF417
                || my_symbol->symbology == BARCODE_PDF417COMP
                || my_symbol->symbology == BARCODE_HIBC_PDF
                || my_symbol->symbology == BARCODE_DBAR_EXPSTK
                || my_symbol->symbology == BARCODE_DBAR_EXPSTK_CC) {
            my_symbol->option_3 = rows;
        } else if (my_symbol->symbology == BARCODE_CODABLOCKF
                || my_symbol->symbology == BARCODE_HIBC_BLOCKF
                || my_symbol->symbology == BARCODE_CODE16K
                || my_symbol->symbology == BARCODE_CODE49) {
            my_symbol->option_1 = rows;
        }
    }
    if (resolution) {
        float scale;
        if (xdim == 0.0) {
            xdim = ZBarcode_Default_Xdim(my_symbol->symbology);
        }
        scale = ZBarcode_Scale_From_XdimDp(my_symbol->symbology, (float)xdim, (float)resolution, NULL /*filetype*/);
        if (scale > 0.0f) {
            my_symbol->scale = scale;
        }
    }
    /*------------------------------------------------------------------------*/
    /* >>> Prepare input dstring and encode it to ECI encoding*/
    Tcl_DStringInit(& dsInput);
    /*------------------------------------------------------------------------*/
    if (!fError) {
        /*--------------------------------------------------------------------*/
        /* >>> Get input mode */
        if ((my_symbol->input_mode & 0x07) == DATA_MODE) {
            /* Binary data */
            pStr = (char *) Tcl_GetByteArrayFromObj(objv[2], &lStr);
        } else {
            /* UTF8 Data */
            pStr = Tcl_GetStringFromObj(objv[2], &lStr);
            Tcl_UtfToExternalDString( hZINTEncoding, pStr, lStr, &dsInput);
            pStr = Tcl_DStringValue( &dsInput );
            lStr = Tcl_DStringLength( &dsInput );
        }
        if (seg_count) {
            segs[0].source = (unsigned char *) pStr;
            segs[0].length = (int)lStr;
            segs[0].eci = my_symbol->eci;
            for (seg_no = 1; seg_no < seg_count; seg_no++) {
                if (!pSegDataObjs[seg_no]) {
                    Tcl_SetObjResult(interp, Tcl_NewStringObj("Segments must be consecutive", -1));
                    fError = 1;
                    break;
                }
            }
            if (!fError) {
                for (seg_no = 1; seg_no < seg_count; seg_no++) {
                    if ((my_symbol->input_mode & 0x07) == DATA_MODE) {
                        Tcl_Size LengthTemp;
                        segs[seg_no].source = (unsigned char *) Tcl_GetByteArrayFromObj(pSegDataObjs[seg_no],
                            &LengthTemp);
                        segs[seg_no].length = (int)LengthTemp;
                    } else {
                        pStr = Tcl_GetStringFromObj(pSegDataObjs[seg_no], &lStr);
                        Tcl_DStringInit(& segInputs[seg_no]);
                        Tcl_UtfToExternalDString( hZINTEncoding, pStr, lStr, &segInputs[seg_no]);
                        segs[seg_no].source = (unsigned char *) Tcl_DStringValue( &segInputs[seg_no] );
                        segs[seg_no].length = (int)Tcl_DStringLength( &segInputs[seg_no] );
                    }
                }
            }
        }
    }
    /*------------------------------------------------------------------------*/
    /* >>> Build symbol graphic */
    if (! fError ) {
        int ErrorNumber;
        Tk_PhotoHandle hPhoto;
        /*--------------------------------------------------------------------*/
        /* call zint graphic creation to buffer */
        if (seg_count) {
            ErrorNumber = ZBarcode_Encode_Segs_and_Buffer(my_symbol,
                segs, seg_count, rotate_angle);
        } else {
            ErrorNumber = ZBarcode_Encode_and_Buffer(my_symbol,
                (unsigned char *) pStr, (int)lStr, rotate_angle);
        }
        /*--------------------------------------------------------------------*/
        /* >> Show a message */
        if( 0 != ErrorNumber )
        {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(my_symbol->errtxt, -1));
        }        
        if( ZINT_ERROR <= ErrorNumber )
        {
            /* >> Encode error */
            fError = 1;
        } else if (
            NULL == (hPhoto = Tk_FindPhoto(interp, Tcl_GetString(objv[3]))))
        {
            Tcl_SetObjResult(interp,
                Tcl_NewStringObj("Unknown photo image", -1));
            fError = 1;
        } else {
            Tk_PhotoImageBlock sImageBlock;
            char * pImageRGBA = NULL;
            if (my_symbol->alphamap == NULL) {
                sImageBlock.pixelPtr = (unsigned char *) my_symbol->bitmap;
                sImageBlock.width = my_symbol->bitmap_width;
                sImageBlock.height = my_symbol->bitmap_height;
                sImageBlock.pitch = 3*my_symbol->bitmap_width;
                sImageBlock.pixelSize = 3;
                sImageBlock.offset[0] = 0;
                sImageBlock.offset[1] = 1;
                sImageBlock.offset[2] = 2;
                sImageBlock.offset[3] = 0;
            } else {
                int index;
                /* Alpha channel present - prepare the image data in rgba order */
                pImageRGBA = ckalloc(my_symbol->bitmap_width*my_symbol->bitmap_height*4);
                for (index = 0; index < my_symbol->bitmap_width*my_symbol->bitmap_height; index++) {
                    pImageRGBA[index*4] = my_symbol->bitmap[index*3];
                    pImageRGBA[index*4+1] = my_symbol->bitmap[index*3+1];
                    pImageRGBA[index*4+2] = my_symbol->bitmap[index*3+2];
                    pImageRGBA[index*4+3] = my_symbol->alphamap[index];
                }
                sImageBlock.pixelPtr = (unsigned char *) pImageRGBA;
                sImageBlock.width = my_symbol->bitmap_width;
                sImageBlock.height = my_symbol->bitmap_height;
                sImageBlock.pitch = 4*my_symbol->bitmap_width;
                sImageBlock.pixelSize = 4;
                sImageBlock.offset[0] = 0;
                sImageBlock.offset[1] = 1;
                sImageBlock.offset[2] = 2;
                sImageBlock.offset[3] = 3;
            }
            if (0 == destWidth) {
                destWidth = my_symbol->bitmap_width;
            }
            if (0 == destHeight) {
                destHeight = my_symbol->bitmap_height;
            }
            if (TCL_OK != Tk_PhotoPutBlock(interp, hPhoto, &sImageBlock,
                destX0, destY0, destWidth, destHeight,
                TK_PHOTO_COMPOSITE_OVERLAY))
            {
                fError = 1;
            }
            if (pImageRGBA != NULL) {
                ckfree(pImageRGBA);
            }
        }
    }
    /*------------------------------------------------------------------------*/
    Tcl_FreeEncoding(hZINTEncoding);
    Tcl_DStringFree(& dsInput);
    ZBarcode_Delete(my_symbol);
    /*------------------------------------------------------------------------*/
    if (fError) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

/* vim: set ts=4 sw=4 et : */
