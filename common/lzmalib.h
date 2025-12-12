/* LzmaLib.h -- LZMA library interface
2009-04-07 : Igor Pavlov : Public domain
Minimal LZMA decompression library for LeakNet integration */

#ifndef __LZMALIB_H
#define __LZMALIB_H

#include "basetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SZ_OK 0
#define SZ_ERROR_DATA 1
#define SZ_ERROR_MEM 2
#define SZ_ERROR_CRC 3
#define SZ_ERROR_UNSUPPORTED 4
#define SZ_ERROR_PARAM 5
#define SZ_ERROR_INPUT_EOF 6
#define SZ_ERROR_OUTPUT_EOF 7
#define SZ_ERROR_READ 8
#define SZ_ERROR_WRITE 9
#define SZ_ERROR_PROGRESS 10
#define SZ_ERROR_FAIL 11
#define SZ_ERROR_THREAD 12
#define SZ_ERROR_ARCHIVE 16
#define SZ_ERROR_NO_ARCHIVE 17

/*
LzmaUncompress
--------------
In:
  dest     - output data
  destLen  - output data size
  src      - input data
  srcLen   - input data size
  props    - LZMA properties (5 bytes)
  propsSize - size of properties (must be 5)

Out:
  destLen  - processed output size
  srcLen   - processed input size

Returns:
  SZ_OK                - OK
  SZ_ERROR_DATA        - Data error
  SZ_ERROR_MEM         - Memory allocation error
  SZ_ERROR_UNSUPPORTED - Unsupported properties
  SZ_ERROR_INPUT_EOF   - It needs more bytes in input buffer (src)
*/

int LzmaUncompress(unsigned char *dest, size_t *destLen, const unsigned char *src, size_t *srcLen,
  const unsigned char *props, size_t propsSize);

#ifdef __cplusplus
}
#endif

#endif