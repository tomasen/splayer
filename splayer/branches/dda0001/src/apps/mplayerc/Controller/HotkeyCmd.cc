#include "stdafx.h"
#include "HotkeyCmd.h"

HotkeyCmd::HotkeyCmd(unsigned short cmd /*= 0*/)
{
  this->cmd = cmd;
}

HotkeyCmd::HotkeyCmd(unsigned short cmd, unsigned short key, unsigned char fVirt, 
                     unsigned int appcmd /*= 0*/, unsigned int mouse /*= NONE*/)
{
  this->cmd = cmd;
  this->key = key;
  this->fVirt = fVirt;
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