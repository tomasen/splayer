/*************************************************************************
 *
 *  LibVP62 Project. See http://libvp62.sourgeforge.net for details.
 *  Copyright (C) 2006   Zeitoun Padli
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 ************************************************************************/

#include <stdlib.h>
#include <string.h>

#define byte unsigned char

class VP62 {
private:
    byte   *inputBuffer;	// Points to payload buffer

    // Frame characteristics
    int	    frameType;
    int	    tagFrame;
    int	    displayRows;
    int	    displayColumns;
    int	    movieWidth;
    int	    movieHeight;
    int	    lastRows;
    int	    lastCols;

    // Interlacing
    int	    interlacedBlock;
    bool    useInterlacing;
    int	    ilProb;

    int	    quantizer;
    int	    lastQuantizer;
    short   coeffScale[64];

    // Arithmetic Coding (AC)
    int	    acHigh;
    unsigned long acCodeWord;
    int	    acBits;
    byte   *acBuffer;
    byte   *acBufferEnd;

    // Buffers
    byte   *yuvCurrentFrame;
    byte   *yuvTaggedFrame;
    byte   *yuvLastFrame;

    int	    yStride;	// actual stride + 96
    int	    uvStride;   // actual stride + 48
    int	    ySize;	// Luma Y
    int	    uvSize;	// Chroma U or V
    int	    mbc;	// Number of horizontal MB
    int	    mbr;	// Number of vertical MB
    int	    orgPlaneStride[6];
    int	    planeStride[6];
    int	    pixelOffset[6]; // Offset to upper left corner pixel for each blocks

    // DC Predictors management
    struct AB {
	char  notNullDC;
	char  refFrame;
	short dcCoeff;
    } *aboveBlocks;	// Above blocks
    int aboveBlockIndex[6];

    struct AB prevBlock[4]; // Left blocks
    short   prevDCRefFrame[3][3];

    // Blocks / Macroblock
    int	    currentMbType;
    int	    prevMbType;
    short   block8x8[6][64];
    byte    block12x12[144];   // Intermediate 12x12 block for filtering
    int	    coeffIndexToPos[64];
    byte    coeffReorder[64];
    short   coeff420[6][64];  // DCT Coeff for each blocks
    struct MB {
	byte type;	// MB type
	short vx,vy;	// Motion vector
    } *macroblocks;

    // Vectors (Motion compensation)
    short   blockVector[6][2];	// Vectors for each block in MB
    short   prevFrameFirstVectorCandidate[2];
    short   prevFrameSecondVectorCandidate[2];
    int	    prevFrameFirstVectorCandidatePos;
    short   tagFrameFirstVectorCandidate[2];
    short   tagFrameSecondVectorCandidate[2];
    int	    tagFrameFirstVectorCandidatePos;

    // Predictors candidates
    int	    predOffset[12];

    // Filtering hints for moved blocks
    int	    blockCopyFiltering;
    int	    blockCopyFilterMode;
    int	    maxVectorLength;
    int	    sampleVarianceThreshold;
    int	    filterSelection;

    // AC Models
    byte    sigVectorModel[2];		    // Delta sign
    byte    dctVectorModel[2];		    // Delta Coding Types
    byte    pdvVectorModel[2][7];	    // Predefined Delta Values
    byte    fdvVectorModel[2][8];	    // 8 bit delta value definition
    byte    sameMbTypeModel[3][10];	    // Same as previous MB type
    byte    nextMbTypeModel[3][10][9];	    // Next MB type
    byte    dccvCoeffModel[2][11];	    // DC Coeff value
    byte    ractCoeffModel[2][3][6][11];    // Run/AC coding type and AC coeff value
    byte    dcctCoeffModel[2][3][5];	    // DC coeff coding type
    byte    runvCoeffModel[2][14];	    // Run value
    byte    mbTypesStats[3][2][10];	    // Contextual, next MbType statistics

    //////// METHODS
    void    decodeFrame();
    int	    parseHeader();
    void    allocateBuffers();
    void    initOffscreenBorders(byte *yuv);
    void    initCoeffScales();

    // Arithmetic coding
    void    acInit(byte *);
    int	    acGetBit(int prob);
    int	    acGetBit();
    int	    acGetBits(int bits);

    // Block blitters
    void    drawBlock(int block);
    void    drawDeltaBlockFromYuv(byte *yuv, int block);
    void    drawDeltaBlockFromMB(short *srcBlock, short *delta, int b);

    // Models
    void    defaultModelsInit();
    void    parseVectorModelsChanges();
    void    parseVectorAdjustment(short *v, int mbType);
    void    parseCoeffModelsChanges();
    void    parseMacroblockTypeModelsChanges();
    void    initCoeffOrderTable();

    // Parsing/Decoding
    void    decodeMacroBlock(int row, int col);
    int	    parseMacroblockType(int prevType, int index);
    void    parseCoeff();
    void    addPredictorsDC();
    void    decodeMbTypeAndVectors(int row, int col);
    void    decode4Vectors(int row, int col);

    // iDCT
    void    iDCT8x8(int block);

    // Motion compensation
    void    getBlock(int block, short *dstBlock, byte *yuv);
    int	    getVectorsPredictors(int row, int col, int refFrame);

    // Copy/filter
    void    fill12x12Block(byte *yuv, int pixOffset, int vx, int vy, int block);
    void    simpleBlockCopy(short *dstBlock, byte *yuv, int offset, int stride);
    void    fourPointFilterHV(short *dstBlock, byte *yuv, int offset, int stride, int delta, int *filter);
    void    fourPointFilterDiag(short *dstBlock, byte *yuv, int offset, int stride, int *hFilter, int *vFilter);
    void    aaFilterHV(short *dstBlock, byte *yuv, int offset, int stride, int delta, int *aa);
    void    aaFilterDiag(short *dstBlock, byte *yuv, int offset, int stride, int *hAA, int *vAA);
    void    filteredBlockCopy(short *dstBlock, byte *yuv, int iOffset, int oOffset, int stride, int x8, int y8, bool useEnhancedFilter, int select);
    void    edgeFilter(int n, int pixInc, int lineInc, int t);

public:
    VP62();	// Constructor
    ~VP62();	// Destructor

    int	    decodePacket(byte *payload, int length);
    void    getImageSize(int *width, int *height);
    void    getDisplaySize(int *width, int *height);   // Maybe different than image
    void    getRGB(byte *rgb);	// Returns RGB32 image
	void	getYUV(byte** yuv, int* pitch);
};
