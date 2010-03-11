// This is the main DLL file.

#include "stdafx.h"

#include "lyriclib.h"
#include "..\..\src\svplib\svplib.h"
#include "..\..\src\svplib\SVPToolBox.h"
#include "..\..\src\subtitles\TextFile.h"

#include <streams.h>
#include <afxtempl.h>
#include <atlpath.h>
#include "..\..\src\apps\mplayerc\mplayerc.h"

#include "db_viewlyrics.h"
#include "db_lrcdb.h"
#include "db_lyrdb.h"
#include "db_leos.h"
#include "db_ailrc.h"
#include "db_lyricsfly.h"

//LINKOPTION_STATIC
#define ID3LIB_LINKOPTION LINKOPTION_STATIC
#include "..\id3lib\include\id3\tag.h"
#include "..\id3lib\include\id3\id3lib_frame.h"
#include <id3/misc_support.h>




static TCHAR* ext_lrc[2][1] = 
{
    {
        _T(".lrc")
    },
    {
        _T("*.lrc")
    }
};

static int LrcFileCompare(const void* elem1, const void* elem2)
{
    return(((LrcFile*)elem1)->fn.CompareNoCase(((LrcFile*)elem2)->fn));
}

BOOL  CLyricLib::isLyricFile(CString szFn){
    int extsubnum = countof(ext_lrc[0]);
    szFn.MakeLower();
    for(int i = 0; i < extsubnum; i++){
        int iPos = szFn.Find( ext_lrc[0][i] );
        if( iPos >= 0 && ( iPos + CString(ext_lrc[0][i]).GetLength() ) == szFn.GetLength() ){
            return TRUE;
        }
    }
    return FALSE;
}

void CLyricLib::fetch_lyric_from_internet()
{
    CAutoLock lck(&m_fetch_lock);
    m_stop_downloading = 0;
    Empty();
    CString szDownloadingForFile = m_sz_current_music_file;
    try{
        ID3_Tag myTag;

        myTag.Link(CStringA(szDownloadingForFile), ID3TT_ALL);
        
        ID3_Frame* myFrame = myTag.Find(ID3FID_TITLE);
        if(myFrame)
        {
             char *sText = ID3_GetString(myFrame, ID3FN_TEXT);
             if(sText){
                title = CString( CStringA(sText) );
             }
        }
       
        myFrame = myTag.Find(ID3FID_ALBUM);
        if(myFrame)
        {
            char *sText = ID3_GetString(myFrame, ID3FN_TEXT);
            if(sText){
                album = CString( CStringA(sText) );
            }
        }

        myFrame = myTag.Find(ID3FID_ORIGARTIST);
        if(myFrame)
        {
            char *sText = ID3_GetString(myFrame, ID3FN_TEXT);
            if(sText){
                artist = CString( CStringA(sText) );
            }
        }
    }catch(...){}
    SVP_LogMsg5(L"Look for Lyric Result %s | %s | %s | %s", szDownloadingForFile, this->title.c_str(), this->artist.c_str(), this->album.c_str());
    

    db_lrcdb static_lrcdb;
    db_lyrdb static_lyrdb;
    db_viewlyrics static_viewlyrics;
    db_leos static_leos;
    db_ailrc static_ailrc;
    db_lyricsfly static_lyricsfly;

      try{
        availableLyrics.clear();
        static_viewlyrics.GetResults(&wcfg, &availableLyrics, this->title, this->artist, this->album);
  

        if(try_download_lyric(szDownloadingForFile))
            return;
        else{
            availableLyrics.clear();
            static_lrcdb.GetResults(&wcfg, &availableLyrics, this->title, this->artist, this->album);
        }
        
   
        if(try_download_lyric(szDownloadingForFile))
            return;
        else{
            availableLyrics.clear();
            static_lyrdb.GetResults(&wcfg, &availableLyrics, this->title, this->artist, this->album);
        }

    
        if(try_download_lyric(szDownloadingForFile))
            return;
        else{
            availableLyrics.clear();
            static_leos.GetResults(&wcfg, &availableLyrics, this->title, this->artist, this->album);
        }
    
        if(try_download_lyric(szDownloadingForFile))
            return;
        else{
            availableLyrics.clear();
            static_ailrc.GetResults(&wcfg, &availableLyrics, this->title, this->artist, this->album);
        }
  
        if(try_download_lyric(szDownloadingForFile))
            return;
        else{
            availableLyrics.clear();
            static_lyricsfly.GetResults(&wcfg, &availableLyrics, this->title, this->artist, this->album);
        }

         try_download_lyric(szDownloadingForFile);

     }catch(...){}

}
bool  CLyricLib::try_download_lyric(CString szDownloadingForFile)
{
    bool ret = false;
    if(availableLyrics.size() > 0)
    {

        for(uint curIndex = 0; curIndex < availableLyrics.size() ; curIndex++){
            if (availableLyrics[ curIndex ].SourceType != ST_INTERNET)
                continue ;

            if(!availableLyrics[ curIndex ].IsTimestamped)
                continue ;

            if(!availableLyrics[ curIndex ].db)
                continue ;
            //SVP_LogMsg5(L"Get %d Lyric Result %d", availableLyrics.size(), curIndex);

            availableLyrics[ curIndex ].db->DownloadLyrics(&wcfg, &availableLyrics[ curIndex ]);

            CSVPToolBox svpTool;
            AppSettings& s = AfxGetAppSettings();
            CPath szStorePath( s.GetSVPSubStorePath() );
            szStorePath.RemoveBackslash();
            szStorePath.AddBackslash();

            CStringArray szaFilename ;
            svpTool.getVideoFileBasename(szDownloadingForFile, &szaFilename);
            szStorePath.Append(szaFilename.GetAt(3));
            szStorePath.AddExtension(L".lrc");

            //svpTool.filePutContent(szStorePath, CString(availableLyrics[curIndex].Text.c_str()) );
            CStdioFile f;

            if(f.Open(szStorePath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary ))
            {
                char uft16mark[3] = {0xff, 0xfe, 0x00};
                f.Write(uft16mark , 2);
                f.Write( availableLyrics[curIndex].Text.c_str(), availableLyrics[curIndex].Text.length() );

                f.Close();
            }
            //AfxMessageBox( availableLyrics[curIndex].Text.c_str() );
            SVP_LogMsg5(L"Get %d Lyric Result %s size %d", availableLyrics.size(),szStorePath , availableLyrics[curIndex].Text.length());
            if(szDownloadingForFile == m_sz_current_music_file){
                if(LoadLyricFile(szStorePath) > 0){
                    ret = true;
                    CWnd* pFrame = AfxGetMainWnd();
                    if(pFrame){
                        CString osd_msg;
                        //
                        osd_msg.Format(ResStr(IDS_GOT_LYRIC_OSD_MSG), availableLyrics[ curIndex ].db->GetName() );
                        ::SendMessage(pFrame->m_hWnd, WM_USER+31, (UINT_PTR)osd_msg.GetBuffer(), 3000); 
                        osd_msg.ReleaseBuffer();
                    }
                }
                break;
            }

            
        }
    }
    return ret;
}

