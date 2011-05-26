////////////////////////////////////////////////////////////////////
#ifndef __MD5__
#define __MD5__

#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <string>

using namespace std;

void MD5(char* M,int nLen,unsigned long output[4]);
wstring MD5PrintfW(char* M,int nLen);
string MD5PrintfA(char* M,int nLen);



#endif