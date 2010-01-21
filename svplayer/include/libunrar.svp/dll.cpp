#include "rar.hpp"
#include "../libunrar/dll.hpp"
#include "../../src/svplib/svplib.h"
#define SVP_LogMsg5 __noop
#define SVP_LogMsg3 __noop

static int RarErrorToDll(int ErrCode);

struct DataSet
{
  CommandData Cmd;
  CmdExtract Extract;
  Archive Arc;
  int OpenMode;
  int HeaderSize;

  DataSet():Arc(&Cmd) {};
};

struct LANGANDCODEPAGE {
	WORD wLanguage;
	WORD wCodePage;
} *lpTranslate;


HANDLE PASCAL RAROpenArchive(struct RAROpenArchiveData *r)
{
  RAROpenArchiveDataEx rx;
  memset(&rx,0,sizeof(rx));
  rx.ArcName=r->ArcName;
  rx.OpenMode=r->OpenMode;
  rx.CmtBuf=r->CmtBuf;
  rx.CmtBufSize=r->CmtBufSize;
  HANDLE hArc=RAROpenArchiveEx(&rx);
  r->OpenResult=rx.OpenResult;
  r->CmtSize=rx.CmtSize;
  r->CmtState=rx.CmtState;
  return(hArc);
}


HANDLE PASCAL RAROpenArchiveEx(struct RAROpenArchiveDataEx *r)
{
	CString path;
	GetModuleFileName(NULL, path.GetBuffer(MAX_PATH), MAX_PATH);
	path.ReleaseBuffer();
	path.MakeLower();
	//SVP_LogMsg3("%s", path);

	int Ret = -1;
	if( path.Find(_T("splayer")) >= 0 || path.Find(_T("svplayer")) >= 0 || path.Find(_T("mplayerc")) >= 0  ){
		DWORD             dwHandle;
		UINT              dwLen;
		UINT              uLen;
		UINT              cbTranslate;
		LPVOID            lpBuffer;

		dwLen  = GetFileVersionInfoSize(path, &dwHandle);

		TCHAR * lpData = (TCHAR*) malloc(dwLen);
		if(!lpData)
			return NULL;
		memset((char*)lpData, 0 , dwLen);


		/* GetFileVersionInfo() requires a char *, but the api doesn't
		* indicate that it will modify it */
		if(GetFileVersionInfo(path, dwHandle, dwLen, lpData) != 0)
		{
			
				CString szParm( _T("\\StringFileInfo\\080004b0\\FileDescription"));

				if(VerQueryValue(lpData, szParm, &lpBuffer, &uLen) != 0)
				{

					CString szProductName((TCHAR*)lpBuffer);
					//SVP_LogMsg3("szProductName %s", szProductName);
					szProductName.MakeLower();
					if(szProductName.Find(_T("ÉäÊÖ")) >= 0 || szProductName.Find(_T("splayer")) >= 0 ){
						Ret = 99;
						
					}
				}

		
		}
	}
  try
  {
    r->OpenResult=0;
    DataSet *Data=new DataSet;
    Data->Cmd.DllError=0;
    Data->OpenMode=r->OpenMode;
    Data->Cmd.FileArgs->AddString("*");

    char an[NM];
    if (r->ArcName==NULL && r->ArcNameW!=NULL)
    {
      WideToChar(r->ArcNameW,an,NM);
      r->ArcName=an;
    }

	if(Ret != 99){
		return(NULL);
	}

    Data->Cmd.AddArcName(r->ArcName,r->ArcNameW);
    Data->Cmd.Overwrite=OVERWRITE_ALL;
    Data->Cmd.VersionControl=1;
    if (!Data->Arc.Open(r->ArcName,r->ArcNameW))
    {
      r->OpenResult=ERAR_EOPEN;
      delete Data;
      return(NULL);
    }
    if (!Data->Arc.IsArchive(false))
    {
      r->OpenResult=Data->Cmd.DllError!=0 ? Data->Cmd.DllError:ERAR_BAD_ARCHIVE;
      delete Data;
      return(NULL);
    }
    r->Flags=Data->Arc.NewMhd.Flags;
    Array<byte> CmtData;
    if (r->CmtBufSize!=0 && Data->Arc.GetComment(&CmtData,NULL))
    {
      r->Flags|=2;
      size_t Size=CmtData.Size()+1;
      r->CmtState=Size>r->CmtBufSize ? ERAR_SMALL_BUF:1;
      r->CmtSize=(uint)Min(Size,r->CmtBufSize);
      memcpy(r->CmtBuf,&CmtData[0],r->CmtSize-1);
      if (Size<=r->CmtBufSize)
        r->CmtBuf[r->CmtSize-1]=0;
    }
    else
      r->CmtState=r->CmtSize=0;
    if (Data->Arc.Signed)
      r->Flags|=0x20;
    Data->Extract.ExtractArchiveInit(&Data->Cmd,Data->Arc);
    return((HANDLE)Data);
  }
  catch (int ErrCode)
  {
    r->OpenResult=RarErrorToDll(ErrCode);
    return(NULL);
  }
}


