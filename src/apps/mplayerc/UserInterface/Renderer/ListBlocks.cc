#include "stdafx.h"
#include "ListBlocks.h"

ListBlocks::ListBlocks():
m_selectblock(NULL),
m_scrollbar(NULL),
m_showscrollbar(FALSE),
m_candragscrollbar(FALSE)
{
  m_headblockpt.x = 0;
  m_headblockpt.y = 0;

  m_tailblockpt.x = 0;
  m_tailblockpt.y = 0;

  AddScrollBar();
}

ListBlocks::~ListBlocks()
{
  std::list<UILayerBlock*>::iterator it;
  for (it = m_blocks.begin(); it != m_blocks.end(); it++)
  {
    (*it)->DeleteAllLayer();
    delete (*it);
  }
  m_blocks.clear();

  m_scrollbar->DeleteAllLayer();
  delete m_scrollbar;
}

void ListBlocks::AddBlock(RECT& updaterect)
{
  UILayerBlock* block = new UILayerBlock;

  block->AddUILayer(L"mark", new UILayer(L"\\skin\\mark.png"));
  block->AddUILayer(L"def", new UILayer(L"\\skin\\def.png"));
  block->AddUILayer(L"play", new UILayer(L"\\skin\\play.png", FALSE));
  block->AddUILayer(L"del", new UILayer(L"\\skin\\del.png", FALSE));

  CalcLayer(m_tailblockpt, block);

  if (m_headblockpt.x == 0 && m_headblockpt.y == 0)
    m_headblockpt = m_tailblockpt;

  if (m_tailblockpt.y + (m_blockrc.bottom - m_blockrc.top) > m_clientrc.bottom)
    m_showscrollbar = TRUE;

  updaterect.left = m_tailblockpt.x;
  updaterect.top = m_tailblockpt.y;
  updaterect.right = updaterect.left + (m_blockrc.right - m_blockrc.left);
  updaterect.bottom = updaterect.top + (m_blockrc.bottom - m_blockrc.top) + m_margin.bottom;

  CalcBreakline(m_tailblockpt);
  m_blocks.push_back(block);
}

void ListBlocks::DelBlock(UILayerBlock* block)
{
  POINT pt = {0, 0};
  UILayer* layer = NULL;

  std::list<UILayerBlock*>::iterator it;
  for (it = m_blocks.begin(); it != m_blocks.end(); it++)
  {
    if (*it == block)
    {
      if (it == m_blocks.begin())
      {
        pt.x = m_margin.left;
        pt.y = m_margin.top;
        it++;
        
        if (it == m_blocks.end())
          break;

        CalcLayer(pt, *it);
        continue;
      }
      else
      {
        it--;
        (*it)->GetUILayer(L"mark", &layer);
        layer->GetTexturePos(pt);
        it++;
      }
      continue;
    }
    
    if (pt.x != 0 && pt.y != 0)
    {
      CalcBreakline(pt);
      CalcLayer(pt, *it);
    }
  }

  m_showscrollbar = (pt.y + (m_blockrc.bottom - m_blockrc.top) > m_clientrc.bottom) ?
                    TRUE : FALSE;
  m_selectblock = NULL;
  m_blocks.remove(block);
  delete block;

  if (m_blocks.empty())
  {
    m_tailblockpt.x = m_margin.left;
    m_tailblockpt.y = m_margin.top;
  }
  else
  {
    CalcBreakline(pt);
    m_tailblockpt = pt;
  }
}

void ListBlocks::CalcLayer(POINT& layerpt, UILayerBlock* block)
{
  UILayer* layer = NULL;
  UILayer* def = NULL;
  UILayer* play = NULL;
  UILayer* del = NULL;

  block->GetUILayer(L"mark", &layer);
  block->GetUILayer(L"def", &def);
  block->GetUILayer(L"play", &play);
  block->GetUILayer(L"del", &del);

  POINT play_fixpt = {40, 70};
  POINT def_fixpt = {5, 5};
  POINT del_fixpt = {95, 8};

  layer->SetTexturePos(layerpt);

  POINT defpt = {def_fixpt.x+layerpt.x, def_fixpt.y+layerpt.y};
  def->SetTexturePos(defpt);

  POINT playpt = {play_fixpt.x+layerpt.x, play_fixpt.y+layerpt.y};
  play->SetTexturePos(playpt);

  POINT delpt = {del_fixpt.x+layerpt.x, del_fixpt.y+layerpt.y};
  del->SetTexturePos(delpt);
}

