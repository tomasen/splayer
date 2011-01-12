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
  std::vector<UILayerBlock*>::iterator it;
  for (it = m_blocks.begin(); it != m_blocks.end(); it++)
  {
    (*it)->DeleteAllLayer();
    delete (*it);
  }
  m_blocks.clear();

  m_scrollbar->DeleteAllLayer();
  delete m_scrollbar;
}

void ListBlocks::AddBlock()
{
  UILayerBlock* block = new UILayerBlock;
  UILayer* mark = new UILayer(L"\\skin\\mark.png");
  UILayer* def = new UILayer(L"\\skin\\def.png");
  UILayer* play = new UILayer(L"\\skin\\play.png", FALSE);

  block->AddUILayer(L"mark", mark);
  block->AddUILayer(L"def", def);
  block->AddUILayer(L"play", play);

  CalcLayer(m_tailblockpt, mark, def, play);

  if (m_headblockpt.x == 0 && m_headblockpt.y == 0)
    m_headblockpt = m_tailblockpt;

  CalcBreakline(m_tailblockpt);

  if (m_tailblockpt.y + (m_blockrc.bottom - m_blockrc.top) > m_clientrc.bottom)
    m_showscrollbar = TRUE;

  m_blocks.push_back(block);
}

void ListBlocks::CalcLayer(POINT& markpt, UILayer* mark,
                          UILayer* def, UILayer* play)
{
  POINT play_fixpt = {40, 70};
  POINT def_fixpt = {5, 5};

  mark->SetTexturePos(markpt);

  POINT defpt = {def_fixpt.x+markpt.x, def_fixpt.y+markpt.y};
  def->SetTexturePos(defpt);

  POINT playpt = {play_fixpt.x+markpt.x, play_fixpt.y+markpt.y};
  play->SetTexturePos(playpt);
}

void ListBlocks::DoPaint(WTL::CDC& dc)
{
  POINT blockpt;
  UILayer* layer = NULL;
  std::vector<UILayerBlock*>::iterator it;

  for (it = m_blocks.begin(); it != m_blocks.end(); it++)
  {
    if ((*it)->GetUILayer(L"mark", &layer) == FALSE)
      continue;
    layer->GetTexturePos(blockpt);

    if (blockpt.x == 0 && blockpt.y == 0 || blockpt.y > m_clientrc.bottom)
      break;

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
  UILayer* layer = NULL;
  UILayer* def = NULL;
  UILayer* play = NULL;

  std::vector<UILayerBlock*>::iterator it;
  POINT blockpt = {m_margin.left, m_margin.top};

  for (it = m_blocks.begin(); it != m_blocks.end(); it++)
  {
    (*it)->GetUILayer(L"mark", &layer);
    (*it)->GetUILayer(L"def", &def);
    (*it)->GetUILayer(L"play", &play);

    CalcLayer(blockpt, layer, def, play);
    if (*it == m_blocks.front())
      m_headblockpt = blockpt;

    CalcBreakline(blockpt);

    if (*it == m_blocks.back())
      m_tailblockpt = blockpt;
  }

  m_showscrollbar = (m_tailblockpt.y + (m_blockrc.bottom - m_blockrc.top) > m_clientrc.bottom) ?
                        TRUE : FALSE;

  POINT pos = {(m_clientrc.right-m_clientrc.left) - m_scrollbarright, (m_clientrc.bottom-m_clientrc.top)/3};
  UpdateScrollBar(pos);
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

BOOL ListBlocks::SelectEffect(POINT& curr, RECT& updaterc)
{
  POINT selectblock;
  UILayer* mark = NULL;
  UILayer* play = NULL;
  BOOL ret = FALSE;

  if (m_selectblock == NULL)
  {
    std::vector<UILayerBlock*>::iterator it;
    for (it = m_blocks.begin(); it != m_blocks.end(); it++)
    {
      (*it)->GetUILayer(L"mark", &mark);
      mark->GetTexturePos(selectblock);
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
  else
  {
    m_selectblock->GetUILayer(L"mark", &mark);
    mark->GetTexturePos(selectblock);
    if (IsSelectBlock(curr, selectblock, updaterc))
      ret = TRUE;
    else
    {
      m_selectblock->GetUILayer(L"play", &play);
      play->SetDisplay(FALSE);
      m_selectblock = NULL;
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

BOOL ListBlocks::ApplyScrollBarOffset()
{
  UILayer* layer = NULL;
  UILayer* play = NULL;
  UILayer* def = NULL;
  POINT blockpt;

  std::vector<UILayerBlock*>::iterator it;
  for (it = m_blocks.begin(); it != m_blocks.end(); it++)
  {
    if ((*it)->GetUILayer(L"mark", &layer) == FALSE)
      continue;

    layer->GetTexturePos(blockpt);
    blockpt.y -= m_scrollbaroffset;

    (*it)->GetUILayer(L"def", &def);
    (*it)->GetUILayer(L"play", &play);

    CalcLayer(blockpt, layer, def, play);
  }

  m_headblockpt.y -= m_scrollbaroffset;
  m_tailblockpt.y -= m_scrollbaroffset;

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

  m_scrollbaroffset = curr.y - m_startdrag.y;
  // m_scrollbaroffset < 0 滚动条向上滑动， 反之向下滑动
  if (m_scrollbaroffset < 0 && (m_headblockpt.y-m_scrollbaroffset) > m_margin.top
    || m_scrollbaroffset > 0 && (m_tailblockpt.y+(m_blockrc.bottom-m_blockrc.top)-m_scrollbaroffset)
      < (m_clientrc.bottom - m_margin.top))
    return FALSE;

  m_startdrag = curr;
  ApplyScrollBarOffset();

  POINT pt;
  UILayer* layer = NULL;
  m_scrollbar->GetUILayer(L"layer", &layer);
  layer->GetTexturePos(pt);
  pt.y += m_scrollbaroffset;
  UpdateScrollBar(pt);
  return TRUE;
}

BOOL ListBlocks::UnDragScrollBar()
{
  m_candragscrollbar = FALSE;
  
  POINT pos = {(m_clientrc.right-m_clientrc.left) - m_scrollbarright, (m_clientrc.bottom-m_clientrc.top)/3};
  UpdateScrollBar(pos);

  return TRUE;
}