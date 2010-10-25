#ifndef THEMEPKGCONTROLLER_H
#define THEMEPKGCONTROLLER_H

#include "../Model/ThemePkg.h"

class ThemePkgController
{
public:
  void ApplyThemePkg(std::wstring input_pkg_filename);
  void RevertToDefault();
};

#endif // THEMEPKGCONTROLLER_H