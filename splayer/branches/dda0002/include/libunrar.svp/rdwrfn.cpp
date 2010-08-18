#include "rar.hpp"


#include "../../src/svplib/svplib.h"
#define SVP_LogMsg5 __noop
#define SVP_LogMsg3 __noop

ComprDataIO::ComprDataIO()
{
  Init();
}


void ComprDataIO::Init()
{
  UnpackFromMemory=false;
  UnpackToMemory=false;
  UnpPackedSize=0;
  ShowProgress=true;
  TestMode=false;
  SkipUnpCRC=false;
  PackVolume=false;
  UnpVolume=false;
  NextVolumeMissing=false;
  SrcFile=NULL;
  DestFile=NULL;
  UnpWrSize=0;
  Command=NULL;
  Encryption=0;
  Decryption=0;
  TotalPackRead=0;
  CurPackRead=CurPackWrite=CurUnpRead=CurUnpWrite=0;
  PackFileCRC=UnpFileCRC=PackedCRC=0xffffffff;
  LastPercent=-1;
  SubHead=NULL;
  SubHeadPos=NULL;
  CurrentCommand=0;
  ProcessedArcSize=TotalArcSize=0;
}


/*
* Inspired from llink (by lundman) which in turn was loosely based on
* patches from XBMC (by elupus).
*/
int ComprDataIO::UnpReadSeek(char *file, uint64_t offset)
{
	uint64_t OldPos;
	uint64_t HeadSize;
	Archive *SrcArc = (Archive *)SrcFile;

	if (!SrcFile->IsOpened()) {
		return(-1);
	}

	HeadSize = SrcArc->Tell() - CurUnpRead;

#ifdef DEBUG
	printf("\n%s: Seek begin, HeadSize=%d\n", __func__, HeadSize);
	printf("%s: offset=%llu, ", __func__, offset);
	printf("readpos=%llu, ", ReadPos);
	printf("diff=%llu, ", offset - ReadPos);
	printf("tell=%llu, ", SrcArc->Tell());
	printf("CurUnpRead=%llu\n", CurUnpRead);
#endif

	/* Dumb backward seek, reset to beginning and forward seek */
	if (offset < ReadPos) {
#ifdef DEBUG
		printf("Seeking backwards\n");
#endif
		SrcArc->Close();
		SrcArc->Open(SrcArc->FirstVolumeName, SrcArc->FirstVolumeNameW);

		if (SrcArc->GetHeaderType() != FILE_HEAD)
			SrcArc->SearchBlock(FILE_HEAD);
		do {
#ifdef DEBUG
			printf("File: %s, ", SrcArc->NewLhd.FileName);
			printf("packsize=%llu, ", SrcArc->NewLhd.FullPackSize);
			printf("method=%x\n", SrcArc->NewLhd.Method);
#endif
			/* XXX: fix unicode */
			if (_stricmp(SrcArc->NewLhd.FileName, file) == 0){
				SVP_LogMsg3(" ComprDataIO::UnpReadSeek match %s %s | %s ", file , SrcArc->FileName,  CStringA(SrcArc->FileNameW));
				//strcpy(SrcArc->FileName ,  CStringA(SrcArc->FileNameW));
				break;
			}
			//if (strcasecmp(SrcArc->NewLhd.FileName, file) == 0)
			//	break;
		} while (SrcArc->SearchBlock(FILE_HEAD));

		UnpVolume = true;
		SrcArc->Seek(SrcArc->CurBlockPos, SEEK_CUR);
		SrcArc->ReadHeader();

		CurUnpRead = ReadPos = 0;
	}

#ifdef DEBUG
	printf("%s: offset=%llu, ", __func__, offset);
	printf("readpos=%llu, ", ReadPos);
	printf("diff=%llu, ", offset - ReadPos);
	printf("tell=%llu, ", SrcArc->Tell());
	printf("CurUnpRead=%llu\n", CurUnpRead);
#endif

	/* Forward seek to correct volume */
	if (offset > ReadPos + SrcArc->NewLhd.FullPackSize - CurUnpRead) {
#ifdef DEBUG
		printf("%s: Seeking forward, ", __func__);
		printf("FullPackSize=%llu\n", SrcArc->NewLhd.FullPackSize);
#endif
		OldPos = ReadPos;
		ReadPos -= CurUnpRead;

		for (;;) {
			ReadPos += SrcArc->NewLhd.FullPackSize;

#ifdef DEBUG
			printf("%s: New readpos=%llu, ", __func__, ReadPos);
			printf("offset=%llu\n", offset);
#endif
			if (ReadPos >= offset) {	
				ReadPos -= SrcArc->NewLhd.FullPackSize;
#ifdef DEBUG
				printf("%s: offset reached\n", __func__);
#endif
				break;
			}
			if (!MergeArchive(*SrcArc, this, true, CurrentCommand)) {
				NextVolumeMissing = true;
#ifdef DEBUG
				printf("%s: volume missing\n", __func__);
#endif
				return (-1);
			}
		}
	}
	else {
		ReadPos -= CurUnpRead;
	}

#ifdef DEBUG
	printf("%s: offset=%llu, ", __func__, offset);
	printf("readpos=%llu, ", ReadPos);
	printf("diff=%llu, ", offset - ReadPos);
	printf("tell=%llu, ", SrcArc->Tell());
	printf("CurUnpRead=%llu\n", CurUnpRead);
#endif

	/* Absolute number of bytes read */
	CurUnpRead = offset - ReadPos;
	ReadPos += CurUnpRead;
	UnpPackedSize = SrcArc->NewLhd.FullPackSize - CurUnpRead;

	SrcFile->Seek(CurUnpRead + HeadSize, SEEK_SET);

#ifdef DEBUG
	printf("%s: offset=%llu, ", __func__, offset);
	printf("readpos=%llu, ", ReadPos);
	printf("diff=%llu, ", offset - ReadPos);
	printf("tell=%llu, ", SrcArc->Tell());
	printf("CurUnpRead=%llu, ", CurUnpRead);
	printf("UnpPackedSize=%llu\n", UnpPackedSize);
	printf("Seek complete\n\n");
#endif

	return (0);
}



