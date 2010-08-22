#include "stdafx.h"
#include "HotkeySchemeParser.h"
#include "../resource.h"

void HotkeySchemeParser::PopulateDefaultScheme()
{
  // This will populate |m_list| with hard coded default hotkey scheme settings.
  // At the time this was written, it's copied directly from the original hotkey definition
  // in CMPlayerCApp::Settings::UpdateData of mplayerc.cpp

  m_schemename = L"SPlayer";

#define ADDCMD(cmd) m_list.push_back(HotkeyCmd##cmd)

  ADDCMD((ID_BOSS, VK_OEM_3, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PLAY_PLAYPAUSE, VK_SPACE, FVIRTKEY|FNOINVERT, APPCOMMAND_MEDIA_PLAY_PAUSE, HotkeyCmd::LDOWN));
  ADDCMD((ID_PLAY_SEEKFORWARDMED, VK_RIGHT, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_PLAY_SEEKBACKWARDMED, VK_LEFT, FVIRTKEY|FNOINVERT));

  ADDCMD((ID_FILE_OPENQUICK, 'Q', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_FILE_OPENURLSTREAM, 'U', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_FILE_OPENMEDIA, 'O', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_FILE_OPENFOLDER, 'F', FVIRTKEY|FCONTROL|FNOINVERT));

  ADDCMD((ID_SUBMOVEUP,  VK_OEM_4 /* [ */, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_SUBMOVEDOWN,  VK_OEM_6  /* ] */, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_SUB2MOVEUP,  VK_OEM_4, FVIRTKEY|FALT|FCONTROL|FNOINVERT));
  ADDCMD((ID_SUB2MOVEDOWN,  VK_OEM_6, FVIRTKEY|FALT|FCONTROL|FNOINVERT));
  ADDCMD((ID_SUBFONTDOWNBOTH,  VK_F1, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_SUBFONTUPBOTH,  VK_F2, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_SUB1FONTDOWN,  VK_F3, FVIRTKEY|FSHIFT|FNOINVERT));
  ADDCMD((ID_SUB1FONTUP,  VK_F4, FVIRTKEY|FSHIFT|FNOINVERT));
  ADDCMD((ID_SUB2FONTDOWN,  VK_F5, FVIRTKEY|FSHIFT|FNOINVERT));
  ADDCMD((ID_SUB2FONTUP,  VK_F6, FVIRTKEY|FSHIFT|FNOINVERT));

  ADDCMD((ID_SUB_DELAY_DOWN, VK_F1, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_SUB_DELAY_UP, VK_F2,   FVIRTKEY|FNOINVERT));
  ADDCMD((ID_SUB_DELAY_DOWN2, VK_F1, FVIRTKEY|FSHIFT|FNOINVERT));
  ADDCMD((ID_SUB_DELAY_UP2, VK_F2,   FVIRTKEY|FSHIFT|FNOINVERT));

  ADDCMD((ID_SHOW_VIDEO_STAT_OSD,  VK_TAB, FVIRTKEY|FNOINVERT));

  ADDCMD((ID_BRIGHTINC, VK_HOME, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_BRIGHTDEC, VK_END, FVIRTKEY|FALT|FNOINVERT));

  ADDCMD((ID_ABCONTROL_TOGGLE,  VK_F7, FVIRTKEY|FSHIFT|FNOINVERT));
  ADDCMD((ID_ABCONTROL_SETA,  VK_F8, FVIRTKEY|FSHIFT|FNOINVERT));
  ADDCMD((ID_ABCONTROL_SETB,  VK_F9, FVIRTKEY|FSHIFT|FNOINVERT));

  ADDCMD((ID_FILE_OPENDVD, 'D', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_FILE_OPENBDVD, 'B', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_FILE_OPENDEVICE, 'V', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_FILE_SAVE_COPY, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_FILE_SAVE_IMAGE, 'I', FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_FILE_SAVE_IMAGE_AUTO, VK_F5, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_FILE_LOAD_SUBTITLE, 'L', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_FILE_SAVE_SUBTITLE, 'S', FVIRTKEY|FCONTROL|FNOINVERT));

  ADDCMD((ID_FILE_COPYTOCLIPBOARD, 'C', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_FILE_PROPERTIES, VK_F10, FVIRTKEY|FSHIFT|FNOINVERT));
  ADDCMD((ID_FILE_EXIT, 'X', FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_TOGGLE_SUBTITLE, 'H', FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_VF_FROMINSIDE, 'C', FVIRTKEY|FNOINVERT));
  ADDCMD((ID_PLAY_PLAY, 0, FVIRTKEY|FNOINVERT,APPCOMMAND_MEDIA_PLAY));
  ADDCMD((ID_PLAY_PAUSE, 0, FVIRTKEY|FNOINVERT,APPCOMMAND_MEDIA_PAUSE));
  ADDCMD((ID_PLAY_MANUAL_STOP, VK_OEM_PERIOD, FVIRTKEY|FNOINVERT, APPCOMMAND_MEDIA_STOP));
  ADDCMD((ID_PLAY_FRAMESTEP, VK_RIGHT, FVIRTKEY|FCONTROL|FALT|FNOINVERT));
  ADDCMD((ID_PLAY_FRAMESTEPCANCEL, VK_LEFT, FVIRTKEY|FCONTROL|FALT|FNOINVERT));
  ADDCMD((ID_PLAY_INCRATE, VK_UP, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PLAY_DECRATE, VK_DOWN, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_VIEW_FULLSCREEN, VK_RETURN, FVIRTKEY|FALT|FNOINVERT, 0, HotkeyCmd::LDBLCLK));
  ADDCMD((ID_VIEW_FULLSCREEN, VK_RETURN, FVIRTKEY|FNOINVERT, 0, HotkeyCmd::MUP));
  ADDCMD((ID_PLAY_INCAUDDELAY, VK_ADD, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_PLAY_DECAUDDELAY, VK_SUBTRACT, FVIRTKEY|FNOINVERT));

  ADDCMD((ID_VIEW_PLAYLIST, 'P', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_VIEW_PLAYLIST, '7', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PLAY_RESETRATE, 'R', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PLAY_SEEKFORWARDSMALL, 0, FVIRTKEY|FNOINVERT,APPCOMMAND_MEDIA_FAST_FORWARD));
  ADDCMD((ID_PLAY_SEEKBACKWARDSMALL, 0, FVIRTKEY|FNOINVERT,APPCOMMAND_MEDIA_REWIND));
  ADDCMD((ID_PLAY_SEEKFORWARDMED, VK_RIGHT, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PLAY_SEEKBACKWARDMED, VK_LEFT, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PLAY_SEEKFORWARDLARGE, VK_RIGHT, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_PLAY_SEEKBACKWARDLARGE, VK_LEFT, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_PLAY_SEEKKEYFORWARD, VK_RIGHT, FVIRTKEY|FSHIFT|FNOINVERT));
  ADDCMD((ID_PLAY_SEEKKEYBACKWARD, VK_LEFT, FVIRTKEY|FSHIFT|FNOINVERT));
  ADDCMD((ID_NAVIGATE_SKIPFORWARD, VK_NEXT, FVIRTKEY|FNOINVERT, APPCOMMAND_MEDIA_NEXTTRACK, HotkeyCmd::X2DOWN));
  ADDCMD((ID_NAVIGATE_SKIPBACK, VK_PRIOR, FVIRTKEY|FNOINVERT, APPCOMMAND_MEDIA_PREVIOUSTRACK, HotkeyCmd::X1DOWN));
  ADDCMD((ID_NAVIGATE_SKIPFORWARDPLITEM, VK_NEXT, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_NAVIGATE_SKIPBACKPLITEM, VK_PRIOR, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_VIEW_CAPTIONMENU, '0', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_VIEW_SEEKER, '1', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_VIEW_CONTROLS, '2', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_VIEW_INFORMATION, '3', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_VIEW_STATISTICS, '4', FVIRTKEY|FCONTROL|FNOINVERT));

  ADDCMD((ID_SHOWTRANSPRANTBAR, '5', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_VIEW_SUBRESYNC, '6', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_VIEW_CAPTURE, '8', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_VIEW_SHADEREDITOR, '9', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_VIEW_PRESETS_MINIMAL, '1', FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_PRESETS_COMPACT, '2', FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_PRESETS_NORMAL, '3', FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_FULLSCREEN_SECONDARY, VK_F11, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_ZOOM_50, '1', FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_VIEW_ZOOM_100, '2', FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_VIEW_ZOOM_200, '3', FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_VIEW_ZOOM_AUTOFIT, '4', FVIRTKEY|FALT|FNOINVERT));	
  ADDCMD((ID_ASPECTRATIO_NEXT, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_VF_HALF, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_VF_NORMAL, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_VF_DOUBLE, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_VF_STRETCH, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_VF_FROMINSIDE, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_VF_FROMOUTSIDE, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_SWITCH_AUDIO_DEVICE, 'A', FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_ONTOP_ALWAYS, 'T', FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PLAY_GOTO, 'G', FVIRTKEY|FCONTROL|FNOINVERT));

  ADDCMD((ID_VIEW_RESET, VK_NUMPAD5, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_INCSIZE, VK_NUMPAD9, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_INCWIDTH, VK_NUMPAD6, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_INCHEIGHT, VK_NUMPAD8, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_DECSIZE, VK_NUMPAD1, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_DECWIDTH, VK_NUMPAD4, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VIEW_DECHEIGHT, VK_NUMPAD2, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_PANSCAN_CENTER, VK_NUMPAD5, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PANSCAN_MOVELEFT, VK_NUMPAD4, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PANSCAN_MOVERIGHT, VK_NUMPAD6, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PANSCAN_MOVEUP, VK_NUMPAD8, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PANSCAN_MOVEDOWN, VK_NUMPAD2, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PANSCAN_MOVEUPLEFT, VK_NUMPAD7, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PANSCAN_MOVEUPRIGHT, VK_NUMPAD9, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PANSCAN_MOVEDOWNLEFT, VK_NUMPAD1, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PANSCAN_MOVEDOWNRIGHT, VK_NUMPAD3, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_PANSCAN_ROTATEXP, VK_NUMPAD8, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_PANSCAN_ROTATEXM, VK_NUMPAD2, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_PANSCAN_ROTATEYP, VK_NUMPAD4, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_PANSCAN_ROTATEYM, VK_NUMPAD6, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_PANSCAN_ROTATEZP, VK_NUMPAD1, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_PANSCAN_ROTATEZM, VK_NUMPAD3, FVIRTKEY|FALT|FNOINVERT));
  ADDCMD((ID_VOLUME_UP, VK_UP, FVIRTKEY|FNOINVERT, APPCOMMAND_VOLUME_UP, HotkeyCmd::WUP));//APPCOMMAND_VOLUME_UP
  ADDCMD((ID_VOLUME_DOWN, VK_DOWN, FVIRTKEY|FNOINVERT, APPCOMMAND_VOLUME_DOWN, HotkeyCmd::WDOWN));//APPCOMMAND_VOLUME_DOWN
  ADDCMD((ID_VOLUME_MUTE, 'M', FVIRTKEY|FCONTROL|FNOINVERT, APPCOMMAND_VOLUME_MUTE));//, APPCOMMAND_VOLUME_MUTE

  ADDCMD((ID_NAVIGATE_TITLEMENU, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_ROOTMENU, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_SUBPICTUREMENU, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_AUDIOMENU, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_ANGLEMENU, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_CHAPTERMENU, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_MENU_LEFT, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_MENU_RIGHT, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_MENU_UP, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_MENU_DOWN, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_MENU_ACTIVATE, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_MENU_BACK, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_NAVIGATE_MENU_LEAVE, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_DVD_SUB_ONOFF, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_DVD_ANGLE_NEXT, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_DVD_ANGLE_PREV, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_DVD_AUDIO_NEXT, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_DVD_AUDIO_PREV, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_DVD_SUB_NEXT, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_DVD_SUB_PREV, 0, FVIRTKEY|FNOINVERT));

  ADDCMD((ID_MENU_PLAYER_SHORT, 0, FVIRTKEY|FNOINVERT, 0, HotkeyCmd::RUP));

  ADDCMD((ID_VIEW_OPTIONS, 'O', FVIRTKEY|FNOINVERT));
  ADDCMD((ID_STREAM_AUDIO_NEXT, 'A', FVIRTKEY|FNOINVERT));
  ADDCMD((ID_STREAM_AUDIO_PREV, 'A', FVIRTKEY|FSHIFT|FNOINVERT));
  ADDCMD((ID_STREAM_SUB_NEXT, 'S', FVIRTKEY|FNOINVERT));
  ADDCMD((ID_STREAM_SUB_PREV, 'S', FVIRTKEY|FSHIFT|FNOINVERT));
  ADDCMD((ID_STREAM_SUB_ONOFF, 'W', FVIRTKEY|FNOINVERT));
  ADDCMD((ID_SUBTITLES_SUBITEM_START+2, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_OGM_AUDIO_NEXT, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_OGM_AUDIO_PREV, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_OGM_SUB_NEXT, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_OGM_SUB_PREV, 0, FVIRTKEY|FNOINVERT));
  ADDCMD((ID_VSYNC_OFFSET_MORE, VK_UP, FVIRTKEY|FALT|FCONTROL|FSHIFT|FNOINVERT));
  ADDCMD((ID_VSYNC_OFFSET_LESS, VK_DOWN, FVIRTKEY|FALT|FCONTROL|FSHIFT|FNOINVERT));
  ADDCMD((ID_SHOWDRAWSTAT, 'J', FVIRTKEY|FCONTROL|FNOINVERT));

  ADDCMD((ID_PLAYLIST_DELETEITEM, VK_DELETE, FVIRTKEY|FNOINVERT));

  ADDCMD((ID_SEEK_TO_BEGINNING,VK_HOME, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_SEEK_TO_MIDDLE, VK_DELETE, FVIRTKEY|FCONTROL|FNOINVERT));
  ADDCMD((ID_SEEK_TO_END, VK_END, FVIRTKEY|FCONTROL|FNOINVERT));

#undef ADDCMD
}

