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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "VP62.h"

#define FRAME_INTRA 0
#define MB_SIZE	16

#define REF_FRAME_CURRENT  0
#define REF_FRAME_PREVIOUS 1
#define REF_FRAME_TAGGED   2

#define MB_INTER_NOVEC_PF   0	// Inter MB, no vector, from previous frame
#define MB_INTRA	    1	// Intra MB
#define MB_INTER_DELTA_PF   2	// Inter MB, above/left vector + delta, from previous Frame
#define MB_INTER_V1_PF	    3	// Inter MB, first vector, from previous Frame
#define MB_INTER_V2_PF	    4	// Inter MB, second vector, from previous Frame
#define MB_INTER_NOVEC_TF   5	// Inter MB, no vector, from tagged frame
#define MB_INTER_DELTA_TF   6	// Inter MB, above/left vector + delta, from tagged Frame
#define MB_INTER_4V	    7	// Inter MB, 4 vectors, from previous Frame
#define MB_INTER_V1_TF	    8	// Inter MB, first vector, from tagged Frame
#define MB_INTER_V2_TF	    9	// Inter MB, second vector, from tagged Frame

static int referenceFrame[10] =	{
	    REF_FRAME_PREVIOUS,	// Mb type 0
	    REF_FRAME_CURRENT,	// Mb type 1
	    REF_FRAME_PREVIOUS,	// Mb type 2
	    REF_FRAME_PREVIOUS,	// Mb type 3
	    REF_FRAME_PREVIOUS,	// Mb type 4
	    REF_FRAME_TAGGED,	// Mb type 5
	    REF_FRAME_TAGGED,	// Mb type 6
	    REF_FRAME_PREVIOUS,	// Mb type 7
	    REF_FRAME_TAGGED,	// Mb type 8
	    REF_FRAME_TAGGED	// Mb type 9
	};

static int b6to4[6] = 	{ 0, 0, 1, 1, 2, 3  };
static int b6to3[6] = 	{ 0, 0, 0, 0, 1, 2  };

// For fast clipping and abs value computation
static byte clip0to255[1024];	// Indexes from -512 to +512, biased by 512
static byte valAbs[1024];   // Ditto

#define CLIP(v) (clip0to255[(v)+512])
#define ABS(v) (valAbs[(v)+512])

// Constructor
VP62::VP62()
{
    inputBuffer = NULL;

    frameType = FRAME_INTRA;
    tagFrame = 0;
    movieWidth = 0;
    movieHeight = 0;
    useInterlacing = 0;

    quantizer = 0;
    lastQuantizer = -1;

    currentMbType = 0;
    prevFrameFirstVectorCandidatePos = 0;
    tagFrameFirstVectorCandidatePos = 0;
    interlacedBlock = 0;

    acHigh = 255;
    acBits = 0;
    acCodeWord = 0;
    acBuffer = NULL;
    acBufferEnd = NULL;

    yStride = 0;
    uvStride = 0;
    ySize = 0;
    uvSize = 0;
    mbr = 0;
    mbc = 0;

    prevMbType = MB_INTER_NOVEC_PF;
    lastRows = 0;
    lastCols = 0;

    blockCopyFiltering = 0;
    blockCopyFilterMode = 0;
    maxVectorLength = 0;
    sampleVarianceThreshold = 0;
    filterSelection = 0;
    ilProb = 0;

    // Buffers
    macroblocks = NULL;
    aboveBlocks = NULL;
    yuvCurrentFrame = NULL;
    yuvTaggedFrame = NULL;
    yuvLastFrame = NULL;

    // Optimisation tables
    int i;

    for(i = 0; i < 1024; i++) {
	int v = i - 512;
	clip0to255[i] = (byte)(v >= 0 ? v <= 255 ? v : 255 : 0);
	valAbs[i] = (byte)(v >= 0 ? v : -v);
    }
}

// Desctructor
VP62::~VP62()
{
    free(yuvCurrentFrame);
    free(yuvTaggedFrame);
    free(yuvLastFrame);
    free(macroblocks);
    free(aboveBlocks);
}

// Decode a payload
int VP62::decodePacket(byte *buffer, int length)
{
    int res;

    inputBuffer = buffer;
    acBufferEnd = buffer+length;

    res = parseHeader();

    switch(res) {
	case 0:	// Error
	    return -1;
	case 1:
	    break;
	case 2: // Image size has changed
	    allocateBuffers();
	    break;
    }

    initCoeffScales();

    decodeFrame();

    return 0;
}

// Parse the header and check for image resizing
int VP62::parseHeader()
{
    int res = 1;
    int parseFilterInfo;
    byte header1;

    header1 = inputBuffer[0];

    frameType = (header1 >> 7) & 1;
    quantizer = (header1 >> 1) & 0x3f;

    if (header1 & 1) {
	return 0;   // Unexpected header
    }

    if (frameType == FRAME_INTRA) {
	byte header2;

	header2 = inputBuffer[1];

	if ((header2 & 0xfe) != 0x46) {
	    return 0;	// Unexpected header
	}

	useInterlacing = (header2 & 1) != 0;

	int imgRows = inputBuffer[2] * 2;
	int imgCols = inputBuffer[3] * 2;

	// Aspect ratio info
	displayRows    = inputBuffer[4] * 2;
	displayColumns = inputBuffer[5] * 2;

	acInit(inputBuffer + 6);

	acGetBits(2);	// Dummy

	if (imgRows != lastRows || imgCols != lastCols) {
	    // Size has changed
	    lastRows = imgRows;
	    lastCols = imgCols;
	    movieWidth = imgCols * 8;
	    movieHeight = imgRows * 8;
	    res = 2;	// Reallocate buffers!
	}

	parseFilterInfo = 1;
    } else {
	acInit(inputBuffer + 1);

	tagFrame = acGetBit();

	blockCopyFiltering = acGetBit();
	if (blockCopyFiltering) {
	    blockCopyFiltering = (blockCopyFiltering << 1) | acGetBit();
	}

	parseFilterInfo = acGetBit();
    }

    if (parseFilterInfo) {
	if (acGetBit()) {
	    blockCopyFilterMode = 2;
	    sampleVarianceThreshold = acGetBits(5);
	    maxVectorLength = (1 << (acGetBits(3) + 1));
	} else if (acGetBit()) {
	    blockCopyFilterMode = 1;
	} else {
	    blockCopyFilterMode = 0;
	}

	filterSelection = acGetBits(4);
    }

    acGetBit();	// Dummy

    return res;
}

// From so called quantizer value, initialize DC and A coeffs scale factors
void VP62::initCoeffScales()
{
    if (quantizer != lastQuantizer) {
	static byte acCoeffScale[64] = 	{ 94, 92, 90, 88, 86, 82, 78, 74,
					  70, 66, 62, 58, 54, 53, 52, 51,
					  50, 49, 48, 47, 46, 45, 44, 43,
					  42, 40, 39, 37, 36, 35, 34, 33,
					  32, 31, 30, 29, 28, 27, 26, 25,
					  24, 23, 22, 21, 20, 19, 18, 17,
					  16, 15, 14, 13, 12, 11, 10,  9,
					   8,  7,  6,  5,  4,  3,  2,  1  };
	static byte dcCoeffScale[64] = 	{ 47, 47, 47, 47, 45, 43, 43, 43,
					  43, 43, 42, 41, 41, 40, 40, 40,
					  40, 35, 35, 35, 35, 33, 33, 33,
					  33, 32, 32, 32, 27, 27, 26, 26,
					  25, 25, 24, 24, 23, 23, 19, 19,
					  19, 19, 18, 18, 17, 16, 16, 16,
					  16, 16, 15, 11, 11, 11, 10, 10,
					   9,  8,  7,  5,  3,  3,  2,  2  };

	int q;
	short scale;

	lastQuantizer = quantizer;
	coeffScale[0] = (short)(dcCoeffScale[quantizer] * 4);

	scale = (short)(acCoeffScale[quantizer] * 4);

	for(q = 1; q < 64; q++) {
	    coeffScale[q] = scale;
	}
    }
}

// Parse and decode a frame
void VP62::decodeFrame()
{
    int mbRow, mbCol;
    int rowMax = mbr - 3;
    int colMax = mbc - 3;
    int block;

    if (frameType == FRAME_INTRA) {
	// Reset models
	defaultModelsInit();

	// Reset MB types
	int mb;

	for(mb = 0; mb < mbc*mbr; mb++) {
	    macroblocks[mb].type = MB_INTRA;
	}
    } else {
	// Parse inter frame specific tables changes
	parseMacroblockTypeModelsChanges();
	parseVectorModelsChanges();
	prevMbType = 0;
    }

    parseCoeffModelsChanges();

    if (useInterlacing) {
	ilProb = acGetBits(8);
    }

    // Reset all refFrame DC values
    memset(prevDCRefFrame, 0, sizeof(prevDCRefFrame));
    // For refFrame as Intra, chroma blocks DC are biased by 128
    prevDCRefFrame[1][0] = 128;	// U
    prevDCRefFrame[2][0] = 128;	// V

    // Above block value memory init
    for(block = 0; block < mbc * 4; block++) {
	aboveBlocks[block].notNullDC = 0;
	aboveBlocks[block].refFrame = -1;    // Impossible value
	aboveBlocks[block].dcCoeff = 0;
    }

    // Main macroblocks loop
    for(mbRow = 3; mbRow < rowMax; mbRow++) {
	// Left block value memory init
	for(block = 0; block < 4; block++) {
	    prevBlock[block].notNullDC = 0;
	    prevBlock[block].refFrame = -1; // No reference
	    prevBlock[block].dcCoeff = 0;
	}

	// Those indirectly points to above line corresponding block
	aboveBlockIndex[0] = 0;
	aboveBlockIndex[1] = 1;
	aboveBlockIndex[2] = 0;
	aboveBlockIndex[3] = 1;
	aboveBlockIndex[4] = 2;
	aboveBlockIndex[5] = 3;

	pixelOffset[0] = (mbRow * yStride) * 16 + 48;		// Upper left block
	pixelOffset[1] = pixelOffset[0] + 8;			// Upper right block
	pixelOffset[2] = pixelOffset[0] + (yStride * 8);	// Lower left block
	pixelOffset[3] = pixelOffset[2] + 8;			// Lower right block
	pixelOffset[4] = ySize + (mbRow * uvStride * 8) + 24;	// Offset in U plane
	pixelOffset[5] = pixelOffset[4] + uvSize;		// Offset in V plane

	for(mbCol = 3; mbCol < colMax; mbCol++) {
	    int y, uv;

	    decodeMacroBlock(mbRow, mbCol);

	    // Update pixel offsets and above line block mem index
	    for(y = 0; y < 4; y++) {
		aboveBlockIndex[y] += 4;
		pixelOffset[y] += 16;
	    }

	    for(uv = 4; uv < 6; uv++) {
		aboveBlockIndex[uv] += 4;
		pixelOffset[uv] += 8;
	    }
	}
    }

    // Swap last and current frames
    byte *tmp = yuvLastFrame;
    yuvLastFrame = yuvCurrentFrame;
    yuvCurrentFrame = tmp;

    // Fill offscreen borders
    initOffscreenBorders(yuvLastFrame);

    // If intra or tagged, fill tagFrame buffer
    if (frameType == FRAME_INTRA || tagFrame) {
	memcpy(yuvTaggedFrame, yuvLastFrame, ySize + 2 * uvSize);
    }
}

void VP62::initCoeffOrderTable()
{
    int idx = 1;
    int g;
    int pos;

    coeffIndexToPos[0] = 0;
    for(g = 0; g < 16; g++) {
	for(pos = 1; pos < 64; pos++) {
	    if (coeffReorder[pos] == g) {
		coeffIndexToPos[idx] = pos;
		idx++;
	    }
	}
    }
}

