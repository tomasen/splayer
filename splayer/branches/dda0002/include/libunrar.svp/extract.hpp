#ifndef _RAR_EXTRACT_
#define _RAR_EXTRACT_

enum EXTRACT_ARC_CODE {EXTRACT_ARC_NEXT,EXTRACT_ARC_REPEAT};

class CmdExtract
{
  private:
    EXTRACT_ARC_CODE ExtractArchive(CommandData *Cmd);
    RarTime StartTime; // time when extraction started

    ComprDataIO DataIO;
    Unpack *Unp;
    unsigned long TotalFileCount;

    unsigned long FileCount;
    unsigned long MatchedArgs;
    bool FirstFile;
    bool AllMatchesExact;
    bool ReconstructDone;

    char ArcName[NM];
    wchar ArcNameW[NM];

    char Password[MAXPASSWORD];
    bool PasswordAll;
    bool PrevExtracted;
    char DestFileName[NM];
    wchar DestFileNameW[NM];
    bool PasswordCancelled;

	uint64_t FileOffset;
	uint64_t BytesLeft; 
	char *ExtractFile;

  public:
    CmdExtract();
    ~CmdExtract();
	int ExtractChunkInit(Archive &Arc, int HeaderSize, char *file);
	int ExtractChunkClose(Archive &Arc);
	int ExtractChunk(Archive &Arc, char *buf, size_t len);
	int ExtractChunkSeek(Archive &Arc, uint64_t offset, int cur);
    void DoExtract(CommandData *Cmd);
    void ExtractArchiveInit(CommandData *Cmd,Archive &Arc);
    bool ExtractCurrentFile(CommandData *Cmd,Archive &Arc,size_t HeaderSize,
                            bool &Repeat);
    static int64_t  UnstoreFile(ComprDataIO &DataIO,int64 DestUnpSize);

    bool SignatureFound;
};

#endif
