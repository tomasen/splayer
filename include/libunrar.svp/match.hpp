#ifndef _RAR_MATCH_
#define _RAR_MATCH_

enum {
   MATCH_NAMES,        // Compare names only.

   MATCH_PATH,         // Compares names and paths. Both must match exactly.
                       // Unlike MATCH_EXACTPATH, also matches names if
                       // mask contains path only and this path is a part
                       // of name path.

   MATCH_EXACTPATH,    // Compares names and paths. Both must match exactly.

   MATCH_SUBPATH,      // Names must be the same, but path in mask is allowed
                       // to be only a part of name path.

   MATCH_WILDSUBPATH   // Works as MATCH_SUBPATH if mask contains wildcards
                       // and as MATCH_PATH otherwise.
};

#define MATCH_MODEMASK           0x0000ffff
#define MATCH_FORCECASESENSITIVE 0x80000000

bool CmpName(char *Wildcard,char *Name,int CmpPath);
bool CmpName(wchar *Wildcard,wchar *Name,int CmpPath);

#endif
