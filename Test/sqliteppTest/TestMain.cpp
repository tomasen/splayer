// sqliteppTest.cpp : Defines the entry point for the console application.
//
//
//
// Unit Test
//

#include "stdafx.h"
#include <vector>
#include "appSQLlite.h"
#include <sqlitepp.hpp>
#include <string>
#include <stdexcept>
#include <iostream>

int t4 = 1;
int t1 = 1;
int t2 = 1;
int t3 = 1;



int _tmain(int argc, _TCHAR* argv[])
{
/* step0 init-  *********************
*first check del file mytest1.db
*
************************************/
  const wchar_t *filename = L"./mytest1.db";
  const wchar_t *sqlc1 = L"CREATE TABLE IF NOT EXISTS \"settingint\" ( \"hkey\" TEXT,  \"sect\" TEXT,  \"sval\" INTEGER)";
  const wchar_t *sqlc2 = L"CREATE TABLE IF NOT EXISTS \"settingstring\" (\"hkey\" TEXT,   \"sect\" TEXT, \"vstring\" TEXT)";
  const wchar_t *sqlc3 = L"CREATE TABLE IF NOT EXISTS \"settingbin2\" (\"skey\" TEXT,   \"sect\" TEXT, \"vdata\" BLOB)";

  const wchar_t *sqlc4 = L"CREATE UNIQUE INDEX IF NOT EXISTS \"pkey\" on settingint (hkey ASC, sect ASC)";
	const wchar_t *sqlc5 = L"CREATE UNIQUE INDEX IF NOT EXISTS \"pkeystring\" on settingstring (hkey ASC, sect ASC)";
	const wchar_t *sqlc6 = L"CREATE UNIQUE INDEX IF NOT EXISTS \"pkeybin\" on settingbin2 (skey ASC, sect ASC)";


  int i = 0;
  std::wstring sql, sql2, sql3, sql4, sql5;
  SQLliteapp p(filename);
  //wchar_t path[1000];
  //::GetCurrentDirectory(0,path);
  //printf("%s\n",path);
  wprintf(L"THE CHEIF DB_FILE is %s\n",filename);


/* step1: create table  *************
*
************************************/
  p.exec_sql(sqlc1);
  p.exec_sql(sqlc2);
  p.exec_sql(sqlc3);
  p.exec_sql(sqlc4);
  p.exec_sql(sqlc5);
  p.exec_sql(sqlc6);


  std::wstring str1 = L"hello";
  std::wstring str2 = L"world";
/* step2 int-test  *******************
*
*   write int - WriteProfileInt
    get int - GetProfileInt
    compare
************************************/

  if(t2)
  {

  int geti1 = 1;
  int geti2 = 100;

  geti1 = p.GetProfileInt(str2.c_str(), str1.c_str(), -10, false);
  if(geti1 == -10)
    printf("OK--1\n");   // ok

  p.WriteProfileInt(str1.c_str(), str2.c_str(), geti2, false);
  geti1 = p.GetProfileInt(str1.c_str(), str2.c_str(), -1, false);
  if(geti2 == geti1)
    printf("OK--2\n");   // ok

  }

/* step3 string-test  ***************
*
*   write str - WriteProfileString
    get str - GetProfileString
    compare
************************************/
if(t3)
  {
  std::wstring str3s1, str3s2;
  str3s1 = L"ss";
  str3s2 = L"hello\n";
  str3s1 = p.GetProfileString(str2.c_str(), str1.c_str(),L"xx",false);
  if(str3s1 == L"xx")
    printf("OK--3\n");   // ok

  p.WriteProfileString(str1.c_str(), str2.c_str(), str3s2.c_str());
  str3s1 = p.GetProfileString(str1.c_str(), str2.c_str(), L"xx",false);
  if(str3s1.compare(str3s2) == 0)
    printf("OK--4\n");   // ok
  }


/* step4 binrary-test  **************
*
*   write bin
    get bin - donot forget new/delete
**************************************/
if(t4)
{
  char *pp = 0; //donot forget new/delete
  char buf[100];
  char str3[] = "hello4";
  char str4[] = "world4";
  for(i=0;i<80;i++)
  {
    buf[i] = i * 3 % 5 + 2;
  }

  p.GetProfileBinary(str2.c_str(), str1.c_str(),(LPBYTE*)&pp,(UINT*)&i,false);
  if( i == 0 )
  {
    printf("ok--5 bin\n");
  }

  if(i>0 && pp)
  {
    delete[] pp;
    pp = NULL;
  }


  p.WriteProfileBinary(str1.c_str(), str2.c_str(), (LPBYTE)buf, 30); //w 80 BYTE
  p.GetProfileBinary(str1.c_str(), str2.c_str(), (LPBYTE*)&pp, (UINT*)&i, false);
  
  if(memcmp(pp,buf,30) == 0)
      printf("ok--6 bin\n");

  if(i>0 && pp)
  {
    delete[] pp;
    pp = NULL;
  }

}

printf("ALL tests done, ok x 6 ?\n\n");
system("dir .");
system("pause");
return 0;
}

