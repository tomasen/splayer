#include "rar.hpp"

ScanTree::ScanTree(StringList *FileMasks,RECURSE_MODE Recurse,bool GetLinks,SCAN_DIRS GetDirs)
{
  ScanTree::FileMasks=FileMasks;
  ScanTree::Recurse=Recurse;
  ScanTree::GetLinks=GetLinks;
  ScanTree::GetDirs=GetDirs;

  ScanEntireDisk=false;

  SetAllMaskDepth=0;
  *CurMask=0;
  *CurMaskW=0;
  memset(FindStack,0,sizeof(FindStack));
  Depth=0;
  Errors=0;
  *ErrArcName=0;
  Cmd=NULL;
}


ScanTree::~ScanTree()
{
  for (int I=Depth;I>=0;I--)
    if (FindStack[I]!=NULL)
      delete FindStack[I];
}


SCAN_CODE ScanTree::GetNext(FindData *FindData)
{
  if (Depth<0)
    return(SCAN_DONE);

  SCAN_CODE FindCode;
  while (1)
  {
    if (*CurMask==0 && !GetNextMask())
      return(SCAN_DONE);

    FindCode=FindProc(FindData);
    if (FindCode==SCAN_ERROR)
    {
      Errors++;
      continue;
    }
    if (FindCode==SCAN_NEXT)
      continue;
    if (FindCode==SCAN_SUCCESS && FindData->IsDir && GetDirs==SCAN_SKIPDIRS)
      continue;
    if (FindCode==SCAN_DONE && GetNextMask())
      continue;
    break;
  }
  return(FindCode);
}


bool ScanTree::GetNextMask()
{
  if (!FileMasks->GetString(CurMask,CurMaskW,sizeof(CurMask)))
    return(false);
  CurMask[ASIZE(CurMask)-1]=0;
  CurMaskW[ASIZE(CurMaskW)-1]=0;
#ifdef _WIN_32
  UnixSlashToDos(CurMask);
#endif

  // We wish to scan entire disk if mask like c:\ is specified
  // regardless of recursion mode. Use c:\*.* mask when need to scan only 
  // the root directory.
  ScanEntireDisk=IsDiskLetter(CurMask) && IsPathDiv(CurMask[2]) && CurMask[3]==0;

  char *Name=PointToName(CurMask);
  if (*Name==0)
    strcat(CurMask,MASKALL);
  if (Name[0]=='.' && (Name[1]==0 || Name[1]=='.' && Name[2]==0))
  {
    AddEndSlash(CurMask);
    strcat(CurMask,MASKALL);
  }
  SpecPathLength=Name-CurMask;

  bool WideName=(*CurMaskW!=0);

  if (WideName)
  {
    wchar *NameW=PointToName(CurMaskW);
    if (*NameW==0)
      strcatw(CurMaskW,MASKALLW);
    if (NameW[0]=='.' && (NameW[1]==0 || NameW[1]=='.' && NameW[2]==0))
    {
      AddEndSlash(CurMaskW);
      strcatw(CurMaskW,MASKALLW);
    }
    SpecPathLengthW=NameW-CurMaskW;
  }
  else
  {
    wchar WideMask[NM];
    CharToWide(CurMask,WideMask);
    SpecPathLengthW=PointToName(WideMask)-WideMask;
  }
  Depth=0;

  strcpy(OrigCurMask,CurMask);
  strcpyw(OrigCurMaskW,CurMaskW);

  return(true);
}


