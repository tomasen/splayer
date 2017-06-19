// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SINET_DYN_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SINET_DYN_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SINET_DYN_EXPORTS
#define SINET_DYN_API __declspec(dllexport)
#else
#define SINET_DYN_API __declspec(dllimport)
#endif

#define SINET_DYN_CALLBACK __stdcall