void CLyricLib::GetLrcFileNames(CString fn, CAtlArray<CString>& paths, CAtlArray<LrcFile>& ret, BOOL byDir)
{
    ret.RemoveAll();

    int extlistnum = countof(ext_lrc);
    int extsubnum = countof(ext_lrc[0]);

    fn.Replace('\\', '/');

    bool fWeb = false;
    {
        //		int i = fn.Find(_T("://"));
        int i = fn.Find(_T("http://"));
        if(i > 0) {fn = _T("http") + fn.Mid(i); fWeb = true;}
    }

    int	l = fn.GetLength(), l2 = l;
    l2 = fn.ReverseFind('.');
    l = fn.ReverseFind('/') + 1;
    if(l2 < l) l2 = l;

    CString orgpath = fn.Left(l);
    CString title = fn.Mid(l, l2-l);
    if (byDir){
        orgpath = fn;
        title = _T("");

    }
    CString filename = title + _T(".nooneexpectsthespanishinquisition");

    if(!fWeb)
    {
        // struct _tfinddata_t file, file2;
        // long hFile, hFile2 = 0;

        WIN32_FIND_DATA wfd, wfd2;
        HANDLE hFile, hFile2;

        for(int k = 0; k < paths.GetCount(); k++)
        {
            CString path = paths[k];
            path.Replace('\\', '/');

            l = path.GetLength();
            if(l > 0 && path[l-1] != '/') path += '/';

            if(path.Find(':') == -1 && path.Find(_T("\\\\")) != 0) path = orgpath + path;

            path.Replace(_T("/./"), _T("/"));
            path.Replace('/', '\\');

            // CAtlList<CString> sl;

            bool fEmpty = true;

            if((hFile = FindFirstFile(path + title + _T("*"), &wfd)) != INVALID_HANDLE_VALUE)
            {
                do
                {
                    if(filename.CompareNoCase(wfd.cFileName) != 0) 
                    {
                        fEmpty = false;
                        // sl.AddTail(path + file.name);
                    }
                }
                while(FindNextFile(hFile, &wfd));

                FindClose(hFile);
            }

            // TODO: use 'sl' in the next step to find files (already a nice speedup as it is now...)
            if(fEmpty) continue;

            for(int j = 0; j < extlistnum; j++)
            {
                for(int i = 0; i < extsubnum; i++)
                {

                    CString szBuf = path + title + ext_lrc[j][i];
                    //SVP_LogMsg5(L"get sub1 %s %s %s %s",szBuf, path,title ,ext_lrc[j][i]);
                    if(byDir){szBuf = path + _T("*") + ext_lrc[j][i];}
                    if((hFile = FindFirstFile(szBuf, &wfd)) != INVALID_HANDLE_VALUE)
                    {
                        do
                        {
                            CString fn = path + wfd.cFileName;

                            hFile2 = INVALID_HANDLE_VALUE;

                            if(j == 0 || (hFile2 = FindFirstFile(fn.Left(fn.ReverseFind('.')) + _T(".avi"), &wfd2)) == INVALID_HANDLE_VALUE)
                            {
                                LrcFile f;
                                //SVP_LogMsg5(L"get sub %s %s",szBuf, fn);
                                f.fn = fn;
                                ret.Add(f);
                            }

                            if(hFile2 != INVALID_HANDLE_VALUE)
                            {
                                FindClose(hFile2);
                            }
                        }
                        while(FindNextFile(hFile, &wfd));

                        FindClose(hFile);
                    }
                }
            }
        }
    }
    
    // sort files, this way the user can define the order (movie.00.English.srt, movie.01.Hungarian.srt, etc)

    qsort(ret.GetData(), ret.GetCount(), sizeof(LrcFile), LrcFileCompare);
}


