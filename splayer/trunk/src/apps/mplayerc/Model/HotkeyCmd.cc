#include "stdafx.h"
#include "HotkeyCmd.h"
#include "../resource.h"

HotkeyCmd::HotkeyCmd(unsigned short cmd /*= 0*/)
{
  this->cmd = cmd;
}

HotkeyCmd::HotkeyCmd(unsigned short cmd, unsigned short key, unsigned char fVirt, 
                     unsigned int ids_cmd_comment,
                     unsigned int appcmd /*= 0*/, unsigned int mouse /*= NONE*/)
{
  this->cmd = cmd;
  this->key = key;
  this->fVirt = fVirt;
  this->ids_cmd_comment = ids_cmd_comment;
  this->appcmd = appcmdorg = appcmd;
  this->mouse = mouseorg = mouse;
  backup = *this;
}

bool HotkeyCmd::operator==(const HotkeyCmd& wc) const
{
  return(cmd > 0 && cmd == wc.cmd);
}

void HotkeyCmd::Restore()
{
  *(ACCEL*)this = backup;
  appcmd = appcmdorg;
  mouse = mouseorg;
}

bool HotkeyCmd::IsModified()
{
  return(memcmp((const ACCEL*)this, &backup, sizeof(ACCEL)) || 
    appcmd != appcmdorg || mouse != mouseorg);
}

std::wstring MakeAccelModLabel(BYTE fVirt)
{
  std::wstring str;
  if (fVirt&FCONTROL)
  {
    if(!str.empty())
      str += L" + ";
    str += L"Ctrl";
  }
  if (fVirt&FALT)
  {
    if(!str.empty())
      str += L" + ";
    str += L"Alt";
  }
  if (fVirt&FSHIFT)
  {
    if(!str.empty())
      str += L" + ";
    str += L"Shift";
  }
  if (str.empty())
    str = L"None";
  return str;
}

