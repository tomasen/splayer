#include "stdafx.h"
#include "ContentType.h"
#include "Strings.h"
#include <atlutil.h>
#include <atlrx.h>

typedef CAtlRegExp<CAtlRECharTraits> CAtlRegExpT;
typedef CAtlREMatchContext<CAtlRECharTraits> CAtlREMatchContextT;

class ContentTypeTemp
{
public:
  static bool FindRedir(CUrl& src, CString ct, CString& body, CAtlList<CString>& urls, CAutoPtrList<CAtlRegExpT>& res)
  {
    POSITION pos = res.GetHeadPosition();
    while(pos)
    {
      CAtlRegExpT* re = res.GetNext(pos);

      CAtlREMatchContextT mc;
      const CAtlREMatchContextT::RECHAR* s = (LPCTSTR)body;
      const CAtlREMatchContextT::RECHAR* e = NULL;
      for(; s && re->Match(s, &mc, &e); s = e)
      {
        const CAtlREMatchContextT::RECHAR* szStart = 0;
        const CAtlREMatchContextT::RECHAR* szEnd = 0;
        mc.GetMatch(0, &szStart, &szEnd);

        CString url;
        url.Format(_T("%.*s"), szEnd - szStart, szStart);
        url.Trim();

        if(url.CompareNoCase(_T("asf path")) == 0) continue;

        CUrl dst;
        dst.CrackUrl(CString(url));
        if(_tcsicmp(src.GetSchemeName(), dst.GetSchemeName())
          || _tcsicmp(src.GetHostName(), dst.GetHostName())
          || _tcsicmp(src.GetUrlPath(), dst.GetUrlPath()))
        {
          urls.AddTail(url);
        }
        else
        {
          // recursive
          urls.RemoveAll();
          break;
        }
      }
    }

    return urls.GetCount() > 0;
  }

  static bool FindRedir(CString& fn, CString ct, CAtlList<CString>& fns, CAutoPtrList<CAtlRegExpT>& res)
  {
    CString body;

    //   CTextFile f(CTextFile::ANSI);
    //   if(f.Open(fn)) for(CString tmp; f.ReadString(tmp); body += tmp + '\n');

    CString dir = fn.Left(max(fn.ReverseFind('/'), fn.ReverseFind('\\'))+1); // "ReverseFindOneOf"

    POSITION pos = res.GetHeadPosition();
    while(pos)
    {
      CAtlRegExpT* re = res.GetNext(pos);

      CAtlREMatchContextT mc;
      const CAtlREMatchContextT::RECHAR* s = (LPCTSTR)body;
      const CAtlREMatchContextT::RECHAR* e = NULL;
      for(; s && re->Match(s, &mc, &e); s = e)
      {
        const CAtlREMatchContextT::RECHAR* szStart = 0;
        const CAtlREMatchContextT::RECHAR* szEnd = 0;
        mc.GetMatch(0, &szStart, &szEnd);

        CString fn2;
        fn2.Format(_T("%.*s"), szEnd - szStart, szStart);
        fn2.Trim();

        if(!fn2.CompareNoCase(_T("asf path"))) continue;
        if(fn2.Find(_T("EXTM3U")) == 0 || fn2.Find(_T("#EXTINF")) == 0) continue;

        if(fn2.Find(_T(":")) < 0 && fn2.Find(_T("\\\\")) != 0 && fn2.Find(_T("//")) != 0)
        {
          CPath p;
          p.Combine(dir, fn2);
          fn2 = (LPCTSTR)p;
        }

        if(!fn2.CompareNoCase(fn))
          continue;

        fns.AddTail(fn2);
      }
    }

    return fns.GetCount() > 0;
  }

};

