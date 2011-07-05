//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
#include <string.h>

enum FACET_TYPE
{
	//
	// value is timeDuration
	//
	FACET_DURATION     = 1,

	//
	// value is "hex" or "base64"
	//
	FACET_ENCODING     = 2,

	//
	// specified set of values -- this constrains
	// the datatype to the specified values
	//
	FACET_ENUMERATION  = 4,

	//
	// number of units of length
	// units of length depends on the data type
	// value is nonNegativeInteger
	//
	FACET_LENGTH       = 8,

	//
	// Upper bound value (all values are less than this value). 
	// This value must be the same data type as the inherited data type.
	//
	FACET_MAXEXCLUSIVE = 16,

	//
	// Maximum value. 
	// This value must be the same data type as the inherited data type.
	//
	FACET_MAXINCLUSIVE = 32,

	//
	// Maximum number of units of length. 
	// Units of length depends on the data type. 
	// This value must be nonNegativeInteger.
	//
	FACET_MAXLENGTH    = 64,

	//
	// Lower bound value (all values are greater than this value). 
	// This value must be the same data type as the inherited data type.
	//
	FACET_MINEXCLUSIVE = 128,

	//
	// Minimum value. 
	// This value must be the same data type as the inherited data type.
	//
	FACET_MININCLUSIVE = 256,

	//
	// Maximum number of units of length. 
	// Units of length depends on the data type. 
	// This value must be nonNegativeInteger.
	//
	FACET_MINLENGTH    = 512,

	//
	// Specific pattern that the data type’s values must match. 
	// This constrains the data type to literals that match the 
	// specified pattern. 
	// The pattern value must be a regular expression.
	//
	FACET_PATTERN      = 1024,

	//
	// Frequency of recurrence for recurringDuration and its derivatives. 
	// This value must be timeDuration.
	//
	FACET_PERIOD       = 2048,

	//
	// Maximum number of digits for data types derived from decimal. 
	// This value must a positiveInteger.
	//
	FACET_PRECISION    = 4096,

	//
	// Maximum number of digits in the fractional portion for data types 
	// derived from decimal. 
	// This value must a nonNegativeInteger.
	//
	FACET_SCALE        = 8192,
	
	//
	// whiteSpace provides for: constraining a value space to the white space
	// normalization rules.
	//
	FACET_WHITESPACE   = 16384
};

class CFacetLookup
{
public:

	struct HashNode
	{
		wchar_t * key;
		unsigned long hash;
		size_t link;
		size_t entries;
		FACET_TYPE data;
	};

protected:
	const static HashNode m_data[34];
	const static size_t m_size = 30;
	const static size_t m_tableSize = 15;

public:

	static unsigned long Hash( const wchar_t * sz )
	{
		unsigned long hash;
		hash = 0;
		while ( *sz != 0 )
		{
			hash = (hash<<3)+hash+(*sz);
			sz++;
		}
		return hash;
	}

	static const HashNode * Lookup( const wchar_t * key )
	{
		unsigned long hash;
		const HashNode * p;
		unsigned long index;
		hash = Hash(key);
		index = hash % m_size;
		p = &m_data[index];
		while (p->key)
		{
			if (p->hash==hash && !wcscmp(key, p->key))
				break;

			if (p->link)
			{
				p = &m_data[p->link];
			}
			else
			{
				p = 0;
				break;
			}
		}

		if (p && p->key)
			return p;
		return NULL;
	}
};

const CFacetLookup::HashNode CFacetLookup::m_data[34] =
{
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"pattern", 0x03f0ab26, 0, 0, FACET_PATTERN },
	{ 0, 0x00000000, 0, 0,  },
	{ L"minExclusive", 0x68be850c, 30, 1, FACET_MINEXCLUSIVE },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"duration", 0x20a8dd8e, 0, 0, FACET_DURATION },
	{ L"whiteSpace", 0xe8cc7a8d, 0, 0, FACET_WHITESPACE },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"length", 0x006cca22, 0, 0, FACET_LENGTH },
	{ 0, 0x00000000, 0, 0,  },
	{ L"maxInclusive", 0xd872dfa8, 0, 0, FACET_MAXINCLUSIVE },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"encoding", 0x20accab7, 31, 1, FACET_ENCODING },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"maxExclusive", 0xd109555e, 32, 1, FACET_MAXEXCLUSIVE },
	{ L"enumeration", 0x39b9a407, 0, 0, FACET_ENUMERATION },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"scale", 0x000cc020, 0, 0, FACET_SCALE },
	{ 0, 0x00000000, 0, 0,  },
	{ L"minInclusive", 0x70280f56, 33, 1, FACET_MININCLUSIVE },
	{ 0, 0x00000000, 0, 0,  },
	{ L"minLength", 0x3966c1de, 0, 0, FACET_MINLENGTH  },
	{ L"period", 0x0070709b, 0, 0, FACET_PERIOD  },
	{ L"maxLength", 0x376ffd80, 0, 0, FACET_MAXLENGTH  },
	{ L"precision", 0x437651d4, 0, 0, FACET_PRECISION  },
};

