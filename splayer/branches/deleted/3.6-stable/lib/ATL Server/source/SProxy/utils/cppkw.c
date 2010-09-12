/*
 *
 * cppkw.c
 *
 * generate the cppkeywords.in file
 * this will include all reserved macros in
 * addition to all reserved keywords
 *
 */
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include <string.h>
#include <stdio.h>
#include <malloc.h>

// usage : cppkw <location of keywdat.h> <location of p0io.c> <outputfile>
int main(int argc, char *argv[])
{
	char *szRoot;
	char *szData;
	char *szTemp;
	char *szEnd;
	FILE *fpIn;
	FILE *fpOut;
	int nRet;
	long nFileLength = 0;
	
	
	nRet = 0;
	szRoot = NULL;
	szData = NULL;
	szTemp = NULL;
	szEnd = NULL;
	fpIn = NULL;
	fpOut = NULL;
	
	
	if (argc < 4)
	{
		printf("usage : cppkw <location of keywdat.h> <location of p0io.c> <outputfile>\n");
		return 1;
	}
	
	fpIn = fopen(argv[1], "r");
	if (!fpIn)
	{
		printf("error : could not open keywdat file \"%s\"\n", argv[1]);
		return 1;
	}
	
	fpOut = fopen(argv[3], "w");
	if (!fpOut)
	{
		printf("error : could not open output file \"%s\" for writing\n", argv[3]);
		goto error;
	}
	
	fprintf(fpOut, 
		"%%class=CCppKeywordLookup\r\n"
		"%%preamble\r\n"
		"//\r\n"
		"// generated with VC7Libs\\Nonship\\Src\\Sproxy2\\static_hash\\static_hash /i cppkeywords.in /o CppKeywordTable.h /w\r\n"
		"//\r\n"
		"\r\n"
		"%%type=char\r\n"
		"%%data\r\n");
	
	nFileLength = _filelength(_fileno(fpIn));
	
	szData = (char *)malloc(nFileLength+1);
	if (!szData)
	{
		printf("out of memory\n");
		goto error;
	}
	fread(szData, nFileLength, 1, fpIn);
	szData[nFileLength] = '\0';
	szTemp = szRoot = szData;
	szEnd = szData+nFileLength;
	szTemp = strstr(szTemp, "\nDAT(\"");
	
	while (szTemp != NULL)
	{
		char *szTemp2 = NULL;		
		szData = szTemp;
		szData+= sizeof("\nDAT(\"")-1;
		if (szData >= szEnd)
		{
			printf("error in (expected) format of keywdat file.\n");
			goto error;
		}
		
		szTemp2 = strchr(szData, '"');
		if (!szTemp2)
		{
			printf("error in (expected) format of keywdat file.\n");
			goto error;
		}
		
		fprintf(fpOut, "%.*s,0\r\n", szTemp2-szData, szData);
		
		szTemp = strstr(szTemp+1, "\nDAT(\"");
	}
	
	fclose(fpIn);
	
	fpIn = fopen(argv[2], "r");
	if (!fpIn)
	{
		printf("error : could not open p0io file \"%s\" for reading.\n", argv[2]);
		goto error;
	}
	
	nFileLength = _filelength(_fileno(fpIn));
	if ((szEnd-szRoot) < nFileLength)
	{
		free(szRoot);
		szRoot = (char *)malloc(nFileLength+1);
		if (!szRoot)
		{
			printf("out of memory.\n");
			goto error;
		}
	}
	
	fread(szRoot, nFileLength, 1, fpIn);
	szRoot[nFileLength] = '\0';
	szTemp = szData = szRoot;
	szEnd = szData+nFileLength;
	
	szTemp = strstr(szTemp, "GetIdForString((pIdString_t) \"");
	while (szTemp != NULL)
	{
		char *szTemp2 = NULL;
		szData = szTemp;
		szData += sizeof("GetIdForString((pIdString_t) \"")-1;
		if (szData >= szEnd)
		{
			printf("error in (expected) format of p0io file.\n");
			goto error;
		}
		szTemp2 = strchr(szData, '"');
		if (!szTemp2)
		{
			printf("error in (expected) format of p0io file.\n");
			goto error;
		}
		fprintf(fpOut, "%.*s,0\r\n", szTemp2-szData, szData);
		szTemp = strstr(szTemp+1, "GetIdForString((pIdString_t) \"");
	}
	
	goto end;
	
error:

	nRet = 1;
	
end:

	if (fpIn != NULL)
	{
		fclose(fpIn);
	}
	if (fpOut != NULL)
	{
		fclose(fpOut);
	}
	if (szRoot != NULL)
	{
		free(szRoot);
	}

	return nRet;
}