std::wstring ContentType::Get(const wchar_t* fn, std::vector<std::wstring>* redir /* = NULL */)
{
	CUrl url;
  std::wstring ct, body;

  if (wcsstr(fn, L"://") >= 0)
	{
		url.CrackUrl(fn);

		if(_tcsicmp(url.GetSchemeName(), _T("pnm")) == 0)
			return L"audio/x-pn-realaudio";

		if(_tcsicmp(url.GetSchemeName(), _T("mms")) == 0)
			return L"video/x-ms-asf";
		
		if(_tcsicmp(url.GetSchemeName(), _T("rtsp")) == 0)
			return L"application/vnd.rn-realvideo";

		if(_tcsicmp(url.GetSchemeName(), _T("http")) != 0)
			return L"";

// 		DWORD ProxyEnable = 0;
// 		CString ProxyServer;
// 		DWORD ProxyPort = 0;
// 
// 		ULONG len = 256+1;
// 		CRegKey key;
// 		if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), KEY_READ)
// 		&& ERROR_SUCCESS == key.QueryDWORDValue(_T("ProxyEnable"), ProxyEnable) && ProxyEnable
// 		&& ERROR_SUCCESS == key.QueryStringValue(_T("ProxyServer"), ProxyServer.GetBufferSetLength(256), &len))
// 		{
// 			ProxyServer.ReleaseBufferSetLength(len);
// 
// 			CAtlList<CString> sl;
// 			ProxyServer = Explode(ProxyServer, sl, ';');
// 			if(sl.GetCount() > 1)
// 			{
// 				POSITION pos = sl.GetHeadPosition();
// 				while(pos)
// 				{
// 					CAtlList<CString> sl2;
// 					if(!Explode(sl.GetNext(pos), sl2, '=', 2).CompareNoCase(_T("http"))
// 					&& sl2.GetCount() == 2)
// 					{
// 						ProxyServer = sl2.GetTail();
// 						break;
// 					}
// 				}
// 			}
// 
// 			ProxyServer = Explode(ProxyServer, sl, ':');
// 			if(sl.GetCount() > 1) ProxyPort = _tcstol(sl.GetTail(), NULL, 10);
// 		}

// 		CSocket s;
// 		s.Create();
// 		CString szHostName = url.GetHostName();
// 		UINT iPort = url.GetPortNumber();
// 		if(s.Connect(
// 			ProxyEnable ? ProxyServer  : szHostName, 
// 			ProxyEnable ? ProxyPort : iPort))
// 		{
// 			CStringA host = CStringA(url.GetHostName());
// 			CStringA path = CStringA(url.GetUrlPath()) + CStringA(url.GetExtraInfo());
// 
// 			if(ProxyEnable) path = "http://" + host + path;
// 
// 			CStringA hdr;
// 			hdr.Format(
// 				"GET %s HTTP/1.0\r\n"
// 				"User-Agent: Media Player Classic\r\n"
// 				"Host: %s\r\n"
// 				"Accept: */*\r\n"
// 				"\r\n", path, host);
// 
// // MessageBox(NULL, CString(hdr), _T("Sending..."), MB_OK);
// 
// 			if(s.Send((LPCSTR)hdr, hdr.GetLength()) < hdr.GetLength()) return "";
// 
// 			hdr.Empty();
// 			while(1)
// 			{
// 				CStringA str;
// 				str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
// 				if(str.IsEmpty()) break;
// 				hdr += str;
// 				int hdrend = hdr.Find("\r\n\r\n");
// 				if(hdrend >= 0) {body = hdr.Mid(hdrend+4); hdr = hdr.Left(hdrend); break;}
// 			}

// MessageBox(NULL, CString(hdr), _T("Received..."), MB_OK);

// 			CAtlList<CStringA> sl;
// 			Explode(hdr, sl, '\n');
// 			POSITION pos = sl.GetHeadPosition();
// 			while(pos)
// 			{
// 				CStringA& hdrline = sl.GetNext(pos);
// 				CAtlList<CStringA> sl2;
// 				Explode(hdrline, sl2, ':', 2);
// 				CStringA field = sl2.RemoveHead().MakeLower();
// 				if(field == "location" && !sl2.IsEmpty())
// 					return GetContentType(CString(sl2.GetHead()), redir);
// 				if(field == "content-type" && !sl2.IsEmpty())
// 					ct = sl2.GetHead();
// 			}
// 
// 			while(body.GetLength() < 256)
// 			{
// 				CStringA str;
// 				str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
// 				if(str.IsEmpty()) break;
// 				body += str;
// 			}
// 
// 			if(body.GetLength() >= 8)
// 			{
// 				CStringA str = TToA(body);
// 				if(!strncmp((LPCSTR)str, ".ra", 3))
// 					return "audio/x-pn-realaudio";
// 				if(!strncmp((LPCSTR)str, ".RMF", 4))
// 					return "audio/x-pn-realaudio";
// 				if(*(DWORD*)(LPCSTR)str == 0x75b22630)
// 					return "video/x-ms-wmv";
// 				if(!strncmp((LPCSTR)str+4, "moov", 4))
// 					return "video/quicktime";
// 			}
// 
// 			if(redir && (ct == _T("audio/x-scpls") || ct == _T("audio/x-mpegurl")))
// 			{
// 				while(body.GetLength() < 4*1024) // should be enough for a playlist...
// 				{
// 					CStringA str;
// 					str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
// 					if(str.IsEmpty()) break;
// 					body += str;
// 				}
// 			}
// 		}
	}
	else if (lstrlen(fn) > 0)
	{
    ATL::CPath p(fn);
    std::wstring ext = p.GetExtension().MakeLower();
		if (ext == _T(".asx"))
      ct = _T("video/x-ms-asf");
		else if (ext == _T(".pls"))
      ct = _T("audio/x-scpls");
		else if (ext == _T(".m3u"))
      ct = _T("audio/x-mpegurl");
		else if (ext == _T(".qtl"))
      ct = _T("application/x-quicktimeplayer");
		else if (ext == _T(".mpcpl"))
      ct = _T("application/x-mpc-playlist");
		else if (ext == _T(".cue"))
      ct = _T("application/x-cue-playlist");
    else if (ext == _T(".bdmv"))
      ct = _T("application/x-bdmv-playlist");

    FILE* file = NULL;
    if (_wfopen_s(&file, fn, L"rb") == 0)
    {
      fseek(file, 0, SEEK_END);
      int file_size = ftell(file);
      if (file_size > 0)
      {
        rewind(file);
        std::vector<unsigned char> buffer(file_size);
        if (fread(&buffer[0], 1, file_size, file) == file_size)
        {
          buffer.push_back(0);
          body = Strings::StringToWString(std::string((char*)&buffer[0]));
        }
      }
      fclose(file);
    }
	}

	if (body.length() >= 4) // here only those which cannot be opened through dshow
	{
		if (!wcsncmp(body.c_str(), L".ra", 3))
			return L"audio/x-pn-realaudio";
		if (!wcsncmp(body.c_str(), L"FWS", 3))
			return L"application/x-shockwave-flash";
	}

	if (redir && !ct.empty())
	{
		CAutoPtrList<CAtlRegExpT> res;
		CAutoPtr<CAtlRegExpT> re;

		if(ct == _T("video/x-ms-asf"))
		{
			// ...://..."/>
			re.Attach(new CAtlRegExpT());
			if(re && REPARSE_ERROR_OK == re->Parse(_T("{[a-zA-Z]+://[^\n\">]*}"), FALSE))
				res.AddTail(re);
			// Ref#n= ...://...\n
			re.Attach(new CAtlRegExpT());
			if(re && REPARSE_ERROR_OK == re->Parse(_T("Ref\\z\\b*=\\b*[\"]*{([a-zA-Z]+://[^\n\"]+}"), FALSE))
				res.AddTail(re);
		}
		else if(ct == _T("audio/x-scpls"))
		{
			// File1=...\n
			re.Attach(new CAtlRegExp<>());
			if(re && REPARSE_ERROR_OK == re->Parse(_T("file\\z\\b*=\\b*[\"]*{[^\n\"]+}"), FALSE))
				res.AddTail(re);
		}
		else if(ct == _T("audio/x-mpegurl"))
		{
			// #comment
			// ...
			re.Attach(new CAtlRegExp<>());
			if(re && REPARSE_ERROR_OK == re->Parse(_T("{[^#][^\n]+}"), FALSE))
				res.AddTail(re);
		}
		else if(ct == _T("audio/x-pn-realaudio"))
		{
			// rtsp://...
			re.Attach(new CAtlRegExp<>());
			if(re && REPARSE_ERROR_OK == re->Parse(_T("{rtsp://[^\n]+}"), FALSE))
				res.AddTail(re);
		}

		if (!body.empty())
		{
// 			if (wcsstr(fn, L"://") >= 0)
//         FindRedir(url, ct, body, *redir, res);
// 			else
//         FindRedir(fn, ct, *redir, res);
		}
	}

	return ct;
}
