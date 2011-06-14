/* crc32.h */
#ifndef CRC32_H
#define CRC32_H

#ifdef __cplusplus
extern "C" {
#endif

/* hash functions */

#define crc32_update(crc, buf, len) (crc = get_crc32(crc, buf, len))

unsigned get_crc32(unsigned crcinit, const unsigned char *msg, size_t size);
unsigned get_crc32_str(unsigned crcinit, const char *str);

#ifdef GENERATE_CRC32_TABLE
void crc32_init_table(void); /* initialize algorithm static data */
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CRC32_H */
