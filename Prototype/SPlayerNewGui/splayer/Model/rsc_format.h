#ifndef RSC_FORMAT_H
#define RSC_FORMAT_H

namespace rsc_format
{

bool Parse(const wchar_t* filename, std::map<std::wstring, std::wstring>& str_output,
           std::map<std::wstring, std::vector<unsigned char> >& buf_output);

} // namespace rsc_format

#endif // RSC_FORMAT_H