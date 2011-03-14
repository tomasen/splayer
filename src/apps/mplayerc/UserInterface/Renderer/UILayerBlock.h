#pragma once

#include "UILayer.h"

class UILayerBlock
{
public:
  UILayerBlock();
  ~UILayerBlock();

  // Interface
public:
  BOOL GetUILayer(std::wstring key, UILayer** layer);
  BOOL AddUILayer(std::wstring key, UILayer* layer);
  BOOL DelUILayer(std::wstring key);
  BOOL DoPaint(WTL::CDC& dc);
  BOOL DeleteAllLayer();

private:
  std::map<std::wstring, UILayer*> m_layers;

};