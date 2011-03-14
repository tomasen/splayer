#include "stdafx.h"
#include "UILayerBlock.h"

UILayerBlock::UILayerBlock()
{

}

UILayerBlock::~UILayerBlock()
{

}

BOOL UILayerBlock::GetUILayer(std::wstring key, UILayer** layer)
{
  *layer = NULL;

  if (m_layers.empty())
    return FALSE;

  if (m_layers.find(key) == m_layers.end())
    return FALSE;

  *layer = m_layers[key];
  return TRUE;
}

BOOL UILayerBlock::AddUILayer(std::wstring key, UILayer* layer)
{
  if (m_layers.find(key) != m_layers.end())
    return FALSE;

  m_layers[key] = layer;
  return TRUE;
}

BOOL UILayerBlock::DelUILayer(std::wstring key)
{
  if (m_layers.find(key) == m_layers.end())
    return FALSE;

  m_layers.erase(key);
  return TRUE;
}

BOOL UILayerBlock::DoPaint(WTL::CDC& dc)
{
  if (m_layers.empty())
    return FALSE;

  std::map<std::wstring, UILayer*>::iterator it;
  for (it = m_layers.begin(); it != m_layers.end(); it++)
  {
    (it->second)->DoPaint(dc);
  }

  return TRUE;
}

BOOL UILayerBlock::DeleteAllLayer()
{
  if (m_layers.empty())
    return FALSE;

  std::map<std::wstring, UILayer*>::iterator it;
  for (it = m_layers.begin(); it != m_layers.end(); it++)
  {
    delete (it->second);
  }

  m_layers.clear();

  return TRUE;
}