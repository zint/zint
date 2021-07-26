/* gif.c - Handles output to gif file */

/*
    libzint - the open source barcode library
    Copyright (C) 2009 - 2021 Robin Stuart <rstuart114@gmail.com>

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

#include <errno.h>
#include <stdio.h>
#include "common.h"
#include <math.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#include <malloc.h>
#endif

#define SSET    "0123456789ABCDEF"

typedef struct s_statestruct {
    unsigned char *pOut;
    unsigned char *pIn;
    unsigned int InLen;
    unsigned int OutLength;
    unsigned int OutPosCur;
    unsigned int OutByteCountPos;
    unsigned short ClearCode;
    unsigned short FreeCode;
    char fByteCountByteSet;
    unsigned char OutBitsFree;
    unsigned short NodeAxon[4096];
    unsigned short NodeNext[4096];
    unsigned char NodePix[4096];
    unsigned char colourCode[10];
    unsigned char colourPaletteIndex[10];
    int colourCount;
} statestruct;

/* Transform a Pixel to a lzw colourmap index and move to next pixel.
 * All colour values are listed in colourCode with corresponding palette index
 */
static unsigned char NextPaletteIndex(statestruct *pState)
{
    unsigned char pixelColour;
    int colourIndex;
    pixelColour = *(pState->pIn);
    (pState->pIn)++;
    (pState->InLen)--;
    for (colourIndex = 0; colourIndex < pState->colourCount; colourIndex++) {
        if (pixelColour == pState->colourCode[colourIndex])
            return pState->colourPaletteIndex[colourIndex];

    }
    return 0; /* Not reached */
}


static int BufferNextByte(statestruct *pState) {
    (pState->OutPosCur)++;
    /* Check if this position is a byte count position
     * fg_f_bytecountbyte_set indicates, if byte count position bytes should be
     * inserted in general.
     * If this is true, and the distance to the last byte count position is 256
     * (e.g. 255 bytes in between), a byte count byte is inserted, and the value
     * of the last one is set to 255.
     * */
    if (pState->fByteCountByteSet && (pState->OutByteCountPos + 256 == pState->OutPosCur)) {
        (pState->pOut)[pState->OutByteCountPos] = 255;
        pState->OutByteCountPos = pState->OutPosCur;
        (pState->OutPosCur)++;
    }
    if (pState->OutPosCur >= pState->OutLength)
        return 1;

    (pState->pOut)[pState->OutPosCur] = 0x00;
    return 0;
}

static int AddCodeToBuffer(statestruct *pState, unsigned short CodeIn, unsigned char CodeBits) {
    /* Check, if we may fill up the current byte completely */
    if (CodeBits >= pState->OutBitsFree) {
        (pState->pOut)[pState->OutPosCur] |= (unsigned char) (CodeIn << (8 - pState->OutBitsFree));
        if (BufferNextByte(pState))
            return -1;
        CodeIn = (unsigned short) (CodeIn >> pState->OutBitsFree);
        CodeBits -= pState->OutBitsFree;
        pState->OutBitsFree = 8;
        /* Write a full byte if there are at least 8 code bits left */
        if (CodeBits >= pState->OutBitsFree) {
            (pState->pOut)[pState->OutPosCur] = (unsigned char) CodeIn;
            if (BufferNextByte(pState))
                return -1;
            CodeIn = (unsigned short) (CodeIn >> 8);
            CodeBits -= 8;
        }
    }
    /* The remaining bits of CodeIn fit in the current byte. */
    if (CodeBits > 0) {
        (pState->pOut)[pState->OutPosCur] |= (unsigned char) (CodeIn << (8 - pState->OutBitsFree));
        pState->OutBitsFree -= CodeBits;
    }
    return 0;
}

static void FlushStringTable(statestruct *pState) {
    unsigned short Pos;
    for (Pos = 0; Pos < pState->ClearCode; Pos++) {
        (pState->NodeAxon)[Pos] = 0;
    }
}

static unsigned short FindPixelOutlet(statestruct *pState, unsigned short HeadNode, unsigned char Byte) {
    unsigned short Outlet;

    Outlet = (pState->NodeAxon)[HeadNode];
    while (Outlet) {
        if ((pState->NodePix)[Outlet] == Byte)
            return Outlet;
        Outlet = (pState->NodeNext)[Outlet];
    }
    return 0;
}

