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
-	No changes here, take 2.5.1 framework files
 2017-05-12 2.6.0 HaO
-	No changes here, take 2.6 framework files
 2017-08-29 2.6.1 HaO
-	Framework 2.6.1 extensions
-	EAN/UPC Codes with included check digit
-	UPNQR Code
-	Misspelled symbology: AztecRunes
 2017-10-23 2.6.2 HaO
-	Framework 2.6.2 bugfixes
-   Allow dll unload
 2018-02-13 2.6.3 HaO
 -	Framework trunk update
 -	Added VIN and MailMark symbologies.
 2018-11-02 2.6.4 HaO
 -	Framework trunk update
 -	Add options -bold, -dotted, -dotsize, -dmre, -eci
 -	Implemented ECI logic
 2019-09-01 2.6.5 HaO
 -	Framework 2.6.5 update
 -	Add option -gssep
 2019-09-18 2.6.6 HaO
 -	Framework 2.6.6 update
 2019-10-07 2.6.7 HaO
 -	Framework 2.6.7 update
 2019-12-05 2.7.0 HaO
 -	Framework 2.7.0 update
 -  Add symbology rmqr
 2020-02-01 2.7.1 HaO
 -	Framework 2.7.1 update
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
/* > File option defines */

#define VERSION "2.7.1"

/*----------------------------------------------------------------------------*/
/* >>>>> Hepler defines */

#define STRING( x ) #x

/*----------------------------------------------------------------------------*/
/* >>>> External Prototypes (exports) */
EXPORT int Zint_Init (Tcl_Interp *interp);
EXPORT int Zint_Unload (Tcl_Interp *Interp, int Flags);
/*----------------------------------------------------------------------------*/
/* >>>> local prototypes */
static int Zint(ClientData unused, Tcl_Interp *interp, int objc,
    Tcl_Obj *CONST objv[]);
static int Encode(Tcl_Interp *interp, int objc,
    Tcl_Obj *CONST objv[]);
/*----------------------------------------------------------------------------*/
/* >>>> File Global Variables */

/* >> List of Codes */

static char *s_code_list[] = {
    "Code11",
    "Matrix2of5",
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
    "Postnet",
    "MSIPlessey",
    "FIM",
    "Logmars",
    "Pharma",
    "PZN",
    "PharmaTwo",
    "PDF417",
    "PDF417Truncated",
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
    "RSS14Stacked",
    "RSS14SstackedOmni",
    "RSSExpandedStacked",
    "Planet",
    "MicroPDF417",
    "OneCode",
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
    "EAN128-CC",
    "RSS14-CC",
    "RSSLimited-CC",
    "RSSEXPanded-CC",
    "UPCA-CC",
    "UPCE-CC",
    "RSS14Stacked-CC",
    "RSS14Omni-CC",
    "RSSExpandedStacked-CC",
    "Channel",
    "CodeOne",
    "GridMatrix",
    "UPNQR",
    "rMQR",
    NULL};

static int s_code_number[] = {
    BARCODE_CODE11,
    BARCODE_C25MATRIX,
    BARCODE_C25INTER,
    BARCODE_C25IATA,
    BARCODE_C25LOGIC,
    BARCODE_C25IND,
    BARCODE_CODE39,
    BARCODE_EXCODE39,
    BARCODE_EANX,
	BARCODE_EANX_CHK,
    BARCODE_EAN128,
    BARCODE_CODABAR,
    BARCODE_CODE128,
    BARCODE_DPLEIT,
    BARCODE_DPIDENT,
    BARCODE_CODE16K,
    BARCODE_CODE49,
    BARCODE_CODE93,
    BARCODE_FLAT,
    BARCODE_RSS14,
    BARCODE_RSS_LTD,
    BARCODE_RSS_EXP,
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
    BARCODE_PDF417TRUNC,
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
    BARCODE_RSS14STACK,
    BARCODE_RSS14STACK_OMNI,
    BARCODE_RSS_EXPSTACK,
    BARCODE_PLANET,
    BARCODE_MICROPDF417,
    BARCODE_ONECODE,
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
    BARCODE_EAN128_CC,
    BARCODE_RSS14_CC,
    BARCODE_RSS_LTD_CC,
    BARCODE_RSS_EXP_CC,
    BARCODE_UPCA_CC,
    BARCODE_UPCE_CC,
    BARCODE_RSS14STACK_CC,
    BARCODE_RSS14_OMNI_CC,
    BARCODE_RSS_EXPSTACK_CC,
    BARCODE_CHANNEL,
    BARCODE_CODEONE,
    BARCODE_GRIDMATRIX,
    BARCODE_UPNQR,
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
    "jis0208",      /*20: ** Shift-JIS (JISX 0208 amd JISX 0201)*/
    "cp1250",       /*21: Windows-1250*/
    "cp1251",       /*22: Windows-1251*/
    "cp1252",       /*23: Windows-1252*/
    "cp1256",       /*24: Windows-1256*/
    "unicode",      /*25: ** UCS-2 Unicode (High order byte first)*/
    "utf-8",        /*26: Unicode (UTF-8)*/
    "ascii",        /*27: ISO-646:1991 7-bit character set*/
    "big5",         /*28: ** Big-5 (Taiwan) Chinese Character Set*/
    "euc-cn",       /*29: ** GB (PRC) Chinese Character Set*/
    "iso2022-kr",   /*30: ** Korean Character Set (KSX1001:1998)*/
	NULL
};

