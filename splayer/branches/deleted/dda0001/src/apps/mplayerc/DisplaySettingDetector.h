#pragma once
#include <windows.h>
#include <winsvc.h>
#include <stdio.h>

class CDisplaySettingDetector
{
public:
    CDisplaySettingDetector(void);
    ~CDisplaySettingDetector(void);

    char Video0Name[200];
    void init();
    int GetVideoAccelLevel();
    void SetVideoAccelLevel(int level);
private:
   bool startswith(const char * src, const char * prefix);
    bool startsiwith(const char * src, const char * prefix);
   
};