void ListBlocks::DoPaint(WTL::CDC& dc)
{
  POINT blockpt;
  UILayer* layer = NULL;
  std::list<UILayerBlock*>::iterator it;

  RECT rc;
  std::wstring str;
  
  dc.SetBkMode(TRANSPARENT);

  for (it = m_blocks.begin(); it != m_blocks.end(); it++)
  {
    if ((*it)->GetUILayer(L"mark", &layer) == FALSE)
      continue;
    layer->GetTexturePos(blockpt);

    if (blockpt.x == 0 && blockpt.y == 0 || blockpt.y > m_clientrc.bottom)
      break;

    str = L"block_";
    rc.left = blockpt.x;
    rc.top = blockpt.y + (m_blockrc.bottom-m_blockrc.top);
    rc.right = blockpt.x + (m_blockrc.right-m_blockrc.left);
    rc.bottom = blockpt.y+160;
    wchar_t addr[10];
    wsprintf(addr, L"%d\n", *it);
    str += addr;
    dc.DrawText(str.c_str(), str.size(), &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_END_ELLIPSIS);

    (*it)->DoPaint(dc);
  }

  if (m_showscrollbar)
    m_scrollbar->DoPaint(dc);
}

void ListBlocks::SetOption(RECT& blockrc, RECT& margin, int scrollbar_right)
{
  m_margin = margin;
  m_blockrc = blockrc;
  m_scrollbarright = scrollbar_right;
  m_tailblockpt.x = m_margin.left;
  m_tailblockpt.y = m_margin.top;
  m_scrollbaroffset = m_margin.top;
}

void ListBlocks::SetClientrc(RECT& clientrc)
{
  m_clientrc = clientrc;
}

BOOL ListBlocks::CalcBreakline(POINT& blockpt)
{
  int clientwidth = m_clientrc.right - m_clientrc.left;
  int clientheight = m_clientrc.bottom - m_clientrc.top;
  int blockwidth = m_blockrc.right - m_blockrc.left;
  int blockheight = m_blockrc.bottom - m_blockrc.top;

  int blockoffset = m_margin.right + blockwidth + m_margin.left;
  blockpt.x += blockoffset;

  if (blockpt.x + blockwidth+m_scrollbarright > clientwidth)
  { // breakline
    blockpt.x = m_margin.left;
    blockpt.y = blockpt.y + blockheight + m_margin.bottom;
    return TRUE;
  }

  return FALSE;
}

void ListBlocks::AutoBreakline()
{
  std::list<UILayerBlock*>::iterator it;
  POINT blockpt = {m_margin.left, m_scrollbaroffset};

  for (it = m_blocks.begin(); it != m_blocks.end(); it++)
  {
    CalcLayer(blockpt, *it);
    if (*it == m_blocks.front())
      m_headblockpt = blockpt;

    CalcBreakline(blockpt);

    if (*it == m_blocks.back())
      m_tailblockpt = blockpt;
  }

  if (m_scrollbaroffset < 0)
    m_showscrollbar = TRUE;
  else
  m_showscrollbar = (m_tailblockpt.y + (m_blockrc.bottom - m_blockrc.top) > m_clientrc.bottom) ?
                        TRUE : FALSE;

  POINT pos = {(m_clientrc.right-m_clientrc.left) - m_scrollbarright, (m_clientrc.bottom-m_clientrc.top)/3};
  UpdateScrollBar(pos);
}

BOOL ListBlocks::SelectBlockClick(HWND hwnd)
{
  if (m_selectblock && m_selectdellayer)
    DelBlock(m_selectblock);
  else if (m_selectblock)
    MessageBox(hwnd, L"player", L"", MB_OK);

  return TRUE;
}

BOOL ListBlocks::IsSelectBlock(POINT& curr, POINT& pos, RECT& updaterc)
{
  RECT hitarea;

  hitarea.top = pos.y;
  hitarea.left = pos.x;
  hitarea.right = pos.x + (m_blockrc.right-m_blockrc.left);
  hitarea.bottom = pos.y + (m_blockrc.bottom-m_blockrc.top);
  updaterc = hitarea;

  return PtInRect(&hitarea, curr);
}

BOOL ListBlocks::SelectBlockdel(POINT& curr)
{
  RECT delrc;
  POINT delpt;
  BOOL ret = FALSE;
  UILayer* del = NULL;
  UILayer* play = NULL;

  m_selectblock->GetUILayer(L"del", &del);
  del->GetTexturePos(delpt);
  del->GetTextureRect(delrc);
  delrc.bottom += delpt.y;
  delrc.right += delpt.x;
  delrc.top = delpt.y;
  delrc.left = delpt.x;
  if (PtInRect(&delrc, curr))
  {
    m_selectblock->GetUILayer(L"play", &play);
    play->SetDisplay(FALSE);
    del->SetDisplay(TRUE);
    m_selectdellayer = del;
    ret = TRUE;
  } // end del select
  else
  {
    ret = FALSE;
    del->SetDisplay(FALSE);
    m_selectdellayer = NULL;
  }

  return ret;
}

