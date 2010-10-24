#include "stdafx.h"
#include "ThemePkgController.h"

void ThemePkgController::DownloadThemePkg()
{
  if (!m_pool)
    m_pool = sinet::pool::create_instance();
}

bool ThemePkgController::IsThemePkgDownloadComplete()
{
  // detect download completion
  // if it's completed, use ThemePkg model to read in all downloaded theme pkgs
  return false;
}

std::vector<ThemePkg> ThemePkgController::GetDownloadedPkgs()
{
  return std::vector<ThemePkg>();
}