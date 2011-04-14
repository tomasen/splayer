#include "stdafx.h"
#include "md5.h"

const unsigned int s[4][4]=
{
	{7,12,17,22},{5,9,14,20},
	{4,11,16,23},{6,10,15,21}
};

const unsigned long t[64]={//t[i]=4294967296*fabs(sin(i+1));
	0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
	0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
	0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
	0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
	0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
	0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
	0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
	0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};

const int serial[64]=
{
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
	1,6,11,0,5,10,15,4,9,14,3,8,13,2,7,12,
	5,8,11,14,1,4,7,10,13,0,3,6,9,12,15,2,
	0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9
};

void func(unsigned long& a,
		  unsigned long b,
		  unsigned long c,
		  unsigned long d,
		  unsigned long M,
		  unsigned long t,
		  int s,
		  int turn)
{
	unsigned long temp;
	switch(turn)
	{
	case 0:
		temp=(b&c)|((~b)&d);
		break;
	case 1:
		temp=(d&b)|((~d)&c);
		break;
	case 2:
		temp=b^c^d;
		break;
	case 3:
		temp=c^(b|(~d));
		break;
	}
	temp+=M+t+a;
	_asm
	{
		mov ecx,s
			rol temp,cl
	}
	a=b+temp;
}

void MD512(const unsigned long M[16],unsigned long hash[4])
{
	int i,j,index=0;
	for (i=0;i<4;i++)
		for (j=0;j<4;j++)
		{
			func(hash[0],hash[1],hash[2],hash[3],M[serial[index]],t[index],s[i][0],i);
			index++;
			func(hash[3],hash[0],hash[1],hash[2],M[serial[index]],t[index],s[i][1],i);
			index++;
			func(hash[2],hash[3],hash[0],hash[1],M[serial[index]],t[index],s[i][2],i);
			index++;
			func(hash[1],hash[2],hash[3],hash[0],M[serial[index]],t[index],s[i][3],i);
			index++;
		}
}

void MD5(char* M,int nLen,unsigned long output[4])
{
	int i,j;
	unsigned long Hash[4]={0x67452301,0xefcdab89,0x98badcfe,0x10325476};
	unsigned long hash[4];

	/////////////////////////////////////////////////////////
	//填充
	__int64 BitsLen=nLen*8;
	int oldlen=nLen;
	while(nLen%64!=56)
	{
		M[nLen++]=0;
	}
	M[oldlen] = (char)0x80;
	*(__int64*)(M+nLen)=BitsLen;
	nLen+=8;

	/////////////////////////////////////////////////////////
	//开始处理分组
	for (i=0;i<nLen;i+=64)
	{
		memcpy(hash,Hash,sizeof(long)*4);
		MD512((const unsigned long*)&M[i],hash);//处理512bits分组
		for (j=0;j<4;j++)
			Hash[j]+=hash[j];
	}

	/////////////////////////////////////////////////////////
	//处理输出。
	for (i=0;i<4;i++)
		for (j=3;j>=0;j--)
		{
			*((char*)(output+i)+j)=*((char*)(Hash+i)+3-j);
		}
}


wstring MD5PrintfW(char* M,int nLen)
{
	WCHAR   ret[128] = {0};
	unsigned long md5ret[4] = {0};
    MD5(M, nLen, md5ret);
    swprintf_s(ret, 128, L"%0.8X%0.8X%0.8X%0.8X", md5ret[0], md5ret[1], md5ret[2], md5ret[3]);

	return ret;
}

string MD5PrintfA(char* M,int nLen)
{
	char   ret[128] = {0};
	unsigned long md5ret[4] = {0};
	MD5(M, nLen, md5ret);
	sprintf_s(ret, 128, "%0.8X%0.8X%0.8X%0.8X", md5ret[0], md5ret[1], md5ret[2], md5ret[3]);

	return ret;
}