int ComprDataIO::UnpRead(byte *Addr,size_t Count)
{
  int RetCode=0,TotalRead=0;
  byte *ReadAddr;
  ReadAddr=Addr;
  while (Count > 0)
  {
    Archive *SrcArc=(Archive *)SrcFile;

    size_t ReadSize=((int64)Count>UnpPackedSize) ? (size_t)UnpPackedSize:Count;
    if (UnpackFromMemory)
    {
      memcpy(Addr,UnpackFromMemoryAddr,UnpackFromMemorySize);
      RetCode=(int)UnpackFromMemorySize;
      UnpackFromMemorySize=0;
    }
    else
    {
      if (!SrcFile->IsOpened())
        return(-1);
      RetCode=SrcFile->Read(ReadAddr,ReadSize);
      FileHeader *hd=SubHead!=NULL ? SubHead:&SrcArc->NewLhd;
      if (hd->Flags & LHD_SPLIT_AFTER)
        PackedCRC=CRC(PackedCRC,ReadAddr,RetCode); //shoule be ReadSize?? 
    }
    CurUnpRead+=RetCode;
    TotalRead+=RetCode;
#ifndef NOVOLUME
    // These variable are not used in NOVOLUME mode, so it is better
    // to exclude commands below to avoid compiler warnings.
    ReadAddr+=RetCode;
    Count-=RetCode;
#endif
    UnpPackedSize-=RetCode;
    if (UnpPackedSize == 0 && UnpVolume)
    {
#ifndef NOVOLUME
      if (!MergeArchive(*SrcArc,this,true,CurrentCommand))
#endif
      {
        NextVolumeMissing=true;
        return(-1);
      }
    }
    else
      break;
  }
  Archive *SrcArc=(Archive *)SrcFile;
  if (SrcArc!=NULL)
    ShowUnpRead(SrcArc->CurBlockPos+CurUnpRead,UnpArcSize);
  if (RetCode!=-1)
  {
    RetCode=TotalRead;
#ifndef NOCRYPT
    if (Decryption)
#ifndef SFX_MODULE
      if (Decryption<20)
        Decrypt.Crypt(Addr,RetCode,(Decryption==15) ? NEW_CRYPT : OLD_DECODE);
      else
        if (Decryption==20)
          for (int I=0;I<RetCode;I+=16)
            Decrypt.DecryptBlock20(&Addr[I]);
        else
#endif
        {
          int CryptSize=(RetCode & 0xf)==0 ? RetCode:((RetCode & ~0xf)+16);
          Decrypt.DecryptBlock(Addr,CryptSize);
        }
#endif
  }
  Wait();
  return(RetCode);
}


#if defined(RARDLL) && defined(_MSC_VER) && !defined(_M_X64)
// Disable the run time stack check for unrar.dll, so we can manipulate
// with ProcessDataProc call type below. Run time check would intercept
// a wrong ESP before we restore it.
#pragma runtime_checks( "s", off )
#endif

