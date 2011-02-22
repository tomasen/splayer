//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
#include <string.h>
#include <stdio.h>

char g_szClass[256] = { 0 };
char g_szLookup[256] = { 0 };
int  g_nAssumeValid = 0;
char g_szData[256] = { 'c', 'h', 'a', 'r', ' ', '*', '\0' };
char g_szOutFile[256] = { 0 };
char g_szInFile[256] = { 0 };
int  g_nUnicode = 0;
char * g_szpreamble = "";
int g_nPreamble = 0;

unsigned long hash_string( const char * sz, int nlen )
{
	const char* pch;
	unsigned long hash;
	int i;

	hash = 0;
	pch = sz;
//	while( *pch != 0 )
	for (i=0; i<nlen; i++)
	{
		hash = (hash<<3)+hash+(*pch);
		pch++;
	}

	return( hash );
}

struct static_hash_node
{
	char * key;
	unsigned long hash;
	size_t link;  // static time -- index, run time -- pointer
	size_t entries;

	char * data;
};

struct static_hash_table
{
	struct static_hash_node *pdata;
	int size;
	int max_size;
	int non_root_entries;
};

struct input_data
{
	char * key;
	int keylen;
	char * data;
	int datalen;
	struct input_data * next;
};

struct input_table
{
	struct input_data * phead;
	struct input_data * ptail;
	int size;
};

struct input_data * add_input_data(struct input_table * ptable)
{
	struct input_data * pnew;
	pnew = (struct input_data *) malloc(sizeof(*pnew));
	if (pnew)
	{
		memset(pnew, 0x00, sizeof(*pnew));
		if (ptable->ptail)
		{
			ptable->ptail->next = pnew;
		}
		ptable->ptail = pnew;
		if (!ptable->phead)
		{
			ptable->phead = pnew;
		}
		ptable->size += 1;
	}

	return pnew;
}

int get_input_from_line(struct input_data * pdata, char ** pszline)
{
	char * szcomma;
	char * szendln;
	char *szline;

	szline = *pszline;

	szcomma = strchr(szline, ',');
	if (!szcomma)
	{
		return 0;
	}
	pdata->key = szline;
	pdata->keylen = szcomma-szline;
	szcomma++;

	szendln = strchr(szcomma, '\n');
	if (!szendln)
	{
		return 0;
	}

	pdata->data = szcomma;
	pdata->datalen = szendln-szcomma;
	if (*(szendln-1) == '\r')
	{
		pdata->datalen -= 1;
	}

	*pszline = szendln+1;

	return 1;
}

void destroy_input_table(struct input_table * ptable)
{
	struct input_data *pdata;

	while (ptable->phead)
	{
		pdata = ptable->phead;
		ptable->phead = ptable->phead->next;
		free(pdata);
	}
}

int choose_static_hash_size(struct static_hash_table * ptable)
{
	return 2*(ptable->size);
}

void destroy_runtime_hash(struct static_hash_table *ptable)
{
	int i;
	struct static_hash_node *p;
	struct static_hash_node *temp;
	int size;
	size=ptable->max_size;
	p=(void*)0;
	temp=(void*)0;
	
	if (!ptable || !ptable->pdata)
		return;
	
	for (i=0; i<size; i++)
	{
		p = &ptable->pdata[i];
		if (p->key)
			free(p->key);
		if (p->data)
			free(p->data);
			
		p = (struct static_hash_node *)(p->link);
		while (p)
		{
			temp = p;
			p = (struct static_hash_node *)(p->link);
			free(temp);
		}
	}
	
	free(ptable->pdata);
}

int create_runtime_hash(struct static_hash_table *ptable, int max_size)
{
	if (!ptable)
		return 0;
		
	if (ptable->pdata)
		destroy_runtime_hash(ptable);

	memset(ptable, 0x00, sizeof(ptable));
	ptable->pdata = (struct static_hash_node *)malloc(max_size*sizeof(struct static_hash_node));
	if (!ptable->pdata)
		return 0;
	memset(ptable->pdata, 0x00, max_size*sizeof(struct static_hash_node));
	ptable->max_size = max_size;
	ptable->size = 0;
	
	return 1;
}

struct static_hash_node * runtime_lookup(struct static_hash_table *ptable, const char *key, int keylen, int *found, unsigned long *phash, unsigned long *pindex)
{
	unsigned long hash;
	struct static_hash_node *p;
	*found = 0;
	hash = hash_string(key, keylen);
	if (phash)
		*phash = hash;
//	hash = hash % ptable->max_size;
	p = &ptable->pdata[hash % ptable->max_size];
	while (p->key)
	{
		if (p->hash==hash && !strncmp(key, p->key, keylen))
		{
			*found = 1;
			break;
		}
		
		if (p->link)
			p = (struct static_hash_node *)(p->link);
		else
			break;
	}
	
