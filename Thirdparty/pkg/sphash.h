#ifndef SPHASH_H
#define SPHASH_H

#if defined(WIN32)

#ifdef SPHASH_EXPORTS
#define SPHASH_API __declspec(dllexport)
#else
#define SPHASH_API __declspec(dllimport)
#endif

#else

#define SPHASH_API

#endif // defined(WIN32)

#define   HASH_ALGO_MD5         0


#define   HASH_MOD_FILE_STR     0 // subsitle hash - pure md5 or xor md5 if mutilple files - return hex string for hash
#define   HASH_MOD_FILE_BIN     1 // same as HASH_MOD_FILE_STR - return binary result of md5 (16 bytes binary)
#define   HASH_MOD_VIDEO_STR    2 // svp hash for video file - return hex string
#define   HASH_MOD_BINARY_STR   10 // pure md5 hash for binary - return hex string
#define   HASH_MOD_BINARY_BIN   11 // pure md5 hash for binary - return binary result of md5 (16 bytes binary)

extern "C"
{

// hash_file expects |file_name| to be double null terminated.
// to hash more than one file, use L"filename1\0filename2\0\0"
SPHASH_API void hash_file(int hash_mode, int hash_algorithm, 
                          const wchar_t* file_name, 
                          char* result_inout, int* result_len);

SPHASH_API void hash_data(int hash_mode, int hash_algorithm, 
                          char* result_inout, int* result_len);

};

#endif // SPHASH_H