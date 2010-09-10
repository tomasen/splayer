#ifndef HOTKEYCMD_H
#define HOTKEYCMD_H

//////////////////////////////////////////////////////////////////////////
//
//  HotkeyCmd is the wrapper for a "command" in mplayerc, it's rewritten
//  from the original HotkeyCmd class in mplayerc.h . The parent class ACCEL
//  is a Windows-specific accelerator key structure defined in winuser.h .
//
//  Note: for quick integration, the original member syntax of the class
//  isn't changed to comply with new coding style. This has to be done
//  later.
//
class HotkeyCmd :
  public ACCEL
{
public:
  HotkeyCmd(unsigned short cmd = 0);
  HotkeyCmd(unsigned short cmd, unsigned short key, unsigned char fVirt,
            unsigned int ids_cmd_comment,
            unsigned int appcmd = 0, unsigned int mouse = NONE);
  bool operator == (const HotkeyCmd& wc) const;

  enum {  // this keywords will become usable and distinguishable as int
    NONE = 0,
    LDOWN,
    LUP,
    LDBLCLK,
    MDOWN,
    MUP,
    MDBLCLK,
    RDOWN,
    RUP,
    RDBLCLK,
    X1DOWN,
    X1UP,
    X1DBLCLK,
    X2DOWN,
    X2UP,
    X2DBLCLK,
    WUP,
    WDOWN,
    LAST
  };

  unsigned int  ids_cmd_comment;
  unsigned int  appcmd;
  unsigned int  mouse;

  void Restore();
  bool IsModified();

  std::wstring MakeTextLabel();

private:
  ACCEL         backup;
  unsigned int  appcmdorg;
  unsigned int  mouseorg;
};

#endif // HOTKEYCMD_H