std::wstring HotkeyCmd::MakeTextLabel()
{
  if (cmd == 0)
    return L"";

  WTL::CString str;

  if (fVirt&1)
    switch(key)
  {
    case VK_LBUTTON: str = _T("LBtn"); break;
    case VK_RBUTTON: str = _T("RBtn"); break;
    case VK_CANCEL: str = _T("Cancel"); break;
    case VK_MBUTTON: str = _T("MBtn"); break;
    case VK_XBUTTON1: str = _T("X1Btn"); break;
    case VK_XBUTTON2: str = _T("X2Btn"); break;
    case VK_BACK: str = _T("Back"); break;
    case VK_TAB: str = _T("Tab"); break;
    case VK_CLEAR: str = _T("Clear"); break;
    case VK_RETURN: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_RETURN); break;
    case VK_SHIFT: str = _T("Shift"); break;
    case VK_CONTROL: str = _T("Ctrl"); break;
    case VK_MENU: str = _T("Alt"); break;
    case VK_PAUSE: str = _T("Pause"); break;
    case VK_CAPITAL: str = _T("Capital"); break;
      //	case VK_KANA: str = _T("Kana"); break;
      //	case VK_HANGEUL: str = _T("Hangeul"); break;
    case VK_HANGUL: str = _T("Hangul"); break;
    case VK_JUNJA: str = _T("Junja"); break;
    case VK_FINAL: str = _T("Final"); break;
      //	case VK_HANJA: str = _T("Hanja"); break;
    case VK_KANJI: str = _T("Kanji"); break;
    case VK_ESCAPE: str = _T("Esc"); break;
    case VK_CONVERT: str = _T("Convert"); break;
    case VK_NONCONVERT: str = _T("Non Convert"); break;
    case VK_ACCEPT: str = _T("Accept"); break;
    case VK_MODECHANGE: str = _T("Mode Change"); break;
    case VK_SPACE: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_SPACE); break;
    case VK_PRIOR: str = _T("PgUp"); break;
    case VK_NEXT: str = _T("PgDn"); break;
    case VK_END: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_END); break;
    case VK_HOME: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_HOME); break;
    case VK_LEFT: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_LEFT_ARROW); break;
    case VK_UP: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_UP_ARROW); break;
    case VK_RIGHT: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_RIGHT_ARROW); break;
    case VK_DOWN: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_DOWN_ARROW); break;
    case VK_SELECT: str = _T("Select"); break;
    case VK_PRINT: str = _T("Print"); break;
    case VK_EXECUTE: str = _T("Execute"); break;
    case VK_SNAPSHOT: str = _T("Snapshot"); break;
    case VK_INSERT: str = _T("Insert"); break;
    case VK_DELETE: str = _T("Delete"); break;
    case VK_HELP: str = _T("Help"); break;
    case VK_LWIN: str = _T("LWin"); break;
    case VK_RWIN: str = _T("RWin"); break;
    case VK_APPS: str = _T("Apps"); break;
    case VK_SLEEP: str = _T("Sleep"); break;
    case VK_NUMPAD0: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_NUMBERPAD_0); break;
    case VK_NUMPAD1: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_NUMBERPAD_1); break;
    case VK_NUMPAD2: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_NUMBERPAD_2); break;
    case VK_NUMPAD3: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_NUMBERPAD_3); break;
    case VK_NUMPAD4: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_NUMBERPAD_4); break;
    case VK_NUMPAD5: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_NUMBERPAD_5); break;
    case VK_NUMPAD6: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_NUMBERPAD_6); break;
    case VK_NUMPAD7: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_NUMBERPAD_7); break;
    case VK_NUMPAD8: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_NUMBERPAD_8); break;
    case VK_NUMPAD9: str.LoadString(IDS_ACCEL_HOTKEY_KEYNAME_NUMBERPAD_9); break;
    case VK_MULTIPLY: str = _T("Multiply"); break;
    case VK_ADD: str = _T("Add"); break;
    case VK_SEPARATOR: str = _T("Separator"); break;
    case VK_SUBTRACT: str = _T("Subtract"); break;
    case VK_DECIMAL: str = _T("Decimal"); break;
    case VK_DIVIDE: str = _T("Divide"); break;
    case VK_F1: str = _T("F1"); break;
    case VK_F2: str = _T("F2"); break;
    case VK_F3: str = _T("F3"); break;
    case VK_F4: str = _T("F4"); break;
    case VK_F5: str = _T("F5"); break;
    case VK_F6: str = _T("F6"); break;
    case VK_F7: str = _T("F7"); break;
    case VK_F8: str = _T("F8"); break;
    case VK_F9: str = _T("F9"); break;
    case VK_F10: str = _T("F10"); break;
    case VK_F11: str = _T("F11"); break;
    case VK_F12: str = _T("F12"); break;
    case VK_F13: str = _T("F13"); break;
    case VK_F14: str = _T("F14"); break;
    case VK_F15: str = _T("F15"); break;
    case VK_F16: str = _T("F16"); break;
    case VK_F17: str = _T("F17"); break;
    case VK_F18: str = _T("F18"); break;
    case VK_F19: str = _T("F19"); break;
    case VK_F20: str = _T("F20"); break;
    case VK_F21: str = _T("F21"); break;
    case VK_F22: str = _T("F22"); break;
    case VK_F23: str = _T("F23"); break;
    case VK_F24: str = _T("F24"); break;
    case VK_NUMLOCK: str = _T("Numlock"); break;
    case VK_SCROLL: str = _T("Scroll"); break;
      //	case VK_OEM_NEC_EQUAL: str = _T("OEM NEC Equal"); break;
    case VK_OEM_FJ_JISHO: str = _T("OEM FJ Jisho"); break;
    case VK_OEM_FJ_MASSHOU: str = _T("OEM FJ Msshou"); break;
    case VK_OEM_FJ_TOUROKU: str = _T("OEM FJ Touroku"); break;
    case VK_OEM_FJ_LOYA: str = _T("OEM FJ Loya"); break;
    case VK_OEM_FJ_ROYA: str = _T("OEM FJ Roya"); break;
    case VK_LSHIFT: str = _T("LShift"); break;
    case VK_RSHIFT: str = _T("RShift"); break;
    case VK_LCONTROL: str = _T("LCtrl"); break;
    case VK_RCONTROL: str = _T("RCtrl"); break;
    case VK_LMENU: str = _T("LAlt"); break;
    case VK_RMENU: str = _T("RAlt"); break;
    case VK_BROWSER_BACK: str = _T("Browser Back"); break;
    case VK_BROWSER_FORWARD: str = _T("Browser Forward"); break;
    case VK_BROWSER_REFRESH: str = _T("Browser Refresh"); break;
    case VK_BROWSER_STOP: str = _T("Browser Stop"); break;
    case VK_BROWSER_SEARCH: str = _T("Browser Search"); break;
    case VK_BROWSER_FAVORITES: str = _T("Browser Favorites"); break;
    case VK_BROWSER_HOME: str = _T("Browser Home"); break;
    case VK_VOLUME_MUTE: str = _T("Volume Mute"); break;
    case VK_VOLUME_DOWN: str = _T("Volume Down"); break;
    case VK_VOLUME_UP: str = _T("Volume Up"); break;
    case VK_MEDIA_NEXT_TRACK: str = _T("Media Next Track"); break;
    case VK_MEDIA_PREV_TRACK: str = _T("Media Prev Track"); break;
    case VK_MEDIA_STOP: str = _T("Media Stop"); break;
    case VK_MEDIA_PLAY_PAUSE: str = _T("Media Play/Pause"); break;
    case VK_LAUNCH_MAIL: str = _T("Launch Mail"); break;
    case VK_LAUNCH_MEDIA_SELECT: str = _T("Launch Media Select"); break;
    case VK_LAUNCH_APP1: str = _T("Launch App1"); break;
    case VK_LAUNCH_APP2: str = _T("Launch App2"); break;
    case VK_OEM_1: str = _T("OEM 1"); break;
    case VK_OEM_PLUS: str = _T("Plus"); break;
    case VK_OEM_COMMA: str = _T("Comma"); break;
    case VK_OEM_MINUS: str = _T("Minus"); break;
    case VK_OEM_PERIOD: str = _T("Period"); break;
    case VK_OEM_2: str = _T("OEM 2"); break;
    case VK_OEM_3: str = _T("OEM 3"); break;
    case VK_OEM_4: str = _T("OEM 4"); break;
    case VK_OEM_5: str = _T("OEM 5"); break;
    case VK_OEM_6: str = _T("OEM 6"); break;
    case VK_OEM_7: str = _T("OEM 7"); break;
    case VK_OEM_8: str = _T("OEM 8"); break;
    case VK_OEM_AX: str = _T("OEM AX"); break;
    case VK_OEM_102: str = _T("OEM 102"); break;
    case VK_ICO_HELP: str = _T("ICO Help"); break;
    case VK_ICO_00: str = _T("ICO 00"); break;
    case VK_PROCESSKEY: str = _T("Process Key"); break;
    case VK_ICO_CLEAR: str = _T("ICO Clear"); break;
    case VK_PACKET: str = _T("Packet"); break;
    case VK_OEM_RESET: str = _T("OEM Reset"); break;
    case VK_OEM_JUMP: str = _T("OEM Jump"); break;
    case VK_OEM_PA1: str = _T("OEM PA1"); break;
    case VK_OEM_PA2: str = _T("OEM PA2"); break;
    case VK_OEM_PA3: str = _T("OEM PA3"); break;
    case VK_OEM_WSCTRL: str = _T("OEM WSCtrl"); break;
    case VK_OEM_CUSEL: str = _T("OEM CUSEL"); break;
    case VK_OEM_ATTN: str = _T("OEM ATTN"); break;
    case VK_OEM_FINISH: str = _T("OEM Finish"); break;
    case VK_OEM_COPY: str = _T("OEM Copy"); break;
    case VK_OEM_AUTO: str = _T("OEM Auto"); break;
    case VK_OEM_ENLW: str = _T("OEM ENLW"); break;
    case VK_OEM_BACKTAB: str = _T("OEM Backtab"); break;
    case VK_ATTN: str = _T("ATTN"); break;
    case VK_CRSEL: str = _T("CRSEL"); break;
    case VK_EXSEL: str = _T("EXSEL"); break;
    case VK_EREOF: str = _T("EREOF"); break;
    case VK_PLAY: str = _T("Play"); break;
    case VK_ZOOM: str = _T("Zoom"); break;
    case VK_NONAME: str = _T("Noname"); break;
    case VK_PA1: str = _T("PA1"); break;
    case VK_OEM_CLEAR: str = _T("OEM Clear"); break;
    default: 
      if('0' <= key && key <= '9' || 'A' <= key && key <= 'Z')
        str.Format(_T("%c"), (TCHAR)key);
      break;
  }

  if (str.IsEmpty() || !(fVirt&1))
    str.Format(_T("%c"), (TCHAR)key);

  if (fVirt&(FCONTROL|FALT|FSHIFT))
  {
    str.Insert(0, L" + ");
    str.Insert(0, MakeAccelModLabel(fVirt).c_str());
  }

  str.Replace(_T(" + "), _T("+"));

  return (LPCTSTR)str;
}