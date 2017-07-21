// pch.h for precompiled headers
#ifndef PCH_H
#define PCH_H

#include <string>
#include <vector>
#include <map>

#ifdef WIN32

#ifndef WINVER
#define WINVER 0x0400
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif // WIN32

#endif // PCH_H