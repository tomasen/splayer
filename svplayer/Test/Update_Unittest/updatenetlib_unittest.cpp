
#include "common.h"
#include "gtest/gtest.h"
#include "cupdatenetlib.h"
#include "conrgette_interface.h"


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

    TEST(ConrgetteTest, applypatch) 
    {
        CString strOld(_T("..\\..\\Update_Unittest\\Data\\1048\\splayer\\splayer.exe"));
        CString strMatch(_T("..\\..\\Update_Unittest\\Data\\1056\\splayer\\splayer.exe"));
        CString strPatch(_T("..\\..\\Update_Unittest\\Data\\1048to1056.patch"));
        CString strNew(_T("..\\..\\Update_Unittest\\Data\\mergeresult.exe"));
        MergeAndCompare(strOld, strMatch, strPatch, strNew);
    }

    TEST(ConrgetteTest, applypatch2) 
    {
        CString strOld(_T("..\\..\\Update_Unittest\\Data\\1056\\splayer\\splayer.exe"));
        CString strMatch(_T("..\\..\\Update_Unittest\\Data\\1058\\splayer\\splayer.exe"));
        CString strPatch(_T("..\\..\\Update_Unittest\\Data\\1056to1058.patch"));
        CString strNew(_T("..\\..\\Update_Unittest\\Data\\mergeresult1.exe"));
        MergeAndCompare(strOld, strMatch, strPatch, strNew);
    }
}
