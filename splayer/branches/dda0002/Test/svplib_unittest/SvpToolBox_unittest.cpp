
#include "common.h"
#include "gtest/gtest.h"
#include "svptoolbox.h"


namespace 
{
    TEST(SvpToolsBoxTest, getVideoFileBasename) 
    {
        {
            TCHAR szName[] = _T("c:\\folder\\name.ext");
            CSVPToolBox toolbox;
            CStringArray retArray;
            CString strRet = toolbox.getVideoFileBasename(szName, &retArray);
            ASSERT_TRUE(retArray[0]==_T("c:\\folder\\name"));
            ASSERT_TRUE(retArray[1]==_T(".ext"));
            ASSERT_TRUE(retArray[2]==_T("c:\\folder\\"));
            ASSERT_TRUE(retArray[3]==_T("name"));
        }
        {
            TCHAR szName[] = _T("X:\\downloads\\Everybody Loves Raymond  First season\\Everybody_Loves_Raymond_-_1x02_-_I_Love_You.(DVD).MEDiEVAL.[www.dream2008.cn].xvid.rmvb");
            CSVPToolBox toolbox;
            CStringArray retArray;
            CString strRet = toolbox.getVideoFileBasename(szName, &retArray);
            ASSERT_TRUE(retArray[0]==_T("X:\\downloads\\Everybody Loves Raymond  First season\\Everybody_Loves_Raymond_-_1x02_-_I_Love_You.(DVD).MEDiEVAL.[www.dream2008.cn].xvid"));
            ASSERT_TRUE(retArray[1]==_T(".rmvb"));
            ASSERT_TRUE(retArray[2]==_T("X:\\downloads\\Everybody Loves Raymond  First season\\"));
            ASSERT_TRUE(retArray[3]==_T("Everybody_Loves_Raymond_-_1x02_-_I_Love_You.(DVD).MEDiEVAL.[www.dream2008.cn].xvid"));
        }
        {
            TCHAR szName[] = _T("rar://X:\\ENT\\Top_Gear.14x05.720p_HDTV_x264-FoV\\top_gear.14x05.720p_hdtv_x264-fov.rar?top_gear.14x05.720p_hdtv_x264-fov.mkv");
            CSVPToolBox toolbox;
            CStringArray retArray;
            CString strRet = toolbox.getVideoFileBasename(szName, &retArray);
            ASSERT_TRUE(retArray[0]==_T("X:\\ENT\\Top_Gear.14x05.720p_HDTV_x264-FoV\\top_gear.14x05.720p_hdtv_x264-fov"));
            ASSERT_TRUE(retArray[1]==_T(".mkv"));
            ASSERT_TRUE(retArray[2]==_T("X:\\ENT\\Top_Gear.14x05.720p_HDTV_x264-FoV\\"));
            ASSERT_TRUE(retArray[3]==_T("top_gear.14x05.720p_hdtv_x264-fov"));
        }

    }
}