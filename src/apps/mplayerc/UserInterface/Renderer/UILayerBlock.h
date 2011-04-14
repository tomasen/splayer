#pragma once

#include "UILayer.h"

class UILayerBlock
{
public:
  UILayerBlock(void);
  UILayerBlock(std::wstring& name);
  ~UILayerBlock();

public:
  BOOL GetUILayer(std::wstring key, UILayer** layer);
  BOOL AddUILayer(std::wstring key, UILayer* layer);
  BOOL DelUILayer(std::wstring key);
  BOOL DoPaint(WTL::CDC& dc);
  BOOL DeleteAllLayer();

  void GetBlockName(std::wstring& out);

private:
  std::map<std::wstring, UILayer*> m_layers;
  std::wstring m_blockname;
};