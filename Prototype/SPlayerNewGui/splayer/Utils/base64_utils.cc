#include "stdafx.h"
#include "base64_utils.h"

namespace base64_utils
{

char Base64EncodeSingle(unsigned char uc)
{
  if (uc < 26)
    return 'A' + uc;
  if (uc < 52)
    return 'a' + (uc-26);
  if (uc < 62)
    return '0' + (uc-52);
  if (uc == 62)
    return '+';
  return '/';
}

std::string Encode(std::vector<unsigned char>& buffer)
{
  std::string ret;
  for (size_t i = 0; i < buffer.size(); i += 3)
  {
    unsigned char by1=0, by2=0, by3=0;
    by1 = buffer[i];
    if (i + 1 < buffer.size())
	    by2 = buffer[i+1];
    if (i + 2 < buffer.size())
	    by3 = buffer[i+2];

    unsigned char by4=0, by5=0, by6=0, by7=0;
    by4 = by1 >> 2;
    by5 = ((by1 & 0x3) << 4) | (by2 >> 4);
    by6 = ((by2 & 0xf) << 2) | (by3 >> 6);
    by7 = by3 & 0x3f;
    ret += Base64EncodeSingle(by4);
    ret += Base64EncodeSingle(by5);

    if (i + 1 < buffer.size())
	    ret += Base64EncodeSingle(by6);
    else
	    ret += "=";

    if (i + 2 < buffer.size())
	    ret += Base64EncodeSingle(by7);
    else
	    ret += "=";
    if (i % (76 / 4 * 3) == 0)
	    ret += "\r\n";
  }
  return ret;
}

unsigned char Base64DecodeSingle(char c)
{
  if (c >= 'A' && c <= 'Z')
    return c - 'A';
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 26;
  if (c >= '0' && c <= '9')
    return c - '0' + 52;
  if (c == '+')
    return 62;
	return 63;
}

bool IsBase64(char c)
{
  if (c >= 'A' && c <= 'Z')
    return true;
  if (c >= 'a' && c <= 'z')
    return true;
  if (c >= '0' && c <= '9')
    return true;
  if (c == '+')
    return true;
  if (c == '/')
    return true;
  if (c == '=')
    return true;
	return false;
}

std::vector<unsigned char> Decode(std::string& input)
{
  std::vector<unsigned char> ret;

  std::string str;
  for (size_t i = 0; i < input.length(); i++)
  {
    if (IsBase64(input[i]))
	    str += input[i];
  }

  if (str.length() == 0)
    return ret;

  for (size_t i=0; i < str.length(); i+=4)
  {
    char c1='A', c2='A', c3='A', c4='A';
    c1 = str[i];
    if (i + 1 < str.length())
	    c2 = str[i+1];
    if (i + 2 < str.length())
	    c3 = str[i+2];
    if (i + 3 <str.length())
	    c4 = str[i+3];
    unsigned char by1=0, by2=0, by3=0, by4=0;
    by1 = Base64DecodeSingle(c1);
    by2 = Base64DecodeSingle(c2);
    by3 = Base64DecodeSingle(c3);
    by4 = Base64DecodeSingle(c4);
    ret.push_back( (by1 << 2) | (by2 >> 4) );
    if (c3 != '=')
	    ret.push_back( ((by2 & 0xf) << 4) | (by3 >> 2) );
    if (c4 != '=')
	    ret.push_back( ((by3 & 0x3) << 6) | by4 );
  }

  return ret;
}

} // namespace base64_utils