static int NextCode(statestruct *pState, unsigned char *pPixelValueCur, unsigned char CodeBits) {
    unsigned short UpNode;
    unsigned short DownNode;
    /* start with the root node for last pixel chain */
    UpNode = *pPixelValueCur;
    if ((pState->InLen) == 0)
        return AddCodeToBuffer(pState, UpNode, CodeBits);

    *pPixelValueCur = NextPaletteIndex(pState);
    /* Follow the string table and the data stream to the end of the longest string that has a code */
    while (0 != (DownNode = FindPixelOutlet(pState, UpNode, *pPixelValueCur))) {
        UpNode = DownNode;
        if ((pState->InLen) == 0)
            return AddCodeToBuffer(pState, UpNode, CodeBits);

        *pPixelValueCur = NextPaletteIndex(pState);
    }
    /* Submit 'UpNode' which is the code of the longest string */
    if (AddCodeToBuffer(pState, UpNode, CodeBits))
        return -1;
    /* ... and extend the string by appending 'PixelValueCur' */
    /* Create a successor node for 'PixelValueCur' whose code is 'freecode' */
    (pState->NodePix)[pState->FreeCode] = *pPixelValueCur;
    (pState->NodeAxon)[pState->FreeCode] = (pState->NodeNext)[pState->FreeCode] = 0;
    /* ...and link it to the end of the chain emanating from fg_axon[UpNode]. */
    DownNode = (pState->NodeAxon)[UpNode];
    if (!DownNode) {
        (pState->NodeAxon)[UpNode] = pState->FreeCode;
    } else {
        while ((pState->NodeNext)[DownNode]) {
            DownNode = (pState->NodeNext)[DownNode];
        }
        (pState->NodeNext)[DownNode] = pState->FreeCode;
    }
    return 1;
}

static int gif_lzw(statestruct *pState, int paletteBitSize) {
    unsigned char PixelValueCur;
    unsigned char CodeBits;
    unsigned short Pos;

    // > Get first data byte
    if (pState->InLen == 0)
        return 0;
    PixelValueCur = NextPaletteIndex(pState);
    /* Number of bits per data item (=pixel)
     * We need at least a value of 2, otherwise the cc and eoi code consumes
     * the whole string table
     */
    if (paletteBitSize == 1)
        paletteBitSize = 2;

    /* initial size of compression codes */
    CodeBits = paletteBitSize + 1;
    pState->ClearCode = (1 << paletteBitSize);
    pState->FreeCode = pState->ClearCode + 2;
    pState->OutBitsFree = 8;
    pState->OutPosCur = -1;
    pState->fByteCountByteSet = 0;

    if (BufferNextByte(pState))
        return 0;

    for (Pos = 0; Pos < pState->ClearCode; Pos++)
        (pState->NodePix)[Pos] = (unsigned char) Pos;

    FlushStringTable(pState);

    /* Write what the GIF specification calls the "code size". */
    (pState->pOut)[pState->OutPosCur] = paletteBitSize;
    /* Reserve first bytecount byte */
    if (BufferNextByte(pState))
        return 0;
    pState->OutByteCountPos = pState->OutPosCur;
    if (BufferNextByte(pState))
        return 0;
    pState->fByteCountByteSet = 1;
    /* Submit one 'ClearCode' as the first code */
    if (AddCodeToBuffer(pState, pState->ClearCode, CodeBits))
        return 0;

    for (;;) {
        int Res;
        /* generate and save the next code, which may consist of multiple input pixels. */
        Res = NextCode(pState, &PixelValueCur, CodeBits);
        if (Res < 0)
            return 0;
        //* Check for end of data stream */
        if (!Res) {
            /* submit 'eoi' as the last item of the code stream */
            if (AddCodeToBuffer(pState, (unsigned short) (pState->ClearCode + 1), CodeBits))
                return 0;
            pState->fByteCountByteSet = 0;
            if (pState->OutBitsFree < 8) {
                if (BufferNextByte(pState))
                    return 0;
            }
            // > Update last bytecount byte;
            if (pState->OutByteCountPos < pState->OutPosCur) {
                (pState->pOut)[pState->OutByteCountPos]
                    = (unsigned char) (pState->OutPosCur - pState->OutByteCountPos - 1);
            }
            pState->OutPosCur++;
            return pState->OutPosCur;
        }
        /* Check for currently last code */
        if (pState->FreeCode == (1U << CodeBits))
            CodeBits++;
        pState->FreeCode++;
        /* Check for full stringtable */
        if (pState->FreeCode == 0xfff) {
            FlushStringTable(pState);
            if (AddCodeToBuffer(pState, pState->ClearCode, CodeBits))
                return 0;

            CodeBits = (unsigned char) (1 + paletteBitSize);
            pState->FreeCode = (unsigned short) (pState->ClearCode + 2);
        }
    }
}

