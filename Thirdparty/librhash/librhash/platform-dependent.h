/* platform-dependent.h */
#ifndef PLATFORM_DEPENDENT_H
#define PLATFORM_DEPENDENT_H

/* if compiler is MS VC++ */
#if _MSC_VER > 1000
#define inline __inline

#if _MSC_VER > 1300
/* disable warnings: The POSIX name for this item is deprecated. Use the ISO C++ conformant name. */
#pragma warning(disable : 4996)
/* NOTE: _CRT_SECURE_NO_DEPRECATE can be defined before including any file, but doesn't suppress all C4996 */

/* disable warnings: pointer truncation from 'const unsigned char *' to 'int' */
#pragma warning(disable : 4311)
#endif /* _MSC_VER > 1300 */

/* NOTE:
#if _MSC_VER >= 1400
// this is Visual C++ 2005
#elif _MSC_VER >= 1310
// this is Visual c++ .NET 2003
#elif _MSC_VER > 1300
// this is Visual C++ .NET 2002
#endif*/

#ifndef UNDER_CE
#include <io.h> /* for setmode(), _isatty() */
#else /* UNDER_CE */
#include <stddef.h>

/*static inline char* strdup(const char* str) {
  char* res = malloc(strlen(str)+1);
  if(res) strcpy(res, str);
  return res;
};*/
#endif /* UNDER_CE */

/* check the file state for being a directory */
#define S_ISDIR(state) ( ((state) & 0xF000) == 0x4000 )

#endif /* _MSC_VER > 1000 */

#endif /* PLATFORM_DEPENDENT_H */
