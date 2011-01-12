#pragma once

#include "UILayerBlock.h"
#include "UILayer.h"

class ListBlocks
{
public:
  ListBlocks();
  ~ListBlocks();

  void AddBlock();
  //void DelBlock();

  void AutoBreakline();

  void DoPaint(WTL::CDC& dc);
  void SetOption(RECT& blockrc, RECT& margin, int scrollbar_right);
  void SetClientrc(RECT& clientrc);

  BOOL CalcBreakline(POINT& blockpt);
  void CalcLayer(POINT& markpt, UILayer* mark,
    UILayer* def, UILayer* play);

  BOOL IsSelectBlock(POINT& curr, POINT& pos, RECT& updaterc);
  BOOL SelectEffect(POINT& curr, RECT& updaterc);

  BOOL AddScrollBar();
  BOOL UpdateScrollBar(POINT& pos);
  BOOL DragScrollBar(POINT& curr);
  BOOL ApplyScrollBarOffset();
  BOOL SelectScrollBar(POINT& curr, RECT& updaterc);
  BOOL UnDragScrollBar();

private:
  std::vector<UILayerBlock*> m_blocks;
  POINT m_tailblockpt;
  POINT m_headblockpt;

  UILayerBlock* m_selectblock;
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