/* hex.c - conversion for hexadecimal and base32 strings.
 *
 * Copyleft:
 * I, the author, hereby release this code into the public domain.
 * This applies worldwide. I grant any entity the right to use this work for
 * ANY PURPOSE, without any conditions, unless such conditions are required
 * by law.
 */
#include "hex.h"

/**
 * Store hexadecimal representation of a byte to given buffer.
 *
 * @param dst  the buffer to receive two symbols of hex representation
 * @param byte the byte to decode
 * @param upper_case flag to print string in uppercase
 * @return pointer to the next char in buffer (dst+2)
 */
char* put_hex_char(char *dst, const unsigned char byte, int upper_case)
{
  const char add = (upper_case ? 'A'-10 : 'a'-10);
  unsigned char c = (byte >> 4) & 15;
  *dst++ = (c>9 ? c+add : c+'0');
  c = byte & 15;
  *dst++ = (c>9 ? c+add : c+'0');
  return dst;
}

/**
 * Store hexadecimal representation of a binary string to given buffer.
 *
 * @param dst the buffer to receive hexadecimal representation
 * @param str binary string
 * @param len string length
 * @param upper_case flag to print string in uppercase
 */
void byte_to_hex(char *dst, const unsigned char *src, unsigned len, int upper_case)
{
  while(len-- > 0) {
    dst = put_hex_char(dst, *src++, upper_case);
  }
  *dst='\0';
}

/**
 * Encode a binary string to base32.
 *
 * @param dst the buffer to store result
 * @param str binary string
 * @param len string length
 * @param upper_case flag to print string in uppercase
 */
void byte_to_base32(char* dest, const unsigned char* src, unsigned len, int upper_case)
{
  const char a = (upper_case ? 'A' : 'a');
  unsigned shift = 0;
  unsigned char word;
  const unsigned char* e = src + len;
  while(src < e) {
    if(shift > 3) {
      word = (*src & (0xFF >> shift));
      shift = (shift + 5) % 8;
      word <<= shift;
      if(src + 1 < e)
        word |= *(src + 1) >> (8 - shift);
      ++src;
    } else {
      shift = (shift + 5) % 8;
      word = ( *src >> ( (8 - shift)&7 ) ) & 0x1F;
      if(shift == 0) src++;
    }
    *dest++ = ( word < 26 ? word + a : word + '2' - 26 );
  }
  *dest = '\0';
}

/**
 * Parse given base32 string and store result to bin.
 *
 * @param str string to parse
 * @param bin result
 * @param len string length
 */
void base32tobyte(const char* str, unsigned char* bin, int len)
{
  const char* e = str + len;
  unsigned shift = 0;
  unsigned char b;
  for(; str<e; str++) {
    b = BASE32TODIGIT(*str);
    shift = (shift + 5) % 8;
    if(shift < 5) {
      *bin++ |= (b >> shift);
    }
    *bin |= b << (8 - shift);
  }
}

/**
 * Encode a binary string to base64.
 * Encoded output length is alwais a multiple of 4 bytes.
 *
 * @param dst the buffer to store result
 * @param str binary string
 * @param len string length
 */
void byte_to_base64(char* dest, const unsigned char* src, unsigned len)
{
  static const char* tail = "0123456789+/";
  unsigned shift = 0;
  unsigned char word;
  const unsigned char* e = src + len;
  while(src < e) {
    if(shift > 2) {
      word = (*src & (0xFF >> shift));
      shift = (shift + 6) % 8;
      word <<= shift;
      if(src + 1 < e)
        word |= *(src + 1) >> (8 - shift);
      ++src;
    } else {
      shift = (shift + 6) % 8;
      word = ( *src >> ( (8 - shift) & 7 ) ) & 0x3F;
      if(shift == 0) src++;
    }
    *dest++ = ( word < 52 ? (word < 26 ? word + 'A' : word - 26 + 'a') : tail[word - 52]);
  }
  if(shift > 0) {
    *dest++ = '=';
    if(shift == 6) *dest++ = '=';
  }
  *dest = '\0';
}


#define HEX2DIGIT(a) ( (a)<='9' ? (a)&0xF : ((a)-'a'+10)&0xF )

/**
 * Convert a hex string with even length to a binary string.
 *
 * @param str string to parse
 * @param bin result
 * @param len string length
 */
void hex_to_byte(const char* str, unsigned char* bin, int len)
{
  /* NOTE: supported parsing only for even len */
  for(; len>=2; len-=2, str+=2) *(bin++) = (HEX2DIGIT(str[0])<<4) | HEX2DIGIT(str[1]);
}

/**
 * Convert a hex string to an integer.
 *
 * @param str string to parse
 * @return parsed unsigned
 */
unsigned hex_to_uint(const char* str)
{
  register unsigned res = 0;
  const char *e;
  for(e=str+8; str<e; str++) res = (res<<4) | HEX2DIGIT(*str);
  return res;
}
