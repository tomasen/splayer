#ifndef BASE64_UTILS_H
#define BASE64_UTILS_H

namespace base64_utils
{

std::string Encode(std::vector<unsigned char>& buffer);
std::vector<unsigned char> Decode(std::string& input);

} // namespace base64_utils

#endif // BASE64_UTILS_H