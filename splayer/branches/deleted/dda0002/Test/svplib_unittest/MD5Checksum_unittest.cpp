//
// HEADER NAME.
//      common.h: The common header file for the unit test 
//
// FUNCTIONAL DESCRIPTION.
//      Include the common header files.
//
//

#include "common.h"
#include "gtest/gtest.h"
#include "md5checksum.h"

namespace 
{
    TEST(MD5Test, regression) 
    {
        CMD5Checksum md5checksum;
        {
            CHAR sz[] = "The quick brown fox jumps over the lazy dog";
            CString strResult = md5checksum.GetMD5((BYTE*) sz, 43);
            ASSERT_TRUE(strResult == _T("9e107d9d372bb6826bd81d3542a419d6"));
        }

        {
            CHAR sz[] = "The quick brown fox jumps over the lazy dog.";
            CString strResult = md5checksum.GetMD5((BYTE*) sz, 44);
            ASSERT_TRUE(strResult == _T("e4d909c290d0fb1ca068ffaddf22cbd0"));
        }

        {
            CHAR sz[] = "";
            CString strResult = md5checksum.GetMD5((BYTE*) sz, 0);
            ASSERT_TRUE(strResult == _T("d41d8cd98f00b204e9800998ecf8427e"));
        }

    }
}