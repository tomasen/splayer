#pragma once

#include <threadhelper.h>

// The spider base class, abstract
class MediaSpiderAbstract :
  public ThreadHelperImpl<MediaSpiderAbstract>
{
protected:
  void _Thread();
};