// Load models with default values
void VP62::defaultModelsInit()
{
    dctVectorModel[0] = 162;
    dctVectorModel[1] = 164;
    sigVectorModel[0] = 128;
    sigVectorModel[1] = 128;

    static byte defMbTypesStats[3][2][10] = { {
						{ 69,  1, 1, 44,  6, 1, 0, 1, 0, 0 },
						{ 42,  2, 7, 42, 22, 3, 2, 5, 1, 0 }
					      },
					      {
						{ 229, 1, 0,  0,  0, 1, 0, 0, 1, 0 },
						{ 8,   1, 8,  0,  0, 2, 1, 0, 1, 0 }
					      },
					      {
						{ 122, 1, 1, 46,  0, 1, 0, 0, 1, 0 },
						{ 35,  1, 6, 34,  0, 2, 1, 1, 1, 0 }
					      }
					    };
    memcpy(mbTypesStats, defMbTypesStats, sizeof(mbTypesStats));

    static byte defFdvVectorModel[2][8] =  { { 247, 210, 135, 68, 138, 220, 239, 246  },
					     { 244, 184, 201, 44, 173, 221, 239, 253  } };
    memcpy(fdvVectorModel, defFdvVectorModel, sizeof(fdvVectorModel));

    static byte defPdvVectorModel[2][7] = { { 225, 146, 172, 147, 214, 39, 156  },
			    { 204, 170, 119, 235, 140, 230, 228  } };
    memcpy(pdvVectorModel, defPdvVectorModel, sizeof(pdvVectorModel));

    if (useInterlacing) {
	static byte ilCoeffReorder[64] =    { 0, 1, 0, 1, 1, 2, 5, 3,
					      2, 2, 2, 2, 4, 7, 8, 10,
					      9, 7, 5, 4, 2, 3, 5, 6,
					      8, 9, 11, 12, 13, 12, 11, 10,
					      9, 7, 5, 4, 6, 7, 9, 11,
					      12, 12, 13, 13, 14, 12, 11, 9,
					      7, 9, 11, 12, 14, 14, 14, 15,
					      13, 11, 13, 15, 15, 15, 15, 15  };
	memcpy(coeffReorder, ilCoeffReorder, sizeof(coeffReorder));
    } else {
	static byte defCoeffReorder[64] =    { 0, 0, 1, 1, 1, 2, 2, 2,
					       2, 2, 2, 3, 3, 4, 4, 4,
					       5, 5, 5, 5, 6, 6, 7, 7,
					       7, 7, 7, 8, 8, 9, 9, 9,
					       9, 9, 9, 10, 10, 11, 11, 11,
					       11, 11, 11, 12, 12, 12, 12, 12,
					       12, 13, 13, 13, 13, 13, 14, 14,
					       14, 14, 15, 15, 15, 15, 15, 15  };
	memcpy(coeffReorder, defCoeffReorder, sizeof(coeffReorder));
    }

    initCoeffOrderTable();

    static byte defRunvCoeffModel[2][14] = { {198, 197, 196, 146, 198, 204, 169, 142, 130, 136, 149, 149, 191, 249},
			      {135, 201, 181, 154,  98, 117, 132, 126, 146, 169, 184, 240, 246, 254}  };
    memcpy(runvCoeffModel, defRunvCoeffModel, sizeof(runvCoeffModel));
}

static byte preDefMbTypeStats[3][16][10][2] = {
    {
	{ {   9, 15 }, {  32, 25 }, {  7,  19 }, {   9, 21 }, {  1, 12 }, { 14, 12 }, { 3, 18 }, { 14, 23 }, {  3, 10 }, { 0,  4 } },
	{ {  48, 39 }, {   1,  2 }, { 11,  27 }, {  29, 44 }, {  7, 27 }, {  1,  4 }, { 0,  3 }, {  1,  6 }, {  1,  2 }, { 0,  0 } },
	{ {  21, 32 }, {   1,  2 }, {  4,  10 }, {  32, 43 }, {  6, 23 }, {  2,  3 }, { 1, 19 }, {  1,  6 }, { 12, 21 }, { 0,  7 } },
	{ {  69, 83 }, {   0,  0 }, {  0,   2 }, {  10, 29 }, {  3, 12 }, {  0,  1 }, { 0,  3 }, {  0,  3 }, {  2,  2 }, { 0,  0 } },
	{ {  11, 20 }, {   1,  4 }, { 18,  36 }, {  43, 48 }, { 13, 35 }, {  0,  2 }, { 0,  5 }, {  3, 12 }, {  1,  2 }, { 0,  0 } },
	{ {  70, 44 }, {   0,  1 }, {  2,  10 }, {  37, 46 }, {  8, 26 }, {  0,  2 }, { 0,  2 }, {  0,  2 }, {  0,  1 }, { 0,  0 } },
	{ {   8, 15 }, {   0,  1 }, {  8,  21 }, {  74, 53 }, { 22, 42 }, {  0,  1 }, { 0,  2 }, {  0,  3 }, {  1,  2 }, { 0,  0 } },
	{ { 141, 42 }, {   0,  0 }, {  1,   4 }, {  11, 24 }, {  1, 11 }, {  0,  1 }, { 0,  1 }, {  0,  2 }, {  0,  0 }, { 0,  0 } },
	{ {   8, 19 }, {   4, 10 }, { 24,  45 }, {  21, 37 }, {  9, 29 }, {  0,  3 }, { 1,  7 }, { 11, 25 }, {  0,  2 }, { 0,  1 } },
	{ {  46, 42 }, {   0,  1 }, {  2,  10 }, {  54, 51 }, { 10, 30 }, {  0,  2 }, { 0,  2 }, {  0,  1 }, {  0,  1 }, { 0,  0 } },
	{ {  28, 32 }, {   0,  0 }, {  3,  10 }, {  75, 51 }, { 14, 33 }, {  0,  1 }, { 0,  2 }, {  0,  1 }, {  1,  2 }, { 0,  0 } },
	{ { 100, 46 }, {   0,  1 }, {  3,   9 }, {  21, 37 }, {  5, 20 }, {  0,  1 }, { 0,  2 }, {  1,  2 }, {  0,  1 }, { 0,  0 } },
	{ {  27, 29 }, {   0,  1 }, {  9,  25 }, {  53, 51 }, { 12, 34 }, {  0,  1 }, { 0,  3 }, {  1,  5 }, {  0,  2 }, { 0,  0 } },
	{ {  80, 38 }, {   0,  0 }, {  1,   4 }, {  69, 33 }, {  5, 16 }, {  0,  1 }, { 0,  1 }, {  0,  0 }, {  0,  1 }, { 0,  0 } },
	{ {  16, 20 }, {   0,  0 }, {  2,   8 }, { 104, 49 }, { 15, 33 }, {  0,  1 }, { 0,  1 }, {  0,  1 }, {  1,  1 }, { 0,  0 } },
	{ { 194, 16 }, {   0,  0 }, {  1,   1 }, {   1,  9 }, {  1,  3 }, {  0,  0 }, { 0,  1 }, {  0,  1 }, {  0,  0 }, { 0,  0 } }
    },
    {
	{ {  41, 22 }, {   1,  0 }, {  1,  31 }, {   0,  0 }, {  0,  0 }, {  0,  1 }, { 1,  7 }, {  0,  1 }, { 98, 25 }, { 4, 10 } },
	{ { 123, 37 }, {   6,  4 }, {  1,  27 }, {   0,  0 }, {  0,  0 }, {  5,  8 }, { 1,  7 }, {  0,  1 }, { 12, 10 }, { 0,  2 } },
	{ {  26, 14 }, {  14, 12 }, {  0,  24 }, {   0,  0 }, {  0,  0 }, { 55, 17 }, { 1,  9 }, {  0, 36 }, {  5,  7 }, { 1,  3 } },
	{ { 209,  5 }, {   0,  0 }, {  0,  27 }, {   0,  0 }, {  0,  0 }, {  0,  1 }, { 0,  1 }, {  0,  1 }, {  0,  0 }, { 0,  0 } },
	{ {   2,  5 }, {   4,  5 }, {  0, 121 }, {   0,  0 }, {  0,  0 }, {  0,  3 }, { 2,  4 }, {  1,  4 }, {  2,  2 }, { 0,  1 } },
	{ { 175,  5 }, {   0,  1 }, {  0,  48 }, {   0,  0 }, {  0,  0 }, {  0,  2 }, { 0,  1 }, {  0,  2 }, {  0,  1 }, { 0,  0 } },
	{ {  83,  5 }, {   2,  3 }, {  0, 102 }, {   0,  0 }, {  0,  0 }, {  1,  3 }, { 0,  2 }, {  0,  1 }, {  0,  0 }, { 0,  0 } },
	{ { 233,  6 }, {   0,  0 }, {  0,   8 }, {   0,  0 }, {  0,  0 }, {  0,  1 }, { 0,  1 }, {  0,  0 }, {  0,  1 }, { 0,  0 } },
	{ {  34, 16 }, { 112, 21 }, {  1,  28 }, {   0,  0 }, {  0,  0 }, {  6,  8 }, { 1,  7 }, {  0,  3 }, {  2,  5 }, { 0,  2 } },
	{ { 159, 35 }, {   2,  2 }, {  0,  25 }, {   0,  0 }, {  0,  0 }, {  3,  6 }, { 0,  5 }, {  0,  1 }, {  4,  4 }, { 0,  1 } },
	{ {  75, 39 }, {   5,  7 }, {  2,  48 }, {   0,  0 }, {  0,  0 }, {  3, 11 }, { 2, 16 }, {  1,  4 }, {  7, 10 }, { 0,  2 } },
	{ { 212, 21 }, {   0,  1 }, {  0,   9 }, {   0,  0 }, {  0,  0 }, {  1,  2 }, { 0,  2 }, {  0,  0 }, {  2,  2 }, { 0,  0 } },
	{ {   4,  2 }, {   0,  0 }, {  0, 172 }, {   0,  0 }, {  0,  0 }, {  0,  1 }, { 0,  2 }, {  0,  0 }, {  2,  0 }, { 0,  0 } },
	{ { 187, 22 }, {   1,  1 }, {  0,  17 }, {   0,  0 }, {  0,  0 }, {  3,  6 }, { 0,  4 }, {  0,  1 }, {  4,  4 }, { 0,  1 } },
	{ { 133,  6 }, {   1,  2 }, {  1,  70 }, {   0,  0 }, {  0,  0 }, {  0,  2 }, { 0,  4 }, {  0,  3 }, {  1,  1 }, { 0,  0 } },
	{ { 251,  1 }, {   0,  0 }, {  0,   2 }, {   0,  0 }, {  0,  0 }, {  0,  0 }, { 0,  0 }, {  0,  0 }, {  0,  0 }, { 0,  0 } }
    },
    {
	{ {   2,  3 }, {   2,  3 }, {  0,   2 }, {   0,  2 }, {  0,  0 }, { 11,  4 }, { 1,  4 }, {  0,  2 }, {  3,  2 }, { 0,  4 } },
	{ {  49, 46 }, {   3,  4 }, {  7,  31 }, {  42, 41 }, {  0,  0 }, {  2,  6 }, { 1,  7 }, {  1,  4 }, {  2,  4 }, { 0,  1 } },
	{ {  26, 25 }, {   1,  1 }, {  2,  10 }, {  67, 39 }, {  0,  0 }, {  1,  1 }, { 0, 14 }, {  0,  2 }, { 31, 26 }, { 1,  6 } },
	{ { 103, 46 }, {   1,  2 }, {  2,  10 }, {  33, 42 }, {  0,  0 }, {  1,  4 }, { 0,  3 }, {  0,  1 }, {  1,  3 }, { 0,  0 } },
	{ {  14, 31 }, {   9, 13 }, { 14,  54 }, {  22, 29 }, {  0,  0 }, {  2,  6 }, { 4, 18 }, {  6, 13 }, {  1,  5 }, { 0,  1 } },
	{ {  85, 39 }, {   0,  0 }, {  1,   9 }, {  69, 40 }, {  0,  0 }, {  0,  1 }, { 0,  3 }, {  0,  1 }, {  2,  3 }, { 0,  0 } },
	{ {  31, 28 }, {   0,  0 }, {  3,  14 }, { 130, 34 }, {  0,  0 }, {  0,  1 }, { 0,  3 }, {  0,  1 }, {  3,  3 }, { 0,  1 } },
	{ { 171, 25 }, {   0,  0 }, {  1,   5 }, {  25, 21 }, {  0,  0 }, {  0,  1 }, { 0,  1 }, {  0,  0 }, {  0,  0 }, { 0,  0 } },
	{ {  17, 21 }, {  68, 29 }, {  6,  15 }, {  13, 22 }, {  0,  0 }, {  6, 12 }, { 3, 14 }, {  4, 10 }, {  1,  7 }, { 0,  3 } },
	{ {  51, 39 }, {   0,  1 }, {  2,  12 }, {  91, 44 }, {  0,  0 }, {  0,  2 }, { 0,  3 }, {  0,  1 }, {  2,  3 }, { 0,  1 } },
	{ {  81, 25 }, {   0,  0 }, {  2,   9 }, { 106, 26 }, {  0,  0 }, {  0,  1 }, { 0,  1 }, {  0,  1 }, {  1,  1 }, { 0,  0 } },
	{ { 140, 37 }, {   0,  1 }, {  1,   8 }, {  24, 33 }, {  0,  0 }, {  1,  2 }, { 0,  2 }, {  0,  1 }, {  1,  2 }, { 0,  0 } },
	{ {  14, 23 }, {   1,  3 }, { 11,  53 }, {  90, 31 }, {  0,  0 }, {  0,  3 }, { 1,  5 }, {  2,  6 }, {  1,  2 }, { 0,  0 } },
	{ { 123, 29 }, {   0,  0 }, {  1,   7 }, {  57, 30 }, {  0,  0 }, {  0,  1 }, { 0,  1 }, {  0,  1 }, {  0,  1 }, { 0,  0 } },
	{ {  13, 14 }, {   0,  0 }, {  4,  20 }, { 175, 20 }, {  0,  0 }, {  0,  1 }, { 0,  1 }, {  0,  1 }, {  1,  1 }, { 0,  0 } },
	{ { 202, 23 }, {   0,  0 }, {  1,   3 }, {   2,  9 }, {  0,  0 }, {  0,  1 }, { 0,  1 }, {  0,  1 }, {  0,  0 }, { 0,  0 } }
    }
}; 

