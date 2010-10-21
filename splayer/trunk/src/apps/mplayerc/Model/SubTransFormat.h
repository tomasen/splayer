#ifndef SUBTRANSFORMAT_H
#define SUBTRANSFORMAT_H

class SubTransFormat
{
public:
  static int ExtractDataFromAiSubRecvBuffer_STL(CAtlList<CString>* m_handlemsgs, std::wstring szFilePath,
                                                FILE* sAiSubRecvBuff,std::wstring tmpoutfile,
                                                std::vector<std::wstring> &szaSubDescs,
                                                std::vector<std::wstring> &tmpfiles);
  static int PackGZfile(std::wstring fnin, std::wstring fnout);
  static std::wstring GetTempFileName();
  static std::wstring GetTempDir();
  static std::wstring ExtractRarFile(std::wstring rarfn);
  static std::wstring GetSameTmpName(std::wstring fnin);
  static std::wstring GetShortFileNameForSearch(std::wstring szFnPath);
  static std::wstring GetHashSignature(const char* szTerm2, const char* szTerm3, char* uniqueIDHash);
  static std::wstring GetVideoFileBasename(std::wstring szVidPath, std::vector<std::wstring>* szaPathInfo = NULL);
  static std::wstring GetSubFileByTempid_STL(size_t iTmpID, std::wstring szVidPath,
                                              std::vector<std::wstring> szaSubDescs,
                                              std::vector<std::wstring> tmpfiles);
  static BOOL IfFileExist_STL(std::wstring szPathname, BOOL evenSlowDriver = TRUE);
  static BOOL IsSpecFanSub(std::wstring szPath, std::wstring szOEM);

private:
  static int HandleSubPackage(FILE* fp, std::vector<std::wstring> &szaSubDescs,
                              std::vector<std::wstring> &tmpfiles);
  static int ExtractSubFiles(FILE* fp, std::vector<std::wstring> &tmpfiles);
  static int ExtractEachSubFile(FILE* fp, std::vector<std::wstring> &tmpfiles);
  static int UnpackGZFile(std::wstring fnin, std::wstring fnout);
  static int Explode(std::wstring szIn, std::wstring szTok, std::vector<std::wstring>* szaOut);
  static BOOL SplitPath_STL(std::wstring fnSVPRarPath, std::wstring &fnrar, std::wstring &fninrar);
  static BOOL IfDirExist_STL(std::wstring path);
  static BOOL IfDirWritable_STL(std::wstring szDir);
  static BOOL GetAppDataPath(std::wstring& path);
  static std::wstring GetPlayerPath_STL(std::wstring progName);
  static std::wstring DetectSubFileLanguage_STL(std::wstring fn);
  static std::wstring GetShortFileNameForSearch2(std::wstring szFn);
  static char* ReadToPTCharByLength(FILE* fp, size_t length);
  static void FilePutContent(std::wstring szFilePath, std::wstring szData, BOOL bAppend);
};

#endif // SUBTRANSFORMAT_H