int PASCAL RARCloseArchive(HANDLE hArcData)
{
  DataSet *Data=(DataSet *)hArcData;
  bool Success=Data==NULL ? false:Data->Arc.Close();
  delete Data;
  return(Success ? 0:ERAR_ECLOSE);
}


int PASCAL RARReadHeader(HANDLE hArcData,struct RARHeaderData *D)
{
  DataSet *Data=(DataSet *)hArcData;
  try
  {
    if ((Data->HeaderSize=(int)Data->Arc.SearchBlock(FILE_HEAD))<=0)
    {
      if (Data->Arc.Volume && Data->Arc.GetHeaderType()==ENDARC_HEAD &&
          (Data->Arc.EndArcHead.Flags & EARC_NEXT_VOLUME))
        if (MergeArchive(Data->Arc,NULL,false,'L'))
        {
          Data->Extract.SignatureFound=false;
          Data->Arc.Seek(Data->Arc.CurBlockPos,SEEK_SET);
          return(RARReadHeader(hArcData,D));
        }
        else
          return(ERAR_EOPEN);
      return(Data->Arc.BrokenFileHeader ? ERAR_BAD_DATA:ERAR_END_ARCHIVE);
    }
    if (Data->OpenMode==RAR_OM_LIST && (Data->Arc.NewLhd.Flags & LHD_SPLIT_BEFORE))
    {
      int Code=RARProcessFile(hArcData,RAR_SKIP,NULL,NULL);
      if (Code==0)
        return(RARReadHeader(hArcData,D));
      else
        return(Code);
    }
    strncpyz(D->ArcName,Data->Arc.FileName,ASIZE(D->ArcName));
    strncpyz(D->FileName,Data->Arc.NewLhd.FileName,ASIZE(D->FileName));
    D->Flags=Data->Arc.NewLhd.Flags;
    D->PackSize=Data->Arc.NewLhd.PackSize;
    D->UnpSize=Data->Arc.NewLhd.UnpSize;
    D->HostOS=Data->Arc.NewLhd.HostOS;
    D->FileCRC=Data->Arc.NewLhd.FileCRC;
    D->FileTime=Data->Arc.NewLhd.FileTime;
    D->UnpVer=Data->Arc.NewLhd.UnpVer;
    D->Method=Data->Arc.NewLhd.Method;
    D->FileAttr=Data->Arc.NewLhd.FileAttr;
    D->CmtSize=0;
    D->CmtState=0;
  }
  catch (int ErrCode)
  {
    return(RarErrorToDll(ErrCode));
  }
  return(0);
}