	if (pindex)
		*pindex = hash % ptable->max_size;
	
	return p;
}

char * _strndup(const char * str, int nlen)
{
	char * str2;
	if (nlen == -1)
	{
		nlen = strlen(str);
	}

	str2 = (char *) malloc(nlen+1);
	if (str2)
	{
		memcpy(str2, str, nlen);
		str2[nlen] = '\0';
	}

	return str2;
}

int set_runtime_element(struct static_hash_table *ptable, const char *key, int nkeylen, const char *data, int ndatalen)
{
	struct static_hash_node *p;
	struct static_hash_node *newnode;
	int found;
	unsigned long hash;
	unsigned long index;
	
	p = runtime_lookup(ptable, key, nkeylen, &found, &hash, &index);
	if (found)
	{
		if (p->data)
			free(p->data);
	}
	else // !found
	{	
		if (p->key)
		{
			newnode = (struct static_hash_node *)malloc(sizeof(struct static_hash_node));
			if (!newnode)
				return 0;
			memset(newnode, 0x00, sizeof(struct static_hash_node));
			while (p->link != 0)
				p = (struct static_hash_node *)(p->link);
				
			p->link = (size_t)newnode;
			ptable->pdata[index].entries++;
			ptable->non_root_entries++;
			p = newnode;
		}
		
		p->key = _strndup(key, nkeylen);
		p->hash = hash;
		if (!p->key)
			return 0;
			
		ptable->size++;
	}

	p->data = _strndup(data, ndatalen);
	if (!p->data)
		return 0;
	return 1;
}