bool HotkeySchemeParser::ReadFromFile(const wchar_t* filename)
{
  // reset
  m_list.resize(0);
  m_schemename.resize(0);

  // open file
  FILE* file = NULL;
  if (_wfopen_s(&file, filename, L"rb") != 0)
    return false;

  // validate bom
  wchar_t bom = 0;
  if (fread(&bom, sizeof(wchar_t), 1, file) != 1)
    return false;

  if (bom != 0xfeff)
    return false;

  // a single buffer for the file
  std::vector<wchar_t> buffer;
  size_t chars_read = 0;
  wchar_t pass_buf[1024];
  while (true)
  {
    chars_read = fread(pass_buf, sizeof(wchar_t), 1024, file);
    if (chars_read > 0)
    {
      buffer.resize(buffer.size() + chars_read);
      memcpy(&buffer[buffer.size() - chars_read], pass_buf, chars_read * sizeof(wchar_t));
    }
    if (chars_read < 1024)
      break;
  }

  if (buffer.size() == 0)
    return false;

  // parse buffer to lines
  std::vector<wchar_t>::iterator it_pos = buffer.begin();
  for (std::vector<wchar_t>::iterator it = buffer.begin(); it != buffer.end(); it++)
  {
    if (*it == L'\n')
    {
      std::wstring line(&(*it_pos), std::distance(it_pos, it));
      it_pos = it;

      if (line.length() == 0)
        continue;

      // scan this line for possible input values

      // check scheme name
      if (m_schemename.length() == 0)
      {
        size_t bracket_l = line.find(L'[');
        size_t bracket_r = line.find(L']');
        if (bracket_l == 0 && bracket_r > 0 && bracket_r != std::wstring::npos)
        {
          m_schemename.assign(line.c_str()+1, bracket_r-1);
          continue;
        }
      }

      // find equation mark
      size_t equal_mark = line.find(L'=');
      if (equal_mark == 0 || equal_mark == std::wstring::npos)
        continue;

      // cmd id
      std::wstring cmd_s(line.c_str(), equal_mark);
      int cmd = _wtoi(cmd_s.c_str());

      // others
      std::vector<unsigned int> params;
      size_t pos_start = equal_mark + 1;
      wchar_t* end_ptr;
      while (true)
      {
        std::wstring field;
        size_t pos_comma = line.find(L',', pos_start);
        if (pos_comma != std::wstring::npos)
        {
          field.assign(line.c_str()+pos_start, pos_comma - pos_start);
          pos_start = pos_comma+1;
        }
        else
          field.assign(line.c_str()+pos_start);

        if (params.size() < 2)
        {
          unsigned int var = (unsigned int)wcstoul(field.c_str(), &end_ptr, 16);
          params.push_back(var);
        }
        else
          params.push_back(_wtoi(field.c_str()));

        if (pos_comma == std::wstring::npos)
          break;
      }

      if (params.size() == 4)
        m_list.push_back(HotkeyCmd(cmd, params[0], params[1], params[2], params[3]));
      else if (params.size() == 2)
        m_list.push_back(HotkeyCmd(cmd, params[0], params[1]));
    }
  }

  return true;
}

