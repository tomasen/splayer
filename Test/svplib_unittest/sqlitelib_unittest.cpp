#include "logging.h"
#include "common.h"
#include "gtest/gtest.h"

#include "svplib.h"
#include "libsqlite.h"

namespace 
{
#pragma pack(push, 1)
    typedef struct
    {
        bool fValid;
        CSize size; 
        int bpp, freq;
    } dispmode;
#pragma pack(pop)

    class SQLiteTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            int iDescLen;
            CSVPToolBox svpTool;
            m_sqlite_setting = NULL;
            CString ini = svpTool.GetPlayerPath(L"settting.db");
            char * buff = svpTool.CStringToUTF8(ini, &iDescLen);
            //file name encoded in UTF-8
            m_sqlite_setting = new SQLITE3( buff );
            free(buff);

            ASSERT_TRUE(m_sqlite_setting->db_open);

            //create tables
            m_sqlite_setting->exec_sql("CREATE TABLE IF NOT EXISTS \"settingint\" ( \"hkey\" TEXT,  \"sect\" TEXT,  \"sval\" INTEGER)");
            m_sqlite_setting->exec_sql("CREATE TABLE IF NOT EXISTS \"settingstring\" (  \"hkey\" TEXT,   \"sect\" TEXT,   \"vstring\" TEXT)");
            m_sqlite_setting->exec_sql("CREATE TABLE IF NOT EXISTS \"settingbin2\" (   \"skey\" TEXT,   \"sect\" TEXT,   \"vdata\" BLOB)");
            m_sqlite_setting->exec_sql("CREATE UNIQUE INDEX IF NOT EXISTS \"pkey\" on settingint (hkey ASC, sect ASC)");
            m_sqlite_setting->exec_sql("CREATE UNIQUE INDEX IF NOT EXISTS \"pkeystring\" on settingstring (hkey ASC, sect ASC)");
            m_sqlite_setting->exec_sql("CREATE UNIQUE INDEX IF NOT EXISTS \"pkeybin\" on settingbin2 (skey ASC, sect ASC)");
            m_sqlite_setting->exec_sql("PRAGMA synchronous=OFF");
            m_sqlite_setting->exec_sql("DROP TABLE  IF EXISTS  \"settingbin\""); //old wrong one
        }
        
        virtual void TearDown() { 
            if (m_sqlite_setting)
            {
                m_sqlite_setting->exec_sql("PRAGMA synchronous=ON");
                delete m_sqlite_setting;
                m_sqlite_setting = NULL;
            }
        }

        SQLITE3* m_sqlite_setting ;
    };


    TEST(SQLITETest, filename) 
    {
        int iDescLen;
        CSVPToolBox svpTool;
        CString ini = svpTool.GetPlayerPath(L"ÖÐÎÄsettting.db");
        char * buff = svpTool.CStringToUTF8(ini, &iDescLen);
        //file name encoded in UTF-8
        SQLITE3* sqlite_setting = new SQLITE3( buff );
        free(buff);
        if(!sqlite_setting->db_open){
            delete sqlite_setting;
            sqlite_setting = NULL;
        }
        delete sqlite_setting;
        sqlite_setting = NULL;
    }

    TEST_F(SQLiteTest, SettingDBString) 
    {
        BOOL b;
        m_sqlite_setting->begin_transaction();

        b = m_sqlite_setting->WriteProfileString(_T("commonsection"), _T("StrKey1"), _T("value1"));
        ASSERT_TRUE(b);
        b = m_sqlite_setting->WriteProfileString(_T("commonsection"), _T("StrKey2"), _T("value1"));
        ASSERT_TRUE(b);
        b = m_sqlite_setting->WriteProfileString(_T("commonsection"), _T("StrKey3"), _T("value1"));
        ASSERT_TRUE(b);
        b = m_sqlite_setting->WriteProfileString(_T("commonsection"), _T("StrKey4"), _T("value1"));
        ASSERT_TRUE(b);
        m_sqlite_setting->end_transaction();

        CString strResult;
        strResult = m_sqlite_setting->GetProfileString(_T("commonsection"), _T("StrKey1"));
        ASSERT_TRUE(strResult.Compare(_T("value1")) == 0);
        CString strResult2;
        strResult2 = m_sqlite_setting->GetProfileString(_T("commonsection"), _T("KeyNotExists"), _T("DEFAULT"), FALSE);
        ASSERT_TRUE(strResult2.Compare(_T("DEFAULT")) == 0);
        strResult2 = m_sqlite_setting->GetProfileString(_T("commonsection"), _T("KeyNotExists"), NULL, FALSE);
        ASSERT_TRUE(strResult2.IsEmpty());
    }


    TEST_F(SQLiteTest, SettingDBBin) 
    {
        BOOL b;
        m_sqlite_setting->begin_transaction();
        dispmode mymode;
        mymode.bpp = 120;
        mymode.freq = 130;
        mymode.fValid = 1;
        mymode.size.cx = 1024;
        mymode.size.cy = 768;
        b = m_sqlite_setting->WriteProfileBinary(_T("commonsection"), _T("BINKEY1"), (BYTE*)&mymode, sizeof(dispmode));
        ASSERT_TRUE(b);

        m_sqlite_setting->end_transaction();

        dispmode readmode;
        BYTE* ptr;
        UINT len;

        BOOL bret1 = m_sqlite_setting->GetProfileBinary(_T("commonsection"), _T("BINKEY1"), &ptr, &len, FALSE);
        ASSERT_TRUE(bret1);
        ASSERT_EQ(len, sizeof(dispmode));

        memcpy(&readmode, ptr, len);
        delete ptr;
        ASSERT_EQ(readmode.bpp, mymode.bpp);
        ASSERT_EQ(readmode.freq, mymode.freq);
        ASSERT_EQ(readmode.fValid, mymode.fValid);
        ASSERT_EQ(readmode.size.cx, mymode.size.cx);
        ASSERT_EQ(readmode.size.cy, mymode.size.cy);

        //test the override previous value
        mymode.size.cx = 2048;
        mymode.size.cy = 1768;
        b = m_sqlite_setting->WriteProfileBinary(_T("commonsection"), _T("BINKEY1"), (BYTE*)&mymode, sizeof(dispmode));
        ASSERT_TRUE(b);
        bret1 = m_sqlite_setting->GetProfileBinary(_T("commonsection"), _T("BINKEY1"), &ptr, &len, FALSE);
        ASSERT_TRUE(bret1);
        ASSERT_EQ(len, sizeof(dispmode));

        memcpy(&readmode, ptr, len);
        delete ptr;
        ASSERT_EQ(readmode.bpp, mymode.bpp);
        ASSERT_EQ(readmode.freq, mymode.freq);
        ASSERT_EQ(readmode.fValid, mymode.fValid);
        ASSERT_EQ(readmode.size.cx, mymode.size.cx);
        ASSERT_EQ(readmode.size.cy, mymode.size.cy);

    }

    TEST_F(SQLiteTest, SettingDBInt) 
    {
        BOOL b;
        m_sqlite_setting->begin_transaction();

        b = m_sqlite_setting->WriteProfileInt(_T("commonsection"), _T("IntKey1"), 1);
        ASSERT_TRUE(b);
        b = m_sqlite_setting->WriteProfileInt(_T("commonsection"), _T("IntKey2"), 1);
        ASSERT_TRUE(b);
        b = m_sqlite_setting->WriteProfileInt(_T("commonsection"), _T("IntKey3"), 1);
        ASSERT_TRUE(b);
        b = m_sqlite_setting->WriteProfileInt(_T("commonsection"), _T("IntKey4"), 1);
        ASSERT_TRUE(b);
        b = m_sqlite_setting->WriteProfileInt(_T("commonsection"), _T("IntKey5"), 1);
        ASSERT_TRUE(b);
        b = m_sqlite_setting->WriteProfileInt(_T("commonsection"), _T("IntKey6"), 1);
        ASSERT_TRUE(b);
        b = m_sqlite_setting->WriteProfileInt(_T("commonsection"), _T("IntKey7"), 1);
        ASSERT_TRUE(b);

        m_sqlite_setting->end_transaction();

        int nValue;
        nValue = m_sqlite_setting->GetProfileInt(_T("commonsection"), _T("IntKey1"), 0);
        ASSERT_EQ(nValue, 1);
        nValue = m_sqlite_setting->GetProfileInt(_T("commonsection"), _T("KeyNotExists"), 1234, FALSE);
        ASSERT_EQ(nValue, 1234);
    }
}
