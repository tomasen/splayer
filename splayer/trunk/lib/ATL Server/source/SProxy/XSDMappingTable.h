#include <string.h>

//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

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

enum XSDTYPE
{
	XSDTYPE_ERR = -2,
	XSDTYPE_UNK = -1,
	
	//
	// string
	//
	XSDTYPE_STRING = 0,
	
	//
	// boolean
	//
	XSDTYPE_BOOLEAN,
	
	//
	// Represents single-precision 32-bit floating point numbers.
	//
	XSDTYPE_FLOAT,
	
	//
	// Represents double-precision 64-bit floating point numbers.
	//
	XSDTYPE_DOUBLE,
	
	//
	// Represents arbitrary precision numbers.
	//
	XSDTYPE_DECIMAL,
	
	//
	// Represents a duration of time.
	//
	XSDTYPE_DURATION,

#if 0
/* removed */
	//
	// Represents a timeDuration that recurs at a specific 
	// timeDuration, starting from a specific origin. 
	// This data type cannot be used directly in a schema. 
	// Only derived data types that specify duration and period can be used.
	//
	XSDTYPE_UNK, // abstract
#endif
	
	//
	// Represents binary data. 
	//
	XSDTYPE_HEXBINARY,
	
	//
	// Represents binary data. 
	//
	XSDTYPE_BASE64BINARY,
	
	//
	// Represents a Uniform Resource Identifier 
	// (URI) as defined by RFC 2396.
	//
	XSDTYPE_ANYURI,
	
	//
	// Represents the ID attribute type defined in the XML 1.0 
	// Recommendation. The ID must be an NCName and must be 
	// unique within an XML document.
	//
	XSDTYPE_ID,
	
	//
	// Represents a reference to an element that has an ID attribute 
	// that matches the specified ID. An IDREF must be an NCName and 
	// must be a value of an element or attribute of type ID within 
	// the XML document.
	//
	XSDTYPE_IDREF,
	
	//
	// Represents the ENTITY attribute type in XML 1.0 Recommendation. 
	// This is a reference to an unparsed entity with a name that matches 
	// the specified name. An ENTITY must be an NCName and must be 
	// declared in the schema as an unparsed entity name.
	//
	XSDTYPE_ENTITY,
	
	//
	// Represents the NOTATION attribute type in XML 1.0 Recommendation. 
	// This is a reference to a notation with a name that matches the 
	// specified name. A NOTATION must be an NCName and must be declared 
	// in the schema as a notation name.
	//
	XSDTYPE_NOTATION,
	
	//
	// Represents a qualified name. A qualified name is composed of a prefix 
	// and a local name separated by a colon. Both the prefix and local names 
	// must be of type NCName. The prefix must be associated with a namespace 
	// URI reference, using a namespace declaration.
	//
	XSDTYPE_QNAME,
	
	//
	//	Begin Derived Types
	//
	
	//
	// CDATA represents white space normalized strings.  The value space of CDATA
	// is the set of strings that do not contain the carriage-return (#xD), line-feed
	// (#xA), nor tab (#x9) characters.  The lexical space of CDATA is the set of
	// strings that do not cotain the newline (#xD) nor tab (#x9) characters
	//
	XSDTYPE_NORMALIZEDSTRING,
	
	//
	// token represents tokenized strings.  The value space of token is the set of
	// strings that do not contain the line-feed (#xA) nor tab (#x9) characters,
	// that have no leading or trailing spaces (#x20) and that have no internal sequences
	// of two or more spaces.  The lexical space of token is the set of strings
	// that do not contain the line-feed (#xA) nor tab (#x9) characters, that have
	// no trailing or leading spaces (#x20) and that have no internal sequences
	// of two or more spaces
	XSDTYPE_TOKEN,
	
	
	//
	// Represents natural language identifiers (defined by RFC 1766).
	//
	XSDTYPE_LANGUAGE,
	