SCAN_CODE ScanTree::FindProc(FindData *FindData)
{
  if (*CurMask==0)
    return(SCAN_NEXT);
  bool FastFindFile=false;
  
  if (FindStack[Depth]==NULL) // No FindFile object for this depth yet.
  {
    bool Wildcards=IsWildcard(CurMask,CurMaskW);

    // If we have a file name without wildcards, we can try to use
    // FastFind to optimize speed. For example, in Unix it results in
    // stat call instead of opendir/readdir/closedir.
    bool FindCode=!Wildcards && FindFile::FastFind(CurMask,CurMaskW,FindData,GetLinks);

    bool IsDir=FindCode && FindData->IsDir;

    // SearchAll means that we'll use "*" mask for search, so we'll find
    // subdirectories and will be able to recurse into them.
    // We do not use "*" for directories at any level or for files
    // at top level in recursion mode.
    bool SearchAll=!IsDir && (Depth>0 || Recurse==RECURSE_ALWAYS ||
                   Wildcards && Recurse==RECURSE_WILDCARDS || 
                   ScanEntireDisk && Recurse!=RECURSE_DISABLE);
    if (Depth==0)
      SearchAllInRoot=SearchAll;
    if (SearchAll || Wildcards)
    {
      // Create the new FindFile object for wildcard based search.
      FindStack[Depth]=new FindFile;
      char SearchMask[NM];
      strcpy(SearchMask,CurMask);
      if (SearchAll)
        strcpy(PointToName(SearchMask),MASKALL);
      FindStack[Depth]->SetMask(SearchMask);
      if (*CurMaskW)
      {
        wchar SearchMaskW[NM];
        strcpyw(SearchMaskW,CurMaskW);
        if (SearchAll)
          strcpyw(PointToName(SearchMaskW),MASKALLW);
        FindStack[Depth]->SetMaskW(SearchMaskW);
      }
    }
    else
    {
      // Either we failed to fast find or we found a file or we found
      // a directory in RECURSE_DISABLE mode, so we do not need to scan it.
      // We can return here and do not need to process further.
      // We need to process further only if we fast found a directory.
      if (!FindCode || !FindData->IsDir || Recurse==RECURSE_DISABLE)
      {
         // Return SCAN_SUCCESS if we found a file.
        SCAN_CODE RetCode=SCAN_SUCCESS;

        if (!FindCode)
        {
          // Return SCAN_ERROR if problem is more serious than just
          // "file not found".
          RetCode=FindData->Error ? SCAN_ERROR:SCAN_NEXT;

          // If we failed to find an object, but our current mask is excluded,
          // we skip this object and avoid indicating an error.
          if (Cmd!=NULL && Cmd->ExclCheck(CurMask,true,true))
            RetCode=SCAN_NEXT;
          else
            ErrHandler.OpenErrorMsg(ErrArcName,CurMask);
        }

        // If we searched only for one file or directory in "fast find" 
        // (without a wildcard) mode, let's set masks to zero, 
        // so calling function will know that current mask is used 
        // and next one must be read from mask list for next call.
        // It is not necessary for directories, because even in "fast find"
        // mode, directory recursing will quit by (Depth < 0) condition,
        // which returns SCAN_DONE to calling function.
        *CurMask=0;
        *CurMaskW=0;

        return(RetCode);
      }

      // We found a directory using only FindFile::FastFind function.
      FastFindFile=true;
    }
  }

  if (!FastFindFile && !FindStack[Depth]->Next(FindData,GetLinks))
  {
    // We cannot find anything more in directory either because of
    // some error or just as result of all directory entries already read.

    bool Error=FindData->Error;

#ifdef _WIN_32
    if (Error)
    {
      // Do not display an error if we cannot scan contents of reparse
      // point. Vista contains a lot of reparse (or junction) points,
      // which are not accessible.
      if ((FindData->FileAttr & FILE_ATTRIBUTE_REPARSE_POINT)!=0)
        Error=false;

      // Do not display an error if we cannot scan contents of
      // "System Volume Information" folder. Normally it is not accessible.
      if (strstr(CurMask,"System Volume Information\\")!=NULL)
        Error=false;
    }
#endif

    if (Error && Cmd!=NULL && Cmd->ExclCheck(CurMask,true,true))
      Error=false;

#ifndef SILENT
    if (Error)
    {
      Log(NULL,St(MScanError),CurMask);
    }
#endif

    char DirName[NM];
    wchar DirNameW[NM];
    *DirName=0;
    *DirNameW=0;

    // Going to at least one directory level higher.
    delete FindStack[Depth];
    FindStack[Depth--]=NULL;
    while (Depth>=0 && FindStack[Depth]==NULL)
      Depth--;
    if (Depth < 0)
    {
      // Directories scanned both in normal and FastFindFile mode,
      // finally exit from scan here, by (Depth < 0) condition.

      if (Error)
        Errors++;
      return(SCAN_DONE);
    }
    char *Slash=strrchrd(CurMask,CPATHDIVIDER);
    if (Slash!=NULL)
    {
      char Mask[NM];
      strcpy(Mask,Slash);
      if (Depth<SetAllMaskDepth)
        strcpy(Mask+1,PointToName(OrigCurMask));
      *Slash=0;
      strcpy(DirName,CurMask);
      char *PrevSlash=strrchrd(CurMask,CPATHDIVIDER);
      if (PrevSlash==NULL)
        strcpy(CurMask,Mask+1);
      else
        strcpy(PrevSlash,Mask);
    }

    if (*CurMaskW!=0)
    {
      wchar *Slash=strrchrw(CurMaskW,CPATHDIVIDER);
      if (Slash!=NULL)
      {
        wchar Mask[NM];
        strcpyw(Mask,Slash);
        if (Depth<SetAllMaskDepth)
          strcpyw(Mask+1,PointToName(OrigCurMaskW));
        *Slash=0;
        strcpyw(DirNameW,CurMaskW);
        wchar *PrevSlash=strrchrw(CurMaskW,CPATHDIVIDER);
        if (PrevSlash==NULL)
          strcpyw(CurMaskW,Mask+1);
        else
          strcpyw(PrevSlash,Mask);
      }
#ifndef _WIN_CE
      if (LowAscii(CurMaskW))
        *CurMaskW=0;
#endif
    }
    if (GetDirs==SCAN_GETDIRSTWICE &&
        FindFile::FastFind(DirName,DirNameW,FindData,GetLinks) && FindData->IsDir)
    {
      FindData->Flags|=FDDF_SECONDDIR;
      return(Error ? SCAN_ERROR:SCAN_SUCCESS);
    }
    return(Error ? SCAN_ERROR:SCAN_NEXT);
  }

  if (FindData->IsDir)
  {
    // If we found the directory in top (Depth==0) directory
    // and if we are not in "fast find" (directory name only as argument)
    // or in recurse (SearchAll was set when opening the top directory) mode,
    // we do not recurse into this directory. We either return it by itself
    // or skip it.
    if (!FastFindFile && Depth==0 && !SearchAllInRoot)
      return(GetDirs==SCAN_GETCURDIRS ? SCAN_SUCCESS:SCAN_NEXT);

    // Let's check if directory name is excluded, so we do not waste
    // time searching in directory, which will be excluded anyway.
    // We set CheckInclList parameter of ExclCheck to 'true' to ignore
    // the inclusion list here. We do it to correctly handle the situation,
    // when a user added files in the directory to inclusion list,
    // but did not add their parent directory to this list.
    if (Cmd!=NULL && Cmd->ExclCheck(FindData->Name,false,false))
    {
      // If we are here in "fast find" mode, it means that entire directory
      // specified in command line is excluded. Then we need to return
      // SCAN_DONE to go to next mask and avoid the infinite loop
      // in GetNext() function. Such loop would be possible in case of
      // SCAN_NEXT code and "rar a arc dir -xdir" command.

      return(FastFindFile ? SCAN_DONE:SCAN_NEXT);
    }
    
    char Mask[NM];

    strcpy(Mask,FastFindFile ? MASKALL:PointToName(CurMask));
    strcpy(CurMask,FindData->Name);

    if (strlen(CurMask)+strlen(Mask)+1>=NM || Depth>=MAXSCANDEPTH-1)
    {
#ifndef SILENT
      Log(NULL,"\n%s%c%s",CurMask,CPATHDIVIDER,Mask);
      Log(NULL,St(MPathTooLong));
#endif
      return(SCAN_ERROR);
    }

    AddEndSlash(CurMask);
    strcat(CurMask,Mask);

    if (*CurMaskW && *FindData->NameW==0)
      CharToWide(FindData->Name,FindData->NameW);
    if (*FindData->NameW!=0)
    {
      wchar Mask[NM];
      if (FastFindFile)
        strcpyw(Mask,MASKALLW);
      else
        if (*CurMaskW)
          strcpyw(Mask,PointToName(CurMaskW));
        else
          CharToWide(PointToName(CurMask),Mask);
      strcpyw(CurMaskW,FindData->NameW);
      AddEndSlash(CurMaskW);
      strcatw(CurMaskW,Mask);
    }
    Depth++;

    // We need to use OrigCurMask for depths less than SetAllMaskDepth
    // and "*" for depths equal or larger than SetAllMaskDepth.
    // It is important when "fast finding" directories at Depth > 0.
    // For example, if current directory is RootFolder and we compress
    // the following directories structure:
    //   RootFolder
    //     +--Folder1
    //     |  +--Folder2
    //     |  +--Folder3
    //     +--Folder4
    // with 'rar a -r arcname Folder2' command, rar could add not only
    // Folder1\Folder2 contents, but also Folder1\Folder3 if we were using
    // "*" mask at all levels. We need to use "*" mask inside of Folder2,
    // but return to "Folder2" mask when completing scanning Folder2.
    // We can rewrite SearchAll expression above to avoid fast finding
    // directories at Depth > 0, but then 'rar a -r arcname Folder2'
    // will add the empty Folder2 and do not add its contents.

    if (FastFindFile)
      SetAllMaskDepth=Depth;
  }
  if (!FastFindFile && !CmpName(CurMask,FindData->Name,MATCH_NAMES))
    return(SCAN_NEXT);

  return(SCAN_SUCCESS);
}
