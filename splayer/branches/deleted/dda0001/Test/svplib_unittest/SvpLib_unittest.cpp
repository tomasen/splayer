
#include "logging.h"
#include "common.h"
#include "gtest/gtest.h"

#include "svplib.h"

namespace 
{
    TEST(SvpLibTest, logtime) 
    {
        CString strLine;
        for (DWORD i = 0;  i</*1000*/10; i++)
        {
            strLine.Format(_T("Line %d "), i);
            SVP_LogMsg(strLine);
        }
    }

    TEST(NewLogTest, logtime) 
    {
        CStringA strLine;
        for (DWORD i = 0;  i<1000; i++)
        {
            strLine.Format("Line %d ", i);
            LOG(INFO) << strLine;
        }
    }

    TEST(NewLogTest, Chinese) 
    {
        LOG(INFO) << "ÖÐÎÄ²âÊÔblahblah";
    }

}