	//
	// Represents the IDREFS attribute type. IDREFS contains a 
	// set of values of type IDREF.
	//
	XSDTYPE_IDREFS,
	
	//
	// Represents the ENTITIES attribute type. ENTITIES 
	// contains a set of values of type ENTITY.
	//
	XSDTYPE_ENTITIES,
	
	//
	// Represents the NMTOKEN attribute type. An NMTOKEN 
	// is set of name characters (letters, digits, and other 
	// characters) in any combination. Unlike Name and NCName, 
	// NMTOKEN has no restrictions on the starting character.
	//
	XSDTYPE_NMTOKEN,
	
	//
	// Represents the NMTOKENS attribute type. NMTOKENS contains 
	// a set of values of type NMTOKEN.
	//
	XSDTYPE_NMTOKENS,
	
	//
	// Represents names in XML. A Name is a token that begins with 
	// a letter, underscore, or colon and continues with name 
	// characters (letters, digits, and other characters).
	//
	XSDTYPE_NAME,
	
	//
	// Represents “non-colonized” names. This data type is the 
	// same as Name—except it cannot begin with a colon.
	//
	XSDTYPE_NCNAME,
	
	//
	// Represents a sequence of decimal digits with an optional leading 
	// sign (+ or -). This data type is derived from decimal.
	//
	XSDTYPE_INTEGER,
	
	//
	// Represents an integer that is less than or equal to zero. A 
	// nonPositiveInteger consists of a negative sign (-) and sequence 
	// of decimal digits. This data type is derived from integer.
	//
	XSDTYPE_NONPOSITIVEINTEGER,
	
	//
	// Represents an integer that is less than zero. A negativeInteger 
	// consists of a negative sign (-) and sequence of decimal digits. 
	// This data type is derived from nonPositiveInteger.
	//
	XSDTYPE_NEGATIVEINTEGER,
	
	//
	// Represents an integer with a minimum value of -9223372036854775808 
	// and maximum of 9223372036854775807. This data type is derived from integer.
	//
	XSDTYPE_LONG,
	
	//
	// Represents an integer with a minimum value of -32768 and maximum of 32767. 
	// This data type is derived from long.
	//
	XSDTYPE_INT,
	
	//
	// Represents an integer with a minimum value of -2147483648 and maximum of 2147483647. 
	// This data type is derived from int.
	//
	XSDTYPE_SHORT,
	
	//
	// Represents an integer with a minimum value of -128 and maximum of 127. 
	// This data type is derived from short.
	//
	XSDTYPE_BYTE,
	
	//
	// Represents an integer that is greater than or equal to zero. 
	// This data type is derived from integer.
	//
	XSDTYPE_NONNEGATIVEINTEGER,
	
	//
	// Represents an integer with a minimum of zero and maximum of 18446744073709551615. 
	// This data type is derived from nonNegativeInteger.
	//
	XSDTYPE_UNSIGNEDLONG,
	
	//
	// Represents an integer with a minimum of zero and maximum of 4294967295. 
	// This data type is derived from unsignedLong.
	//
	XSDTYPE_UNSIGNEDINT,
	
	//
	// Represents an integer with a minimum of zero and maximum of 65535. 
	// This data type is derived from unsignedInt.
	//
	XSDTYPE_UNSIGNEDSHORT,
	
	//
	// Represents an integer with a minimum of zero and maximum of 255. 
	// This data type is derived from unsignedShort.
	//
	XSDTYPE_UNSIGNEDBYTE,
	
	//
	// Represents an integer that is greater than zero. 
	// This data type is derived from nonNegativeInteger.
	//
	XSDTYPE_POSITIVEINTEGER,
	
	//
	// Represents an instant of time. 
	// This data type is derived from recurringDuration.
	//
	XSDTYPE_DATETIME,
	