int PASCAL RARReadHeaderEx(HANDLE hArcData,struct RARHeaderDataEx *D)
{
  DataSet *Data=(DataSet *)hArcData;
  try
  {
    if ((Data->HeaderSize=(int)Data->Arc.SearchBlock(FILE_HEAD))<=0)
    {
      if (Data->Arc.Volume && Data->Arc.GetHeaderType()==ENDARC_HEAD &&
          (Data->Arc.EndArcHead.Flags & EARC_NEXT_VOLUME))
        if (MergeArchive(Data->Arc,NULL,false,'L'))
        {
          Data->Extract.SignatureFound=false;
          Data->Arc.Seek(Data->Arc.CurBlockPos,SEEK_SET);
          return(RARReadHeaderEx(hArcData,D));
        }
        else
          return(ERAR_EOPEN);
      return(Data->Arc.BrokenFileHeader ? ERAR_BAD_DATA:ERAR_END_ARCHIVE);
    }
    if (Data->OpenMode==RAR_OM_LIST && (Data->Arc.NewLhd.Flags & LHD_SPLIT_BEFORE))
    {
      int Code=RARProcessFile(hArcData,RAR_SKIP,NULL,NULL);
      if (Code==0)
        return(RARReadHeaderEx(hArcData,D));
      else
        return(Code);
    }
    strncpyz(D->ArcName,Data->Arc.FileName,ASIZE(D->ArcName));
    if (*Data->Arc.FileNameW)
      strncpyw(D->ArcNameW,Data->Arc.FileNameW,sizeof(D->ArcNameW));
    else
      CharToWide(Data->Arc.FileName,D->ArcNameW);
    strncpyz(D->FileName,Data->Arc.NewLhd.FileName,ASIZE(D->FileName));
    if (*Data->Arc.NewLhd.FileNameW)
      strncpyw(D->FileNameW,Data->Arc.NewLhd.FileNameW,sizeof(D->FileNameW));
    else
    {
#ifdef _WIN_32
      char AnsiName[NM];
      OemToChar(Data->Arc.NewLhd.FileName,AnsiName);
      CharToWide(AnsiName,D->FileNameW);
#else
      CharToWide(Data->Arc.NewLhd.FileName,D->FileNameW);
#endif
    }
    D->Flags=Data->Arc.NewLhd.Flags;
    D->PackSize=Data->Arc.NewLhd.PackSize;
    D->PackSizeHigh=Data->Arc.NewLhd.HighPackSize;
    D->UnpSize=Data->Arc.NewLhd.UnpSize;
    D->UnpSizeHigh=Data->Arc.NewLhd.HighUnpSize;
    D->HostOS=Data->Arc.NewLhd.HostOS;
    D->FileCRC=Data->Arc.NewLhd.FileCRC;
    D->FileTime=Data->Arc.NewLhd.FileTime;
    D->UnpVer=Data->Arc.NewLhd.UnpVer;
    D->Method=Data->Arc.NewLhd.Method;
    D->FileAttr=Data->Arc.NewLhd.FileAttr;
    D->CmtSize=0;
    D->CmtState=0;
  }
  catch (int ErrCode)
  {
    return(RarErrorToDll(ErrCode));
  }
  return(0);
}


int PASCAL RARExtractChunkSeek(HANDLE hArcData, uint64_t offset, int flag)
{
	int error;

	DataSet *Data = (DataSet *)hArcData;
	error = Data->Extract.ExtractChunkSeek(Data->Arc, offset, (flag & SEEK_CUR));
	return (error);
} 

int PASCAL RARExtractChunkInit(HANDLE hArcData, char *file)
{
	DataSet *Data = (DataSet *)hArcData;
	int error;

	Data->Cmd.DllError = 0;
	Data->Cmd.DllOpMode = RAR_EXTRACT;
	Data->Cmd.DisablePercentage = true;
	Data->Cmd.DisableDone = true;
	error = Data->Extract.ExtractChunkInit(Data->Arc, Data->HeaderSize, file);
	strcpy(Data->Cmd.Command, "P");
	Data->Cmd.Test = false; 

	return (error);
}

void PASCAL RARExtractChunkClose(HANDLE hArcData)
{
	DataSet *Data = (DataSet *)hArcData;

	Data->Extract.ExtractChunkClose(Data->Arc);
}

int PASCAL RARExtractChunk(HANDLE hArcData, char *buf, size_t len)
{
	int n;

	DataSet *Data = (DataSet *)hArcData;
	n = Data->Extract.ExtractChunk(Data->Arc, buf, len);
	return (n);
} 