void VP62::parseMacroblockTypeModelsChanges()
{
    int ctx;
    int type;
    int i;

    for(ctx = 0; ctx < 3; ctx++) {
	if (acGetBit(174)) {
	    int idx = acGetBits(4);
	    for(type = 0; type < 10; type++) {
		mbTypesStats[ctx][0][type] = preDefMbTypeStats[ctx][idx][type][0];    // Prev type identity weight
		mbTypesStats[ctx][1][type] = preDefMbTypeStats[ctx][idx][type][1];    // Type weight
	    }
	}
	if (acGetBit(254)) {
	    for(type = 0; type < 10; type++) {
		for(i=0; i<2; i++) {
		    if (acGetBit(205)) {
			int delta;
			int neg;

			delta = 0;
			
			neg = acGetBit();   // Sign

			if (acGetBit(171)) {
			    if (acGetBit(199)) {
				delta = 4 * acGetBits(7);
			    } if (acGetBit(140)) {
				delta = 12;
			    } else if (acGetBit(125)) {
				delta = 16;
			    } else if (acGetBit(104)) {
				delta = 20;
			    } else {
				delta = 24;
			    }
			} else if (acGetBit(83)) {
			    delta = 4;
			} else {
			    delta = 8;
			}

			if (neg) delta = -delta;

			mbTypesStats[ctx][i][type] += delta;
			mbTypesStats[ctx][i][type] = CLIP(mbTypesStats[ctx][i][type]);
		    }
		}
	    }
	}
    }

    // Compute MBType probability tables based on previous MBType
    for(ctx = 0; ctx < 3; ctx++) {	// Predictors context
	int p[10];

	// Scale weights by 100
	for(type = 0; type < 10; type++) {
	    p[type] = 100 * mbTypesStats[ctx][1][type];
	}

	for(type = 0; type < 10; type++) {  // Previous MB type

	    // Conservative MBType probability
	    sameMbTypeModel[ctx][type] = 255 - (255 * mbTypesStats[ctx][0][type]) / (1 + mbTypesStats[ctx][0][type] + mbTypesStats[ctx][1][type]);

	    // For same type, make weight null
	    p[type] = 0;

	    // Binary tree parsing probabilities (see VP62::parseMacroblockType)
	    int p02, p34, p0234;
	    int p17, p56, p89, p5689, p175689;

	    p02 = p[0] + p[2];
	    p34 = p[3] + p[4];
	    p0234 = p02 + p34;
	    p17 = p[1] + p[7];
	    p56 = p[5] + p[6];
	    p89 = p[8] + p[9];
	    p5689 = p56 + p89;
	    p175689 = p17 + p5689;

	    nextMbTypeModel[ctx][type][0] = 1 + 255 * p0234 / (1 + p0234 + p175689);
	    nextMbTypeModel[ctx][type][1] = 1 + 255 * p02 / (1 + p0234);
	    nextMbTypeModel[ctx][type][2] = 1 + 255 * p17 / (1 + p175689);
	    nextMbTypeModel[ctx][type][3] = 1 + 255 * p[0] / (1 + p02);
	    nextMbTypeModel[ctx][type][4] = 1 + 255 * p[3] / (1 + p34);
	    nextMbTypeModel[ctx][type][5] = 1 + 255 * p[1] / (1 + p17);
	    nextMbTypeModel[ctx][type][6] = 1 + 255 * p56 / (1 + p5689);
	    nextMbTypeModel[ctx][type][7] = 1 + 255 * p[5] / (1 + p56);
	    nextMbTypeModel[ctx][type][8] = 1 + 255 * p[8] / (1 + p89);

	    // Restore initial value
	    p[type] = 100 * mbTypesStats[ctx][1][type];
	}
    }
}

void VP62::parseVectorModelsChanges()
{
    int comp, node;

    static byte sigdctPCT[2][2] = { { 237, 246 },
			            { 231, 243 } };

    for(comp = 0; comp < 2; comp++) {
	if (acGetBit(sigdctPCT[comp][0])) {
	    dctVectorModel[comp] = acGetBits(7) << 1;
	    if (dctVectorModel[comp] == 0) {
		dctVectorModel[comp] = 1;
	    }
	}
	if (acGetBit(sigdctPCT[comp][1])) {
	    sigVectorModel[comp] = acGetBits(7) << 1;
	    if (sigVectorModel[comp] == 0) {
		sigVectorModel[comp] = 1;
	    }
	}
    }

    static byte pdvPCT[2][7] = { { 253, 253, 254, 254, 254, 254, 254 },
			         { 245, 253, 254, 254, 254, 254, 254 } };
    for(comp = 0; comp < 2; comp++) {
	for(node = 0; node < 7; node++) {
	    if (acGetBit(pdvPCT[comp][node])) {
		pdvVectorModel[comp][node] = acGetBits(7) << 1;
		if (pdvVectorModel[comp][node] == 0) {
		    pdvVectorModel[comp][node] = 1;
		}
	    }
	}
    }

    static byte fdvPCT[2][8] = { { 254, 254, 254, 254, 254, 250, 250, 252 },
			         { 254, 254, 254, 254, 254, 251, 251, 254 } };
    for(comp = 0; comp < 2; comp++) {
	for(node = 0; node < 8; node++) {
	    if (acGetBit(fdvPCT[comp][node])) {
		fdvVectorModel[comp][node] = acGetBits(7) << 1;
		if (fdvVectorModel[comp][node] == 0) {
		    fdvVectorModel[comp][node] = 1;
		}
	    }
	}
    }
}