void ComprDataIO::UnpWrite(byte *Addr,size_t Count)
{

#ifdef RARDLL
  RAROptions *Cmd=((Archive *)SrcFile)->GetRAROptions();
  if (Cmd->DllOpMode!=RAR_SKIP)
  {
    if (Cmd->Callback!=NULL &&
        Cmd->Callback(UCM_PROCESSDATA,Cmd->UserData,(LPARAM)Addr,Count)==-1)
      ErrHandler.Exit(USER_BREAK);
    if (Cmd->ProcessDataProc!=NULL)
    {
      // Here we preserve ESP value. It is necessary for those developers,
      // who still define ProcessDataProc callback as "C" type function,
      // even though in year 2001 we announced in unrar.dll whatsnew.txt
      // that it will be PASCAL type (for compatibility with Visual Basic).
#if defined(_MSC_VER)
#ifndef _M_X64
      __asm mov ebx,esp
#endif
#elif defined(_WIN_32) && defined(__BORLANDC__)
      _EBX=_ESP;
#endif
      int RetCode=Cmd->ProcessDataProc(Addr,(int)Count);

      // Restore ESP after ProcessDataProc with wrongly defined calling
      // convention broken it.
#if defined(_MSC_VER)
#ifndef _M_X64
      __asm mov esp,ebx
#endif
#elif defined(_WIN_32) && defined(__BORLANDC__)
      _ESP=_EBX;
#endif
      if (RetCode==0)
        ErrHandler.Exit(USER_BREAK);
    }
  }
#endif // RARDLL

  UnpWrAddr=Addr;
  UnpWrSize=Count;
  if (UnpackToMemory)
  {
    if (Count <= UnpackToMemorySize)
    {
      memcpy(UnpackToMemoryAddr,Addr,Count);
      UnpackToMemoryAddr+=Count;
      UnpackToMemorySize-=Count;
    }
  }
  else
    if (!TestMode)
      DestFile->Write(Addr,Count);
  CurUnpWrite+=Count;
  if (!SkipUnpCRC)
#ifndef SFX_MODULE
    if (((Archive *)SrcFile)->OldFormat)
      UnpFileCRC=OldCRC((ushort)UnpFileCRC,Addr,Count);
    else
#endif
      UnpFileCRC=CRC(UnpFileCRC,Addr,Count);
  ShowUnpWrite();
  Wait();
}

#if defined(RARDLL) && defined(_MSC_VER) && !defined(_M_X64)
// Restore the run time stack check for unrar.dll.
#pragma runtime_checks( "s", restore )
#endif






void ComprDataIO::ShowUnpRead(int64 ArcPos,int64 ArcSize)
{
  if (ShowProgress && SrcFile!=NULL)
  {
    if (TotalArcSize!=0)
    {
      // important when processing several archives or multivolume archive
      ArcSize=TotalArcSize;
      ArcPos+=ProcessedArcSize;
    }

    Archive *SrcArc=(Archive *)SrcFile;
    RAROptions *Cmd=SrcArc->GetRAROptions();

    int CurPercent=ToPercent(ArcPos,ArcSize);
    if (!Cmd->DisablePercentage && CurPercent!=LastPercent)
    {
      mprintf("\b\b\b\b%3d%%",CurPercent);
      LastPercent=CurPercent;
    }
  }
}


void ComprDataIO::ShowUnpWrite()
{
}








void ComprDataIO::SetFiles(File *SrcFile,File *DestFile)
{
  if (SrcFile!=NULL)
    ComprDataIO::SrcFile=SrcFile;
  if (DestFile!=NULL)
    ComprDataIO::DestFile=DestFile;
  LastPercent=-1;
}


void ComprDataIO::GetUnpackedData(byte **Data,size_t *Size)
{
  *Data=UnpWrAddr;
  *Size=UnpWrSize;
}


void ComprDataIO::SetEncryption(int Method,const char *Password,const byte *Salt,bool Encrypt,bool HandsOffHash)
{
  if (Encrypt)
  {
    Encryption=*Password ? Method:0;
#ifndef NOCRYPT
    Crypt.SetCryptKeys(Password,Salt,Encrypt,false,HandsOffHash);
#endif
  }
  else
  {
    Decryption=*Password ? Method:0;
#ifndef NOCRYPT
    Decrypt.SetCryptKeys(Password,Salt,Encrypt,Method<29,HandsOffHash);
#endif
  }
}


#if !defined(SFX_MODULE) && !defined(NOCRYPT)
void ComprDataIO::SetAV15Encryption()
{
  Decryption=15;
  Decrypt.SetAV15Encryption();
}
#endif


#if !defined(SFX_MODULE) && !defined(NOCRYPT)
void ComprDataIO::SetCmt13Encryption()
{
  Decryption=13;
  Decrypt.SetCmt13Encryption();
}
#endif




void ComprDataIO::SetUnpackToMemory(byte *Addr,uint Size)
{
  UnpackToMemory=true;
  UnpackToMemoryAddr=Addr;
  UnpackToMemorySize=Size;
}