	//
	// Represents the time of day. 
	// This data type is derived from recurringDuration.
	//
	XSDTYPE_TIME,
	
#if 0
/* removed ??? */
	//
	// Represents a period of time with a start and an end. 
	// This data type is derived from recurringDuration. 
	// This data type cannot be used directly in a schema. 
	// Only derived data types that specify duration can be used.
	//
	XSDTYPE_TIMEPERIOD,
#endif
	
	//
	// Represents a timePeriod that begins at midnight of the specified 
	// day and ends at midnight the following day. 
	// This data type is derived from timePeriod. 
	// The format of the date is CCYY-MM-DD. 
	// Example: 2000-09-27
	//
	XSDTYPE_DATE,
	
	//
	// Represents a timePeriod that begins at midnight of the first day of 
	// the specified month and ends at midnight of the last day. This data type 
	// is independent of the number days in the month. 
	// This data type is derived from timePeriod.
	//
	XSDTYPE_GMONTH,
	XSDTYPE_GYEARMONTH,
	
	//
	// Represents a timePeriod that begins at midnight of the first day of 
	// the specified year and ends at midnight of the last day. This data 
	// type is independent of the number days in the year. 
	// This data type is derived from timePeriod.
	//
	XSDTYPE_GYEAR,
	
#if 0
/* removed */
	//
	// Represents a timePeriod that begins at midnight of the first day of the 
	// specified century and ends at midnight of the last day. 
	// This data type is derived from timePeriod.
	//
	XSDTYPE_CENTURY,
#endif
	
	//
	// Represents a recurring day in the year. 
	// This data type is derived from recurringDuration.
	//
	XSDTYPE_GMONTHDAY,
	
	//
	// Represents a recurring day in the month. 
	// This data type is derived from recurringDuration.
	//
	XSDTYPE_GDAY
};

enum XSDFLAGS
{
	XSDFLAG_NONE = 0,
	XSDFLAG_ABSTRACT = 1,
	XSDFLAG_PRIMITIVE = 2,
	XSDFLAG_DERIVED = 4
};

struct _xsdtypemapping
{
	XSDTYPE xsdType;
	XSDTYPE baseType;
	DWORD   dwFlags;
	DWORD   dwAllowableFacets;
};

class CXSDTypeLookup
{
public:

