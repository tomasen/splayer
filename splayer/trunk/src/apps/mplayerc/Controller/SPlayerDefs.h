#ifndef SPLAYERDEFS_H
#define SPLAYERDEFS_H

// int variable ids for PlayerPreference
#define INTVAR_CL_SWITCHES          1000
#define INTVAR_LOGO_AUTOSTRETCH     1001
#define INTVAR_PLAYLIST_CURRENT     1002
#define INTVAR_SHUFFLEPLAYLISTITEMS 1003
#define INTVAR_AUTOLOADAUDIO        1004
#define INTVAR_TOGGLEFULLSCRENWHENPLAYBACKSTARTED   1005

// int64 variables
#define INT64VAR_MAINWINDOW         1000

// string variables
#define STRVAR_HOTKEYSCHEME         1000

// string array variables
#define STRARRAY_PLAYLIST           1000

// custom messages
#define WM_SPLAYER_CMD      WM_USER+9901

// custom commands
#define CMD_OPEN_CURRENTPLAYLISTITEM   1
#define CMD_OPEN_PLAYLIST              2

#endif // SPLAYERDEFS_H
