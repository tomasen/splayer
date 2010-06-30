#include "common.h"

//-----------------------------------------------------------------------------
// Useful directory separator check
//-----------------------------------------------------------------------------
inline BOOL IsDirSep(CHAR ch)
{
    return ('\\' == ch || '/' == ch);
}

inline BOOL IsDirSep(WCHAR ch)
{
    return (L'\\' == ch || L'/' == ch);
}


//---------------------------------------------------------------------------
// concatenates two paths - szPathAbs is always absolute (including drive letter), szPathRel can be relative (in which case we'll attempt 
// to add it to szPathAbs), or absolute (in which case it's the result)
//---------------------------------------------------------------------------
BOOL LUtilConcatPaths(LPCTSTR szPathAbs, LPCTSTR szPathRel, CString &strPath)
{
    BOOL fSucceed = TRUE;
    BOOL fNeedToCompressPath = false;

    //	ASSERT (szPathAbs && szPathRel);

    strPath.Empty();

    if (!szPathRel)
    {
        fSucceed = FALSE;
    }
    else
    {
        // check whether the path is an absolute path
        if (IsDirSep(szPathRel[0]))
        {
            // if not UNC, then prepend drive-letter
            if (!IsDirSep(szPathRel[1]))
            {
                if (!szPathAbs)
                {
                    return FALSE;
                }
                strPath += szPathAbs[0];
                strPath += _T(':');
            }
        }
        else if (!(((szPathRel[0] >= _T('a') && szPathRel[0] <= _T('z')) || (szPathRel[0] >= _T('A') && szPathRel[0] <= _T('Z')))
            && szPathRel[1] == _T(':')))
        {
            if (!szPathAbs)
            {
                return FALSE;
            }
            // got a relative path -> prepend pathAbs
            strPath += szPathAbs;

            if (szPathRel[0] != _T('\0')&& strPath.Right(1) != _T('\\')) {
                strPath += _T('\\');
            }
        }

        strPath += szPathRel;

        {
            // GetFullPathName needs a buffer large enough to do the processing of the path, so we
            // pass in a buffer that is 2*MAX_PATH and then fail if the final result is longer than MAX_PATH.
            TCHAR szPath[2 * MAX_PATH + 1], *pch;

            szPath[0] = _T('\0');
            DWORD dwRet = GetFullPathName(strPath, 2 * MAX_PATH, szPath, &pch);

            if (dwRet == 0 || dwRet > MAX_PATH || szPath[0] == _T('\0'))
                fSucceed = FALSE;
            else
                strPath = szPath;
        }
    }

    return fSucceed;
}