/* The ECI numerical number to pass to ZINT */
static int s_eci_number[] = {
    3,4,5,6,7,8,9,10,11,12,13,15,16,17,18,20,21,22,23,24,25,26,27,28,29,30
};

/* Flag if an encoding is delivered as unicode. If not, native encoding is
 * required.
 * Those encodings are marked with "**" in the upper comments.
 */
static int s_eci_unicode_input[] = {
    /* 3,4,5,6,7,8,9,10,11,12,13,15,16,17,18,20,21,22,23,24,25,26,27,28,29,30 */
       1,1,1,1,1,1,1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0
};


/* Version information */
static char version_string[] = VERSION;
/* Help text */
static char help_message[] = "zint tcl(stub,obj) dll\n"
    " Generate barcode in tk images\n"
    "Usage:\n"
    " zint encode data photo option value...\n"
    "  data: data to encode in the symbol\n"
    "  photo: a tcl photo image handle ('p' after 'image create photo p')\n"
    "  Available options:\n"
    "   -barcode choice: symbology, use 'zint symbology' to get a list\n"
    "   -bind bool: bars above/below the code, size set by -border\n"
    "   -border integer: width of a border around the symbol. Use with -bind/-box 1\n"
    "   -box bool: box around bar code, size set be -border\n"
    "   -height integer: Symbol height in modules\n"
    "   -whitesp integer: horizontal quiet zone in modules\n"
    "   -fg color: set foreground color as 6 hex rrggbb\n"
    "   -bg color: set background color as 6 hex rrggbb\n"
    "   -cols integer: PDF417, Codablock F: number of columns\n"
    "   -rows integer: Codablock F: number of rows\n"
    "   -vers integer: Symbology option, QR-Code, Plessy\n"
    "   -dmre bool: Allow Data Matrix Rectangular Extended\n"
    "   -rotate angle: Image rotation by 0,90 or 270 degrees\n"
    "   -secure integer: EC Level (PDF417, QR)\n"
    "   -mode: Structured primary data mode (Maxicode, Composite)\n"
    "   -primary text: Structured primary data (Maxicode, Composite)\n"
    "   -dotty bool: use dots instead of boxes for matrix codes\n"
    "   -dotsize number: radius ratio of dots from 0.01 to 1.0\n" 
    "   -scale double: Scale the image to this factor\n"
    "   -format binary|unicode|gs1: input data format. Default:unicode\n"
	"   -gssep bool: for gs1, use gs as separator instead fnc1 (Datamatrix only)\n"
    "   -eci number: ECI to use\n"
    "   -notext bool: no interpretation line\n"
    "   -square bool: force Data Matrix symbols to be square\n"
    "   -init bool: Create reader initialisation symbol (Code 128, Data Matrix)\n"
    "   -smalltext bool: tiny interpretation line font\n"
    "   -bold bool: use bold text\n"
    "   -to {x0 y0 ?width? ?height?}: place to put in photo image\n"
    "\n"
    "zint symbologies: List available symbologies\n"
    "zint eci: List available eci tables\n"
    " zint help\n"
    " zint version\n"
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
    /*------------------------------------------------------------------------*/
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.1", 0) == NULL)
#else
    if (Tcl_PkgRequire(interp, "Tcl", "8.1", 0) == NULL)
#endif
    {
        return TCL_ERROR;
    }
    /*------------------------------------------------------------------------*/
#ifdef USE_TK_STUBS
    if (Tk_InitStubs(interp, "8.1", 0) == NULL)
#else
    if (Tcl_PkgRequire(interp, "Tk", "8.1", 0) == NULL)
