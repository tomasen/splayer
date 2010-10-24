#ifndef THEMEPKGCONTROLLER_H
#define THEMEPKGCONTROLLER_H

#include "../Model/ThemePkg.h"
#include <sinet.h>

class ThemePkgController
{
public:
  void DownloadThemePkg();
  bool IsThemePkgDownloadComplete();
  std::vector<ThemePkg> GetDownloadedPkgs();

private:
  std::vector<ThemePkg> m_downloaded_pkgs;
  sinet::refptr<sinet::pool>  m_pool;
};

#endif // THEMEPKGCONTROLLER_H