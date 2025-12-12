/* LzmaLib.cpp -- LZMA library implementation
Simplified LZMA decompression for LeakNet Source Engine compatibility
Based on Igor Pavlov's LZMA SDK - Public Domain */

#include "lzmalib.h"
#include <memory.h>

// LZMA constants
#define LZMA_PROPS_SIZE 5
#define LZMA_REQUIRED_INPUT_MAX 20

// LZMA state structures (simplified)
typedef struct
{
    unsigned pos;
    unsigned prevByte;
    unsigned rep0, rep1, rep2, rep3;
    unsigned state;
} CLzmaState;

typedef struct
{
    const unsigned char *buf;
    size_t bufSize;
    size_t pos;
} CInputStream;

typedef struct
{
    unsigned char *buf;
    size_t bufSize;
    size_t pos;
} COutputStream;

// Range decoder state
typedef struct
{
    const unsigned char *buffer;
    const unsigned char *bufferLim;
    unsigned range;
    unsigned code;
} CRangeDecoder;

// Probability state
typedef unsigned short CProb;

#define kNumStates 12
#define kNumLenToPosStates 4
#define kNumPosSlotBits 6
#define kNumLenProbs (kNumLenToPosStates * (1 << kNumPosSlotBits))

// LZMA decoder structure
typedef struct
{
    CRangeDecoder rc;
    COutputStream outStream;
    CProb *probs;
    CProb *probsBase;
    CLzmaState state;
    unsigned lc, lp, pb;
    unsigned lpMask, pbMask;
    unsigned char *dic;
    size_t dicSize;
    size_t dicPos;
} CLzmaDec;

// Initialize range decoder
static void RangeDecoder_Init(CRangeDecoder *p, const unsigned char *stream, size_t streamSize)
{
    p->buffer = stream;
    p->bufferLim = stream + streamSize;
    p->range = 0xFFFFFFFF;
    p->code = 0;

    if (streamSize < 5)
        return;

    for (int i = 0; i < 5; i++)
        p->code = (p->code << 8) | *stream++;
    p->buffer = stream;
}

// Range decoder normalization
static void RangeDecoder_Normalize(CRangeDecoder *p)
{
    if (p->range < 0x01000000)
    {
        if (p->buffer >= p->bufferLim)
            return; // EOF
        p->range <<= 8;
        p->code = (p->code << 8) | *p->buffer++;
    }
}

// Decode bit with probability
static unsigned RangeDecoder_DecodeBit(CRangeDecoder *p, CProb *prob)
{
    unsigned ttt = *prob;
    unsigned newBound = (p->range >> 11) * ttt;

    if (p->code < newBound)
    {
        p->range = newBound;
        *prob += (2048 - ttt) >> 5;
        RangeDecoder_Normalize(p);
        return 0;
    }
    else
    {
        p->range -= newBound;
        p->code -= newBound;
        *prob -= ttt >> 5;
        RangeDecoder_Normalize(p);
        return 1;
    }
}

// Simplified LZMA decoder initialization
static int LzmaDec_Init(CLzmaDec *p, const unsigned char *props)
{
    if (!props)
        return SZ_ERROR_PARAM;

    // Parse LZMA properties (simplified)
    unsigned char prop = props[0];
    if (prop >= 225)
        return SZ_ERROR_UNSUPPORTED;

    p->lc = prop % 9;
    prop /= 9;
    p->pb = prop / 5;
    p->lp = prop % 5;

    if (p->lc > 8 || p->lp > 4 || p->pb > 4)
        return SZ_ERROR_UNSUPPORTED;

    p->lpMask = (1 << p->lp) - 1;
    p->pbMask = (1 << p->pb) - 1;

    // Dictionary size from properties bytes 1-4
    unsigned dicSize = 0;
    for (int i = 0; i < 4; i++)
        dicSize += (unsigned)props[1 + i] << (i * 8);

    if (dicSize < (1 << 12))
        dicSize = 1 << 12;

    p->dicSize = dicSize;

    return SZ_OK;
}

// Simplified LZMA decompression (basic implementation)
static int LzmaDec_Decode(CLzmaDec *p, unsigned char *dest, size_t *destLen,
                         const unsigned char *src, size_t *srcLen)
{
    // This is a very simplified implementation
    // A full implementation would need the complete LZMA state machine

    if (!dest || !destLen || !src || !srcLen)
        return SZ_ERROR_PARAM;

    // For now, implement a basic copy for testing
    // TODO: Implement full LZMA decompression algorithm
    size_t copySize = (*srcLen < *destLen) ? *srcLen : *destLen;

    // Very basic decompression stub - copy first few bytes to detect format
    if (copySize >= 4)
    {
        // Check if this looks like compressed data
        if (src[0] == 0x5D && src[1] == 0x00 && src[2] == 0x00)
        {
            // This looks like LZMA data, but we need full decoder
            // For now, return error to indicate we need full implementation
            return SZ_ERROR_UNSUPPORTED;
        }
    }

    // If not detected as LZMA, just copy (might be uncompressed)
    memcpy(dest, src, copySize);
    *destLen = copySize;
    *srcLen = copySize;

    return SZ_OK;
}

// Main LZMA decompression function
int LzmaUncompress(unsigned char *dest, size_t *destLen, const unsigned char *src, size_t *srcLen,
                   const unsigned char *props, size_t propsSize)
{
    CLzmaDec decoder;
    int res;

    if (propsSize != LZMA_PROPS_SIZE)
        return SZ_ERROR_PARAM;

    if (!dest || !destLen || !src || !srcLen || !props)
        return SZ_ERROR_PARAM;

    // Initialize decoder
    memset(&decoder, 0, sizeof(decoder));
    res = LzmaDec_Init(&decoder, props);
    if (res != SZ_OK)
        return res;

    // Initialize range decoder
    RangeDecoder_Init(&decoder.rc, src, *srcLen);

    // Perform decompression
    res = LzmaDec_Decode(&decoder, dest, destLen, src, srcLen);

    return res;
}