CLyricLib::CLyricLib(void)
: m_stop_downloading(0)
{
}

CLyricLib::~CLyricLib(void)
{
}



int CLyricLib::LoadLyricFile(CString sz_lyric_file_path)
{
    m_has_lyric = false;
    CSVPToolBox svptool;
    CString szContent, szBuff ;
    CTextFile ctxt;
    UINT ulRequiredConvert = 0;
    if( ctxt.Open(sz_lyric_file_path) )
    {
        if(!ctxt.IsUnicode())
        {
            ulRequiredConvert = svptool.DetectFileCharset(sz_lyric_file_path);
        }
        if(ulRequiredConvert)
        {
            UINT cCodePage = CP_ACP;
            switch(ulRequiredConvert)
            {
                case GB2312_CHARSET:
                    cCodePage = 936;
                    break;
                case CHINESEBIG5_CHARSET:
                    cCodePage = 950;
                    break;
              
            }
            SVP_LogMsg5(L"lyric charset %d | %d", ulRequiredConvert, cCodePage);

            CStringA szBufA;
            while(ctxt.ReadString(szBufA))
            {
                szBuff = svptool.AnsiToCString(cCodePage, szBufA.GetBuffer(), szBufA.GetLength());
                szBufA.ReleaseBuffer();
                
                szContent.Append(szBuff );
                szContent.Append(_T("\n"));
            }
                    
               
        }else{
            while(ctxt.ReadString(szBuff))
            {
                szContent.Append(szBuff );
                szContent.Append(_T("\n"));
            }
        }
    }

    if(!szContent.IsEmpty()){
        //SVP_LogMsg5(L"lyric read %s %d",szContent,  szContent.GetLength());
        if(lyrics_parser.read_lyrics(szContent.GetBuffer()))
            m_has_lyric = true;
        
        szContent.ReleaseBuffer();
        if(m_has_lyric)
            return lyrics_parser.get_count();
    }

	return -1;
}

CString CLyricLib::GetCurrentLyricLineByTime(LONGLONG rt_now, int* lasting_time_in_ms)
{
    *lasting_time_in_ms = -1;
    if(m_has_lyric)
	    return lyrics_parser.find_lyric_line( double(rt_now)/1000/10000 ) ;
    else
        return L"";
}
void CLyricLib::Empty()
{
	m_lyrics.RemoveAll();
	m_has_lyric = false;
	m_sz_current_lyric_file.Empty();
	
}