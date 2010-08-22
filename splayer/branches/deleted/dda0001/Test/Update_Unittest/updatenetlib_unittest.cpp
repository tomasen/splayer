
#include "common.h"
#include "gtest/gtest.h"
#include "cupdatenetlib.h"
#include "conrgette_interface.h"
#include "svptoolbox.h"

char* szUrl = "http://127.0.0.1:8080/Update";
//char* szUrl = "http://svplayer.shooter.cn/api/updater.php";


namespace 
{
    //old + patch = new, compare to match.
    void MergeAndCompare(CString strOld, CString strMatch, CString strPatch, CString strNew)
    {
        bool b = ApplyEnsemblePatch(strOld, strPatch, strNew);
        ASSERT_TRUE(b);
        BYTE* pb1;
        BYTE* pb2;
        DWORD dw1, dw2;
        b = ReadFileToBuffer(strMatch, pb1, &dw1);
        b = ReadFileToBuffer(strNew, pb2, &dw2);
        ASSERT_EQ(dw1, dw2);
        ASSERT_EQ(0, memcmp(pb1, pb2,dw1));
        delete pb1;
        delete pb2;
    }

    class ConrgetteTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            CSVPToolBox tb;
            CopyFile(_T("..\\..\\Update_Unittest\\web\\patch\\1048.patch"),
                _T("..\\..\\Update_Unittest\\Data\\1048to1058.patch"),
                FALSE);
            CopyFile(_T("..\\..\\Update_Unittest\\web\\patch\\1056.patch"),
                _T("..\\..\\Update_Unittest\\Data\\1056to1058.patch"),
                FALSE);
        }

        //virtual void TearDown() { 
        //}

    };


    TEST_F(ConrgetteTest, applypatch) 
    {
        CString strOld(_T("..\\..\\Update_Unittest\\Data\\1048\\splayer\\splayer.exe"));
        CString strMatch(_T("..\\..\\Update_Unittest\\Data\\1058\\splayer\\splayer.exe"));
        CString strPatch(_T("..\\..\\Update_Unittest\\Data\\1048to1058.patch"));
        CString strNew(_T("..\\..\\Update_Unittest\\Data\\mergeresult.exe"));
        MergeAndCompare(strOld, strMatch, strPatch, strNew);
    }

    TEST_F(ConrgetteTest, applypatch2) 
    {
        CString strOld(_T("..\\..\\Update_Unittest\\Data\\1056\\splayer\\splayer.exe"));
        CString strMatch(_T("..\\..\\Update_Unittest\\Data\\1058\\splayer\\splayer.exe"));
        CString strPatch(_T("..\\..\\Update_Unittest\\Data\\1056to1058.patch"));
        CString strNew(_T("..\\..\\Update_Unittest\\Data\\mergeresult1.exe"));
        MergeAndCompare(strOld, strMatch, strPatch, strNew);
    }


    class UpdateTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            memset(&sei, 0, sizeof(sei));
            sei.cbSize =  sizeof(sei) ;
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;
            sei.nShow = SW_SHOWNORMAL; 
            sei.lpVerb = _T("open");
            sei.lpFile = _T("python.exe");
            sei.lpParameters = _T("code.py");
        }

        virtual void TearDown() { 
            if (sei.hProcess)
            {
                TerminateProcess(sei.hProcess, 0);
                WaitForSingleObject( sei.hProcess, INFINITE );
            }
        }

        SHELLEXECUTEINFO sei ;
    };

    TEST_F(UpdateTest, basic)
    {
        sei.lpDirectory =_T("..\\..\\Update_Unittest\\web\\basic");
        ASSERT_TRUE(ShellExecuteEx(&sei));
        Sleep(3000); 
        cupdatenetlib upl;
        CSVPToolBox svpToolBox;
        upl.downloadList();
        ASSERT_EQ(1, upl.iSVPCU_TOTAL_FILE);
        int i = upl.downloadFiles() ;
        ASSERT_EQ(i, 1);
        ASSERT_EQ(upl.GetReadyToCopyCount(), 0);
    }

    TEST_F(UpdateTest, basicGz)
    {
        DeleteFile(_T(".\\splayer.exe"));
        DeleteFile(_T(".\\UPD\\063288abed7d083fd0d502fee3bca74b"));
        sei.lpDirectory =_T("..\\..\\Update_Unittest\\web\\splayer");
        ASSERT_TRUE(ShellExecuteEx(&sei));
        Sleep(3000); 
        cupdatenetlib upl;
        CSVPToolBox svpToolBox;
        upl.downloadList();
        ASSERT_EQ(1, upl.iSVPCU_TOTAL_FILE);
        int i = upl.downloadFiles() ;
        ASSERT_EQ(i, 1);

        CMD5Checksum md5checksum;
        CString strMerged = md5checksum.GetMD5(_T(".\\UPD\\063288abed7d083fd0d502fee3bca74b"));
        CString strTarget = md5checksum.GetMD5(_T("..\\..\\Update_Unittest\\Data\\1058\\splayer\\splayer.exe"));
        ASSERT_TRUE(strMerged.CompareNoCase(strTarget) == 0);
        ASSERT_EQ(upl.GetReadyToCopyCount(), 1);
    }


    TEST_F(UpdateTest, ExistingMd5Match)
    {
        //setup the environment
        CopyFile(_T("..\\..\\Update_Unittest\\Data\\1058\\splayer\\splayer.exe"), _T(".\\splayer.exe"), FALSE);
        sei.lpDirectory =_T("..\\..\\Update_Unittest\\web\\splayer");
        ShellExecuteEx(&sei);
        Sleep(3000); 
        cupdatenetlib upl;
        CSVPToolBox svpToolBox;
        upl.downloadList();
        ASSERT_EQ(1, upl.iSVPCU_TOTAL_FILE);
        int i = upl.downloadFiles() ;
        ASSERT_EQ(i, 0);
        ASSERT_EQ(upl.GetReadyToCopyCount(), 0);
    }

    TEST_F(UpdateTest, BasicPatch)
    {
        //1058 to 1048
        //
        DeleteFile(_T(".\\splayer.exe"));
        DeleteFile(_T(".\\UPD\\063288abed7d083fd0d502fee3bca74b"));
        CopyFile(_T("..\\..\\Update_Unittest\\Data\\1056\\splayer\\splayer.exe"), _T(".\\splayer.exe"), FALSE);
        sei.lpDirectory =_T("..\\..\\Update_Unittest\\web\\patch");
        ShellExecuteEx(&sei);
        Sleep(3000); 

        //get the MD5
        CMD5Checksum md5checksum;
        CString strOldMd5 = md5checksum.GetMD5(_T(".\\splayer.exe"));

        cupdatenetlib upl;
        CSVPToolBox svpToolBox;
        upl.downloadList();
        ASSERT_EQ(1, upl.iSVPCU_TOTAL_FILE);
        int i = upl.downloadFiles() ;
        ASSERT_EQ(i, 1);

        CString strMerged = md5checksum.GetMD5(_T(".\\UPD\\063288abed7d083fd0d502fee3bca74b"));
        CString strTarget = md5checksum.GetMD5(_T("..\\..\\Update_Unittest\\Data\\1058\\splayer\\splayer.exe"));
        ASSERT_TRUE(strMerged.CompareNoCase(strTarget) == 0);
        ASSERT_EQ(upl.GetReadyToCopyCount(), 1);
    }

    //new client - new server
    //new client - old server  MD5 is ignored, safe
    //old client - new server  No MD5 sent to server. safe
}