BOOL ListBlocks::SelectBlockEffect(POINT& curr, RECT& updaterc)
{
  UILayer* layer = NULL;
  UILayer* play = NULL;

  POINT selectblock;
  BOOL ret = FALSE;
  BOOL selectdel = FALSE;

  if (m_selectblock)
  {
    // del select
    selectdel = SelectBlockdel(curr);

    // block select
    if (!selectdel)
    {
      m_selectblock->GetUILayer(L"mark", &layer);
      layer->GetTexturePos(selectblock);
      if (IsSelectBlock(curr, selectblock, updaterc))
      {
        m_selectblock->GetUILayer(L"play", &play);
        play->SetDisplay(TRUE);
        ret = TRUE;
      }
      else
      {
        m_selectblock->GetUILayer(L"play", &play);
        play->SetDisplay(FALSE);
        m_selectblock = NULL;
      }
    }
  }
  else
  { // find block
    std::list<UILayerBlock*>::iterator it;
    for (it = m_blocks.begin(); it != m_blocks.end(); it++)
    {
      (*it)->GetUILayer(L"mark", &layer);
      layer->GetTexturePos(selectblock);

      if (IsSelectBlock(curr, selectblock, updaterc))
      {
        (*it)->GetUILayer(L"play", &play);
        play->SetDisplay(TRUE);
        m_selectblock = (*it);
        ret = TRUE;
        break;
      }
    }
  }

  return ret;
}

BOOL ListBlocks::AddScrollBar()
{
  if (m_scrollbar)
    return FALSE;

  m_scrollbar = new UILayerBlock;
  UILayer* layer = new UILayer(L"\\skin\\rol.png");
  m_scrollbar->AddUILayer(L"layer", layer);

  return TRUE;
}

BOOL ListBlocks::UpdateScrollBar(POINT& pos)
{
  UILayer* layer = NULL;
  m_scrollbar->GetUILayer(L"layer", &layer);
  layer->SetTexturePos(pos);

  return TRUE;
}

BOOL ListBlocks::ApplyScrollBarOffset(int dragoffset)
{
  UILayer* layer = NULL;
  POINT blockpt;

  std::list<UILayerBlock*>::iterator it;
  for (it = m_blocks.begin(); it != m_blocks.end(); it++)
  {
    (*it)->GetUILayer(L"mark", &layer);
    layer->GetTexturePos(blockpt);
    blockpt.y -= dragoffset;

    CalcLayer(blockpt, *it);
  }

  m_headblockpt.y -= dragoffset;
  m_tailblockpt.y -= dragoffset;
  m_scrollbaroffset -= dragoffset;
  return TRUE;
}

BOOL ListBlocks::SelectScrollBar(POINT& curr, RECT& updaterc)
{
  POINT pt;
  RECT rc;
  UILayer* layer = NULL;

  m_scrollbar->GetUILayer(L"layer", &layer);
  layer->GetTexturePos(pt);
  layer->GetTextureRect(rc);

  rc.right = pt.x + (rc.right - rc.left);
  rc.bottom = pt.y + (rc.bottom - rc.top);
  rc.left = pt.x;
  rc.top = pt.y;

  m_candragscrollbar = PtInRect(&rc, curr);
  if (m_candragscrollbar)
  {
    m_startdrag = curr;
    updaterc.left = pt.x;
    updaterc.top = 0;
    updaterc.right = rc.right;
    updaterc.bottom = m_clientrc.bottom;
  }

  return TRUE;
}

BOOL ListBlocks::DragScrollBar(POINT& curr)
{
  if (m_candragscrollbar == FALSE)
    return FALSE;

  int dragoffset = curr.y - m_startdrag.y;

  // dragoffset < 0 滚动条向上滑动， 反之向下滑动
  if (dragoffset < 0 && (m_headblockpt.y-dragoffset) > m_margin.top
    || dragoffset > 0 && (m_tailblockpt.y+(m_blockrc.bottom-m_blockrc.top)-dragoffset)
      < (m_clientrc.bottom - m_margin.bottom))
    return FALSE;

  POINT pt;
  RECT rect;
  UILayer* layer = NULL;
  m_scrollbar->GetUILayer(L"layer", &layer);
  layer->GetTexturePos(pt);
  layer->GetTextureRect(rect);

  if (dragoffset < 0 && pt.y + dragoffset < m_clientrc.top
    || dragoffset > 0 && pt.y + dragoffset+(rect.bottom-rect.top) > m_clientrc.bottom)
    return FALSE;

  m_startdrag = curr;
  pt.y += dragoffset;
  UpdateScrollBar(pt);
  ApplyScrollBarOffset(dragoffset);
  return TRUE;
}

BOOL ListBlocks::UnDragScrollBar()
{
  m_candragscrollbar = FALSE;
  
  POINT pos = {(m_clientrc.right-m_clientrc.left) - m_scrollbarright, (m_clientrc.bottom-m_clientrc.top)/3};
  UpdateScrollBar(pos);

  return TRUE;
}