bool HotkeySchemeParser::WriteToFile(const wchar_t* filename)
{
  // File format [UTF-16 encoded with BOM, line break is CRLF]
  // --------------------------------------------------------
  // [Hotkey Scheme Name]
  // 948 = 0, 0x03, 0, 8
  // 1334 = 0x23, 0x08
  // --------------------------------------------------------
  std::vector<std::wstring> linefeeds;
  wchar_t line[128];
  swprintf_s(line, 128, L"[%s]\r\n", m_schemename.c_str());
  linefeeds.push_back(line);
  
  unsigned int appcmd;
  unsigned int mouse;
  unsigned int virt;
  unsigned int key;
  for (std::vector<HotkeyCmd>::iterator it = m_list.begin(); it != m_list.end(); it++)
  {
    // note: we choose to put everything to (unsigned int) for safe conversion
    appcmd  = it->appcmd;
    mouse   = it->mouse;
    virt    = it->fVirt;
    key     = it->key;
    if (appcmd != 0 || mouse != HotkeyCmd::NONE)
      _swprintf_p(line, 128, L"%1$d = %2$#2x, %3$#2x, %4$d, %5$d\r\n", it->cmd, key, virt, 
        appcmd, mouse);
    else
      _swprintf_p(line, 128, L"%1$d = %2$#2x, %3$#2x\r\n", it->cmd, key, virt);
    linefeeds.push_back(line);
  }

  // create file
  FILE* file = NULL;
  if (_wfopen_s(&file, filename, L"wb") != 0)
    return false;

  // write bom
  wchar_t bom = 0xfeff;
  if (fwrite(&bom, sizeof(bom), 1, file) != 1)
  {
    fclose(file);
    return false;
  }

  // write list
  for (std::vector<std::wstring>::iterator it = linefeeds.begin(); it != linefeeds.end(); it++)
  {
    if (fwrite(it->c_str(), it->length() * sizeof(wchar_t), 1, file) != 1)
    {
      fclose(file);
      return false;
    }
  }

  fclose(file);
  return true;
}

std::wstring HotkeySchemeParser::GetSchemeName()
{
  return m_schemename;
}

std::vector<HotkeyCmd> HotkeySchemeParser::GetScheme()
{
  return m_list;
}