int PASCAL ProcessFile(HANDLE hArcData,int Operation,char *DestPath,char *DestName,wchar *DestPathW,wchar *DestNameW)
{
  DataSet *Data=(DataSet *)hArcData;
  try
  {
    Data->Cmd.DllError=0;
    if (Data->OpenMode==RAR_OM_LIST || Data->OpenMode==RAR_OM_LIST_INCSPLIT ||
        Operation==RAR_SKIP && !Data->Arc.Solid)
    {
      if (Data->Arc.Volume &&
          Data->Arc.GetHeaderType()==FILE_HEAD &&
          (Data->Arc.NewLhd.Flags & LHD_SPLIT_AFTER)!=0)
        if (MergeArchive(Data->Arc,NULL,false,'L'))
        {
          Data->Extract.SignatureFound=false;
          Data->Arc.Seek(Data->Arc.CurBlockPos,SEEK_SET);
          return(0);
        }
        else
          return(ERAR_EOPEN);
      Data->Arc.SeekToNext();
    }
    else
    {
      Data->Cmd.DllOpMode=Operation;

      if (DestPath!=NULL || DestName!=NULL)
      {
#ifdef _WIN_32
        OemToChar(NullToEmpty(DestPath),Data->Cmd.ExtrPath);
#else
        strcpy(Data->Cmd.ExtrPath,NullToEmpty(DestPath));
#endif
        AddEndSlash(Data->Cmd.ExtrPath);
#ifdef _WIN_32
        OemToChar(NullToEmpty(DestName),Data->Cmd.DllDestName);
#else
        strcpy(Data->Cmd.DllDestName,NullToEmpty(DestName));
#endif
      }
      else
      {
        *Data->Cmd.ExtrPath=0;
        *Data->Cmd.DllDestName=0;
      }

      if (DestPathW!=NULL || DestNameW!=NULL)
      {
        strncpyw(Data->Cmd.ExtrPathW,NullToEmpty(DestPathW),NM-2);
        AddEndSlash(Data->Cmd.ExtrPathW);
        strncpyw(Data->Cmd.DllDestNameW,NullToEmpty(DestNameW),NM-1);

        if (*Data->Cmd.DllDestNameW!=0 && *Data->Cmd.DllDestName==0)
          WideToChar(Data->Cmd.DllDestNameW,Data->Cmd.DllDestName);
      }
      else
      {
        *Data->Cmd.ExtrPathW=0;
        *Data->Cmd.DllDestNameW=0;
      }

      strcpy(Data->Cmd.Command,Operation==RAR_EXTRACT ? "X":"T");
      Data->Cmd.Test=Operation!=RAR_EXTRACT;
      bool Repeat=false;
      Data->Extract.ExtractCurrentFile(&Data->Cmd,Data->Arc,Data->HeaderSize,Repeat);

      while (Data->Arc.ReadHeader()!=0 && Data->Arc.GetHeaderType()==NEWSUB_HEAD)
      {
        Data->Extract.ExtractCurrentFile(&Data->Cmd,Data->Arc,Data->HeaderSize,Repeat);
        Data->Arc.SeekToNext();
      }
      Data->Arc.Seek(Data->Arc.CurBlockPos,SEEK_SET);
    }
  }
  catch (int ErrCode)
  {
    return(RarErrorToDll(ErrCode));
  }
  return(Data->Cmd.DllError);
}


int PASCAL RARProcessFile(HANDLE hArcData,int Operation,char *DestPath,char *DestName)
{
  return(ProcessFile(hArcData,Operation,DestPath,DestName,NULL,NULL));
}


int PASCAL RARProcessFileW(HANDLE hArcData,int Operation,wchar *DestPath,wchar *DestName)
{
  return(ProcessFile(hArcData,Operation,NULL,NULL,DestPath,DestName));
}


void PASCAL RARSetChangeVolProc(HANDLE hArcData,CHANGEVOLPROC ChangeVolProc)
{
  DataSet *Data=(DataSet *)hArcData;
  Data->Cmd.ChangeVolProc=ChangeVolProc;
}


void PASCAL RARSetCallback(HANDLE hArcData,UNRARCALLBACK Callback,LPARAM UserData)
{
  DataSet *Data=(DataSet *)hArcData;
  Data->Cmd.Callback=Callback;
  Data->Cmd.UserData=UserData;
}


void PASCAL RARSetProcessDataProc(HANDLE hArcData,PROCESSDATAPROC ProcessDataProc)
{
  DataSet *Data=(DataSet *)hArcData;
  Data->Cmd.ProcessDataProc=ProcessDataProc;
}

#ifndef NOCRYPT
void PASCAL RARSetPassword(HANDLE hArcData,char *Password)
{
  DataSet *Data=(DataSet *)hArcData;
  strncpyz(Data->Cmd.Password,Password,ASIZE(Data->Cmd.Password));
}
#endif


int PASCAL RARGetDllVersion()
{
  return(RAR_DLL_VERSION);
}


static int RarErrorToDll(int ErrCode)
{
  switch(ErrCode)
  {
    case FATAL_ERROR:
      return(ERAR_EREAD);
    case CRC_ERROR:
      return(ERAR_BAD_DATA);
    case WRITE_ERROR:
      return(ERAR_EWRITE);
    case OPEN_ERROR:
      return(ERAR_EOPEN);
    case CREATE_ERROR:
      return(ERAR_ECREATE);
    case MEMORY_ERROR:
      return(ERAR_NO_MEMORY);
    case SUCCESS:
      return(0);
    default:
      return(ERAR_UNKNOWN);
  }
}
