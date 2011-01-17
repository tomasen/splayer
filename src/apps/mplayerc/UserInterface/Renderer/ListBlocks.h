#pragma once

#include "UILayerBlock.h"
#include "UILayer.h"
#include <list>

class ListBlocks
{
public:
  ListBlocks();
  ~ListBlocks();

  void AddBlock(RECT& updaterect);
  void DelBlock(UILayerBlock* block);

  void AutoBreakline();

  void DoPaint(WTL::CDC& dc);
  void SetOption(RECT& blockrc, RECT& margin, int scrollbar_right);
  void SetClientrc(RECT& clientrc);

  BOOL CalcBreakline(POINT& blockpt);

  void CalcLayer(POINT& layerpt, UILayerBlock* block);

  BOOL IsSelectBlock(POINT& curr, POINT& pos, RECT& updaterc);
  BOOL SelectBlockEffect(POINT& curr, RECT& updaterc);
  BOOL SelectBlockdel(POINT& curr);

  BOOL AddScrollBar();
  BOOL UpdateScrollBar(POINT& pos);
  BOOL DragScrollBar(POINT& curr);
  BOOL ApplyScrollBarOffset(int offset);
  BOOL SelectScrollBar(POINT& curr, RECT& updaterc);
  BOOL UnDragScrollBar();

  BOOL SelectBlockClick(HWND hwnd);

private:
  std::list<UILayerBlock*> m_blocks;
  POINT m_tailblockpt;
  POINT m_headblockpt;

  UILayerBlock* m_selectblock;
  UILayer* m_selectdellayer;
  UILayerBlock*  m_scrollbar;

  RECT m_margin;
  RECT m_clientrc;
  RECT m_blockrc;

  int m_scrollbarright;
  int m_scrollbaroffset;
  BOOL m_candragscrollbar;
  BOOL m_showscrollbar;

  POINT m_startdrag;
};