void VP62::parseCoeffModelsChanges()
{
    int bt;  // bt Block Type 0 for Y, 1 for U or V
    int node;
    int cg, ct, ctx;
    int defaultProb[11];

    for(node = 0; node < 11; node++) {
	defaultProb[node] = 128;
    }

    static byte dccvPCT[2][11] = { { 146, 255, 181, 207, 232, 243, 238, 251, 244, 250, 249 },
				   { 179, 255, 214, 240, 250, 255, 244, 255, 255, 255, 255 } };
    for(bt = 0; bt < 2; bt++) {
	for(node = 0; node < 11; node++) {
	    if (acGetBit(dccvPCT[bt][node])) {
		defaultProb[node] = acGetBits(7) << 1;
		defaultProb[node] += defaultProb[node] != 0 ? 0 : 1;  // Cannot be 0
		dccvCoeffModel[bt][node] = defaultProb[node];
	    } else if (frameType == FRAME_INTRA) {
		dccvCoeffModel[bt][node] = defaultProb[node];
	    }
	}
    }

    if (acGetBit(128)) {
	static byte coeffReorderPCT[64] =  { 255, 132, 132, 159, 153, 151, 161, 170, 164, 162, 136, 110, 103, 114, 129, 118, 
					     124, 125, 132, 136, 114, 110, 142, 135, 134, 123, 143, 126, 153, 183, 166, 161, 
					     171, 180, 179, 164, 203, 218, 225, 217, 215, 206, 203, 217, 229, 241, 248, 243, 
					     253, 255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255  };

	int pos;

	for(pos = 1; pos < 64; pos++) {
	    if (acGetBit(coeffReorderPCT[pos])) {
		coeffReorder[pos] = (byte)acGetBits(4);
	    }
	}

	initCoeffOrderTable();
    }

    static byte runvPCT[2][14] = { {219, 246, 238, 249, 232, 239, 249, 255, 248, 253, 239, 244, 241, 248},
			           {198, 232, 251, 253, 219, 241, 253, 255, 248, 249, 244, 238, 251, 255} };

    for(cg = 0; cg < 2; cg++) {
	for(node = 0; node < 14; node++) {
	    if (acGetBit(runvPCT[cg][node])) {
		runvCoeffModel[cg][node] = acGetBits(7) << 1;
		runvCoeffModel[cg][node] += runvCoeffModel[cg][node] != 0 ? 0 : 1;
	    }
	}
    }

    static byte ractPCT[3][2][6][11] = {
					    {
						{
						    { 227, 246, 230, 247, 244, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 209, 231, 231, 249, 249, 253, 255, 255, 255 },
						    { 255, 255, 225, 242, 241, 251, 253, 255, 255, 255, 255 },
						    { 255, 255, 241, 253, 252, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 248, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
						},
						{
						    { 240, 255, 248, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 240, 253, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
						}
					    },
					    {
						{
						    { 206, 203, 227, 239, 247, 255, 253, 255, 255, 255, 255 },
						    { 207, 199, 220, 236, 243, 252, 252, 255, 255, 255, 255 },
						    { 212, 219, 230, 243, 244, 253, 252, 255, 255, 255, 255 },
						    { 236, 237, 247, 252, 253, 255, 255, 255, 255, 255, 255 }, 
						    { 240, 240, 248, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
						},
						{
						    { 230, 233, 249, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 238, 238, 250, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 248, 251, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
						}
					    },
					    {
						{
						    { 225, 239, 227, 231, 244, 253, 243, 255, 255, 253, 255 },
						    { 232, 234, 224, 228, 242, 249, 242, 252, 251, 251, 255 },
						    { 235, 249, 238, 240, 251, 255, 249, 255, 253, 253, 255 },
						    { 249, 253, 251, 250, 255, 255, 255, 255, 255, 255, 255 },
						    { 251, 250, 249, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
						},
						{
						    { 243, 244, 250, 250, 255, 255, 255, 255, 255, 255, 255 },
						    { 249, 248, 250, 253, 255, 255, 255, 255, 255, 255, 255 }, 
						    { 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
						    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
						}
					    }
					};
    for(ct = 0; ct < 3; ct++) {  // Code Type
	for(bt = 0; bt < 2; bt++) {	// Block Type  0 for Y, 1 for U or V
	    for(cg = 0; cg < 6; cg++) {
		for(node = 0; node < 11; node++) {
		    if (acGetBit(ractPCT[ct][bt][cg][node])) {
			defaultProb[node] = acGetBits(7) << 1;
			defaultProb[node] += defaultProb[node] != 0 ? 0 : 1;
			ractCoeffModel[bt][ct][cg][node] = defaultProb[node];
		    } else if (frameType == FRAME_INTRA) {
			ractCoeffModel[bt][ct][cg][node] = defaultProb[node];
		    }
		}
	    }
	}
    }

    // dcctCoeffModel is a linear combination of dccvCoeffModel
    static int lc[3][5][2] = {{{ 122, 133  }, { 0, 1  }, { 78, 171  }, { 139, 117 }, { 168, 79  }},
			      {{ 133, 51  },  { 0, 1  }, { 169, 71  }, { 214, 44  }, { 210, 38  }},
			      {{ 142, -16  }, { 0, 1  }, { 221, -30 }, { 246, -3  }, { 203, 17  }}};
    for(bt = 0; bt < 2; bt++) {  // bt = Block Type  0 for Y, 1 for U or V
	for(ctx = 0; ctx < 3; ctx++) {
	    for(node = 0; node < 5; node++) {
		int v = ((dccvCoeffModel[bt][node] * lc[ctx][node][0] + 128) >> 8) + lc[ctx][node][1];
		v = v <= 255 ? v : 255;
		v = v >= 1 ? v : 1;
		dcctCoeffModel[bt][ctx][node] = v;
	    }
	}
    }
}

// Parse and decode a macroblock, draw it into current Yuv buffer
void VP62::decodeMacroBlock(int row, int col)
{
    int b;
    short movedFilteredBlock8x8[64];

    currentMbType = MB_INTRA;

    if (useInterlacing) {
	int prob = ilProb;

	if (row > 3) {
	    if (interlacedBlock != 0) {
		prob -= prob >> 1;
	    } else {
		prob += (256 - prob) >> 1;
	    }
	}

	interlacedBlock = acGetBit(prob);

	if (interlacedBlock) {
	    planeStride[0] = planeStride[1] = planeStride[2] = planeStride[3] = yStride * 2;
	    pixelOffset[2] -= yStride * 7;
	    pixelOffset[3] -= yStride * 7;
	}
    }

    if (frameType != FRAME_INTRA) {
	decodeMbTypeAndVectors(row, col);	// Modifies currentMbType
    }

    memset(coeff420, 0, sizeof(coeff420));

    parseCoeff();   // Coeff get relative values

    addPredictorsDC();	// Adjust by predictors values

    for(b=0; b < 6; b++) {
	iDCT8x8(b);
    }

    switch(currentMbType) {
	case MB_INTRA:	// Intra MB
	    for(b=0; b < 6; b++) {
		drawBlock(b);
	    }
	    break;
	case MB_INTER_NOVEC_PF:	// Inter MB, no vector, from previous frame
	    for(b=0; b < 6; b++) {
		drawDeltaBlockFromYuv(yuvLastFrame, b);
	    }
	    break;
	case MB_INTER_NOVEC_TF:	// Inter MB, no vector, from tagged frame
	    for(b=0; b < 6; b++) {
		drawDeltaBlockFromYuv(yuvTaggedFrame, b);
	    }
	    break;
	case MB_INTER_4V:	// Inter MB, 4 vectors, from previous Frame
	case MB_INTER_DELTA_PF:	// Inter MB, above/left vector + delta, from previous Frame
	case MB_INTER_V1_PF:	// Inter MB, first vector, from previous Frame
	case MB_INTER_V2_PF:	// Inter MB, second vector, from previous Frame
	    for(b=0; b < 6; b++) {
		getBlock(b, movedFilteredBlock8x8, yuvLastFrame);	// Retrieve moved block
		drawDeltaBlockFromMB(movedFilteredBlock8x8, block8x8[b], b);	// Draw delta movedBlock to yuvCurrentFrame
	    }
	    break;
	case MB_INTER_DELTA_TF: // Inter MB, above/left vector + delta, from tagged Frame
	case MB_INTER_V1_TF: // Inter MB, first vector, from tagged Frame
	case MB_INTER_V2_TF: // Inter MB, second vector, from tagged Frame
	    for(b=0; b < 6; b++) {
		getBlock(b, movedFilteredBlock8x8, yuvTaggedFrame);	// Retrieve moved block
		drawDeltaBlockFromMB(movedFilteredBlock8x8, block8x8[b], b);	// Draw delta movedBlock to yuvCurrentFrame
	    }
	    break;
    }

    if (interlacedBlock) {
	planeStride[0] = planeStride[1] = planeStride[2] = planeStride[3] = yStride;
	pixelOffset[2] += yStride * 7;
	pixelOffset[3] += yStride * 7;
    }
}

// Replicate beginning/end of lines/columns to offscreen pixels
void VP62::initOffscreenBorders(byte *yuv)
{
    int Yoffset = 48 * yStride + 48;
    int Uoffset = yStride * (movieHeight + 96) + 24 * uvStride + 24;
    int Voffset = yStride * (movieHeight + 96) + uvStride * (movieHeight / 2 + 48) + 24 * uvStride + 24;

    int height = movieHeight;
    int line = Yoffset;
    int lastx = (line + movieWidth) - 1;
    int border1 = line - 48;
    int border2 = lastx + 1;
    int y,x;

    // Y
    for(y = 0; y < height; y++) {
	for(x = 0; x < 48; x++) {
	    yuv[border1 + x] = yuv[line];   // Left
	    yuv[border2 + x] = yuv[lastx];  // Right
	}

	line += yStride;
	lastx += yStride;
	border1 += yStride;
	border2 += yStride;
    }

    line = 48 * yStride;
    lastx = line + (height - 1) * yStride;
    border1 = 0;
    border2 = lastx + yStride;
    for(y = 0; y < 48; y++) {
	memcpy(yuv + border1, yuv + line, yStride);	// Above
	memcpy(yuv + border2, yuv + lastx, yStride);	// Below
	border1 += yStride;
	border2 += yStride;
    }

    // UV
    height = movieHeight/2;
    int offset = Uoffset;
    int plane;
    for(plane = 0; plane < 2; plane++) {
	line = offset;
	lastx = (line + movieWidth/2) - 1;
	border1 = line - 24;
	border2 = lastx + 1;
	for(y = 0; y < height; y++) {
	    for(x = 0; x < 24; x++) {
		yuv[border1 + x] = yuv[line];
		yuv[border2 + x] = yuv[lastx];
	    }

	    line += uvStride;
	    lastx += uvStride;
	    border1 += uvStride;
	    border2 += uvStride;
	}

	line = offset - 24;
	lastx = line + (height - 1) * uvStride;
	border1 = line - 24 * uvStride;
	border2 = lastx + uvStride;
	int y;
	for(y = 0; y < 24; y++) {
	    memcpy(yuv + border1, yuv + line, uvStride);
	    memcpy(yuv + border2, yuv + lastx, uvStride);
	    border1 += uvStride;
	    border2 += uvStride;
	}

	offset = Voffset;
    }
}

static byte coeffParseTable[6][11] =   { { 159, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				         { 145, 165, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				         { 140, 148, 173, 0, 0, 0, 0, 0, 0, 0, 0 },
				         { 135, 140, 155, 176, 0, 0, 0, 0, 0, 0, 0 },
				         { 130, 134, 141, 157, 180, 0, 0, 0, 0, 0, 0 },
				         { 129, 130, 133, 140, 153, 177, 196, 230, 243, 254, 254 }
				       };

static byte coeffGroups[64] = 	{  0, 0, 1, 1, 1, 2, 2, 2,
				   2, 2, 2, 3, 3, 3, 3, 3,
				   3, 3, 3, 3, 3, 3, 4, 4,
				   4, 4, 4, 4, 4, 4, 4, 4,
				   4, 4, 4, 4, 4, 5, 5, 5,
				   5, 5, 5, 5, 5, 5, 5, 5,
				   5, 5, 5, 5, 5, 5, 5, 5,
				   5, 5, 5, 5, 5, 5, 5, 5
			        };

void VP62::parseCoeff()
{
    static int bias[] =	{ 5, 7, 11, 19, 35, 67 };
    static int bitLength[] = { 0, 1, 2, 3, 4, 10 };

    int bt = 0;	// Plane type	0 for Y, 1 for U or V
    int b;

    for(b = 0; b < 6; b++) {
	int coeffIndex;
	int ctx = prevBlock[b6to4[b]].notNullDC + aboveBlocks[aboveBlockIndex[b]].notNullDC;
	int coeffCodeType;
	int sign;
	int coeff;
	int nbBits;
	int idx;

	if (b > 3) bt = 1;

	// DC values
	if (acGetBit(dcctCoeffModel[bt][ctx][0]) == 0) {
	    coeffCodeType = 0;
	    prevBlock[b6to4[b]].notNullDC = 0;
	    aboveBlocks[aboveBlockIndex[b]].notNullDC = 0;
	} else {
	    prevBlock[b6to4[b]].notNullDC = 1;
	    aboveBlocks[aboveBlockIndex[b]].notNullDC = 1;

	    if (acGetBit(dcctCoeffModel[bt][ctx][2])) {
		coeffCodeType = 2;
		if (acGetBit(dcctCoeffModel[bt][ctx][3])) {
		    if (acGetBit(dccvCoeffModel[bt][6])) {
			if (acGetBit(dccvCoeffModel[bt][8])) {
			    idx = 4 + acGetBit(dccvCoeffModel[bt][10]);
			} else {
			    idx = 2 + acGetBit(dccvCoeffModel[bt][9]);
			}
		    } else {
			idx = acGetBit(dccvCoeffModel[bt][7]);
		    }
		    coeff = bias[idx];
		    nbBits = bitLength[idx];	// Actually nb bits - 1
		    do {
			coeff += acGetBit(coeffParseTable[idx][nbBits]) << nbBits;
		    } while(--nbBits >= 0);
		    sign = acGetBit();
		    coeff420[b][0] = (short)((coeff ^ -sign) + sign);
		} else {
		    if (acGetBit(dcctCoeffModel[bt][ctx][4])) {
			coeff = 3 + acGetBit(dccvCoeffModel[bt][ 5]);
		    } else {
			coeff = 2;
		    }
		    sign = acGetBit();
		    coeff420[b][0] = (short)((coeff ^ -sign) + sign);
		}
	    } else {
		coeffCodeType = 1;
		sign = acGetBit();
		coeff420[b][0] = (short)((1 ^ -sign) + sign);
	    }
	}

	coeffIndex = 1;
	// AC (or TCOEFF)
	while (coeffIndex < 64) {
	    int getCoeff;
	    int ct = coeffCodeType;
	    int cg = coeffGroups[coeffIndex];

	    if (coeffIndex > 1 && coeffCodeType == 0) {
		getCoeff = 1;	// Force to parse COEFF if not first AC coeff
	    } else {
		// RUN or COEFF?
		getCoeff = acGetBit(ractCoeffModel[bt][ct][cg][0]);
	    }

	    if (getCoeff) {
		// Parse COEFF
		if (acGetBit(ractCoeffModel[bt][ct][cg][ 2])) {
		    coeffCodeType = 2;
		    if (acGetBit(ractCoeffModel[bt][ct][cg][ 3])) {
			if (acGetBit(ractCoeffModel[bt][ct][cg][ 6])) {
			    if (acGetBit(ractCoeffModel[bt][ct][cg][ 8])) {
				idx = 4 + acGetBit(ractCoeffModel[bt][ct][cg][ 10]);
			    } else {
				idx = 2 + acGetBit(ractCoeffModel[bt][ct][cg][ 9]);
			    }
			} else {
			    idx = acGetBit(ractCoeffModel[bt][ct][cg][ 7]);
			}
			coeff = bias[idx];
			nbBits = bitLength[idx];    // Actually nb bits - 1
			do {
			    coeff += acGetBit(coeffParseTable[idx][nbBits]) << nbBits;
			} while(--nbBits >= 0);
			sign = acGetBit();
			coeff420[b][coeffIndexToPos[coeffIndex]] = (short)((coeff ^ -sign) + sign);
		    } else {
			if (acGetBit(ractCoeffModel[bt][ct][cg][ 4])) {
			    coeff = 3 + acGetBit(ractCoeffModel[bt][ct][cg][ 5]);
			} else {
			    coeff = 2;
			}
			sign = acGetBit();
			coeff420[b][coeffIndexToPos[coeffIndex]] = (short)((coeff ^ -sign) + sign);
		    }
		} else {
		    coeffCodeType = 1;
		    sign = acGetBit();
		    coeff420[b][coeffIndexToPos[coeffIndex]] = (short)((1 ^ -sign) + sign);
		}
		coeffIndex++;
	    } else {
		// Parse RUN
		if (acGetBit(ractCoeffModel[bt][ct][cg][ 1])) {
		    coeffCodeType = 0;	// Next time parse a COEFF
		} else {
		    break;  // BREAK!!!!!!!!!!!!!!
		}

		int gr = 0;
		if (coeffIndex >= 6) {
		    gr = 1;
		}
		int run;
		if (acGetBit(runvCoeffModel[gr][0]) == 0) {
		    if (acGetBit(runvCoeffModel[gr][1]) == 0) {
			run = 1 + acGetBit(runvCoeffModel[gr][2]);
		    } else {
			run = 3 + acGetBit(runvCoeffModel[gr][3]);
		    }
		} else if (acGetBit(runvCoeffModel[gr][4]) == 0) {
		    if (acGetBit(runvCoeffModel[gr][5]) == 0) {
			run = 5 + acGetBit(runvCoeffModel[gr][6]);
		    } else {
			run = 7 + acGetBit(runvCoeffModel[gr][7]);
		    }
		} else {
		    run = 9 + acGetBit(runvCoeffModel[gr][8]);
		    run += acGetBit(runvCoeffModel[gr][9]) << 1;
		    run += acGetBit(runvCoeffModel[gr][10]) << 2;
		    run += acGetBit(runvCoeffModel[gr][11]) << 3;
		    run += acGetBit(runvCoeffModel[gr][12]) << 4;
		    run += acGetBit(runvCoeffModel[gr][13]) << 5;
		}
		coeffIndex += run;
	    }
	}   // end while
    }	// end for
}

void VP62::addPredictorsDC()
{
    int b;
    int refFrame = referenceFrame[currentMbType];

    for(b = 0; b < 6; b++) {
	int aboveRefFrame = aboveBlocks[aboveBlockIndex[b]].refFrame;
	short prevLineDC = aboveBlocks[aboveBlockIndex[b]].dcCoeff;
	int prevRefFrame = prevBlock[b6to4[b]].refFrame;
	short prevDC = prevBlock[b6to4[b]].dcCoeff;
	int dc = prevDCRefFrame[b6to3[b]][refFrame];	// Same DC as last computed for same refFrame
	if (refFrame == prevRefFrame) {
	    dc = prevDC;    // Same DC as left block
	}
	if (refFrame == aboveRefFrame) {
	    dc = prevLineDC;	// Same DC as above block
	    if (refFrame == prevRefFrame) {
		// Average DC of left and above block
		dc += prevDC;
		dc /= 2;
	    }
	}
	coeff420[b][0] += dc;
	prevDCRefFrame[b6to3[b]][refFrame] = coeff420[b][0];
	aboveBlocks[aboveBlockIndex[b]].dcCoeff = coeff420[b][0];
	aboveBlocks[aboveBlockIndex[b]].refFrame = refFrame;
	prevBlock[b6to4[b]].dcCoeff = coeff420[b][0];
	prevBlock[b6to4[b]].refFrame = refFrame;
    }
}

// Draw MB to yuvCurrentFrame
void VP62::drawBlock(int b)
{
    int xOffset = pixelOffset[b];
    int stride = planeStride[b];
    short *block = block8x8[b];
    int y,x;

    for(y = 0; y < 8; y++) {
	for(x = 0; x < 8; x++) {
	    short val = *block++;
	    yuvCurrentFrame[xOffset + x] = CLIP(val + 128);
	}

	xOffset += stride;
    }
}

// Draw delta MB to yuvCurrentFrame from yuv
void VP62::drawDeltaBlockFromYuv(byte *yuv, int b)
{
    int offset = pixelOffset[b];
    int stride = planeStride[b];
    short *block = block8x8[b];
    int y,x;

    for(y = 0; y < 8; y++) {
	for(x = 0; x < 8; x++) {
	    int v = yuv[offset + x] + (*block++);
	    yuvCurrentFrame[offset + x] = CLIP(v);
	}

	offset += stride;
    }
}

// Draw delta MB to yuvCurrentFrame from movedFilteredBlock8x8
void VP62::drawDeltaBlockFromMB(short *srcBlock, short *delta, int b)
{
    int offset = pixelOffset[b];
    int stride = planeStride[b];
    int y,x;

    for(y = 0; y < 8; y++) {
	for(x = 0; x < 8; x++) {
	    int v = (*srcBlock++) + (*delta++);
	    yuvCurrentFrame[offset + x] = CLIP(v);
	}

	offset += stride;
    }
}

// mbType can only be MB_INTER_DELTA_PF or MB_INTER_DELTA_TF
void VP62::parseVectorAdjustment(short *v, int mbType)
{
    int comp;
    short vector[2];

    vector[0] = vector[1] = 0;
    if (mbType == MB_INTER_DELTA_PF) {
	if (prevFrameFirstVectorCandidatePos < 2) {
	    vector[0] = prevFrameFirstVectorCandidate[0];
	    vector[1] = prevFrameFirstVectorCandidate[1];
	}
    } else if (tagFrameFirstVectorCandidatePos < 2) {
	vector[0] = tagFrameFirstVectorCandidate[0];
	vector[1] = tagFrameFirstVectorCandidate[1];
    }

    // X then Y
    for(comp = 0; comp < 2; comp++) {
	int delta = 0;

	if (acGetBit(dctVectorModel[comp]) == 0) {
	    if (acGetBit(pdvVectorModel[comp][0])) {
		delta += 4;
		if (acGetBit(pdvVectorModel[comp][4])) {
		    delta += 2 + acGetBit(pdvVectorModel[comp][6]);
		} else {
		    delta += acGetBit(pdvVectorModel[comp][5]);
		}
	    } else if (acGetBit(pdvVectorModel[comp][1])) {
		delta +=  2 + acGetBit(pdvVectorModel[comp][3]);
	    } else {
		delta = acGetBit(pdvVectorModel[comp][2]);
	    }
	} else {
	    // Couldn't screw up more
	    delta = acGetBit(fdvVectorModel[comp][0]);
	    delta += acGetBit(fdvVectorModel[comp][1]) << 1;
	    delta += acGetBit(fdvVectorModel[comp][2]) << 2;
	    delta += acGetBit(fdvVectorModel[comp][7]) << 7;
	    delta += acGetBit(fdvVectorModel[comp][6]) << 6;
	    delta += acGetBit(fdvVectorModel[comp][5]) << 5;
	    delta += acGetBit(fdvVectorModel[comp][4]) << 4;
	    if ((delta & 0xf0) != 0) {
		delta += acGetBit(fdvVectorModel[comp][3]) << 3;
	    } else {
		delta += 8;
	    }
	    // Ugly ain't it?
	}

	// Sign
	if (delta) {
	    int neg = acGetBit(sigVectorModel[comp]);
	    if (neg) {
		delta = -delta;
	    }
	}

	// Assign component
	v[comp] = (short)(delta + vector[comp]);
    }
}

// refFrame is either 1 or 2, that is previous frame or tagged frame
int VP62::getVectorsPredictors(int row, int col, int refFrame)
{
    int mbOffset;
    short firstVector[2];
    short secondVector[2];
    int vctx;
    int ctx;
    int pos;
    int firstPos;

    mbOffset = row * mbc + col;
    vctx = 1;   // No predictors
    firstVector[0] = 0;
    firstVector[1] = 0;
    secondVector[0] = 0;
    secondVector[1] = 0;

    for(pos = 0; pos < 12; pos++) {
	int offset = predOffset[pos] + mbOffset;

	if (referenceFrame[macroblocks[offset].type] != refFrame) {
	    continue;
	}

	if (macroblocks[offset].vx == 0 && macroblocks[offset].vy == 0) {
	    continue;
	}

	firstVector[0] = macroblocks[offset].vx;
	firstVector[1] = macroblocks[offset].vy;

	vctx = 2;   // One predictor

	break;
    }

    firstPos = pos;

    for(pos++; pos < 12; pos++) {
	int offset = predOffset[pos] + mbOffset;

	if (referenceFrame[macroblocks[offset].type] != refFrame) {
	    continue;
	}

	if (macroblocks[offset].vx == firstVector[0] && macroblocks[offset].vy == firstVector[1]
	 || macroblocks[offset].vx == 0 && macroblocks[offset].vy == 0) {
	    continue;
	}

	secondVector[0] = macroblocks[offset].vx;
	secondVector[1] = macroblocks[offset].vy;

	vctx = 0;   // Two predictors

	break;
    }

    if (refFrame == REF_FRAME_PREVIOUS) {
	// Previous frame
	ctx = vctx;

	prevFrameFirstVectorCandidatePos = firstPos;

	prevFrameFirstVectorCandidate[0] = firstVector[0];
	prevFrameFirstVectorCandidate[1] = firstVector[1];

	prevFrameSecondVectorCandidate[0] = secondVector[0];
	prevFrameSecondVectorCandidate[1] = secondVector[1];
    } else {
	// Tagged frame  (refFrame == REF_FRAME_TAGGED)
	ctx = 0;    // Two predictors context

	tagFrameFirstVectorCandidatePos = firstPos;

	tagFrameFirstVectorCandidate[0] = firstVector[0];
	tagFrameFirstVectorCandidate[1] = firstVector[1];

	tagFrameSecondVectorCandidate[0] = secondVector[0];
	tagFrameSecondVectorCandidate[1] = secondVector[1];
    }

    return ctx;
}

// IDCT stuff

static byte zigzag[64] = { 0, 1, 8, 16, 9, 2, 3, 10,
			  17, 24, 32, 25, 18, 11, 4, 5,
			  12, 19, 26, 33, 40, 48, 41, 34,
			  27, 20, 13, 6, 7, 14, 21, 28,
			  35, 42, 49, 56, 57, 50, 43, 36,
			  29, 22, 15, 23, 30, 37, 44, 51,
			  58, 59, 52, 45, 38, 31, 39, 46,
			  53, 60, 61, 54, 47, 55, 62, 63  };

//
// Inverse DCT defined as:
//
//            sqrt(2)      7              PI
//  Qj = X0 * ------- + Sum     Xn * Cos( -- * n*(2*j+1) )
//               2         n=1            16
//
void VP62::iDCT8x8(int b)
{
    int src = 0;
    int dst = 0;
    int row, col;
    short *output = block8x8[b];
    int scoeff[64]; // Scaled coeffs
    int c;

    for(c = 0; c < 64; c++) {
	scoeff[zigzag[c]] = coeff420[b][c] * coeffScale[c];
    }

// 64277 = Cos   PI/16	* 65536
#define COS_1_16 64277
// 60547 = Cos 2xPI/16	* 65536
#define COS_2_16 60547
// 54491 = Cos 3xPI/16	* 65536
#define COS_3_16 54491
// 46341 = Cos 4xPI/16	* 65536
#define COS_4_16 46341
// 36410 = Cos 5xPI/16	* 65536
#define COS_5_16 36410
// 25080 = Cos 6xPI/16	* 65536
#define COS_6_16 25080
// 12785 = Cos 7xPI/16	* 65536
#define COS_7_16 12785

    for(row = 0; row < 8; row++) {
	int x0 = scoeff[src];
	int x1 = scoeff[src + 1];
	int x2 = scoeff[src + 2];
	int x3 = scoeff[src + 3];
	int x4 = scoeff[src + 4];
	int x5 = scoeff[src + 5];
	int x6 = scoeff[src + 6];
	int x7 = scoeff[src + 7];
	if (x0 | x1 | x2 | x3 | x4 | x5 | x6 | x7) {
	    int t0 = ((COS_1_16 * x1) >> 16) + ((COS_7_16 * x7) >> 16);
	    int t1 = ((COS_7_16 * x1) >> 16) - ((COS_1_16 * x7) >> 16);
	    int t2 = ((COS_3_16 * x3) >> 16) + ((COS_5_16 * x5) >> 16);
	    int t3 = ((COS_3_16 * x5) >> 16) - ((COS_5_16 * x3) >> 16);
	    int u0 = (COS_4_16 * (t0 - t2)) >> 16;
	    int u1 = (COS_4_16 * (t1 - t3)) >> 16;
	    int u2 = t0 + t2;
	    int u3 = t1 + t3;
	    int t4 = (COS_4_16 * (x0 + x4)) >> 16;
	    int t5 = (COS_4_16 * (x0 - x4)) >> 16;
	    int t6 = ((COS_2_16 * x2) >> 16) + ((COS_6_16 * x6) >> 16);
	    int t7 = ((COS_6_16 * x2) >> 16) - ((COS_2_16 * x6) >> 16);
	    int u4 = t4 - t6;
	    int u5 = t4 + t6;
	    int v0 = t5 + u0;
	    int v1 = u1 - t7;
	    int v2 = t5 - u0;
	    int v3 = u1 + t7;
	    scoeff[src] = u5 + u2;
	    scoeff[src + 7] = u5 - u2;
	    scoeff[src + 1] = v0 + v3;
	    scoeff[src + 2] = v0 - v3;
	    scoeff[src + 3] = u4 + u3;
	    scoeff[src + 4] = u4 - u3;
	    scoeff[src + 5] = v2 + v1;
	    scoeff[src + 6] = v2 - v1;
	}
	src += 8;
    }

    src = 0;
    for(col = 0; col < 8; col++) {
	int x0 = scoeff[src];
	int x1 = scoeff[src + 8];
	int x2 = scoeff[src + 16];
	int x3 = scoeff[src + 24];
	int x4 = scoeff[src + 32];
	int x5 = scoeff[src + 40];
	int x6 = scoeff[src + 48];
	int x7 = scoeff[src + 56];
	if (x0 | x1 | x2 | x3 | x4 | x5 | x6 | x7) {
	    int t0 = ((COS_1_16 * x1) >> 16) + ((COS_7_16 * x7) >> 16);
	    int t1 = ((COS_7_16 * x1) >> 16) - ((COS_1_16 * x7) >> 16);
	    int t2 = ((COS_3_16 * x3) >> 16) + ((COS_5_16 * x5) >> 16);
	    int t3 = ((COS_3_16 * x5) >> 16) - ((COS_5_16 * x3) >> 16);
	    int u0 = (COS_4_16 * (t0 - t2)) >> 16;
	    int u1 = (COS_4_16 * (t1 - t3)) >> 16;
	    int u2 = t0 + t2;
	    int u3 = t1 + t3;
	    int t4 = (COS_4_16 * (x0 + x4)) >> 16;
	    int t5 = (COS_4_16 * (x0 - x4)) >> 16;
	    int t6 = ((COS_2_16 * x2) >> 16) + ((COS_6_16 * x6) >> 16);
	    int t7 = ((COS_6_16 * x2) >> 16) - ((COS_2_16 * x6) >> 16);
	    int u4 = t4 - t6;
	    int u5 = t4 + t6;
	    int v0 = t5 + u0;
	    int v1 = u1 - t7;
	    int v2 = t5 - u0;
	    int v3 = u1 + t7;
	    output[dst] = (short)((u5 + u2 + 8) >> 4);
	    output[dst + 56] = (short)((u5 - u2 + 8) >> 4);
	    output[dst +  8] = (short)((v0 + v3 + 8) >> 4);
	    output[dst + 16] = (short)((v0 - v3 + 8) >> 4);
	    output[dst + 24] = (short)((u4 + u3 + 8) >> 4);
	    output[dst + 32] = (short)((u4 - u3 + 8) >> 4);
	    output[dst + 40] = (short)((v2 + v1 + 8) >> 4);
	    output[dst + 48] = (short)((v2 - v1 + 8) >> 4);
	} else {
	    output[dst] = 0;
	    output[dst +  8] = 0;
	    output[dst + 16] = 0;
	    output[dst + 24] = 0;
	    output[dst + 32] = 0;
	    output[dst + 40] = 0;
	    output[dst + 48] = 0;
	    output[dst + 56] = 0;
	}
	src++;
	dst++;
    }
}

void VP62::simpleBlockCopy(short *dstBlock, byte *yuv, int offset, int stride)
{
    int y,x;

    for(y = 0; y < 8; y++) {
	for(x = 0; x < 8; x++) {
	    *dstBlock++ = (short)yuv[offset + x];
	}
	offset += stride;
    }
}

void VP62::fourPointFilterHV(short *dstBlock, byte *yuv, int offset, int stride, int delta, int *filter)
{
    int y,x;

    for(y = 0; y < 8; y++) {
	for(x = 0; x < 8; x++) {
	    int p = yuv[offset + x - delta   ] * filter[0]
		  + yuv[offset + x           ] * filter[1]
		  + yuv[offset + x + delta   ] * filter[2]
		  + yuv[offset + x + 2*delta ] * filter[3]
		  + 64;
	    p /= 128;
	    *dstBlock++ = CLIP(p);
	}

	offset += stride;
    }
}

void VP62::aaFilterHV(short *dstBlock, byte *yuv, int offset, int stride, int delta, int *aa)
{
    int y,x;

    for(y = 0; y < 8; y++) {
	for(x = 0; x < 8; x++) {
	    *dstBlock++ = (short)((yuv[offset + x] * aa[0] + yuv[offset + x + delta] * aa[1] + 64) >> 7);
	}
	offset += stride;
    }
}

void VP62::fourPointFilterDiag(short *dstBlock, byte *yuv, int offset, int stride, int *hFilter, int *vFilter)
{
    int y,x;
    int line;
    int tmp[8*11];  // Intermediate block of 11 rows by 8 pixels
    int src;

    offset -= stride;

    line = 0;
    for(y = 0; y < 11; y++) {
	for(x = 0; x < 8; x++) {
	    int p =  yuv[offset + x - 1] * hFilter[0]
	           + yuv[offset + x    ] * hFilter[1]
		   + yuv[offset + x + 1] * hFilter[2]
		   + yuv[offset + x + 2] * hFilter[3] + 64;
	    p /= 128;
	    tmp[line + x] = CLIP(p);
	}
	offset += stride;
	line += 8;
    }

    src = 8;
    for(y = 0; y < 8; y++) {
	for(x = 0; x < 8; x++) {
	    int p = tmp[src -  8] * vFilter[0]
		  + tmp[src     ] * vFilter[1]
		  + tmp[src +  8] * vFilter[2]
		  + tmp[src + 16] * vFilter[3] + 64;
	    p >>= 7;
	    *dstBlock++ = CLIP(p);
	    src++;
	}
    }
}

void VP62::aaFilterDiag(short *dstBlock, byte *yuv, int offset, int stride, int *horizAa, int *vertAa)
{
    int y,x;
    int line;
    int src;
    int tmp[8*9];   // Intermediate block of 9 rows of 8 pixels

    line = 0;
    for(y = 0; y < 9; y++) {
	for(x = 0; x < 8; x++) {
	    tmp[line + x] = (yuv[offset + x] * horizAa[0] + yuv[offset + x + 1] * horizAa[1] + 64) >> 7;
	}
	offset += stride;
	line += 8;
    }

    src = 0;
    for(y = 0; y < 8; y++) {
	for(x = 0; x < 8; x++) {
	    int p = tmp[src] * vertAa[0] + tmp[src + 8] * vertAa[1] + 64;
	    *dstBlock++ = (short)(p >> 7);
	    src++;
	}
    }
}

static int antiAliasing[8][2] = { { 128, 0  },
				  { 112, 16  },
				  { 96, 32  },
				  { 80, 48  },
				  { 64, 64  },
				  { 48, 80  },
				  { 32, 96  },
				  { 16, 112  }};

static int blockCopyFilter[16][8][4] = {
			    { 	{  0, 128, 0, 0  },	// 0
				{ -3, 122, 9, 0  },
				{ -4, 109, 24, -1  },
				{ -5, 91, 45, -3  },
				{ -4, 68, 68, -4  },
				{ -3, 45, 91, -5  },
				{ -1, 24, 109, -4  },
				{  0, 9, 122, -3  }},
			    { 	{  0, 128, 0, 0  },	// 1
				{ -4, 124, 9, -1  },
				{ -5, 110, 25, -2  },
				{ -6, 91, 46, -3  },
				{ -5, 69, 69, -5  },
				{ -3, 46, 91, -6  },
				{ -2, 25, 110, -5  },
				{ -1, 9, 124, -4  }},
			    { 	{  0, 128, 0, 0  },	// 2
				{ -4, 123, 10, -1  },
				{ -6, 110, 26, -2  },
				{ -7, 92, 47, -4  },
				{ -6, 70, 70, -6  },
				{ -4, 47, 92, -7  },
				{ -2, 26, 110, -6  },
				{ -1, 10, 123, -4  }},
			    { 	{  0, 128, 0, 0  },	// 3
				{ -5, 124, 10, -1  },
				{ -7, 110, 27, -2  },
				{ -7, 91, 48, -4  },
				{ -6, 70, 70, -6  },
				{ -4, 48, 92, -8  },
				{ -2, 27, 110, -7  },
				{ -1, 10, 124, -5  }},
			    { 	{  0, 128, 0, 0  },	// 4
				{ -6, 124, 11, -1  },
				{ -8, 111, 28, -3  },
				{ -8, 92, 49, -5  },
				{ -7, 71, 71, -7  },
				{ -5, 49, 92, -8  },
				{ -3, 28, 111, -8  },
				{ -1, 11, 124, -6  }},
			    { 	{  0, 128, 0, 0  },	// 5
				{ -6, 123, 12, -1  },
				{ -9, 111, 29, -3  },
				{ -9, 93, 50, -6  },
				{ -8, 72, 72, -8  },
				{ -6, 50, 93, -9  },
				{ -3, 29, 111, -9  },
				{ -1, 12, 123, -6  }},
			    { 	{  0, 128, 0, 0  },	// 6
				{ -7, 124, 12, -1  },
				{ -10, 111, 30, -3  },
				{ -10, 93, 51, -6  },
				{ -9, 73, 73, -9  },
				{ -6, 51, 93, -10  },
				{ -3, 30, 111, -10  },
				{ -1, 12, 124, -7  }},
			    { 	{  0, 128, 0, 0  },	// 7
				{ -7, 123, 13, -1  },
				{ -11, 112, 31, -4  },
				{ -11, 94, 52, -7  },
				{ -10, 74, 74, -10  },
				{ -7, 52, 94, -11  },
				{ -4, 31, 112, -11  },
				{ -1, 13, 123, -7  }},
			    { 	{  0, 128, 0, 0  },	// 8
				{ -8, 124, 13, -1  },
				{ -12, 112, 32, -4  },
				{ -12, 94, 53, -7  },
				{ -10, 74, 74, -10  },
				{ -7, 53, 94, -12  },
				{ -4, 32, 112, -12  },
				{ -1, 13, 124, -8  }},
			    { 	{  0, 128, 0, 0  },	// 9
				{ -9, 124, 14, -1  },
				{ -13, 112, 33, -4  },
				{ -13, 95, 54, -8  },
				{ -11, 75, 75, -11  },
				{ -8, 54, 95, -13  },
				{ -4, 33, 112, -13  },
				{ -1, 14, 124, -9  }},
			    { 	{  0, 128, 0, 0  },	// 10
				{ -9, 123, 15, -1  },
				{ -14, 113, 34, -5  },
				{ -14, 95, 55, -8  },
				{ -12, 76, 76, -12  },
				{ -8, 55, 95, -14  },
				{ -5, 34, 112, -13  },
				{ -1, 15, 123, -9  }},
			    { 	{  0, 128, 0, 0  },	// 11
				{ -10, 124, 15, -1  },
				{ -14, 113, 34, -5  },
				{ -15, 96, 56, -9  },
				{ -13, 77, 77, -13  },
				{ -9, 56, 96, -15  },
				{ -5, 34, 113, -14  },
				{ -1, 15, 124, -10  }},
			    { 	{  0, 128, 0, 0  },	// 12
				{ -10, 123, 16, -1  },
				{ -15, 113, 35, -5  },
				{ -16, 98, 56, -10  },
				{ -14, 78, 78, -14  },
				{ -10, 56, 98, -16  },
				{ -5, 35, 113, -15  },
				{ -1, 16, 123, -10  }},
			    { 	{  0, 128, 0, 0  },	// 13
				{ -11, 124, 17, -2  },
				{ -16, 113, 36, -5  },
				{ -17, 98, 57, -10  },
				{ -14, 78, 78, -14  },
				{ -10, 57, 98, -17  },
				{ -5, 36, 113, -16  },
				{ -2, 17, 124, -11  }},
			    { 	{  0, 128, 0, 0  },	// 14
				{ -12, 125, 17, -2  },
				{ -17, 114, 37, -6  },
				{ -18, 99, 58, -11  },
				{ -15, 79, 79, -15  },
				{ -11, 58, 99, -18  },
				{ -6, 37, 114, -17  },
				{ -2, 17, 125, -12  }},
			    { 	{  0, 128, 0, 0  },	// 15
				{ -12, 124, 18, -2  },
				{ -18, 114, 38, -6  },
				{ -19, 99, 59, -11  },
				{ -16, 80, 80, -16  },
				{ -11, 59, 99, -19  },
				{ -6, 38, 114, -18  },
				{ -2, 18, 124, -12  }}
			    };

void VP62::filteredBlockCopy(short *dstBlock, byte *yuv, int iOffset, int oOffset, int stride, int x8, int y8, bool useEnhancedFilter, int select)
{
    int delta = oOffset - iOffset;
    if (delta < 0) {
	int tmp = iOffset;
	iOffset = oOffset;
	oOffset = tmp;
	delta = oOffset - iOffset;
    }
    if (useEnhancedFilter) {
	if (delta == 1) { // Left or Right combine
	    fourPointFilterHV(dstBlock, yuv, iOffset, stride, delta, blockCopyFilter[select][x8]);
	} else if (delta == stride)	{ // Above or Below combine
	    fourPointFilterHV(dstBlock, yuv, iOffset, stride, delta, blockCopyFilter[select][y8]);
	} else if (delta == stride - 1) { // Lower-left or Upper-right combine
	    fourPointFilterDiag(dstBlock, yuv, iOffset - 1, stride, blockCopyFilter[select][x8], blockCopyFilter[select][y8]);
	} else if (delta == stride + 1) { // Lower-right or Upper-left combine
	    fourPointFilterDiag(dstBlock, yuv, iOffset, stride, blockCopyFilter[select][x8], blockCopyFilter[select][y8]);
	}
    } else {
	if (delta == 1) { // Left or Right combine
	    aaFilterHV(dstBlock, yuv, iOffset, stride, delta, antiAliasing[x8]);
	} else if (delta == stride)	{ // Above or Below combine
	    aaFilterHV(dstBlock, yuv, iOffset, stride, delta, antiAliasing[y8]);
	} else if (delta == stride - 1) { // Lower-left or Upper-right combine
	    aaFilterDiag(dstBlock, yuv, iOffset - 1, stride, antiAliasing[x8], antiAliasing[y8]);
	} else if (delta == stride + 1) { // Lower-right or Upper-left combine
	    aaFilterDiag(dstBlock, yuv, iOffset, stride, antiAliasing[x8], antiAliasing[y8]);
	}
    }
}

void VP62::decode4Vectors(int row, int col)
{
    int type[4];
    int vxSum;
    int vySum;
    int yPlane;
    short vector[2];

    // Parse each block type
    for(yPlane=0; yPlane < 4; yPlane++) {
	type[yPlane] = acGetBits(2);
	if (type[yPlane]) type[yPlane]++;	// Only returns 0, 2, 3 or 4 (all INTER_PF)
    }

    vxSum = vySum = 0;
    // Get vectors
    for(yPlane = 0; yPlane < 4; yPlane++) {
	switch(type[yPlane]) {
	    case MB_INTER_NOVEC_PF:	// No move
		blockVector[yPlane][0] = 0;
		blockVector[yPlane][1] = 0;
		break;
	    case MB_INTER_V1_PF:
		blockVector[yPlane][0] = prevFrameFirstVectorCandidate[0];
		blockVector[yPlane][1] = prevFrameFirstVectorCandidate[1];
		vxSum += prevFrameFirstVectorCandidate[0];
		vySum += prevFrameFirstVectorCandidate[1];
		break;
	    case MB_INTER_V2_PF:
		blockVector[yPlane][0] = prevFrameSecondVectorCandidate[0];
		blockVector[yPlane][1] = prevFrameSecondVectorCandidate[1];
		vxSum += prevFrameSecondVectorCandidate[0];
		vySum += prevFrameSecondVectorCandidate[1];
		break;
	    case MB_INTER_DELTA_PF:
		parseVectorAdjustment(vector, type[yPlane]);
		blockVector[yPlane][0] = vector[0];
		blockVector[yPlane][1] = vector[1];
		vxSum += vector[0];
		vySum += vector[1];
		break;
	}
    }

    // This is the one selected for the whole Macroblock for prediction
    macroblocks[row * mbc + col].vx = blockVector[3][0];
    macroblocks[row * mbc + col].vy = blockVector[3][1];

    // Chroma vectors are average luma vectors
    vxSum /= 4;
    vySum /= 4;
    blockVector[4][0] = (short)vxSum;
    blockVector[4][1] = (short)vySum;
    blockVector[5][0] = (short)vxSum;
    blockVector[5][1] = (short)vySum;
}

int VP62::parseMacroblockType(int prevType, int ctx)
{
    int type;
    if (acGetBit(sameMbTypeModel[ctx][prevType])) {
	type = prevType;
    } else {
	if (acGetBit(nextMbTypeModel[ctx][prevType][0])) {
	    // 1 5 6 7 8 9
	    if (acGetBit(nextMbTypeModel[ctx][prevType][2])) {
		// 5 6 8 9
		if (acGetBit(nextMbTypeModel[ctx][prevType][6])) {
		    // 8 9
		    type = 8 + acGetBit(nextMbTypeModel[ctx][prevType][8]);
		} else {
		    // 5 6
		    type = 5 + acGetBit(nextMbTypeModel[ctx][prevType][7]);
		}
	    } else {
		// 1 7
		if (acGetBit(nextMbTypeModel[ctx][prevType][5])) {
		    type = 7;
		} else {
		    type = 1;
		}
	    }
	} else {
	    // 0 2 3 4
	    if (acGetBit(nextMbTypeModel[ctx][prevType][1])) {
		// 3 4
		type = 3 + acGetBit(nextMbTypeModel[ctx][prevType][4]);
	    } else {
		// 0 2
		if(acGetBit(nextMbTypeModel[ctx][prevType][3])) {
		    type = 2;
		} else {
		    type = 0;
		}
	    }
	}
    }
    return type;
}

void VP62::decodeMbTypeAndVectors(int row, int col)
{
    int ctx;	// Predictors contexts, 0 1 or 2
    short vector[2];
    int b;

    ctx = getVectorsPredictors(row, col, REF_FRAME_PREVIOUS);	// Get predictors from previous frame

    int mbType = parseMacroblockType(prevMbType, ctx);

    prevMbType = mbType;
    currentMbType = mbType;

    macroblocks[row * mbc + col].type = (byte)mbType;

    if (mbType == MB_INTER_4V) { // 4 motion vectors
	decode4Vectors(row, col);
	return;	// RETURN!!!
    }

    short vx, vy;
    switch(mbType) {
	case MB_INTER_V1_PF:
	    vx = prevFrameFirstVectorCandidate[0];
	    vy = prevFrameFirstVectorCandidate[1];
	    break;

	case MB_INTER_V2_PF:
	    vx = prevFrameSecondVectorCandidate[0];
	    vy = prevFrameSecondVectorCandidate[1];
	    break;

	case MB_INTER_V1_TF:
	    getVectorsPredictors(row, col, REF_FRAME_TAGGED);  // Get predictors from tagged frame
	    vx = tagFrameFirstVectorCandidate[0];
	    vy = tagFrameFirstVectorCandidate[1];
	    break;

	case MB_INTER_V2_TF:
	    getVectorsPredictors(row, col, REF_FRAME_TAGGED);  // Get predictors from tagged frame
	    vx = tagFrameSecondVectorCandidate[0];
	    vy = tagFrameSecondVectorCandidate[1];
	    break;

	case MB_INTER_DELTA_PF:
	    parseVectorAdjustment(vector, mbType);
	    vx = vector[0];
	    vy = vector[1];
	    break;

	case MB_INTER_DELTA_TF:
	    getVectorsPredictors(row, col, REF_FRAME_TAGGED);  // Get predictors from tagged frame
	    parseVectorAdjustment(vector, mbType);
	    vx = vector[0];
	    vy = vector[1];
	    break;

	default:	// MB_INTER_NOVEC_PF, MB_INTRA and MB_INTER_NOVEC_TF
	    vx = 0;
	    vy = 0;
	    break;
    }

    macroblocks[row * mbc + col].vx = vx;
    macroblocks[row * mbc + col].vy = vy;

    // Same vector for all blocks
    for(b = 0; b < 6; b++) {
	blockVector[b][0] = vx;
	blockVector[b][1] = vy;
    }
}

static byte filterThreshold[64] = { 14, 14, 13, 13, 12, 12, 10, 10,
			           10, 10, 8, 8, 8, 8, 8, 8,
			           8, 8, 8, 8, 8, 8, 8, 8,
			           8, 8, 8, 8, 8, 8, 8, 8,
			           8, 8, 8, 8, 7, 7, 7, 7,
			           7, 7, 6, 6, 6, 6, 6, 6,
			           5, 5, 5, 5, 4, 4, 4, 4,
			           4, 4, 4, 3, 3, 3, 3, 2  };

static int adjust(int v, int t)
{
    int V = v;
    int s = V >> 31;

    V ^= s;
    V -= s;	// V = abs(v), s retains sign

    if (V >= 2*t) return v;
    if (V <= t) return v;

    V = 2*t - V;

    // Reapply sign
    V += s;
    V ^= s;

    return V;
}

// Structuring element:   1  -3  (3)  -1	(center)
void VP62::edgeFilter(int n, int pixInc, int lineInc, int t)
{
    int pix2Inc = 2 * pixInc;
    int i ;

    for(i = 0; i < 12; i++) {
	int v = (block12x12[n - pix2Inc] + 3*(block12x12[n]-block12x12[n - pixInc]) - block12x12[n + pixInc] + 4) >> 3;
	v = adjust(v,t);
	block12x12[n - pixInc] = CLIP(block12x12[n - pixInc] + v);
	block12x12[n] = CLIP(block12x12[n] - v);
	n += lineInc;
    }
}

// 4th of coordinates for Y
// 8th of coordinates for UV
static int coordDiv[6] = { 4, 4, 4, 4, 8, 8 };

void VP62::fill12x12Block(byte *yuv, int blockStartOffset, int vx, int vy, int b)
{
    int dx,dy;
    int y,r,t,offset;
    int ovx,ovy;

    dx = vx / coordDiv[b];
    dy = vy / coordDiv[b];

    // Fill a 12x12 block moved by dx,dy
    offset = blockStartOffset + orgPlaneStride[b] * dy + dx;
    offset -= planeStride[b] * 2;   // 2 lines above
    offset -= 2;    // 2 pixels left

    for(y = r = 0; r < 12; r++) {
	int c;
	for(c = 0; c < 12; c++) {
	    block12x12[y + c] = yuv[offset + c];
	}

	offset += planeStride[b];
	y += 12;
    }

    // Deblocking filter
    t = filterThreshold[quantizer];
    ovx = (8 - (dx & 7)) & 7;   // X edge coordinate
    if (ovx) {
	edgeFilter(2 + ovx, 1, 12, t);
    }
    ovy = (8 - (dy & 7)) & 7;   // Y edge coordinate
    if (ovy) {
	edgeFilter(2*12 + (ovy * 12), 12, 1, t);
    }
}

static int sampleVariance(byte *yuv, int pixOffset, int stride)
{
    int sum, squareSum;
    int i,j;

    sum = squareSum = 0;
    for(i = 0; i < 8; i += 2) {
	for(j = 0; j < 8; j += 2) {
	    sum += yuv[pixOffset + j];
	    squareSum += yuv[pixOffset + j] * yuv[pixOffset + j];
	}
	pixOffset += stride * 2;
    }
    return ((squareSum * 16) - sum * sum) / (16*16);
}

// Fill movedFilteredBlock8x8 that will be combined with current frame
void VP62::getBlock(int b, short *dstBlock, byte *yuv)
{
    int stride;
    int pixOffset;
    byte *yuvBlockSource;
    int integerBlockOffset;
    int overlapBlockOffset;
    int fracX;	// Modulo on x by fraction of coordinate
    int fracY;	// Modulo on y by fraction of coordinate
    int mask = coordDiv[b]-1;	// 3 or 7

    if (blockCopyFiltering)	{ // NOTE: always seen as true
	fill12x12Block(yuv, pixelOffset[b], blockVector[b][0], blockVector[b][1], b);
	yuvBlockSource = block12x12;
	pixOffset = 0;
	stride = 12;
	integerBlockOffset = stride*2+2;    // Offset to 8x8 block corner in this 12x12 block
	overlapBlockOffset = integerBlockOffset;
	fracX = blockVector[b][0] & mask;
	fracY = blockVector[b][1] & mask;
    } else {
	int dx = blockVector[b][0];
	int dy = blockVector[b][1];
	fracX = dx & mask;
	fracY = dy & mask;

	dx /= coordDiv[b];
	dy /= coordDiv[b];

	yuvBlockSource = yuv;
	stride = planeStride[b];
	pixOffset = pixelOffset[b] + (orgPlaneStride[b] * dy + dx);
	integerBlockOffset = overlapBlockOffset = 0;
    }

    if (fracX) {
	overlapBlockOffset += (blockVector[b][0] <= 0 ? 0 : 1) * 2 - 1;	// Horizontally	+/-1
    }
    if (fracY) {
	overlapBlockOffset += ((blockVector[b][1] <= 0 ? 0 : 1) * 2 - 1) * stride;    // Vertically +/-1
    }

    // All following code takes quarter of coordinate adjustment, so block copy will happen
    // with weights tables use. Header also gives hint on how to use the most appropriate 8th
    // of position filter table.
    // Actually 8th of coordinates for UV, quarter coordinates for Y
    if (integerBlockOffset != overlapBlockOffset) {
	bool useEnhancedFilters;

	useEnhancedFilters = false;

	if (b < 4) {
	    // Corresponding 8th of coordinates positions for Y blocks
	    fracX *= 2;
	    fracY *= 2;
	    if (blockCopyFilterMode == 2) {
		if (maxVectorLength == 0 || (ABS(blockVector[b][0]) <= maxVectorLength && ABS(blockVector[b][1]) <= maxVectorLength)) {
		    if (sampleVarianceThreshold) {
			int sv;
			sv = sampleVariance(yuvBlockSource, pixOffset + integerBlockOffset, stride);
			useEnhancedFilters = sv >= sampleVarianceThreshold;
		    } else {
			useEnhancedFilters = true;
		    }
		}
	    } else {
		useEnhancedFilters = blockCopyFilterMode == 1;
	    }
	}

	filteredBlockCopy(dstBlock, yuvBlockSource, pixOffset + integerBlockOffset, pixOffset + overlapBlockOffset, stride, fracX, fracY, useEnhancedFilters, filterSelection);
    } else {
	simpleBlockCopy(dstBlock, yuvBlockSource, pixOffset + integerBlockOffset, stride);
    }
}

// Arithmetic Coding methods
void VP62::acInit(byte *bufferStart)
{
    acHigh = 255;
    acBits = 8;
    acBuffer = bufferStart;
    acCodeWord = (*acBuffer++) << 8;
    acCodeWord |= (*acBuffer++);
}

int VP62::acGetBit(int prob)
{
    unsigned int low = 1 + (((acHigh - 1) * prob) / 256);
    unsigned int low_shift = low << 8;
    int bit = acCodeWord<low_shift? 0 : 1;
    if (bit) {
	acHigh -= low;
	acCodeWord -= low_shift;
    } else {
	acHigh = low;
    }
    while (acHigh < 128) {  // Renormalize
	acHigh <<= 1;
	acCodeWord <<= 1;
	if (--acBits == 0) {
	    acBits = 8;
	    acCodeWord |= *acBuffer++;
	}
    }
    return bit;
}

int VP62::acGetBit()
{
    // Equiprobable
    int low = (acHigh + 1) >> 1;
    unsigned int low_shift = low << 8;
    int bit = acCodeWord<low_shift? 0 : 1;
    if (bit != 0) {
	acHigh = (acHigh - low) << 1;
	acCodeWord -= low_shift;
    } else {
	acHigh = low << 1;
    }
    acCodeWord <<= 1;
    if (--acBits == 0) {
	acBits = 8;
	acCodeWord |= *acBuffer++;
    }
    return bit;
}

int VP62::acGetBits(int bits)
{
    int value = 0;

    while(bits--) {
	value <<= 1;
	value |= acGetBit();
    }

    return value;
}

// Globals init
void VP62::allocateBuffers()
{
    int b, pos;

    yStride = movieWidth + 96;
    uvStride = yStride / 2;
    ySize = yStride * (movieHeight + 96);
    uvSize = ySize / 4;

    mbr = 6 + movieHeight / 16;
    mbc = 6 + movieWidth / 16;

    aboveBlocks = (AB *)realloc(aboveBlocks, mbc * 4 * sizeof(AB));

    macroblocks = (MB *)realloc(macroblocks, mbc*mbr*sizeof(MB));

    int totalSize = ySize + (uvSize * 2);

    yuvCurrentFrame = (byte*)realloc(yuvCurrentFrame, totalSize);
    yuvTaggedFrame  = (byte*)realloc(yuvTaggedFrame, totalSize);
    yuvLastFrame    = (byte*)realloc(yuvLastFrame, totalSize);

    // Some inits

    for(b = 0; b < 4; b++) {
	planeStride[b] = yStride;
	orgPlaneStride[b] = yStride;
    }

    for(; b < 6; b++) {
	planeStride[b] = uvStride;
	orgPlaneStride[b] = uvStride;
    }

    // Row/Col relative positions or surrounding blocks, from closest to farthest
    static int candidatePredictorRelativePosition[12][2] = {
				   {  0, -1 },	// index 0 -> above block
				   { -1,  0 },	// index 1 -> left block
				   { -1, -1 },
				   {  1, -1 },
				   {  0, -2 },
				   { -2,  0 },
				   { -2, -1 },
				   { -1, -2 },
				   {  1, -2 },
				   {  2, -1 },
				   { -2, -2 },
				   {  2, -2 }
				   };
    for(pos = 0; pos < 12; pos++) {
	predOffset[pos] = candidatePredictorRelativePosition[pos][1] * mbc + candidatePredictorRelativePosition[pos][0];	// Offset to 12 surrounding MBs, like MV candidates in H263
    }
}

////////////////////// PUBLIC ///////////////////////////////

// Output is BGRA  (Little endian)
void VP62::getRGB(byte *rgb32)
{
    byte *Y0, *Y1, *U, *V;
    byte *line0, *line1;
    int y,x;
    
    Y0 = yuvLastFrame + yStride * 48 + 48;
    Y1 = Y0 + yStride;
    U = yuvLastFrame + ySize + uvStride * 24 + 24;
    V = yuvLastFrame + ySize + uvSize + uvStride * 24 + 24;
    
    line0 = rgb32;
    line1 = line0 + movieWidth*4;
    
    for(y=0; y < movieHeight/2; y++) {
	byte *ptr0, *ptr1;
	
	ptr0 = line0;
	ptr1 = line1;
	
	for(x=0; x < movieWidth/2; x++) {
	    int y,u,v,uv;
	    int r,g,b;

	    // U V
	    u = U[x] - 128;
	    v = V[x] - 128;

	    uv = -6657 * u - 13424 * v;
	    u = 33311 * u;
	    v = 26355 * v;

	    //////// FIRST LINE
	    // Y
	    y = 19077 * (Y0[2*x] - 16);

	    r = (y + v) >> 14;
	    g = (y + uv) >> 14;
	    b = (y + u) >> 14;

	    r = CLIP(r);
	    g = CLIP(g);
	    b = CLIP(b);

	    *ptr0++ = (byte)b;
	    *ptr0++ = (byte)g;
	    *ptr0++ = (byte)r;
	    *ptr0++ = 0xff;

	    // Y
	    y = 19077 * (Y0[2*x+1] - 16);

	    r = (y + v) >> 14;
	    g = (y + uv) >> 14;
	    b = (y + u) >> 14;

	    r = CLIP(r);
	    g = CLIP(g);
	    b = CLIP(b);

	    *ptr0++ = (byte)b;
	    *ptr0++ = (byte)g;
	    *ptr0++ = (byte)r;
	    *ptr0++ = 0xff;
	
	    //////// SECOND LINE
	    // Y
	    y = 19077 * (Y1[2*x] - 16);

	    r = (y + v) >> 14;
	    g = (y + uv) >> 14;
	    b = (y + u) >> 14;

	    r = CLIP(r);
	    g = CLIP(g);
	    b = CLIP(b);

	    *ptr1++ = (byte)b;
	    *ptr1++ = (byte)g;
	    *ptr1++ = (byte)r;
	    *ptr1++ = 0xff;

	    // Y
	    y = 19077 * (Y1[2*x+1] - 16);

	    r = (y + v) >> 14;
	    g = (y + uv) >> 14;
	    b = (y + u) >> 14;

	    r = CLIP(r);
	    g = CLIP(g);
	    b = CLIP(b);

	    *ptr1++ = (byte)b;
	    *ptr1++ = (byte)g;
	    *ptr1++ = (byte)r;
	    *ptr1++ = 0xff;
	}

	Y0 += 2*yStride;
	Y1 += 2*yStride;
	line0 += 2*movieWidth*4;
	line1 += 2*movieWidth*4;

	U += uvStride;
	V += uvStride;
    }
}

void VP62::getImageSize(int *width, int *height)
{
    *width = movieWidth;
    *height = movieHeight;
}

void VP62::getDisplaySize(int *width, int *height)
{
    *width = displayColumns * 8;
    *height = displayRows * 8;
}

void VP62::getYUV(byte** yuv, int* pitch)
{
    yuv[0] = yuvLastFrame + yStride * 48 + 48;
    yuv[1] = yuvLastFrame + ySize + uvStride * 24 + 24;
    yuv[2] = yuvLastFrame + ySize + uvSize + uvStride * 24 + 24;
	*pitch = yStride;
}