void generate_static_table(struct static_hash_table *ptable, /*FILE *fp,*/ const char * szTableName)
{
	int i;
	int counter;
	struct static_hash_node *p;
	const char *szkey;
	const char *szdata;
	const char *szkeyquote;
	const char *szunicode;
	char * szchartype;
	char * szcmpfn;
	int ideal_size;
	FILE *fp;
	counter=1;
	ideal_size = choose_static_hash_size(ptable);
	
	if (g_nUnicode)
	{
		szchartype = "wchar_t";
		szcmpfn = "wcscmp";
	}
	else
	{
		szchartype = "char";
		szcmpfn = "strcmp";
	}
	
	fp = fopen(g_szOutFile, "wb");
	if (!fp)
	{
		printf("could not open file for write: %s\r\n", g_szOutFile);
		return;
	}
	
	fprintf(fp, "#include <string.h>\r\n\r\n");
	fprintf(fp, "%.*s", g_nPreamble, g_szpreamble);
	fprintf(fp, "class %s\r\n"
	        "{\r\n"
			"public:\r\n\r\n"
			"	struct HashNode\r\n"
			"	{\r\n"
			"		%s * key;\r\n"
			"		unsigned long hash;\r\n"
			"		size_t link;\r\n"
			"		size_t entries;\r\n"
			"		%s data;\r\n"
			"	};\r\n"
			"\r\n"
			"protected:\r\n"
			"	const static HashNode m_data[%d];\r\n"
			"	const static size_t m_size = %d;\r\n"
			"	const static size_t m_tableSize = %d;\r\n"
			"\r\n"
	        "public:\r\n\r\n"
	        "	static unsigned long Hash( const %s * sz )\r\n"
	        "	{\r\n"
	        "		unsigned long hash;\r\n"
	        "		hash = 0;\r\n"
	        "		while ( *sz != 0 )\r\n"
	        "		{\r\n"
	        "			hash = (hash<<3)+hash+(*sz);\r\n"
	        "			sz++;\r\n"
	        "		}\r\n"
	        "		return hash;\r\n"
	        "	}\r\n"
	        "\r\n"
	        "	static const HashNode * %s( const %s * key )\r\n"
	        "	{\r\n"
	        "		unsigned long hash;\r\n"
	        "		const HashNode * p;\r\n"
	        "		unsigned long index;\r\n"
	        "		hash = Hash(key);\r\n"
	        "		index = hash %% m_size;\r\n"
	        "		p = &m_data[index];\r\n"
	        "		while (p->key)\r\n"
	        "		{\r\n"
	        "			if (p->hash==hash && !%s(key, p->key))\r\n"
	        "				break;\r\n"
	        "\r\n"
	        "			if (p->link)\r\n"
	        "			{\r\n"
	        "				p = &m_data[p->link];\r\n"
	        "			}\r\n"
	        "			else\r\n"
	        "			{\r\n"
	        "				p = 0;\r\n"
	        "				break;\r\n"
	        "			}\r\n"
	        "		}\r\n"	
	        "\r\n"	
			"		if (p && p->key)\r\n"	
	        "			return p;\r\n"
			"		return NULL;\r\n"
	        "	}\r\n"
	        "};\r\n"
	        "\r\n"
	        "const %s::HashNode %s::m_data[%d] =\r\n"
	        "{\r\n",
			g_szClass,
			szchartype,
			g_szData,
			ptable->max_size+ptable->non_root_entries,
			ptable->max_size,
			ptable->size,
			szchartype,
			g_szLookup,
			szchartype,
			szcmpfn,
			g_szClass,
			g_szClass,
			ptable->max_size+ptable->non_root_entries
	);	
//	fprintf(fp, "const static static_hash_node %s_data[%d] = \r\n{\r\n", szTableName, size);
//	fprintf(fp, "static struct static_hash_node %s_data[%d] = \r\n{\r\n", szTableName, ptable->max_size+ptable->non_root_entries);
	
	// initial loop
	for (i=0; i<ptable->max_size; i++)
	{
		szkey = ptable->pdata[i].key;
		szdata = ptable->pdata[i].data;
		if (!szkey)
		{
			szkey = "0";
			szkeyquote = "";
			szunicode = "";
		}
		else
		{
			if (g_nUnicode)
				szunicode = "L";
			else
				szunicode = "";
			szkeyquote = "\"";
		}
		if (!szdata)
		{
			szdata = "";
//			szdataquote = "";
		}
//		else
//			szdataquote = "\"";
//		fprintf(fp, "{ %s, %s, %u, %u },"
		fprintf(fp, "\t{ %s%s%s%s, 0x%.08x, %u, %u, %s },\r\n",
			szunicode,
			szkeyquote,
			szkey,
			szkeyquote,
			ptable->pdata[i].hash,
			ptable->pdata[i].entries ? ptable->max_size+counter-1 : 0,
			ptable->pdata[i].entries,
			szdata);
		
		counter+= ptable->pdata[i].entries;
	}
	
	counter = ptable->max_size+1;
	
	// secondary loop (handles collisions)
	for (i=0; i<ptable->max_size; i++)
	{
		p = &ptable->pdata[i];
		p = (struct static_hash_node *)(p->link);
		while (p)
		{
			szkey = p->key;
			szdata = p->data;
			if (!szkey)
			{
				szkey = "0";
				szkeyquote = "";
				szunicode = "";
			}
			else
			{
				if (g_nUnicode)
					szunicode = "L";
				else
					szunicode = "";
				szkeyquote = "\"";
			}
			if (!szdata)
			{
				szdata = "";
//				szdataquote = "";
			}
//			else
//				szdataquote = "\"";
			
	//		fprintf(fp, "{ %s, %s, %u, %u },"
			fprintf(fp, "\t{ %s%s%s%s, 0x%.08x, %u, %u, %s  },\r\n",
				szunicode,
				szkeyquote,
				szkey,
				szkeyquote,
				p->hash,
				p->link ? counter : 0,
				0,
				szdata);
			
			counter++;
			p = (struct static_hash_node *)(p->link);
		}
	}
	
	fprintf(fp, "};\r\n\r\n");

#if 0
	fprintf(fp, "struct static_hash_table %s_table = {"
		   " %s_data,"
		   " %d,"
		   " %d,"
		   " %d,};",
		   szTableName,
		   szTableName,
		   ptable->size,
		   ptable->max_size,
		   ptable->non_root_entries);
#endif
		
	fclose(fp);
}

#if 0
struct static_hash_node * static_lookup(struct static_hash_table *ptable, const char *key, int *found, unsigned long *phash, unsigned long *pindex)
{
	unsigned long hash;
	struct static_hash_node *p;
	unsigned long index;
	*found = 0;
	hash = hash_string(key);
	if (phash)
		*phash = hash;
	index = hash % ptable->max_size;
	p = &ptable->pdata[index];
	while (p->key)
	{
		if (p->hash==hash && !strcmp(key, p->key))
		{
			*found = 1;
			break;
		}
		
		if (p->link)
			p = &ptable->pdata[p->link];
		else
			break;
	}
	
	if (pindex)
		*pindex = index;
	