/*
 * Called function to save in gif format
 */
INTERNAL int gif_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf) {
    unsigned char outbuf[10];
    FILE *gif_file;
    unsigned short usTemp;
    int byte_out;
    int colourCount;
    unsigned char paletteRGB[10][3];
    int paletteCount, paletteCountCur, paletteIndex;
    unsigned int pixelIndex;
    int paletteBitSize;
    int paletteSize;
    statestruct State;
    int transparent_index;
    int bgindex = -1, fgindex = -1;
    const int output_to_stdout = symbol->output_options & BARCODE_STDOUT;

    unsigned char backgroundColourIndex;
    unsigned char RGBCur[3];
    unsigned char RGBUnused[3] = {0,0,0};

    int colourIndex;

    int fFound;

    unsigned char pixelColour;
    unsigned int bitmapSize = symbol->bitmap_height * symbol->bitmap_width;

    /* Allow for overhead of 4 == code size + byte count + overflow byte + zero terminator */
    unsigned int lzoutbufSize = bitmapSize + 4;
#ifdef _MSC_VER
    char *lzwoutbuf;
#endif

#ifndef _MSC_VER
    char lzwoutbuf[lzoutbufSize];
#else
    lzwoutbuf = (char *) _alloca(lzoutbufSize);
#endif /* _MSC_VER */

    /*
     * Build a table of the used palette items.
     * Currently, there are the following 10 colour codes:
     * '0': standard background
     * '1': standard foreground
     * 'W': white
     * 'C': cyan
     * 'B': blue
     * 'M': magenta
     * 'R': red
     * 'Y': yellow
     * 'G': green
     * 'K': black
     * '0' and '1' may be identical to one of the other values
     *
     * A data structure is set up as follows:
     * state.colourCode: list of colour codes
     * paletteIndex: palette index of the corresponding colour code
     *  There are colourCount entries in the upper lists.
     * paletteRGB: RGB value at the palette position
     *  There are paletteCount entries.
     *  This value is smaller to colourCount, if multiple colour codes have the
     *  same RGB value and point to the same palette value.
     * Example:
     *  0 1 W K are present. 0 is equal to white, while 1 is blue
     *  The resulting tables are:
     *  paletteItem: ['0']=0 (white), ['1']=1 (blue), ['W']=0 (white),
     *               ['K']=2 (black)
     *  Thus, there are 4 colour codes and 3 palette entries.

     */
    colourCount = 0;
    paletteCount = 0;
    /* loop over all pixels */
    for (pixelIndex = 0; pixelIndex < bitmapSize; pixelIndex++) {
        fFound = 0;
        /* get pixel colour code */
        pixelColour = pixelbuf[pixelIndex];
        /* look, if colour code is already in colour list */
        for (colourIndex = 0; colourIndex < colourCount; colourIndex++) {
            if ((State.colourCode)[colourIndex] == pixelColour) {
                fFound = 1;
                break;
            }
        }
        /* If colour is already present, go to next colour code */
        if (fFound)
            continue;

        /* Colour code not present - add colour code */
        /* Get RGB value */
        switch (pixelColour) {
            case '0': /* standard background */
                RGBCur[0] = (unsigned char) (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
                RGBCur[1] = (unsigned char) (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
                RGBCur[2] = (unsigned char) (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);
                break;
            case '1': /* standard foreground */
                RGBCur[0] = (unsigned char) (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
                RGBCur[1] = (unsigned char) (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
                RGBCur[2] = (unsigned char) (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
                break;
            case 'W': /* white */
                RGBCur[0] = 255; RGBCur[1] = 255; RGBCur[2] = 255;
                break;
            case 'C': /* cyan */
                RGBCur[0] = 0; RGBCur[1] = 255; RGBCur[2] = 255;
                break;
            case 'B': /* blue */
                RGBCur[0] = 0; RGBCur[1] = 0; RGBCur[2] = 255;
                break;
            case 'M': /* magenta */
                RGBCur[0] = 255; RGBCur[1] = 0; RGBCur[2] = 255;
                break;
            case 'R': /* red */
                RGBCur[0] = 255; RGBCur[1] = 0; RGBCur[2] = 0;
                break;
            case 'Y': /* yellow */
                RGBCur[0] = 255; RGBCur[1] = 255; RGBCur[2] = 0;
                break;
            case 'G': /* green */
                RGBCur[0] = 0; RGBCur[1] = 255; RGBCur[2] = 0;
                break;
            case 'K': /* black */
                RGBCur[0] = 0; RGBCur[1] = 0; RGBCur[2] = 0;
                break;
            default: /* error case - return  */
                strcpy(symbol->errtxt, "612: unknown pixel colour");
                return ZINT_ERROR_INVALID_DATA;
        }
        /* Search, if RGB value is already present */
        fFound = 0;
        for (paletteIndex = 0; paletteIndex < paletteCount; paletteIndex++) {
            if (RGBCur[0] == paletteRGB[paletteIndex][0]
                && RGBCur[1] == paletteRGB[paletteIndex][1]
                && RGBCur[2] == paletteRGB[paletteIndex][2])
            {
                fFound = 1;
                break;
            }
        }
        /* RGB not present, add it */
        if (!fFound) {
            paletteIndex = paletteCount;
            paletteRGB[paletteIndex][0] = RGBCur[0];
            paletteRGB[paletteIndex][1] = RGBCur[1];

            paletteRGB[paletteIndex][2] = RGBCur[2];

            paletteCount++;

            if (pixelColour == '0') bgindex = paletteIndex;
            if (pixelColour == '1') fgindex = paletteIndex;
        }
        /* Add palette index to current colour code */
        (State.colourCode)[colourCount] = pixelColour;
        (State.colourPaletteIndex)[colourCount] = paletteIndex;
        colourCount++;
    }
    State.colourCount = colourCount;

    /* Set transparency */
    /* Note: does not allow both transparent foreground and background -
     * background takes priority */
    transparent_index = -1;
    if (strlen(symbol->fgcolour) > 6) {
        if ((symbol->fgcolour[6] == '0') && (symbol->fgcolour[7] == '0')) {
            // Transparent foreground
            transparent_index = fgindex;
        }
    }
    if (strlen(symbol->bgcolour) > 6) {
        if ((symbol->bgcolour[6] == '0') && (symbol->bgcolour[7] == '0')) {
            // Transparent background
            transparent_index = bgindex;
        }
    }

    /* find palette bit size from palette size*/

    /* 1,2 -> 1, 3,4 ->2, 5,6,7,8->3 */
    paletteBitSize = 0;
    paletteCountCur = paletteCount - 1;
    while (paletteCountCur != 0) {
        paletteBitSize++;
        paletteCountCur >>= 1;
    }
    /* Minimum is 1 */
    if (paletteBitSize == 0)
        paletteBitSize = 1;

    /* palette size 2 ^ bit size */
    paletteSize = 1 << paletteBitSize;

    /* Open output file in binary mode */
    if (output_to_stdout) {
#ifdef _MSC_VER
        if (-1 == _setmode(_fileno(stdout), _O_BINARY)) {
            sprintf(symbol->errtxt, "610: Could not set stdout to binary (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_ACCESS;
        }
#endif
        gif_file = stdout;
    } else {
        if (!(gif_file = fopen(symbol->outfile, "wb"))) {
            sprintf(symbol->errtxt, "611: Could not open output file (%d: %.30s)", errno, strerror(errno));
            return ZINT_ERROR_FILE_ACCESS;
        }
    }

    /* GIF signature (6) */
    memcpy(outbuf, "GIF87a", 6);
    if (transparent_index != -1)
        outbuf[4] = '9';

    fwrite(outbuf, 6, 1, gif_file);
    /* Screen Descriptor (7) */
    /* Screen Width */
    usTemp = (unsigned short) symbol->bitmap_width;
    outbuf[0] = (unsigned char) (0xff & usTemp);
    outbuf[1] = (unsigned char) ((0xff00 & usTemp) / 0x100);
    /* Screen Height */
    usTemp = (unsigned short) symbol->bitmap_height;
    outbuf[2] = (unsigned char) (0xff & usTemp);
    outbuf[3] = (unsigned char) ((0xff00 & usTemp) / 0x100);
    /* write ImageBits-1 to the three least significant bits of byte 5  of
     * the Screen Descriptor
     * Bits 76543210
     *      1        : Global colour map
     *       111     : 8 bit colour depth of the palette
     *          0    : Not ordered in decreasing importance
     *           xxx : palette bit zize - 1
     */
    outbuf[4] = (unsigned char) (0xf0 | (0x7 & (paletteBitSize - 1)));

    /*
     * Background colour index
     * Default to 0. If colour code 0 or K is present, it is used as index
     */

    backgroundColourIndex = 0;
    for (colourIndex = 0; colourIndex < colourCount; colourIndex++) {
        if ((State.colourCode)[colourIndex] == '0' || (State.colourCode)[colourIndex] == 'W') {
            backgroundColourIndex = (State.colourPaletteIndex)[colourIndex];
            break;
        }
    }
    outbuf[5] = backgroundColourIndex;
    /* Byte 7 must be 0x00  */
    outbuf[6] = 0x00;
    fwrite(outbuf, 7, 1, gif_file);
    /* Global Color Table (paletteSize*3) */
    fwrite(paletteRGB, 3*paletteCount, 1, gif_file);
    /* add unused palette items to fill palette size */
    for (paletteIndex = paletteCount; paletteIndex < paletteSize; paletteIndex++) {
        fwrite(RGBUnused, 3, 1, gif_file);
    }

    /* Graphic control extension (8) */
    /* A graphic control extension block is used for overlay gifs.
     * This is necessary to define a transparent color.
     */
    if (transparent_index != -1) {
        /* Extension Introducer = '!' */
        outbuf[0] = '\x21';
        /* Graphic Control Label */
        outbuf[1] = '\xf9';
        /* Block Size */
        outbuf[2] = 4;
        /* Packet fields:
         * 3 Reserved
         * 3 Disposal Method: 0 No Action, 1 No Dispose, 2: Background, 3: Prev.
         * 1 User Input Flag: 0: no user input, 1: user input
         * 1 Transparent Color Flag: 0: No Transparency, 1: Transparency index
         */
        outbuf[3] = 1;
        /* Delay Time */
        outbuf[4] = 0;
        outbuf[5] = 0;
        /* Transparent Color Index */
        outbuf[6] = (unsigned char) transparent_index;
        /* Block Terminator */
        outbuf[7] = 0;
        fwrite(outbuf, 8, 1, gif_file);
    }
    /* Image Descriptor */
    /* Image separator character = ',' */
    outbuf[0] = 0x2c;
    /* "Image Left" */
    outbuf[1] = 0x00;
    outbuf[2] = 0x00;
    /* "Image Top" */
    outbuf[3] = 0x00;
    outbuf[4] = 0x00;
    /* Image Width (low byte first) */
    outbuf[5] = (unsigned char) (0xff & symbol->bitmap_width);
    outbuf[6] = (unsigned char) ((0xff00 & symbol->bitmap_width) / 0x100);
    /* Image Height */
    outbuf[7] = (unsigned char) (0xff & symbol->bitmap_height);
    outbuf[8] = (unsigned char) ((0xff00 & symbol->bitmap_height) / 0x100);

    /* Byte 10 contains the interlaced flag and
     * information on the local color table.
     * There is no local color table if its most significant bit is reset.
     */
    outbuf[9] = 0x00;
    fwrite(outbuf, 10, 1, gif_file);

    /* prepare state array */
    State.pIn = pixelbuf;
    State.InLen = bitmapSize;
    State.pOut = (unsigned char *) lzwoutbuf;
    State.OutLength = lzoutbufSize;

    /* call lzw encoding */
    byte_out = gif_lzw(&State, paletteBitSize);
    if (byte_out <= 0) {
        if (!output_to_stdout) {
            fclose(gif_file);
        }
        strcpy(symbol->errtxt, "613: Insufficient memory for LZW buffer");
        return ZINT_ERROR_MEMORY;
    }
    fwrite(lzwoutbuf, byte_out, 1, gif_file);

    /* GIF terminator */
    fputc('\x3b', gif_file);
    if (output_to_stdout) {
        fflush(gif_file);
    } else {
        fclose(gif_file);
    }

    return 0;
}
