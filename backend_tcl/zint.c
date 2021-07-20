/* zint_tcl.c TCL binding for zint */
/*
    zint - the open source tcl binding to the zint barcode library
    Copyright (C) 2014 Harald Oehlmann <oehhar@users.sourceforge.net>

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
/* vim: set ts=4 sw=4 et : */
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
#include <zintconfig.h>
#include <string.h>

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#define USE_TCL_STUBS
#define USE_TK_STUBS
#endif

#include <tcl.h>
#include <tk.h>

#undef EXPORT
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif


/*----------------------------------------------------------------------------*/
/* >>>>> Hepler defines */

/* Two macros are necessary to not include the define name, but the value */
#define STRING(x) #x
#define TOSTRING(x) STRING(x)

/* Define VERSION as the first 3 digits of the zint library version number */
#define VERSION TOSTRING( ZINT_VERSION_MAJOR ) \
        "." TOSTRING( ZINT_VERSION_MINOR ) \
        "." TOSTRING( ZINT_VERSION_RELEASE )

/*----------------------------------------------------------------------------*/
/* >>>> External Prototypes (exports) */
EXPORT int Zint_Init (Tcl_Interp *interp);
EXPORT int Zint_Unload (Tcl_Interp *Interp, int Flags);
/*----------------------------------------------------------------------------*/
/* >>>> local prototypes */
static void InterpCleanupProc(ClientData clientData, Tcl_Interp *interp);
static int CheckForTk(Tcl_Interp *interp, int *tkFlagPtr);
static int Zint(ClientData unused, Tcl_Interp *interp, int objc,
    Tcl_Obj *CONST objv[]);
static int Encode(Tcl_Interp *interp, int objc,
    Tcl_Obj *CONST objv[]);
/*----------------------------------------------------------------------------*/
/* >>>> File Global Variables */

/* >> List of Codes */

static char *s_code_list[] = {
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
    "PDF417",
    "PDF417Compact",
    "MaxiCode",
    "QR",
    "Code128B",
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
    "MailMark",
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
    NULL};

static int s_code_number[] = {
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
    BARCODE_PDF417,
    BARCODE_PDF417COMP,
    BARCODE_MAXICODE,
    BARCODE_QRCODE,
    BARCODE_CODE128B,
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
    BARCODE_MAILMARK,
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
    0};

/* ECI TCL encoding names.
 * The ECI comments are given after the name.
 * A ** indicates encodings where native data must be delivered and not utf-8
 */
static char *s_eci_list[] = {
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
    "unicode",      /*25: UCS-2BE (High order byte first) Unicode BMP*/
    "utf-8",        /*26: Unicode (UTF-8)*/
    "ascii",        /*27: ISO-646:1991 7-bit character set*/
    "big5",         /*28: Big5 (Taiwan) Chinese Character Set*/
    "euc-cn",       /*29: GB (PRC) Chinese Character Set*/
    "iso2022-kr",   /*30: Korean Character Set EUC-KR (KS X 1001:2002)*/
    NULL
};

/* The ECI numerical number to pass to ZINT */
static int s_eci_number[] = {
    3,4,5,6,7,8,9,10,11,12,13,15,16,17,18,20,21,22,23,24,25,26,27,28,29,30
};