	struct HashNode
	{
		wchar_t * key;
		unsigned long hash;
		size_t link;
		size_t entries;
		_xsdtypemapping data;
	};

protected:
	const static HashNode m_data[99];
	const static size_t m_size = 88;
	const static size_t m_tableSize = 44;

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

__declspec(selectany) const CXSDTypeLookup::HashNode CXSDTypeLookup::m_data[99] =
{
	{ 0, 0x00000000, 0, 0,  },
	{ L"positiveInteger", 0xabe363a1, 0, 0, {XSDTYPE_POSITIVEINTEGER, XSDTYPE_NONNEGATIVEINTEGER, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ L"hexBinary", 0x2bb71922, 88, 1, {XSDTYPE_HEXBINARY, XSDTYPE_UNK, XSDFLAG_ABSTRACT | XSDFLAG_PRIMITIVE, FACET_ENCODING | FACET_LENGTH | FACET_PATTERN | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_ENUMERATION} },
	{ 0, 0x00000000, 0, 0,  },
	{ L"NOTATION", 0x191078f4, 89, 1, {XSDTYPE_NOTATION, XSDTYPE_QNAME, XSDFLAG_DERIVED, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE} },
	{ 0, 0x00000000, 0, 0,  },
	{ L"gYearMonth", 0x4f23e716, 0, 0, {XSDTYPE_GYEARMONTH, XSDTYPE_UNK, XSDFLAG_DERIVED, FACET_DURATION | FACET_PERIOD | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"language", 0x224b89a4, 0, 0, {XSDTYPE_LANGUAGE, XSDTYPE_TOKEN, XSDFLAG_DERIVED, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION | FACET_WHITESPACE} },
	{ L"ENTITIES", 0x16785275, 90, 2, {XSDTYPE_ENTITIES, XSDTYPE_IDREF, XSDFLAG_DERIVED, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_ENUMERATION | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"anyURI", 0x0063dfd8, 0, 0, {XSDTYPE_ANYURI, XSDTYPE_UNK, XSDFLAG_PRIMITIVE, FACET_LENGTH | FACET_PATTERN | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_ENUMERATION} },
	{ L"token", 0x000cfee1, 92, 1, {XSDTYPE_TOKEN, XSDTYPE_NORMALIZEDSTRING, XSDFLAG_DERIVED, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ L"int", 0x0000258b, 93, 1, {XSDTYPE_INT, XSDTYPE_LONG, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ L"byte", 0x000141d4, 0, 0, {XSDTYPE_BYTE, XSDTYPE_SHORT, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ L"ID", 0x000002d5, 94, 1, {XSDTYPE_ID, XSDTYPE_UNK, XSDFLAG_PRIMITIVE, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE} },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"long", 0x00015af0, 95, 1, {XSDTYPE_LONG, XSDTYPE_INTEGER, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ L"IDREF", 0x00082d32, 0, 0, {XSDTYPE_IDREF, XSDTYPE_UNK, XSDFLAG_PRIMITIVE, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE} },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"dateTime", 0x20088755, 0, 0, {XSDTYPE_DATETIME, XSDTYPE_UNK, XSDFLAG_DERIVED, FACET_DURATION | FACET_PERIOD | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ L"string", 0x0074a4ff, 96, 1, {XSDTYPE_STRING, XSDTYPE_UNK, XSDFLAG_PRIMITIVE, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION} },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"normalizedString", 0x66e5729c, 0, 0, {XSDTYPE_NORMALIZEDSTRING, XSDTYPE_STRING, XSDFLAG_DERIVED, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ L"duration", 0x20a8dd8e, 0, 0, {XSDTYPE_DURATION, XSDTYPE_UNK, XSDFLAG_PRIMITIVE, FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE} },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"double", 0x0066a733, 0, 0, {XSDTYPE_DOUBLE, XSDTYPE_UNK, XSDFLAG_PRIMITIVE, FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE} },
	{ L"nonNegativeInteger", 0x44dfc4fc, 0, 0, {XSDTYPE_NONNEGATIVEINTEGER, XSDTYPE_INTEGER, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ L"IDREFS", 0x00499715, 0, 0, {XSDTYPE_IDREFS, XSDTYPE_IDREF, XSDFLAG_DERIVED, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_ENUMERATION | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"nonPositiveInteger", 0xe6262dcc, 0, 0, {XSDTYPE_NONPOSITIVEINTEGER, XSDTYPE_INTEGER, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ L"integer", 0x03c3739e, 97, 1, {XSDTYPE_INTEGER, XSDTYPE_DECIMAL, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ L"short", 0x000cd310, 0, 0, {XSDTYPE_SHORT, XSDTYPE_INT, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ L"Name", 0x00010109, 0, 0, {XSDTYPE_NAME, XSDTYPE_TOKEN, XSDFLAG_DERIVED, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"decimal", 0x03912567, 0, 0, {XSDTYPE_DECIMAL, XSDTYPE_UNK, XSDFLAG_PRIMITIVE, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE} },
	{ L"gYear", 0x000b7108, 0, 0, {XSDTYPE_GYEAR, XSDTYPE_UNK, XSDFLAG_DERIVED, FACET_DURATION | FACET_PERIOD | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ L"QName", 0x00091cfa, 0, 0, {XSDTYPE_QNAME, XSDTYPE_UNK, XSDFLAG_PRIMITIVE, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE} },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"ENTITY", 0x00470435, 98, 1, {XSDTYPE_ENTITY, XSDTYPE_UNK, XSDFLAG_PRIMITIVE, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE} },
	{ L"float", 0x000b90ae, 0, 0, {XSDTYPE_FLOAT, XSDTYPE_UNK, XSDFLAG_PRIMITIVE, FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE} },
	{ L"time", 0x00016fc7, 0, 0, {XSDTYPE_TIME, XSDTYPE_UNK, XSDFLAG_DERIVED, FACET_DURATION | FACET_PERIOD | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ L"boolean", 0x038b29b0, 0, 0, {XSDTYPE_BOOLEAN, XSDTYPE_UNK, XSDFLAG_PRIMITIVE, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_PATTERN} },
	{ L"negativeInteger", 0x0a9cfad1, 0, 0, {XSDTYPE_NEGATIVEINTEGER, XSDTYPE_NONPOSITIVEINTEGER, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE} },
	{ L"NCName", 0x004dfdaa, 0, 0, {XSDTYPE_NMTOKEN, XSDTYPE_NAME, XSDFLAG_DERIVED, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION | FACET_WHITESPACE} },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ 0, 0x00000000, 0, 0,  },
	{ L"base64Binary", 0x3c32ab72, 0, 0, {XSDTYPE_BASE64BINARY, XSDTYPE_UNK, XSDFLAG_ABSTRACT | XSDFLAG_PRIMITIVE, FACET_ENCODING | FACET_LENGTH | FACET_PATTERN | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_ENUMERATION}  },
	{ L"NMTOKEN", 0x02c74884, 0, 0, {XSDTYPE_NMTOKEN, XSDTYPE_TOKEN, XSDFLAG_DERIVED, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_PATTERN | FACET_ENUMERATION | FACET_WHITESPACE}  },
	{ L"unsignedShort", 0x6d8f7ad5, 91, 0, {XSDTYPE_UNSIGNEDSHORT, XSDTYPE_UNSIGNEDINT, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE}  },
	{ L"gDay", 0x00013eb5, 0, 0, {XSDTYPE_GDAY, XSDTYPE_UNK, XSDFLAG_DERIVED, FACET_DURATION | FACET_PERIOD | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE}  },
	{ L"unsignedByte", 0xb6d6e2b1, 0, 0, {XSDTYPE_UNSIGNEDBYTE, XSDTYPE_UNSIGNEDSHORT, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE}  },
	{ L"gMonthDay", 0x222e572b, 0, 0, {XSDTYPE_GMONTHDAY, XSDTYPE_UNK, XSDFLAG_DERIVED, FACET_DURATION | FACET_PERIOD | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE}  },
	{ L"unsignedLong", 0xb6d6fbcd, 0, 0, {XSDTYPE_UNSIGNEDLONG, XSDTYPE_NONNEGATIVEINTEGER, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE}  },
	{ L"unsignedInt", 0x30c28cc0, 0, 0, {XSDTYPE_UNSIGNEDINT, XSDTYPE_UNSIGNEDLONG, XSDFLAG_DERIVED, FACET_PRECISION | FACET_SCALE | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE}  },
	{ L"NMTOKENS", 0x19018cf7, 0, 0, {XSDTYPE_NMTOKENS, XSDTYPE_NMTOKEN, XSDFLAG_DERIVED, FACET_LENGTH | FACET_MINLENGTH | FACET_MAXLENGTH | FACET_ENUMERATION | FACET_WHITESPACE}  },
	{ L"date", 0x00013fee, 0, 0, {XSDTYPE_DATE, XSDTYPE_DATE, XSDFLAG_DERIVED, FACET_DURATION | FACET_PERIOD | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE}  },
	{ L"gMonth", 0x0065e6cd, 0, 0, {XSDTYPE_GMONTH, XSDTYPE_UNK, XSDFLAG_DERIVED, FACET_DURATION | FACET_PERIOD | FACET_PATTERN | FACET_ENUMERATION | FACET_MAXINCLUSIVE | FACET_MAXEXCLUSIVE | FACET_MININCLUSIVE | FACET_MINEXCLUSIVE | FACET_WHITESPACE}  },
};