#endif
    {
        return TCL_ERROR;
    }
    /*------------------------------------------------------------------------*/
    Tcl_CreateObjCommand(interp, "zint", Zint, (ClientData)NULL,
        (Tcl_CmdDeleteProc *)NULL);
    Tcl_PkgProvide (interp, "zint", version_string);
    /*------------------------------------------------------------------------*/
    return TCL_OK;
}
//------------------------------------------------------------------------------
// >>>> Unload Procedures
//------------------------------------------------------------------------------
EXPORT int Zint_Unload (Tcl_Interp *Interp, int Flags)
{
	// Allow unload
	return TCL_OK;
}
/*----------------------------------------------------------------------------*/
/* >>>>> Called routine */
/*----------------------------------------------------------------------------*/
/* Decode tcl commands */
static int Zint(ClientData unused, Tcl_Interp *interp, int objc,
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
/*----------------------------------------------------------------------------*/
/* >>>>> Encode */
/*----------------------------------------------------------------------------*/
/* Encode image */
static int Encode(Tcl_Interp *interp, int objc,
    Tcl_Obj *CONST objv[])
{
    struct zint_symbol *hSymbol;
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
    hSymbol = ZBarcode_Create();
    hSymbol->input_mode = UNICODE_MODE;
    hSymbol->option_3 = 0;
    /*------------------------------------------------------------------------*/
    /* >> Decode options */
    for (optionPos = 4; optionPos < objc; optionPos+=2) {
        /*--------------------------------------------------------------------*/
        /* Option list and indexes */
        char *optionList[] = {
            "-barcode", "-bg", "-bind", "-bold", "-border", "-box", "-cols",
            "-dmre", "-dotsize", "-dotty", "-eci", "-fg", "-format", "-gssep",
			"-height", "-init", "-mode", "-notext", "-primary", "-rotate",
			"-rows", "-scale", "-secure", "-smalltext", "-square", "-to",
			"-vers", "-whitesp", NULL};
        enum iOption {
            iBarcode, iBG, iBind, iBold, iBorder, iBox, iCols,
            iDMRE, iDotSize, iDotty, iECI, iFG, iFormat, iGSSep, iHeight,
            iInit, iMode, iNoText, iPrimary, iRotate, iRows,
            iScale, iSecure, iSmallText, iSquare, iTo, iVers,
            iWhiteSp
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
		case iGSSep:
        case iInit:
        case iNoText:
        case iSmallText:
        case iSquare:
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
            if (lStr != 6) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Color is not 6 hex",-1));
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
        case iBorder:
        case iCols:
        case iHeight:
        case iMode:
        case iRotate:
        case iRows:
        case iSecure:
        case iVers:
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
        case iBind:
            if (intValue) {
                hSymbol->output_options |= BARCODE_BIND;
            } else {
                hSymbol->output_options &= ~BARCODE_BIND;
            }
            break;
        case iBold:
            if (intValue) {
                hSymbol->output_options |= BOLD_TEXT;
            } else {
                hSymbol->output_options &= ~BOLD_TEXT;
            }
            break;
        case iBox:
            if (intValue) {
                hSymbol->output_options |= BARCODE_BOX;
            } else {
                hSymbol->output_options &= ~BARCODE_BOX;
            }
            break;
        case iDotSize:
            if (doubleValue < 0.01) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Dot size below 0.01", -1));
                fError = 1;
            } else {
                hSymbol->dot_size = (float)doubleValue;
            }
            break;
        case iDotty:
            if (intValue) {
                hSymbol->output_options |= BARCODE_DOTTY_MODE;
            } else {
                hSymbol->output_options &= ~BARCODE_DOTTY_MODE;
            }
            break;
        case iGSSep:
            if (intValue) {
                hSymbol->output_options |= GS1_GS_SEPARATOR;
            } else {
                hSymbol->output_options &= ~GS1_GS_SEPARATOR;
            }
            break;
        case iECI:
            if(Tcl_GetIndexFromObj(interp, objv[optionPos+1],
                (const char **) s_eci_list,"-eci", optionPos, &ECIIndex)
                == TCL_ERROR)
            {
                fError = 1;
            } else {
                hSymbol->eci = s_eci_number[ECIIndex];
            }
            break;
        case iInit:
            if (intValue) {
                hSymbol->output_options |= READER_INIT;
            } else {
                hSymbol->output_options &= ~READER_INIT;
            }
            break;
        case iSmallText:
            if (intValue) {
                hSymbol->output_options |= SMALL_TEXT;
            } else {
                hSymbol->output_options &= ~SMALL_TEXT;
            }
            break;
        case iFG:
            strncpy(hSymbol->fgcolour, pStr, 6);
            hSymbol->fgcolour[6]='\0';
            break;
        case iBG:
            strncpy(hSymbol->bgcolour, pStr, 6);
            hSymbol->bgcolour[6]='\0';
            break;
        case iNoText:
            hSymbol->show_hrt = (intValue?0:1);
            break;
        case iSquare:
            /* DM_SQUARE overwrites DM_DMRE */
            if (intValue)
                hSymbol->option_3 = DM_SQUARE;
            break;
        case iDMRE:
            /* DM_DMRE overwrites DM_SQUARE */
            if (intValue)
                hSymbol->option_3 = DM_DMRE;
            break;
        case iScale:
            if (doubleValue < 0.01) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Scale below 0.01", -1));
                fError = 1;
            } else {
                hSymbol->scale = (float)doubleValue;
            }
            break;
        case iBorder:
            if (intValue < 0 || intValue > 1000) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Border out of range", -1));
                fError = 1;
            } else {
                hSymbol->border_width = intValue;
            }
            break;
        case iHeight:
            if (intValue < 1 || intValue > 1000) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Height out of range", -1));
                fError = 1;
            } else {
                hSymbol->height = intValue;
            }
            break;
        case iCols:
        case iVers:
            /* >> Int in Option 2 */
            if (intValue < 1
                || (optionIndex==iCols && intValue > 66)
                || (optionIndex==iVers && intValue > 47))
            {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("cols/vers out of range", -1));
                fError = 1;
            } else {
                hSymbol->option_2 = intValue;
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
                hSymbol->option_1 = intValue;
            }
            break;
        case iPrimary:
            strcpy(hSymbol->primary, Tcl_DStringValue( &dString ) );
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
                hSymbol->symbology = s_code_number[intValue];
            }
            break;
        case iWhiteSp:
            hSymbol->whitespace_width = intValue;
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
                    case iBinary: hSymbol->input_mode = DATA_MODE; break;
                    case iGS1: hSymbol->input_mode = GS1_MODE; break;
                    default: hSymbol->input_mode = UNICODE_MODE; break;
                }
            }
        }
    }
    /*------------------------------------------------------------------------*/
    /* >>> Prepare input dstring and encode it to ECI encoding*/
    Tcl_DStringInit(& dsInput);
    /*------------------------------------------------------------------------*/
    if (!fError) {
        /*--------------------------------------------------------------------*/
        /* >>> Get input mode */
        if (hSymbol->input_mode == DATA_MODE) {
            /* Binary data */
            pStr = (char *) Tcl_GetByteArrayFromObj(objv[2], &lStr);
        } else {
            /* UTF8 Data (or ECI encoding) */
            if ( ! s_eci_unicode_input[ECIIndex]) {
                /* For this ECI, the Data must be encoded in the ECI encoding */
                Tcl_FreeEncoding(hZINTEncoding);
                hZINTEncoding = Tcl_GetEncoding(interp, s_eci_list[ECIIndex]);
                if (NULL == hZINTEncoding) {
                    /* Interpreter has error message */
                    fError = 1;
                }
                /* we must indicate binary data */
                hSymbol->input_mode = DATA_MODE;
            }
            if (! fError ) {
                pStr = Tcl_GetStringFromObj(objv[2], &lStr);
                Tcl_UtfToExternalDString( hZINTEncoding, pStr, lStr, &dsInput);
                pStr = Tcl_DStringValue( &dsInput );
                lStr = Tcl_DStringLength( &dsInput );
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
        ErrorNumber = ZBarcode_Encode_and_Buffer(hSymbol,
            (unsigned char *) pStr, lStr, rotate_angle);
        /*--------------------------------------------------------------------*/
        /* >> Show a message */
        if( 0 != ErrorNumber )
        {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(hSymbol->errtxt, -1));
        }        
        if( 5 <= ErrorNumber )
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
            sImageBlock.pixelPtr = (unsigned char *) hSymbol->bitmap;
            sImageBlock.width = hSymbol->bitmap_width;
            sImageBlock.height = hSymbol->bitmap_height;
            sImageBlock.pitch = 3*hSymbol->bitmap_width;
            sImageBlock.pixelSize = 3;
            sImageBlock.offset[0] = 0;
            sImageBlock.offset[1] = 1;
            sImageBlock.offset[2] = 2;
            sImageBlock.offset[3] = 0;
            if (0 == destWidth) {
                destWidth = hSymbol->bitmap_width;
            }
            if (0 == destHeight) {
                destHeight = hSymbol->bitmap_height;
            }
            if (TCL_OK != Tk_PhotoPutBlock(interp, hPhoto, &sImageBlock,
                destX0, destY0, destWidth, destHeight,
                TK_PHOTO_COMPOSITE_OVERLAY))
            {
                fError = 1;
            }
        }
    }
    /*------------------------------------------------------------------------*/
    Tcl_FreeEncoding(hZINTEncoding);
    Tcl_DStringFree(& dsInput);
    ZBarcode_Delete(hSymbol);
    /*------------------------------------------------------------------------*/
    if (fError) {
        return TCL_ERROR;
    }
    return TCL_OK;
}