/* Version information */
static char version_string[] = VERSION;
/* Help text */
static char help_message[] = "zint tcl(stub,obj) dll\n"
    " Generate barcode in tk images\n"
    "Usage:\n"
    " zint encode data photo ?option value? ...\n"
    "  data: data to encode in the symbol\n"
    "  photo: a tcl photo image handle ('p' after 'image create photo p')\n"
    "  Available options:\n"
    "   -barcode choice: symbology, use 'zint symbology' to get a list\n"
    "   -addongap number: (7..12, default: 9) set add-on gap in multiple of module size (UPC/EAN-CC)\n"
    "   -bg color: set background color as 6 or 8 hex rrggbbaa\n"
    /* cli option --binary internally handled */
    "   -bind bool: bars above/below the code, size set by -border\n"
    "   -bold bool: use bold text\n"
    "   -border integer: width of a border around the symbol. Use with -bind/-box 1\n"
    "   -box bool: box around bar code, size set be -border\n"
    /* cli option --cmyk not supported as no corresponding output */
    "   -cols integer: PDF417, Codablock F, DotCode: number of columns\n"
    /* cli option --data is standard parameter */
    "   -dmre bool: Allow Data Matrix Rectangular Extended\n"
    "   -dotsize number: radius ratio of dots from 0.01 to 1.0\n" 
    "   -dotty bool: use dots instead of boxes for matrix codes\n"
    /* cli option --dump not supported */
    /* cli option --ecinos not supported */
    "   -eci number: ECI to use\n"
    /* cli option --esc not supported */
    "   -fg color: set foreground color as 6 or 8 hex rrggbbaa\n"
    /* replaces cli options --binary and --gs1 */
    "   -format binary|unicode|gs1: input data format. Default:unicode\n"
    "   -fullmultibyte bool: allow multibyte compaction for xQR, HanXin, Gridmatrix\n"
    /* cli option --gs1 replaced by -format */
    "   -gs1nocheck bool: for gs1, do not check validity of data (allows non-standard symbols)\n"
    "   -gs1parens bool: for gs1, AIs enclosed in parentheses instead of square brackets\n"
    "   -gssep bool: for gs1, use gs as separator instead fnc1 (Datamatrix only)\n"
    "   -height double: Symbol height in modules\n"
    /* cli option --input not supported */
    "   -init bool: Create reader initialisation symbol (Code 128, Data Matrix)\n"
    "   -mask number: set masking pattern to use (QR/MicroQR/HanXin/DotCode)\n"
    /* cli option --mirror not supported */
    "   -mode number: set encoding mode (MaxiCode, Composite)\n"
    "   -nobackground bool: set background transparent\n"
    "   -notext bool: no interpretation line\n"
    /* cli option --output not supported */
    "   -primary text: Structured primary data (MaxiCode, Composite)\n"
    "   -reverse bool: Reverse colours (white on black)\n"
    "   -rotate angle: Image rotation by 0,90 or 270 degrees\n"
    "   -rows integer: Codablock F: number of rows\n"
    "   -scale double: Scale the image to this factor\n"
    "   -scmvv number: Prefix SCM with [)>\\R01\\Gvv (vv is NUMBER) (MaxiCode)\n"
    "   -secure integer: EC Level (PDF417, QR)\n"
    "   -separator 0..4 (default: 1) : Stacked symbologies: separator width\n"
    /* cli option --small replaced by -smalltext */
    "   -smalltext bool: tiny interpretation line font\n"
    "   -square bool: force Data Matrix symbols to be square\n"
    /* cli option --types not supported */
    "   -vers integer: Symbology option\n"
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
EXPORT int Zint_Init (Tcl_Interp *interp)
{
    int * tkFlagPtr;
    /*------------------------------------------------------------------------*/
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.5", 0) == NULL)
#else
    if (Tcl_PkgRequire(interp, "Tcl", "8.5", 0) == NULL)
#endif
    {
        return TCL_ERROR;
    }
    /*------------------------------------------------------------------------*/
    /* This procedure is called once per thread and any thread local data     */
    /* should be allocated and initialized here (and not in static variables) */
    
    /* Create a flag if Tk is loaded */
    tkFlagPtr = (int *)ckalloc(sizeof(int));
    *tkFlagPtr = 0;
    Tcl_CallWhenDeleted(interp, InterpCleanupProc, (ClientData)tkFlagPtr);
    /*------------------------------------------------------------------------*/
    Tcl_CreateObjCommand(interp, "zint", Zint, (ClientData)tkFlagPtr,
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
EXPORT int Zint_Unload (Tcl_Interp *Interp, int Flags)
{
    // Allow unload
    return TCL_OK;
}
/*----------------------------------------------------------------------------*/
/* >>>>> Called routine */
/*----------------------------------------------------------------------------*/
/* Decode tcl commands */
static int Zint(ClientData tkFlagPtr, Tcl_Interp *interp, int objc,
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
        if ( ! Tcl_PkgPresent(interp, "Tk", "8.5", 0) ) {
            Tcl_SetResult(interp, "package Tk not loaded", TCL_STATIC);
            return TCL_ERROR;
        }
    }
#ifdef USE_TK_STUBS
    if (*tkFlagPtr < 0 || Tk_InitStubs(interp, "8.5", 0) == NULL) {
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
    int lStr;
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
    unsigned int cap;
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
        char *optionList[] = {
            "-addongap", "-barcode", "-bg", "-bind", "-bold", "-border", "-box",
            "-cols", "-dmre", "-dotsize", "-dotty", "-eci", "-fg", "-format",
            "-fullmultibyte", "-gs1nocheck", "-gs1parens", "-gssep", "-height",
            "-init", "-mask", "-mode",
            "-nobackground", "-notext", "-primary", "-reverse", "-rotate",
            "-rows", "-scale", "-scmvv", "-secure", "-separator", "-smalltext",
            "-square", "-to", "-vers", "-vwhitesp", "-werror", "-whitesp",
            NULL};
        enum iOption {
            iAddonGap, iBarcode, iBG, iBind, iBold, iBorder, iBox,
            iCols, iDMRE, iDotSize, iDotty, iECI, iFG, iFormat,
            iFullMultiByte, iGS1NoCheck, iGS1Parens, iGSSep, iHeight,
            iInit, iMask, iMode,
            iNoBackground, iNoText, iPrimary, iReverse, iRotate,
            iRows, iScale, iSCMvv, iSecure, iSeparator, iSmallText,
            iSquare, iTo, iVers, iVWhiteSp, iWError, iWhiteSp
            };
        int optionIndex;
        int intValue;
        double doubleValue;
        /*--------------------------------------------------------------------*/
        if(Tcl_GetIndexFromObj(interp, objv[optionPos],
            (const char **) optionList,
            "zint option", optionPos-1, &optionIndex)
            == TCL_ERROR)
        {
            fError = 1;
            break;
        }
        /*--------------------------------------------------------------------*/
        /* >> Decode object */
        switch (optionIndex) {
        case iBind:
        case iBold:
        case iBox:
        case iDMRE:
        case iDotty:
        case iGS1NoCheck:
        case iGS1Parens:
        case iGSSep:
        case iInit:
        case iNoBackground:
        case iNoText:
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
        case iDotSize:
        case iScale:
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
        case iHeight:
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
                Tcl_SetObjResult(interp,Tcl_NewStringObj("String to long", -1));
                fError = 1;
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
                (const char **) s_eci_list,"-eci", optionPos, &ECIIndex)
                == TCL_ERROR)
            {
                fError = 1;
            } else {
                my_symbol->eci = s_eci_number[ECIIndex];
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
        case iNoText:
            my_symbol->show_hrt = (intValue?0:1);
            break;
        case iSquare:
            /* DM_SQUARE overwrites DM_DMRE */
            if (intValue)
                my_symbol->option_3 = DM_SQUARE;
            break;
        case iDMRE:
            /* DM_DMRE overwrites DM_SQUARE */
            if (intValue)
                my_symbol->option_3 = DM_DMRE;
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
        case iBorder:
            if (intValue < 0 || intValue > 1000) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Border out of range", -1));
                fError = 1;
            } else {
                my_symbol->border_width = intValue;
            }
            break;
        case iHeight:
            if ((float)doubleValue < 0.5f || (float)doubleValue > 1000.0f) {
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
                || (optionIndex==iVers && intValue > 47))
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
            /* >> Int in Option 1 */
            if ( (optionIndex==iSecure && (intValue < 1 || intValue > 8))
                || (optionIndex==iMode && (intValue < 0 || intValue > 6))
                || (optionIndex==iRows && (intValue < 0 || intValue > 44)))
            {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("secure/mode/rows out of range", -1));
                fError = 1;
            } else {
                my_symbol->option_1 = intValue;
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
                    "rotate", optionPos, &intValue)
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
                (const char **) s_code_list,"-barcode", optionPos, &intValue)
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
                } else if ((
                    TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                        0, &poParam)
                    || TCL_OK != Tcl_GetIntFromObj(interp,poParam,&destX0)
                    || TCL_OK != Tcl_ListObjIndex(interp, objv[optionPos+1],
                        1, &poParam)
                    || TCL_OK != Tcl_GetIntFromObj(interp,poParam,&destY0)
                    || lStr == 4) && (
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
                    "format", optionPos, &intValue)
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
    }
    /*------------------------------------------------------------------------*/
    /* >>> Build symbol graphic */
    if (! fError ) {
        int ErrorNumber;
        Tk_PhotoHandle hPhoto;
        /*--------------------------------------------------------------------*/
        /* call zint graphic creation to buffer */
        ErrorNumber = ZBarcode_Encode_and_Buffer(my_symbol,
            (unsigned char *) pStr, lStr, rotate_angle);
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
