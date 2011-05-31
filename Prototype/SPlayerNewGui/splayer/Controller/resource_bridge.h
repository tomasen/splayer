#ifndef RESOURCE_BRIDGE_H
#define RESOURCE_BRIDGE_H

#include "mod_inst.h"

///////////////////////////////////////////////////////////////////////////////
//
//  ResourceBridge act as the basis of cross platform, format-neutral string
//  and image resource loading mechanism. It's a "ModuleInstance" accessible
//  from anywhere within the executable module. When initialized for the
//  first time, it uses "rsc_format" class to load up the entire resource
//  file and parse it into cached map. String data will be transformed into
//  "std::wstring", and buffer will be transformed into
//  "std::vector<unsigned char>", thus no decoding is done.
//
//  Function rsc_format::Parse will read in zlib compressed yaml file
//  produced by the resource compilation script. The script takes in the
//  original yaml resource (see trunk/src/apps/splayer_rsc/splayer_rsc.yaml),
//  strip comments, replace <<<filename>>> with base64 encoded data of real
//  file contents (this step is called binary streaming), then produce a new
//  yaml file, and zip it.
//
//  This process depends on yaml-cpp. Parsing performance is similar to XML
//  parsing with rapidxml (feels alike). Note that yaml-cpp runs fairly slow
//  under Debug build, therefore program startup is slower in Debug build.
//
//  ResourceBridge has built-in support for locale. Multilingual resources
//  can be put into a single yaml file, tagged with @locale in node names
//  (see splayer_rsc.yaml). By calling "SetLocale" first, "LoadString" and
//  "LoadBuffer" will attempt to match a locale preferred resource, then
//  fallback to nodes without a locale tag.
//
class ResourceBridge:
  public ModuleInstanceImpl<ResourceBridge>
{
public:
  ResourceBridge(void);

  // set locale to |locale|
  // affecting future LoadString and LoadBuffer call
  void SetLocale(const wchar_t* locale);
  // get locale
  std::wstring GetLocale();
  // load up string in given path
  std::wstring LoadString(const wchar_t* path);
  // load up buffer in given path
  std::vector<unsigned char> LoadBuffer(const wchar_t* path);

private:
  // locale-supported path finding, will locate "path@m_locale" first
  std::wstring GetRealPath(const wchar_t* path);

  std::wstring  m_rsc_basename;
  std::wstring  m_locale;

  std::map<std::wstring, std::wstring>                m_strings;
  std::map<std::wstring, std::vector<unsigned char> > m_buffers;
};

#endif // RESOURCE_BRIDGE_H