	return p;
}
#endif

int all_hashes_unique(struct static_hash_table *ptable)
{
	int i;
	int j;
	struct static_hash_node *p;
	struct static_hash_node *p2;
	
	for (i=0; i<ptable->max_size; i++)
	{
		p = &ptable->pdata[i];
		while (p != 0)
		{
			if (!p->key)
			{
				break;
			}
			
			for (j=0; j<ptable->max_size; j++)
			{
				p2 = &ptable->pdata[j];
				while (p2 != 0)
				{
					if (p2 != p && p2->hash==p->hash)
					{
						return 0;
					}
					
					if (p2->link)
						p2 = (struct static_hash_node *)(p2->link);
					else
						p2 = 0;
				}
			}
			
			if (p->link)
				p = (struct static_hash_node *)(p->link);
			else
				p = 0;
		}
	}
	
	return 1;
}

void print_usage(int i)
{
	printf("usage: static_hash /i infile /o outfile [/c classname] [/d datatype] [/l lookupfunction] [/w] [/v]\r\n\r\n"
	       "/i infile         - the input file (required)\r\n"
	       "/o outfile        - the output file (required)\r\n"
	       "/c classname      - the static hash class (optional - defaults to CStaticHash)\r\n"
	       "/d datatype       - the datatype stored in the has (optional - defaults to char*)\r\n"
	       "/l lookupfunction - the name of the generated lookup function (optional - defaults to Lookup)\r\n"
	       "/v                - assume no lookups with invalid keys (optional - defaults to false)\r\n"
	       "/w                - use UNICODE keys\r\n");
	exit(i);
}

void parse_cmd_line(int argc, char * argv[])
{
	int i;
	
	for (i=1; i<argc; i++)
	{
		if (argv[i][0] != '/')
		{
			printf("unknown option: %s\n", argv[i]);
			print_usage(1);
		}
		switch(argv[i][1])
		{
			case 'C': case 'c':
			{
				if (++i >= argc)
				{
					printf("missing argument: %s\n", argv[i-1]);
					print_usage(1);
				}
				strcpy(g_szClass, argv[i]);
				break;
			}
			case 'D': case 'd':
			{
				if (++i >= argc)
				{
					printf("missing argument: %s\n", argv[i-1]);
					print_usage(1);
				}
				strcpy(g_szData, argv[i]);
				break;
			}
			
			case 'L': case 'l':
			{
				if (++i >= argc)
				{
					printf("missing argument: %s\n", argv[i-1]);
					print_usage(1);
				}
				strcpy(g_szLookup, argv[i]);
				break;
			}
			case 'V': case 'v':
			{
				g_nAssumeValid = 1;
				break;
			}
			case 'W': case 'w':
			{
				g_nUnicode = 1;
				break;
			}
			case 'I' : case 'i':
			{			
				if (++i >= argc)
				{
					printf("missing argument: %s\n", argv[i-1]);
					print_usage(1);
				}
				strcpy(g_szInFile, argv[i]);
				break;
			}
			case 'O' : case 'o':
			{
				if (++i >= argc)
				{
					printf("missing argument: %s\n", argv[i-1]);
					print_usage(1);
				}
				strcpy(g_szOutFile, argv[i]);
				break;
			}
			default:
				break;
		}
	}
	
	if (!g_szClass[0])
	{
		strcpy(g_szClass, "CStaticHash");
	}
	
	if (!g_szLookup[0])
	{
		strcpy(g_szLookup, "Lookup");
	}
	
	if (!g_szOutFile[0])
	{
		printf("must specify output file\n");
		print_usage(1);
	}
	
	if (!g_szInFile[0])
	{
		printf("must specify input file\n");
		print_usage(1);
	}
}

