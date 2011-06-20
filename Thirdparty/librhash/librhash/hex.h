/* hex.h */
#ifndef HEX_H
#define HEX_H

#ifdef __cplusplus
extern "C" {
#endif

void byte_to_hex(char *dst, const unsigned char *src, unsigned len, int upper_case);
void byte_to_base32(char* dest, const unsigned char* src, unsigned len, int upper_case);
void byte_to_base64(char* dest, const unsigned char* src, unsigned len);
char* put_hex_char(char *dst, const unsigned char byte, int upper_case);

/* note: IS_HEX() is defined on ASCII-8 while isxdigit() only when isascii()==true */
#define IS_HEX(c) ((c) <= '9' ? (c) >= '0' : (unsigned)(((c) - 'A') & ~0x20) <= ('F' - 'A' + 0U))
#define IS_BASE32(c) (((c) <= '7' ? ('2' <= (c)) : (unsigned)(((c) - 'A') & ~0x20) <= ('Z' - 'A' + 0U)))

#define BASE32TODIGIT(c) ((c) < 'A' ? (c) - '2' + 26 : ((c) & ~0x20) - 'A')
#define BASE32_LENGTH(bytes) (((bytes) * 8 + 4) / 5)
#define BASE64_LENGTH(bytes) ((((bytes) + 2) / 3) * 4)

void base32tobyte(const char* str, unsigned char* bin, int len);
void hex_to_byte(const char* str, unsigned char* bin, int len);
unsigned hex_to_uint(const char* str);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* HEX_H */