int preprocess_file(char **pszfile)
{
	char *szfile;
	char *szpct;
	char *szendln;

	szfile = *pszfile;

	while (1)
	{
		szpct = strchr(szfile, '%');
		szpct++;
		if (!strncmp(szpct, "class=", sizeof("class=")-1))
		{
			szpct+= sizeof("class=")-1;
			szendln = strchr(szpct, '\n');
			if (!szendln)
				goto error;

			memcpy(g_szClass, szpct, szendln-szpct);
			g_szClass[szendln-szpct] = '\0';
			if (*(szendln-1) == '\r')
			{
				g_szClass[szendln-szpct-1] = '\0';
			}
			szfile = szendln+1;
			continue;
		}
		if (!strncmp(szpct, "type=", sizeof("type=")-1))
		{
			szpct+= sizeof("type=")-1;
			szendln = strchr(szpct, '\n');
			if (!szendln)
				goto error;

			memcpy(g_szData, szpct, szendln-szpct);
			g_szData[szendln-szpct] = '\0';
			if (*(szendln-1) == '\r')
			{
				g_szData[szendln-szpct-1] = '\0';
			}
			szfile = szendln+1;
			continue;
		}
		if (!strncmp(szpct, "preamble", sizeof("preamble")-1))
		{
			szpct+= sizeof("preamble")-1;
			szendln = strchr(szpct, '\n');
			if (!szendln)
				goto error;

			g_szpreamble = szendln+1;
			szpct = strchr(szendln, '%');
			if (!szpct)
				goto error;

			g_nPreamble = szpct-g_szpreamble;
			szfile = szpct;
// printf("%.*s", g_nPreamble, g_szpreamble);
			continue;
		}
		if (!strncmp(szpct, "data", sizeof("data")-1))
		{
			szendln = strchr(szpct, '\n');
			if (!szendln)
				goto error;
			szfile = szendln+1;
			break;
		}
	}

	*pszfile = szfile;
	return 1;

error:
	printf("bad input file\r\n");
	return 0;
}

#ifndef NO_MAIN
int main(int argc, char * argv[])
{
	struct static_hash_table tbl;
	struct input_table intbl;
	struct input_data *pdata;
	long length;
	FILE *fp;
	char * szfile;
	char * szfileorg;
	char * szend;
	int ideal_size;
	parse_cmd_line(argc, argv);
	memset(&tbl, 0x00, sizeof(tbl));
	memset(&intbl, 0x00, sizeof(intbl));
	pdata = 0;

	fp = fopen(g_szInFile, "rb");
	if (!fp)
	{
		printf("could not open file for read: %s\r\n", g_szInFile);
		return 1;
	}

	length = _filelength(_fileno(fp));
	szfile = (char *) malloc(length*sizeof(char));
	if (!szfile)
	{
		printf("out of memory\r\n");
		return 1;
	}
	fread(szfile, length, 1, fp);
	szend = szfile+length;
	szfileorg = szfile;

	if (!preprocess_file(&szfile))
	{
		return 1;
	}

	while (szfile != szend)
	{
		pdata = add_input_data(&intbl);
		if (!pdata)
		{
			printf("out of memory\r\n");
			return 1;
		}
		if (!get_input_from_line(pdata, &szfile))
		{
			printf("bad input file\r\n");
			return 1;
		}
//		printf("%.*s, %.*s\r\n", pdata->keylen, pdata->key, pdata->datalen, pdata->data);
	}

	pdata = intbl.phead;
	ideal_size = 2*(intbl.size);
	create_runtime_hash(&tbl, ideal_size);
	while (pdata)
	{
		set_runtime_element(&tbl, pdata->key, pdata->keylen, pdata->data, pdata->datalen);
		pdata = pdata->next;
	}

	generate_static_table(&tbl, "soap_types");
	destroy_runtime_hash(&tbl);
	destroy_input_table(&intbl);
	free(szfileorg);
	fclose(fp);
#if 0
	create_runtime_hash(&tbl, 17);
	set_runtime_element(&tbl, "BSTR", "string");
	set_runtime_element(&tbl, "int", "int");
	set_runtime_element(&tbl, "long", "int");
	set_runtime_element(&tbl, "char", "byte");
	set_runtime_element(&tbl, "short", "short");
	set_runtime_element(&tbl, "__int8", "byte");
	set_runtime_element(&tbl, "__int16", "short");
	set_runtime_element(&tbl, "__int32", "int");
	set_runtime_element(&tbl, "__int64", "long");
	set_runtime_element(&tbl, "float", "float");
	set_runtime_element(&tbl, "double", "double");
	set_runtime_element(&tbl, "unsigned int", "unsignedInt");
	set_runtime_element(&tbl, "unsigned long", "unsignedInt");
	set_runtime_element(&tbl, "unsigned char", "unsignedByte");
	set_runtime_element(&tbl, "unsigned short", "unsignedShort");
	set_runtime_element(&tbl, "unsigned __int8", "unsignedByte");
	set_runtime_element(&tbl, "unsigned __int16", "unsignedShort");
	set_runtime_element(&tbl, "unsigned __int32", "unsignedInt");
//	printf("%d\n", all_hashes_unique(&tbl));
	generate_static_table(&tbl, "soap_types");
	destroy_runtime_hash(&tbl);
#endif
}
#endif